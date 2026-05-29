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

#include "cel-c/type.h"

#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/string_view.h"
#include "cel-c/type_kind.h"
#include "upb/reflection/def.h"

struct cel_ListType {
  cel_TypeKind kind;

  const cel_Type* cel_nonnull elem;
};

struct cel_MapType {
  cel_TypeKind kind;

  const cel_Type* cel_nonnull key_val[2];
};

struct cel_StructType {
  cel_TypeKind kind;

  cel_StringView name;
};

struct cel_EnumType {
  cel_TypeKind kind;

  cel_StringView name;
};

struct cel_OpaqueType {
  cel_TypeKind kind;

  cel_StringView name;
  size_t params_len;
  const cel_Type* cel_nonnull params[];
};

struct cel_OptionalType {
  cel_OpaqueType super;
};

struct cel_TypeType {
  cel_TypeKind kind;

  const cel_Type* cel_nonnull type;
};

struct cel_TypeParamType {
  cel_TypeKind kind;

  cel_StringView name;
};

struct cel_FunctionType {
  cel_TypeKind kind;

  const cel_Type* cel_nonnull result;
  size_t args_len;
  const cel_Type* cel_nonnull args[];
};

struct cel_Type {
  union {
    struct {
      cel_TypeKind kind;
    };
    cel_ListType list_type;
    cel_MapType map_type;
    cel_StructType struct_type;
    cel_EnumType enum_type;
    cel_OpaqueType opaque_type;
    cel_OptionalType optional_type;
    cel_TypeType type_type;
    cel_TypeParamType type_param_type;
    cel_FunctionType function_type;
  };
};

bool cel_IsWellKnownMessageType(const upb_MessageDef* cel_nonnull def) {
  switch (upb_MessageDef_WellKnownType(def)) {
    case kUpb_WellKnown_Any:
      return true;
    case kUpb_WellKnown_Duration:
      return true;
    case kUpb_WellKnown_Timestamp:
      return true;
    case kUpb_WellKnown_DoubleValue:
      return true;
    case kUpb_WellKnown_FloatValue:
      return true;
    case kUpb_WellKnown_Int64Value:
      return true;
    case kUpb_WellKnown_UInt64Value:
      return true;
    case kUpb_WellKnown_Int32Value:
      return true;
    case kUpb_WellKnown_UInt32Value:
      return true;
    case kUpb_WellKnown_StringValue:
      return true;
    case kUpb_WellKnown_BytesValue:
      return true;
    case kUpb_WellKnown_BoolValue:
      return true;
    case kUpb_WellKnown_Value:
      return true;
    case kUpb_WellKnown_ListValue:
      return true;
    case kUpb_WellKnown_Struct:
      return true;
    default:
      return false;
  }
}

static const cel_StringView _cel_kWellKnownMessageTypeNames[] = {
    CEL_STRINGVIEW_C("Any"),         CEL_STRINGVIEW_C("BoolValue"),
    CEL_STRINGVIEW_C("BytesValue"),  CEL_STRINGVIEW_C("DoubleValue"),
    CEL_STRINGVIEW_C("Duration"),    CEL_STRINGVIEW_C("FloatValue"),
    CEL_STRINGVIEW_C("Int32Value"),  CEL_STRINGVIEW_C("Int64Value"),
    CEL_STRINGVIEW_C("ListValue"),   CEL_STRINGVIEW_C("StringValue"),
    CEL_STRINGVIEW_C("Struct"),      CEL_STRINGVIEW_C("Timestamp"),
    CEL_STRINGVIEW_C("UInt32Value"), CEL_STRINGVIEW_C("UInt64Value"),
    CEL_STRINGVIEW_C("Value"),
};

static int _cel_kWellKnownMessageTypeNames_Compare(
    const void* cel_nullability_unknown lhs,
    const void* cel_nullability_unknown rhs) {
  return cel_StringView_Compare(*(const cel_StringView*)lhs,
                                *(const cel_StringView*)rhs);
}

