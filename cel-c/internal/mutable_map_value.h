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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_MUTABLE_MAP_VALUE_H_
#define THIRD_PARTY_CEL_C_INTERNAL_MUTABLE_MAP_VALUE_H_

#include <stdint.h>
#include <string.h>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/value.h"

CEL_BEGIN_DECLS

extern const cel_MapValueVTable _cel_MutableMapValueVTable;

static CEL_INLINE void _cel_MutableMapValue_Set(
    cel_MapValue* cel_nonnull map_value) {
  CEL_ASSERT_NOT_NULL(map_value);

  map_value->vtable = &_cel_MutableMapValueVTable;
  memset(&map_value->content, 0, sizeof(map_value->content));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_MapValue_IsMutable(
    const cel_MapValue* cel_nonnull map_value) {
  CEL_ASSERT_NOT_NULL(map_value);

  return map_value->vtable == &_cel_MutableMapValueVTable;
}

CEL_ATTRIBUTE_NOTHROW
bool _cel_MutableMapValue_Reserve(cel_MapValue* cel_nonnull map_value,
                                  uint32_t size, cel_Arena* cel_nonnull arena);

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM {
  _cel_MutableMapValueInsertResult_kInserted = 0,
  _cel_MutableMapValueInsertResult_kReplaced,
  _cel_MutableMapValueInsertResult_kOutOfMemory,
} _cel_MutableMapValueInsertResult;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
_cel_MutableMapValueInsertResult _cel_MutableMapValue_Insert(
    cel_MapValue* cel_nonnull map_value, const cel_MapValueKey* cel_nonnull key,
    cel_MapValueKey* cel_nullable* cel_nullable out_key,
    cel_Value* cel_nullable* cel_nonnull out_value,
    cel_Arena* cel_nonnull arena);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_MUTABLE_MAP_VALUE_H_
