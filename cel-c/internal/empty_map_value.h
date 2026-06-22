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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_EMPTY_MAP_VALUE_H_
#define THIRD_PARTY_CEL_C_INTERNAL_EMPTY_MAP_VALUE_H_

#include "cel-c/assert.h"
#include "cel-c/internal/config.h"
#include "cel-c/value.h"

CEL_BEGIN_DECLS

extern const cel_MapValueVTable _cel_EmptyMapValueVTable;

extern cel_MapValueIterator _cel_EmptyMapValueIterator;

static CEL_INLINE void _cel_EmptyMapValue_Set(
    cel_MapValue* cel_nonnull map_value) {
  CEL_ASSERT_NOT_NULL(map_value);

  map_value->vtable = &_cel_EmptyMapValueVTable;
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_EMPTY_MAP_VALUE_H_
