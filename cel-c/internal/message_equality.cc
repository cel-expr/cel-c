// Copyright 2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "cel-c/internal/message_equality.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/internal/any.h"
#include "cel-c/internal/number.h"
#include "cel-c/internal/setjmp.h"
#include "cel-c/string_view.h"
#include "cel-c/well_known_types.h"
#include "upb/base/descriptor_constants.h"
#include "upb/base/string_view.h"
#include "upb/message/array.h"
#include "upb/message/map.h"
#include "upb/message/message.h"
#include "upb/reflection/def.h"
#include "upb/reflection/message.h"
#include "upb/wire/eps_copy_input_stream.h"
#include "upb/wire/reader.h"
#include "upb/wire/types.h"

typedef struct _cel_UnknownField _cel_UnknownField;
typedef struct _cel_UnknownFields _cel_UnknownFields;

struct _cel_UnknownFields {
  _cel_UnknownField* cel_nullable head;
  _cel_UnknownField* cel_nullable tail;
  _cel_UnknownField* cel_nullable mid;
  uint32_t size;
  int32_t balance;
};

struct _cel_UnknownField {
  _cel_UnknownField* cel_nullable prev;
  _cel_UnknownField* cel_nullable next;
#ifdef _MSC_VER
#pragma pack(push, 4)
#endif
  union CEL_ATTRIBUTE_PACKED(4) {
    uint64_t varint;
    uint64_t fixed64;
    uint32_t fixed32;
    struct CEL_ATTRIBUTE_PACKED(4) {
      const char* cel_nullability_unknown data;
      uint32_t size;
    } delimited;
    _cel_UnknownFields* cel_nullable group;
  } data;
#ifdef _MSC_VER
#pragma pack(pop)
#endif
  uint32_t field_num : 29;
  uint8_t wire_type : 3;
};

typedef struct {
  const upb_DefPool* const cel_nonnull def_pool;
  const cel_WellKnownTypes* const cel_nonnull wkts;
  cel_Allocator* const cel_nonnull alloc;
  cel_Arena* cel_nullable arena;
  upb_EpsCopyInputStream stream;
  _cel_MessageEquality result;
  int depth;
  _cel_jmp_buf jmp;
} _cel_MessageEqualityState;

CEL_ATTRIBUTE_NORETURN
static void _cel_MessageEqualityState_Throw(
    _cel_MessageEqualityState* cel_nonnull state, _cel_MessageEquality result) {
  CEL_ASSERT_NE(result, _cel_MessageEquality_kEqual);
  CEL_ASSERT_NE(result, _cel_MessageEquality_kNotEqual);

  state->result = result;
  _cel_longjmp(state->jmp);
}

CEL_ATTRIBUTE_NODISCARD
static cel_Arena* cel_nonnull
_cel_MessageEqualityState_Arena(_cel_MessageEqualityState* cel_nonnull state) {
  cel_Arena* arena = state->arena;
  if (arena == cel_nullptr) {
    arena = state->arena = cel_Arena_New(state->alloc);
    if (CEL_UNLIKELY(arena == cel_nullptr)) {
      _cel_MessageEqualityState_Throw(state, _cel_MessageEquality_kOutOfMemory);
    }
  }
  return arena;
}

CEL_ATTRIBUTE_NODISCARD
static _cel_UnknownFields* cel_nonnull
_cel_UnknownFields_New(_cel_MessageEqualityState* cel_nonnull state) {
  _cel_UnknownFields* fields = reinterpret_cast<_cel_UnknownFields*>(
      cel_Arena_Malloc(_cel_MessageEqualityState_Arena(state),
                       sizeof(_cel_UnknownFields), cel_nullptr));
  if (CEL_UNLIKELY(fields == cel_nullptr)) {
    _cel_MessageEqualityState_Throw(state, _cel_MessageEquality_kOutOfMemory);
  }
  fields->head = cel_nullptr;
  fields->tail = cel_nullptr;
  fields->mid = cel_nullptr;
  fields->size = 0;
  fields->balance = 0;
  return fields;
}

CEL_ATTRIBUTE_NODISCARD
static _cel_UnknownField* cel_nonnull _cel_UnknownField_New(
    _cel_MessageEqualityState* cel_nonnull state, uint32_t tag) {
  _cel_UnknownField* field = reinterpret_cast<_cel_UnknownField*>(
      cel_Arena_Malloc(_cel_MessageEqualityState_Arena(state),
                       sizeof(_cel_UnknownField), cel_nullptr));
  if (CEL_UNLIKELY(field == cel_nullptr)) {
    _cel_MessageEqualityState_Throw(state, _cel_MessageEquality_kOutOfMemory);
  }
  field->prev = cel_nullptr;
  field->next = cel_nullptr;
  field->field_num = upb_WireReader_GetFieldNumber(tag);
  field->wire_type = upb_WireReader_GetWireType(tag);
  return field;
}

CEL_ATTRIBUTE_NODISCARD
static int _cel_UnknownField_Compare(const _cel_UnknownField* cel_nonnull lhs,
                                     const _cel_UnknownField* cel_nonnull rhs) {
  const uint32_t lhs_field_num = lhs->field_num;
  const uint32_t rhs_field_num = rhs->field_num;
  if (lhs_field_num < rhs_field_num) {
    return -1;
  }
  if (lhs_field_num > rhs_field_num) {
    return 1;
  }
  const uint8_t lhs_wire_type = lhs->wire_type;
  const uint8_t rhs_wire_type = rhs->wire_type;
  return lhs_wire_type < rhs_wire_type   ? -1
         : lhs_wire_type > rhs_wire_type ? 1
                                         : 0;
}

static void _cel_UnknownFields_Add(_cel_UnknownFields* cel_nonnull fields,
                                   _cel_UnknownField* cel_nonnull field) {
  if (fields->tail != cel_nullptr) {
    _cel_UnknownField* tail = fields->tail;
    if (_cel_UnknownField_Compare(field, tail) >= 0) {
      // Fast path. Fields are in order already.
      field->prev = tail;
      tail->next = field;
      fields->tail = field;
      ++fields->balance;
    } else {
      // Slow path.
      _cel_UnknownField* node = fields->mid;
      CEL_ASSERT_NOT_NULL(node);
      while (_cel_UnknownField_Compare(field, node) < 0) {
        node = node->prev;
        if (node == cel_nullptr) {
          field->next = fields->head;
          fields->head->prev = field;
          fields->head = field;
          goto inserted;
        }
        CEL_ASSERT_NOT_NULL(node);
      }
      CEL_ASSERT_NOT_NULL(node);
      while (_cel_UnknownField_Compare(field, node) >= 0) {
        node = node->next;
        CEL_ASSERT_NOT_NULL(node);
      }
      field->next = node;
      field->prev = node->prev;
      if (node->prev != cel_nullptr) {
        node->prev->next = field;
        node->prev = field;
      } else {
        fields->head = field;
      }
    }
  inserted:
    fields->balance += _cel_UnknownField_Compare(field, fields->mid);
  } else {
    fields->head = fields->mid = fields->tail = field;
  }
  ++fields->size;
  if (fields->size > 2 && (fields->size & 1) == 1) {
    if (fields->balance < 0) {
      fields->mid = fields->mid->prev;
    } else if (fields->balance > 0) {
      fields->mid = fields->mid->next;
    }
    fields->balance = 0;
  }
}

