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

// Internal header providing a dynamic array implementation which uses
// arena-based memory management.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_ARENA_ARRAY_H_
#define THIRD_PARTY_CEL_C_INTERNAL_ARENA_ARRAY_H_

#include <stdalign.h>  // IWYU pragma: keep
#include <stdbool.h>   // IWYU pragma: keep
#include <stddef.h>

#include "cel-c/arena.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/generic_array.h"

CEL_BEGIN_DECLS

#ifdef _cel_ArenaArray
#error _cel_ArenaArray cannot be directly set
#endif

typedef struct _cel_ArenaArray {
  _cel_GenericArray g;
} _cel_ArenaArray;

CEL_STATIC_ASSERT(sizeof(_cel_ArenaArray) == sizeof(_cel_GenericArray));
CEL_STATIC_ASSERT(alignof(_cel_ArenaArray) == alignof(_cel_GenericArray));

#ifdef _cel_ArenaArray_ValueType
#error _cel_ArenaArray_ValueType cannot be directly set
#endif

#define _cel_ArenaArray_ValueType(arr) cel_typeof_unqual(*(arr)->t)

#ifdef _cel_ArenaArray_ValueSize
#error _cel_ArenaArray_ValueSize cannot be directly set
#endif

#define _cel_ArenaArray_ValueSize(arr) sizeof(_cel_ArenaArray_ValueType(arr))

#ifdef _cel_ArenaArray_Construct
#error _cel_ArenaArray_Construct cannot be directly set
#endif

static CEL_INLINE void _cel_ArenaArray_Construct(CEL_NONNULL(_cel_ArenaArray*)
                                                     arr) {
  _cel_GenericArray_Construct(&arr->g);
}

#define _cel_ArenaArray_Construct(arr) _cel_ArenaArray_Construct(&(arr)->v)

#ifdef _cel_ArenaArray_Size
#error _cel_ArenaArray_Size cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_ArenaArray_Size(CEL_NONNULL(const _cel_ArenaArray*) arr) {
  return _cel_GenericArray_Size(&arr->g);
}

#define _cel_ArenaArray_Size(arr) _cel_ArenaArray_Size(&(arr)->v)

#ifdef _cel_ArenaArray_Empty
#error _cel_ArenaArray_Empty cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ArenaArray_Empty(CEL_NONNULL(const _cel_ArenaArray*)
                                                 arr) {
  return _cel_GenericArray_Empty(&arr->g);
}

#define _cel_ArenaArray_Empty(arr) _cel_ArenaArray_Empty(&(arr)->v)

#ifdef _cel_ArenaArray_Capacity
#error _cel_ArenaArray_Capacity cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_ArenaArray_Capacity(CEL_NONNULL(const _cel_ArenaArray*) arr) {
  return _cel_GenericArray_Capacity(&arr->g);
}

#define _cel_ArenaArray_Capacity(arr) _cel_ArenaArray_Capacity(&(arr)->v)

#ifdef _cel_ArenaArray_Data
#error _cel_ArenaArray_Data cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(const void*)
    _cel_ArenaArray_Data(CEL_NONNULL(const _cel_ArenaArray*) arr) {
  return _cel_GenericArray_Data(&arr->g);
}

#define _cel_ArenaArray_Data(arr)                                             \
  ((CEL_NULLABLE(const _cel_ArenaArray_ValueType(arr)*))_cel_ArenaArray_Data( \
      &(arr)->v))

#ifdef _cel_ArenaArray_MutableData
#error _cel_ArenaArray_MutableData cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_ArenaArray_MutableData(CEL_NONNULL(_cel_ArenaArray*) arr) {
  return _cel_GenericArray_MutableData(&arr->g);
}

#define _cel_ArenaArray_MutableData(arr)                                       \
  ((CEL_NULLABLE(_cel_ArenaArray_ValueType(arr)*))_cel_ArenaArray_MutableData( \
      &(arr)->v))

#ifdef _cel_ArenaArray_At
#error _cel_ArenaArray_At cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const void*)
    _cel_ArenaArray_At(CEL_NONNULL(const _cel_ArenaArray*) arr, size_t idx,
                       size_t ele_size) {
  return _cel_GenericArray_At(&arr->g, idx, ele_size);
}

#define _cel_ArenaArray_At(arr, idx)                                       \
  ((CEL_NONNULL(const _cel_ArenaArray_ValueType(arr)*))_cel_ArenaArray_At( \
      &(arr)->v, (idx), _cel_ArenaArray_ValueSize(arr)))

