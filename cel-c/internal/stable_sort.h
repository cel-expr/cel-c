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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_MERGESORT_H_
#define THIRD_PARTY_CEL_C_INTERNAL_MERGESORT_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/internal/alloca.h"

CEL_BEGIN_DECLS

typedef struct {
  char* cel_nullability_unknown ptr;
  size_t size;
  int (*cmp)(const void* cel_nullability_unknown,
             const void* cel_nullability_unknown);
  void* cel_nullability_unknown tmp;
} __cel_StableSortState;

CEL_ATTRIBUTE_NODISCARD
static inline int __cel_StableSortCompare(
    const __cel_StableSortState* cel_nonnull state, ptrdiff_t a, ptrdiff_t b) {
  return (*state->cmp)(state->ptr + ((size_t)a * state->size),
                       state->ptr + ((size_t)b * state->size));
}

static inline void __cel_StableSortSwap(
    const __cel_StableSortState* cel_nonnull state, ptrdiff_t a, ptrdiff_t b) {
  char* const ap = state->ptr + ((size_t)a * state->size);
  char* const bp = state->ptr + ((size_t)b * state->size);
  memcpy(state->tmp, ap, state->size);
  memcpy(ap, bp, state->size);
  memcpy(bp, state->tmp, state->size);
}

static inline void __cel_StableSortInsertionSort(
    const __cel_StableSortState* cel_nonnull state, ptrdiff_t a, ptrdiff_t b) {
  for (ptrdiff_t i = a + 1; i < b; ++i) {
    for (ptrdiff_t j = i; j > a && __cel_StableSortCompare(state, j, j - 1) < 0;
         --j) {
      __cel_StableSortSwap(state, j, j - 1);
    }
  }
}

static inline void __cel_StableSortSwapRange(
    const __cel_StableSortState* cel_nonnull state, ptrdiff_t a, ptrdiff_t b,
    ptrdiff_t n) {
  for (ptrdiff_t i = 0; i < n; ++i) {
    __cel_StableSortSwap(state, a + i, b + i);
  }
}

static inline void __cel_StableSortRotate(
    const __cel_StableSortState* cel_nonnull state, ptrdiff_t a, ptrdiff_t m,
    ptrdiff_t b) {
  ptrdiff_t i = m - a;
  ptrdiff_t j = b - m;
  while (i != j) {
    if (i > j) {
      __cel_StableSortSwapRange(state, m - i, m, j);
      i -= j;
    } else {
      __cel_StableSortSwapRange(state, m - i, m + j - i, i);
      j -= i;
    }
  }
  __cel_StableSortSwapRange(state, m - i, m, i);
}

static inline void __cel_StableSortSymMerge(
    const __cel_StableSortState* cel_nonnull state, ptrdiff_t a, ptrdiff_t m,
    ptrdiff_t b) {
  if (m - a == 1) {
    ptrdiff_t i = m;
    ptrdiff_t j = b;
    while (i < j) {
      ptrdiff_t h = (ptrdiff_t)((size_t)(i + j) >> 1);
      if (__cel_StableSortCompare(state, h, a) < 0) {
        i = h + 1;
      } else {
        j = h;
      }
    }
    for (ptrdiff_t k = a; k < i - 1; ++k) {
      __cel_StableSortSwap(state, k, k + 1);
    }
    return;
  }

  if (b - m == 1) {
    ptrdiff_t i = a;
    ptrdiff_t j = m;
    while (i < j) {
      ptrdiff_t h = (ptrdiff_t)((size_t)(i + j) >> 1);
      if (!(__cel_StableSortCompare(state, m, h) < 0)) {
        i = h + 1;
      } else {
        j = h;
      }
    }
    for (ptrdiff_t k = m; k > i; --k) {
      __cel_StableSortSwap(state, k, k - 1);
    }
    return;
  }

  ptrdiff_t mid = (ptrdiff_t)((size_t)(a + b) >> 1);
  ptrdiff_t n = mid + m;
  ptrdiff_t start;
  ptrdiff_t r;
  if (m > mid) {
    start = n - b;
    r = mid;
  } else {
    start = a;
    r = m;
  }
  ptrdiff_t p = n - 1;
  while (start < r) {
    ptrdiff_t c = (ptrdiff_t)((size_t)(start + r) >> 1);
    if (!(__cel_StableSortCompare(state, p - c, c) < 0)) {
      start = c + 1;
    } else {
      r = c;
    }
  }
  ptrdiff_t end = n - start;
  if (start < m && m < end) {
    __cel_StableSortRotate(state, start, m, end);
  }
  if (a < start && start < mid) {
    __cel_StableSortSymMerge(state, a, start, mid);
  }
  if (mid < end && end < b) {
    __cel_StableSortSymMerge(state, mid, end, b);
  }
}

// Algorithm is adapted from Go's `sort.Stable` at version 1.24.2.
static inline void __cel_StableSort(
    const __cel_StableSortState* cel_nonnull state, ptrdiff_t n) {
  ptrdiff_t block_size = 20;
  ptrdiff_t a = 0;
  ptrdiff_t b = block_size;
  while (b <= n) {
    __cel_StableSortInsertionSort(state, a, b);
    a = b;
    b += block_size;
  }
  __cel_StableSortInsertionSort(state, a, n);

  while (block_size < n) {
    a = 0;
    b = block_size * 2;
    while (b <= n) {
      __cel_StableSortSymMerge(state, a, a + block_size, b);
      a = b;
      b += block_size * 2;
    }
    ptrdiff_t m = block_size + a;
    if (m < n) {
      __cel_StableSortSymMerge(state, a, m, n);
    }
    block_size *= 2;
  }
}

// Stable sorting algorithm which sorts in-place.
static inline void _cel_StableSort(
    void* cel_nullability_unknown ptr, size_t count, size_t size,
    int (*cmp)(const void* cel_nullability_unknown,
               const void* cel_nullability_unknown)) {
  CEL_ASSERT_LE(count, (size_t)PTRDIFF_MAX);
  CEL_ASSERT_LE(size, (size_t)PTRDIFF_MAX);
  CEL_ASSERT_NOT_NULL(cmp);

  if (count > 0 && size > 0) {
    __cel_StableSortState state;
    memset(&state, 0, sizeof(state));
    state.ptr = (char*)ptr;
    state.size = size;
    state.cmp = cmp;
    state.tmp = _cel_alloca(size);
    __cel_StableSort(&state, (ptrdiff_t)count);
  }
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_MERGESORT_H_
