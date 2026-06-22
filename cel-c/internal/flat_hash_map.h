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

// Internal header providing a flat hash map implementation which uses
// allocator-based memory management.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_FLAT_HASH_MAP_H_
#define THIRD_PARTY_CEL_C_INTERNAL_FLAT_HASH_MAP_H_

#include <stdalign.h>  // IWYU pragma: keep
#include <stdbool.h>   // IWYU pragma: keep
#include <stddef.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/config.h"
#include "cel-c/internal/generic_flat_hash.h"

CEL_BEGIN_DECLS

#ifdef _cel_FlatHashMap
#error _cel_FlatHashMap cannot be directly set
#endif

typedef struct {
  _cel_GenericFlatHash g;
} _cel_FlatHashMap;

#define _cel_FlatHashMap_KeyType(map) cel_typeof_unqual((map)->t->key)

#define _cel_FlatHashMap_KeySize(map) sizeof(_cel_FlatHashMap_KeyType(map))

#define _cel_FlatHashMap_MappedType(map) cel_typeof_unqual((map)->t->val)

#define _cel_FlatHashMap_MappedSize(map) \
  sizeof(_cel_FlatHashMap_MappedType(map))

#define _cel_FlatHashMap_ValueType(map) cel_typeof_unqual(*(map)->t)

#define _cel_FlatHashMap_ValueSize(map) sizeof(_cel_FlatHashMap_ValueType(map))

#define _cel_FlatHashMapNode(map) \
  CEL_NULLABILITY_UNKNOWN(const _cel_FlatHashMap_ValueType(&(map)) *)

#define _cel_FlatHashMapMutableNode(map) \
  CEL_NULLABILITY_UNKNOWN(_cel_FlatHashMap_ValueType(&(map)) *)

#ifdef _cel_FlatHashMap_Construct
#error _cel_FlatHashMap_Construct cannot be directly set
#endif

static CEL_INLINE void _cel_FlatHashMap_Construct(
    CEL_NONNULL(_cel_FlatHashMap *) map,
    CEL_NONNULL(_cel_GenericFlatHashHasher) hasher,
    CEL_NONNULL(_cel_GenericFlatHashEqualer) equaler) {
  _cel_GenericFlatHash_Construct(&map->g, hasher, equaler);
}

#define _cel_FlatHashMap_Construct(map, hasher, equaler)            \
  _cel_FlatHashMap_Construct(                                       \
      &(map)->v, (CEL_NONNULL(_cel_GenericFlatHashHasher))(hasher), \
      (CEL_NONNULL(_cel_GenericFlatHashEqualer))(equaler))

#ifdef _cel_FlatHashMap_Destruct
#error _cel_FlatHashMap_Destruct cannot be directly set
#endif

static CEL_INLINE void _cel_FlatHashMap_Destruct(CEL_NONNULL(_cel_FlatHashMap *)
                                                     map,
                                                 CEL_NONNULL(cel_Allocator *)
                                                     alloc,
                                                 size_t ent_size) {
  return _cel_GenericFlatHash_DestructAllocator(&map->g, alloc, ent_size);
}

#define _cel_FlatHashMap_Destruct(map, alloc) \
  _cel_FlatHashMap_Destruct(&(map)->v, (alloc), _cel_FlatHashMap_ValueSize(map))

#ifdef _cel_FlatHashMap_Size
#error _cel_FlatHashMap_Size cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_FlatHashMap_Size(CEL_NONNULL(const _cel_FlatHashMap *) map) {
  return _cel_GenericFlatHash_Size(&map->g);
}

#define _cel_FlatHashMap_Size(map) _cel_FlatHashMap_Size(&(map)->v)

#ifdef _cel_FlatHashMap_Empty
#error _cel_FlatHashMap_Empty cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_FlatHashMap_Empty(
    CEL_NONNULL(const _cel_FlatHashMap *) map) {
  return _cel_GenericFlatHash_Empty(&map->g);
}

#define _cel_FlatHashMap_Empty(map) _cel_FlatHashMap_Empty(&(map)->v)

#ifdef _cel_FlatHashMap_Capacity
#error _cel_FlatHashMap_Capacity cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_FlatHashMap_Capacity(CEL_NONNULL(const _cel_FlatHashMap *) map) {
  return _cel_GenericFlatHash_Capacity(&map->g);
}

#define _cel_FlatHashMap_Capacity(map) _cel_FlatHashMap_Capacity(&(map)->v)

#ifdef _cel_FlatHashMap_Reserve
#error _cel_FlatHashMap_Reserve cannot be directly set
#endif

static CEL_INLINE bool _cel_FlatHashMap_Reserve(
    CEL_NONNULL(_cel_FlatHashMap *) map, CEL_NONNULL(cel_Allocator *) alloc,
    size_t new_cap, size_t ent_size) {
  return _cel_GenericFlatHash_ReserveAllocator(&map->g, alloc, new_cap,
                                               ent_size);
}

#define _cel_FlatHashMap_Reserve(map, alloc, new_cap)     \
  _cel_FlatHashMap_Reserve(&(map)->v, (alloc), (new_cap), \
                           _cel_FlatHashMap_ValueSize(map))

#ifdef _cel_FlatHashMap_Clear
#error _cel_FlatHashMap_Clear cannot be directly set
#endif

static CEL_INLINE void _cel_FlatHashMap_Clear(CEL_NONNULL(_cel_FlatHashMap *)
                                                  map) {
  _cel_GenericFlatHash_ClearArena(&map->g);
}

