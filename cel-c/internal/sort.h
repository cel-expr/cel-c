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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_SORT_H_
#define THIRD_PARTY_CEL_C_INTERNAL_SORT_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/internal/alloca.h"

CEL_BEGIN_DECLS

typedef struct {
  void* cel_nullability_unknown ptr;
  size_t size;
  int (*cmp)(const void* cel_nullability_unknown,
             const void* cel_nullability_unknown);
  void* cel_nullability_unknown tmp;
} __cel_SortState;

static inline void __cel_SortSwap(const __cel_SortState* cel_nonnull state,
                                  size_t l, size_t r) {
  char* const lp = (char*)state->ptr + (l * state->size);
  char* const rp = (char*)state->ptr + (r * state->size);
  memcpy(state->tmp, lp, state->size);
  memcpy(lp, rp, state->size);
  memcpy(rp, state->tmp, state->size);
}

// Hoare partition scheme.
static inline ptrdiff_t __cel_SortPartition(
    const __cel_SortState* cel_nonnull state, ptrdiff_t lo, ptrdiff_t hi) {
  CEL_ASSERT_GE(lo, 0);
  CEL_ASSERT_GE(hi, 0);
  CEL_ASSERT_LT(lo, hi);

  const void* const pivot =
      ((const char*)state->ptr) + ((size_t)lo * state->size);

  ptrdiff_t i = lo - 1;
  ptrdiff_t j = hi + 1;

  while (true) {
    do {
      ++i;
    } while (i <= hi &&
             (*state->cmp)((const char*)state->ptr + ((size_t)i * state->size),
                           pivot) < 0);
    do {
      --j;
    } while (j >= lo &&
             (*state->cmp)((const char*)state->ptr + ((size_t)j * state->size),
                           pivot) > 0);
    if (i >= j) {
      return j;
    }
    __cel_SortSwap(state, (size_t)i, (size_t)j);
  }
}

// Quicksort.
static inline void __cel_Sort(const __cel_SortState* cel_nonnull state,
                              ptrdiff_t lo, ptrdiff_t hi) {
  if (lo >= 0 && lo < hi) {
    const ptrdiff_t p = __cel_SortPartition(state, lo, hi);
    __cel_Sort(state, lo, p);
    __cel_Sort(state, p + 1, hi);
  }
}

// Unstable sorting algorithm which sorts in-place.
static inline void _cel_Sort(void* cel_nullability_unknown ptr, size_t count,
                             size_t size,
                             int (*cmp)(const void* cel_nullability_unknown,
                                        const void* cel_nullability_unknown)) {
  CEL_ASSERT_LE(count, (size_t)PTRDIFF_MAX);
  CEL_ASSERT_LE(size, (size_t)PTRDIFF_MAX);
  CEL_ASSERT_NOT_NULL(cmp);

  if (count > 0 && size > 0) {
    __cel_SortState state;
    memset(&state, 0, sizeof(state));
    state.ptr = ptr;
    state.size = size;
    state.cmp = cmp;
    state.tmp = _cel_alloca(size);
    __cel_Sort(&state, 0, (ptrdiff_t)(count - 1));
  }
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_SORT_H_
