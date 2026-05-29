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

#include "cel-c/well_known_types.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "upb/base/descriptor_constants.h"
#include "upb/reflection/def.h"

CEL_ATTRIBUTE_NODISCARD
static const upb_MessageDef* cel_nullable _cel_WellKnownType_FindMessage(
    const upb_DefPool* cel_nonnull def_pool, cel_StringView name,
    bool error_if_not_found, cel_Status* cel_nonnull status) {
  CEL_ASSERT(cel_Status_Ok(status));

  const upb_MessageDef* def = upb_DefPool_FindMessageByNameWithSize(
      def_pool, cel_StringView_Data(name), cel_StringView_Size(name));
  if (CEL_UNLIKELY(def == cel_nullptr)) {
    if (error_if_not_found) {
      cel_NotFoundStatusF(
          status, "cel: well known message type not found: " CEL_STRINGVIEW_FMT,
          CEL_STRINGVIEW_ARGS(name));
    }
    return cel_nullptr;
  }
  return def;
}

typedef enum {
  _cel_WellKnownTypeFieldLabel_kOptional = 1,
  _cel_WellKnownTypeFieldLabel_kRepeated,
  _cel_WellKnownTypeFieldLabel_kMap,
} _cel_WellKnownTypeFieldLabel;

static const char* cel_nonnull _cel_WellKnownType_LabelName(upb_Label label) {
  switch (label) {
    case kUpb_Label_Optional:
      return "optional";
    case kUpb_Label_Required:
      return "required";
    case kUpb_Label_Repeated:
      return "repeated";
    default:
      return "unknown";
  }
}

