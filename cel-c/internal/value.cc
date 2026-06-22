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

#include "cel-c/value.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/internal/any.h"
#include "cel-c/internal/parsed_map_field_value.h"
#include "cel-c/internal/parsed_message_value.h"
#include "cel-c/internal/parsed_repeated_field_value.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"
#include "cel-c/type.h"
#include "cel-c/value_kind.h"
#include "upb/base/descriptor_constants.h"
#include "upb/message/array.h"
#include "upb/message/message.h"
#include "upb/reflection/def.h"
#include "upb/reflection/message.h"

#define _cel_kDoubleToIntMax ((double)(int64_t)INT64_MAX)
#define _cel_kDoubleToIntMin ((double)(int64_t)INT64_MIN)
#define _cel_kDoubleToUintMin ((double)(uint64_t)0)
#define _cel_kDoubleToUintMax ((double)(uint64_t)UINT64_MAX)

extern "C" const cel_Value cel_NullValue = {
    .data =
        {
            .v = {(char)0},
        },
    .kind = cel_ValueKind_kNull,
    .padding = {(char)0},
};

extern "C" const cel_Value cel_FalseValue = {
    .data =
        {
            .bl = false,
        },
    .kind = cel_ValueKind_kBool,
    .padding = {(char)0},
};

extern "C" const cel_Value cel_TrueValue = {
    .data =
        {
            .bl = true,
        },
    .kind = cel_ValueKind_kBool,
    .padding = {(char)0},
};

extern "C" cel_StringView cel_Value_TypeName(
    const cel_Value* cel_nonnull value) {
  CEL_ASSERT_NOT(cel_Value_IsError(value));
  CEL_ASSERT_NOT(cel_Value_IsUnknown(value));

  switch (cel_Value_Kind(value)) {
    case cel_ValueKind_kNull:
      return cel_StringView_From("null_type");
    case cel_ValueKind_kBool:
      return cel_StringView_From("bool");
    case cel_ValueKind_kInt:
      return cel_StringView_From("int");
    case cel_ValueKind_kUint:
      return cel_StringView_From("uint");
    case cel_ValueKind_kDouble:
      return cel_StringView_From("double");
    case cel_ValueKind_kString:
      return cel_StringView_From("string");
    case cel_ValueKind_kBytes:
      return cel_StringView_From("bytes");
    case cel_ValueKind_kDuration:
      return cel_StringView_From("google.protobuf.Duration");
    case cel_ValueKind_kTimestamp:
      return cel_StringView_From("google.protobuf.Timestamp");
    case cel_ValueKind_kList:
      return cel_StringView_From("list");
    case cel_ValueKind_kMap:
      return cel_StringView_From("map");
    case cel_ValueKind_kStruct:
      return cel_StructValue_TypeName(cel_Value_GetStruct(value));
    case cel_ValueKind_kOpaque:
      return cel_OpaqueValue_TypeName(cel_Value_GetOpaque(value));
    case cel_ValueKind_kType:
      return cel_StringView_From("type");
    default:
      return cel_StringView_From("");
  }
}

