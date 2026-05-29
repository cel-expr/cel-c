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

#ifndef THIRD_PARTY_CEL_C_REF_H_
#define THIRD_PARTY_CEL_C_REF_H_

#include <stdbool.h>
#include <stddef.h>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/constant.h"
#include "cel-c/string_view.h"

CEL_BEGIN_DECLS

typedef enum CEL_ATTRIBUTE_OPEN_ENUM {
  cel_RefKind_kIdent = 1,
  cel_RefKind_kFunction,
} cel_RefKind;

typedef struct cel_Ref cel_Ref;
typedef struct cel_IdentRef cel_IdentRef;
typedef struct cel_FunctionRef cel_FunctionRef;
typedef struct cel_FunctionOverloadRef cel_FunctionOverloadRef;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_RefKind cel_Ref_Kind(const cel_Ref* cel_nonnull ref);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Ref_IsIdent(const cel_Ref* cel_nonnull ref) {
  return cel_Ref_Kind(ref) == cel_RefKind_kIdent;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Ref_IsFunction(const cel_Ref* cel_nonnull ref) {
  return cel_Ref_Kind(ref) == cel_RefKind_kFunction;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_Ref_Name(const cel_Ref* cel_nonnull ref);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Ref* cel_nullability_unknown
cel_IdentRef_ConstUpCast(const cel_IdentRef* cel_nullability_unknown ref) {
  return (const cel_Ref*)ref;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Ref* cel_nullability_unknown
cel_IdentRef_MutableUpCast(cel_IdentRef* cel_nullability_unknown ref) {
  return (cel_Ref*)ref;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_IdentRef* cel_nullability_unknown
cel_IdentRef_ConstDownCast(const cel_Ref* cel_nullability_unknown ref) {
  CEL_ASSERT(ref == cel_nullptr || cel_Ref_IsIdent(ref));

  return (const cel_IdentRef*)ref;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_IdentRef* cel_nullability_unknown
cel_IdentRef_MutableDownCast(cel_Ref* cel_nullability_unknown ref) {
  CEL_ASSERT(ref == cel_nullptr || cel_Ref_IsIdent(ref));

  return (cel_IdentRef*)ref;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_IdentRef* cel_nullable
cel_IdentRef_New(cel_StringView name, cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_Constant* cel_nonnull
cel_IdentRef_Value(const cel_IdentRef* cel_nonnull ref);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Constant* cel_nonnull
cel_IdentRef_MutableValue(cel_IdentRef* cel_nonnull ref) {
  return (cel_Constant*)cel_IdentRef_Value(ref);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_IdentRef_HasValue(
    const cel_IdentRef* cel_nonnull ref) {
  return cel_Constant_Kind(cel_IdentRef_Value(ref)) !=
         cel_ConstantKind_kUnspecified;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Ref* cel_nullability_unknown
cel_FunctionRef_ConstUpCast(
    const cel_FunctionRef* cel_nullability_unknown ref) {
  return (const cel_Ref*)ref;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Ref* cel_nullability_unknown
cel_FunctionRef_MutableUpCast(cel_FunctionRef* cel_nullability_unknown ref) {
  return (cel_Ref*)ref;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_FunctionRef* cel_nullability_unknown
cel_FunctionRef_ConstDownCast(const cel_Ref* cel_nullability_unknown ref) {
  CEL_ASSERT(ref == cel_nullptr || cel_Ref_IsFunction(ref));

  return (const cel_FunctionRef*)ref;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_FunctionRef* cel_nullability_unknown
cel_FunctionRef_MutableDownCast(cel_Ref* cel_nullability_unknown ref) {
  CEL_ASSERT(ref == cel_nullptr || cel_Ref_IsFunction(ref));

  return (cel_FunctionRef*)ref;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_FunctionRef* cel_nullable
cel_FunctionRef_New(cel_StringView name, cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN size_t cel_FunctionRef_Overloads(
    const cel_FunctionRef* cel_nonnull ref,
    cel_FunctionOverloadRef * cel_nullable * cel_nullable head,
    cel_FunctionOverloadRef * cel_nullable * cel_nullable tail);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_FunctionRef_AppendOverload(
    cel_FunctionRef* cel_nonnull ref,
    cel_FunctionOverloadRef* cel_nonnull overload);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_FunctionOverloadRef* cel_nullable
cel_FunctionOverloadRef_New(cel_StringView id, cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView
cel_FunctionOverloadRef_Id(const cel_FunctionOverloadRef* cel_nonnull ref);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_FunctionOverloadRef* cel_nullable
cel_FunctionOverloadRef_Prev(const cel_FunctionOverloadRef* cel_nonnull ref);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_FunctionOverloadRef* cel_nullable
cel_FunctionOverloadRef_Next(const cel_FunctionOverloadRef* cel_nonnull ref);

CEL_END_DECLS

#ifndef __cplusplus
#define cel_Ref_UpCast(ref)                                 \
  (_Generic((ref),                                          \
       const cel_IdentRef*: cel_IdentRef_ConstUpCast,       \
       cel_IdentRef*: cel_IdentRef_MutableUpCast,           \
       const cel_FunctionRef*: cel_FunctionRef_ConstUpCast, \
       cel_FunctionRef*: cel_FunctionRef_MutableUpCast)((ref)))
#define cel_IdentRef_DownCast(ref)                 \
  (_Generic((ref),                                 \
       const cel_Ref*: cel_IdentRef_ConstDownCast, \
       cel_Ref*: cel_IdentRef_MutableDownCast)((ref)))
#define cel_FunctionRef_DownCast(ref)                 \
  (_Generic((ref),                                    \
       const cel_Ref*: cel_FunctionRef_ConstDownCast, \
       cel_Ref*: cel_FunctionRef_MutableDownCast)((ref)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Ref* cel_nullability_unknown
cel_Ref_UpCast(const cel_IdentRef* cel_nullability_unknown ref) {
  return cel_IdentRef_ConstUpCast(ref);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Ref* cel_nullability_unknown
cel_Ref_UpCast(cel_IdentRef* cel_nullability_unknown ref) {
  return cel_IdentRef_MutableUpCast(ref);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Ref* cel_nullability_unknown
cel_Ref_UpCast(const cel_FunctionRef* cel_nullability_unknown ref) {
  return cel_FunctionRef_ConstUpCast(ref);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Ref* cel_nullability_unknown
cel_Ref_UpCast(cel_FunctionRef* cel_nullability_unknown ref) {
  return cel_FunctionRef_MutableUpCast(ref);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_IdentRef* cel_nullability_unknown
cel_IdentRef_DownCast(const cel_Ref* cel_nullability_unknown ref) {
  return cel_IdentRef_ConstDownCast(ref);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_IdentRef* cel_nullability_unknown
cel_IdentRef_DownCast(cel_Ref* cel_nullability_unknown ref) {
  return cel_IdentRef_MutableDownCast(ref);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_FunctionRef* cel_nullability_unknown
cel_FunctionRef_DownCast(const cel_Ref* cel_nullability_unknown ref) {
  return cel_FunctionRef_ConstDownCast(ref);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_FunctionRef* cel_nullability_unknown
cel_FunctionRef_DownCast(cel_Ref* cel_nullability_unknown ref) {
  return cel_FunctionRef_MutableDownCast(ref);
}
#endif

#endif  // THIRD_PARTY_CEL_C_REF_H_
