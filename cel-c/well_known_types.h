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

#ifndef THIRD_PARTY_CEL_C_WELL_KNOWN_TYPES_H_
#define THIRD_PARTY_CEL_C_WELL_KNOWN_TYPES_H_

#include <stdbool.h>  // IWYU pragma: keep

#include "cel-c/config.h"
#include "cel-c/status.h"
#include "upb/reflection/def.h"

CEL_BEGIN_DECLS

typedef struct {
  const upb_EnumDef* cel_nonnull def;
  const upb_EnumValueDef* cel_nonnull value_def;
} cel_NullValueWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_NullValueWellKnownType_Initialize(
    cel_NullValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef struct {
  const upb_MessageDef* cel_nonnull def;
  const upb_FieldDef* cel_nonnull value_def;
} cel_WrapperWellKnownType;

typedef cel_WrapperWellKnownType cel_BoolValueWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_BoolValueWellKnownType_Initialize(
    cel_BoolValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef cel_WrapperWellKnownType cel_Int32ValueWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Int32ValueWellKnownType_Initialize(
    cel_Int32ValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef cel_WrapperWellKnownType cel_UInt32ValueWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_UInt32ValueWellKnownType_Initialize(
    cel_UInt32ValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef cel_WrapperWellKnownType cel_Int64ValueWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Int64ValueWellKnownType_Initialize(
    cel_Int64ValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef cel_WrapperWellKnownType cel_UInt64ValueWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_UInt64ValueWellKnownType_Initialize(
    cel_UInt64ValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef cel_WrapperWellKnownType cel_FloatValueWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_FloatValueWellKnownType_Initialize(
    cel_FloatValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef cel_WrapperWellKnownType cel_DoubleValueWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_DoubleValueWellKnownType_Initialize(
    cel_DoubleValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef cel_WrapperWellKnownType cel_BytesValueWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_BytesValueWellKnownType_Initialize(
    cel_BytesValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef cel_WrapperWellKnownType cel_StringValueWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_StringValueWellKnownType_Initialize(
    cel_StringValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef struct {
  const upb_MessageDef* cel_nonnull def;
  const upb_FieldDef* cel_nonnull seconds_def;
  const upb_FieldDef* cel_nonnull nanos_def;
} cel_TemporalWellKnownType;

typedef cel_TemporalWellKnownType cel_DurationWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_DurationWellKnownType_Initialize(
    cel_DurationWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef cel_TemporalWellKnownType cel_TimestampWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_TimestampWellKnownType_Initialize(
    cel_TimestampWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef struct {
  const upb_MessageDef* cel_nonnull def;
  const upb_FieldDef* cel_nonnull type_url_def;
  const upb_FieldDef* cel_nonnull value_def;
} cel_AnyWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_AnyWellKnownType_Initialize(
    cel_AnyWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef struct {
  const upb_MessageDef* cel_nonnull def;
  const upb_OneofDef* cel_nonnull kind_def;
  const upb_FieldDef* cel_nonnull null_value_def;
  const upb_FieldDef* cel_nonnull number_value_def;
  const upb_FieldDef* cel_nonnull string_value_def;
  const upb_FieldDef* cel_nonnull bool_value_def;
  const upb_FieldDef* cel_nonnull struct_value_def;
  const upb_FieldDef* cel_nonnull list_value_def;
} cel_ValueWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_ValueWellKnownType_Initialize(
    cel_ValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef struct {
  const upb_MessageDef* cel_nonnull def;
  const upb_FieldDef* cel_nonnull fields_def;
  const upb_FieldDef* cel_nonnull fields_key_def;
  const upb_FieldDef* cel_nonnull fields_value_def;
} cel_StructWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_StructWellKnownType_Initialize(
    cel_StructWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef struct {
  const upb_MessageDef* cel_nonnull def;
  const upb_FieldDef* cel_nonnull values_def;
} cel_ListValueWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_ListValueWellKnownType_Initialize(
    cel_ListValueWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef struct {
  cel_ValueWellKnownType value;
  cel_StructWellKnownType struct_value;
  cel_ListValueWellKnownType list_value;
} cel_JsonWellKnownType;

typedef struct {
  const upb_MessageDef* cel_nonnull def;
  const upb_FieldDef* cel_nonnull paths_def;
} cel_FieldMaskWellKnownType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_FieldMaskWellKnownType_Initialize(
    cel_FieldMaskWellKnownType* cel_nonnull wkt,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

typedef struct {
  cel_NullValueWellKnownType null_value;
  cel_BoolValueWellKnownType bool_value;
  cel_Int32ValueWellKnownType int32_value;
  cel_UInt32ValueWellKnownType uint32_value;
  cel_Int64ValueWellKnownType int64_value;
  cel_UInt64ValueWellKnownType uint64_value;
  cel_FloatValueWellKnownType float_value;
  cel_DoubleValueWellKnownType double_value;
  cel_BytesValueWellKnownType bytes_value;
  cel_StringValueWellKnownType string_value;
  cel_DurationWellKnownType duration;
  cel_TimestampWellKnownType timestamp;
  cel_AnyWellKnownType any;
  union {
    struct {
      cel_ValueWellKnownType value;
      cel_StructWellKnownType struct_value;
      cel_ListValueWellKnownType list_value;
    };
    cel_JsonWellKnownType json;
  };
  cel_FieldMaskWellKnownType field_mask;
} cel_WellKnownTypes;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_WellKnownTypes_Initialize(
    cel_WellKnownTypes* cel_nonnull wkts,
    const upb_DefPool* cel_nonnull def_pool, cel_Status* cel_nonnull status);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_WELL_KNOWN_TYPES_H_
