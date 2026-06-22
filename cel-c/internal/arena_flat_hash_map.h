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
// arena-based memory management.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_ARENA_FLAT_HASH_MAP_H_
#define THIRD_PARTY_CEL_C_INTERNAL_ARENA_FLAT_HASH_MAP_H_

#include <stdalign.h>  // IWYU pragma: keep
#include <stdbool.h>   // IWYU pragma: keep
#include <stddef.h>
#include <string.h>

#include "cel-c/arena.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/generic_flat_hash.h"

CEL_BEGIN_DECLS

#ifdef _cel_ArenaFlatHashMap
#error _cel_ArenaFlatHashMap cannot be directly set
#endif

typedef struct {
  _cel_GenericFlatHash g;
} _cel_ArenaFlatHashMap;

#define _cel_ArenaFlatHashMap_KeyType(map) cel_typeof_unqual((map)->t->key)

#define _cel_ArenaFlatHashMap_KeySize(map) \
  sizeof(_cel_ArenaFlatHashMap_KeyType(map))

#define _cel_ArenaFlatHashMap_MappedType(map) cel_typeof_unqual((map)->t->val)

#define _cel_ArenaFlatHashMap_MappedSize(map) \
  sizeof(_cel_ArenaFlatHashMap_MappedType(map))

#define _cel_ArenaFlatHashMap_ValueType(map) cel_typeof_unqual(*(map)->t)

#define _cel_ArenaFlatHashMap_ValueSize(map) \
  sizeof(_cel_ArenaFlatHashMap_ValueType(map))

#define _cel_ArenaFlatHashMapNode(map) \
  CEL_NULLABILITY_UNKNOWN(const _cel_ArenaFlatHashMap_ValueType(&(map)) *)

#define _cel_ArenaFlatHashMapMutableNode(map) \
  CEL_NULLABILITY_UNKNOWN(_cel_ArenaFlatHashMap_ValueType(&(map)) *)

#ifdef _cel_ArenaFlatHashMap_Construct
#error _cel_ArenaFlatHashMap_Construct cannot be directly set
#endif

static CEL_INLINE void _cel_ArenaFlatHashMap_Construct(
    CEL_NONNULL(_cel_ArenaFlatHashMap *) map,
    CEL_NONNULL(_cel_GenericFlatHashHasher) hasher,
    CEL_NONNULL(_cel_GenericFlatHashEqualer) equaler) {
  _cel_GenericFlatHash_Construct(&map->g, hasher, equaler);
}

#define _cel_ArenaFlatHashMap_Construct(map, hasher, equaler)       \
  _cel_ArenaFlatHashMap_Construct(                                  \
      &(map)->v, (CEL_NONNULL(_cel_GenericFlatHashHasher))(hasher), \
      (CEL_NONNULL(_cel_GenericFlatHashEqualer))(equaler))

#ifdef _cel_ArenaFlatHashMap_Size
#error _cel_ArenaFlatHashMap_Size cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_ArenaFlatHashMap_Size(CEL_NONNULL(const _cel_ArenaFlatHashMap *) map) {
  return _cel_GenericFlatHash_Size(&map->g);
}

#define _cel_ArenaFlatHashMap_Size(map) _cel_ArenaFlatHashMap_Size(&(map)->v)

#ifdef _cel_ArenaFlatHashMap_Empty
#error _cel_ArenaFlatHashMap_Empty cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ArenaFlatHashMap_Empty(
    CEL_NONNULL(const _cel_ArenaFlatHashMap *) map) {
  return _cel_GenericFlatHash_Empty(&map->g);
}

#define _cel_ArenaFlatHashMap_Empty(map) _cel_ArenaFlatHashMap_Empty(&(map)->v)

#ifdef _cel_ArenaFlatHashMap_Capacity
#error _cel_ArenaFlatHashMap_Capacity cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_ArenaFlatHashMap_Capacity(CEL_NONNULL(const _cel_ArenaFlatHashMap *) map) {
  return _cel_GenericFlatHash_Capacity(&map->g);
}

#define _cel_ArenaFlatHashMap_Capacity(map) \
  _cel_ArenaFlatHashMap_Capacity(&(map)->v)

#ifdef _cel_ArenaFlatHashMap_Reserve
#error _cel_ArenaFlatHashMap_Reserve cannot be directly set
#endif

static CEL_INLINE bool _cel_ArenaFlatHashMap_Reserve(
    CEL_NONNULL(_cel_ArenaFlatHashMap *) map, CEL_NONNULL(cel_Arena *) arena,
    size_t new_cap, size_t ent_size) {
  return _cel_GenericFlatHash_ReserveArena(&map->g, arena, new_cap, ent_size);
}

#define _cel_ArenaFlatHashMap_Reserve(map, arena, new_cap)     \
  _cel_ArenaFlatHashMap_Reserve(&(map)->v, (arena), (new_cap), \
                                _cel_ArenaFlatHashMap_ValueSize(map))

#ifdef _cel_ArenaFlatHashMap_Clear
#error _cel_ArenaFlatHashMap_Clear cannot be directly set
#endif

static CEL_INLINE void _cel_ArenaFlatHashMap_Clear(
    CEL_NONNULL(_cel_ArenaFlatHashMap *) map) {
  _cel_GenericFlatHash_ClearArena(&map->g);
}

#define _cel_ArenaFlatHashMap_Clear(map) _cel_ArenaFlatHashMap_Clear(&(map)->v)

