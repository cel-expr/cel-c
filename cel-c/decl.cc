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

#include "cel-c/decl.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/constant.h"
#include "cel-c/function_scope.h"
#include "cel-c/string_view.h"
#include "cel-c/type.h"
#include "cel-c/type_kind.h"

struct cel_IdentDecl {
  cel_StringView name;
  cel_DeclKind kind;
  cel_StringView doc;

  const cel_Type* cel_nonnull type;
  cel_Constant value;
};

struct cel_FunctionDecl {
  cel_StringView name;
  cel_DeclKind kind;
  cel_StringView doc;

  cel_FunctionOverloadDecl* cel_nullable head;
  cel_FunctionOverloadDecl* cel_nullable tail;
  size_t overloads;
};

struct cel_FunctionOverloadDecl {
  cel_FunctionOverloadDecl* cel_nullable prev;
  cel_FunctionOverloadDecl* cel_nullable next;
  cel_FunctionDecl* cel_nullable function;
  cel_StringView id;
  const cel_FunctionType* cel_nonnull type;
  cel_FunctionScope scope;
  cel_StringView doc;
};

struct cel_Decl {
  union {
    struct {
      cel_StringView name;
      cel_DeclKind kind;
      cel_StringView doc;
    };
    cel_IdentDecl ident_decl;
    cel_FunctionDecl function_decl;
  };
};

extern "C" cel_DeclKind cel_Decl_Kind(const cel_Decl* cel_nonnull decl) {
  CEL_ASSERT_NOT_NULL(decl);

  return decl->kind;
}

extern "C" cel_StringView cel_Decl_Name(const cel_Decl* cel_nonnull decl) {
  CEL_ASSERT_NOT_NULL(decl);

  return decl->name;
}

extern "C" cel_StringView cel_Decl_Doc(const cel_Decl* cel_nonnull decl) {
  CEL_ASSERT_NOT_NULL(decl);

  return decl->doc;
}

extern "C" void cel_Decl_SetDoc(cel_Decl* cel_nonnull decl,
                                cel_StringView doc) {
  CEL_ASSERT_NOT_NULL(decl);

  decl->doc = doc;
}

extern "C" cel_IdentDecl* cel_nullable
cel_IdentDecl_New(cel_StringView name, cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(arena);

  cel_IdentDecl* decl = (cel_IdentDecl*)cel_Arena_Malloc(
      arena, sizeof(cel_IdentDecl), cel_nullptr);
  if (CEL_LIKELY(decl != cel_nullptr)) {
    memset(decl, 0, sizeof(*decl));
    decl->name = name;
    decl->kind = cel_DeclKind_kIdent;
    decl->type = cel_DynType;
  }
  return decl;
}

extern "C" const cel_Type* cel_nonnull
cel_IdentDecl_Type(const cel_IdentDecl* cel_nonnull decl) {
  CEL_ASSERT_NOT_NULL(decl);

  return decl->type;
}

extern "C" void cel_IdentDecl_SetType(cel_IdentDecl* cel_nonnull decl,
                                      const cel_Type* cel_nonnull type) {
  CEL_ASSERT_NOT_NULL(decl);

  decl->type = type;
}

extern "C" const cel_Constant* cel_nonnull
cel_IdentDecl_Value(const cel_IdentDecl* cel_nonnull decl) {
  CEL_ASSERT_NOT_NULL(decl);

  return &decl->value;
}

extern "C" cel_Constant* cel_nonnull
cel_IdentDecl_MutableValue(cel_IdentDecl* cel_nonnull decl) {
  CEL_ASSERT_NOT_NULL(decl);

  return &decl->value;
}

extern "C" cel_FunctionDecl* cel_nullable
cel_FunctionDecl_New(cel_StringView name, cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(arena);

  cel_FunctionDecl* decl = (cel_FunctionDecl*)cel_Arena_Malloc(
      arena, sizeof(cel_FunctionDecl), cel_nullptr);
  if (CEL_LIKELY(decl != cel_nullptr)) {
    memset(decl, 0, sizeof(*decl));
    decl->name = name;
    decl->kind = cel_DeclKind_kFunction;
  }
  return decl;
}