#ifdef _cel_ArenaArray_MutableAt
#error _cel_ArenaArray_MutableAt cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_ArenaArray_MutableAt(CEL_NONNULL(_cel_ArenaArray*) arr, size_t idx,
                              size_t ele_size) {
  return _cel_GenericArray_MutableAt(&arr->g, idx, ele_size);
}

#define _cel_ArenaArray_MutableAt(arr, idx)                                 \
  ((CEL_NONNULL(_cel_ArenaArray_ValueType(arr)*))_cel_ArenaArray_MutableAt( \
      &(arr)->v, (idx), _cel_ArenaArray_ValueSize(arr)))

#ifdef _cel_ArenaArray_Front
#error _cel_ArenaArray_Front cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const void*)
    _cel_ArenaArray_Front(CEL_NONNULL(const _cel_ArenaArray*) arr) {
  return _cel_GenericArray_Front(&arr->g);
}

#define _cel_ArenaArray_Front(arr)                                            \
  ((CEL_NONNULL(const _cel_ArenaArray_ValueType(arr)*))_cel_ArenaArray_Front( \
      &(arr)->v))

#ifdef _cel_ArenaArray_MutableFront
#error _cel_ArenaArray_MutableFront cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_ArenaArray_MutableFront(CEL_NONNULL(_cel_ArenaArray*) arr) {
  return _cel_GenericArray_MutableFront(&arr->g);
}

#define _cel_ArenaArray_MutableFront(arr)                                      \
  ((CEL_NONNULL(_cel_ArenaArray_ValueType(arr)*))_cel_ArenaArray_MutableFront( \
      &(arr)->v))

#ifdef _cel_ArenaArray_Back
#error _cel_ArenaArray_Back cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const void*)
    _cel_ArenaArray_Back(CEL_NONNULL(const _cel_ArenaArray*) arr,
                         size_t ele_size) {
  return _cel_GenericArray_Back(&arr->g, ele_size);
}

#define _cel_ArenaArray_Back(arr)                                            \
  ((CEL_NONNULL(const _cel_ArenaArray_ValueType(arr)*))_cel_ArenaArray_Back( \
      &(arr)->v, _cel_ArenaArray_ValueSize(arr)))

#ifdef _cel_ArenaArray_MutableBack
#error _cel_ArenaArray_MutableBack cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_ArenaArray_MutableBack(CEL_NONNULL(_cel_ArenaArray*) arr,
                                size_t ele_size) {
  return _cel_GenericArray_MutableBack(&arr->g, ele_size);
}

#define _cel_ArenaArray_MutableBack(arr)                                      \
  ((CEL_NONNULL(_cel_ArenaArray_ValueType(arr)*))_cel_ArenaArray_MutableBack( \
      &(arr)->v, _cel_ArenaArray_ValueSize(arr)))

#ifdef _cel_ArenaArray_Reserve
#error _cel_ArenaArray_Reserve cannot be directly set
#endif

static CEL_INLINE bool _cel_ArenaArray_Reserve(CEL_NONNULL(_cel_ArenaArray*)
                                                   arr,
                                               CEL_NONNULL(cel_Arena*) arena,
                                               size_t new_cap,
                                               size_t ele_size) {
  return _cel_GenericArray_ReserveArena(&arr->g, arena, new_cap, ele_size);
}

#define _cel_ArenaArray_Reserve(arr, arena, new_capacity)     \
  _cel_ArenaArray_Reserve(&(arr)->v, (arena), (new_capacity), \
                          _cel_ArenaArray_ValueSize(arr))

#ifdef _cel_ArenaArray_Resize
#error _cel_ArenaArray_Resize cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_ArenaArray_Resize(CEL_NONNULL(_cel_ArenaArray*) arr,
                           CEL_NONNULL(cel_Arena*) arena, size_t new_len,
                           size_t ele_size) {
  return _cel_GenericArray_ResizeArena(&arr->g, arena, new_len, ele_size);
}

#define _cel_ArenaArray_Resize(arr, arena, new_size)                      \
  ((CEL_NULLABLE(_cel_ArenaArray_ValueType(arr)*))_cel_ArenaArray_Resize( \
      &(arr)->v, (arena), (new_size), _cel_ArenaArray_ValueSize(arr)))

#ifdef _cel_ArenaArray_Push
#error _cel_ArenaArray_Push cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_ArenaArray_Push(CEL_NONNULL(_cel_ArenaArray*) arr,
                         CEL_NONNULL(cel_Arena*) arena, size_t ele_size) {
  return _cel_GenericArray_PushArena(&arr->g, arena, ele_size);
}

#define _cel_ArenaArray_Push(arr, arena)                                \
  ((CEL_NULLABLE(_cel_ArenaArray_ValueType(arr)*))_cel_ArenaArray_Push( \
      &(arr)->v, (arena), _cel_ArenaArray_ValueSize(arr)))

