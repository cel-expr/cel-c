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

// Internal header providing a generic dynamic array implementation.

// IWYU pragma: private
// IWYU pragma: friend "cel-c/src/(?:(?:arena|generic)_)?array\.[hc]"

#ifndef THIRD_PARTY_CEL_C_SRC_GENERIC_ARRAY_H_
#define THIRD_PARTY_CEL_C_SRC_GENERIC_ARRAY_H_

#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>

#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/src/config.h"

CEL_BEGIN_DECLS

// _cel_GenericArray
//
// A generic implementation of a dynamic array for any type that is trivially
// copyable. Supports both allocator-based and arena-based memory management.
// Should not be used directly except by `_cel_Array` and `_cel_ArenaArray`.
typedef struct _cel_GenericArray {
  CEL_NULLABLE(void*) ptr;
  // Length, number of elements in the array. Always less than or equal to cap.
  size_t len;
  // Capacity, number of elements we can store before we have to grow. Always
  // greater than or equal to len.
  size_t cap;
} _cel_GenericArray;

// _cel_GenericArray_Construct
//
// Constructs the array. Upon return the array has a size and capacity of 0.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericArray_Construct(CEL_NONNULL(_cel_GenericArray*) arr);

// _cel_GenericArray_DestructAllocator
//
// Destructs the array.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericArray_DestructAllocator(CEL_NONNULL(_cel_GenericArray*) arr,
                                         CEL_NONNULL(cel_Allocator*) alloc,
                                         size_t ele_size);

// _cel_GenericArray_Size
//
// Returns the number of elements in the array.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_GenericArray_Size(CEL_NONNULL(const _cel_GenericArray*) arr) {
  CEL_ASSERT_NOT_NULL(arr);
  return arr->len;
}

// _cel_GenericArray_Empty
//
// Equivalent to `_cel_GenericArray_Size(arr) == 0`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_GenericArray_Empty(
    CEL_NONNULL(const _cel_GenericArray*) arr) {
  return _cel_GenericArray_Size(arr) == 0;
}

// _cel_GenericArray_Capacity
//
// Returns the number of elements that can be in the array before adding more
// will require a reallocation.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_GenericArray_Capacity(CEL_NONNULL(const _cel_GenericArray*) arr) {
  CEL_ASSERT_NOT_NULL(arr);
  return arr->cap;
}

// _cel_GenericArray_Data
//
// Returns a constant pointer to the first element in the backing contiguous
// array.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(const void*)
    _cel_GenericArray_Data(CEL_NONNULL(const _cel_GenericArray*) arr) {
  CEL_ASSERT_NOT_NULL(arr);
  return arr->ptr;
}

// _cel_GenericArray_Data
//
// Returns a mutable pointer to the first element in the backing contiguous
// array.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_GenericArray_MutableData(CEL_NONNULL(_cel_GenericArray*) arr) {
  return (CEL_NULLABLE(void*))_cel_GenericArray_Data(arr);
}

// _cel_GenericArray_At
//
// Returns a constant pointer to the element at index `idx` in the backing
// contiguous array.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const void*)
    _cel_GenericArray_At(CEL_NONNULL(const _cel_GenericArray*) arr, size_t idx,
                         size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_LT(idx, arr->len);
  CEL_ASSERT_GT(ele_size, 0);
  return ((const char*)arr->ptr) + (idx * ele_size);
}

// _cel_GenericArray_MutableAt
//
// Returns a mutable pointer to the element at index `idx` in the backing
// contiguous array.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_GenericArray_MutableAt(CEL_NONNULL(_cel_GenericArray*) arr, size_t idx,
                                size_t ele_size) {
  return (CEL_NONNULL(void*))_cel_GenericArray_At(arr, idx, ele_size);
}

// _cel_GenericArray_Front
//
// Returns a constant pointer to the first element in the backing contiguous
// array.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const void*)
    _cel_GenericArray_Front(CEL_NONNULL(const _cel_GenericArray*) arr) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_NOT(_cel_GenericArray_Empty(arr));
  return arr->ptr;
}

