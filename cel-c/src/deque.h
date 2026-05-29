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

// Internal header providing a deque implementation which uses allocator-based
// memory management.

#ifndef THIRD_PARTY_CEL_C_SRC_DEQUE_H_
#define THIRD_PARTY_CEL_C_SRC_DEQUE_H_

#include <stdalign.h>  // IWYU pragma: keep
#include <stdbool.h>   // IWYU pragma: keep
#include <stddef.h>

#include "cel-c/alloc.h"
#include "cel-c/config.h"
#include "cel-c/src/generic_deque.h"

CEL_BEGIN_DECLS

#ifdef _cel_Deque
#error _cel_Deque cannot be directly set
#endif

typedef struct _cel_Deque {
  _cel_GenericDeque g;
} _cel_Deque;

CEL_STATIC_ASSERT(sizeof(_cel_Deque) == sizeof(_cel_GenericDeque));
CEL_STATIC_ASSERT(alignof(_cel_Deque) == alignof(_cel_GenericDeque));

#ifdef _cel_Deque_ValueType
#error _cel_Deque_ValueType cannot be directly set
#endif

#define _cel_Deque_ValueType(deq) cel_typeof_unqual(*(deq)->t)

#ifdef _cel_Deque_ValueSize
#error _cel_Deque_ValueSize cannot be directly set
#endif

#define _cel_Deque_ValueSize(deq) sizeof(_cel_Deque_ValueType(deq))

#ifdef _cel_Deque_Construct
#error _cel_Deque_Construct cannot be directly set
#endif

static CEL_INLINE void _cel_Deque_Construct(CEL_NONNULL(_cel_Deque*) deq) {
  _cel_GenericDeque_Construct(&deq->g);
}

#define _cel_Deque_Construct(deq) _cel_Deque_Construct(&(deq)->v)

#ifdef _cel_Deque_Destruct
#error _cel_Deque_Destruct cannot be directly set
#endif

static CEL_INLINE void _cel_Deque_Destruct(CEL_NONNULL(_cel_Deque*) deq,
                                           CEL_NONNULL(cel_Allocator*) alloc,
                                           size_t ele_size) {
  _cel_GenericDeque_DestructAllocator(&deq->g, alloc, ele_size);
}

#define _cel_Deque_Destruct(deq, alloc) \
  _cel_Deque_Destruct(&(deq)->v, (alloc), _cel_Deque_ValueSize(deq))

#ifdef _cel_Deque_Size
#error _cel_Deque_Size cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t _cel_Deque_Size(CEL_NONNULL(const _cel_Deque*) deq) {
  return _cel_GenericDeque_Size(&deq->g);
}

#define _cel_Deque_Size(deq) _cel_Deque_Size(&(deq)->v)

#ifdef _cel_Deque_Empty
#error _cel_Deque_Empty cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_Deque_Empty(CEL_NONNULL(const _cel_Deque*) deq) {
  return _cel_GenericDeque_Empty(&deq->g);
}

#define _cel_Deque_Empty(deq) _cel_Deque_Empty(&(deq)->v)

#ifdef _cel_Deque_At
#error _cel_Deque_At cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(const void*)
    _cel_Deque_At(CEL_NONNULL(const _cel_Deque*) deq, size_t idx,
                  size_t ele_size) {
  return _cel_GenericDeque_At(&deq->g, idx, ele_size);
}

#define _cel_Deque_At(deq, idx)                                   \
  ((CEL_NULLABLE(const _cel_Deque_ValueType(deq)*))_cel_Deque_At( \
      &(deq)->v, (idx), _cel_Deque_ValueSize(deq)))

#ifdef _cel_Deque_MutableAt
#error _cel_Deque_MutableAt cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_Deque_MutableAt(CEL_NONNULL(_cel_Deque*) deq, size_t idx,
                         size_t ele_size) {
  return _cel_GenericDeque_MutableAt(&deq->g, idx, ele_size);
}

#define _cel_Deque_MutableAt(deq, idx)                             \
  ((CEL_NULLABLE(_cel_Deque_ValueType(deq)*))_cel_Deque_MutableAt( \
      &(deq)->v, (idx), _cel_Deque_ValueSize(deq)))

#ifdef _cel_Deque_PushBack
#error _cel_Deque_PushBack cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_Deque_PushBack(CEL_NONNULL(_cel_Deque*) deq,
                        CEL_NONNULL(cel_Allocator*) alloc, size_t ele_size) {
  return _cel_GenericDeque_PushBackAllocator(&deq->g, alloc, ele_size);
}

#define _cel_Deque_PushBack(deq, alloc)                           \
  ((CEL_NULLABLE(_cel_Deque_ValueType(deq)*))_cel_Deque_PushBack( \
      &(deq)->v, (alloc), _cel_Deque_ValueSize(deq)))

#ifdef _cel_Deque_PopBack
#error _cel_Deque_PopBack cannot be directly set
#endif

static CEL_INLINE void _cel_Deque_PopBack(CEL_NONNULL(_cel_Deque*) deq,
                                          CEL_NONNULL(cel_Allocator*) alloc,
                                          size_t ele_size) {
  _cel_GenericDeque_PopBackAllocator(&deq->g, alloc, ele_size);
}

