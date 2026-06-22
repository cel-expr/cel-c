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
// allocator-based memory management.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_ARRAY_H_
#define THIRD_PARTY_CEL_C_INTERNAL_ARRAY_H_

#include <stdalign.h>  // IWYU pragma: keep
#include <stdbool.h>   // IWYU pragma: keep
#include <stddef.h>

#include "cel-c/alloc.h"
#include "cel-c/config.h"
#include "cel-c/internal/generic_array.h"

CEL_BEGIN_DECLS

#ifdef _cel_Array
#error _cel_Array cannot be directly set
#endif

typedef struct _cel_Array {
  _cel_GenericArray g;
} _cel_Array;

CEL_STATIC_ASSERT(sizeof(_cel_Array) == sizeof(_cel_GenericArray));
CEL_STATIC_ASSERT(alignof(_cel_Array) == alignof(_cel_GenericArray));

#ifdef _cel_Array_ValueType
#error _cel_Array_ValueType cannot be directly set
#endif

#define _cel_Array_ValueType(arr) cel_typeof_unqual(*(arr)->t)

#ifdef _cel_Array_ValueSize
#error _cel_Array_ValueSize cannot be directly set
#endif

#define _cel_Array_ValueSize(arr) sizeof(_cel_Array_ValueType(arr))

#ifdef _cel_Array_Construct
#error _cel_Array_Construct cannot be directly set
#endif

static CEL_INLINE void _cel_Array_Construct(CEL_NONNULL(_cel_Array*) arr) {
  _cel_GenericArray_Construct(&arr->g);
}

#define _cel_Array_Construct(arr) _cel_Array_Construct(&(arr)->v)

#ifdef _cel_Array_Destruct
#error _cel_Array_Destruct cannot be directly set
#endif

static CEL_INLINE void _cel_Array_Destruct(CEL_NONNULL(_cel_Array*) arr,
                                           CEL_NONNULL(cel_Allocator*) alloc,
                                           size_t ele_size) {
  _cel_GenericArray_DestructAllocator(&arr->g, alloc, ele_size);
}

#define _cel_Array_Destruct(arr, alloc) \
  _cel_Array_Destruct(&(arr)->v, (alloc), _cel_Array_ValueSize(arr))

#ifdef _cel_Array_Size
#error _cel_Array_Size cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t _cel_Array_Size(CEL_NONNULL(const _cel_Array*) arr) {
  return _cel_GenericArray_Size(&arr->g);
}

#define _cel_Array_Size(arr) _cel_Array_Size(&(arr)->v)

#ifdef _cel_Array_Empty
#error _cel_Array_Empty cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_Array_Empty(CEL_NONNULL(const _cel_Array*) arr) {
  return _cel_GenericArray_Empty(&arr->g);
}

#define _cel_Array_Empty(arr) _cel_Array_Empty(&(arr)->v)

#ifdef _cel_Array_Capacity
#error _cel_Array_Capacity cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t _cel_Array_Capacity(CEL_NONNULL(const _cel_Array*)
                                                 arr) {
  return _cel_GenericArray_Capacity(&arr->g);
}

#define _cel_Array_Capacity(arr) _cel_Array_Capacity(&(arr)->v)

#ifdef _cel_Array_Data
#error _cel_Array_Data cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(const void*)
    _cel_Array_Data(CEL_NONNULL(const _cel_Array*) arr) {
  return _cel_GenericArray_Data(&arr->g);
}

#define _cel_Array_Data(arr) \
  ((CEL_NULLABLE(const _cel_Array_ValueType(arr)*))_cel_Array_Data(&(arr)->v))

#ifdef _cel_Array_MutableData
#error _cel_Array_MutableData cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_Array_MutableData(CEL_NONNULL(_cel_Array*) arr) {
  return _cel_GenericArray_MutableData(&arr->g);
}

#define _cel_Array_MutableData(arr) \
  ((CEL_NULLABLE(_cel_Array_ValueType(arr)*))_cel_Array_MutableData(&(arr)->v))

#ifdef _cel_Array_At
#error _cel_Array_At cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const void*)
    _cel_Array_At(CEL_NONNULL(const _cel_Array*) arr, size_t idx,
                  size_t ele_size) {
  return _cel_GenericArray_At(&arr->g, idx, ele_size);
}

#define _cel_Array_At(arr, idx)                                  \
  ((CEL_NONNULL(const _cel_Array_ValueType(arr)*))_cel_Array_At( \
      &(arr)->v, (idx), _cel_Array_ValueSize(arr)))

#ifdef _cel_Array_MutableAt
#error _cel_Array_MutableAt cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_Array_MutableAt(CEL_NONNULL(_cel_Array*) arr, size_t idx,
                         size_t ele_size) {
  return _cel_GenericArray_MutableAt(&arr->g, idx, ele_size);
}