// _cel_GenericArray_MutableFront
//
// Returns a mutable pointer to the first element in the backing contiguous
// array.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_GenericArray_MutableFront(CEL_NONNULL(_cel_GenericArray*) arr) {
  return (CEL_NONNULL(void*))_cel_GenericArray_Front(arr);
}

// _cel_GenericArray_Back
//
// Returns a constant pointer to the last element in the backing contiguous
// array.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const void*)
    _cel_GenericArray_Back(CEL_NONNULL(const _cel_GenericArray*) arr,
                           size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_NOT(_cel_GenericArray_Empty(arr));
  CEL_ASSERT_GT(ele_size, 0);
  return ((const char*)arr->ptr) + ((arr->len - 1) * ele_size);
}

// _cel_GenericArray_MutableBack
//
// Returns a mutable pointer to the last element in the backing contiguous
// array.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_GenericArray_MutableBack(CEL_NONNULL(_cel_GenericArray*) arr,
                                  size_t ele_size) {
  return (CEL_NONNULL(void*))_cel_GenericArray_Back(arr, ele_size);
}

// _cel_GenericArray_ReserveAllocator
//
// Requests that the array expand its capacity to be at least `new_cap`. Returns
// `true` if it was successful, `false` otherwise.
CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericArray_ReserveAllocator(CEL_NONNULL(_cel_GenericArray*) arr,
                                        CEL_NONNULL(cel_Allocator*) alloc,
                                        size_t new_cap, size_t ele_size);

// _cel_GenericArray_ReserveArena
//
// Requests that the array expand its capacity to be at least `new_cap`. Returns
// `true` if it was successful, `false` otherwise. This is a hint.
CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericArray_ReserveArena(CEL_NONNULL(_cel_GenericArray*) arr,
                                    CEL_NONNULL(cel_Arena*) arena,
                                    size_t new_cap, size_t ele_size);

// _cel_GenericArray_ShrinkToFitAllocator
//
// Requests that the array shrink its capacity to be the length of the array.
// This is a hint.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericArray_ShrinkToFitAllocator(CEL_NONNULL(_cel_GenericArray*) arr,
                                            CEL_NONNULL(cel_Allocator*) alloc,
                                            size_t ele_size);

// _cel_GenericArray_ResizeAllocator
//
// Resizes the array to hold exact `new_len` elements. If `new_len` is greater
// than the current size and any reallocation was successful, returns a mutable
// pointer to the first uninitialized element. Otherwise `nullptr` is returned.
// If `new_len` is less than or equal to the current size, returns a mutable
// pointer to the end of the array.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_NULLABLE(void*)
_cel_GenericArray_ResizeAllocator(CEL_NONNULL(_cel_GenericArray*) arr,
                                  CEL_NONNULL(cel_Allocator*) alloc,
                                  size_t new_len, size_t ele_size);

// _cel_GenericArray_ResizeArena
//
// Resizes the array to hold exact `new_len` elements. If `new_len` is greater
// than the current size and any reallocation was successful, returns a mutable
// pointer to the first uninitialized element. Otherwise `nullptr` is returned.
// If `new_len` is less than or equal to the current size, returns a mutable
// pointer to the end of the array.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_NULLABLE(void*)
_cel_GenericArray_ResizeArena(CEL_NONNULL(_cel_GenericArray*) arr,
                              CEL_NONNULL(cel_Arena*) arena, size_t new_len,
                              size_t ele_size);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_NULLABLE(void*)
_cel_GenericArray_AppendAllocator(CEL_NONNULL(_cel_GenericArray*) arr,
                                  CEL_NONNULL(cel_Allocator*) alloc, size_t n,
                                  size_t ele_size);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_NULLABLE(void*)
_cel_GenericArray_AppendArena(CEL_NONNULL(_cel_GenericArray*) arr,
                              CEL_NONNULL(cel_Arena*) arena, size_t n,
                              size_t ele_size);

