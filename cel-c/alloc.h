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

#ifndef THIRD_PARTY_CEL_C_ALLOC_H_
#define THIRD_PARTY_CEL_C_ALLOC_H_

#include <stdalign.h>  // IWYU pragma: keep
#include <stdarg.h>
#include <stddef.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "upb/mem/alloc.h"

CEL_BEGIN_DECLS

#define cel_Allocator_kMaxAlign cel_kMaxAlign

typedef upb_alloc cel_Allocator;

// Default `cel_Allocator` implementation.
CEL_EXTERN CEL_NONNULL(cel_Allocator*) const cel_DefaultAllocator;

// cel_Allocator_Malloc
//
// Allocates `size` bytes of memory from the system and returns a pointer to the
// allocated memory. If `size` is `0` or the system is out of memory, a null
// pointer is returned. If `actual_size` is not null, the actual usable size in
// bytes of the underlying allocation is stored in `actual_size` which is
// guaranteed to be greater than or equal to `size` when the allocation is
// successful.
CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_MALLOC
static CEL_INLINE CEL_NULLABLE(void*)
    cel_Allocator_Malloc(CEL_NONNULL(cel_Allocator*) alloc, size_t size,
                         CEL_NULLABLE(size_t*) actual_size) {
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_NOT_NULL(alloc->func);

  return (*alloc->func)(alloc, cel_nullptr, 0, size, actual_size);
}

// cel_Allocator_Calloc
//
// Allocates `num` elements which are each `size` bytes from the system and
// returns a pointer to the allocated memory, the memory is initialized to zero.
// If `num` is `0`, `size` is `0`, or the system is out of memory, a null
// pointer is returned. If `actual_num` is not null, the actual usable size in
// elements of the underlying allocation is stored in `actual_num` which is
// guaranteed to be greater than or equal to `num` when the allocation is
// successful.
CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_MALLOC
CEL_EXTERN CEL_NULLABLE(void*)
    cel_Allocator_Calloc(CEL_NONNULL(cel_Allocator*) alloc, size_t num,
                         size_t size, CEL_NULLABLE(size_t*) actual_num);

// cel_Allocator_Realloc
//
// Reallocates a previously allocated block of memory. If `old_size` is `0`,
// this is equivalent to calling `cel_Allocator_Malloc`. If `new_size` is `0`,
// this is equivalent to call `cel_Allocator_FreeSized`. Otherwise a new block
// of memory is allocated, the lesser of `old_size` and `new_size` bytes are
// copied from the old memory block to the new memory block, and the old memory
// block is freed. If the system is out of memory, a null pointer is returned
// and no memory is allocated or freed.
CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    cel_Allocator_Realloc(CEL_NONNULL(cel_Allocator*) alloc,
                          CEL_NULLABLE(void*) addr, size_t old_size,
                          size_t new_size, CEL_NULLABLE(size_t*) actual_size) {
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_NOT_NULL(alloc->func);

  return (*alloc->func)(alloc, addr, old_size, new_size, actual_size);
}

// cel_Allocator_Free
//
// Deallocates a previously allocated block of memory.
CEL_ATTRIBUTE_NOTHROW
static CEL_INLINE void cel_Allocator_Free(CEL_NONNULL(cel_Allocator*) alloc,
                                          CEL_NULLABLE(void*) addr) {
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_NOT_NULL(alloc->func);

  (*alloc->func)(alloc, addr, 0, 0, cel_nullptr);
}

// cel_Allocator_FreeSized
//
// Deallocates a previously allocated block of memory with a known size.
CEL_ATTRIBUTE_NOTHROW
static CEL_INLINE void cel_Allocator_FreeSized(CEL_NONNULL(cel_Allocator*)
                                                   alloc,
                                               CEL_NULLABLE(void*) addr,
                                               size_t size) {
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_NOT_NULL(alloc->func);

  (*alloc->func)(alloc, addr, size, 0, cel_nullptr);
}

// cel_Allocator_StrDup
//
// Convenience function which duplicates the given null-terminated string. This
// function is similar to `strdup`. If the allocation fails, a null pointer is
// returned. If `actual_size` is not null, the actual usable size in bytes of
// the underlying allocation is stored in `actual_size`. This is
// **different** from the resulting string length as returned by `strlen`.
CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
CEL_EXTERN CEL_NULLABLE(char*)
    cel_Allocator_StrDup(CEL_NONNULL(cel_Allocator*) alloc,
                         CEL_NULLABLE(size_t*) actual_size,
                         CEL_NONNULL(const char*) str);

// cel_Allocator_PrintF
//
// Convenience function which prints a formatted string into memory allocated
// using `alloc`. If the allocation fails or the format string is invalid, a
// null pointer is returned. This function is similar to `asprintf`. If
// `actual_size` is not null, the actual usable size in bytes of the underlying
// allocation is stored in `actual_size`. This is **different** from the
// resulting string length as returned by `strlen`.
CEL_ATTRIBUTE_FORMAT(3, 4)
CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
CEL_EXTERN CEL_NULLABLE(char*)
    cel_Allocator_PrintF(CEL_NONNULL(cel_Allocator*) alloc,
                         CEL_NULLABLE(size_t*) actual_size,
                         CEL_NONNULL(const char*) fmt, ...);

// cel_Allocator_VPrintF
//
// See `cel_Allocator_PrintF`.
CEL_ATTRIBUTE_VFORMAT(3)
CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
CEL_EXTERN CEL_NULLABLE(char*)
    cel_Allocator_VPrintF(CEL_NONNULL(cel_Allocator*) alloc,
                          CEL_NULLABLE(size_t*) actual_size,
                          CEL_NONNULL(const char*) fmt, va_list args);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_ALLOC_H_
