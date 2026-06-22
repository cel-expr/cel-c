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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_MUTABLE_LIST_VALUE_H_
#define THIRD_PARTY_CEL_C_INTERNAL_MUTABLE_LIST_VALUE_H_

#include <stdint.h>
#include <string.h>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/internal/config.h"
#include "cel-c/value.h"

CEL_BEGIN_DECLS

extern const cel_ListValueVTable _cel_MutableListValueVTable;

static CEL_INLINE void _cel_MutableListValue_Set(
    cel_ListValue* cel_nonnull list_value) {
  CEL_ASSERT_NOT_NULL(list_value);

  list_value->vtable = &_cel_MutableListValueVTable;
  memset(&list_value->content, 0, sizeof(list_value->content));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ListValue_IsMutable(
    const cel_ListValue* cel_nonnull list_value) {
  CEL_ASSERT_NOT_NULL(list_value);

  return list_value->vtable == &_cel_MutableListValueVTable;
}

CEL_ATTRIBUTE_NOTHROW
bool _cel_MutableListValue_Reserve(cel_ListValue* cel_nonnull list_value,
                                   uint32_t size, cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
cel_Value* cel_nullable
_cel_MutableListValue_AddN(cel_ListValue* cel_nonnull list_value, uint32_t size,
                           cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Value* cel_nullable _cel_MutableListValue_Add(
    cel_ListValue* cel_nonnull list_value, cel_Arena* cel_nonnull arena) {
  return _cel_MutableListValue_AddN(list_value, 1, arena);
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_MUTABLE_LIST_VALUE_H_
