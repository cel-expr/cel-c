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

#include "cel-c/internal/generic_flat_hash.h"

#include <limits.h>
#include <stdalign.h>  // IWYU pragma: keep
#include <stdbool.h>   // IWYU pragma: keep
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/internal/align.h"
#include "cel-c/internal/bit.h"
#include "cel-c/internal/bitset.h"
#include "cel-c/internal/config.h"

CEL_ATTRIBUTE_NODISCARD
static size_t _cel_GenericFlatHash_Rehash(
    CEL_NONNULL(const _cel_GenericFlatHash*) fh,
    CEL_NONNULL(_cel_BitSetWord*) new_ctrl, CEL_NONNULL(char*) new_ents,
    size_t old_slot, size_t new_cap, size_t ent_size) {
  size_t new_slot = SIZE_MAX;
  if (fh->len != 0) {
    CEL_ASSERT_GT(new_cap, fh->cap);

    const size_t new_mask = new_cap - 1;
    CEL_NONNULL(const _cel_BitSetWord*) const old_ctrl = fh->ctrl;
    const size_t old_cap = fh->cap;
    CEL_NONNULL(const char*)
    const old_ents = reinterpret_cast<const char*>(fh->ents);
    size_t bit = _cel_BitSet_kBegin;
    while (_cel_BitSet_Next(old_ctrl, old_cap, &bit)) {
      CEL_NONNULL(const char*) old_ent = old_ents + (ent_size * bit);
      size_t slot = ((*fh->h)(old_ent)) & new_mask;
      while (_cel_BitSet_Test(new_ctrl, new_cap, slot)) {
        slot = (slot + 1) & new_mask;
      }
      _cel_BitSet_Set(new_ctrl, new_cap, slot);
      CEL_NONNULL(char*) new_ent = new_ents + (slot * ent_size);
      memcpy(new_ent, old_ent, ent_size);
      if (bit == old_slot) {
        new_slot = slot;
      }
    }
  }
  CEL_ASSERT(old_slot == SIZE_MAX || new_slot != SIZE_MAX);
  return new_slot;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_GenericFlatHash_PreInsertAllocator(
    CEL_NONNULL(_cel_GenericFlatHash*) fh, CEL_NONNULL(cel_Allocator*) alloc,
    size_t ent_size) {
  CEL_ASSERT_EQ(fh->cap, 0);

  size_t size =
      _cel_align_up(_cel_BitSet_WordsForBits(16) * sizeof(_cel_BitSetWord),
                    cel_kMaxAlign) +
      (ent_size * 16);
  CEL_NULLABLE(char*)
  data =
      reinterpret_cast<char*>(cel_Allocator_Malloc(alloc, size, cel_nullptr));
  if (CEL_UNLIKELY(data == cel_nullptr)) {
    return false;
  }
  CEL_NONNULL(_cel_BitSetWord*) ctrl = (CEL_NONNULL(_cel_BitSetWord*))data;
  CEL_NONNULL(char*)
  ents = data +
         _cel_align_up(_cel_BitSet_WordsForBits(16) * sizeof(_cel_BitSetWord),
                       cel_kMaxAlign);
  _cel_BitSet_Reset(ctrl, 16);
  fh->ctrl = ctrl;
  fh->ents = ents;
  fh->cap = 16;
  fh->thr = 10;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_GenericFlatHash_GrowAllocator(
    CEL_NONNULL(_cel_GenericFlatHash*) fh, CEL_NONNULL(cel_Allocator*) alloc,
    size_t new_cap, CEL_NONNULL(size_t*) slot, size_t ent_size) {
  CEL_ASSERT(_cel_has_single_bit(new_cap));
  CEL_ASSERT_GT(new_cap, fh->cap);

  size_t size =
      _cel_align_up(_cel_BitSet_WordsForBits(new_cap) * sizeof(_cel_BitSetWord),
                    cel_kMaxAlign) +
      (ent_size * new_cap);
  CEL_NULLABLE(char*)
  data =
      reinterpret_cast<char*>(cel_Allocator_Malloc(alloc, size, cel_nullptr));
  if (CEL_UNLIKELY(data == cel_nullptr)) {
    return false;
  }
  CEL_NONNULL(_cel_BitSetWord*) ctrl = (CEL_NONNULL(_cel_BitSetWord*))data;
  CEL_NONNULL(char*)
  ents = data + _cel_align_up(
                    _cel_BitSet_WordsForBits(new_cap) * sizeof(_cel_BitSetWord),
                    cel_kMaxAlign);
  _cel_BitSet_Reset(ctrl, new_cap);

  *slot = _cel_GenericFlatHash_Rehash(fh, ctrl, ents, *slot, new_cap, ent_size);

  size_t old_size =
      _cel_align_up(_cel_BitSet_WordsForBits(fh->cap) * sizeof(_cel_BitSetWord),
                    cel_kMaxAlign) +
      (fh->cap * ent_size);
  CEL_NONNULL(void*) old_data = fh->ctrl;
  cel_Allocator_FreeSized(alloc, old_data, old_size);

  fh->ctrl = ctrl;
  fh->ents = ents;
  fh->cap = new_cap;
  fh->thr = (new_cap * 2) / 3;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_GenericFlatHash_PostInsertAllocator(
    CEL_NONNULL(_cel_GenericFlatHash*) fh, CEL_NONNULL(cel_Allocator*) alloc,
    CEL_NONNULL(size_t*) slot, size_t ent_size) {
  CEL_ASSERT_GT(fh->len, fh->thr);

  return _cel_GenericFlatHash_GrowAllocator(fh, alloc, fh->cap << 1, slot,
                                            ent_size);
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_GenericFlatHash_PreInsertArena(
    CEL_NONNULL(_cel_GenericFlatHash*) fh, CEL_NONNULL(cel_Arena*) arena,
    size_t ent_size) {
  CEL_ASSERT_EQ(fh->cap, 0);

  size_t size =
      _cel_align_up(_cel_BitSet_WordsForBits(16) * sizeof(_cel_BitSetWord),
                    cel_kMaxAlign) +
      (ent_size * 16);
  CEL_NULLABLE(char*)
  data = reinterpret_cast<char*>(cel_Arena_Malloc(arena, size, cel_nullptr));
  if (CEL_UNLIKELY(data == cel_nullptr)) {
    return false;
  }
  CEL_NONNULL(_cel_BitSetWord*) ctrl = (CEL_NONNULL(_cel_BitSetWord*))data;
  CEL_NONNULL(char*)
  ents = data +
         _cel_align_up(_cel_BitSet_WordsForBits(16) * sizeof(_cel_BitSetWord),
                       cel_kMaxAlign);
  _cel_BitSet_Reset(ctrl, 16);
  fh->ctrl = ctrl;
  fh->ents = ents;
  fh->cap = 16;
  fh->thr = 10;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_GenericFlatHash_GrowArena(
    CEL_NONNULL(_cel_GenericFlatHash*) fh, CEL_NONNULL(cel_Arena*) arena,
    size_t new_cap, CEL_NONNULL(size_t*) slot, size_t ent_size) {
  CEL_ASSERT(_cel_has_single_bit(new_cap));
  CEL_ASSERT_GT(new_cap, fh->cap);

  size_t size =
      _cel_align_up(_cel_BitSet_WordsForBits(new_cap) * sizeof(_cel_BitSetWord),
                    cel_kMaxAlign) +
      (ent_size * new_cap);
  CEL_NULLABLE(char*)
  data = reinterpret_cast<char*>(cel_Arena_Malloc(arena, size, cel_nullptr));
  if (CEL_UNLIKELY(data == cel_nullptr)) {
    return false;
  }
  CEL_NONNULL(_cel_BitSetWord*) ctrl = (CEL_NONNULL(_cel_BitSetWord*))data;
  CEL_NONNULL(char*)
  ents = data + _cel_align_up(
                    _cel_BitSet_WordsForBits(new_cap) * sizeof(_cel_BitSetWord),
                    cel_kMaxAlign);
  _cel_BitSet_Reset(ctrl, new_cap);

  *slot = _cel_GenericFlatHash_Rehash(fh, ctrl, ents, *slot, new_cap, ent_size);

  fh->ctrl = ctrl;
  fh->ents = ents;
  fh->cap = new_cap;
  fh->thr = (new_cap * 2) / 3;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_GenericFlatHash_PostInsertArena(
    CEL_NONNULL(_cel_GenericFlatHash*) fh, CEL_NONNULL(cel_Arena*) arena,
    CEL_NONNULL(size_t*) slot, size_t ent_size) {
  CEL_ASSERT_GT(fh->len, fh->thr);

  return _cel_GenericFlatHash_GrowArena(fh, arena, fh->cap << 1, slot,
                                        ent_size);
}

extern "C" void _cel_GenericFlatHash_Construct(
    CEL_NONNULL(_cel_GenericFlatHash*) fh,
    CEL_NONNULL(_cel_GenericFlatHashHasher) hasher,
    CEL_NONNULL(_cel_GenericFlatHashEqualer) equaler) {
  CEL_ASSERT_NOT_NULL(fh);
  CEL_ASSERT_NOT_NULL(hasher);
  CEL_ASSERT_NOT_NULL(equaler);

  memset(fh, '\0', sizeof(*fh));
  fh->eq = equaler;
  fh->h = hasher;
}

extern "C" void _cel_GenericFlatHash_DestructAllocator(
    CEL_NONNULL(_cel_GenericFlatHash*) fh, CEL_NONNULL(cel_Allocator*) alloc,
    size_t ent_size) {
  CEL_ASSERT_NOT_NULL(fh);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_GT(ent_size, 0);

  if (fh->cap != 0) {
    size_t size = _cel_align_up(_cel_BitSet_WordsForBits(fh->cap) *
                                    sizeof(_cel_BitSetWord),
                                cel_kMaxAlign) +
                  (fh->cap * ent_size);
    CEL_NONNULL(void*) data = fh->ctrl;
    cel_Allocator_FreeSized(alloc, data, size);
  }
}

extern "C" bool _cel_GenericFlatHash_ReserveAllocator(
    CEL_NONNULL(_cel_GenericFlatHash*) fh, CEL_NONNULL(cel_Allocator*) alloc,
    size_t new_cap, size_t ent_size) {
  CEL_ASSERT_NOT_NULL(fh);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_GT(ent_size, 0);

  if (new_cap <= fh->cap) {
    return true;
  }

  new_cap = ((size_t)1) << ((sizeof(size_t) * CHAR_BIT) -
                            _cel_leading_zeros(new_cap));
  size_t unused_slot = SIZE_MAX;
  return _cel_GenericFlatHash_GrowAllocator(fh, alloc, new_cap, &unused_slot,
                                            ent_size);
}

extern "C" bool _cel_GenericFlatHash_ReserveArena(
    CEL_NONNULL(_cel_GenericFlatHash*) fh, CEL_NONNULL(cel_Arena*) arena,
    size_t new_cap, size_t ent_size) {
  CEL_ASSERT_NOT_NULL(fh);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_GT(ent_size, 0);

  if (new_cap <= fh->cap) {
    return true;
  }

  new_cap = ((size_t)1) << ((sizeof(size_t) * CHAR_BIT) -
                            _cel_leading_zeros(new_cap));
  size_t unused_slot = SIZE_MAX;
  return _cel_GenericFlatHash_GrowArena(fh, arena, new_cap, &unused_slot,
                                        ent_size);
}

extern "C" void _cel_GenericFlatHash_ClearAllocator(
    CEL_NONNULL(_cel_GenericFlatHash*) fh) {
  CEL_ASSERT_NOT_NULL(fh);

  if (fh->ctrl != cel_nullptr && fh->len != 0) {
    _cel_BitSet_Reset(fh->ctrl, _cel_BitSet_WordsForBits(fh->cap));
    fh->len = 0;
  }
}

extern "C" void _cel_GenericFlatHash_ClearArena(
    CEL_NONNULL(_cel_GenericFlatHash*) fh) {
  CEL_ASSERT_NOT_NULL(fh);

  if (fh->ctrl != cel_nullptr && fh->len != 0) {
    _cel_BitSet_Reset(fh->ctrl, _cel_BitSet_WordsForBits(fh->cap));
    fh->len = 0;
  }
}

extern "C" void _cel_GenericFlatHash_ResetAllocator(
    CEL_NONNULL(_cel_GenericFlatHash*) fh, CEL_NONNULL(cel_Allocator*) alloc,
    size_t ent_size) {
  CEL_ASSERT_NOT_NULL(fh);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_GT(ent_size, 0);

  CEL_NONNULL(_cel_GenericFlatHashHasher)
  hasher = fh->h;
  CEL_NONNULL(_cel_GenericFlatHashEqualer)
  equaler = fh->eq;
  _cel_GenericFlatHash_DestructAllocator(fh, alloc, ent_size);
  _cel_GenericFlatHash_Construct(fh, hasher, equaler);
}

extern "C" CEL_NULLABLE(const void*)
    _cel_GenericFlatHash_Find(CEL_NONNULL(const _cel_GenericFlatHash*) fh,
                              CEL_NONNULL(const void*) key, size_t ent_size) {
  CEL_ASSERT_NOT_NULL(fh);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_GT(ent_size, 0);

  if (fh->len == 0) {
    return cel_nullptr;
  }
  size_t hash = (*fh->h)(key);
  size_t mask = fh->cap - 1;
  size_t slot = hash & mask;
  if (_cel_BitSet_Test(fh->ctrl, fh->cap, slot)) {
    CEL_NONNULL(const char*)
    ents = reinterpret_cast<char*>(fh->ents);
    do {
      if ((*fh->eq)(ents + (ent_size * slot), key)) {
        return ents + (ent_size * slot);
      }
      slot = (slot + 1) & mask;
    } while (_cel_BitSet_Test(fh->ctrl, fh->cap, slot));
  }
  return cel_nullptr;
}

extern "C" void _cel_GenericFlatHash_Erase(CEL_NONNULL(_cel_GenericFlatHash*)
                                               fh,
                                           CEL_NONNULL(const void*) ent,
                                           size_t ent_size) {
  CEL_ASSERT_NOT_NULL(fh);
  CEL_ASSERT_NOT_NULL(ent);
  CEL_ASSERT_GT(ent_size, 0);

  CEL_NONNULL(char*)
  ents = reinterpret_cast<char*>(fh->ents);

  CEL_ASSERT_GE((CEL_NONNULL(const char*))ent, ents);
  CEL_ASSERT_LT((CEL_NONNULL(const char*))ent, ents + (fh->cap * ent_size));

  size_t slot = ((size_t)(((CEL_NONNULL(const char*))ent) - ents)) / ent_size;
  CEL_ASSERT(_cel_BitSet_Test(fh->ctrl, fh->cap, slot));
  _cel_BitSet_Clear(fh->ctrl, fh->cap, slot);
  --(fh->len);

  size_t mask = fh->cap - 1;
  size_t i = slot;
  size_t j = slot;
  while (true) {
    j = (j + 1) & mask;
    if (!_cel_BitSet_Test(fh->ctrl, fh->cap, j)) {
      break;
    }
    const size_t k = (*fh->h)(ents + (j * ent_size)) & mask;
    if (i <= j) {
      if (i < k && k <= j) {
        continue;
      }
    } else {
      if (k <= j || i < k) {
        continue;
      }
    }
    _cel_BitSet_Set(fh->ctrl, fh->cap, i);
    memcpy(ents + (i * ent_size), ents + (j * ent_size), ent_size);
    _cel_BitSet_Clear(fh->ctrl, fh->cap, j);
    i = j;
  }
}

extern "C" bool _cel_GenericFlatHash_InsertAllocator(
    CEL_NONNULL(_cel_GenericFlatHash*) fh, CEL_NONNULL(cel_Allocator*) alloc,
    CEL_NONNULL(const void*) key, CEL_NONNULL(CEL_NULLABLE(void*) *) ent,
    size_t ent_size, size_t key_size) {
  CEL_ASSERT_NOT_NULL(fh);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(ent);
  CEL_ASSERT_GT(ent_size, 0);

  if (CEL_UNLIKELY(fh->cap == 0 && !_cel_GenericFlatHash_PreInsertAllocator(
                                       fh, alloc, ent_size))) {
    *ent = cel_nullptr;
    return false;
  }

  size_t hash = (*fh->h)(key);
  size_t mask = fh->cap - 1;
  size_t slot = hash & mask;
  CEL_NONNULL(char*)
  ents = reinterpret_cast<char*>(fh->ents);
  while (_cel_BitSet_Test(fh->ctrl, fh->cap, slot)) {
    if ((*fh->eq)(ents + (ent_size * slot), key)) {
      *ent = ents + (ent_size * slot);
      return false;
    }
    slot = (slot + 1) & mask;
  }

  memcpy(ents + (slot * ent_size), key, key_size);
  _cel_BitSet_Set(fh->ctrl, fh->cap, slot);
  ++(fh->len);

  if (CEL_UNLIKELY(fh->len > fh->thr)) {
    if (CEL_UNLIKELY(!_cel_GenericFlatHash_PostInsertAllocator(fh, alloc, &slot,
                                                               ent_size))) {
      _cel_BitSet_Clear(fh->ctrl, fh->cap, slot);
      --(fh->len);
      *ent = cel_nullptr;
      return false;
    }
    ents = reinterpret_cast<char*>(fh->ents);
  }

  *ent = ents + (ent_size * slot);

  return true;
}

extern "C" bool _cel_GenericFlatHash_InsertArena(
    CEL_NONNULL(_cel_GenericFlatHash*) fh, CEL_NONNULL(cel_Arena*) arena,
    CEL_NONNULL(const void*) key, CEL_NONNULL(CEL_NULLABLE(void*) *) ent,
    size_t ent_size, size_t key_size) {
  CEL_ASSERT_NOT_NULL(fh);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(ent);
  CEL_ASSERT_GT(ent_size, 0);

  if (CEL_UNLIKELY(fh->cap == 0 &&
                   !_cel_GenericFlatHash_PreInsertArena(fh, arena, ent_size))) {
    *ent = cel_nullptr;
    return false;
  }

  size_t hash = (*fh->h)(key);
  size_t mask = fh->cap - 1;
  size_t slot = hash & mask;
  CEL_NONNULL(char*)
  ents = reinterpret_cast<char*>(fh->ents);
  while (_cel_BitSet_Test(fh->ctrl, fh->cap, slot)) {
    if ((*fh->eq)(ents + (ent_size * slot), key)) {
      *ent = ents + (ent_size * slot);
      return false;
    }
    slot = (slot + 1) & mask;
  }

  memcpy(ents + (slot * ent_size), key, key_size);
  _cel_BitSet_Set(fh->ctrl, fh->cap, slot);
  ++(fh->len);

  if (CEL_UNLIKELY(fh->len > fh->thr)) {
    if (CEL_UNLIKELY(!_cel_GenericFlatHash_PostInsertArena(fh, arena, &slot,
                                                           ent_size))) {
      _cel_BitSet_Clear(fh->ctrl, fh->cap, slot);
      --(fh->len);
      *ent = cel_nullptr;
      return false;
    }
    ents = reinterpret_cast<char*>(fh->ents);
  }

  *ent = ents + (ent_size * slot);

  return true;
}

extern "C" bool _cel_GenericFlatHash_Next(
    CEL_NONNULL(const _cel_GenericFlatHash*) fh,
    CEL_NONNULL(CEL_NULLABLE(const void*) *) ent, size_t ent_size) {
  CEL_ASSERT_NOT_NULL(fh);
  CEL_ASSERT_NOT_NULL(ent);
  CEL_ASSERT_GT(ent_size, 0);

  CEL_NULLABLE(const char*)
  ents = reinterpret_cast<const char*>(fh->ents);

  size_t i;
  if (*ent == _cel_GenericFlatHash_kBegin) {
    i = _cel_BitSet_kBegin;
  } else {
    CEL_ASSERT_GE((CEL_NONNULL(const char*)) * ent, ents);
    CEL_ASSERT_LT((CEL_NONNULL(const char*)) * ent,
                  ents + (fh->cap * ent_size));
    i = (((size_t)(((CEL_NONNULL(const char*)) * ent) - ents)) / ent_size);
  }
  const bool next = _cel_BitSet_Next(fh->ctrl, fh->cap, &i);
  *ent = ents + (ent_size * i);
  return next;
}