CEL_ATTRIBUTE_NODISCARD
static _cel_UnknownFields* cel_nullable
_cel_UnknownFields_Read(_cel_MessageEqualityState* cel_nonnull state,
                        _cel_UnknownFields* fields, const char** buf) {
  const char* ptr = *buf;
  while (!upb_EpsCopyInputStream_IsDone(&state->stream, &ptr)) {
    uint32_t tag;
    ptr = upb_WireReader_ReadTag(ptr, &tag, &state->stream);
    uint8_t wire_type = upb_WireReader_GetWireType(tag);
    if (wire_type == kUpb_WireType_EndGroup) {
      break;
    }
    if (CEL_UNLIKELY(fields == cel_nullptr)) {
      fields = _cel_UnknownFields_New(state);
    }
    _cel_UnknownField* field = _cel_UnknownField_New(state, tag);
    switch (wire_type) {
      case kUpb_WireType_Varint:
        ptr =
            upb_WireReader_ReadVarint(ptr, &field->data.varint, &state->stream);
        break;
      case kUpb_WireType_64Bit:
        ptr = upb_WireReader_ReadFixed64(ptr, &field->data.fixed64,
                                         &state->stream);
        break;
      case kUpb_WireType_32Bit:
        ptr = upb_WireReader_ReadFixed32(ptr, &field->data.fixed32,
                                         &state->stream);
        break;
      case kUpb_WireType_Delimited: {
        int size;
        upb_StringView sv;
        ptr = upb_WireReader_ReadSize(ptr, &size, &state->stream);
        ptr = upb_EpsCopyInputStream_ReadStringAlwaysAlias(&state->stream, ptr,
                                                           size, &sv);
        field->data.delimited.data = sv.data;
        field->data.delimited.size = sv.size;
      } break;
      case kUpb_WireType_StartGroup:
        CEL_ASSERT_GT(state->depth, 0);
        if (--state->depth == 0) {
          _cel_MessageEqualityState_Throw(
              state, _cel_MessageEquality_kMaxDepthExceeded);
        }
        field->data.group = _cel_UnknownFields_Read(state, cel_nullptr, &ptr);
        ++state->depth;
        break;
      default:
        CEL_UNREACHABLE();
    }
    _cel_UnknownFields_Add(fields, field);
  }
  *buf = ptr;
  return fields;
}