CEL_ATTRIBUTE_NODISCARD
static const upb_FieldDef* cel_nullable _cel_WellKnownType_CheckField(
    const upb_FieldDef* cel_nonnull field_def, uint32_t number, upb_CType type,
    _cel_WellKnownTypeFieldLabel label, cel_StringView type_name,
    const upb_OneofDef* cel_nullable oneof, cel_Status* cel_nonnull status) {
  CEL_ASSERT(cel_Status_Ok(status));

  if (CEL_UNLIKELY(upb_FieldDef_Number(field_def) != number)) {
    cel_FailedPreconditionStatusF(
        status,
        "cel: well known message type field number unexpected: "
        "%s: got %" PRIu32 ", want %" PRIu32,
        upb_FieldDef_FullName(field_def), upb_FieldDef_Number(field_def),
        number);
    return cel_nullptr;
  }
  if (CEL_UNLIKELY(upb_FieldDef_CType(field_def) != type)) {
    cel_FailedPreconditionStatusF(
        status,
        "cel: well known message type field type unexpected: "
        "%s: got %d, want %d",
        upb_FieldDef_FullName(field_def), upb_FieldDef_CType(field_def), type);
    return cel_nullptr;
  }
  switch (label) {
    case _cel_WellKnownTypeFieldLabel_kOptional:
      if (CEL_UNLIKELY(upb_FieldDef_Label(field_def) != kUpb_Label_Optional)) {
        cel_FailedPreconditionStatusF(
            status,
            "cel: well known message type field label unexpected: "
            "%s: got %s, want optional",
            upb_FieldDef_FullName(field_def),
            _cel_WellKnownType_LabelName(upb_FieldDef_Label(field_def)));
        return cel_nullptr;
      }
      break;
    case _cel_WellKnownTypeFieldLabel_kRepeated:
      if (CEL_UNLIKELY(upb_FieldDef_IsMap(field_def))) {
        cel_FailedPreconditionStatusF(
            status,
            "cel: well known message type field label unexpected: "
            "%s: got map, want repeated",
            upb_FieldDef_FullName(field_def));
        return cel_nullptr;
      }
      if (CEL_UNLIKELY(!upb_FieldDef_IsRepeated(field_def))) {
        cel_FailedPreconditionStatusF(
            status,
            "cel: well known message type field label unexpected: "
            "%s: got %s, want repeated",
            upb_FieldDef_FullName(field_def),
            _cel_WellKnownType_LabelName(upb_FieldDef_Label(field_def)));
        return cel_nullptr;
      }
      break;
    case _cel_WellKnownTypeFieldLabel_kMap:
      if (CEL_UNLIKELY(!upb_FieldDef_IsMap(field_def))) {
        cel_FailedPreconditionStatusF(
            status,
            "cel: well known message type field label unexpected: "
            "%s: got %s, want map",
            upb_FieldDef_FullName(field_def),
            _cel_WellKnownType_LabelName(upb_FieldDef_Label(field_def)));
        return cel_nullptr;
      }
      break;
    default:
      CEL_UNREACHABLE();
  }
  if (type == kUpb_CType_Message) {
    if (CEL_UNLIKELY(!cel_StringView_Equals(
            cel_StringView_From(
                upb_MessageDef_FullName(upb_FieldDef_MessageSubDef(field_def))),
            type_name))) {
      cel_FailedPreconditionStatusF(
          status,
          "cel: well known message type field type name unexpected: "
          "%s: got %s, want " CEL_STRINGVIEW_FMT,
          upb_FieldDef_FullName(field_def),
          upb_MessageDef_FullName(upb_FieldDef_MessageSubDef(field_def)),
          CEL_STRINGVIEW_ARGS(type_name));
      return cel_nullptr;
    }
  } else if (type == kUpb_CType_Enum) {
    if (CEL_UNLIKELY(!cel_StringView_Equals(
            cel_StringView_From(
                upb_EnumDef_FullName(upb_FieldDef_EnumSubDef(field_def))),
            type_name))) {
      cel_FailedPreconditionStatusF(
          status,
          "cel: well known message type field type name unexpected: "
          "%s: got %s, want " CEL_STRINGVIEW_FMT,
          upb_FieldDef_FullName(field_def),
          upb_EnumDef_FullName(upb_FieldDef_EnumSubDef(field_def)),
          CEL_STRINGVIEW_ARGS(type_name));
      return cel_nullptr;
    }
  }
  const upb_OneofDef* got_oneof = upb_FieldDef_ContainingOneof(field_def);
  if (CEL_UNLIKELY(got_oneof != oneof)) {
    cel_FailedPreconditionStatusF(
        status,
        "cel: well known message type field oneof unexpected: "
        "%s: got %s, want %s",
        upb_FieldDef_FullName(field_def),
        got_oneof != cel_nullptr ? upb_OneofDef_Name(got_oneof) : "null",
        oneof != cel_nullptr ? upb_OneofDef_Name(oneof) : "null");
    return cel_nullptr;
  }
  return field_def;
}

CEL_ATTRIBUTE_NODISCARD
static const upb_FieldDef* cel_nullable _cel_WellKnownType_FindField(
    const upb_MessageDef* cel_nonnull def, cel_StringView name, uint32_t number,
    upb_CType type, _cel_WellKnownTypeFieldLabel label,
    cel_StringView type_name, const upb_OneofDef* cel_nullable oneof,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT(cel_Status_Ok(status));

  const upb_FieldDef* field_def = upb_MessageDef_FindFieldByNameWithSize(
      def, cel_StringView_Data(name), cel_StringView_Size(name));
  if (CEL_UNLIKELY(field_def == cel_nullptr)) {
    cel_NotFoundStatusF(
        status,
        "cel: well known message type field not found: %s." CEL_STRINGVIEW_FMT,
        upb_MessageDef_FullName(def), CEL_STRINGVIEW_ARGS(name));
    return cel_nullptr;
  }
  return _cel_WellKnownType_CheckField(field_def, number, type, label,
                                       type_name, oneof, status);
}

