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

// Internal header providing a deque implementation which uses arena-based
// memory management.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_ARENA_DEQUE_H_
#define THIRD_PARTY_CEL_C_INTERNAL_ARENA_DEQUE_H_

#include <stdalign.h>  // IWYU pragma: keep
#include <stdbool.h>   // IWYU pragma: keep
#include <stddef.h>

#include "cel-c/arena.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/generic_deque.h"

CEL_BEGIN_DECLS

#ifdef _cel_ArenaDeque
#error _cel_ArenaDeque cannot be directly set
#endif

typedef struct _cel_ArenaDeque {
  _cel_GenericDeque g;
} _cel_ArenaDeque;

CEL_STATIC_ASSERT(sizeof(_cel_ArenaDeque) == sizeof(_cel_GenericDeque));
CEL_STATIC_ASSERT(alignof(_cel_ArenaDeque) == alignof(_cel_GenericDeque));

#ifdef _cel_ArenaDeque_ValueType
#error _cel_ArenaDeque_ValueType cannot be directly set
#endif

#define _cel_ArenaDeque_ValueType(deq) cel_typeof_unqual(*(deq)->t)

#ifdef _cel_ArenaDeque_ValueSize
#error _cel_ArenaDeque_ValueSize cannot be directly set
#endif

#define _cel_ArenaDeque_ValueSize(deq) sizeof(_cel_ArenaDeque_ValueType(deq))

#ifdef _cel_ArenaDeque_Construct
#error _cel_ArenaDeque_Construct cannot be directly set
#endif

static CEL_INLINE void _cel_ArenaDeque_Construct(CEL_NONNULL(_cel_ArenaDeque*)
                                                     deq) {
  _cel_GenericDeque_Construct(&deq->g);
}

#define _cel_ArenaDeque_Construct(deq) _cel_ArenaDeque_Construct(&(deq)->v)

#ifdef _cel_ArenaDeque_Size
#error _cel_ArenaDeque_Size cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_ArenaDeque_Size(CEL_NONNULL(const _cel_ArenaDeque*) deq) {
  return _cel_GenericDeque_Size(&deq->g);
}

#define _cel_ArenaDeque_Size(deq) _cel_ArenaDeque_Size(&(deq)->v)

#ifdef _cel_ArenaDeque_Empty
#error _cel_ArenaDeque_Empty cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ArenaDeque_Empty(CEL_NONNULL(const _cel_ArenaDeque*)
                                                 deq) {
  return _cel_GenericDeque_Empty(&deq->g);
}

#define _cel_ArenaDeque_Empty(deq) _cel_ArenaDeque_Empty(&(deq)->v)

#ifdef _cel_ArenaDeque_At
#error _cel_ArenaDeque_At cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(const void*)
    _cel_ArenaDeque_At(CEL_NONNULL(const _cel_ArenaDeque*) deq, size_t idx,
                       size_t ele_size) {
  return _cel_GenericDeque_At(&deq->g, idx, ele_size);
}

#define _cel_ArenaDeque_At(deq, idx)                                        \
  ((CEL_NULLABLE(const _cel_ArenaDeque_ValueType(deq)*))_cel_ArenaDeque_At( \
      &(deq)->v, (idx), _cel_ArenaDeque_ValueSize(deq)))

#ifdef _cel_ArenaDeque_MutableAt
#error _cel_ArenaDeque_MutableAt cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_ArenaDeque_MutableAt(CEL_NONNULL(_cel_ArenaDeque*) deq, size_t idx,
                              size_t ele_size) {
  return _cel_GenericDeque_MutableAt(&deq->g, idx, ele_size);
}

#define _cel_ArenaDeque_MutableAt(deq, idx)                                  \
  ((CEL_NULLABLE(_cel_ArenaDeque_ValueType(deq)*))_cel_ArenaDeque_MutableAt( \
      &(deq)->v, (idx), _cel_ArenaDeque_ValueSize(deq)))

#ifdef _cel_ArenaDeque_PushBack
#error _cel_ArenaDeque_PushBack cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_ArenaDeque_PushBack(CEL_NONNULL(_cel_ArenaDeque*) deq,
                             CEL_NONNULL(cel_Arena*) arena, size_t ele_size) {
  return _cel_GenericDeque_PushBackArena(&deq->g, arena, ele_size);
}

#define _cel_ArenaDeque_PushBack(deq, arena)                                \
  ((CEL_NULLABLE(_cel_ArenaDeque_ValueType(deq)*))_cel_ArenaDeque_PushBack( \
      &(deq)->v, (arena), _cel_ArenaDeque_ValueSize(deq)))

#ifdef _cel_ArenaDeque_PopBack
#error _cel_ArenaDeque_PopBack cannot be directly set
#endif