bool cel_IsWellKnownMessageTypeName(cel_StringView name) {
  return cel_StringView_ConsumePrefix(
             &name, cel_StringView_From("google.protobuf.")) &&
         bsearch(&name, &_cel_kWellKnownMessageTypeNames[0],
                 cel_arraysize(_cel_kWellKnownMessageTypeNames),
                 sizeof(*_cel_kWellKnownMessageTypeNames),
                 &_cel_kWellKnownMessageTypeNames_Compare) != cel_nullptr;
}

bool cel_IsWellKnownEnumType(const upb_EnumDef* cel_nonnull def) {
  const char* name = upb_EnumDef_FullName(def);
  return name != cel_nullptr && strcmp(name, "google.protobuf.NullValue") == 0;
}

bool cel_IsWellKnownEnumTypeName(cel_StringView name) {
  return cel_StringView_Equals(
      name, cel_StringView_From("google.protobuf.NullValue"));
}

static const cel_Type _cel_DynType = {
    .kind = cel_TypeKind_kDyn,
};

const cel_Type* const cel_nonnull cel_DynType = &_cel_DynType;

static const cel_Type _cel_NullType = {
    .kind = cel_TypeKind_kNull,
};

const cel_Type* const cel_nonnull cel_NullType = &_cel_NullType;

static const cel_Type _cel_BoolType = {
    .kind = cel_TypeKind_kBool,
};

const cel_Type* const cel_nonnull cel_BoolType = &_cel_BoolType;

static const cel_Type _cel_IntType = {
    .kind = cel_TypeKind_kInt,
};

const cel_Type* const cel_nonnull cel_IntType = &_cel_IntType;

static const cel_Type _cel_UintType = {
    .kind = cel_TypeKind_kUint,
};

const cel_Type* const cel_nonnull cel_UintType = &_cel_UintType;

static const cel_Type _cel_DoubleType = {
    .kind = cel_TypeKind_kDouble,
};

const cel_Type* const cel_nonnull cel_DoubleType = &_cel_DoubleType;

static const cel_Type _cel_StringType = {
    .kind = cel_TypeKind_kString,
};

const cel_Type* const cel_nonnull cel_StringType = &_cel_StringType;

static const cel_Type _cel_BytesType = {
    .kind = cel_TypeKind_kBytes,
};

const cel_Type* const cel_nonnull cel_BytesType = &_cel_BytesType;

static const cel_Type _cel_AnyType = {
    .kind = cel_TypeKind_kAny,
};

const cel_Type* const cel_nonnull cel_AnyType = &_cel_AnyType;

static const cel_Type _cel_DurationType = {
    .kind = cel_TypeKind_kDuration,
};

const cel_Type* const cel_nonnull cel_DurationType = &_cel_DurationType;

static const cel_Type _cel_TimestampType = {
    .kind = cel_TypeKind_kTimestamp,
};

const cel_Type* const cel_nonnull cel_TimestampType = &_cel_TimestampType;

static const cel_Type _cel_UnknownType = {
    .kind = cel_TypeKind_kUnknown,
};

const cel_Type* const cel_nonnull cel_UnknownType = &_cel_UnknownType;

static const cel_Type _cel_ErrorType = {
    .kind = cel_TypeKind_kError,
};

const cel_Type* const cel_nonnull cel_ErrorType = &_cel_ErrorType;

static const cel_Type _cel_BoolWrapperType = {
    .kind = cel_TypeKind_kBoolWrapper,
};

const cel_Type* const cel_nonnull cel_BoolWrapperType = &_cel_BoolWrapperType;

static const cel_Type _cel_IntWrapperType = {
    .kind = cel_TypeKind_kIntWrapper,
};

const cel_Type* const cel_nonnull cel_IntWrapperType = &_cel_IntWrapperType;

static const cel_Type _cel_UintWrapperType = {
    .kind = cel_TypeKind_kUintWrapper,
};

const cel_Type* const cel_nonnull cel_UintWrapperType = &_cel_UintWrapperType;

static const cel_Type _cel_DoubleWrapperType = {
    .kind = cel_TypeKind_kDoubleWrapper,
};