extern "C" size_t cel_FunctionDecl_Overloads(
    const cel_FunctionDecl* cel_nonnull decl,
    cel_FunctionOverloadDecl * cel_nullable * cel_nullable head,
    cel_FunctionOverloadDecl * cel_nullable * cel_nullable tail) {
  CEL_ASSERT_NOT_NULL(decl);

  if (head != cel_nullptr) {
    *head = decl->head;
  }
  if (tail != cel_nullptr) {
    *tail = decl->tail;
  }
  return decl->overloads;
}

static bool _cel_IsTypeAssignable(const cel_Type* cel_nonnull to,
                                  const cel_Type* cel_nonnull from) {
  if (cel_Type_Equals(to, from)) {
    return true;
  }
  const cel_TypeKind to_kind = cel_Type_Kind(to);
  if (to_kind == cel_TypeKind_kDyn) {
    return true;
  }
  switch (to_kind) {
    case cel_TypeKind_kBoolWrapper:
      return _cel_IsTypeAssignable(cel_NullType, from) ||
             _cel_IsTypeAssignable(cel_BoolType, from);
    case cel_TypeKind_kIntWrapper:
      return _cel_IsTypeAssignable(cel_NullType, from) ||
             _cel_IsTypeAssignable(cel_IntType, from);
    case cel_TypeKind_kUintWrapper:
      return _cel_IsTypeAssignable(cel_NullType, from) ||
             _cel_IsTypeAssignable(cel_UintType, from);
    case cel_TypeKind_kDoubleWrapper:
      return _cel_IsTypeAssignable(cel_NullType, from) ||
             _cel_IsTypeAssignable(cel_DoubleType, from);
    case cel_TypeKind_kBytesWrapper:
      return _cel_IsTypeAssignable(cel_NullType, from) ||
             _cel_IsTypeAssignable(cel_BytesType, from);
    case cel_TypeKind_kStringWrapper:
      return _cel_IsTypeAssignable(cel_NullType, from) ||
             _cel_IsTypeAssignable(cel_StringType, from);
    default:
      break;
  }
  const cel_TypeKind from_kind = cel_Type_Kind(from);
  if (to_kind != from_kind ||
      !cel_StringView_Equals(cel_Type_Name(to), cel_Type_Name(from))) {
    return false;
  }
  size_t from_params_len;
  const cel_Type* const* from_params = cel_Type_Params(from, &from_params_len);
  size_t to_params_len;
  const cel_Type* const* to_params = cel_Type_Params(to, &to_params_len);
  if (from_params_len != to_params_len) {
    return false;
  }
  bool params_overlap = true;
  for (size_t i = 0; i < from_params_len && params_overlap; ++i) {
    params_overlap = _cel_IsTypeAssignable(from_params[i], to_params[i]);
  }
  return params_overlap;
}

static bool _cel_SignaturesOverlap(
    const cel_FunctionOverloadDecl* cel_nonnull lhs,
    const cel_FunctionOverloadDecl* cel_nonnull rhs) {
  if (cel_StringView_Equals(cel_FunctionOverloadDecl_Id(lhs),
                            cel_FunctionOverloadDecl_Id(rhs))) {
    return true;
  }
  if (cel_FunctionOverloadDecl_Scope(lhs) !=
      cel_FunctionOverloadDecl_Scope(rhs)) {
    return false;
  }
  const cel_FunctionType* lhs_type = cel_FunctionOverloadDecl_Type(lhs);
  const cel_FunctionType* rhs_type = cel_FunctionOverloadDecl_Type(rhs);
  size_t lhs_type_args_len;
  const cel_Type* const* lhs_type_args =
      cel_FunctionType_Args(lhs_type, &lhs_type_args_len);
  size_t rhs_type_args_len;
  const cel_Type* const* rhs_type_args =
      cel_FunctionType_Args(rhs_type, &rhs_type_args_len);
  if (lhs_type_args_len != rhs_type_args_len) {
    return false;
  }
  bool args_overlap = true;
  for (size_t i = 0; i < lhs_type_args_len && args_overlap; ++i) {
    args_overlap = _cel_IsTypeAssignable(lhs_type_args[i], rhs_type_args[i]);
  }
  return args_overlap;
}