#ifdef _cel_ArenaFlatHashMap_Find
#error _cel_ArenaFlatHashMap_Find cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(const void *)
    _cel_ArenaFlatHashMap_Find(CEL_NONNULL(const _cel_ArenaFlatHashMap *) map,
                               CEL_NONNULL(const void *) key, size_t ent_size) {
  return _cel_GenericFlatHash_Find(&map->g, key, ent_size);
}

#define _cel_ArenaFlatHashMap_Find(map, key)                    \
  ((CEL_NULLABLE(const _cel_ArenaFlatHashMap_ValueType(map) *)) \
       _cel_ArenaFlatHashMap_Find(&(map)->v, (key),             \
                                  _cel_ArenaFlatHashMap_ValueSize(map)))

#ifdef _cel_ArenaFlatHashMap_MutableFind
#error _cel_ArenaFlatHashMap_MutableFind cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void *)
    _cel_ArenaFlatHashMap_MutableFind(CEL_NONNULL(_cel_ArenaFlatHashMap *) map,
                                      CEL_NONNULL(const void *) key,
                                      size_t ent_size) {
  return _cel_GenericFlatHash_MutableFind(&map->g, key, ent_size);
}

#define _cel_ArenaFlatHashMap_MutableFind(map, key)       \
  ((CEL_NULLABLE(_cel_ArenaFlatHashMap_ValueType(map) *)) \
       _cel_ArenaFlatHashMap_MutableFind(                 \
           &(map)->v, (key), _cel_ArenaFlatHashMap_ValueSize(map)))

#ifdef _cel_ArenaFlatHashMap_Erase
#error _cel_ArenaFlatHashMap_Erase cannot be directly set
#endif

static CEL_INLINE void _cel_ArenaFlatHashMap_Erase(
    CEL_NONNULL(_cel_ArenaFlatHashMap *) map, CEL_NONNULL(const void *) ent,
    size_t ent_size) {
  _cel_GenericFlatHash_Erase(&map->g, ent, ent_size);
}

#define _cel_ArenaFlatHashMap_Erase(map, ent)   \
  _cel_ArenaFlatHashMap_Erase(&(map)->v, (ent), \
                              _cel_ArenaFlatHashMap_ValueSize(map))

#ifdef _cel_ArenaFlatHashMap_Insert
#error _cel_ArenaFlatHashMap_Insert cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ArenaFlatHashMap_Insert(
    CEL_NONNULL(_cel_ArenaFlatHashMap *) map, CEL_NONNULL(cel_Arena *) arena,
    CEL_NONNULL(const void *) key, CEL_NONNULL(CEL_NULLABLE(void *) *) ent,
    size_t ent_size, size_t key_size) {
  return _cel_GenericFlatHash_InsertArena(&map->g, arena, key, ent, ent_size,
                                          key_size);
}

#define _cel_ArenaFlatHashMap_Insert(map, arena, key, ent)                 \
  _cel_ArenaFlatHashMap_Insert(&(map)->v, (arena), (key),                  \
                               (CEL_NONNULL(CEL_NULLABLE(void *) *))(ent), \
                               _cel_ArenaFlatHashMap_ValueSize(map),       \
                               _cel_ArenaFlatHashMap_KeySize(map))

#ifdef _cel_ArenaFlatHashMap_kBegin
#error _cel_ArenaFlatHashMap_kBegin cannot be directly set
#endif

#define _cel_ArenaFlatHashMap_kBegin _cel_GenericFlatHash_kBegin

#ifdef _cel_ArenaFlatHashMap_Next
#error _cel_ArenaFlatHashMap_Next cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ArenaFlatHashMap_Next(
    CEL_NONNULL(const _cel_ArenaFlatHashMap *) map,
    CEL_NONNULL(CEL_NULLABLE(const void *) *) ent, size_t ent_size) {
  return _cel_GenericFlatHash_Next(&map->g, ent, ent_size);
}

#define _cel_ArenaFlatHashMap_Next(map, ent)                                   \
  _cel_ArenaFlatHashMap_Next(&(map)->v,                                        \
                             (CEL_NONNULL(CEL_NULLABLE(const void *) *))(ent), \
                             _cel_ArenaFlatHashMap_ValueSize(map))

#ifdef _cel_ArenaFlatHashMap_MutableNext
#error _cel_ArenaFlatHashMap_MutableNext cannot be directly set
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ArenaFlatHashMap_MutableNext(
    CEL_NONNULL(_cel_ArenaFlatHashMap *) map,
    CEL_NONNULL(CEL_NULLABLE(void *) *) ent, size_t ent_size) {
  return _cel_GenericFlatHash_MutableNext(&map->g, ent, ent_size);
}

#define _cel_ArenaFlatHashMap_MutableNext(map, ent)          \
  _cel_ArenaFlatHashMap_MutableNext(                         \
      &(map)->v, (CEL_NONNULL(CEL_NULLABLE(void *) *))(ent), \
      _cel_ArenaFlatHashMap_ValueSize(map))

#define _cel_ArenaFlatHashMap(k, m)  \
  union {                            \
    CEL_NULLABILITY_UNKNOWN(struct { \
      k key;                         \
      m val;                         \
    } *)                             \
    t;                               \
    _cel_ArenaFlatHashMap v;         \
  }

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_ARENA_FLAT_HASH_MAP_H_