const cel_Type* const cel_nonnull cel_DoubleWrapperType =
    &_cel_DoubleWrapperType;

static const cel_Type _cel_StringWrapperType = {
    .kind = cel_TypeKind_kStringWrapper,
};

const cel_Type* const cel_nonnull cel_StringWrapperType =
    &_cel_StringWrapperType;

static const cel_Type _cel_BytesWrapperType = {
    .kind = cel_TypeKind_kBytesWrapper,
};

const cel_Type* const cel_nonnull cel_BytesWrapperType = &_cel_BytesWrapperType;

cel_TypeKind cel_Type_Kind(const cel_Type* cel_nonnull type) {
  CEL_ASSERT_NOT_NULL(type);

  return type->kind;
}

cel_StringView cel_Type_Name(const cel_Type* cel_nonnull type) {
  CEL_ASSERT_NOT_NULL(type);

  switch (cel_Type_Kind(type)) {
    case cel_TypeKind_kDyn:
      return cel_StringView_From("dyn");
    case cel_TypeKind_kNull:
      return cel_StringView_From("null_type");
    case cel_TypeKind_kBool:
      return cel_StringView_From("bool");
    case cel_TypeKind_kInt:
      return cel_StringView_From("int");
    case cel_TypeKind_kUint:
      return cel_StringView_From("uint");
    case cel_TypeKind_kDouble:
      return cel_StringView_From("double");
    case cel_TypeKind_kString:
      return cel_StringView_From("string");
    case cel_TypeKind_kBytes:
      return cel_StringView_From("bytes");
    case cel_TypeKind_kStruct:
      return cel_StructType_Name(cel_StructType_DownCast(type));
    case cel_TypeKind_kDuration:
      return cel_StringView_From("google.protobuf.Duration");
    case cel_TypeKind_kTimestamp:
      return cel_StringView_From("google.protobuf.Timestamp");
    case cel_TypeKind_kList:
      return cel_StringView_From("list");
    case cel_TypeKind_kMap:
      return cel_StringView_From("map");
    case cel_TypeKind_kUnknown:
      return cel_StringView_From("**unknown**");
    case cel_TypeKind_kType:
      return cel_StringView_From("type");
    case cel_TypeKind_kError:
      return cel_StringView_From("**error**");
    case cel_TypeKind_kAny:
      return cel_StringView_From("google.protobuf.Any");
    case cel_TypeKind_kOpaque:
      return cel_OpaqueType_Name(cel_OpaqueType_DownCast(type));
    case cel_TypeKind_kBoolWrapper:
      return cel_StringView_From("google.protobuf.BoolValue");
    case cel_TypeKind_kIntWrapper:
      return cel_StringView_From("google.protobuf.Int64Value");
    case cel_TypeKind_kUintWrapper:
      return cel_StringView_From("google.protobuf.UInt64Value");
    case cel_TypeKind_kDoubleWrapper:
      return cel_StringView_From("google.protobuf.DoubleValue");
    case cel_TypeKind_kStringWrapper:
      return cel_StringView_From("google.protobuf.StringValue");
    case cel_TypeKind_kBytesWrapper:
      return cel_StringView_From("google.protobuf.BytesValue");
    case cel_TypeKind_kTypeParam:
      return cel_TypeParamType_Name(cel_TypeParamType_DownCast(type));
    case cel_TypeKind_kFunction:
      return cel_StringView_From("**function**");
    case cel_TypeKind_kEnum:
      return cel_EnumType_Name(cel_EnumType_DownCast(type));
    default:
      return cel_StringView_From("");
  }
}

