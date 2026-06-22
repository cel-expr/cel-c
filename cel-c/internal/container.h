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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_CONTAINER_H_
#define THIRD_PARTY_CEL_C_INTERNAL_CONTAINER_H_

#include <stddef.h>

#include "cel-c/alloc.h"
#include "cel-c/assert.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/string.h"
#include "cel-c/string_view.h"

CEL_BEGIN_DECLS

typedef struct {
  _cel_String name;
  size_t count;
} _cel_Container;

typedef struct {
  cel_StringView name;
  size_t size;
} _cel_ContainerIterator;

static CEL_INLINE void _cel_Container_Construct(
    _cel_Container* cel_nonnull container) {
  CEL_ASSERT_NOT_NULL(container);

  _cel_String_Construct(&container->name);
  container->count = 0;
}

static CEL_INLINE void _cel_Container_Destruct(
    _cel_Container* cel_nonnull container, cel_Allocator* cel_nonnull alloc) {
  CEL_ASSERT_NOT_NULL(container);
  CEL_ASSERT_NOT_NULL(alloc);

  _cel_String_Destruct(&container->name, alloc);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_Container_Empty(
    const _cel_Container* cel_nonnull container) {
  return container->count == 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_Container_Count(const _cel_Container* cel_nonnull container) {
  return container->count;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_Container_Update(
    _cel_Container* cel_nonnull container, cel_StringView name,
    cel_Allocator* cel_nonnull alloc) {
  CEL_ASSERT_NOT_NULL(container);
  CEL_ASSERT_NOT_NULL(alloc);

  _cel_String_Clear(&container->name);
  container->count = 0;
  _cel_String_Reserve(&container->name, alloc, cel_StringView_Size(name));

  cel_StringViewTokenizer tokenizer =
      cel_StringView_Tokenize(name, cel_StringView_FromString("."));
  cel_StringView token;
  while (cel_StringViewTokenizer_Next(&tokenizer, &token)) {
    if (cel_StringView_Empty(token)) {
      continue;
    }
    ++container->count;
    if (!_cel_String_Empty(&container->name)) {
      if (!_cel_String_PushBack(&container->name, alloc, '.')) {
        return false;
      }
    }
    if (!_cel_String_Append(&container->name, alloc, token)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_ContainerIterator
_cel_Container_Iterate(const _cel_Container* cel_nonnull container) {
  CEL_ASSERT_NOT_NULL(container);

  _cel_ContainerIterator iter;
  iter.name = _cel_String_ToStringView(&container->name);
  iter.size = container->count;
  return iter;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ContainerIterator_HasNext(
    const _cel_ContainerIterator* cel_nonnull iter) {
  CEL_ASSERT_NOT_NULL(iter);

  return iter->size > 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
_cel_ContainerIterator_Next(_cel_ContainerIterator* cel_nonnull iter) {
  CEL_ASSERT_NOT_NULL(iter);
  CEL_ASSERT_GT(iter->size, 0);

  cel_StringView name = iter->name;
  --iter->size;
  const char* pos =
      cel_StringView_FindLast(iter->name, cel_StringView_FromString("."));
  if (pos == cel_nullptr) {
    CEL_ASSERT_EQ(iter->size, 0);
    pos = cel_StringView_Data(iter->name);
  }
  cel_StringView_RemoveSuffix(&iter->name, (cel_StringView_Data(iter->name) +
                                            cel_StringView_Size(iter->name)) -
                                               pos);
  return name;
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_CONTAINER_H_
