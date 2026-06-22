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

#include "cel-c/internal/generic_array.h"

#include <errno.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/internal/asan.h"
#include "cel-c/internal/ckdint.h"
#include "cel-c/internal/config.h"

static void _cel_GenericArray_Annotate(CEL_NULLABLE(const void*) ptr,
                                       size_t cap, size_t old_len,
                                       size_t new_len, size_t ele_size) {
  _cel_sanitizer_annotate_contiguous_container(
      ptr, (((CEL_NULLABLE(const char*))ptr) + (cap * ele_size)),
      (((CEL_NULLABLE(const char*))ptr) + (old_len * ele_size)),
      (((CEL_NULLABLE(const char*))ptr) + (new_len * ele_size)));
}

// This function abstracts away the complication ASan imposes on us resizing.
CEL_ATTRIBUTE_NODISCARD
static bool _cel_GenericArray_ReallocateAllocator(
    CEL_NONNULL(_cel_GenericArray*) arr, CEL_NONNULL(cel_Allocator*) alloc,
    size_t new_len, size_t new_cap, size_t ele_size) {
  CEL_ASSERT_NE(new_cap, arr->cap);
  CEL_ASSERT_LE(new_len, new_cap);
  size_t new_cap_bytes;
  if (CEL_UNLIKELY(_cel_ckd_mul(&new_cap_bytes, new_cap, ele_size))) {
    errno = ENOMEM;
    return false;
  }
  size_t actual_new_cap_bytes;
  CEL_NULLABLE(void*) new_ptr;
#ifdef _CEL_HAVE_ASAN
  // Under ASan we cannot reallocate directly as we have to perform bookkeeping
  // work. Instead perform an allocation, copy, and deallocate the old
  // allocation.
  new_ptr = cel_Allocator_Malloc(alloc, new_cap_bytes, &actual_new_cap_bytes);
  if (CEL_UNLIKELY(new_ptr == cel_nullptr)) {
    return false;
  }
  new_cap = actual_new_cap_bytes / ele_size;
  // Annotate new allocation with ASan to match the state before copying to it.
  _cel_GenericArray_Annotate(new_ptr, new_cap, new_cap, new_len, ele_size);
  memcpy(new_ptr, arr->ptr,
         (arr->len < new_len ? arr->len : new_len) * ele_size);
  // Annotate the old allocation back to its original state, which is the whole
  // allocation is valid.
  _cel_GenericArray_Annotate(arr->ptr, arr->cap, arr->len, arr->cap, ele_size);
  cel_Allocator_FreeSized(alloc, arr->ptr, arr->cap * ele_size);
#else
  new_ptr = cel_Allocator_Realloc(alloc, arr->ptr, arr->cap * ele_size,
                                  new_cap_bytes, &actual_new_cap_bytes);
  if (CEL_UNLIKELY(new_ptr == cel_nullptr)) {
    return false;
  }
  new_cap = actual_new_cap_bytes / ele_size;
#endif
  arr->ptr = new_ptr;
  arr->cap = new_cap;
  arr->len = new_len;
  return true;
}

extern "C" void _cel_GenericArray_Construct(CEL_NONNULL(_cel_GenericArray*)
                                                arr) {
  CEL_ASSERT_NOT_NULL(arr);
  memset(arr, '\0', sizeof(*arr));
}

extern "C" void _cel_GenericArray_DestructAllocator(
    CEL_NONNULL(_cel_GenericArray*) arr, CEL_NONNULL(cel_Allocator*) alloc,
    size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_GT(ele_size, 0);
  CEL_ASSERT_LE(arr->len, arr->cap);
  if (arr->cap > 0) {
    _cel_GenericArray_Annotate(arr->ptr, arr->cap, /*old_len=*/arr->len,
                               /*new_len=*/arr->cap, ele_size);
    cel_Allocator_FreeSized(alloc, arr->ptr, arr->cap * ele_size);
  }
}

extern "C" bool _cel_GenericArray_ReserveAllocator(
    CEL_NONNULL(_cel_GenericArray*) arr, CEL_NONNULL(cel_Allocator*) alloc,
    size_t new_cap, size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_GT(ele_size, 0);
  CEL_ASSERT_LE(arr->len, arr->cap);
  if (arr->cap >= new_cap) {
    return true;
  }
  return _cel_GenericArray_ReallocateAllocator(arr, alloc, arr->len, new_cap,
                                               ele_size);
}

extern "C" bool _cel_GenericArray_ReserveArena(CEL_NONNULL(_cel_GenericArray*)
                                                   arr,
                                               CEL_NONNULL(cel_Arena*) arena,
                                               size_t new_cap,
                                               size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_GT(ele_size, 0);
  CEL_ASSERT_LE(arr->len, arr->cap);
  if (arr->cap >= new_cap) {
    return true;
  }
  size_t new_cap_bytes;
  if (CEL_UNLIKELY(_cel_ckd_mul(&new_cap_bytes, new_cap, ele_size))) {
    errno = ENOMEM;
    return false;
  }
  CEL_NULLABLE(void*)
  new_ptr = cel_Arena_Realloc(arena, arr->ptr, arr->cap * ele_size,
                              new_cap_bytes, cel_nullptr);
  if (CEL_UNLIKELY(new_ptr == cel_nullptr)) {
    return false;
  }
  arr->ptr = new_ptr;
  arr->cap = new_cap;
  return true;
}