const cel_Type* cel_nonnull const* cel_nullability_unknown
cel_Type_Params(const cel_Type* cel_nonnull type, size_t* cel_nullable size) {
  CEL_ASSERT_NOT_NULL(type);

  switch (cel_Type_Kind(type)) {
    case cel_TypeKind_kList:
      if (size != cel_nullptr) {
        *size = 1;
      }
      return &cel_ListType_DownCast(type)->elem;
    case cel_TypeKind_kMap:
      if (size != cel_nullptr) {
        *size = 2;
      }
      return cel_MapType_DownCast(type)->key_val;
    case cel_TypeKind_kOpaque:
      return cel_OpaqueType_Params(cel_OpaqueType_DownCast(type), size);
    default:
      if (size != cel_nullptr) {
        *size = 0;
      }
      return cel_nullptr;
  }
}

bool cel_Type_Equals(const cel_Type* cel_nonnull lhs,
                     const cel_Type* cel_nonnull rhs) {
  CEL_ASSERT_NOT_NULL(lhs);
  CEL_ASSERT_NOT_NULL(rhs);

  if (lhs == rhs) {
    return true;
  }

  const cel_TypeKind kind = cel_Type_Kind(lhs);
  if (kind != cel_Type_Kind(rhs)) {
    return false;
  }

  switch (kind) {
    case cel_TypeKind_kDyn:
      return true;
    case cel_TypeKind_kNull:
      return true;
    case cel_TypeKind_kBool:
      return true;
    case cel_TypeKind_kInt:
      return true;
    case cel_TypeKind_kUint:
      return true;
    case cel_TypeKind_kDouble:
      return true;
    case cel_TypeKind_kString:
      return true;
    case cel_TypeKind_kBytes:
      return true;
    case cel_TypeKind_kStruct:
      return cel_StructType_Equals(cel_StructType_DownCast(lhs),
                                   cel_StructType_DownCast(rhs));
    case cel_TypeKind_kDuration:
      return true;
    case cel_TypeKind_kTimestamp:
      return true;
    case cel_TypeKind_kList:
      return cel_ListType_Equals(cel_ListType_DownCast(lhs),
                                 cel_ListType_DownCast(rhs));
    case cel_TypeKind_kMap:
      return cel_MapType_Equals(cel_MapType_DownCast(lhs),
                                cel_MapType_DownCast(rhs));
    case cel_TypeKind_kUnknown:
      return true;
    case cel_TypeKind_kType:
      return cel_TypeType_Equals(cel_TypeType_DownCast(lhs),
                                 cel_TypeType_DownCast(rhs));
    case cel_TypeKind_kError:
      return true;
    case cel_TypeKind_kAny:
      return true;
    case cel_TypeKind_kOpaque:
      return cel_OpaqueType_Equals(cel_OpaqueType_DownCast(lhs),
                                   cel_OpaqueType_DownCast(rhs));
    case cel_TypeKind_kBoolWrapper:
      return true;
    case cel_TypeKind_kIntWrapper:
      return true;
    case cel_TypeKind_kUintWrapper:
      return true;
    case cel_TypeKind_kDoubleWrapper:
      return true;
    case cel_TypeKind_kStringWrapper:
      return true;
    case cel_TypeKind_kBytesWrapper:
      return true;
    case cel_TypeKind_kTypeParam:
      return cel_TypeParamType_Equals(cel_TypeParamType_DownCast(lhs),
                                      cel_TypeParamType_DownCast(rhs));
    case cel_TypeKind_kFunction:
      return cel_FunctionType_Equals(cel_FunctionType_DownCast(lhs),
                                     cel_FunctionType_DownCast(rhs));
    case cel_TypeKind_kEnum:
      return cel_EnumType_Equals(cel_EnumType_DownCast(lhs),
                                 cel_EnumType_DownCast(rhs));
    default:
      return false;
  }
}

const cel_ListType* cel_nullable cel_ListType_New(
    const cel_Type* cel_nonnull element, cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(element);
  CEL_ASSERT_NOT_NULL(arena);

  cel_ListType* type =
      (cel_ListType*)cel_Arena_Malloc(arena, sizeof(cel_ListType), cel_nullptr);
  if (CEL_LIKELY(type != cel_nullptr)) {
    memset(type, '\0', sizeof(*type));
    type->kind = cel_TypeKind_kList;
    type->elem = element;
  }
  return type;
}

