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

// Internal header providing a generic deque implementation.

// IWYU pragma: private
// IWYU pragma: friend "cel-c/src/(?:(?:arena|generic)_)?deque\.[hc]"

#ifndef THIRD_PARTY_CEL_C_SRC_GENERIC_DEQUE_H_
#define THIRD_PARTY_CEL_C_SRC_GENERIC_DEQUE_H_

#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>

#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"

CEL_BEGIN_DECLS

struct _cel_GenericDequeBlock;

// _cel_GenericDeque
//
// A generic implementation of a deque for any type. Supports both
// allocator-based and arena-based memory management. Should not be used
// directly except by `_cel_Deque` and `_cel_ArenaDeque`.
typedef struct _cel_GenericDeque {
  CEL_NULLABLE(struct _cel_GenericDequeBlock*) head;
  CEL_NULLABLE(struct _cel_GenericDequeBlock*) tail;
  size_t head_pos;
  size_t tail_pos;
  size_t len;
  CEL_NULLABLE(struct _cel_GenericDequeBlock*) cache;
} _cel_GenericDeque;

// _cel_GenericDeque_Construct
//
// Constructs the deque. Upon return the deque has a size of 0.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericDeque_Construct(CEL_NONNULL(_cel_GenericDeque*) deq);

// _cel_GenericDeque_DestructAllocator
//
// Destructs the deque.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericDeque_DestructAllocator(CEL_NONNULL(_cel_GenericDeque*) deq,
                                         CEL_NONNULL(cel_Allocator*) alloc,
                                         size_t ele_size);

// _cel_GenericDeque_Size
//
// Returns the number of elements in the deque.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_GenericDeque_Size(CEL_NONNULL(const _cel_GenericDeque*) deq) {
  CEL_ASSERT_NOT_NULL(deq);
  return deq->len;
}

// _cel_GenericDeque_Empty
//
// Equivalent to `_cel_GenericDeque_Empty(arr) == 0`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_GenericDeque_Empty(
    CEL_NONNULL(const _cel_GenericDeque*) deq) {
  return _cel_GenericDeque_Size(deq) == 0;
}

// _cel_GenericDeque_At
//
// Access an element by its index. This is not preformed in constant time, so
// avoid doing so unless you absolutely need a deque.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_NULLABLE(const void*)
_cel_GenericDeque_At(CEL_NONNULL(const _cel_GenericDeque*) deq, size_t idx,
                     size_t ele_size);

// _cel_GenericDeque_MutableAt
//
// Access an element by its index. This is not preformed in constant time, so
// avoid doing so unless you absolutely need a deque.
static CEL_INLINE CEL_ATTRIBUTE_NODISCARD CEL_NULLABLE(void*)
    _cel_GenericDeque_MutableAt(CEL_NONNULL(_cel_GenericDeque*) deq, size_t idx,
                                size_t ele_size) {
  return (void*)_cel_GenericDeque_At(deq, idx, ele_size);
}

// _cel_GenericDeque_PushFrontAllocator
//
// Adds an element to the front of the deque, returning a pointer to it. Returns
// null if out of memory.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_NULLABLE(void*)
_cel_GenericDeque_PushFrontAllocator(CEL_NONNULL(_cel_GenericDeque*) deq,
                                     CEL_NONNULL(cel_Allocator*) alloc,
                                     size_t ele_size);

// _cel_GenericDeque_PushFrontArena
//
// Adds an element to the front of the deque, returning a pointer to it. Returns
// null if out of memory.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_NULLABLE(void*)
_cel_GenericDeque_PushFrontArena(CEL_NONNULL(_cel_GenericDeque*) deq,
                                 CEL_NONNULL(cel_Arena*) arena,
                                 size_t ele_size);

// _cel_GenericDeque_PopFrontAllocator
//
// Removes the element at the front of the deque.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericDeque_PopFrontAllocator(CEL_NONNULL(_cel_GenericDeque*) deq,
                                         CEL_NONNULL(cel_Allocator*) alloc,
                                         size_t ele_size);

// _cel_GenericDeque_PopFrontArena
//
// Removes the element at the front of the deque.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericDeque_PopFrontArena(CEL_NONNULL(_cel_GenericDeque*) deq);

// _cel_GenericDeque_PeekFront
//
// Returns a pointer to the constant element at the front of the deque.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_NONNULL(const void*)
_cel_GenericDeque_PeekFront(CEL_NONNULL(const _cel_GenericDeque*) deq,
                            size_t ele_size);

// _cel_GenericDeque_MutablePeekFront
//
// Returns a pointer to the mutable element at the front of the deque.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_GenericDeque_MutablePeekFront(CEL_NONNULL(_cel_GenericDeque*) deq,
                                       size_t ele_size) {
  return (CEL_NONNULL(void*))_cel_GenericDeque_PeekFront(deq, ele_size);
}

// _cel_GenericDeque_PushBackAllocator
//
// Adds an element to the back of the deque, returning a pointer to it. Returns
// null if out of memory.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_NULLABLE(void*)
_cel_GenericDeque_PushBackAllocator(CEL_NONNULL(_cel_GenericDeque*) deq,
                                    CEL_NONNULL(cel_Allocator*) alloc,
                                    size_t ele_size);

// _cel_GenericDeque_PushBackArena
//
// Adds an element to the back of the deque, returning a pointer to it. Returns
// null if out of memory.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_NULLABLE(void*)
_cel_GenericDeque_PushBackArena(CEL_NONNULL(_cel_GenericDeque*) deq,
                                CEL_NONNULL(cel_Arena*) arena, size_t ele_size);

// _cel_GenericDeque_PopBackAllocator
//
// Removes the element at the back of the deque.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericDeque_PopBackAllocator(CEL_NONNULL(_cel_GenericDeque*) deq,
                                        CEL_NONNULL(cel_Allocator*) alloc,
                                        size_t ele_size);

// _cel_GenericDeque_PopBackArena
//
// Removes the element at the back of the deque.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericDeque_PopBackArena(CEL_NONNULL(_cel_GenericDeque*) deq);

// _cel_GenericDeque_PeekBack
//
// Returns a pointer to the constant element at the back of the deque.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_NONNULL(const void*)
_cel_GenericDeque_PeekBack(CEL_NONNULL(const _cel_GenericDeque*) deq,
                           size_t ele_size);

// _cel_GenericDeque_MutablePeekBack
//
// Returns a pointer to the mutable element at the back of the deque.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_GenericDeque_MutablePeekBack(CEL_NONNULL(_cel_GenericDeque*) deq,
                                      size_t ele_size) {
  return (CEL_NONNULL(void*))_cel_GenericDeque_PeekBack(deq, ele_size);
}

// _cel_GenericDeque_ResetAllocator
//
// Resets the deque to its initial state just after construction.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericDeque_ResetAllocator(CEL_NONNULL(_cel_GenericDeque*) deq,
                                      CEL_NONNULL(cel_Allocator*) alloc,
                                      size_t ele_size);

// _cel_GenericDeque_ClearAllocator
//
// Clears the deque resulting in the size being 0.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericDeque_ClearAllocator(CEL_NONNULL(_cel_GenericDeque*) deq,
                                      CEL_NONNULL(cel_Allocator*) alloc,
                                      size_t ele_size);

// _cel_GenericDeque_ClearArena
//
// Clears the deque resulting in the size being 0.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericDeque_ClearArena(CEL_NONNULL(_cel_GenericDeque*) deq);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_SRC_GENERIC_DEQUE_H_