extern "C" void _cel_GenericArray_ShrinkToFitAllocator(
    CEL_NONNULL(_cel_GenericArray*) arr, CEL_NONNULL(cel_Allocator*) alloc,
    size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_GT(ele_size, 0);
  CEL_ASSERT_LE(arr->len, arr->cap);
  if (arr->len == arr->cap) {
    return;
  }
  if (arr->len == 0) {
    _cel_GenericArray_ResetAllocator(arr, alloc, ele_size);
    return;
  }
  CEL_USED(_cel_GenericArray_ReallocateAllocator(arr, alloc, arr->len, arr->len,
                                                 ele_size));
}

extern "C" CEL_NULLABLE(void*)
    _cel_GenericArray_ResizeAllocator(CEL_NONNULL(_cel_GenericArray*) arr,
                                      CEL_NONNULL(cel_Allocator*) alloc,
                                      size_t new_len, size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_GT(ele_size, 0);
  CEL_ASSERT_LE(arr->len, arr->cap);
  if (CEL_UNLIKELY(
          !_cel_GenericArray_ReserveAllocator(arr, alloc, new_len, ele_size))) {
    return cel_nullptr;
  }
  CEL_NULLABLE(void*) ptr;
  if (new_len < arr->len) {
    ptr = ((CEL_NONNULL(char*))arr->ptr) + (new_len * ele_size);
  } else {
    ptr = ((CEL_NONNULL(char*))arr->ptr) + (arr->len * ele_size);
  }
  _cel_GenericArray_Annotate(arr->ptr, arr->cap, arr->len, new_len, ele_size);
  arr->len = new_len;
  return ptr;
}

extern "C" CEL_NULLABLE(void*)
    _cel_GenericArray_ResizeArena(CEL_NONNULL(_cel_GenericArray*) arr,
                                  CEL_NONNULL(cel_Arena*) arena, size_t new_len,
                                  size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_GT(ele_size, 0);
  CEL_ASSERT_LE(arr->len, arr->cap);
  if (CEL_UNLIKELY(
          !_cel_GenericArray_ReserveArena(arr, arena, new_len, ele_size))) {
    return cel_nullptr;
  }
  CEL_NULLABLE(void*) ptr;
  if (new_len < arr->len) {
    ptr = ((CEL_NONNULL(char*))arr->ptr) + (new_len * ele_size);
  } else {
    ptr = ((CEL_NONNULL(char*))arr->ptr) + (arr->len * ele_size);
  }
  arr->len = new_len;
  return ptr;
}

extern "C" CEL_NULLABLE(void*)
    _cel_GenericArray_AppendAllocator(CEL_NONNULL(_cel_GenericArray*) arr,
                                      CEL_NONNULL(cel_Allocator*) alloc,
                                      size_t n, size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_GT(ele_size, 0);
  CEL_ASSERT_LE(arr->len, arr->cap);
  if (n == 0) {
    return cel_nullptr;
  }
  if (CEL_UNLIKELY(n > arr->cap - arr->len)) {
    size_t min_cap;
    if (CEL_UNLIKELY(_cel_ckd_add(&min_cap, arr->len, n))) {
      errno = ENOMEM;
      return cel_nullptr;
    }
    size_t new_cap = arr->cap;
    if (new_cap < 8) {
      new_cap = 8;
    }
    while (new_cap < min_cap) {
      if (CEL_UNLIKELY(_cel_ckd_mul(&new_cap, new_cap, (size_t)2))) {
        new_cap = min_cap;
        break;
      }
    };
    size_t old_len = arr->len;
    if (CEL_UNLIKELY(!_cel_GenericArray_ReallocateAllocator(
            arr, alloc, old_len + n, new_cap, ele_size))) {
      return cel_nullptr;
    }
    // _cel_GenericArray_ReallocateAllocator has already done the len upate and
    // annotation update.
    CEL_NONNULL(void*)
    ptr = ((CEL_NONNULL(char*))arr->ptr) + (old_len * ele_size);
    return ptr;
  }
  size_t new_len = arr->len + n;
  _cel_GenericArray_Annotate(arr->ptr, arr->cap, arr->len, new_len, ele_size);
  CEL_NONNULL(void*)
  ptr = ((CEL_NONNULL(char*))arr->ptr) + (arr->len * ele_size);
  arr->len = new_len;
  return ptr;
}

