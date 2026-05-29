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

#ifndef THIRD_PARTY_CEL_C_DECL_H_
#define THIRD_PARTY_CEL_C_DECL_H_

#include <stdbool.h>
#include <stddef.h>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/constant.h"
#include "cel-c/function_scope.h"
#include "cel-c/string_view.h"
#include "cel-c/type.h"

CEL_BEGIN_DECLS

typedef enum CEL_ATTRIBUTE_OPEN_ENUM {
  cel_DeclKind_kIdent = 1,
  cel_DeclKind_kFunction
} cel_DeclKind;

typedef struct cel_Decl cel_Decl;
typedef struct cel_IdentDecl cel_IdentDecl;
typedef struct cel_FunctionDecl cel_FunctionDecl;
typedef struct cel_FunctionOverloadDecl cel_FunctionOverloadDecl;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_DeclKind cel_Decl_Kind(const cel_Decl* cel_nonnull decl);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Decl_IsIdent(const cel_Decl* cel_nonnull decl) {
  return cel_Decl_Kind(decl) == cel_DeclKind_kIdent;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Decl_IsFunction(const cel_Decl* cel_nonnull decl) {
  return cel_Decl_Kind(decl) == cel_DeclKind_kFunction;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_Decl_Name(const cel_Decl* cel_nonnull decl);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_Decl_Doc(const cel_Decl* cel_nonnull decl);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Decl_SetDoc(cel_Decl* cel_nonnull decl, cel_StringView doc);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Decl* cel_nullability_unknown
cel_IdentDecl_ConstUpCast(const cel_IdentDecl* cel_nullability_unknown decl) {
  return (const cel_Decl*)decl;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Decl* cel_nullability_unknown
cel_IdentDecl_MutableUpCast(cel_IdentDecl* cel_nullability_unknown decl) {
  return (cel_Decl*)decl;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_IdentDecl* cel_nullability_unknown
cel_IdentDecl_ConstDownCast(const cel_Decl* cel_nullability_unknown decl) {
  CEL_ASSERT(decl == cel_nullptr || cel_Decl_IsIdent(decl));

  return (const cel_IdentDecl*)decl;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_IdentDecl* cel_nullability_unknown
cel_IdentDecl_MutableDownCast(cel_Decl* cel_nullability_unknown decl) {
  CEL_ASSERT(decl == cel_nullptr || cel_Decl_IsIdent(decl));

  return (cel_IdentDecl*)decl;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_IdentDecl* cel_nullable
cel_IdentDecl_New(cel_StringView name, cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_Type* cel_nonnull
cel_IdentDecl_Type(const cel_IdentDecl* cel_nonnull decl);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_IdentDecl_SetType(cel_IdentDecl* cel_nonnull decl,
                                      const cel_Type* cel_nonnull type);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_Constant* cel_nonnull
cel_IdentDecl_Value(const cel_IdentDecl* cel_nonnull decl);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_Constant* cel_nonnull
cel_IdentDecl_MutableValue(cel_IdentDecl* cel_nonnull decl);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_IdentDecl_HasValue(
    const cel_IdentDecl* cel_nonnull decl) {
  return cel_Constant_Kind(cel_IdentDecl_Value(decl)) !=
         cel_ConstantKind_kUnspecified;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Decl* cel_nullability_unknown
cel_FunctionDecl_ConstUpCast(
    const cel_FunctionDecl* cel_nullability_unknown decl) {
  return (const cel_Decl*)decl;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Decl* cel_nullability_unknown
cel_FunctionDecl_MutableUpCast(cel_FunctionDecl* cel_nullability_unknown decl) {
  return (cel_Decl*)decl;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_FunctionDecl* cel_nullability_unknown
cel_FunctionDecl_ConstDownCast(const cel_Decl* cel_nullability_unknown decl) {
  CEL_ASSERT(decl == cel_nullptr || cel_Decl_IsFunction(decl));

  return (const cel_FunctionDecl*)decl;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_FunctionDecl* cel_nullability_unknown
cel_FunctionDecl_MutableDownCast(cel_Decl* cel_nullability_unknown decl) {
  CEL_ASSERT(decl == cel_nullptr || cel_Decl_IsFunction(decl));

  return (cel_FunctionDecl*)decl;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_FunctionDecl* cel_nullable
cel_FunctionDecl_New(cel_StringView name, cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN size_t cel_FunctionDecl_Overloads(
    const cel_FunctionDecl* cel_nonnull decl,
    cel_FunctionOverloadDecl * cel_nullable * cel_nullable head,
    cel_FunctionOverloadDecl * cel_nullable * cel_nullable tail);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_FunctionDecl_AddOverload(
    cel_FunctionDecl* cel_nonnull decl,
    cel_FunctionOverloadDecl* cel_nonnull overload);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_FunctionOverloadDecl* cel_nullable cel_FunctionOverloadDecl_New(
    cel_StringView id, cel_FunctionScope scope,
    const cel_FunctionType* cel_nonnull type, cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView
cel_FunctionOverloadDecl_Id(const cel_FunctionOverloadDecl* cel_nonnull decl);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView
cel_FunctionOverloadDecl_Doc(const cel_FunctionOverloadDecl* cel_nonnull decl);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_FunctionOverloadDecl_SetDoc(
    cel_FunctionOverloadDecl* cel_nonnull decl, cel_StringView doc);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_FunctionScope cel_FunctionOverloadDecl_Scope(
    const cel_FunctionOverloadDecl* cel_nonnull decl);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_FunctionDecl* cel_nullable cel_FunctionOverloadDecl_Function(
    const cel_FunctionOverloadDecl* cel_nonnull decl);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_FunctionType* cel_nonnull
cel_FunctionOverloadDecl_Type(const cel_FunctionOverloadDecl* cel_nonnull decl);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_FunctionOverloadDecl* cel_nullable
cel_FunctionOverloadDecl_Prev(const cel_FunctionOverloadDecl* cel_nonnull decl);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_FunctionOverloadDecl* cel_nullable
cel_FunctionOverloadDecl_Next(const cel_FunctionOverloadDecl* cel_nonnull decl);

CEL_END_DECLS

#ifndef __cplusplus
#define cel_Decl_UpCast(decl)                                 \
  (_Generic((decl),                                           \
       const cel_IdentDecl*: cel_IdentDecl_ConstUpCast,       \
       cel_IdentDecl*: cel_IdentDecl_MutableUpCast,           \
       const cel_FunctionDecl*: cel_FunctionDecl_ConstUpCast, \
       cel_FunctionDecl*: cel_FunctionDecl_MutableUpCast)((decl)))
#define cel_IdentDecl_DownCast(decl)                 \
  (_Generic((decl),                                  \
       const cel_Decl*: cel_IdentDecl_ConstDownCast, \
       cel_Decl*: cel_IdentDecl_MutableDownCast)((decl)))
#define cel_FunctionDecl_DownCast(decl)                 \
  (_Generic((decl),                                     \
       const cel_Decl*: cel_FunctionDecl_ConstDownCast, \
       cel_Decl*: cel_FunctionDecl_MutableDownCast)((decl)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Decl* cel_nullability_unknown
cel_Decl_UpCast(const cel_IdentDecl* cel_nullability_unknown decl) {
  return cel_IdentDecl_ConstUpCast(decl);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Decl* cel_nullability_unknown
cel_Decl_UpCast(cel_IdentDecl* cel_nullability_unknown decl) {
  return cel_IdentDecl_MutableUpCast(decl);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Decl* cel_nullability_unknown
cel_Decl_UpCast(const cel_FunctionDecl* cel_nullability_unknown decl) {
  return cel_FunctionDecl_ConstUpCast(decl);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Decl* cel_nullability_unknown
cel_Decl_UpCast(cel_FunctionDecl* cel_nullability_unknown decl) {
  return cel_FunctionDecl_MutableUpCast(decl);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_IdentDecl* cel_nullability_unknown
cel_IdentDecl_DownCast(const cel_Decl* cel_nullability_unknown decl) {
  return cel_IdentDecl_ConstDownCast(decl);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_IdentDecl* cel_nullability_unknown
cel_IdentDecl_DownCast(cel_Decl* cel_nullability_unknown decl) {
  return cel_IdentDecl_MutableDownCast(decl);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_FunctionDecl* cel_nullability_unknown
cel_FunctionDecl_DownCast(const cel_Decl* cel_nullability_unknown decl) {
  return cel_FunctionDecl_ConstDownCast(decl);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_FunctionDecl* cel_nullability_unknown
cel_FunctionDecl_DownCast(cel_Decl* cel_nullability_unknown decl) {
  return cel_FunctionDecl_MutableDownCast(decl);
}
#endif

#endif  // THIRD_PARTY_CEL_C_DECL_H_