const cel_Type* cel_nonnull
cel_ListType_Element(const cel_ListType* cel_nonnull type) {
  CEL_ASSERT_NOT_NULL(type);

  return type->elem;
}

bool cel_ListType_Equals(const cel_ListType* cel_nonnull lhs,
                         const cel_ListType* cel_nonnull rhs) {
  CEL_ASSERT_NOT_NULL(lhs);
  CEL_ASSERT_NOT_NULL(rhs);

  if (lhs == rhs) {
    return true;
  }
  return cel_Type_Equals(cel_ListType_Element(lhs), cel_ListType_Element(rhs));
}

const cel_MapType* cel_nullable cel_MapType_New(
    const cel_Type* cel_nonnull key, const cel_Type* cel_nonnull value,
    cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(arena);

  cel_MapType* type =
      (cel_MapType*)cel_Arena_Malloc(arena, sizeof(cel_MapType), cel_nullptr);
  if (CEL_LIKELY(type != cel_nullptr)) {
    memset(type, '\0', sizeof(*type));
    type->kind = cel_TypeKind_kMap;
    type->key_val[0] = key;
    type->key_val[1] = value;
  }
  return type;
}

const cel_Type* cel_nonnull
cel_MapType_Key(const cel_MapType* cel_nonnull type) {
  CEL_ASSERT_NOT_NULL(type);

  return type->key_val[0];
}

const cel_Type* cel_nonnull
cel_MapType_Value(const cel_MapType* cel_nonnull type) {
  CEL_ASSERT_NOT_NULL(type);

  return type->key_val[1];
}

bool cel_MapType_Equals(const cel_MapType* cel_nonnull lhs,
                        const cel_MapType* cel_nonnull rhs) {
  CEL_ASSERT_NOT_NULL(lhs);
  CEL_ASSERT_NOT_NULL(rhs);

  if (lhs == rhs) {
    return true;
  }
  return cel_Type_Equals(cel_MapType_Key(lhs), cel_MapType_Key(rhs)) &&
         cel_Type_Equals(cel_MapType_Value(lhs), cel_MapType_Value(rhs));
}

const cel_StructType* cel_nullable
cel_StructType_New(cel_StringView name, cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(arena);

  cel_StructType* type = (cel_StructType*)cel_Arena_Malloc(
      arena, sizeof(cel_StructType), cel_nullptr);
  if (CEL_LIKELY(type != cel_nullptr)) {
    memset(type, '\0', sizeof(*type));
    type->kind = cel_TypeKind_kStruct;
    type->name = name;
  }
  return type;
}

cel_StringView cel_StructType_Name(const cel_StructType* cel_nonnull type) {
  CEL_ASSERT_NOT_NULL(type);

  return type->name;
}

bool cel_StructType_Equals(const cel_StructType* cel_nonnull lhs,
                           const cel_StructType* cel_nonnull rhs) {
  CEL_ASSERT_NOT_NULL(lhs);
  CEL_ASSERT_NOT_NULL(rhs);

  if (lhs == rhs) {
    return true;
  }
  return cel_StringView_Equals(cel_StructType_Name(lhs),
                               cel_StructType_Name(rhs));
}

const cel_EnumType* cel_nullable
cel_EnumType_New(cel_StringView name, cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(arena);

  cel_EnumType* type =
      (cel_EnumType*)cel_Arena_Malloc(arena, sizeof(cel_EnumType), cel_nullptr);
  if (CEL_LIKELY(type != cel_nullptr)) {
    memset(type, '\0', sizeof(*type));
    type->kind = cel_TypeKind_kEnum;
    type->name = name;
  }
  return type;
}

cel_StringView cel_EnumType_Name(const cel_EnumType* cel_nonnull type) {
  CEL_ASSERT_NOT_NULL(type);

  return type->name;
}

bool cel_EnumType_Equals(const cel_EnumType* cel_nonnull lhs,
                         const cel_EnumType* cel_nonnull rhs) {
  CEL_ASSERT_NOT_NULL(lhs);
  CEL_ASSERT_NOT_NULL(rhs);

  if (lhs == rhs) {
    return true;
  }
  return cel_StringView_Equals(cel_EnumType_Name(lhs), cel_EnumType_Name(rhs));
}