#define _cel_Array_MutableAt(arr, idx)                            \
  ((CEL_NONNULL(_cel_Array_ValueType(arr)*))_cel_Array_MutableAt( \
      &(arr)->v, (idx), _cel_Array_ValueSize(arr)))

#ifdef _cel_Array_Front
#error _cel_Array_Front cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const void*)
    _cel_Array_Front(CEL_NONNULL(const _cel_Array*) arr) {
  return _cel_GenericArray_Front(&arr->g);
}

#define _cel_Array_Front(arr) \
  ((CEL_NONNULL(const _cel_Array_ValueType(arr)*))_cel_Array_Front(&(arr)->v))

#ifdef _cel_Array_MutableFront
#error _cel_Array_MutableFront cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_Array_MutableFront(CEL_NONNULL(_cel_Array*) arr) {
  return _cel_GenericArray_MutableFront(&arr->g);
}

#define _cel_Array_MutableFront(arr) \
  ((CEL_NONNULL(_cel_Array_ValueType(arr)*))_cel_Array_MutableFront(&(arr)->v))

#ifdef _cel_Array_Back
#error _cel_Array_Back cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const void*)
    _cel_Array_Back(CEL_NONNULL(const _cel_Array*) arr, size_t ele_size) {
  return _cel_GenericArray_Back(&arr->g, ele_size);
}

#define _cel_Array_Back(arr)                                       \
  ((CEL_NONNULL(const _cel_Array_ValueType(arr)*))_cel_Array_Back( \
      &(arr)->v, _cel_Array_ValueSize(arr)))

#ifdef _cel_Array_MutableBack
#error _cel_Array_MutableBack cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(void*)
    _cel_Array_MutableBack(CEL_NONNULL(_cel_Array*) arr, size_t ele_size) {
  return _cel_GenericArray_MutableBack(&arr->g, ele_size);
}

#define _cel_Array_MutableBack(arr)                                 \
  ((CEL_NONNULL(_cel_Array_ValueType(arr)*))_cel_Array_MutableBack( \
      &(arr)->v, _cel_Array_ValueSize(arr)))

#ifdef _cel_Array_Reserve
#error _cel_Array_Reserve cannot be directly set
#endif

static CEL_INLINE bool _cel_Array_Reserve(CEL_NONNULL(_cel_Array*) arr,
                                          CEL_NONNULL(cel_Allocator*) alloc,
                                          size_t new_cap, size_t ele_size) {
  return _cel_GenericArray_ReserveAllocator(&arr->g, alloc, new_cap, ele_size);
}

#define _cel_Array_Reserve(arr, alloc, new_capacity)     \
  _cel_Array_Reserve(&(arr)->v, (alloc), (new_capacity), \
                     _cel_Array_ValueSize(arr))

#ifdef _cel_Array_ShrinkToFit
#error _cel_Array_ShrinkToFit cannot be directly set
#endif

static CEL_INLINE void _cel_Array_ShrinkToFit(CEL_NONNULL(_cel_Array*) arr,
                                              CEL_NONNULL(cel_Allocator*) alloc,
                                              size_t ele_size) {
  _cel_GenericArray_ShrinkToFitAllocator(&arr->g, alloc, ele_size);
}

#define _cel_Array_ShrinkToFit(arr, alloc) \
  _cel_Array_ShrinkToFit(&(arr)->v, (alloc), _cel_Array_ValueSize(arr))

#ifdef _cel_Array_Resize
#error _cel_Array_Resize cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_Array_Resize(CEL_NONNULL(_cel_Array*) arr,
                      CEL_NONNULL(cel_Allocator*) alloc, size_t new_len,
                      size_t ele_size) {
  return _cel_GenericArray_ResizeAllocator(&arr->g, alloc, new_len, ele_size);
}

#define _cel_Array_Resize(arr, alloc, new_size)                 \
  ((CEL_NULLABLE(_cel_Array_ValueType(arr)*))_cel_Array_Resize( \
      &(arr)->v, (alloc), (new_size), _cel_Array_ValueSize(arr)))

#ifdef _cel_Array_Push
#error _cel_Array_Push cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_Array_Push(CEL_NONNULL(_cel_Array*) arr,
                    CEL_NONNULL(cel_Allocator*) alloc, size_t ele_size) {
  return _cel_GenericArray_PushAllocator(&arr->g, alloc, ele_size);
}

#define _cel_Array_Push(arr, alloc)                           \
  ((CEL_NULLABLE(_cel_Array_ValueType(arr)*))_cel_Array_Push( \
      &(arr)->v, (alloc), _cel_Array_ValueSize(arr)))

