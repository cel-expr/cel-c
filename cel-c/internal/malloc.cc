// Copyright 2024 Google LLC
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

#include "cel-c/internal/malloc.h"

#if defined(__APPLE__)
#include <malloc/malloc.h>
#endif

#if defined(__FreeBSD__)
#include <malloc_np.h>
#endif

#include <errno.h>
#include <limits.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/internal/bit.h"
#include "cel-c/internal/ckdint.h"
#include "cel-c/internal/config.h"

#ifdef _CEL_HAVE_SANITIZER
#include <sanitizer/allocator_interface.h>
#endif

#if _CEL_HAVE_ATTRIBUTE_WEAK && !defined(__FreeBSD__) && \
    !defined(__APPLE__) && !defined(_CEL_HAVE_SANITIZER)
extern "C" _CEL_ATTRIBUTE_WEAK size_t nallocx(size_t size, int opts) {
  CEL_USED(opts);
  return size;
}
#ifndef MALLOCX_LG_ALIGN
#define MALLOCX_LG_ALIGN(la) ((int)(la))
#endif
#ifndef MALLOCX_ALIGN
#define MALLOCX_ALIGN(a) MALLOCX_LG_ALIGN(_cel_ffs(a) - 1)
#endif
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t _cel_MallocGoodSize(size_t size) {
#if defined(_CEL_HAVE_SANITIZER)
  return __sanitizer_get_estimated_allocated_size(size);
#elif defined(__APPLE__)
  return malloc_good_size(size);
#elif defined(__FreeBSD__) || _CEL_HAVE_ATTRIBUTE_WEAK
  return nallocx(size, MALLOCX_ALIGN(alignof(max_align_t)));
#else
  return size;
#endif
}

extern "C" CEL_NULLABLE(void*)
    _cel_Malloc(size_t size, CEL_NULLABLE(size_t*) actual_size) {
  if (CEL_UNLIKELY(size == 0)) {
    if (actual_size != cel_nullptr) {
      *actual_size = 0;
    }
    return cel_nullptr;
  }
  if (CEL_UNLIKELY(size > (size_t)PTRDIFF_MAX)) {
    if (actual_size != cel_nullptr) {
      *actual_size = 0;
    }
    errno = ENOMEM;
    return cel_nullptr;
  }
  if (actual_size != cel_nullptr) {
    size = _cel_MallocGoodSize(size);
  }
  void* p = malloc(size);
  if (actual_size != cel_nullptr) {
    *actual_size = p != cel_nullptr ? size : 0;
  }
  if (CEL_UNLIKELY(p == cel_nullptr)) {
    errno = ENOMEM;
  }
  return p;
}

extern "C" CEL_NULLABLE(void*)
    _cel_Calloc(size_t num, size_t size, CEL_NULLABLE(size_t*) actual_num) {
  if (CEL_UNLIKELY(num == 0 || size == 0)) {
    if (actual_num != cel_nullptr) {
      *actual_num = 0;
    }
    return cel_nullptr;
  }
  size_t total_size;
  if (CEL_UNLIKELY(_cel_ckd_mul(&total_size, num, size) ||
                   total_size > (size_t)PTRDIFF_MAX)) {
    if (actual_num != cel_nullptr) {
      *actual_num = 0;
    }
    errno = ENOMEM;
    return cel_nullptr;
  }
  void* addr = _cel_Malloc(total_size, actual_num);
  if (CEL_LIKELY(addr != cel_nullptr)) {
    memset(addr, 0, actual_num != cel_nullptr ? *actual_num : total_size);
  }
  if (actual_num != cel_nullptr) {
    *actual_num /= size;
  }
  return addr;
}

extern "C" CEL_NULLABLE(void*)
    _cel_Realloc(CEL_NULLABLE(void*) addr, size_t old_size, size_t new_size,
                 CEL_NULLABLE(size_t*) actual_size) {
  if (old_size == 0) {
    CEL_ASSERT_NULL(addr);
    return _cel_Malloc(new_size, actual_size);
  }
  if (new_size == 0) {
    _cel_FreeSized(addr, old_size);
    if (actual_size != cel_nullptr) {
      *actual_size = 0;
    }
    return cel_nullptr;
  }
  if (actual_size == cel_nullptr) {
    return realloc(addr, new_size);
  }
  void* new_addr = _cel_Malloc(new_size, actual_size);
  if (CEL_LIKELY(new_addr != cel_nullptr)) {
    memcpy(new_addr, addr, old_size < new_size ? old_size : new_size);
    _cel_FreeSized(addr, old_size);
  }
  return new_addr;
}

extern "C" void _cel_Free(CEL_NULLABLE(void*) addr) { free(addr); }

extern "C" void _cel_FreeSized(CEL_NULLABLE(void*) addr, size_t size) {
  CEL_ASSERT((addr == cel_nullptr && size == 0) || size != 0);

  // Because we may be using `nallocx`/`malloc_good_size` above, the passed in
  // `size` may not be in the acceptable range for `free_sized`. So fallback to
  // `free`.
  CEL_USED(size);
  free(addr);
}