extern "C" bool cel_FunctionDecl_AddOverload(
    cel_FunctionDecl* cel_nonnull decl,
    cel_FunctionOverloadDecl* cel_nonnull overload) {
  CEL_ASSERT_NOT_NULL(decl);
  CEL_ASSERT_NOT_NULL(overload);
  CEL_ASSERT_NULL(overload->function);

  cel_FunctionOverloadDecl* head = decl->head;
  for (; head != cel_nullptr; head = head->next) {
    if (_cel_SignaturesOverlap(head, overload)) {
      return false;
    }
  }

  overload->function = decl;
  overload->prev = decl->tail;
  if (decl->tail != cel_nullptr) {
    decl->tail->next = overload;
  } else {
    decl->head = overload;
  }
  decl->tail = overload;
  ++decl->overloads;
  return true;
}

extern "C" cel_FunctionOverloadDecl* cel_nullable cel_FunctionOverloadDecl_New(
    cel_StringView id, cel_FunctionScope scope,
    const cel_FunctionType* cel_nonnull type, cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_NOT_NULL(type);

  cel_FunctionOverloadDecl* decl = (cel_FunctionOverloadDecl*)cel_Arena_Malloc(
      arena, sizeof(cel_FunctionOverloadDecl), cel_nullptr);
  if (CEL_LIKELY(decl != cel_nullptr)) {
    memset(decl, 0, sizeof(*decl));
    decl->id = id;
    decl->type = type;
    decl->scope = scope;
  }
  return decl;
}

extern "C" cel_StringView cel_FunctionOverloadDecl_Id(
    const cel_FunctionOverloadDecl* cel_nonnull decl) {
  CEL_ASSERT_NOT_NULL(decl);

  return decl->id;
}

extern "C" cel_StringView cel_FunctionOverloadDecl_Doc(
    const cel_FunctionOverloadDecl* cel_nonnull decl) {
  CEL_ASSERT_NOT_NULL(decl);

  return decl->doc;
}

extern "C" void cel_FunctionOverloadDecl_SetDoc(
    cel_FunctionOverloadDecl* cel_nonnull decl, cel_StringView doc) {
  CEL_ASSERT_NOT_NULL(decl);

  decl->doc = doc;
}

extern "C" cel_FunctionScope cel_FunctionOverloadDecl_Scope(
    const cel_FunctionOverloadDecl* cel_nonnull decl) {
  CEL_ASSERT_NOT_NULL(decl);

  return decl->scope;
}

extern "C" cel_FunctionDecl* cel_nullable cel_FunctionOverloadDecl_Function(
    const cel_FunctionOverloadDecl* cel_nonnull decl) {
  CEL_ASSERT_NOT_NULL(decl);

  return decl->function;
}

extern "C" const cel_FunctionType* cel_nonnull cel_FunctionOverloadDecl_Type(
    const cel_FunctionOverloadDecl* cel_nonnull decl) {
  CEL_ASSERT_NOT_NULL(decl);

  return decl->type;
}

extern "C" cel_FunctionOverloadDecl* cel_nullable cel_FunctionOverloadDecl_Prev(
    const cel_FunctionOverloadDecl* cel_nonnull decl) {
  CEL_ASSERT_NOT_NULL(decl);

  return decl->prev;
}

extern "C" cel_FunctionOverloadDecl* cel_nullable cel_FunctionOverloadDecl_Next(
    const cel_FunctionOverloadDecl* cel_nonnull decl) {
  CEL_ASSERT_NOT_NULL(decl);

  return decl->next;
}