cel_OpaqueType* cel_nullable
cel_OpaqueType_New(cel_StringView name, size_t params_len,
                   const cel_Type * cel_nullability_unknown *
                       cel_nullability_unknown * cel_nullability_unknown params,
                   cel_Arena* cel_nonnull arena) {
  CEL_ASSERT(params != cel_nullptr || params_len == 0);
  CEL_ASSERT_NOT_NULL(arena);

  const size_t size =
      offsetof(cel_OpaqueType, params) + (sizeof(const cel_Type*) * params_len);
  cel_OpaqueType* type =
      (cel_OpaqueType*)cel_Arena_Malloc(arena, size, cel_nullptr);
  if (CEL_LIKELY(type != cel_nullptr)) {
    memset(type, '\0', size);
    type->kind = cel_TypeKind_kOpaque;
    type->name = name;
    type->params_len = params_len;
    if (params_len > 0) {
      *params = type->params;
    }
  }
  return type;
}

cel_StringView cel_OpaqueType_Name(const cel_OpaqueType* cel_nonnull type) {
  CEL_ASSERT_NOT_NULL(type);

  return type->name;
}

const cel_Type* const cel_nonnull* cel_nullability_unknown
cel_OpaqueType_Params(const cel_OpaqueType* cel_nonnull type,
                      size_t* cel_nullable size) {
  CEL_ASSERT_NOT_NULL(type);

  if (size != cel_nullptr) {
    *size = type->params_len;
  }
  return type->params;
}

bool cel_OpaqueType_Equals(const cel_OpaqueType* cel_nonnull lhs,
                           const cel_OpaqueType* cel_nonnull rhs) {
  CEL_ASSERT_NOT_NULL(lhs);
  CEL_ASSERT_NOT_NULL(rhs);

  if (lhs == rhs) {
    return true;
  }
  size_t lhs_params_size;
  size_t rhs_params_size;
  const cel_Type* const* lhs_params =
      cel_OpaqueType_Params(lhs, &lhs_params_size);
  const cel_Type* const* rhs_params =
      cel_OpaqueType_Params(rhs, &rhs_params_size);
  if (lhs_params_size != rhs_params_size) {
    return false;
  }
  if (!cel_StringView_Equals(cel_OpaqueType_Name(lhs),
                             cel_OpaqueType_Name(rhs))) {
    return false;
  }
  for (size_t i = 0; i < lhs_params_size; ++i) {
    if (!cel_Type_Equals(lhs_params[i], rhs_params[i])) {
      return false;
    }
  }
  return true;
}

const cel_OptionalType* cel_nullable cel_OptionalType_New(
    const cel_Type* cel_nonnull param, cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(param);
  CEL_ASSERT_NOT_NULL(arena);

  const cel_Type** params;
  cel_OpaqueType* type = cel_OpaqueType_New(
      cel_StringView_FromString("optional_type"), 1, &params, arena);
  if (CEL_LIKELY(type != cel_nullptr)) {
    *params = param;
  }
  return cel_OptionalType_MutableDownCast(type);
}

const cel_TypeType* cel_nullable cel_TypeType_New(
    const cel_Type* cel_nonnull type, cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(type);
  CEL_ASSERT_NOT_NULL(arena);

  cel_TypeType* type_type =
      (cel_TypeType*)cel_Arena_Malloc(arena, sizeof(cel_TypeType), cel_nullptr);
  if (CEL_LIKELY(type_type != cel_nullptr)) {
    memset(type_type, '\0', sizeof(*type_type));
    type_type->kind = cel_TypeKind_kType;
    type_type->type = type;
  }
  return type_type;
}

const cel_Type* cel_nonnull
cel_TypeType_Type(const cel_TypeType* cel_nonnull type) {
  CEL_ASSERT_NOT_NULL(type);

  return type->type;
}