// _cel_GenericArray_PushAllocator
//
// Equivalent to
// `_cel_GenericArray_AppendAllocator(arr, alloc, 1, ele_size)`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_GenericArray_PushAllocator(CEL_NONNULL(_cel_GenericArray*) arr,
                                    CEL_NONNULL(cel_Allocator*) alloc,
                                    size_t ele_size) {
  return _cel_GenericArray_AppendAllocator(arr, alloc, /*n=*/1, ele_size);
}

// _cel_GenericArray_PushArena
//
// Equivalent to
// `_cel_GenericArray_AppendArena(arr, arena, 1, ele_size)`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_GenericArray_PushArena(CEL_NONNULL(_cel_GenericArray*) arr,
                                CEL_NONNULL(cel_Arena*) arena,
                                size_t ele_size) {
  return _cel_GenericArray_AppendArena(arr, arena, /*n=*/1, ele_size);
}

// _cel_GenericArray_PopAllocator
//
// Reduces the size of the array by one, effectively removing the last element.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericArray_PopAllocator(CEL_NONNULL(_cel_GenericArray*) arr,
                                    size_t ele_size);

// _cel_GenericArray_PopArena
//
// Reduces the size of the array by one, effectively removing the last element.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericArray_PopArena(CEL_NONNULL(_cel_GenericArray*) arr);

// _cel_GenericArray_EraseAllocator
//
// Erases the element at index `idx`, shifting every element after `idx` left by
// one and reducing the size by one.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericArray_EraseAllocator(CEL_NONNULL(_cel_GenericArray*) arr,
                                      size_t idx, size_t ele_size);

// _cel_GenericArray_EraseArena
//
// Erases the element at index `idx`, shifting every element after `idx` left by
// one and reducing the size by one.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericArray_EraseArena(CEL_NONNULL(_cel_GenericArray*) arr,
                                  size_t idx, size_t ele_size);

// _cel_GenericArray_ClearAllocator
//
// Sets the arrays size to 0.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericArray_ClearAllocator(CEL_NONNULL(_cel_GenericArray*) arr,
                                      size_t ele_size);

// _cel_GenericArray_ClearArena
//
// Sets the arrays size to 0.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericArray_ClearArena(CEL_NONNULL(_cel_GenericArray*) arr);

// _cel_GenericArray_ResetAllocator
//
// Deallocates any backing array and sets the size and capacity to 0. This is
// equivalent to calling `_cel_GenericArray_DestructAllocator()` followed by
// `_cel_GenericArray_Construct()`.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericArray_ResetAllocator(CEL_NONNULL(_cel_GenericArray*) arr,
                                      CEL_NONNULL(cel_Allocator*) alloc,
                                      size_t ele_size);

// _cel_GenericArray_Begin
//
// Equivalent to `_cel_GenericArray_Data`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(const void*)
    _cel_GenericArray_Begin(CEL_NONNULL(const _cel_GenericArray*) arr) {
  CEL_ASSERT_NOT_NULL(arr);
  return arr->ptr;
}

// _cel_GenericArray_MutableBegin
//
// Equivalent to `_cel_GenericArray_MutableData`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_GenericArray_MutableBegin(CEL_NONNULL(_cel_GenericArray*) arr) {
  return (CEL_NULLABLE(void*))_cel_GenericArray_Begin(arr);
}

// _cel_GenericArray_End
//
// Returns a constant pointer that is one past the last element in the array if
// non-empty, or returns `_cel_GenericArray_Begin`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(const void*)
    _cel_GenericArray_End(CEL_NONNULL(const _cel_GenericArray*) arr,
                          size_t ele_size) {
  CEL_ASSERT_NOT_NULL(arr);
  CEL_ASSERT_GT(ele_size, 0);
  return ((CEL_NULLABLE(const char*))arr->ptr) + (arr->len * ele_size);
}

// _cel_GenericArray_MutableEnd
//
// Returns a mutable pointer that is one past the last element in the array if
// non-empty, or returns `_cel_GenericArray_MutableBegin`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_GenericArray_MutableEnd(CEL_NONNULL(_cel_GenericArray*) arr,
                                 size_t ele_size) {
  return (CEL_NULLABLE(void*))_cel_GenericArray_End(arr, ele_size);
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_SRC_GENERIC_ARRAY_H_