#define _cel_FlatHashMap_Clear(map) _cel_FlatHashMap_Clear(&(map)->v)

#ifdef _cel_FlatHashMap_Reset
#error _cel_FlatHashMap_Reset cannot be directly set
#endif

static CEL_INLINE void _cel_FlatHashMap_Reset(CEL_NONNULL(_cel_FlatHashMap *)
                                                  map,
                                              CEL_NONNULL(cel_Allocator *)
                                                  alloc,
                                              size_t ent_size) {
  _cel_GenericFlatHash_ResetAllocator(&map->g, alloc, ent_size);
}

#define _cel_FlatHashMap_Reset(map, alloc) \
  _cel_FlatHashMap_Reset(&(map)->v, (alloc), _cel_FlatHashMap_ValueSize(map))

#ifdef _cel_FlatHashMap_Find
#error _cel_FlatHashMap_Find cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(const void *)
    _cel_FlatHashMap_Find(CEL_NONNULL(const _cel_FlatHashMap *) map,
                          CEL_NONNULL(const void *) key, size_t ent_size) {
  return _cel_GenericFlatHash_Find(&map->g, key, ent_size);
}

#define _cel_FlatHashMap_Find(map, key)               \
  ((CEL_NULLABLE(const _cel_FlatHashMap_ValueType(    \
      map) *))_cel_FlatHashMap_Find(&(map)->v, (key), \
                                    _cel_FlatHashMap_ValueSize(map)))

#ifdef _cel_FlatHashMap_MutableFind
#error _cel_FlatHashMap_MutableFind cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void *)
    _cel_FlatHashMap_MutableFind(CEL_NONNULL(_cel_FlatHashMap *) map,
                                 CEL_NONNULL(const void *) key,
                                 size_t ent_size) {
  return _cel_GenericFlatHash_MutableFind(&map->g, key, ent_size);
}

#define _cel_FlatHashMap_MutableFind(map, key)               \
  ((CEL_NULLABLE(_cel_FlatHashMap_ValueType(                 \
      map) *))_cel_FlatHashMap_MutableFind(&(map)->v, (key), \
                                           _cel_FlatHashMap_ValueSize(map)))

#ifdef _cel_FlatHashMap_Erase
#error _cel_FlatHashMap_Erase cannot be directly set
#endif

static CEL_INLINE void _cel_FlatHashMap_Erase(CEL_NONNULL(_cel_FlatHashMap *)
                                                  map,
                                              CEL_NONNULL(const void *) ent,
                                              size_t ent_size) {
  _cel_GenericFlatHash_Erase(&map->g, ent, ent_size);
}

#define _cel_FlatHashMap_Erase(map, ent) \
  _cel_FlatHashMap_Erase(&(map)->v, (ent), _cel_FlatHashMap_ValueSize(map))

#ifdef _cel_FlatHashMap_Insert
#error _cel_FlatHashMap_Insert cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_FlatHashMap_Insert(
    CEL_NONNULL(_cel_FlatHashMap *) map, CEL_NONNULL(cel_Allocator *) alloc,
    CEL_NONNULL(const void *) key, CEL_NONNULL(CEL_NULLABLE(void *) *) ent,
    size_t ent_size, size_t key_size) {
  return _cel_GenericFlatHash_InsertAllocator(&map->g, alloc, key, ent,
                                              ent_size, key_size);
}

#define _cel_FlatHashMap_Insert(map, alloc, key, ent)                        \
  _cel_FlatHashMap_Insert(                                                   \
      &(map)->v, (alloc), (key), (CEL_NONNULL(CEL_NULLABLE(void *) *))(ent), \
      _cel_FlatHashMap_ValueSize(map), _cel_FlatHashMap_KeySize(map))

#ifdef _cel_FlatHashMap_kBegin
#error _cel_FlatHashMap_kBegin cannot be directly set
#endif

#define _cel_FlatHashMap_kBegin _cel_GenericFlatHash_kBegin

#ifdef _cel_FlatHashMap_Next
#error _cel_FlatHashMap_Next cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_FlatHashMap_Next(
    CEL_NONNULL(const _cel_FlatHashMap *) map,
    CEL_NONNULL(CEL_NULLABLE(const void *) *) ent, size_t ent_size) {
  return _cel_GenericFlatHash_Next(&map->g, ent, ent_size);
}

#define _cel_FlatHashMap_Next(map, ent)                                   \
  _cel_FlatHashMap_Next(&(map)->v,                                        \
                        (CEL_NONNULL(CEL_NULLABLE(const void *) *))(ent), \
                        _cel_FlatHashMap_ValueSize(map))

#ifdef _cel_FlatHashMap_MutableNext
#error _cel_FlatHashMap_MutableNext cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_FlatHashMap_MutableNext(
    CEL_NONNULL(_cel_FlatHashMap *) map,
    CEL_NONNULL(CEL_NULLABLE(void *) *) ent, size_t ent_size) {
  return _cel_GenericFlatHash_MutableNext(&map->g, ent, ent_size);
}

#define _cel_FlatHashMap_MutableNext(map, ent)                             \
  _cel_FlatHashMap_MutableNext(&(map)->v,                                  \
                               (CEL_NONNULL(CEL_NULLABLE(void *) *))(ent), \
                               _cel_FlatHashMap_ValueSize(map))

#define _cel_FlatHashMap(k, m)       \
  union {                            \
    CEL_NULLABILITY_UNKNOWN(struct { \
      k key;                         \
      m val;                         \
    } *)                             \
    t;                               \
    _cel_FlatHashMap v;              \
  }

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_FLAT_HASH_MAP_H_