bool cel_TypeType_Equals(const cel_TypeType* cel_nonnull lhs,
                         const cel_TypeType* cel_nonnull rhs) {
  CEL_ASSERT_NOT_NULL(lhs);
  CEL_ASSERT_NOT_NULL(rhs);

  if (lhs == rhs) {
    return true;
  }
  return cel_Type_Equals(cel_TypeType_Type(lhs), cel_TypeType_Type(rhs));
}

const cel_TypeParamType* cel_nullable
cel_TypeParamType_New(cel_StringView name, cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(arena);

  cel_TypeParamType* type = (cel_TypeParamType*)cel_Arena_Malloc(
      arena, sizeof(cel_TypeParamType), cel_nullptr);
  if (CEL_LIKELY(type != cel_nullptr)) {
    memset(type, '\0', sizeof(*type));
    type->kind = cel_TypeKind_kTypeParam;
    type->name = name;
  }
  return type;
}

cel_StringView cel_TypeParamType_Name(
    const cel_TypeParamType* cel_nonnull type) {
  CEL_ASSERT_NOT_NULL(type);

  return type->name;
}

bool cel_TypeParamType_Equals(const cel_TypeParamType* cel_nonnull lhs,
                              const cel_TypeParamType* cel_nonnull rhs) {
  CEL_ASSERT_NOT_NULL(lhs);
  CEL_ASSERT_NOT_NULL(rhs);

  if (lhs == rhs) {
    return true;
  }
  return cel_StringView_Equals(cel_TypeParamType_Name(lhs),
                               cel_TypeParamType_Name(rhs));
}

cel_FunctionType* cel_nullable
cel_FunctionType_New(const cel_Type* cel_nonnull result, size_t args_len,
                     const cel_Type * cel_nullability_unknown *
                         cel_nullability_unknown * cel_nullability_unknown args,
                     cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT(args != cel_nullptr || args_len == 0);
  CEL_ASSERT_NOT_NULL(arena);

  const size_t size =
      offsetof(cel_FunctionType, args) + (sizeof(const cel_Type*) * args_len);
  cel_FunctionType* type =
      (cel_FunctionType*)cel_Arena_Malloc(arena, size, cel_nullptr);
  if (CEL_LIKELY(type != cel_nullptr)) {
    memset(type, '\0', size);
    type->kind = cel_TypeKind_kFunction;
    type->result = result;
    type->args_len = args_len;
    if (args_len > 0) {
      *args = type->args;
    }
  }
  return type;
}

const cel_Type* cel_nonnull
cel_FunctionType_Result(const cel_FunctionType* cel_nonnull type) {
  CEL_ASSERT_NOT_NULL(type);

  return type->result;
}

const cel_Type* const cel_nonnull* cel_nullability_unknown
cel_FunctionType_Args(const cel_FunctionType* cel_nonnull type,
                      size_t* cel_nullable size) {
  CEL_ASSERT_NOT_NULL(type);

  if (size != cel_nullptr) {
    *size = type->args_len;
  }
  return type->args;
}

bool cel_FunctionType_Equals(const cel_FunctionType* cel_nonnull lhs,
                             const cel_FunctionType* cel_nonnull rhs) {
  CEL_ASSERT_NOT_NULL(lhs);
  CEL_ASSERT_NOT_NULL(rhs);

  if (lhs == rhs) {
    return true;
  }
  size_t lhs_args_size;
  size_t rhs_args_size;
  const cel_Type* const* lhs_args = cel_FunctionType_Args(lhs, &lhs_args_size);
  const cel_Type* const* rhs_args = cel_FunctionType_Args(rhs, &rhs_args_size);
  if (lhs_args_size != rhs_args_size) {
    return false;
  }
  if (!cel_Type_Equals(cel_FunctionType_Result(lhs),
                       cel_FunctionType_Result(rhs))) {
    return false;
  }
  for (size_t i = 0; i < lhs_args_size; ++i) {
    if (!cel_Type_Equals(lhs_args[i], rhs_args[i])) {
      return false;
    }
  }
  return true;
}
