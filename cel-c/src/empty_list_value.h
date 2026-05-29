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

#ifndef THIRD_PARTY_CEL_C_SRC_EMPTY_LIST_VALUE_H_
#define THIRD_PARTY_CEL_C_SRC_EMPTY_LIST_VALUE_H_

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/value.h"

CEL_BEGIN_DECLS

extern const cel_ListValueVTable _cel_EmptyListValueVTable;

extern cel_ListValueIterator _cel_EmptyListValueIterator;

static CEL_INLINE void _cel_EmptyListValue_Set(
    cel_ListValue* cel_nonnull list_value) {
  CEL_ASSERT_NOT_NULL(list_value);

  list_value->vtable = &_cel_EmptyListValueVTable;
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_SRC_EMPTY_LIST_VALUE_H_