static CEL_INLINE void _cel_ArenaDeque_PopBack(CEL_NONNULL(_cel_ArenaDeque*)
                                                   deq) {
  _cel_GenericDeque_PopBackArena(&deq->g);
}

#define _cel_ArenaDeque_PopBack(deq) _cel_ArenaDeque_PopBack(&(deq)->v)

#ifdef _cel_ArenaDeque_PeekBack
#error _cel_ArenaDeque_PeekBack cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const void*)
    _cel_ArenaDeque_PeekBack(CEL_NONNULL(const _cel_ArenaDeque*) deq,
                             size_t ele_size) {
  return _cel_GenericDeque_PeekBack(&deq->g, ele_size);
}

#define _cel_ArenaDeque_PeekBack(deq)                    \
  ((CEL_NULLABLE(const _cel_ArenaDeque_ValueType(deq)*)) \
       _cel_ArenaDeque_PeekBack(&(deq)->v, _cel_ArenaDeque_ValueSize(deq)))

#ifdef _cel_GenericDeque_MutablePeekBack
#error _cel_GenericDeque_MutablePeekBack cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_ArenaDeque_MutablePeekBack(CEL_NONNULL(_cel_ArenaDeque*) deq,
                                    size_t ele_size) {
  return _cel_GenericDeque_MutablePeekBack(&deq->g, ele_size);
}

#define _cel_ArenaDeque_MutablePeekBack(deq)            \
  ((CEL_NULLABLE(_cel_ArenaDeque_ValueType(             \
      deq)*))_cel_ArenaDeque_MutablePeekBack(&(deq)->v, \
                                             _cel_ArenaDeque_ValueSize(deq)))

#ifdef _cel_ArenaDeque_PushFront
#error _cel_ArenaDeque_PushFront cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_ArenaDeque_PushFront(CEL_NONNULL(_cel_ArenaDeque*) deq,
                              CEL_NONNULL(cel_Arena*) arena, size_t ele_size) {
  return _cel_GenericDeque_PushFrontArena(&deq->g, arena, ele_size);
}

#define _cel_ArenaDeque_PushFront(deq, arena)                                \
  ((CEL_NULLABLE(_cel_ArenaDeque_ValueType(deq)*))_cel_ArenaDeque_PushFront( \
      &(deq)->v, (arena), _cel_ArenaDeque_ValueSize(deq)))

#ifdef _cel_ArenaDeque_PopFront
#error _cel_ArenaDeque_PopFront cannot be directly set
#endif

static CEL_INLINE void _cel_ArenaDeque_PopFront(CEL_NONNULL(_cel_ArenaDeque*)
                                                    deq) {
  _cel_GenericDeque_PopFrontArena(&deq->g);
}

#define _cel_ArenaDeque_PopFront(deq) _cel_ArenaDeque_PopFront(&(deq)->v)

#ifdef _cel_ArenaDeque_PeekFront
#error _cel_ArenaDeque_PeekFront cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const void*)
    _cel_ArenaDeque_PeekFront(CEL_NONNULL(const _cel_ArenaDeque*) deq,
                              size_t ele_size) {
  return _cel_GenericDeque_PeekFront(&deq->g, ele_size);
}

#define _cel_ArenaDeque_PeekFront(deq)                   \
  ((CEL_NULLABLE(const _cel_ArenaDeque_ValueType(deq)*)) \
       _cel_ArenaDeque_PeekFront(&(deq)->v, _cel_ArenaDeque_ValueSize(deq)))

#ifdef _cel_GenericDeque_MutablePeekFront
#error _cel_GenericDeque_MutablePeekFront cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_ArenaDeque_MutablePeekFront(CEL_NONNULL(_cel_ArenaDeque*) deq,
                                     size_t ele_size) {
  return _cel_GenericDeque_MutablePeekFront(&deq->g, ele_size);
}

#define _cel_ArenaDeque_MutablePeekFront(deq)            \
  ((CEL_NULLABLE(_cel_ArenaDeque_ValueType(              \
      deq)*))_cel_ArenaDeque_MutablePeekFront(&(deq)->v, \
                                              _cel_ArenaDeque_ValueSize(deq)))

#ifdef _cel_ArenaDeque_Clear
#error _cel_ArenaDeque_Clear cannot be directly set
#endif

static CEL_INLINE void _cel_ArenaDeque_Clear(CEL_NONNULL(_cel_ArenaDeque*)
                                                 deq) {
  _cel_GenericDeque_ClearArena(&deq->g);
}

#define _cel_ArenaDeque_Clear(deq) _cel_ArenaDeque_Clear(&(deq)->v)

#define _cel_ArenaDeque(type)         \
  union {                             \
    CEL_NULLABILITY_UNKNOWN(type*) t; \
    _cel_ArenaDeque v;                \
  }

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_ARENA_DEQUE_H_