extern "C" CEL_NULLABLE(void*)
    _cel_GenericArray_AppendArena(CEL_NONNULL(_cel_GenericArray*) arr,
                                  CEL_NONNULL(cel_Arena*) arena, size_t n,
                                  size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_GT(ele_size, 0);
  CEL_ASSERT_LE(arr->len, arr->cap);
  if (n == 0) {
    return cel_nullptr;
  }
  if (CEL_UNLIKELY(n > arr->cap - arr->len)) {
    size_t min_cap;
    if (CEL_UNLIKELY(_cel_ckd_add(&min_cap, arr->len, n))) {
      errno = ENOMEM;
      return cel_nullptr;
    }
    size_t new_cap = arr->cap;
    if (new_cap < 8) {
      new_cap = 8;
    }
    while (new_cap < min_cap) {
      if (CEL_UNLIKELY(_cel_ckd_mul(&new_cap, new_cap, (size_t)2))) {
        new_cap = min_cap;
        break;
      }
    }
    size_t new_cap_bytes;
    if (CEL_UNLIKELY(_cel_ckd_mul(&new_cap_bytes, new_cap, ele_size))) {
      if (CEL_UNLIKELY(new_cap == min_cap ||
                       _cel_ckd_mul(&new_cap_bytes, min_cap, ele_size))) {
        errno = ENOMEM;
        return cel_nullptr;
      }
      new_cap = min_cap;
    }
    CEL_NULLABLE(void*)
    ptr = cel_Arena_Realloc(arena, arr->ptr, arr->cap * ele_size, new_cap_bytes,
                            cel_nullptr);
    if (CEL_UNLIKELY(ptr == cel_nullptr)) {
      return cel_nullptr;
    }
    arr->ptr = ptr;
    arr->cap = new_cap;
  }
  CEL_NONNULL(void*)
  ptr = ((CEL_NONNULL(char*))arr->ptr) + (arr->len * ele_size);
  arr->len += n;
  return ptr;
}

extern "C" void _cel_GenericArray_PopAllocator(CEL_NONNULL(_cel_GenericArray*)
                                                   arr,
                                               size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_GT(ele_size, 0);
  CEL_ASSERT_NOT(_cel_GenericArray_Empty(arr));
  CEL_ASSERT_LE(arr->len, arr->cap);
  size_t new_len = arr->len - 1;
  _cel_GenericArray_Annotate(arr->ptr, arr->cap, arr->len, new_len, ele_size);
  arr->len = new_len;
}

extern "C" void _cel_GenericArray_PopArena(CEL_NONNULL(_cel_GenericArray*)
                                               arr) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_NOT(_cel_GenericArray_Empty(arr));
  CEL_ASSERT_LE(arr->len, arr->cap);
  --(arr->len);
}

extern "C" void _cel_GenericArray_EraseAllocator(CEL_NONNULL(_cel_GenericArray*)
                                                     arr,
                                                 size_t idx, size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_LT(idx, _cel_GenericArray_Size(arr));
  CEL_ASSERT_GT(ele_size, 0);

  CEL_NONNULL(char*)
  ptr = ((CEL_NONNULL(char*))arr->ptr);
  memmove(ptr + (idx * ele_size), ptr + ((idx + 1) * ele_size),
          (arr->len - (idx + 1)) * ele_size);
  _cel_GenericArray_PopAllocator(arr, ele_size);
}

extern "C" void _cel_GenericArray_EraseArena(CEL_NONNULL(_cel_GenericArray*)
                                                 arr,
                                             size_t idx, size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_LT(idx, _cel_GenericArray_Size(arr));
  CEL_ASSERT_GT(ele_size, 0);

  CEL_NONNULL(char*)
  ptr = ((CEL_NONNULL(char*))arr->ptr);
  memmove(ptr + (idx * ele_size), ptr + ((idx + 1) * ele_size),
          (arr->len - (idx + 1)) * ele_size);
  _cel_GenericArray_PopArena(arr);
}

extern "C" void _cel_GenericArray_ClearAllocator(CEL_NONNULL(_cel_GenericArray*)
                                                     arr,
                                                 size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_LE(arr->len, arr->cap);
  _cel_GenericArray_Annotate(arr->ptr, arr->cap, /*old_len=*/arr->len,
                             /*new_len=*/0, ele_size);
  arr->len = 0;
}

extern "C" void _cel_GenericArray_ClearArena(CEL_NONNULL(_cel_GenericArray*)
                                                 arr) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_LE(arr->len, arr->cap);
  arr->len = 0;
}

extern "C" void _cel_GenericArray_ResetAllocator(CEL_NONNULL(_cel_GenericArray*)
                                                     arr,
                                                 CEL_NONNULL(cel_Allocator*)
                                                     alloc,
                                                 size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_GT(ele_size, 0);
  CEL_ASSERT_LE(arr->len, arr->cap);
  if (arr->cap > 0) {
    _cel_GenericArray_Annotate(arr->ptr, arr->cap, /*old_len=*/arr->len,
                               /*new_len=*/arr->cap, ele_size);
    cel_Allocator_FreeSized(alloc, arr->ptr, arr->cap * ele_size);
  }
  memset(arr, '\0', sizeof(*arr));
}
