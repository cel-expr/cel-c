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

#include "cel-c/internal/mutable_map_value.h"

#include <limits.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/hash.h"
#include "cel-c/internal/bit.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/empty_map_value.h"
#include "cel-c/internal/map_value.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/value.h"

extern const cel_MapValueIteratorVTable _cel_MutableMapValueIteratorVTable;

typedef struct {
  cel_MapValueKey key;
  cel_Value value;
} _cel_MutableMapValueEntry;

typedef struct {
  union {
    struct {
      // ptr points to a memory block which starts with a bitset and pointers to
      // the entries immediately follow.
      _cel_MutableMapValueEntry * cel_nullable * cel_nullable ptr;
      uint32_t len;
      // cap is always a power of 2
      uint32_t cap;
    };
    cel_ValueContent super;
  };
} _cel_MutableMapValueContent;

typedef struct {
  cel_MapValueIterator super;
  cel_Allocator* cel_nonnull alloc;
  _cel_MutableMapValueEntry * cel_nullable * cel_nonnull ents;
  uint32_t len;
  uint32_t cap;
  uint32_t bit;
  uint32_t rem;
} _cel_MutableMapValueIterator;

CEL_ATTRIBUTE_NODISCARD
static bool _cel_MutableMapValueIterator_Done(
    const _cel_MutableMapValueIterator* cel_nonnull iter) {
  return iter->rem == 0;
}

static size_t _cel_MutableMapValueIterator_Advance(
    const _cel_MutableMapValueIterator* cel_nonnull iter) {
  CEL_ASSERT_NOT(_cel_MutableMapValueIterator_Done(iter));
  size_t bit;
  for (bit = iter->bit + 1; iter->ents[bit] == cel_nullptr; ++bit) {
  }
  return bit;
}

#define _cel_MutableMapValueContent_kPtrOffset 0
#define _cel_MutableMapValueContent_kLenOffset \
  (sizeof(void*) / sizeof(uint32_t))
#define _cel_MutableMapValueContent_kCapOffset \
  (_cel_MutableMapValueContent_kLenOffset + 1)

CEL_STATIC_ASSERT(sizeof(_cel_MutableMapValueContent) <=
                  sizeof(cel_ValueContent));
CEL_STATIC_ASSERT(alignof(_cel_MutableMapValueContent) <=
                  alignof(cel_ValueContent));

static bool _cel_MutableMapValue_FastSize(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    size_t* cel_nonnull size) {
  CEL_ASSERT_EQ(vtable, &_cel_MutableMapValueVTable);
  CEL_ASSERT_NOT_NULL(size);

  *size = content.u32[_cel_MutableMapValueContent_kLenOffset];
  return true;
}

static bool _cel_MutableMapValue_SlowSize(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull size,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_MutableMapValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(size);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Value_SetInt(size, content.u32[_cel_MutableMapValueContent_kLenOffset]);
  return true;
}

static bool _cel_MutableMapValue_Get(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValueKey* cel_nonnull key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_MutableMapValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  uint32_t len = content.u32[_cel_MutableMapValueContent_kLenOffset];
  if (len != 0) {
    _cel_MutableMapValueEntry** ents =
        (_cel_MutableMapValueEntry**)
            content.ptr[_cel_MutableMapValueContent_kPtrOffset];
    const size_t mask =
        (size_t)content.u32[_cel_MutableMapValueContent_kCapOffset] - 1;
    size_t bit = cel_HashState_Finalize(
                     _cel_MapValueKey_Hash(key, cel_HashState_Initialize())) &
                 mask;
    _cel_MutableMapValueEntry* ent;
    for (ent = ents[bit]; ent != cel_nullptr; ent = ents[(++bit) & mask]) {
      if (_cel_MapValueKey_Equals(&ent->key, key)) {
        *value = ent->value;
        return true;
      }
    }
  }

  cel_Error* error = cel_Error_New(context->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    cel_OutOfMemoryStatus(status);
    return false;
  }
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kNotFound);
  cel_Error_SetMessage(error, cel_StringView_From("no such key"));
  cel_Value_SetError(value, error);
  return true;
}