extern "C" bool cel_Value_Equals(const cel_Value* cel_nonnull value,
                                 const cel_ValueContext* cel_nonnull context,
                                 const cel_Value* cel_nonnull other,
                                 cel_Value* cel_nonnull result,
                                 cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(other);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const cel_ValueKind value_kind = cel_Value_Kind(value);
  const cel_ValueKind other_kind = cel_Value_Kind(other);

  if (value_kind == cel_ValueKind_kError) {
    *result = *value;
    return true;
  }

  if (other_kind == cel_ValueKind_kError) {
    *result = *other;
    return true;
  }

  if (value == other) {
    cel_Value_SetTrue(result);
    return true;
  }

  if (value_kind != other_kind) {
    {
      const bool null_value = value_kind == cel_ValueKind_kNull;
      const bool other_null_value = other_kind == cel_ValueKind_kNull;
      if (null_value || other_null_value) {
        cel_Value_SetBool(result, null_value == other_null_value);
        return true;
      }
    }
    // Heterogeneous
    switch (value_kind) {
      case cel_ValueKind_kInt:
        switch (other_kind) {
          case cel_ValueKind_kUint: {
            const int64_t int_value = cel_Value_GetInt(value);
            cel_Value_SetBool(
                result, int_value >= 0 &&
                            int_value == (int64_t)cel_Value_GetUint(other));
          } break;
          case cel_ValueKind_kDouble: {
            const double double_value = cel_Value_GetDouble(other);
            cel_Value_SetBool(
                result, double_value >= _cel_kDoubleToIntMin &&
                            double_value <= _cel_kDoubleToIntMax &&
                            double_value == (double)cel_Value_GetInt(value));
          } break;
          default:
            cel_Value_SetFalse(result);
            break;
        }
        break;
      case cel_ValueKind_kUint:
        switch (other_kind) {
          case cel_ValueKind_kInt: {
            const int64_t int_value = cel_Value_GetInt(other);
            cel_Value_SetBool(
                result, int_value >= 0 &&
                            int_value == (int64_t)cel_Value_GetUint(value));
          } break;
          case cel_ValueKind_kDouble: {
            const double double_value = cel_Value_GetDouble(other);
            cel_Value_SetBool(
                result, double_value >= _cel_kDoubleToUintMin &&
                            double_value <= _cel_kDoubleToUintMax &&
                            double_value == (double)cel_Value_GetUint(value));
          } break;
          default:
            cel_Value_SetFalse(result);
            break;
        }
        break;
      case cel_ValueKind_kDouble:
        switch (other_kind) {
          case cel_ValueKind_kInt: {
            const double double_value = cel_Value_GetDouble(value);
            cel_Value_SetBool(
                result, double_value >= _cel_kDoubleToIntMin &&
                            double_value <= _cel_kDoubleToIntMax &&
                            double_value == (double)cel_Value_GetInt(other));
          } break;
          case cel_ValueKind_kUint: {
            const double double_value = cel_Value_GetDouble(value);
            cel_Value_SetBool(
                result, double_value >= _cel_kDoubleToUintMin &&
                            double_value <= _cel_kDoubleToUintMax &&
                            double_value == (double)cel_Value_GetUint(other));
          } break;
          default:
            cel_Value_SetFalse(result);
            break;
        }
        break;
      default:
        cel_Value_SetFalse(result);
        break;
    }
    return true;
  }

  // Homogeneous
  switch (value_kind) {
    case cel_ValueKind_kNull:
      cel_Value_SetTrue(result);
      break;
    case cel_ValueKind_kBool:
      cel_Value_SetBool(result,
                        cel_Value_GetBool(value) == cel_Value_GetBool(other));
      break;
    case cel_ValueKind_kInt:
      cel_Value_SetBool(result,
                        cel_Value_GetInt(value) == cel_Value_GetInt(other));
      break;
    case cel_ValueKind_kUint:
      cel_Value_SetBool(result,
                        cel_Value_GetUint(value) == cel_Value_GetUint(other));
      break;
    case cel_ValueKind_kDouble:
      cel_Value_SetBool(
          result, cel_Value_GetDouble(value) == cel_Value_GetDouble(other));
      break;
    case cel_ValueKind_kString:
      cel_Value_SetBool(result,
                        cel_StringView_Equals(cel_Value_GetString(value),
                                              cel_Value_GetString(other)));
      break;
    case cel_ValueKind_kBytes:
      cel_Value_SetBool(result,
                        cel_StringView_Equals(cel_Value_GetBytes(value),
                                              cel_Value_GetBytes(other)));
      break;
    case cel_ValueKind_kDuration:
      cel_Value_SetBool(result,
                        cel_Duration_Equals(cel_Value_GetDuration(value),
                                            cel_Value_GetDuration(other)));
      break;
    case cel_ValueKind_kTimestamp:
      cel_Value_SetBool(result,
                        cel_Timestamp_Equals(cel_Value_GetTimestamp(value),
                                             cel_Value_GetTimestamp(other)));
      break;
    case cel_ValueKind_kList:
      return cel_ListValue_Equals(cel_Value_GetList(value), context,
                                  cel_Value_GetList(other), result, status);
    case cel_ValueKind_kMap:
      return cel_MapValue_Equals(cel_Value_GetMap(value), context,
                                 cel_Value_GetMap(other), result, status);
    case cel_ValueKind_kStruct:
      return cel_StructValue_Equals(cel_Value_GetStruct(value), context,
                                    cel_Value_GetStruct(other), result, status);
    case cel_ValueKind_kOpaque:
      return cel_OpaqueValue_Equals(cel_Value_GetOpaque(value), context,
                                    cel_Value_GetOpaque(other), result, status);
    case cel_ValueKind_kType:
      cel_Value_SetBool(result,
                        cel_StringView_Equals(cel_Value_GetType(value),
                                              cel_Value_GetType(other)));
      break;
    case cel_ValueKind_kUnknown:
      CEL_ATTRIBUTE_FALLTHROUGH;
    default:
      cel_Value_SetFalse(result);
      break;
  }
  return true;
}

