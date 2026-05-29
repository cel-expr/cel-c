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

#ifndef THIRD_PARTY_CEL_C_TYPE_H_
#define THIRD_PARTY_CEL_C_TYPE_H_

#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>
#include <stdint.h>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/string_view.h"
#include "cel-c/type_kind.h"
#include "upb/reflection/def.h"

CEL_BEGIN_DECLS

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_IsWellKnownMessageType(
    const upb_MessageDef* cel_nonnull def);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_IsWellKnownMessageTypeName(cel_StringView name);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_IsWellKnownEnumType(const upb_EnumDef* cel_nonnull def);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_IsWellKnownEnumTypeName(cel_StringView name);

typedef struct cel_Type cel_Type;
typedef struct cel_ListType cel_ListType;
typedef struct cel_MapType cel_MapType;
typedef struct cel_StructType cel_StructType;
typedef struct cel_EnumType cel_EnumType;
typedef struct cel_OpaqueType cel_OpaqueType;
typedef struct cel_OptionalType cel_OptionalType;
typedef struct cel_TypeParamType cel_TypeParamType;
typedef struct cel_TypeType cel_TypeType;
typedef struct cel_FunctionType cel_FunctionType;

// cel_Type

CEL_EXTERN const cel_Type* const cel_nonnull cel_DynType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_NullType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_BoolType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_IntType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_UintType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_DoubleType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_StringType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_BytesType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_AnyType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_DurationType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_TimestampType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_UnknownType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_ErrorType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_BoolWrapperType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_IntWrapperType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_UintWrapperType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_DoubleWrapperType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_StringWrapperType;