#define _cel_Deque_PopBack(deq, alloc) \
  _cel_Deque_PopBack(&(deq)->v, (alloc), _cel_Deque_ValueSize(deq))

#ifdef _cel_Deque_PeekBack
#error _cel_Deque_PeekBack cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const void*)
    _cel_Deque_PeekBack(CEL_NONNULL(const _cel_Deque*) deq, size_t ele_size) {
  return _cel_GenericDeque_PeekBack(&deq->g, ele_size);
}

#define _cel_Deque_PeekBack(deq)                                        \
  ((CEL_NULLABLE(const _cel_Deque_ValueType(deq)*))_cel_Deque_PeekBack( \
      &(deq)->v, _cel_Deque_ValueSize(deq)))

#ifdef _cel_GenericDeque_MutablePeekBack
#error _cel_GenericDeque_MutablePeekBack cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_Deque_MutablePeekBack(CEL_NONNULL(_cel_Deque*) deq, size_t ele_size) {
  return _cel_GenericDeque_MutablePeekBack(&deq->g, ele_size);
}

#define _cel_Deque_MutablePeekBack(deq)                                  \
  ((CEL_NULLABLE(_cel_Deque_ValueType(deq)*))_cel_Deque_MutablePeekBack( \
      &(deq)->v, _cel_Deque_ValueSize(deq)))

#ifdef _cel_Deque_PushFront
#error _cel_Deque_PushFront cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_Deque_PushFront(CEL_NONNULL(_cel_Deque*) deq,
                         CEL_NONNULL(cel_Allocator*) alloc, size_t ele_size) {
  return _cel_GenericDeque_PushFrontAllocator(&deq->g, alloc, ele_size);
}

#define _cel_Deque_PushFront(deq, alloc)                           \
  ((CEL_NULLABLE(_cel_Deque_ValueType(deq)*))_cel_Deque_PushFront( \
      &(deq)->v, (alloc), _cel_Deque_ValueSize(deq)))

#ifdef _cel_Deque_PopFront
#error _cel_Deque_PopFront cannot be directly set
#endif

static CEL_INLINE void _cel_Deque_PopFront(CEL_NONNULL(_cel_Deque*) deq,
                                           CEL_NONNULL(cel_Allocator*) alloc,
                                           size_t ele_size) {
  _cel_GenericDeque_PopFrontAllocator(&deq->g, alloc, ele_size);
}

#define _cel_Deque_PopFront(deq, alloc) \
  _cel_Deque_PopFront(&(deq)->v, (alloc), _cel_Deque_ValueSize(deq))

#ifdef _cel_Deque_PeekFront
#error _cel_Deque_PeekFront cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const void*)
    _cel_Deque_PeekFront(CEL_NONNULL(const _cel_Deque*) deq, size_t ele_size) {
  return _cel_GenericDeque_PeekFront(&deq->g, ele_size);
}

#define _cel_Deque_PeekFront(deq)                                        \
  ((CEL_NULLABLE(const _cel_Deque_ValueType(deq)*))_cel_Deque_PeekFront( \
      &(deq)->v, _cel_Deque_ValueSize(deq)))

#ifdef _cel_GenericDeque_MutablePeekFront
#error _cel_GenericDeque_MutablePeekFront cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_Deque_MutablePeekFront(CEL_NONNULL(_cel_Deque*) deq, size_t ele_size) {
  return _cel_GenericDeque_MutablePeekFront(&deq->g, ele_size);
}

#define _cel_Deque_MutablePeekFront(deq)                                  \
  ((CEL_NULLABLE(_cel_Deque_ValueType(deq)*))_cel_Deque_MutablePeekFront( \
      &(deq)->v, _cel_Deque_ValueSize(deq)))

#ifdef _cel_Deque_Reset
#error _cel_Deque_Reset cannot be directly set
#endif

static CEL_INLINE void _cel_Deque_Reset(CEL_NONNULL(_cel_Deque*) deq,
                                        CEL_NONNULL(cel_Allocator*) alloc,
                                        size_t ele_size) {
  _cel_GenericDeque_ResetAllocator(&deq->g, alloc, ele_size);
}

#define _cel_Deque_Reset(deq, alloc) \
  _cel_Deque_Reset(&(deq)->v, (alloc), _cel_Deque_ValueSize(deq))

#ifdef _cel_Deque_Clear
#error _cel_Deque_Clear cannot be directly set
#endif

static CEL_INLINE void _cel_Deque_Clear(CEL_NONNULL(_cel_Deque*) deq,
                                        CEL_NONNULL(cel_Allocator*) alloc,
                                        size_t ele_size) {
  _cel_GenericDeque_ClearAllocator(&deq->g, alloc, ele_size);
}

#define _cel_Deque_Clear(deq, alloc) \
  _cel_Deque_Clear(&(deq)->v, (alloc), _cel_Deque_ValueSize(deq))

#define _cel_Deque(type)              \
  union {                             \
    CEL_NULLABILITY_UNKNOWN(type*) t; \
    _cel_Deque v;                     \
  }

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_SRC_DEQUE_H_
