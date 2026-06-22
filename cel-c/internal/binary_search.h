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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_BINARY_SEARCH_H_
#define THIRD_PARTY_CEL_C_INTERNAL_BINARY_SEARCH_H_

#include <stddef.h>
#include <stdint.h>

#include "cel-c/assert.h"
#include "cel-c/internal/config.h"

CEL_BEGIN_DECLS

// Binary search. `bsearch()` from the standard library is not required to be
// binary search, so for consistency across platforms we implement our own
// `bsearch()`.
CEL_ATTRIBUTE_NODISCARD
static inline void* cel_nullable _cel_BinarySearch(
    const void* cel_nullability_unknown key,
    const void* cel_nullability_unknown ptr, size_t count, size_t size,
    int (*cmp)(const void* cel_nullability_unknown,
               const void* cel_nullability_unknown)) {
  CEL_ASSERT_LE(count, (size_t)PTRDIFF_MAX);
  CEL_ASSERT_LE(size, (size_t)PTRDIFF_MAX);
  CEL_ASSERT_NOT_NULL(cmp);

  if (count > 0 && size > 0) {
    ptrdiff_t l = 0;
    ptrdiff_t r = (ptrdiff_t)(count - 1);
    do {
      CEL_ASSERT_GE(l, 0);
      CEL_ASSERT_GE(r, 0);

      const ptrdiff_t m = l + ((r - l) / 2);
      const void* const p = ((const char*)ptr) + ((size_t)m * size);
      const int diff = (*cmp)(key, p);
      if (diff < 0) {
        r = m - 1;
        continue;
      }
      if (diff > 0) {
        l = m + 1;
        continue;
      }
      return (void*)p;
    } while (l <= r);
  }
  return cel_nullptr;
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_BINARY_SEARCH_H_