bool cel_NullValueWellKnownType_Initialize(
    cel_NullValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(wkt);
  CEL_ASSERT_NOT_NULL(def_pool);
  CEL_ASSERT_NOT_NULL(status);

  memset(wkt, 0, sizeof(*wkt));

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_EnumDef* def =
      upb_DefPool_FindEnumByName(def_pool, "google.protobuf.NullValue");
  if (CEL_UNLIKELY(def == cel_nullptr)) {
    cel_NotFoundStatus(
        status,
        cel_StringView_From(
            "cel: well known enum type not found: google.protobuf.NullValue"));
    return false;
  }
  if (CEL_UNLIKELY(upb_EnumDef_ValueCount(def) != 1)) {
    cel_FailedPreconditionStatus(
        status,
        cel_StringView_From("cel: well known enum type does not have exactly "
                       "one value: google.protobuf.NullValue"));
    return false;
  }
  const upb_EnumValueDef* value_def = upb_EnumDef_FindValueByNumber(def, 0);
  if (CEL_UNLIKELY(value_def == cel_nullptr)) {
    cel_NotFoundStatus(
        status, cel_StringView_From("cel: well known enum type value not found: "
                               "google.protobuf.NullValue.0"));
    return false;
  }
  const char* name = upb_EnumValueDef_Name(value_def);
  if (CEL_UNLIKELY(name == cel_nullptr || strcmp(name, "NULL_VALUE") != 0)) {
    cel_NotFoundStatus(
        status, cel_StringView_From("cel: well known enum type value not found: "
                               "google.protobuf.NullValue.NULL_VALUE"));
    return false;
  }

  wkt->def = def;
  wkt->value_def = value_def;
  return true;
}

static bool _cel_WrapperWellKnownType_Initialize(
    cel_BoolValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_StringView name,
    upb_CType type, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(wkt);
  CEL_ASSERT_NOT_NULL(def_pool);
  CEL_ASSERT_NOT_NULL(status);

  memset(wkt, 0, sizeof(*wkt));

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_MessageDef* def = _cel_WellKnownType_FindMessage(
      def_pool, name, /*error_if_not_found=*/true, status);
  if (CEL_UNLIKELY(def == cel_nullptr)) {
    return false;
  }
  const upb_FieldDef* value_def = _cel_WellKnownType_FindField(
      def, cel_StringView_From("value"), UINT32_C(1), type,
      _cel_WellKnownTypeFieldLabel_kOptional, cel_StringView_From(""),
      /*oneof=*/cel_nullptr, status);
  if (CEL_UNLIKELY(value_def == cel_nullptr)) {
    return false;
  }

  wkt->def = def;
  wkt->value_def = value_def;
  return true;
}

bool cel_BoolValueWellKnownType_Initialize(
    cel_BoolValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  return _cel_WrapperWellKnownType_Initialize(
      wkt, def_pool, cel_StringView_From("google.protobuf.BoolValue"),
      kUpb_CType_Bool, status);
}

bool cel_Int32ValueWellKnownType_Initialize(
    cel_Int32ValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  return _cel_WrapperWellKnownType_Initialize(
      wkt, def_pool, cel_StringView_From("google.protobuf.Int32Value"),
      kUpb_CType_Int32, status);
}

bool cel_UInt32ValueWellKnownType_Initialize(
    cel_UInt32ValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  return _cel_WrapperWellKnownType_Initialize(
      wkt, def_pool, cel_StringView_From("google.protobuf.UInt32Value"),
      kUpb_CType_UInt32, status);
}

bool cel_Int64ValueWellKnownType_Initialize(
    cel_Int64ValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  return _cel_WrapperWellKnownType_Initialize(
      wkt, def_pool, cel_StringView_From("google.protobuf.Int64Value"),
      kUpb_CType_Int64, status);
}

bool cel_UInt64ValueWellKnownType_Initialize(
    cel_UInt64ValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  return _cel_WrapperWellKnownType_Initialize(
      wkt, def_pool, cel_StringView_From("google.protobuf.UInt64Value"),
      kUpb_CType_UInt64, status);
}

bool cel_FloatValueWellKnownType_Initialize(
    cel_FloatValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  return _cel_WrapperWellKnownType_Initialize(
      wkt, def_pool, cel_StringView_From("google.protobuf.FloatValue"),
      kUpb_CType_Float, status);
}