#ifdef _cel_Array_Append
#error _cel_Array_Append cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_Array_Append(CEL_NONNULL(_cel_Array*) arr,
                      CEL_NONNULL(cel_Allocator*) alloc, size_t n,
                      size_t ele_size) {
  return _cel_GenericArray_AppendAllocator(&arr->g, alloc, n, ele_size);
}

#define _cel_Array_Append(arr, alloc, n)                        \
  ((CEL_NULLABLE(_cel_Array_ValueType(arr)*))_cel_Array_Append( \
      &(arr)->v, (alloc), (n), _cel_Array_ValueSize(arr)))

#ifdef _cel_Array_Pop
#error _cel_Array_Pop cannot be directly set
#endif

static CEL_INLINE void _cel_Array_Pop(CEL_NONNULL(_cel_Array*) arr,
                                      size_t ele_size) {
  _cel_GenericArray_PopAllocator(&arr->g, ele_size);
}

#define _cel_Array_Pop(arr) _cel_Array_Pop(&(arr)->v, _cel_Array_ValueSize(arr))

#ifdef _cel_Array_Erase
#error _cel_Array_Erase cannot be directly set
#endif

static CEL_INLINE void _cel_Array_Erase(CEL_NONNULL(_cel_Array*) arr,
                                        size_t idx, size_t ele_size) {
  _cel_GenericArray_EraseAllocator(&arr->g, idx, ele_size);
}

#define _cel_Array_Erase(arr, idx) \
  _cel_Array_Erase(&(arr)->v, (idx), _cel_Array_ValueSize(arr))

#ifdef _cel_Array_Clear
#error _cel_Array_Clear cannot be directly set
#endif

static CEL_INLINE void _cel_Array_Clear(CEL_NONNULL(_cel_Array*) arr,
                                        size_t ele_size) {
  _cel_GenericArray_ClearAllocator(&arr->g, ele_size);
}

#define _cel_Array_Clear(arr) \
  _cel_Array_Clear(&(arr)->v, _cel_Array_ValueSize(arr))

#ifdef _cel_Array_Reset
#error _cel_Array_Reset cannot be directly set
#endif

static CEL_INLINE void _cel_Array_Reset(CEL_NONNULL(_cel_Array*) arr,
                                        CEL_NONNULL(cel_Allocator*) alloc,
                                        size_t ele_size) {
  _cel_GenericArray_ResetAllocator(&arr->g, alloc, ele_size);
}

#define _cel_Array_Reset(arr, alloc) \
  _cel_Array_Reset(&(arr)->v, (alloc), _cel_Array_ValueSize(arr))

#ifdef _cel_Array_Begin
#error _cel_Array_Begin cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(const void*)
    _cel_Array_Begin(CEL_NONNULL(const _cel_Array*) arr) {
  return _cel_GenericArray_Begin(&arr->g);
}

#define _cel_Array_Begin(arr) \
  ((CEL_NULLABLE(const _cel_Array_ValueType(arr)*))_cel_Array_Begin(&(arr)->v))

#ifdef _cel_Array_MutableBegin
#error _cel_Array_MutableBegin cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_Array_MutableBegin(CEL_NONNULL(_cel_Array*) arr) {
  return _cel_GenericArray_MutableBegin(&arr->g);
}

#define _cel_Array_MutableBegin(arr) \
  ((CEL_NULLABLE(_cel_Array_ValueType(arr)*))_cel_Array_MutableBegin(&(arr)->v))

#ifdef _cel_Array_End
#error _cel_Array_End cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(const void*)
    _cel_Array_End(CEL_NONNULL(const _cel_Array*) arr, size_t ele_size) {
  return _cel_GenericArray_End(&arr->g, ele_size);
}

#define _cel_Array_End(arr)                                        \
  ((CEL_NULLABLE(const _cel_Array_ValueType(arr)*))_cel_Array_End( \
      &(arr)->v, _cel_Array_ValueSize(arr)))

#ifdef _cel_Array_MutableEnd
#error _cel_Array_MutableEnd cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    _cel_Array_MutableEnd(CEL_NONNULL(_cel_Array*) arr, size_t ele_size) {
  return _cel_GenericArray_MutableEnd(&arr->g, ele_size);
}

#define _cel_Array_MutableEnd(arr)                                  \
  ((CEL_NULLABLE(_cel_Array_ValueType(arr)*))_cel_Array_MutableEnd( \
      &(arr)->v, _cel_Array_ValueSize(arr)))

#define _cel_Array(type)              \
  union {                             \
    CEL_NULLABILITY_UNKNOWN(type*) t; \
    _cel_Array v;                     \
  }

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_ARRAY_H_
