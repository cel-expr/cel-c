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

#include "cel-c/ref.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/constant.h"
#include "cel-c/string_view.h"

struct cel_IdentRef {
  cel_StringView name;
  cel_RefKind kind;

  cel_Constant value;
};

struct cel_FunctionRef {
  cel_StringView name;
  cel_RefKind kind;

  cel_FunctionOverloadRef* head;
  cel_FunctionOverloadRef* tail;
  size_t overloads;
};

struct cel_FunctionOverloadRef {
  cel_FunctionOverloadRef* prev;
  cel_FunctionOverloadRef* next;
  cel_StringView id;
#ifndef NDEBUG
  const cel_FunctionRef* func;
#endif
};

struct cel_Ref {
  union {
    struct {
      cel_StringView name;
      cel_RefKind kind;
    };
    cel_IdentRef variable_ref;
    cel_FunctionRef function_ref;
  };
};

cel_RefKind cel_Ref_Kind(const cel_Ref* cel_nonnull ref) {
  CEL_ASSERT_NOT_NULL(ref);

  return ref->kind;
}

cel_StringView cel_Ref_Name(const cel_Ref* cel_nonnull ref) {
  CEL_ASSERT_NOT_NULL(ref);

  return ref->name;
}

cel_IdentRef* cel_nullable cel_IdentRef_New(cel_StringView name,
                                            cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(arena);

  cel_IdentRef* ref =
      (cel_IdentRef*)cel_Arena_Malloc(arena, sizeof(cel_IdentRef), cel_nullptr);
  if (CEL_LIKELY(ref != cel_nullptr)) {
    memset(ref, 0, sizeof(*ref));
    ref->name = name;
    ref->kind = cel_RefKind_kIdent;
  }
  return ref;
}

const cel_Constant* cel_nonnull
cel_IdentRef_Value(const cel_IdentRef* cel_nonnull ref) {
  CEL_ASSERT_NOT_NULL(ref);

  return &ref->value;
}

cel_FunctionRef* cel_nullable
cel_FunctionRef_New(cel_StringView name, cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(arena);

  cel_FunctionRef* ref = (cel_FunctionRef*)cel_Arena_Malloc(
      arena, sizeof(cel_FunctionRef), cel_nullptr);
  if (CEL_LIKELY(ref != cel_nullptr)) {
    memset(ref, 0, sizeof(*ref));
    ref->name = name;
    ref->kind = cel_RefKind_kFunction;
  }
  return ref;
}

size_t cel_FunctionRef_Overloads(const cel_FunctionRef* cel_nonnull ref,
                                 cel_FunctionOverloadRef * cel_nullable *
                                     cel_nullable head,
                                 cel_FunctionOverloadRef * cel_nullable *
                                     cel_nullable tail) {
  CEL_ASSERT_NOT_NULL(ref);

  if (head != cel_nullptr) {
    *head = ref->head;
  }
  if (tail != cel_nullptr) {
    *tail = ref->tail;
  }
  return ref->overloads;
}

bool cel_FunctionRef_AppendOverload(cel_FunctionRef* cel_nonnull ref,
                                    cel_FunctionOverloadRef* cel_nonnull
                                        overload) {
  CEL_ASSERT_NOT_NULL(ref);
  CEL_ASSERT_NOT_NULL(overload);
#ifndef NDEBUG
  CEL_ASSERT_NULL(overload->func);
#endif

  cel_FunctionOverloadRef* head = ref->head;
  for (; head != cel_nullptr; head = head->next) {
    if (cel_StringView_Equals(head->id, overload->id)) {
      return false;
    }
  }

  overload->prev = ref->tail;
  if (ref->tail != cel_nullptr) {
    ref->tail->next = overload;
  } else {
    ref->head = overload;
  }
  ref->tail = overload;
  ++ref->overloads;

#ifndef NDEBUG
  overload->func = ref;
#endif
  return true;
}

cel_FunctionOverloadRef* cel_nullable
cel_FunctionOverloadRef_New(cel_StringView id, cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(arena);

  cel_FunctionOverloadRef* ref = (cel_FunctionOverloadRef*)cel_Arena_Malloc(
      arena, sizeof(cel_FunctionOverloadRef), cel_nullptr);
  if (CEL_LIKELY(ref != cel_nullptr)) {
    memset(ref, 0, sizeof(*ref));
    ref->id = id;
  }
  return ref;
}

cel_StringView cel_FunctionOverloadRef_Id(
    const cel_FunctionOverloadRef* cel_nonnull ref) {
  CEL_ASSERT_NOT_NULL(ref);

  return ref->id;
}

cel_FunctionOverloadRef* cel_nullable
cel_FunctionOverloadRef_Prev(const cel_FunctionOverloadRef* cel_nonnull ref) {
  CEL_ASSERT_NOT_NULL(ref);

  return ref->prev;
}

cel_FunctionOverloadRef* cel_nullable
cel_FunctionOverloadRef_Next(const cel_FunctionOverloadRef* cel_nonnull ref) {
  CEL_ASSERT_NOT_NULL(ref);

  return ref->next;
}