extern "C" bool cel_Value_FromMessage(
    cel_Value* cel_nonnull value, const cel_ValueContext* cel_nonnull context,
    const upb_Message* message_val,
    const upb_MessageDef* cel_nonnull message_def,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(message_val);
  CEL_ASSERT_NOT_NULL(message_def);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  upb_WellKnown well_known_type = upb_MessageDef_WellKnownType(message_def);
  if (well_known_type == kUpb_WellKnown_Any) {
    CEL_ASSERT_EQ(message_def, context->well_known_types->any.def);
    upb_Message* out_message_val;
    const upb_MessageDef* out_message_def;
    _cel_AnyUnpackResult result = _cel_AnyUnpack(
        message_val, context->def_pool, &context->well_known_types->any,
        context->arena, &out_message_val, &out_message_def);
    if (result != _cel_AnyUnpackResult_kOk) {
      if (result == _cel_AnyUnpackResult_kOutOfMemory) {
        cel_OutOfMemoryStatus(status);
        return false;
      }
      cel_Error* error = cel_Error_New(context->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        cel_OutOfMemoryStatus(status);
        return false;
      }
      cel_Error_SetCanonicalCode(
          error, (cel_ErrorCode)_cel_AnyUnpackResult_ToStatusCode(result));
      cel_Error_SetMessage(error, cel_StringView_FromString(
                                      _cel_AnyUnpackResult_ToMessage(result)));
      cel_Value_SetError(value, error);
      return true;
    }
    message_val = out_message_val;
    message_def = out_message_def;
    well_known_type = upb_MessageDef_WellKnownType(out_message_def);
  }

  switch (well_known_type) {
    case kUpb_WellKnown_Duration: {
      CEL_ASSERT_EQ(message_def, context->well_known_types->duration.def);
      int64_t seconds =
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->duration.seconds_def)
              .int64_val;
      int32_t nanos =
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->duration.nanos_def)
              .int32_val;
      if (cel_Duration_Normalize(&seconds, &nanos)) {
        cel_Value_SetDuration(value, cel_Duration_FromUnix(seconds, nanos));
      } else {
        cel_Error* error = cel_Error_New(context->arena);
        if (CEL_UNLIKELY(error == cel_nullptr)) {
          cel_OutOfMemoryStatus(status);
          return false;
        }
        cel_Error_SetCanonicalCode(error, cel_ErrorCode_kOutOfRange);
        cel_Error_SetMessage(error,
                             cel_StringView_From("duration out of range"));
        cel_Value_SetError(value, error);
      }
      return true;
    }
    case kUpb_WellKnown_Timestamp: {
      CEL_ASSERT_EQ(message_def, context->well_known_types->timestamp.def);
      int64_t seconds =
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->timestamp.seconds_def)
              .int64_val;
      int32_t nanos =
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->timestamp.nanos_def)
              .int32_val;
      if (cel_Timestamp_Normalize(&seconds, &nanos)) {
        cel_Value_SetTimestamp(value, cel_Timestamp_FromUnix(seconds, nanos));
      } else {
        cel_Error* error = cel_Error_New(context->arena);
        if (CEL_UNLIKELY(error == cel_nullptr)) {
          cel_OutOfMemoryStatus(status);
          return false;
        }
        cel_Error_SetCanonicalCode(error, cel_ErrorCode_kOutOfRange);
        cel_Error_SetMessage(error,
                             cel_StringView_From("timestamp out of range"));
        cel_Value_SetError(value, error);
      }
      return true;
    }
    case kUpb_WellKnown_DoubleValue:
      CEL_ASSERT_EQ(message_def, context->well_known_types->double_value.def);
      cel_Value_SetDouble(
          value,
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->double_value.value_def)
              .double_val);
      return true;
    case kUpb_WellKnown_FloatValue:
      CEL_ASSERT_EQ(message_def, context->well_known_types->float_value.def);
      cel_Value_SetDouble(
          value,
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->float_value.value_def)
              .float_val);
      return true;
    case kUpb_WellKnown_Int64Value:
      CEL_ASSERT_EQ(message_def, context->well_known_types->int64_value.def);
      cel_Value_SetInt(
          value,
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->int64_value.value_def)
              .int64_val);
      return true;
    case kUpb_WellKnown_UInt64Value:
      CEL_ASSERT_EQ(message_def, context->well_known_types->uint64_value.def);
      cel_Value_SetUint(
          value,
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->uint64_value.value_def)
              .uint64_val);
      return true;
    case kUpb_WellKnown_Int32Value:
      CEL_ASSERT_EQ(message_def, context->well_known_types->int32_value.def);
      cel_Value_SetInt(
          value,
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->int32_value.value_def)
              .int32_val);
      return true;
    case kUpb_WellKnown_UInt32Value:
      CEL_ASSERT_EQ(message_def, context->well_known_types->uint32_value.def);
      cel_Value_SetUint(
          value,
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->uint32_value.value_def)
              .uint32_val);
      return true;
    case kUpb_WellKnown_StringValue:
      CEL_ASSERT_EQ(message_def, context->well_known_types->string_value.def);
      cel_Value_SetString(
          value,
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->string_value.value_def)
              .str_val);
      return true;
    case kUpb_WellKnown_BytesValue:
      CEL_ASSERT_EQ(message_def, context->well_known_types->bytes_value.def);
      cel_Value_SetBytes(
          value,
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->bytes_value.value_def)
              .str_val);
      return true;
    case kUpb_WellKnown_BoolValue:
      CEL_ASSERT_EQ(message_def, context->well_known_types->bool_value.def);
      cel_Value_SetBool(
          value,
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->bool_value.value_def)
              .bool_val);
      return true;
    case kUpb_WellKnown_Value: {
      CEL_ASSERT_EQ(message_def, context->well_known_types->value.def);
      const upb_FieldDef* field_def = upb_Message_WhichOneofByDef(
          message_val, context->well_known_types->value.kind_def);
      if (field_def == cel_nullptr) {
        cel_Value_SetNull(value);
        return true;
      }
      switch (upb_FieldDef_Number(field_def)) {
        case 1:  // null_value
          CEL_ASSERT_EQ(field_def,
                        context->well_known_types->value.null_value_def);
          cel_Value_SetNull(value);
          return true;
        case 2:  // number_value
          CEL_ASSERT_EQ(field_def,
                        context->well_known_types->value.number_value_def);
          cel_Value_SetDouble(
              value,
              upb_Message_GetFieldByDef(message_val, field_def).double_val);
          return true;
        case 3:  // string_value
          CEL_ASSERT_EQ(field_def,
                        context->well_known_types->value.string_value_def);
          cel_Value_SetString(
              value, upb_Message_GetFieldByDef(message_val, field_def).str_val);
          return true;
        case 4:  // bool_value
          CEL_ASSERT_EQ(field_def,
                        context->well_known_types->value.bool_value_def);
          cel_Value_SetBool(
              value,
              upb_Message_GetFieldByDef(message_val, field_def).bool_val);
          return true;
        case 5:  // struct_value
          CEL_ASSERT_EQ(field_def,
                        context->well_known_types->value.struct_value_def);
          _cel_ParsedMapFieldValue_Set(
              cel_Value_SetMap(value),
              upb_Message_GetFieldByDef(
                  upb_Message_GetFieldByDef(message_val, field_def).msg_val,
                  context->well_known_types->struct_value.fields_def)
                  .map_val,
              context->well_known_types->struct_value.fields_def);
          return true;
        case 6:  // list_value
          CEL_ASSERT_EQ(field_def,
                        context->well_known_types->value.list_value_def);
          _cel_ParsedRepeatedFieldValue_Set(
              cel_Value_SetList(value),
              upb_Message_GetFieldByDef(
                  upb_Message_GetFieldByDef(message_val, field_def).msg_val,
                  context->well_known_types->list_value.values_def)
                  .array_val,
              context->well_known_types->list_value.values_def);
          return true;
        default:
          CEL_UNREACHABLE();
      }
    }
    case kUpb_WellKnown_ListValue:
      CEL_ASSERT_EQ(message_def, context->well_known_types->list_value.def);
      _cel_ParsedRepeatedFieldValue_Set(
          cel_Value_SetList(value),
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->list_value.values_def)
              .array_val,
          context->well_known_types->list_value.values_def);
      return true;
    case kUpb_WellKnown_Struct:
      CEL_ASSERT_EQ(message_def, context->well_known_types->struct_value.def);
      _cel_ParsedMapFieldValue_Set(
          cel_Value_SetMap(value),
          upb_Message_GetFieldByDef(
              message_val, context->well_known_types->struct_value.fields_def)
              .map_val,
          context->well_known_types->struct_value.fields_def);
      return true;
    default:
      _cel_ParsedMessageValue_Set(cel_Value_SetStruct(value), message_val,
                                  message_def);
      return true;
  }
}