static bool _cel_MutableMapValue_Find(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValueKey* cel_nonnull key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_MutableMapValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  uint32_t len = content.u32[_cel_MutableMapValueContent_kLenOffset];
  if (len != 0) {
    _cel_MutableMapValueEntry** ents =
        (_cel_MutableMapValueEntry**)
            content.ptr[_cel_MutableMapValueContent_kPtrOffset];
    const size_t mask =
        (size_t)content.u32[_cel_MutableMapValueContent_kCapOffset] - 1;
    size_t bit = cel_HashState_Finalize(
                     _cel_MapValueKey_Hash(key, cel_HashState_Initialize())) &
                 mask;
    _cel_MutableMapValueEntry* ent;
    for (ent = ents[bit]; ent != cel_nullptr; ent = ents[(++bit) & mask]) {
      if (_cel_MapValueKey_Equals(&ent->key, key)) {
        *value = ent->value;
        return true;
      }
    }
  }

  return false;
}

static bool _cel_MutableMapValue_Has(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValueKey* cel_nonnull key, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_MutableMapValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  uint32_t len = content.u32[_cel_MutableMapValueContent_kLenOffset];
  if (len != 0) {
    _cel_MutableMapValueEntry** ents =
        (_cel_MutableMapValueEntry**)
            content.ptr[_cel_MutableMapValueContent_kPtrOffset];
    const size_t mask =
        (size_t)content.u32[_cel_MutableMapValueContent_kCapOffset] - 1;
    size_t bit = cel_HashState_Finalize(
                     _cel_MapValueKey_Hash(key, cel_HashState_Initialize())) &
                 mask;
    _cel_MutableMapValueEntry* ent;
    for (ent = ents[bit]; ent != cel_nullptr; ent = ents[(++bit) & mask]) {
      if (_cel_MapValueKey_Equals(&ent->key, key)) {
        cel_Value_SetTrue(result);
        return true;
      }
    }
  }

  cel_Value_SetFalse(result);
  return true;
}

static void _cel_MutableMapValue_Rehash(_cel_MutableMapValueEntry** new_ents,
                                        size_t new_cap,
                                        _cel_MutableMapValueEntry** old_ents,
                                        size_t old_cap, size_t len) {
  const size_t new_mask = new_cap - 1;
  const cel_HashState state = cel_HashState_Initialize();
  size_t old_bit;
  for (old_bit = 0; len > 0; ++old_bit) {
    _cel_MutableMapValueEntry* ent = old_ents[old_bit];
    if (ent == cel_nullptr) {
      continue;
    }
    size_t new_bit =
        cel_HashState_Finalize(_cel_MapValueKey_Hash(&ent->key, state)) &
        new_mask;
    for (; new_ents[new_bit] != cel_nullptr;
         new_bit = (new_bit + 1) & new_mask) {
    }
    new_ents[new_bit] = ent;
    --len;
  }
}

static bool _cel_MutableMapValue_Resize(cel_MapValue* cel_nonnull map_value,
                                        _cel_MutableMapValueEntry** old_ents,
                                        uint32_t new_cap, uint32_t old_cap,
                                        uint32_t len,
                                        cel_Arena* cel_nonnull arena) {
  CEL_ASSERT(new_cap > old_cap && _cel_has_single_bit(new_cap) &&
             (old_cap == 0 || _cel_has_single_bit(old_cap)));
  _cel_MutableMapValueEntry** new_ents =
      (_cel_MutableMapValueEntry**)cel_Arena_Malloc(
          arena, sizeof(_cel_MutableMapValueEntry*) * new_cap, cel_nullptr);
  if (CEL_UNLIKELY(new_ents == cel_nullptr)) {
    return false;
  }
  memset(new_ents, 0, sizeof(_cel_MutableMapValueEntry*) * new_cap);
  _cel_MutableMapValue_Rehash(new_ents, new_cap, old_ents, old_cap, len);
  map_value->content.ptr[_cel_MutableMapValueContent_kPtrOffset] = new_ents;
  map_value->content.u32[_cel_MutableMapValueContent_kCapOffset] = new_cap;
  return true;
}

extern "C" bool _cel_MutableMapValue_Reserve(
    cel_MapValue* cel_nonnull map_value, uint32_t size,
    cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(map_value);
  CEL_ASSERT_EQ(map_value->vtable, &_cel_MutableMapValueVTable);
  CEL_ASSERT_NOT_NULL(arena);

  uint32_t cap = map_value->content.u32[_cel_MutableMapValueContent_kCapOffset];
  if (cap >= size) {
    return true;
  }
  if (size > (((uint32_t)1) << ((sizeof(uint32_t) * CHAR_BIT) - 1))) {
    return false;
  }
  size = _cel_bit_ceil(size);
  if (size < 16) {
    size = 16;
  }
  return _cel_MutableMapValue_Resize(
      map_value,
      (_cel_MutableMapValueEntry**)
          map_value->content.ptr[_cel_MutableMapValueContent_kPtrOffset],
      _cel_bit_ceil(size), cap,
      map_value->content.u32[_cel_MutableMapValueContent_kLenOffset], arena);
}