bool cel_DoubleValueWellKnownType_Initialize(
    cel_DoubleValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  return _cel_WrapperWellKnownType_Initialize(
      wkt, def_pool, cel_StringView_From("google.protobuf.DoubleValue"),
      kUpb_CType_Double, status);
}

bool cel_BytesValueWellKnownType_Initialize(
    cel_BytesValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  return _cel_WrapperWellKnownType_Initialize(
      wkt, def_pool, cel_StringView_From("google.protobuf.BytesValue"),
      kUpb_CType_Bytes, status);
}

bool cel_StringValueWellKnownType_Initialize(
    cel_StringValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  return _cel_WrapperWellKnownType_Initialize(
      wkt, def_pool, cel_StringView_From("google.protobuf.StringValue"),
      kUpb_CType_String, status);
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_TemporalWellKnownType_Initialize(
    cel_TemporalWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_StringView name,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(wkt);
  CEL_ASSERT_NOT_NULL(def_pool);
  CEL_ASSERT_NOT_NULL(status);

  memset(wkt, 0, sizeof(*wkt));

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_MessageDef* def = _cel_WellKnownType_FindMessage(
      def_pool, name, /*error_if_not_found=*/true, status);
  if (CEL_UNLIKELY(def == cel_nullptr)) {
    return false;
  }
  const upb_FieldDef* seconds_def = _cel_WellKnownType_FindField(
      def, cel_StringView_From("seconds"), UINT32_C(1), kUpb_CType_Int64,
      _cel_WellKnownTypeFieldLabel_kOptional, cel_StringView_From(""),
      /*oneof=*/cel_nullptr, status);
  if (CEL_UNLIKELY(seconds_def == cel_nullptr)) {
    return false;
  }
  const upb_FieldDef* nanos_def = _cel_WellKnownType_FindField(
      def, cel_StringView_From("nanos"), UINT32_C(2), kUpb_CType_Int32,
      _cel_WellKnownTypeFieldLabel_kOptional, cel_StringView_From(""),
      /*oneof=*/cel_nullptr, status);
  if (CEL_UNLIKELY(nanos_def == cel_nullptr)) {
    return false;
  }

  wkt->def = def;
  wkt->seconds_def = seconds_def;
  wkt->nanos_def = nanos_def;
  return true;
}

bool cel_DurationWellKnownType_Initialize(
    cel_DurationWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  return _cel_TemporalWellKnownType_Initialize(
      wkt, def_pool, cel_StringView_From("google.protobuf.Duration"), status);
}

bool cel_TimestampWellKnownType_Initialize(
    cel_TimestampWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  return _cel_TemporalWellKnownType_Initialize(
      wkt, def_pool, cel_StringView_From("google.protobuf.Timestamp"), status);
}

bool cel_AnyWellKnownType_Initialize(cel_AnyWellKnownType* cel_nonnull wkt,
                                     const upb_DefPool* cel_nonnull def_pool,
                                     cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(wkt);
  CEL_ASSERT_NOT_NULL(def_pool);
  CEL_ASSERT_NOT_NULL(status);

  memset(wkt, 0, sizeof(*wkt));

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_MessageDef* def = _cel_WellKnownType_FindMessage(
      def_pool, cel_StringView_From("google.protobuf.Any"),
      /*error_if_not_found=*/true, status);
  if (CEL_UNLIKELY(def == cel_nullptr)) {
    return false;
  }
  const upb_FieldDef* type_url_def = _cel_WellKnownType_FindField(
      def, cel_StringView_From("type_url"), UINT32_C(1), kUpb_CType_String,
      _cel_WellKnownTypeFieldLabel_kOptional, cel_StringView_From(""),
      /*oneof=*/cel_nullptr, status);
  if (CEL_UNLIKELY(type_url_def == cel_nullptr)) {
    return false;
  }
  const upb_FieldDef* value_def = _cel_WellKnownType_FindField(
      def, cel_StringView_From("value"), UINT32_C(2), kUpb_CType_Bytes,
      _cel_WellKnownTypeFieldLabel_kOptional, cel_StringView_From(""),
      /*oneof=*/cel_nullptr, status);
  if (CEL_UNLIKELY(value_def == cel_nullptr)) {
    return false;
  }

  wkt->def = def;
  wkt->type_url_def = type_url_def;
  wkt->value_def = value_def;
  return true;
}

bool cel_ValueWellKnownType_Initialize(cel_ValueWellKnownType* cel_nonnull wkt,
                                       const upb_DefPool* cel_nonnull def_pool,
                                       cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(wkt);
  CEL_ASSERT_NOT_NULL(def_pool);
  CEL_ASSERT_NOT_NULL(status);

  memset(wkt, 0, sizeof(*wkt));

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_MessageDef* def = _cel_WellKnownType_FindMessage(
      def_pool, cel_StringView_From("google.protobuf.Value"),
      /*error_if_not_found=*/true, status);
  if (CEL_UNLIKELY(def == cel_nullptr)) {
    return false;
  }
  const upb_OneofDef* kind_def = upb_MessageDef_FindOneofByName(def, "kind");
  if (CEL_UNLIKELY(kind_def == cel_nullptr)) {
    cel_NotFoundStatusF(status,
                        "cel: well known message type oneof not found: %s.kind",
                        upb_MessageDef_FullName(def));
    return false;
  }
  if (CEL_UNLIKELY(upb_OneofDef_FieldCount(kind_def) != 6)) {
    cel_FailedPreconditionStatusF(
        status,
        "cel: well known message type oneof does not have "
        "exactly 6 fields: %s %d",
        upb_OneofDef_FullName(kind_def), upb_OneofDef_FieldCount(kind_def));
    return false;
  }
  const upb_FieldDef* null_value_def = _cel_WellKnownType_FindField(
      def, cel_StringView_From("null_value"), UINT32_C(1), kUpb_CType_Enum,
      _cel_WellKnownTypeFieldLabel_kOptional,
      cel_StringView_From("google.protobuf.NullValue"), kind_def, status);
  if (CEL_UNLIKELY(null_value_def == cel_nullptr)) {
    return false;
  }
  const upb_FieldDef* number_value_def = _cel_WellKnownType_FindField(
      def, cel_StringView_From("number_value"), UINT32_C(2), kUpb_CType_Double,
      _cel_WellKnownTypeFieldLabel_kOptional, cel_StringView_From(""), kind_def,
      status);
  if (CEL_UNLIKELY(number_value_def == cel_nullptr)) {
    return false;
  }
  const upb_FieldDef* string_value_def = _cel_WellKnownType_FindField(
      def, cel_StringView_From("string_value"), UINT32_C(3), kUpb_CType_String,
      _cel_WellKnownTypeFieldLabel_kOptional, cel_StringView_From(""), kind_def,
      status);
  if (CEL_UNLIKELY(string_value_def == cel_nullptr)) {
    return false;
  }
  const upb_FieldDef* bool_value_def = _cel_WellKnownType_FindField(
      def, cel_StringView_From("bool_value"), UINT32_C(4), kUpb_CType_Bool,
      _cel_WellKnownTypeFieldLabel_kOptional, cel_StringView_From(""), kind_def,
      status);
  if (CEL_UNLIKELY(bool_value_def == cel_nullptr)) {
    return false;
  }
  const upb_FieldDef* struct_value_def = _cel_WellKnownType_FindField(
      def, cel_StringView_From("struct_value"), UINT32_C(5), kUpb_CType_Message,
      _cel_WellKnownTypeFieldLabel_kOptional,
      cel_StringView_From("google.protobuf.Struct"), kind_def, status);
  if (CEL_UNLIKELY(struct_value_def == cel_nullptr)) {
    return false;
  }
  const upb_FieldDef* list_value_def = _cel_WellKnownType_FindField(
      def, cel_StringView_From("list_value"), UINT32_C(6), kUpb_CType_Message,
      _cel_WellKnownTypeFieldLabel_kOptional,
      cel_StringView_From("google.protobuf.ListValue"), kind_def, status);
  if (CEL_UNLIKELY(list_value_def == cel_nullptr)) {
    return false;
  }

  wkt->def = def;
  wkt->kind_def = kind_def;
  wkt->null_value_def = null_value_def;
  wkt->number_value_def = number_value_def;
  wkt->string_value_def = string_value_def;
  wkt->bool_value_def = bool_value_def;
  wkt->struct_value_def = struct_value_def;
  wkt->list_value_def = list_value_def;
  return true;
}

bool cel_StructWellKnownType_Initialize(
    cel_StructWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(wkt);
  CEL_ASSERT_NOT_NULL(def_pool);
  CEL_ASSERT_NOT_NULL(status);

  memset(wkt, 0, sizeof(*wkt));

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_MessageDef* def = _cel_WellKnownType_FindMessage(
      def_pool, cel_StringView_From("google.protobuf.Struct"),
      /*error_if_not_found=*/true, status);
  if (CEL_UNLIKELY(def == cel_nullptr)) {
    return false;
  }
  const upb_FieldDef* fields_def = _cel_WellKnownType_FindField(
      def, cel_StringView_From("fields"), UINT32_C(1), kUpb_CType_Message,
      _cel_WellKnownTypeFieldLabel_kMap,
      cel_StringView_From("google.protobuf.Struct.FieldsEntry"),
      /*oneof=*/cel_nullptr, status);
  if (CEL_UNLIKELY(fields_def == cel_nullptr)) {
    return false;
  }
  const upb_MessageDef* fields_entry_def =
      upb_FieldDef_MessageSubDef(fields_def);
  const upb_FieldDef* fields_key_def = _cel_WellKnownType_CheckField(
      upb_MessageDef_FindFieldByNumber(fields_entry_def,
                                       kUpb_MapEntry_KeyFieldNumber),
      kUpb_MapEntry_KeyFieldNumber, kUpb_CType_String,
      _cel_WellKnownTypeFieldLabel_kOptional, cel_StringView_From(""),
      /*oneof=*/cel_nullptr, status);
  if (fields_key_def == cel_nullptr) {
    return false;
  }
  const upb_FieldDef* fields_value_def = _cel_WellKnownType_CheckField(
      upb_MessageDef_FindFieldByNumber(fields_entry_def,
                                       kUpb_MapEntry_ValueFieldNumber),
      kUpb_MapEntry_ValueFieldNumber, kUpb_CType_Message,
      _cel_WellKnownTypeFieldLabel_kOptional,
      cel_StringView_From("google.protobuf.Value"),
      /*oneof=*/cel_nullptr, status);
  if (fields_value_def == cel_nullptr) {
    return false;
  }

  wkt->def = def;
  wkt->fields_def = fields_def;
  wkt->fields_key_def = fields_key_def;
  wkt->fields_value_def = fields_value_def;
  return true;
}

bool cel_ListValueWellKnownType_Initialize(
    cel_ListValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(wkt);
  CEL_ASSERT_NOT_NULL(def_pool);
  CEL_ASSERT_NOT_NULL(status);

  memset(wkt, 0, sizeof(*wkt));

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_MessageDef* def = _cel_WellKnownType_FindMessage(
      def_pool, cel_StringView_From("google.protobuf.ListValue"),
      /*error_if_not_found=*/true, status);
  if (CEL_UNLIKELY(def == cel_nullptr)) {
    return false;
  }
  const upb_FieldDef* values_def = _cel_WellKnownType_FindField(
      def, cel_StringView_From("values"), UINT32_C(1), kUpb_CType_Message,
      _cel_WellKnownTypeFieldLabel_kRepeated,
      cel_StringView_From("google.protobuf.Value"),
      /*oneof=*/cel_nullptr, status);
  if (CEL_UNLIKELY(values_def == cel_nullptr)) {
    return false;
  }

  wkt->def = def;
  wkt->values_def = values_def;
  return true;
}

bool cel_FieldMaskWellKnownType_Initialize(
    cel_FieldMaskWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(wkt);
  CEL_ASSERT_NOT_NULL(def_pool);
  CEL_ASSERT_NOT_NULL(status);

  memset(wkt, 0, sizeof(*wkt));

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_MessageDef* def = _cel_WellKnownType_FindMessage(
      def_pool, cel_StringView_From("google.protobuf.FieldMask"),
      /*error_if_not_found=*/false, status);
  if (CEL_UNLIKELY(def == cel_nullptr)) {
    return false;
  }
  const upb_FieldDef* paths_def = _cel_WellKnownType_FindField(
      def, cel_StringView_From("paths"), UINT32_C(1), kUpb_CType_String,
      _cel_WellKnownTypeFieldLabel_kRepeated, cel_StringView_From(""),
      /*oneof=*/cel_nullptr, status);
  if (CEL_UNLIKELY(paths_def == cel_nullptr)) {
    return false;
  }

  wkt->def = def;
  wkt->paths_def = paths_def;
  return true;
}

bool cel_WellKnownTypes_Initialize(cel_WellKnownTypes* cel_nonnull wkts,
                                   const upb_DefPool* cel_nonnull def_pool,
                                   cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(wkts);
  CEL_ASSERT_NOT_NULL(def_pool);
  CEL_ASSERT_NOT_NULL(status);

  memset(wkts, 0, sizeof(*wkts));

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  if (!cel_NullValueWellKnownType_Initialize(&wkts->null_value, def_pool,
                                             status)) {
    return false;
  }
  if (!cel_BoolValueWellKnownType_Initialize(&wkts->bool_value, def_pool,
                                             status)) {
    return false;
  }
  if (!cel_Int32ValueWellKnownType_Initialize(&wkts->int32_value, def_pool,
                                              status)) {
    return false;
  }
  if (!cel_UInt32ValueWellKnownType_Initialize(&wkts->uint32_value, def_pool,
                                               status)) {
    return false;
  }
  if (!cel_Int64ValueWellKnownType_Initialize(&wkts->int64_value, def_pool,
                                              status)) {
    return false;
  }
  if (!cel_UInt64ValueWellKnownType_Initialize(&wkts->uint64_value, def_pool,
                                               status)) {
    return false;
  }
  if (!cel_FloatValueWellKnownType_Initialize(&wkts->float_value, def_pool,
                                              status)) {
    return false;
  }
  if (!cel_DoubleValueWellKnownType_Initialize(&wkts->double_value, def_pool,
                                               status)) {
    return false;
  }
  if (!cel_BytesValueWellKnownType_Initialize(&wkts->bytes_value, def_pool,
                                              status)) {
    return false;
  }
  if (!cel_StringValueWellKnownType_Initialize(&wkts->string_value, def_pool,
                                               status)) {
    return false;
  }
  if (!cel_DurationWellKnownType_Initialize(&wkts->duration, def_pool,
                                            status)) {
    return false;
  }
  if (!cel_TimestampWellKnownType_Initialize(&wkts->timestamp, def_pool,
                                             status)) {
    return false;
  }
  if (!cel_AnyWellKnownType_Initialize(&wkts->any, def_pool, status)) {
    return false;
  }
  if (!cel_ValueWellKnownType_Initialize(&wkts->value, def_pool, status)) {
    return false;
  }
  if (!cel_StructWellKnownType_Initialize(&wkts->struct_value, def_pool,
                                          status)) {
    return false;
  }
  if (!cel_ListValueWellKnownType_Initialize(&wkts->list_value, def_pool,
                                             status)) {
    return false;
  }
  if (!cel_FieldMaskWellKnownType_Initialize(&wkts->field_mask, def_pool,
                                             status) &&
      !cel_Status_Ok(status)) {
    return false;
  }
  return true;
}