extern "C" bool cel_Value_FromEnum(cel_Value* cel_nonnull value,
                                   const cel_ValueContext* cel_nonnull context,
                                   int32_t enum_val,
                                   const upb_EnumDef* cel_nonnull enum_def,
                                   cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(enum_def);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  if (context->well_known_types->null_value.def == enum_def) {
    cel_Value_SetNull(value);
    return true;
  }
  CEL_ASSERT_NOT(cel_IsWellKnownEnumType(enum_def));

  if (upb_EnumDef_IsClosed(enum_def)) {
    const upb_EnumValueDef* enum_val_def =
        upb_EnumDef_FindValueByNumber(enum_def, enum_val);
    if (CEL_UNLIKELY(enum_val_def == cel_nullptr)) {
      cel_Error* error = cel_Error_New(context->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        cel_OutOfMemoryStatus(status);
        return false;
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
      cel_Error_SetMessage(
          error, cel_StringView_From("cel: number not present in closed enum"));
      cel_Value_SetError(value, error);
      return true;
    }
  }
  cel_Value_SetInt(value, enum_val);
  return true;
}

extern "C" bool cel_Value_FromEnumValue(
    cel_Value* cel_nonnull value, const cel_ValueContext* cel_nonnull context,
    const upb_EnumValueDef* enum_val, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(enum_val);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  if (context->well_known_types->null_value.value_def == enum_val) {
    cel_Value_SetNull(value);
  } else {
    CEL_ASSERT_NOT(cel_IsWellKnownEnumType(upb_EnumValueDef_Enum(enum_val)));
    cel_Value_SetInt(value, upb_EnumValueDef_Number(enum_val));
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_Value_FromSingularField(
    cel_Value* cel_nonnull value, const cel_ValueContext* cel_nonnull context,
    upb_MessageValue field_val, const upb_FieldDef* cel_nonnull field_def,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT(upb_FieldDef_IsMap(field_def));

  switch (upb_FieldDef_CType(field_def)) {
    case kUpb_CType_Bool:
      cel_Value_SetBool(value, field_val.bool_val);
      return true;
    case kUpb_CType_Float:
      cel_Value_SetDouble(value, field_val.float_val);
      return true;
    case kUpb_CType_Int32:
      cel_Value_SetInt(value, field_val.int32_val);
      return true;
    case kUpb_CType_UInt32:
      cel_Value_SetUint(value, field_val.uint32_val);
      return true;
    case kUpb_CType_Enum:
      return cel_Value_FromEnum(value, context, field_val.int32_val,
                                upb_FieldDef_EnumSubDef(field_def), status);
    case kUpb_CType_Message:
      return cel_Value_FromMessage(value, context, field_val.msg_val,
                                   upb_FieldDef_MessageSubDef(field_def),
                                   status);
    case kUpb_CType_Double:
      cel_Value_SetDouble(value, field_val.double_val);
      return true;
    case kUpb_CType_Int64:
      cel_Value_SetInt(value, field_val.int64_val);
      return true;
    case kUpb_CType_UInt64:
      cel_Value_SetUint(value, field_val.uint64_val);
      return true;
    case kUpb_CType_String:
      cel_Value_SetString(value, field_val.str_val);
      return true;
    case kUpb_CType_Bytes:
      cel_Value_SetBytes(value, field_val.str_val);
      return true;
  }
}

extern "C" bool cel_Value_FromField(cel_Value* cel_nonnull value,
                                    const cel_ValueContext* cel_nonnull context,
                                    upb_MessageValue field_val,
                                    const upb_FieldDef* cel_nonnull field_def,
                                    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(field_def);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  if (upb_FieldDef_IsMap(field_def)) {
    _cel_ParsedMapFieldValue_Set(cel_Value_SetMap(value), field_val.map_val,
                                 field_def);
    return true;
  }

  if (upb_FieldDef_IsRepeated(field_def)) {
    _cel_ParsedRepeatedFieldValue_Set(cel_Value_SetList(value),
                                      field_val.array_val, field_def);
    return true;
  }

  return _cel_Value_FromSingularField(value, context, field_val, field_def,
                                      status);
}

extern "C" bool cel_Value_FromRepeatedFieldElement(
    cel_Value* cel_nonnull value, const cel_ValueContext* cel_nonnull context,
    upb_MessageValue field_val, const upb_FieldDef* cel_nonnull field_def,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(field_def);
  CEL_ASSERT(upb_FieldDef_IsRepeated(field_def));
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  return _cel_Value_FromSingularField(value, context, field_val, field_def,
                                      status);
}

extern "C" bool cel_Value_FromMapFieldKey(
    cel_Value* cel_nonnull value, const cel_ValueContext* cel_nonnull context,
    upb_MessageValue field_val, const upb_FieldDef* cel_nonnull field_def,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(field_def);
  CEL_ASSERT_NOT(upb_FieldDef_IsRepeated(field_def));
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  return _cel_Value_FromSingularField(value, context, field_val, field_def,
                                      status);
}

extern "C" bool cel_Value_FromMapFieldValue(
    cel_Value* cel_nonnull value, const cel_ValueContext* cel_nonnull context,
    upb_MessageValue field_val, const upb_FieldDef* cel_nonnull field_def,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(field_def);
  CEL_ASSERT_NOT(upb_FieldDef_IsRepeated(field_def));
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  return _cel_Value_FromSingularField(value, context, field_val, field_def,
                                      status);
}
