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

#include "cel-c/src/parsed_message_value.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/status.h"
#include "cel-c/status_code.h"
#include "cel-c/string_view.h"
#include "cel-c/value.h"
#include "cel-c/src/message_equality.h"
#include "upb/base/descriptor_constants.h"
#include "upb/message/array.h"
#include "upb/message/map.h"
#include "upb/message/message.h"
#include "upb/reflection/def.h"
#include "upb/reflection/message.h"

static const cel_StructValueIteratorVTable
    _cel_ParsedMessageValueIteratorVTable;

typedef struct {
  cel_StructValueIterator base;
  cel_Allocator* cel_nonnull alloc;
  const upb_Message* cel_nonnull message;
  const upb_MessageDef* cel_nonnull message_def;
  size_t iter;
} _cel_ParsedMessageValueIterator;

static bool _cel_ParsedMessageValue_Equals(
    const cel_StructValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_StructValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedMessageValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(other);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  if (other->vtable == &_cel_ParsedMessageValueVTable) {
    const upb_Message* message = (const upb_Message*)content.ptr[0];
    const upb_Message* other_message =
        (const upb_Message*)other->content.ptr[0];
    const upb_MessageDef* message_def = (const upb_MessageDef*)content.ptr[1];
    const upb_MessageDef* other_message_def =
        (const upb_MessageDef*)other->content.ptr[1];
    CEL_ASSERT_EQ(message_def, other_message_def);
    if (CEL_LIKELY(message_def == other_message_def)) {
      switch (_cel_Message_Equals(message, other_message, other_message_def,
                                  context->def_pool, context->well_known_types,
                                  context->alloc)) {
        case _cel_MessageEquality_kEqual:
          cel_Value_SetTrue(result);
          return true;
        case _cel_MessageEquality_kNotEqual:
          cel_Value_SetFalse(result);
          return true;
        case _cel_MessageEquality_kOutOfMemory:
          cel_OutOfMemoryStatus(status);
          return false;
        case _cel_MessageEquality_kMaxDepthExceeded:
          cel_Status_SetCanonicalCode(status, cel_StatusCode_kInvalidArgument);
          cel_Status_SetMessage(
              status, cel_StringView_From("max message depth exceeded"));
          return false;
      }
    }
  }

  // Fallback to default implementation.
  return false;
}

static cel_StringView _cel_ParsedMessageValue_TypeName(
    const cel_StructValueVTable* cel_nonnull vtable, cel_ValueContent content) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedMessageValueVTable);

  return cel_StringView_FromString(
      upb_MessageDef_FullName((const upb_MessageDef*)content.ptr[1]));
}

static bool _cel_ParsedMessageValue_Get(
    const cel_StructValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_StructValueKey* cel_nonnull key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedMessageValueVTable);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_Message* message = (const upb_Message*)content.ptr[0];
  const upb_MessageDef* message_def = (const upb_MessageDef*)content.ptr[1];
  const upb_FieldDef* field_def;
  const cel_StructValueKeyKind key_kind = cel_StructValueKey_Kind(key);
  switch (key_kind) {
    case cel_StructValueKeyKind_kName: {
      cel_StringView name = cel_StructValueKey_GetName(key);
      field_def = upb_MessageDef_FindFieldByNameWithSize(
          message_def, cel_StringView_Data(name), cel_StringView_Size(name));
      if (CEL_UNLIKELY(field_def == cel_nullptr)) {
        cel_Error* error = cel_Error_New(context->arena);
        if (CEL_UNLIKELY(error == cel_nullptr)) {
          cel_OutOfMemoryStatus(status);
          return false;
        }
        cel_Error_SetCanonicalCode(error, cel_ErrorCode_kNotFound);
        cel_Error_SetMessage(error, cel_StringView_From("no such field"));
        cel_Value_SetError(value, error);
        return true;
      }
    } break;
    case cel_StructValueKeyKind_kDef:
      field_def = cel_StructValueKey_GetDef(key);
      break;
    default:
      cel_InvalidArgumentStatusF(
          status, "cel: unexpected struct value key kind: %d", key_kind);
      return false;
  }

  CEL_ASSERT_NOT_NULL(field_def);

  return cel_Value_FromField(value, context,
                             upb_Message_GetFieldByDef(message, field_def),
                             field_def, status);
}