CEL_ATTRIBUTE_NODISCARD
static _cel_UnknownFields* cel_nullable
_cel_UnknownFields_FromMessage(_cel_MessageEqualityState* cel_nonnull state,
                               const upb_Message* cel_nonnull msg) {
  _cel_UnknownFields* fields = cel_nullptr;
  cel_StringView unknown;
  uintptr_t iter = kUpb_Message_UnknownBegin;
  while (upb_Message_NextUnknown(msg, &unknown, &iter)) {
    upb_EpsCopyInputStream_Init(&state->stream, &unknown.data,
                                cel_StringView_Size(unknown));
    fields = _cel_UnknownFields_Read(state, fields, &unknown.data);
    CEL_ASSERT(upb_EpsCopyInputStream_IsDone(&state->stream, &unknown.data) &&
               !upb_EpsCopyInputStream_IsError(&state->stream));
  }
  return fields;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_UnknownFields_Equals(
    _cel_MessageEqualityState* cel_nonnull state,
    const _cel_UnknownFields* cel_nullable lhs_fields,
    const _cel_UnknownFields* cel_nullable rhs_fields) {
  const size_t lhs_fields_size =
      lhs_fields != cel_nullptr ? lhs_fields->size : 0;
  const size_t rhs_fields_size =
      rhs_fields != cel_nullptr ? rhs_fields->size : 0;
  if (lhs_fields_size != rhs_fields_size) {
    return false;
  }
  if (lhs_fields_size == 0) {
    return true;
  }

  const _cel_UnknownField* lhs_field = lhs_fields->head;
  const _cel_UnknownField* rhs_field = rhs_fields->head;
  for (size_t i = 0; i < lhs_fields_size;
       ++i, lhs_field = lhs_field->next, rhs_field = rhs_field->next) {
    if (lhs_field->field_num != rhs_field->field_num ||
        lhs_field->wire_type != rhs_field->wire_type) {
      return false;
    }
    switch (lhs_field->wire_type) {
      case kUpb_WireType_Varint:
        if (lhs_field->data.varint != rhs_field->data.varint) {
          return false;
        }
        break;
      case kUpb_WireType_64Bit:
        if (lhs_field->data.fixed64 != rhs_field->data.fixed64) {
          return false;
        }
        break;
      case kUpb_WireType_32Bit:
        if (lhs_field->data.fixed32 != rhs_field->data.fixed32) {
          return false;
        }
        break;
      case kUpb_WireType_Delimited:
        if (!cel_StringView_Equals(
                cel_StringView_FromArray(lhs_field->data.delimited.data,
                                         lhs_field->data.delimited.size),
                cel_StringView_FromArray(rhs_field->data.delimited.data,
                                         rhs_field->data.delimited.size))) {
          return false;
        }
        break;
      case kUpb_WireType_StartGroup:
        if (!_cel_UnknownFields_Equals(state, lhs_field->data.group,
                                       rhs_field->data.group)) {
          return false;
        }
        break;
      default:
        CEL_UNREACHABLE();
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_MessageEqualityState_UnknownEquals(
    _cel_MessageEqualityState* cel_nonnull state,
    const upb_Message* cel_nonnull lhs, const upb_Message* cel_nonnull rhs) {
  const bool lhs_has_unknown = upb_Message_HasUnknown(lhs);
  const bool rhs_has_unknown = upb_Message_HasUnknown(rhs);
  if (!lhs_has_unknown && !rhs_has_unknown) {
    return true;
  }
  if (!(lhs_has_unknown && rhs_has_unknown)) {
    return false;
  }

  _cel_UnknownFields* lhs_fields = _cel_UnknownFields_FromMessage(state, lhs);
  _cel_UnknownFields* rhs_fields = _cel_UnknownFields_FromMessage(state, rhs);
  return _cel_UnknownFields_Equals(state, lhs_fields, rhs_fields);
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_MessageEqualityState_FieldHomoEquals(
    _cel_MessageEqualityState* cel_nonnull state, upb_MessageValue lhs_val,
    upb_MessageValue rhs_val, const upb_FieldDef* cel_nonnull val_def);

CEL_ATTRIBUTE_NODISCARD
static bool _cel_MessageEqualityState_HomoEquals(
    _cel_MessageEqualityState* cel_nonnull state,
    const upb_Message* cel_nonnull lhs_val,
    const upb_Message* cel_nonnull rhs_val,
    const upb_MessageDef* cel_nonnull val_def) {
  if (lhs_val == rhs_val) {
    return true;
  }

  const upb_FieldDef* lhs_field_def;
  const upb_FieldDef* rhs_field_def;
  upb_MessageValue lhs_field_val;
  upb_MessageValue rhs_field_val;
  size_t lhs_iter = static_cast<size_t>(kUpb_Message_Begin);
  size_t rhs_iter = static_cast<size_t>(kUpb_Message_Begin);
  while (true) {
    bool lhs_has_next =
        upb_Message_Next(lhs_val, val_def, state->def_pool, &lhs_field_def,
                         &lhs_field_val, &lhs_iter);
    bool rhs_has_next =
        upb_Message_Next(rhs_val, val_def, state->def_pool, &rhs_field_def,
                         &rhs_field_val, &rhs_iter);
    if (lhs_has_next != rhs_has_next) {
      return false;
    }
    if (!lhs_has_next) {
      break;
    }
    if (lhs_field_def != rhs_field_def) {
      return false;
    }
    if (!_cel_MessageEqualityState_FieldHomoEquals(
            state, lhs_field_val, rhs_field_val, rhs_field_def)) {
      return false;
    }
  }

  return _cel_MessageEqualityState_UnknownEquals(state, lhs_val, rhs_val);
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_MessageEqualityState_SingularFieldHomoEquals(
    _cel_MessageEqualityState* cel_nonnull state, upb_MessageValue lhs_val,
    upb_MessageValue rhs_val, const upb_FieldDef* cel_nonnull val_def) {
  CEL_ASSERT_NOT(upb_FieldDef_IsMap(val_def));

  switch (upb_FieldDef_CType(val_def)) {
    case kUpb_CType_Bool:
      return lhs_val.bool_val == rhs_val.bool_val;
    case kUpb_CType_Float:
      return lhs_val.float_val == rhs_val.float_val;
    case kUpb_CType_Enum:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case kUpb_CType_Int32:
      return lhs_val.int32_val == rhs_val.int32_val;
    case kUpb_CType_UInt32:
      return lhs_val.uint32_val == rhs_val.uint32_val;
    case kUpb_CType_Message:
      return _cel_MessageEqualityState_HomoEquals(
          state, lhs_val.msg_val, rhs_val.msg_val,
          upb_FieldDef_MessageSubDef(val_def));
    case kUpb_CType_Double:
      return lhs_val.double_val == rhs_val.double_val;
    case kUpb_CType_Int64:
      return lhs_val.int64_val == rhs_val.int64_val;
    case kUpb_CType_UInt64:
      return lhs_val.uint64_val == rhs_val.uint64_val;
    case kUpb_CType_String:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case kUpb_CType_Bytes:
      return cel_StringView_Equals(lhs_val.str_val, rhs_val.str_val);
    default:
      CEL_UNREACHABLE();
  }
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_MessageEqualityState_MapFieldHomoEquals(
    _cel_MessageEqualityState* cel_nonnull state,
    const upb_Map* cel_nullable lhs_val, const upb_Map* cel_nullable rhs_val,
    const upb_FieldDef* cel_nonnull val_def) {
  const size_t lhs_val_size =
      lhs_val != cel_nullptr ? upb_Map_Size(lhs_val) : 0;
  const size_t rhs_val_size =
      rhs_val != cel_nullptr ? upb_Map_Size(rhs_val) : 0;

  if (lhs_val_size != rhs_val_size) {
    return false;
  }

  if (lhs_val_size == 0) {
    return true;
  }

  const upb_MessageDef* val_entry_def = upb_FieldDef_MessageSubDef(val_def);
  const upb_FieldDef* val_value_def = upb_MessageDef_FindFieldByNumber(
      val_entry_def, kUpb_MapEntry_ValueFieldNumber);

  size_t lhs_iter = kUpb_Map_Begin;
  upb_MessageValue lhs_val_key;
  upb_MessageValue lhs_val_value;
  while (upb_Map_Next(lhs_val, &lhs_val_key, &lhs_val_value, &lhs_iter)) {
    upb_MessageValue rhs_val_value;
    if (!upb_Map_Get(rhs_val, lhs_val_key, &rhs_val_value)) {
      return false;
    }
    if (!_cel_MessageEqualityState_SingularFieldHomoEquals(
            state, lhs_val_value, rhs_val_value, val_value_def)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_MessageEqualityState_RepeatedFieldHomoEquals(
    _cel_MessageEqualityState* cel_nonnull state,
    const upb_Array* cel_nullable lhs_val,
    const upb_Array* cel_nullable rhs_val,
    const upb_FieldDef* cel_nonnull val_def) {
  const size_t lhs_val_size =
      lhs_val != cel_nullptr ? upb_Array_Size(lhs_val) : 0;
  const size_t rhs_val_size =
      rhs_val != cel_nullptr ? upb_Array_Size(rhs_val) : 0;

  if (lhs_val_size != rhs_val_size) {
    return false;
  }

  if (lhs_val_size == 0) {
    return true;
  }

  for (size_t i = 0; i < lhs_val_size; ++i) {
    if (!_cel_MessageEqualityState_SingularFieldHomoEquals(
            state, upb_Array_Get(lhs_val, i), upb_Array_Get(rhs_val, i),
            val_def)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_MessageEqualityState_FieldHomoEquals(
    _cel_MessageEqualityState* cel_nonnull state, upb_MessageValue lhs_val,
    upb_MessageValue rhs_val, const upb_FieldDef* cel_nonnull val_def) {
  if (upb_FieldDef_IsMap(val_def)) {
    return _cel_MessageEqualityState_MapFieldHomoEquals(
        state, lhs_val.map_val, rhs_val.map_val, val_def);
  }
  if (upb_FieldDef_IsRepeated(val_def)) {
    return _cel_MessageEqualityState_RepeatedFieldHomoEquals(
        state, lhs_val.array_val, rhs_val.array_val, val_def);
  }
  return _cel_MessageEqualityState_SingularFieldHomoEquals(state, lhs_val,
                                                           rhs_val, val_def);
}

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM {
  _cel_MessageEqualityFieldKind_kScalar = 1,
  _cel_MessageEqualityFieldKind_kMessage,
  _cel_MessageEqualityFieldKind_kNullValue,
  _cel_MessageEqualityFieldKind_kRepeated,
  _cel_MessageEqualityFieldKind_kMap,
} _cel_MessageEqualityFieldKind;

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM {
  _cel_MessageEqualityScalarFieldKind_kBool = 1,
  _cel_MessageEqualityScalarFieldKind_kNumber,
  _cel_MessageEqualityScalarFieldKind_kString,
  _cel_MessageEqualityScalarFieldKind_kBytes,
} _cel_MessageEqualityScalarFieldKind;

typedef struct {
  struct {
    union {
      _cel_MessageEqualityScalarFieldKind scalar_def;
      const upb_MessageDef* message_def;
      const upb_FieldDef* field_def;
    } def;
    union {
      bool bool_val;
      _cel_Number num_val;
      cel_StringView str_val;
      const upb_Array* array_val;
      const upb_Map* map_val;
      const upb_Message* msg_val;
    } val;
  } data;
  _cel_MessageEqualityFieldKind kind;
} _cel_MessageEqualityField;

static void _cel_MessageEqualityState_Field(
    _cel_MessageEqualityState* cel_nonnull state, upb_MessageValue in_val,
    const upb_FieldDef* cel_nonnull in_def, bool single,
    _cel_MessageEqualityField* cel_nonnull out) {
  if (!single) {
    if (upb_FieldDef_IsMap(in_def)) {
      out->data.val.map_val = in_val.map_val;
      out->data.def.field_def = in_def;
      out->kind = _cel_MessageEqualityFieldKind_kMap;
      return;
    }
    if (upb_FieldDef_IsRepeated(in_def)) {
      out->data.val.array_val = in_val.array_val;
      out->data.def.field_def = in_def;
      out->kind = _cel_MessageEqualityFieldKind_kRepeated;
      return;
    }
  }
  switch (upb_FieldDef_CType(in_def)) {
    case kUpb_CType_Bool:
      out->data.def.scalar_def = _cel_MessageEqualityScalarFieldKind_kBool;
      out->data.val.bool_val = in_val.bool_val;
      out->kind = _cel_MessageEqualityFieldKind_kScalar;
      break;
    case kUpb_CType_Float:
      out->data.def.scalar_def = _cel_MessageEqualityScalarFieldKind_kNumber;
      out->data.val.num_val = _cel_DoubleNumber(in_val.float_val);
      out->kind = _cel_MessageEqualityFieldKind_kScalar;
      break;
    case kUpb_CType_Int32:
      out->data.def.scalar_def = _cel_MessageEqualityScalarFieldKind_kNumber;
      out->data.val.num_val = _cel_IntNumber(in_val.int32_val);
      out->kind = _cel_MessageEqualityFieldKind_kScalar;
      break;
    case kUpb_CType_UInt32:
      out->data.def.scalar_def = _cel_MessageEqualityScalarFieldKind_kNumber;
      out->data.val.num_val = _cel_UintNumber(in_val.uint32_val);
      out->kind = _cel_MessageEqualityFieldKind_kScalar;
      break;
    case kUpb_CType_Enum: {
      const upb_EnumDef* enum_def = upb_FieldDef_EnumSubDef(in_def);
      const char* enum_def_name = upb_EnumDef_FullName(enum_def);
      if (enum_def_name != cel_nullptr &&
          strcmp(enum_def_name, "google.protobuf.NullValue") == 0) {
        out->kind = _cel_MessageEqualityFieldKind_kNullValue;
      } else {
        out->data.def.scalar_def = _cel_MessageEqualityScalarFieldKind_kNumber;
        out->data.val.num_val = _cel_IntNumber(in_val.int32_val);
        out->kind = _cel_MessageEqualityFieldKind_kScalar;
      }
    } break;
    case kUpb_CType_Message: {
      const upb_MessageDef* message_def = upb_FieldDef_MessageSubDef(in_def);
      upb_WellKnown well_known = upb_MessageDef_WellKnownType(message_def);
      if (well_known == kUpb_WellKnown_Any) {
        CEL_ASSERT_EQ(message_def, state->wkts->any.def);
        upb_Message* out_message;
        const upb_MessageDef* out_message_def;
        cel_Arena* arena = _cel_MessageEqualityState_Arena(state);
        switch (_cel_AnyUnpack(in_val.msg_val, state->def_pool,
                               &state->wkts->any, arena, &out_message,
                               &out_message_def)) {
          case _cel_AnyUnpackResult_kOk:
            in_val.msg_val = out_message;
            message_def = out_message_def;
            well_known = upb_MessageDef_WellKnownType(message_def);
            break;
          case _cel_AnyUnpackResult_kOutOfMemory:
            _cel_MessageEqualityState_Throw(state,
                                            _cel_MessageEquality_kOutOfMemory);
          default:
            message_def =
                out_message_def != cel_nullptr ? out_message_def : message_def;
            in_val.msg_val =
                out_message != cel_nullptr ? out_message : in_val.msg_val;
            well_known = upb_MessageDef_WellKnownType(message_def);
            break;
        }
      }
      switch (well_known) {
        case kUpb_WellKnown_DoubleValue:
          CEL_ASSERT_EQ(message_def, state->wkts->double_value.def);
          out->data.def.scalar_def =
              _cel_MessageEqualityScalarFieldKind_kNumber;
          out->data.val.num_val = _cel_DoubleNumber(
              upb_Message_GetFieldByDef(in_val.msg_val,
                                        state->wkts->double_value.value_def)
                  .double_val);
          out->kind = _cel_MessageEqualityFieldKind_kScalar;
          break;
        case kUpb_WellKnown_FloatValue:
          CEL_ASSERT_EQ(message_def, state->wkts->float_value.def);
          out->data.def.scalar_def =
              _cel_MessageEqualityScalarFieldKind_kNumber;
          out->data.val.num_val = _cel_DoubleNumber(
              upb_Message_GetFieldByDef(in_val.msg_val,
                                        state->wkts->float_value.value_def)
                  .float_val);
          out->kind = _cel_MessageEqualityFieldKind_kScalar;
          break;
        case kUpb_WellKnown_Int64Value:
          CEL_ASSERT_EQ(message_def, state->wkts->int64_value.def);
          out->data.def.scalar_def =
              _cel_MessageEqualityScalarFieldKind_kNumber;
          out->data.val.num_val = _cel_IntNumber(
              upb_Message_GetFieldByDef(in_val.msg_val,
                                        state->wkts->int64_value.value_def)
                  .int64_val);
          out->kind = _cel_MessageEqualityFieldKind_kScalar;
          break;
        case kUpb_WellKnown_UInt64Value:
          CEL_ASSERT_EQ(message_def, state->wkts->uint64_value.def);
          out->data.def.scalar_def =
              _cel_MessageEqualityScalarFieldKind_kNumber;
          out->data.val.num_val = _cel_UintNumber(
              upb_Message_GetFieldByDef(in_val.msg_val,
                                        state->wkts->uint64_value.value_def)
                  .uint64_val);
          out->kind = _cel_MessageEqualityFieldKind_kScalar;
          break;
        case kUpb_WellKnown_Int32Value:
          CEL_ASSERT_EQ(message_def, state->wkts->int32_value.def);
          out->data.def.scalar_def =
              _cel_MessageEqualityScalarFieldKind_kNumber;
          out->data.val.num_val = _cel_IntNumber(
              upb_Message_GetFieldByDef(in_val.msg_val,
                                        state->wkts->int32_value.value_def)
                  .int32_val);
          out->kind = _cel_MessageEqualityFieldKind_kScalar;
          break;
        case kUpb_WellKnown_UInt32Value:
          CEL_ASSERT_EQ(message_def, state->wkts->uint32_value.def);
          out->data.def.scalar_def =
              _cel_MessageEqualityScalarFieldKind_kNumber;
          out->data.val.num_val = _cel_UintNumber(
              upb_Message_GetFieldByDef(in_val.msg_val,
                                        state->wkts->uint32_value.value_def)
                  .uint32_val);
          out->kind = _cel_MessageEqualityFieldKind_kScalar;
          break;
        case kUpb_WellKnown_StringValue:
          CEL_ASSERT_EQ(message_def, state->wkts->string_value.def);
          out->data.def.scalar_def =
              _cel_MessageEqualityScalarFieldKind_kString;
          out->data.val.str_val =
              upb_Message_GetFieldByDef(in_val.msg_val,
                                        state->wkts->string_value.value_def)
                  .str_val;
          out->kind = _cel_MessageEqualityFieldKind_kScalar;
          break;
        case kUpb_WellKnown_BytesValue:
          CEL_ASSERT_EQ(message_def, state->wkts->bytes_value.def);
          out->data.def.scalar_def = _cel_MessageEqualityScalarFieldKind_kBytes;
          out->data.val.str_val =
              upb_Message_GetFieldByDef(in_val.msg_val,
                                        state->wkts->bytes_value.value_def)
                  .str_val;
          out->kind = _cel_MessageEqualityFieldKind_kScalar;
          break;
        case kUpb_WellKnown_BoolValue:
          CEL_ASSERT_EQ(message_def, state->wkts->bool_value.def);
          out->data.def.scalar_def = _cel_MessageEqualityScalarFieldKind_kBool;
          out->data.val.bool_val =
              upb_Message_GetFieldByDef(in_val.msg_val,
                                        state->wkts->bool_value.value_def)
                  .bool_val;
          out->kind = _cel_MessageEqualityFieldKind_kScalar;
          break;
        case kUpb_WellKnown_Value: {
          CEL_ASSERT_EQ(message_def, state->wkts->value.def);
          const upb_FieldDef* field_def = upb_Message_WhichOneofByDef(
              in_val.msg_val, state->wkts->value.kind_def);
          if (field_def == cel_nullptr) {
            out->kind = _cel_MessageEqualityFieldKind_kNullValue;
          } else {
            switch (upb_FieldDef_Number(field_def)) {
              case 1:  // null_value
                out->kind = _cel_MessageEqualityFieldKind_kNullValue;
                break;
              case 2:  // number_value
                out->data.def.scalar_def =
                    _cel_MessageEqualityScalarFieldKind_kNumber;
                out->data.val.num_val = _cel_DoubleNumber(
                    upb_Message_GetFieldByDef(in_val.msg_val, field_def)
                        .double_val);
                out->kind = _cel_MessageEqualityFieldKind_kScalar;
                break;
              case 3:  // string_value
                out->data.def.scalar_def =
                    _cel_MessageEqualityScalarFieldKind_kString;
                out->data.val.str_val =
                    upb_Message_GetFieldByDef(in_val.msg_val, field_def)
                        .str_val;
                out->kind = _cel_MessageEqualityFieldKind_kScalar;
                break;
              case 4:  // bool_value
                out->data.def.scalar_def =
                    _cel_MessageEqualityScalarFieldKind_kBool;
                out->data.val.bool_val =
                    upb_Message_GetFieldByDef(in_val.msg_val, field_def)
                        .bool_val;
                out->kind = _cel_MessageEqualityFieldKind_kScalar;
                break;
              case 5:  // struct_value
                out->data.val.map_val =
                    upb_Message_GetFieldByDef(
                        upb_Message_GetFieldByDef(in_val.msg_val, field_def)
                            .msg_val,
                        state->wkts->struct_value.fields_def)
                        .map_val;
                out->data.def.field_def = state->wkts->struct_value.fields_def;
                out->kind = _cel_MessageEqualityFieldKind_kMap;
                break;
              case 6:  // list_value
                out->data.val.array_val =
                    upb_Message_GetFieldByDef(
                        upb_Message_GetFieldByDef(in_val.msg_val, field_def)
                            .msg_val,
                        state->wkts->list_value.values_def)
                        .array_val;
                out->data.def.field_def = state->wkts->list_value.values_def;
                out->kind = _cel_MessageEqualityFieldKind_kRepeated;
                break;
              default:
                CEL_UNREACHABLE();
            }
          }
        } break;
        case kUpb_WellKnown_ListValue:
          CEL_ASSERT_EQ(message_def, state->wkts->list_value.def);
          out->data.val.array_val =
              upb_Message_GetFieldByDef(in_val.msg_val,
                                        state->wkts->list_value.values_def)
                  .array_val;
          out->data.def.field_def = state->wkts->list_value.values_def;
          out->kind = _cel_MessageEqualityFieldKind_kRepeated;
          break;
        case kUpb_WellKnown_Struct:
          CEL_ASSERT_EQ(message_def, state->wkts->struct_value.def);
          out->data.val.map_val =
              upb_Message_GetFieldByDef(in_val.msg_val,
                                        state->wkts->struct_value.fields_def)
                  .map_val;
          out->data.def.field_def = state->wkts->struct_value.fields_def;
          out->kind = _cel_MessageEqualityFieldKind_kMap;
          break;
        default:
          out->data.def.message_def = message_def;
          out->data.val.msg_val = in_val.msg_val;
          out->kind = _cel_MessageEqualityFieldKind_kMessage;
          break;
      }
    } break;
    case kUpb_CType_Double:
      out->data.def.scalar_def = _cel_MessageEqualityScalarFieldKind_kNumber;
      out->data.val.num_val = _cel_DoubleNumber(in_val.double_val);
      out->kind = _cel_MessageEqualityFieldKind_kScalar;
      break;
    case kUpb_CType_Int64:
      out->data.def.scalar_def = _cel_MessageEqualityScalarFieldKind_kNumber;
      out->data.val.num_val = _cel_IntNumber(in_val.int64_val);
      out->kind = _cel_MessageEqualityFieldKind_kScalar;
      break;
    case kUpb_CType_UInt64:
      out->data.def.scalar_def = _cel_MessageEqualityScalarFieldKind_kNumber;
      out->data.val.num_val = _cel_UintNumber(in_val.uint64_val);
      out->kind = _cel_MessageEqualityFieldKind_kScalar;
      break;
    case kUpb_CType_String:
      out->data.def.scalar_def = _cel_MessageEqualityScalarFieldKind_kString;
      out->data.val.str_val = in_val.str_val;
      out->kind = _cel_MessageEqualityFieldKind_kScalar;
      break;
    case kUpb_CType_Bytes:
      out->data.def.scalar_def = _cel_MessageEqualityScalarFieldKind_kBytes;
      out->data.val.str_val = in_val.str_val;
      out->kind = _cel_MessageEqualityFieldKind_kScalar;
      break;
    default:
      CEL_UNREACHABLE();
  }
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_MessageEqualityState_FieldHeteroEquals(
    _cel_MessageEqualityState* cel_nonnull state, upb_MessageValue lhs_val,
    const upb_FieldDef* cel_nonnull lhs_def, upb_MessageValue rhs_val,
    const upb_FieldDef* cel_nonnull rhs_def, bool single);

typedef bool _cel_MessageEqualityState_MapKeyCoalescer(
    upb_MessageValue in, upb_MessageValue* cel_nonnull out);

// T -> T
static bool _cel_MessageEqualityState_CoalesceMapKeyIdentity(
    upb_MessageValue in, upb_MessageValue* cel_nonnull out) {
  *out = in;
  return true;
}

// int32 -> int64
static bool _cel_MessageEqualityState_CoalesceMapKeyInt32Int64(
    upb_MessageValue in, upb_MessageValue* cel_nonnull out) {
  out->int64_val = in.int32_val;
  return true;
}

// int32 -> uint32
static bool _cel_MessageEqualityState_CoalesceMapKeyInt32UInt32(
    upb_MessageValue in, upb_MessageValue* cel_nonnull out) {
  if (in.int32_val < 0) {
    return false;
  }
  out->uint32_val = (uint32_t)in.int32_val;
  return true;
}

// int32 -> uint64
static bool _cel_MessageEqualityState_CoalesceMapKeyInt32UInt64(
    upb_MessageValue in, upb_MessageValue* cel_nonnull out) {
  if (in.int32_val < 0) {
    return false;
  }
  out->uint64_val = (uint32_t)in.int32_val;
  return true;
}

// uint32 -> int32
static bool _cel_MessageEqualityState_CoalesceMapKeyUInt32Int32(
    upb_MessageValue in, upb_MessageValue* cel_nonnull out) {
  if (in.uint32_val > (uint32_t)INT32_MAX) {
    return false;
  }
  out->int32_val = (int32_t)in.uint32_val;
  return true;
}

// uint32 -> int64
static bool _cel_MessageEqualityState_CoalesceMapKeyUInt32Int64(
    upb_MessageValue in, upb_MessageValue* cel_nonnull out) {
  out->int64_val = (int64_t)(uint64_t)in.uint32_val;
  return true;
}

// uint32 -> uint64
static bool _cel_MessageEqualityState_CoalesceMapKeyUInt32UInt64(
    upb_MessageValue in, upb_MessageValue* cel_nonnull out) {
  out->uint64_val = in.uint32_val;
  return true;
}

// int64 -> int32
static bool _cel_MessageEqualityState_CoalesceMapKeyInt64Int32(
    upb_MessageValue in, upb_MessageValue* cel_nonnull out) {
  if (in.int64_val < INT32_MIN || in.int64_val > INT32_MAX) {
    return false;
  }
  out->int32_val = (int32_t)in.int64_val;
  return true;
}

// int64 -> uint32
static bool _cel_MessageEqualityState_CoalesceMapKeyInt64UInt32(
    upb_MessageValue in, upb_MessageValue* cel_nonnull out) {
  if (in.int64_val < 0 || in.int64_val > (int64_t)(uint64_t)UINT32_MAX) {
    return false;
  }
  out->uint32_val = (uint32_t)(int32_t)in.int64_val;
  return true;
}

// int64 -> uint64
static bool _cel_MessageEqualityState_CoalesceMapKeyInt64UInt64(
    upb_MessageValue in, upb_MessageValue* cel_nonnull out) {
  if (in.int64_val < 0) {
    return false;
  }
  out->uint64_val = (uint64_t)in.int64_val;
  return true;
}

// uint64 -> int32
static bool _cel_MessageEqualityState_CoalesceMapKeyUInt64Int32(
    upb_MessageValue in, upb_MessageValue* cel_nonnull out) {
  if (in.uint64_val > (uint64_t)(uint32_t)INT32_MAX) {
    return false;
  }
  out->int32_val = (uint64_t)(uint32_t)in.uint64_val;
  return true;
}

// uint64 -> int32
static bool _cel_MessageEqualityState_CoalesceMapKeyUInt64Int64(
    upb_MessageValue in, upb_MessageValue* cel_nonnull out) {
  if (in.uint64_val > (uint64_t)INT64_MAX) {
    return false;
  }
  out->int64_val = (int64_t)in.uint64_val;
  return true;
}

// uint64 -> uint32
static bool _cel_MessageEqualityState_CoalesceMapKeyUInt64UInt32(
    upb_MessageValue in, upb_MessageValue* cel_nonnull out) {
  if (in.uint64_val > UINT32_MAX) {
    return false;
  }
  out->uint32_val = (uint32_t)in.uint64_val;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_MessageEqualityState_MapFieldHeteroEquals(
    _cel_MessageEqualityState* cel_nonnull state,
    const upb_Map* cel_nullable lhs_val,
    const upb_FieldDef* cel_nonnull lhs_def,
    const upb_Map* cel_nullable rhs_val,
    const upb_FieldDef* cel_nonnull rhs_def) {
  CEL_ASSERT(upb_FieldDef_IsMap(lhs_def));
  CEL_ASSERT(upb_FieldDef_IsMap(rhs_def));

  const size_t lhs_val_size =
      lhs_val != cel_nullptr ? upb_Map_Size(lhs_val) : 0;
  const size_t rhs_val_size =
      rhs_val != cel_nullptr ? upb_Map_Size(rhs_val) : 0;

  if (lhs_val_size != rhs_val_size) {
    return false;
  }

  if (lhs_val_size == 0) {
    return true;
  }

  const upb_MessageDef* lhs_entry_def = upb_FieldDef_MessageSubDef(lhs_def);
  const upb_FieldDef* lhs_key_def = upb_MessageDef_FindFieldByNumber(
      lhs_entry_def, kUpb_MapEntry_KeyFieldNumber);
  const upb_CType lhs_key_def_type = upb_FieldDef_CType(lhs_key_def);

  const upb_MessageDef* rhs_entry_def = upb_FieldDef_MessageSubDef(rhs_def);
  const upb_FieldDef* rhs_key_def = upb_MessageDef_FindFieldByNumber(
      rhs_entry_def, kUpb_MapEntry_KeyFieldNumber);
  const upb_CType rhs_key_def_type = upb_FieldDef_CType(rhs_key_def);

  _cel_MessageEqualityState_MapKeyCoalescer* map_key_coalescer;

  switch (lhs_key_def_type) {
    case kUpb_CType_Bool:
      if (rhs_key_def_type != kUpb_CType_Bool) {
        return false;
      }
      map_key_coalescer = &_cel_MessageEqualityState_CoalesceMapKeyIdentity;
      break;
    case kUpb_CType_Enum: {
      if (strcmp(upb_EnumDef_FullName(upb_FieldDef_EnumSubDef(lhs_key_def)),
                 "google.protobuf.NullValue") == 0) {
        if (rhs_key_def_type != kUpb_CType_Enum ||
            strcmp(upb_EnumDef_FullName(upb_FieldDef_EnumSubDef(rhs_key_def)),
                   "google.protobuf.NullValue") != 0) {
          return false;
        }
        // Both null.
        map_key_coalescer = &_cel_MessageEqualityState_CoalesceMapKeyIdentity;
        break;
      }
      if (rhs_key_def_type == kUpb_CType_Enum &&
          strcmp(upb_EnumDef_FullName(upb_FieldDef_EnumSubDef(rhs_key_def)),
                 "google.protobuf.NullValue") == 0) {
        return false;
      }
      // Neither null.
      map_key_coalescer = &_cel_MessageEqualityState_CoalesceMapKeyIdentity;
      break;
    }
    case kUpb_CType_Int32:
      switch (rhs_key_def_type) {
        case kUpb_CType_Enum:
          if (strcmp(upb_EnumDef_FullName(upb_FieldDef_EnumSubDef(rhs_key_def)),
                     "google.protobuf.NullValue") == 0) {
            return false;
          }
          CEL_ATTRIBUTE_FALLTHROUGH;
        case kUpb_CType_Int32:
          map_key_coalescer = &_cel_MessageEqualityState_CoalesceMapKeyIdentity;
          break;
        case kUpb_CType_Int64:
          map_key_coalescer =
              &_cel_MessageEqualityState_CoalesceMapKeyInt32Int64;
          break;
        case kUpb_CType_UInt32:
          map_key_coalescer =
              &_cel_MessageEqualityState_CoalesceMapKeyInt32UInt32;
          break;
        case kUpb_CType_UInt64:
          map_key_coalescer =
              &_cel_MessageEqualityState_CoalesceMapKeyInt32UInt64;
          break;
        default:
          return false;
      }
      break;
    case kUpb_CType_Int64:
      switch (rhs_key_def_type) {
        case kUpb_CType_Enum:
          if (strcmp(upb_EnumDef_FullName(upb_FieldDef_EnumSubDef(rhs_key_def)),
                     "google.protobuf.NullValue") == 0) {
            return false;
          }
          CEL_ATTRIBUTE_FALLTHROUGH;
        case kUpb_CType_Int32:
          map_key_coalescer =
              &_cel_MessageEqualityState_CoalesceMapKeyInt64Int32;
          break;
        case kUpb_CType_Int64:
          map_key_coalescer = &_cel_MessageEqualityState_CoalesceMapKeyIdentity;
          break;
        case kUpb_CType_UInt32:
          map_key_coalescer =
              &_cel_MessageEqualityState_CoalesceMapKeyInt64UInt32;
          break;
        case kUpb_CType_UInt64:
          map_key_coalescer =
              &_cel_MessageEqualityState_CoalesceMapKeyInt64UInt64;
          break;
        default:
          return false;
      }
      break;
    case kUpb_CType_UInt32:
      switch (rhs_key_def_type) {
        case kUpb_CType_Enum:
          if (strcmp(upb_EnumDef_FullName(upb_FieldDef_EnumSubDef(rhs_key_def)),
                     "google.protobuf.NullValue") == 0) {
            return false;
          }
          CEL_ATTRIBUTE_FALLTHROUGH;
        case kUpb_CType_Int32:
          map_key_coalescer =
              &_cel_MessageEqualityState_CoalesceMapKeyUInt32Int32;
          break;
        case kUpb_CType_Int64:
          map_key_coalescer =
              &_cel_MessageEqualityState_CoalesceMapKeyUInt32Int64;
          break;
        case kUpb_CType_UInt32:
          map_key_coalescer = &_cel_MessageEqualityState_CoalesceMapKeyIdentity;
          break;
        case kUpb_CType_UInt64:
          map_key_coalescer =
              &_cel_MessageEqualityState_CoalesceMapKeyUInt32UInt64;
          break;
        default:
          return false;
      }
      break;
    case kUpb_CType_UInt64:
      switch (rhs_key_def_type) {
        case kUpb_CType_Enum:
          if (strcmp(upb_EnumDef_FullName(upb_FieldDef_EnumSubDef(rhs_key_def)),
                     "google.protobuf.NullValue") == 0) {
            return false;
          }
          CEL_ATTRIBUTE_FALLTHROUGH;
        case kUpb_CType_Int32:
          map_key_coalescer =
              &_cel_MessageEqualityState_CoalesceMapKeyUInt64Int32;
          break;
        case kUpb_CType_Int64:
          map_key_coalescer =
              &_cel_MessageEqualityState_CoalesceMapKeyUInt64Int64;
          break;
        case kUpb_CType_UInt32:
          map_key_coalescer =
              &_cel_MessageEqualityState_CoalesceMapKeyUInt64UInt32;
          break;
        case kUpb_CType_UInt64:
          map_key_coalescer = &_cel_MessageEqualityState_CoalesceMapKeyIdentity;
          break;
        default:
          return false;
      }
      break;
    case kUpb_CType_String:
      if (rhs_key_def_type != kUpb_CType_String) {
        return false;
      }
      map_key_coalescer = &_cel_MessageEqualityState_CoalesceMapKeyIdentity;
      break;
    default:
      CEL_UNREACHABLE();
  }

  const upb_FieldDef* lhs_value_def = upb_MessageDef_FindFieldByNumber(
      lhs_entry_def, kUpb_MapEntry_ValueFieldNumber);
  const upb_FieldDef* rhs_value_def = upb_MessageDef_FindFieldByNumber(
      lhs_entry_def, kUpb_MapEntry_ValueFieldNumber);

  size_t lhs_iter = kUpb_Map_Begin;
  upb_MessageValue lhs_key;
  upb_MessageValue lhs_value;
  while (upb_Map_Next(lhs_val, &lhs_key, &lhs_value, &lhs_iter)) {
    upb_MessageValue rhs_key;
    if (!(*map_key_coalescer)(lhs_key, &rhs_key)) {
      return false;
    }
    upb_MessageValue rhs_value;
    if (!upb_Map_Get(rhs_val, rhs_key, &rhs_value)) {
      return false;
    }
    if (!_cel_MessageEqualityState_FieldHeteroEquals(
            state, lhs_value, lhs_value_def, rhs_value, rhs_value_def,
            /*single=*/true)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_MessageEqualityState_RepeatedFieldHeteroEquals(
    _cel_MessageEqualityState* cel_nonnull state,
    const upb_Array* cel_nullable lhs_val,
    const upb_FieldDef* cel_nonnull lhs_def,
    const upb_Array* cel_nullable rhs_val,
    const upb_FieldDef* cel_nonnull rhs_def) {
  const size_t lhs_val_size =
      lhs_val != cel_nullptr ? upb_Array_Size(lhs_val) : 0;
  const size_t rhs_val_size =
      rhs_val != cel_nullptr ? upb_Array_Size(rhs_val) : 0;

  if (lhs_val_size != rhs_val_size) {
    return false;
  }

  if (lhs_val_size == 0) {
    return true;
  }

  for (size_t i = 0; i < lhs_val_size; ++i) {
    if (!_cel_MessageEqualityState_FieldHeteroEquals(
            state, upb_Array_Get(lhs_val, i), lhs_def,
            upb_Array_Get(rhs_val, i), rhs_def, /*single=*/true)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_MessageEqualityState_FieldHeteroEquals(
    _cel_MessageEqualityState* cel_nonnull state, upb_MessageValue lhs_val,
    const upb_FieldDef* cel_nonnull lhs_def, upb_MessageValue rhs_val,
    const upb_FieldDef* cel_nonnull rhs_def, bool single) {
  _cel_MessageEqualityField lhs_field;
  _cel_MessageEqualityField rhs_field;

  _cel_MessageEqualityState_Field(state, lhs_val, lhs_def, single, &lhs_field);
  _cel_MessageEqualityState_Field(state, rhs_val, rhs_def, single, &rhs_field);

  switch (lhs_field.kind) {
    case _cel_MessageEqualityFieldKind_kScalar:
      if (lhs_field.kind != rhs_field.kind) {
        return false;
      }
      if (lhs_field.data.def.scalar_def != rhs_field.data.def.scalar_def) {
        return false;
      }
      switch (lhs_field.data.def.scalar_def) {
        case _cel_MessageEqualityScalarFieldKind_kBool:
          return lhs_field.data.val.bool_val == rhs_field.data.val.bool_val;
        case _cel_MessageEqualityScalarFieldKind_kNumber:
          return _cel_Number_Equals(lhs_field.data.val.num_val,
                                    rhs_field.data.val.num_val);
        case _cel_MessageEqualityScalarFieldKind_kString:
          CEL_ATTRIBUTE_FALLTHROUGH;
        case _cel_MessageEqualityScalarFieldKind_kBytes:
          return cel_StringView_Equals(lhs_field.data.val.str_val,
                                       rhs_field.data.val.str_val);
        default:
          CEL_UNREACHABLE();
      }
    case _cel_MessageEqualityFieldKind_kMessage:
      if (lhs_field.kind != rhs_field.kind ||
          lhs_field.data.def.message_def != rhs_field.data.def.message_def) {
        return false;
      }
      return _cel_MessageEqualityState_HomoEquals(
          state, lhs_field.data.val.msg_val, rhs_field.data.val.msg_val,
          lhs_field.data.def.message_def);
    case _cel_MessageEqualityFieldKind_kNullValue:
      return lhs_field.kind == rhs_field.kind;
    case _cel_MessageEqualityFieldKind_kRepeated:
      if (lhs_field.kind != rhs_field.kind) {
        return false;
      }
      return _cel_MessageEqualityState_RepeatedFieldHeteroEquals(
          state, lhs_field.data.val.array_val, lhs_field.data.def.field_def,
          rhs_field.data.val.array_val, rhs_field.data.def.field_def);
    case _cel_MessageEqualityFieldKind_kMap:
      if (lhs_field.kind != rhs_field.kind) {
        return false;
      }
      return _cel_MessageEqualityState_MapFieldHeteroEquals(
          state, lhs_field.data.val.map_val, lhs_field.data.def.field_def,
          rhs_field.data.val.map_val, rhs_field.data.def.field_def);
    default:
      CEL_UNREACHABLE();
  }
}

extern "C" _cel_MessageEquality _cel_Message_Equals(
    const upb_Message* cel_nonnull lhs_val,
    const upb_Message* cel_nonnull rhs_val,
    const upb_MessageDef* cel_nonnull val_def,
    const upb_DefPool* cel_nonnull def_pool,
    const cel_WellKnownTypes* cel_nonnull wkts,
    cel_Allocator* cel_nonnull alloc) {
  CEL_ASSERT_NOT_NULL(lhs_val);
  CEL_ASSERT_NOT_NULL(rhs_val);
  CEL_ASSERT_NOT_NULL(val_def);
  CEL_ASSERT_NOT_NULL(def_pool);
  CEL_ASSERT_NOT_NULL(wkts);
  CEL_ASSERT_NOT_NULL(alloc);

  _cel_MessageEqualityState state = {
      .def_pool = def_pool,
      .wkts = wkts,
      .alloc = alloc,
      .arena = cel_nullptr,
      .depth = 100,
  };
  _cel_MessageEqualityState* volatile state_ptr = &state;
  if (_cel_setjmp(state_ptr->jmp)) {
    CEL_ASSERT_NE(state_ptr->result, _cel_MessageEquality_kEqual);
    CEL_ASSERT_NE(state_ptr->result, _cel_MessageEquality_kNotEqual);
  } else {
    state_ptr->result = _cel_MessageEqualityState_HomoEquals(state_ptr, lhs_val,
                                                             rhs_val, val_def)
                            ? _cel_MessageEquality_kEqual
                            : _cel_MessageEquality_kNotEqual;
  }
  cel_Arena_Delete(state_ptr->arena);
  return state_ptr->result;
}

extern "C" _cel_MessageEquality _cel_MessageField_Equals(
    upb_MessageValue lhs_val, const upb_FieldDef* cel_nonnull lhs_def,
    upb_MessageValue rhs_val, const upb_FieldDef* cel_nonnull rhs_def,
    const upb_DefPool* cel_nonnull def_pool,
    const cel_WellKnownTypes* cel_nonnull wkts,
    cel_Allocator* cel_nonnull alloc) {
  CEL_ASSERT_NOT_NULL(lhs_def);
  CEL_ASSERT_NOT_NULL(rhs_def);
  CEL_ASSERT_NOT_NULL(def_pool);
  CEL_ASSERT_NOT_NULL(wkts);
  CEL_ASSERT_NOT_NULL(alloc);

  _cel_MessageEqualityState state = {
      .def_pool = def_pool,
      .wkts = wkts,
      .alloc = alloc,
      .arena = cel_nullptr,
      .depth = 100,
  };
  _cel_MessageEqualityState* volatile state_ptr = &state;
  if (_cel_setjmp(state_ptr->jmp)) {
    CEL_ASSERT_NE(state_ptr->result, _cel_MessageEquality_kEqual);
    CEL_ASSERT_NE(state_ptr->result, _cel_MessageEquality_kNotEqual);
  } else {
    state_ptr->result =
        _cel_MessageEqualityState_FieldHeteroEquals(
            state_ptr, lhs_val, lhs_def, rhs_val, rhs_def, /*single=*/false)
            ? _cel_MessageEquality_kEqual
            : _cel_MessageEquality_kNotEqual;
  }
  cel_Arena_Delete(state_ptr->arena);
  return state_ptr->result;
}