CEL_EXTERN const cel_Type* const cel_nonnull cel_BytesWrapperType;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_TypeKind cel_Type_Kind(const cel_Type* cel_nonnull type);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_Type_Name(const cel_Type* cel_nonnull type);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_Type* cel_nonnull const* cel_nullability_unknown
cel_Type_Params(const cel_Type* cel_nonnull type, size_t* cel_nullable size);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Type_Equals(const cel_Type* cel_nonnull lhs,
                                const cel_Type* cel_nonnull rhs);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsDyn(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kDyn;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsNull(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kNull;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsBool(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kBool;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsInt(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kInt;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsUint(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kUint;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsDouble(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kDouble;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsString(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kString;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsBytes(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kBytes;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsStruct(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kStruct;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsDuration(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kDuration;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsTimestamp(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kTimestamp;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsList(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kList;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsMap(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kMap;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsUnknown(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kUnknown;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsError(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kError;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsAny(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kAny;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsOpaque(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kOpaque;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsBoolWrapper(
    const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kBoolWrapper;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsIntWrapper(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kIntWrapper;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsUintWrapper(
    const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kUintWrapper;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsDoubleWrapper(
    const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kDoubleWrapper;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsStringWrapper(
    const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kStringWrapper;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsBytesWrapper(
    const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kBytesWrapper;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsWrapper(const cel_Type* cel_nonnull type) {
  switch (cel_Type_Kind(type)) {
    case cel_TypeKind_kBoolWrapper:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_TypeKind_kIntWrapper:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_TypeKind_kUintWrapper:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_TypeKind_kDoubleWrapper:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_TypeKind_kStringWrapper:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_TypeKind_kBytesWrapper:
      return true;
    default:
      return false;
  }
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsType(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kType;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsTypeParam(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kTypeParam;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsFunction(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kFunction;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsEnum(const cel_Type* cel_nonnull type) {
  return cel_Type_Kind(type) == cel_TypeKind_kEnum;
}

// cel_ListType

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_ListType* cel_nullable cel_ListType_New(
    const cel_Type* cel_nonnull element, cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_Type* cel_nonnull
cel_ListType_Element(const cel_ListType* cel_nonnull type);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_ListType_Equals(const cel_ListType* cel_nonnull lhs,
                                    const cel_ListType* cel_nonnull rhs);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_ListType* cel_nullability_unknown
cel_ListType_ConstDownCast(const cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsList(type));

  return (const cel_ListType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_ListType* cel_nullability_unknown
cel_ListType_MutableDownCast(cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsList(type));

  return (cel_ListType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Type* cel_nullability_unknown
cel_ListType_ConstUpCast(const cel_ListType* cel_nullability_unknown type) {
  return (const cel_Type*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Type* cel_nullability_unknown
cel_ListType_MutableUpCast(cel_ListType* cel_nullability_unknown type) {
  return (cel_Type*)type;
}

// cel_MapType

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_MapType* cel_nullable cel_MapType_New(
    const cel_Type* cel_nonnull key, const cel_Type* cel_nonnull value,
    cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_Type* cel_nonnull
cel_MapType_Key(const cel_MapType* cel_nonnull type);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_Type* cel_nonnull
cel_MapType_Value(const cel_MapType* cel_nonnull type);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_MapType_Equals(const cel_MapType* cel_nonnull lhs,
                                   const cel_MapType* cel_nonnull rhs);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_MapType* cel_nullability_unknown
cel_MapType_ConstDownCast(const cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsMap(type));

  return (const cel_MapType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_MapType* cel_nullability_unknown
cel_MapType_MutableDownCast(cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsMap(type));

  return (cel_MapType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Type* cel_nullability_unknown
cel_MapType_ConstUpCast(const cel_MapType* cel_nullability_unknown type) {
  return (const cel_Type*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Type* cel_nullability_unknown
cel_MapType_MutableUpCast(cel_MapType* cel_nullability_unknown type) {
  return (cel_Type*)type;
}

// cel_StructType

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_StructType* cel_nullable
cel_StructType_New(cel_StringView name, cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView
cel_StructType_Name(const cel_StructType* cel_nonnull type);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_StructType_Equals(const cel_StructType* cel_nonnull lhs,
                                      const cel_StructType* cel_nonnull rhs);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_StructType* cel_nullability_unknown
cel_StructType_ConstDownCast(const cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsStruct(type));

  return (const cel_StructType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StructType* cel_nullability_unknown
cel_StructType_MutableDownCast(cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsStruct(type));

  return (cel_StructType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Type* cel_nullability_unknown
cel_StructType_ConstUpCast(const cel_StructType* cel_nullability_unknown type) {
  return (const cel_Type*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Type* cel_nullability_unknown
cel_StructType_MutableUpCast(cel_StructType* cel_nullability_unknown type) {
  return (cel_Type*)type;
}

// cel_EnumType

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_EnumType* cel_nullable
cel_EnumType_New(cel_StringView name, cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView
cel_EnumType_Name(const cel_EnumType* cel_nonnull type);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_EnumType_Equals(const cel_EnumType* cel_nonnull lhs,
                                    const cel_EnumType* cel_nonnull rhs);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_EnumType* cel_nullability_unknown
cel_EnumType_ConstDownCast(const cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsEnum(type));

  return (const cel_EnumType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_EnumType* cel_nullability_unknown
cel_EnumType_MutableDownCast(cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsEnum(type));

  return (cel_EnumType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Type* cel_nullability_unknown
cel_EnumType_ConstUpCast(const cel_EnumType* cel_nullability_unknown type) {
  return (const cel_Type*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Type* cel_nullability_unknown
cel_EnumType_MutableUpCast(cel_EnumType* cel_nullability_unknown type) {
  return (cel_Type*)type;
}

// cel_OpaqueType

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_OpaqueType* cel_nullable
cel_OpaqueType_New(cel_StringView name, size_t params_len,
                   const cel_Type * cel_nullability_unknown *
                       cel_nullability_unknown * cel_nullability_unknown params,
                   cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView
cel_OpaqueType_Name(const cel_OpaqueType* cel_nonnull type);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_Type* const cel_nonnull* cel_nullability_unknown
cel_OpaqueType_Params(const cel_OpaqueType* cel_nonnull type,
                      size_t* cel_nullable size);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_OpaqueType_IsOptional(
    const cel_OpaqueType* cel_nonnull type) {
  CEL_ASSERT_NOT_NULL(type);

  if (!cel_StringView_Equals(cel_OpaqueType_Name(type),
                             cel_StringView_FromString("optional_type"))) {
    return false;
  }
  size_t params_len;
  cel_OpaqueType_Params(type, &params_len);
  return params_len == 1;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Type_IsOptional(const cel_Type* cel_nonnull type) {
  return cel_Type_IsOpaque(type) &&
         cel_OpaqueType_IsOptional((const cel_OpaqueType*)type);
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_OpaqueType_Equals(const cel_OpaqueType* cel_nonnull lhs,
                                      const cel_OpaqueType* cel_nonnull rhs);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_OpaqueType* cel_nullability_unknown
cel_OpaqueType_ConstDownCast(const cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsOpaque(type));

  return (const cel_OpaqueType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_OpaqueType* cel_nullability_unknown
cel_OpaqueType_MutableDownCast(cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsOpaque(type));

  return (cel_OpaqueType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Type* cel_nullability_unknown
cel_OpaqueType_ConstUpCast(const cel_OpaqueType* cel_nullability_unknown type) {
  return (const cel_Type*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Type* cel_nullability_unknown
cel_OpaqueType_MutableUpCast(cel_OpaqueType* cel_nullability_unknown type) {
  return (cel_Type*)type;
}

// cel_OptionalType

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_OptionalType* cel_nullable cel_OptionalType_New(
    const cel_Type* cel_nonnull param, cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Type* cel_nonnull
cel_OptionalType_Param(const cel_OptionalType* cel_nonnull type) {
  return cel_OpaqueType_Params((const cel_OpaqueType*)type, cel_nullptr)[0];
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
static CEL_INLINE bool cel_OptionalType_Equals(
    const cel_OptionalType* cel_nonnull lhs,
    const cel_OptionalType* cel_nonnull rhs);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_OptionalType* cel_nullability_unknown
cel_OptionalType_ConstDownCast(const void* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsOptional((const cel_Type*)type));

  return (const cel_OptionalType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_OptionalType* cel_nullability_unknown
cel_OptionalType_MutableDownCast(void* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsOptional((cel_Type*)type));

  return (cel_OptionalType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Type* cel_nullability_unknown
cel_OptionalType_ConstUpCast(
    const cel_OptionalType* cel_nullability_unknown type) {
  return (const cel_Type*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Type* cel_nullability_unknown
cel_OptionalType_MutableUpCast(cel_OptionalType* cel_nullability_unknown type) {
  return (cel_Type*)type;
}

// cel_TypeType

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_TypeType* cel_nullable cel_TypeType_New(
    const cel_Type* cel_nonnull type, cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_Type* cel_nonnull
cel_TypeType_Type(const cel_TypeType* cel_nonnull type);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_TypeType_Equals(const cel_TypeType* cel_nonnull lhs,
                                    const cel_TypeType* cel_nonnull rhs);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_TypeType* cel_nullability_unknown
cel_TypeType_ConstDownCast(const cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsType(type));

  return (const cel_TypeType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_TypeType* cel_nullability_unknown
cel_TypeType_MutableDownCast(cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsType(type));

  return (cel_TypeType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Type* cel_nullability_unknown
cel_TypeType_ConstUpCast(const cel_TypeType* cel_nullability_unknown type) {
  return (const cel_Type*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Type* cel_nullability_unknown
cel_TypeType_MutableUpCast(cel_TypeType* cel_nullability_unknown type) {
  return (cel_Type*)type;
}

// cel_TypeParamType

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_TypeParamType* cel_nullable
cel_TypeParamType_New(cel_StringView name, cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView
cel_TypeParamType_Name(const cel_TypeParamType* cel_nonnull type);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_TypeParamType_Equals(
    const cel_TypeParamType* cel_nonnull lhs,
    const cel_TypeParamType* cel_nonnull rhs);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_TypeParamType* cel_nullability_unknown
cel_TypeParamType_ConstDownCast(const cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsTypeParam(type));

  return (const cel_TypeParamType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_TypeParamType* cel_nullability_unknown
cel_TypeParamType_MutableDownCast(cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsTypeParam(type));

  return (cel_TypeParamType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Type* cel_nullability_unknown
cel_TypeParamType_ConstUpCast(
    const cel_TypeParamType* cel_nullability_unknown type) {
  return (const cel_Type*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Type* cel_nullability_unknown
cel_TypeParamType_MutableUpCast(
    cel_TypeParamType* cel_nullability_unknown type) {
  return (cel_Type*)type;
}

// cel_FunctionType

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_FunctionType* cel_nullable
cel_FunctionType_New(const cel_Type* cel_nonnull result, size_t args_len,
                     const cel_Type * cel_nullability_unknown *
                         cel_nullability_unknown * cel_nullability_unknown args,
                     cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_Type* cel_nonnull
cel_FunctionType_Result(const cel_FunctionType* cel_nonnull type);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_Type* const cel_nonnull* cel_nullability_unknown
cel_FunctionType_Args(const cel_FunctionType* cel_nonnull type,
                      size_t* cel_nullable size);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_FunctionType_Equals(const cel_FunctionType* cel_nonnull lhs,
                                        const cel_FunctionType* cel_nonnull
                                            rhs);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_FunctionType* cel_nullability_unknown
cel_FunctionType_ConstDownCast(const cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsFunction(type));

  return (const cel_FunctionType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_FunctionType* cel_nullability_unknown
cel_FunctionType_MutableDownCast(cel_Type* cel_nullability_unknown type) {
  CEL_ASSERT(type == cel_nullptr || cel_Type_IsFunction(type));

  return (cel_FunctionType*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Type* cel_nullability_unknown
cel_FunctionType_ConstUpCast(
    const cel_FunctionType* cel_nullability_unknown type) {
  return (const cel_Type*)type;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Type* cel_nullability_unknown
cel_FunctionType_MutableUpCast(cel_FunctionType* cel_nullability_unknown type) {
  return (cel_Type*)type;
}

CEL_END_DECLS

#ifndef __cplusplus
#define cel_Type_UpCast(type)                                   \
  (_Generic((type),                                             \
       const cel_ListType*: cel_ListType_ConstUpCast,           \
       cel_ListType*: cel_ListType_MutableUpCast,               \
       const cel_MapType*: cel_MapType_ConstUpCast,             \
       cel_MapType*: cel_MapType_MutableUpCast,                 \
       const cel_StructType*: cel_StructType_ConstUpCast,       \
       cel_StructType*: cel_StructType_MutableUpCast,           \
       const cel_EnumType*: cel_EnumType_ConstUpCast,           \
       cel_EnumType*: cel_EnumType_MutableUpCast,               \
       const cel_OpaqueType*: cel_OpaqueType_ConstUpCast,       \
       cel_OpaqueType*: cel_OpaqueType_MutableUpCast,           \
       const cel_OptionalType*: cel_OptionalType_ConstUpCast,   \
       cel_OptionalType*: cel_OptionalType_MutableUpCast,       \
       const cel_TypeType*: cel_TypeType_ConstUpCast,           \
       cel_TypeType*: cel_TypeType_MutableUpCast,               \
       const cel_TypeParamType*: cel_TypeParamType_ConstUpCast, \
       cel_TypeParamType*: cel_TypeParamType_MutableUpCast,     \
       const cel_FunctionType*: cel_FunctionType_ConstUpCast,   \
       cel_FunctionType*: cel_FunctionType_MutableUpCast)((type)))
#define cel_OpaqueType_UpCast(type)                                   \
  (_Generic((type),                                                   \
       const cel_OptionalType*: (                                     \
                const cel_OpaqueType* cel_nullability_unknown)(type), \
       cel_OptionalType*: (cel_OpaqueType * cel_nullability_unknown)(type)))
#define cel_ListType_DownCast(type)                 \
  (_Generic((type),                                 \
       const cel_Type*: cel_ListType_ConstDownCast, \
       cel_Type*: cel_ListType_MutableDownCast)((type)))
#define cel_MapType_DownCast(type)                 \
  (_Generic((type),                                \
       const cel_Type*: cel_MapType_ConstDownCast, \
       cel_Type*: cel_MapType_MutableDownCast)((type)))
#define cel_StructType_DownCast(type)                 \
  (_Generic((type),                                   \
       const cel_Type*: cel_StructType_ConstDownCast, \
       cel_Type*: cel_StructType_MutableDownCast)((type)))
#define cel_EnumType_DownCast(type)                 \
  (_Generic((type),                                 \
       const cel_Type*: cel_EnumType_ConstDownCast, \
       cel_Type*: cel_EnumType_MutableDownCast)((type)))
#define cel_OpaqueType_DownCast(type)                 \
  (_Generic((type),                                   \
       const cel_Type*: cel_OpaqueType_ConstDownCast, \
       cel_Type*: cel_OpaqueType_MutableDownCast)((type)))
#define cel_OptionalType_DownCast(type)                       \
  (_Generic((type),                                           \
       const cel_Type*: cel_OptionalType_ConstDownCast,       \
       cel_Type*: cel_OptionalType_MutableDownCast,           \
       const cel_OpaqueType*: cel_OptionalType_ConstDownCast, \
       cel_OpaqueType*: cel_OptionalType_MutableDownCast)((type)))
#define cel_TypeType_DownCast(type)                 \
  (_Generic((type),                                 \
       const cel_Type*: cel_TypeType_ConstDownCast, \
       cel_Type*: cel_TypeType_MutableDownCast)((type)))
#define cel_TypeParamType_DownCast(type)                 \
  (_Generic((type),                                      \
       const cel_Type*: cel_TypeParamType_ConstDownCast, \
       cel_Type*: cel_TypeParamType_MutableDownCast)((type)))
#define cel_FunctionType_DownCast(type)                 \
  (_Generic((type),                                     \
       const cel_Type*: cel_FunctionType_ConstDownCast, \
       cel_Type*: cel_FunctionType_MutableDownCast)((type)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Type* cel_nullability_unknown
cel_Type_UpCast(const cel_ListType* cel_nullability_unknown type) {
  return cel_ListType_ConstUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Type* cel_nullability_unknown
cel_Type_UpCast(cel_ListType* cel_nullability_unknown type) {
  return cel_ListType_MutableUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Type* cel_nullability_unknown
cel_Type_UpCast(const cel_MapType* cel_nullability_unknown type) {
  return cel_MapType_ConstUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Type* cel_nullability_unknown
cel_Type_UpCast(cel_MapType* cel_nullability_unknown type) {
  return cel_MapType_MutableUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Type* cel_nullability_unknown
cel_Type_UpCast(const cel_StructType* cel_nullability_unknown type) {
  return cel_StructType_ConstUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Type* cel_nullability_unknown
cel_Type_UpCast(cel_StructType* cel_nullability_unknown type) {
  return cel_StructType_MutableUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Type* cel_nullability_unknown
cel_Type_UpCast(const cel_EnumType* cel_nullability_unknown type) {
  return cel_EnumType_ConstUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Type* cel_nullability_unknown
cel_Type_UpCast(cel_EnumType* cel_nullability_unknown type) {
  return cel_EnumType_MutableUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Type* cel_nullability_unknown
cel_Type_UpCast(const cel_OpaqueType* cel_nullability_unknown type) {
  return cel_OpaqueType_ConstUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Type* cel_nullability_unknown
cel_Type_UpCast(cel_OpaqueType* cel_nullability_unknown type) {
  return cel_OpaqueType_MutableUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Type* cel_nullability_unknown
cel_Type_UpCast(const cel_OptionalType* cel_nullability_unknown type) {
  return cel_OptionalType_ConstUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Type* cel_nullability_unknown
cel_Type_UpCast(cel_OptionalType* cel_nullability_unknown type) {
  return cel_OptionalType_MutableUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Type* cel_nullability_unknown
cel_Type_UpCast(const cel_TypeType* cel_nullability_unknown type) {
  return cel_TypeType_ConstUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Type* cel_nullability_unknown
cel_Type_UpCast(cel_TypeType* cel_nullability_unknown type) {
  return cel_TypeType_MutableUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Type* cel_nullability_unknown
cel_Type_UpCast(const cel_TypeParamType* cel_nullability_unknown type) {
  return cel_TypeParamType_ConstUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Type* cel_nullability_unknown
cel_Type_UpCast(cel_TypeParamType* cel_nullability_unknown type) {
  return cel_TypeParamType_MutableUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Type* cel_nullability_unknown
cel_Type_UpCast(const cel_FunctionType* cel_nullability_unknown type) {
  return cel_FunctionType_ConstUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Type* cel_nullability_unknown
cel_Type_UpCast(cel_FunctionType* cel_nullability_unknown type) {
  return cel_FunctionType_MutableUpCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_OpaqueType* cel_nullability_unknown
cel_OpaqueType_UpCast(const cel_OptionalType* cel_nullability_unknown type) {
  return (const cel_OpaqueType*)type;
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_OpaqueType* cel_nullability_unknown
cel_OpaqueType_UpCast(cel_OptionalType* cel_nullability_unknown type) {
  return (cel_OpaqueType*)type;
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_ListType* cel_nullability_unknown
cel_ListType_DownCast(const cel_Type* cel_nullability_unknown type) {
  return cel_ListType_ConstDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_ListType* cel_nullability_unknown
cel_ListType_DownCast(cel_Type* cel_nullability_unknown type) {
  return cel_ListType_MutableDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_MapType* cel_nullability_unknown
cel_MapType_DownCast(const cel_Type* cel_nullability_unknown type) {
  return cel_MapType_ConstDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_MapType* cel_nullability_unknown
cel_MapType_DownCast(cel_Type* cel_nullability_unknown type) {
  return cel_MapType_MutableDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_StructType* cel_nullability_unknown
cel_StructType_DownCast(const cel_Type* cel_nullability_unknown type) {
  return cel_StructType_ConstDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_StructType* cel_nullability_unknown
cel_StructType_DownCast(cel_Type* cel_nullability_unknown type) {
  return cel_StructType_MutableDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_EnumType* cel_nullability_unknown
cel_EnumType_DownCast(const cel_Type* cel_nullability_unknown type) {
  return cel_EnumType_ConstDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_EnumType* cel_nullability_unknown
cel_EnumType_DownCast(cel_Type* cel_nullability_unknown type) {
  return cel_EnumType_MutableDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_OpaqueType* cel_nullability_unknown
cel_OpaqueType_DownCast(const cel_Type* cel_nullability_unknown type) {
  return cel_OpaqueType_ConstDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_OpaqueType* cel_nullability_unknown
cel_OpaqueType_DownCast(cel_Type* cel_nullability_unknown type) {
  return cel_OpaqueType_MutableDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_OptionalType* cel_nullability_unknown
cel_OptionalType_DownCast(const cel_Type* cel_nullability_unknown type) {
  return cel_OptionalType_ConstDownCast(cel_OpaqueType_ConstDownCast(type));
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_OptionalType* cel_nullability_unknown
cel_OptionalType_DownCast(cel_Type* cel_nullability_unknown type) {
  return cel_OptionalType_MutableDownCast(cel_OpaqueType_MutableDownCast(type));
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_OptionalType* cel_nullability_unknown
cel_OptionalType_DownCast(const cel_OpaqueType* cel_nullability_unknown type) {
  return cel_OptionalType_ConstDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_OptionalType* cel_nullability_unknown
cel_OptionalType_DownCast(cel_OpaqueType* cel_nullability_unknown type) {
  return cel_OptionalType_MutableDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_TypeType* cel_nullability_unknown
cel_TypeType_DownCast(const cel_Type* cel_nullability_unknown type) {
  return cel_TypeType_ConstDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_TypeType* cel_nullability_unknown
cel_TypeType_DownCast(cel_Type* cel_nullability_unknown type) {
  return cel_TypeType_MutableDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_TypeParamType* cel_nullability_unknown
cel_TypeParamType_DownCast(const cel_Type* cel_nullability_unknown type) {
  return cel_TypeParamType_ConstDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_TypeParamType* cel_nullability_unknown
cel_TypeParamType_DownCast(cel_Type* cel_nullability_unknown type) {
  return cel_TypeParamType_MutableDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_FunctionType* cel_nullability_unknown
cel_FunctionType_DownCast(const cel_Type* cel_nullability_unknown type) {
  return cel_FunctionType_ConstDownCast(type);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_FunctionType* cel_nullability_unknown
cel_FunctionType_DownCast(cel_Type* cel_nullability_unknown type) {
  return cel_FunctionType_MutableDownCast(type);
}
#endif

#ifdef __cplusplus

inline bool operator==(const cel_Type& lhs, const cel_Type& rhs) {
  return cel_Type_Equals(&lhs, &rhs);
}

inline bool operator!=(const cel_Type& lhs, const cel_Type& rhs) {
  return !operator==(lhs, rhs);
}

inline bool operator==(const cel_StructType& lhs, const cel_StructType& rhs) {
  return cel_StructType_Equals(&lhs, &rhs);
}

inline bool operator!=(const cel_StructType& lhs, const cel_StructType& rhs) {
  return !operator==(lhs, rhs);
}

#endif

#endif  // THIRD_PARTY_CEL_C_TYPE_H_