static bool _cel_ParsedMessageValue_Has(
    const cel_StructValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_StructValueKey* cel_nonnull key, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedMessageValueVTable);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_Message* message = (const upb_Message*)content.ptr[0];
  const upb_MessageDef* message_def = (const upb_MessageDef*)content.ptr[1];
  const upb_FieldDef* field_def;
  const cel_StructValueKeyKind key_kind = cel_StructValueKey_Kind(key);
  switch (key_kind) {
    case cel_StructValueKeyKind_kName: {
      cel_StringView name = cel_StructValueKey_GetName(key);
      field_def = upb_MessageDef_FindFieldByNameWithSize(
          message_def, cel_StringView_Data(name), cel_StringView_Size(name));
      if (CEL_UNLIKELY(field_def == cel_nullptr)) {
        cel_Error* error = cel_Error_New(context->arena);
        if (CEL_UNLIKELY(error == cel_nullptr)) {
          cel_OutOfMemoryStatus(status);
          return false;
        }
        cel_Error_SetCanonicalCode(error, cel_ErrorCode_kNotFound);
        cel_Error_SetMessage(error, cel_StringView_From("no such field"));
        cel_Value_SetError(result, error);
        return true;
      }
    } break;
    case cel_StructValueKeyKind_kDef:
      field_def = cel_StructValueKey_GetDef(key);
      break;
    default:
      cel_InvalidArgumentStatusF(
          status, "cel: unexpected struct value key kind: %d", key_kind);
      return false;
  }

  CEL_ASSERT_NOT_NULL(field_def);

  if (upb_FieldDef_IsMap(field_def)) {
    upb_MessageValue value = upb_Message_GetFieldByDef(message, field_def);
    cel_Value_SetBool(result, value.map_val != cel_nullptr &&
                                  upb_Map_Size(value.map_val) > 0);
  } else if (upb_FieldDef_IsRepeated(field_def)) {
    upb_MessageValue value = upb_Message_GetFieldByDef(message, field_def);
    cel_Value_SetBool(result, value.array_val != cel_nullptr &&
                                  upb_Array_Size(value.array_val) > 0);
  } else if (upb_FieldDef_HasPresence(field_def)) {
    cel_Value_SetBool(result, upb_Message_HasFieldByDef(message, field_def));
  } else {
    upb_MessageValue value = upb_Message_GetFieldByDef(message, field_def);
    switch (upb_FieldDef_CType(field_def)) {
      case kUpb_CType_Bool:
        cel_Value_SetBool(result, value.bool_val);
        break;
      case kUpb_CType_Enum:
        CEL_ATTRIBUTE_FALLTHROUGH;
      case kUpb_CType_Int32:
        cel_Value_SetBool(result, value.int32_val != 0);
        break;
      case kUpb_CType_Float:
        CEL_ATTRIBUTE_FALLTHROUGH;
        CEL_STATIC_ASSERT(sizeof(float) == sizeof(uint32_t));
      case kUpb_CType_UInt32:
        cel_Value_SetBool(result, value.uint32_val != 0);
        break;
      case kUpb_CType_Int64:
        cel_Value_SetBool(result, value.int64_val != 0);
        break;
      case kUpb_CType_Double:
        CEL_ATTRIBUTE_FALLTHROUGH;
        CEL_STATIC_ASSERT(sizeof(double) == sizeof(uint64_t));
      case kUpb_CType_UInt64:
        cel_Value_SetBool(result, value.uint64_val != 0);
        break;
      case kUpb_CType_Bytes:
        CEL_ATTRIBUTE_FALLTHROUGH;
      case kUpb_CType_String:
        cel_Value_SetBool(result, value.str_val.size != 0);
        break;
      case kUpb_CType_Message:
        cel_Value_SetBool(result, value.msg_val != cel_nullptr);
        break;
    }
  }

  return true;
}

static cel_StructValueIterator* cel_nullable
_cel_ParsedMessageValue_NewIterator(
    const cel_StructValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedMessageValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return cel_nullptr;
  }

  _cel_ParsedMessageValueIterator* iter =
      (_cel_ParsedMessageValueIterator*)cel_Allocator_Malloc(
          context->alloc, sizeof(_cel_ParsedMessageValueIterator), cel_nullptr);
  if (CEL_UNLIKELY(iter == cel_nullptr)) {
    cel_OutOfMemoryStatus(status);
    return cel_nullptr;
  }
  memset(iter, 0, sizeof(*iter));
  iter->base.vtable = &_cel_ParsedMessageValueIteratorVTable;
  iter->alloc = context->alloc;
  iter->message = (const upb_Message*)content.ptr[0];
  iter->message_def = (const upb_MessageDef*)content.ptr[1];
  iter->iter = kUpb_Message_Begin;
  return &iter->base;
}

const cel_StructValueVTable _cel_ParsedMessageValueVTable = {
    .Equals = &_cel_ParsedMessageValue_Equals,
    .TypeName = &_cel_ParsedMessageValue_TypeName,
    .Get = &_cel_ParsedMessageValue_Get,
    .Has = &_cel_ParsedMessageValue_Has,
    .NewIterator = &_cel_ParsedMessageValue_NewIterator,
};

static void _cel_ParsedMessageValueIterator_Delete(
    cel_StructValueIterator* cel_nonnull iterator) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_NOT_NULL(iterator->vtable);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_ParsedMessageValueIteratorVTable);

  _cel_ParsedMessageValueIterator* iter =
      cel_containerof(iterator, _cel_ParsedMessageValueIterator, base);
  cel_Allocator_FreeSized(iter->alloc, iter, sizeof(*iter));
}

static bool _cel_ParsedMessageValueIterator_Next(
    cel_StructValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_StructValueKey* cel_nonnull key, cel_Value* cel_nullable value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_NOT_NULL(iterator->vtable);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_ParsedMessageValueIteratorVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  _cel_ParsedMessageValueIterator* iter =
      cel_containerof(iterator, _cel_ParsedMessageValueIterator, base);
  const upb_FieldDef* field_def;
  upb_MessageValue field_val;
  size_t iter_next = iter->iter;
  if (upb_Message_Next(iter->message, iter->message_def, context->def_pool,
                       &field_def, &field_val, &iter_next)) {
    cel_StructValueKey_SetDef(key, field_def);
    if (value != cel_nullptr) {
      if (!cel_Value_FromField(value, context, field_val, field_def, status)) {
        return false;
      }
    }
    iter->iter = iter_next;
    return true;
  }
  return false;
}

static bool _cel_ParsedMessageValueIterator_Remaining(
    const cel_StructValueIterator* cel_nonnull iterator,
    size_t* cel_nonnull remaining) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_NOT_NULL(iterator->vtable);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_ParsedMessageValueIteratorVTable);
  CEL_ASSERT_NOT_NULL(remaining);

  // We do not know this in constant time, it requires iterating over all set
  // fields to count.
  return false;
}

static const cel_StructValueIteratorVTable
    _cel_ParsedMessageValueIteratorVTable = {
        .Delete = &_cel_ParsedMessageValueIterator_Delete,
        .Next = &_cel_ParsedMessageValueIterator_Next,
        .Remaining = &_cel_ParsedMessageValueIterator_Remaining,
};
