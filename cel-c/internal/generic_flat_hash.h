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

// Internal header providing a generic flat hash implementation.

// IWYU pragma: private
// IWYU pragma: friend "cel-c/internal/(?:(?:arena|generic)_)?flat_hash(?:_(?:set|map))?\.[hc]"

#ifndef THIRD_PARTY_CEL_C_INTERNAL_GENERIC_FLAT_HASH_H_
#define THIRD_PARTY_CEL_C_INTERNAL_GENERIC_FLAT_HASH_H_

#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>

#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/internal/bitset.h"
#include "cel-c/internal/config.h"

CEL_BEGIN_DECLS

typedef size_t (*_cel_GenericFlatHashHasher)(CEL_NONNULL(const void *) ent);

typedef bool (*_cel_GenericFlatHashEqualer)(CEL_NONNULL(const void *) lhs,
                                            CEL_NONNULL(const void *) rhs);

typedef struct {
  CEL_NULLABLE(_cel_BitSetWord *) ctrl;
  CEL_NULLABLE(void *) ents;
  size_t len;
  size_t cap;
  size_t thr;
  CEL_NONNULL(_cel_GenericFlatHashHasher) h;
  CEL_NONNULL(_cel_GenericFlatHashEqualer) eq;
} _cel_GenericFlatHash;

CEL_ATTRIBUTE_NOTHROW
void _cel_GenericFlatHash_Construct(CEL_NONNULL(_cel_GenericFlatHash *) fh,
                                    CEL_NONNULL(_cel_GenericFlatHashHasher)
                                        hasher,
                                    CEL_NONNULL(_cel_GenericFlatHashEqualer)
                                        equaler);

CEL_ATTRIBUTE_NOTHROW
void _cel_GenericFlatHash_DestructAllocator(CEL_NONNULL(_cel_GenericFlatHash *)
                                                fh,
                                            CEL_NONNULL(cel_Allocator *) alloc,
                                            size_t ent_size);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_GenericFlatHash_Size(CEL_NONNULL(const _cel_GenericFlatHash *) fh) {
  CEL_ASSERT_NOT_NULL(fh);
  return fh->len;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_GenericFlatHash_Empty(
    CEL_NONNULL(const _cel_GenericFlatHash *) fh) {
  return _cel_GenericFlatHash_Size(fh) == 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_GenericFlatHash_Capacity(CEL_NONNULL(const _cel_GenericFlatHash *) fh) {
  CEL_ASSERT_NOT_NULL(fh);
  return fh->cap;
}

CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericFlatHash_ReserveAllocator(CEL_NONNULL(_cel_GenericFlatHash *)
                                               fh,
                                           CEL_NONNULL(cel_Allocator *) alloc,
                                           size_t new_cap, size_t ent_size);

CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericFlatHash_ReserveArena(CEL_NONNULL(_cel_GenericFlatHash *) fh,
                                       CEL_NONNULL(cel_Arena *) arena,
                                       size_t new_cap, size_t ent_size);

CEL_ATTRIBUTE_NOTHROW
void _cel_GenericFlatHash_ClearAllocator(CEL_NONNULL(_cel_GenericFlatHash *)
                                             fh);

CEL_ATTRIBUTE_NOTHROW
void _cel_GenericFlatHash_ClearArena(CEL_NONNULL(_cel_GenericFlatHash *) fh);

CEL_ATTRIBUTE_NOTHROW
void _cel_GenericFlatHash_ResetAllocator(CEL_NONNULL(_cel_GenericFlatHash *) fh,
                                         CEL_NONNULL(cel_Allocator *) alloc,
                                         size_t ent_size);

CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
CEL_NULLABLE(const void *)
_cel_GenericFlatHash_Find(CEL_NONNULL(const _cel_GenericFlatHash *) fh,
                          CEL_NONNULL(const void *) key, size_t ent_size);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void *)
    _cel_GenericFlatHash_MutableFind(CEL_NONNULL(_cel_GenericFlatHash *) fh,
                                     CEL_NONNULL(const void *) key,
                                     size_t ent_size) {
  return (CEL_NULLABLE(void *))_cel_GenericFlatHash_Find(fh, key, ent_size);
}

CEL_ATTRIBUTE_NOTHROW
void _cel_GenericFlatHash_Erase(CEL_NONNULL(_cel_GenericFlatHash *) fh,
                                CEL_NONNULL(const void *) ent, size_t ent_size);

CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
bool _cel_GenericFlatHash_InsertAllocator(
    CEL_NONNULL(_cel_GenericFlatHash *) fh, CEL_NONNULL(cel_Allocator *) alloc,
    CEL_NONNULL(const void *) key, CEL_NONNULL(CEL_NULLABLE(void *) *) ent,
    size_t ent_size, size_t key_size);

CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
bool _cel_GenericFlatHash_InsertArena(CEL_NONNULL(_cel_GenericFlatHash *) fh,
                                      CEL_NONNULL(cel_Arena *) arena,
                                      CEL_NONNULL(const void *) key,
                                      CEL_NONNULL(CEL_NULLABLE(void *) *) ent,
                                      size_t ent_size, size_t key_size);

#ifdef _cel_GenericFlatHash_kBegin
#error _cel_GenericFlatHash_kBegin cannot be directly set
#endif

#define _cel_GenericFlatHash_kBegin ((CEL_NULLABLE(void *)) - 1)

CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
bool _cel_GenericFlatHash_Next(CEL_NONNULL(const _cel_GenericFlatHash *) fh,
                               CEL_NONNULL(CEL_NULLABLE(const void *) *) ent,
                               size_t ent_size);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_GenericFlatHash_MutableNext(
    CEL_NONNULL(_cel_GenericFlatHash *) fh,
    CEL_NONNULL(CEL_NULLABLE(void *) *) ent, size_t ent_size) {
  return _cel_GenericFlatHash_Next(
      fh, (CEL_NONNULL(CEL_NULLABLE(const void *) *))ent, ent_size);
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_GENERIC_FLAT_HASH_H_