extern "C" _cel_MutableMapValueInsertResult _cel_MutableMapValue_Insert(
    cel_MapValue* cel_nonnull map_value, const cel_MapValueKey* cel_nonnull key,
    cel_MapValueKey * cel_nullable * cel_nullable out_key,
    cel_Value * cel_nullable * cel_nonnull out_value,
    cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(map_value);
  CEL_ASSERT_EQ(map_value->vtable, &_cel_MutableMapValueVTable);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(out_value);
  CEL_ASSERT_NOT_NULL(arena);

  uint32_t cap = map_value->content.u32[_cel_MutableMapValueContent_kCapOffset];
  uint32_t len;
  if (CEL_UNLIKELY(cap == 0)) {
    cap = 16;
    len = 0;
    _cel_MutableMapValueEntry** ents =
        (_cel_MutableMapValueEntry**)cel_Arena_Malloc(
            arena, sizeof(_cel_MutableMapValueEntry*) * cap, cel_nullptr);
    if (CEL_UNLIKELY(ents == cel_nullptr)) {
      return _cel_MutableMapValueInsertResult_kOutOfMemory;
    }
    memset(ents, 0, sizeof(_cel_MutableMapValueEntry*) * cap);
    map_value->content.ptr[_cel_MutableMapValueContent_kPtrOffset] = ents;
    map_value->content.u32[_cel_MutableMapValueContent_kCapOffset] = cap;
  } else {
    len = map_value->content.u32[_cel_MutableMapValueContent_kLenOffset];
  }
  _cel_MutableMapValueEntry** ents =
      (_cel_MutableMapValueEntry**)
          map_value->content.ptr[_cel_MutableMapValueContent_kPtrOffset];
  const size_t mask =
      (size_t)map_value->content.u32[_cel_MutableMapValueContent_kCapOffset] -
      1;
  size_t bit = cel_HashState_Finalize(
                   _cel_MapValueKey_Hash(key, cel_HashState_Initialize())) &
               mask;
  _cel_MutableMapValueEntry* ent;
  for (ent = ents[bit]; ent != cel_nullptr; ent = ents[(++bit) & mask]) {
    if (_cel_MapValueKey_Equals(&ent->key, key)) {
      if (out_key != cel_nullptr) {
        *out_key = &ent->key;
      }
      *out_value = &ent->value;
      return _cel_MutableMapValueInsertResult_kReplaced;
    }
  }
  size_t threshold = (size_t)cap * 2 / 3;
  if (len > threshold) {
    // Refuse insertion. We must have failed to resize before.
    return _cel_MutableMapValueInsertResult_kOutOfMemory;
  }
  ent = reinterpret_cast<_cel_MutableMapValueEntry*>(
      cel_Arena_Malloc(arena, sizeof(_cel_MutableMapValueEntry), cel_nullptr));
  if (CEL_UNLIKELY(ent == cel_nullptr)) {
    return _cel_MutableMapValueInsertResult_kOutOfMemory;
  }
  ent->key = *key;
  if (out_key != cel_nullptr) {
    *out_key = &ent->key;
  }
  *out_value = &ent->value;
  ents[bit & mask] = ent;
  map_value->content.u32[_cel_MutableMapValueContent_kLenOffset] = ++len;

  if (len > threshold) {
    (void)_cel_MutableMapValue_Resize(map_value, ents, cap << 1, cap, len,
                                      arena);
  }

  return _cel_MutableMapValueInsertResult_kInserted;
}