#ifdef _cel_ArenaArray_Append
#error _cel_ArenaArray_Append cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_ArenaArray_Append(CEL_NONNULL(_cel_ArenaArray*) arr,
                           CEL_NONNULL(cel_Arena*) arena, size_t n,
                           size_t ele_size) {
  return _cel_GenericArray_AppendArena(&arr->g, arena, n, ele_size);
}

#define _cel_ArenaArray_Append(arr, arena, n)                             \
  ((CEL_NULLABLE(_cel_ArenaArray_ValueType(arr)*))_cel_ArenaArray_Append( \
      &(arr)->v, (arena), (n), _cel_ArenaArray_ValueSize(arr)))

#ifdef _cel_ArenaArray_Pop
#error _cel_ArenaArray_Pop cannot be directly set
#endif

static CEL_INLINE void _cel_ArenaArray_Pop(CEL_NONNULL(_cel_ArenaArray*) arr) {
  _cel_GenericArray_PopArena(&arr->g);
}

#define _cel_ArenaArray_Pop(arr) _cel_ArenaArray_Pop(&(arr)->v)

#ifdef _cel_ArenaArray_Erase
#error _cel_ArenaArray_Erase cannot be directly set
#endif

static CEL_INLINE void _cel_ArenaArray_Erase(CEL_NONNULL(_cel_ArenaArray*) arr,
                                             size_t idx, size_t ele_size) {
  _cel_GenericArray_EraseArena(&arr->g, idx, ele_size);
}

#define _cel_ArenaArray_Erase(arr, idx) \
  _cel_ArenaArray_Erase(&(arr)->v, (idx), _cel_ArenaArray_ValueSize(arr))

#ifdef _cel_ArenaArray_Clear
#error _cel_ArenaArray_Clear cannot be directly set
#endif

static CEL_INLINE void _cel_ArenaArray_Clear(CEL_NONNULL(_cel_ArenaArray*)
                                                 arr) {
  _cel_GenericArray_ClearArena(&arr->g);
}

#define _cel_ArenaArray_Clear(arr) _cel_ArenaArray_Clear(&(arr)->v)

#ifdef _cel_ArenaArray_Begin
#error _cel_ArenaArray_Begin cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(const void*)
    _cel_ArenaArray_Begin(CEL_NONNULL(const _cel_ArenaArray*) arr) {
  return _cel_GenericArray_Begin(&arr->g);
}

#define _cel_ArenaArray_Begin(arr)                                             \
  ((CEL_NULLABLE(const _cel_ArenaArray_ValueType(arr)*))_cel_ArenaArray_Begin( \
      &(arr)->v))

#ifdef _cel_ArenaArray_MutableBegin
#error _cel_ArenaArray_MutableBegin cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_ArenaArray_MutableBegin(CEL_NONNULL(_cel_ArenaArray*) arr) {
  return _cel_GenericArray_MutableBegin(&arr->g);
}

#define _cel_ArenaArray_MutableBegin(arr)   \
  ((CEL_NULLABLE(_cel_ArenaArray_ValueType( \
      arr)*))_cel_ArenaArray_MutableBegin(&(arr)->v))

#ifdef _cel_ArenaArray_End
#error _cel_ArenaArray_End cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(const void*)
    _cel_ArenaArray_End(CEL_NONNULL(const _cel_ArenaArray*) arr,
                        size_t ele_size) {
  return _cel_GenericArray_End(&arr->g, ele_size);
}

#define _cel_ArenaArray_End(arr)                                             \
  ((CEL_NULLABLE(const _cel_ArenaArray_ValueType(arr)*))_cel_ArenaArray_End( \
      &(arr)->v, _cel_ArenaArray_ValueSize(arr)))

#ifdef _cel_ArenaArray_MutableEnd
#error _cel_ArenaArray_MutableEnd cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_ArenaArray_MutableEnd(CEL_NONNULL(_cel_ArenaArray*) arr,
                               size_t ele_size) {
  return _cel_GenericArray_MutableEnd(&arr->g, ele_size);
}

#define _cel_ArenaArray_MutableEnd(arr)                                       \
  ((CEL_NULLABLE(_cel_ArenaArray_ValueType(arr)*))_cel_ArenaArray_MutableEnd( \
      &(arr)->v, _cel_ArenaArray_ValueSize(arr)))

#define _cel_ArenaArray(type)         \
  union {                             \
    CEL_NULLABILITY_UNKNOWN(type*) t; \
    _cel_ArenaArray v;                \
  }

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_ARENA_ARRAY_H_