static cel_MapValueIterator* cel_nullable _cel_MutableMapValue_NewIterator(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_MutableMapValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return cel_nullptr;
  }

  uint32_t len = content.u32[_cel_MutableMapValueContent_kLenOffset];
  if (len == 0) {
    return &_cel_EmptyMapValueIterator;
  }

  _cel_MutableMapValueIterator* iter =
      (_cel_MutableMapValueIterator*)cel_Allocator_Malloc(
          context->alloc, sizeof(_cel_MutableMapValueIterator), cel_nullptr);
  if (CEL_UNLIKELY(iter == cel_nullptr)) {
    cel_OutOfMemoryStatus(status);
    return cel_nullptr;
  }

  _cel_MutableMapValueEntry** ents =
      (_cel_MutableMapValueEntry**)
          content.ptr[_cel_MutableMapValueContent_kPtrOffset];
  iter->super.vtable = &_cel_MutableMapValueIteratorVTable;
  iter->alloc = context->alloc;
  iter->ents = ents;
  iter->len = iter->rem = len;
  iter->cap = content.u32[_cel_MutableMapValueContent_kCapOffset];

  size_t bit;
  for (bit = 0; ents[bit] == cel_nullptr; ++bit) {
  }
  iter->bit = bit;

  return &iter->super;
}

extern "C" const cel_MapValueVTable _cel_MutableMapValueVTable = {
    .Equals = cel_nullptr,
    .FastSize = &_cel_MutableMapValue_FastSize,
    .SlowSize = &_cel_MutableMapValue_SlowSize,
    .Get = &_cel_MutableMapValue_Get,
    .Find = &_cel_MutableMapValue_Find,
    .Has = &_cel_MutableMapValue_Has,
    .NewIterator = &_cel_MutableMapValue_NewIterator,
};

static void _cel_MutableMapValueIterator_Delete(
    cel_ValueIterator* cel_nonnull iterator) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(
      iterator->vtable,
      (const cel_ValueIteratorVTable*)&_cel_MutableMapValueIteratorVTable);

  _cel_MutableMapValueIterator* iter =
      cel_containerof(iterator, _cel_MutableMapValueIterator, super);
  cel_Allocator_FreeSized(iter->alloc, iter, sizeof(*iter));
}

static bool _cel_MutableMapValueIterator_Next1(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_Value* cel_nonnull key_or_value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_MutableMapValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key_or_value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  _cel_MutableMapValueIterator* iter =
      cel_containerof(iterator, _cel_MutableMapValueIterator, super);
  if (_cel_MutableMapValueIterator_Done(iter)) {
    return false;
  }

  _cel_Value_SetMapValueKey(key_or_value, &iter->ents[iter->bit]->key);
  --iter->rem;
  if (iter->rem != 0) {
    iter->bit = _cel_MutableMapValueIterator_Advance(iter);
  }
  return true;
}

static bool _cel_MutableMapValueIterator_Next2(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull key,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_MutableMapValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  _cel_MutableMapValueIterator* iter =
      cel_containerof(iterator, _cel_MutableMapValueIterator, super);
  if (_cel_MutableMapValueIterator_Done(iter)) {
    return false;
  }

  _cel_MutableMapValueEntry* ent = iter->ents[iter->bit];
  _cel_Value_SetMapValueKey(key, &ent->key);
  *value = ent->value;
  --iter->rem;
  if (iter->rem != 0) {
    iter->bit = _cel_MutableMapValueIterator_Advance(iter);
  }
  return true;
}

static bool _cel_MutableMapValueIterator_Remaining(
    const cel_ValueIterator* cel_nonnull iterator,
    size_t* cel_nonnull remaining) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_MutableMapValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(remaining);

  _cel_MutableMapValueIterator* iter =
      cel_containerof(iterator, _cel_MutableMapValueIterator, super);

  *remaining = iter->rem;
  return true;
}

static bool _cel_MutableMapValueIterator_Next(
    cel_MapValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_MapValueKey* cel_nullable key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_MutableMapValueIteratorVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  _cel_MutableMapValueIterator* iter =
      cel_containerof(iterator, _cel_MutableMapValueIterator, super);
  if (_cel_MutableMapValueIterator_Done(iter)) {
    return false;
  }

  _cel_MutableMapValueEntry* ent = iter->ents[iter->bit];
  if (key != cel_nullptr) {
    *key = ent->key;
  }
  *value = ent->value;
  --iter->rem;
  if (iter->rem != 0) {
    iter->bit = _cel_MutableMapValueIterator_Advance(iter);
  }
  return true;
}

const cel_MapValueIteratorVTable _cel_MutableMapValueIteratorVTable = {
    .super =
        {
            .Delete = &_cel_MutableMapValueIterator_Delete,
            .Next1 = &_cel_MutableMapValueIterator_Next1,
            .Next2 = &_cel_MutableMapValueIterator_Next2,
            .Remaining = &_cel_MutableMapValueIterator_Remaining,
        },
    .Next = &_cel_MutableMapValueIterator_Next,
};
