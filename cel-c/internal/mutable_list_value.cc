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

#include "cel-c/internal/mutable_list_value.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/internal/ckdint.h"
#include "cel-c/internal/empty_list_value.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/value.h"

extern const cel_ListValueIteratorVTable _cel_MutableListValueIteratorVTable;

typedef struct {
  union {
    struct {
      cel_Value* cel_nullable ptr;
      uint32_t len;
      uint32_t cap;
    };
    cel_ValueContent super;
  };
} _cel_MutableListValueContent;

typedef struct {
  cel_ListValueIterator super;
  cel_Allocator* cel_nonnull alloc;
  cel_Value* cel_nonnull ptr;
  uint32_t len;
  uint32_t idx;
} _cel_MutableListValueIterator;

#define _cel_MutableListValueContent_kPtrOffset 0
#define _cel_MutableListValueContent_kLenOffset \
  (sizeof(void*) / sizeof(uint32_t))
#define _cel_MutableListValueContent_kCapOffset \
  (_cel_MutableListValueContent_kLenOffset + 1)

CEL_STATIC_ASSERT(sizeof(_cel_MutableListValueContent) <=
                  sizeof(cel_ValueContent));
CEL_STATIC_ASSERT(alignof(_cel_MutableListValueContent) <=
                  alignof(cel_ValueContent));

static bool _cel_MutableListValue_FastSize(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    size_t* cel_nonnull size) {
  CEL_ASSERT_EQ(vtable, &_cel_MutableListValueVTable);
  CEL_ASSERT_NOT_NULL(size);

  *size = content.u32[_cel_MutableListValueContent_kLenOffset];
  return true;
}

static bool _cel_MutableListValue_SlowSize(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull size,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_MutableListValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(size);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Value_SetInt(size, content.u32[_cel_MutableListValueContent_kLenOffset]);
  return true;
}

static bool _cel_MutableListValue_Get(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, size_t index,
    cel_Value* cel_nonnull element, cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_MutableListValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(element);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  uint32_t len = content.u32[_cel_MutableListValueContent_kLenOffset];
  if (CEL_LIKELY(index < len)) {
    const cel_Value* ptr =
        (cel_Value*)content.ptr[_cel_MutableListValueContent_kPtrOffset];
    *element = ptr[index];
    return true;
  }

  cel_Error* error = cel_Error_New(context->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    cel_OutOfMemoryStatus(status);
    return false;
  }
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kOutOfRange);
  cel_Error_SetMessage(error, cel_StringView_From("index out of range"));
  cel_Value_SetError(element, error);
  return true;
}

extern "C" bool _cel_MutableListValue_Reserve(
    cel_ListValue* cel_nonnull list_value, uint32_t size,
    cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(list_value);
  CEL_ASSERT(_cel_ListValue_IsMutable(list_value));
  CEL_ASSERT_NOT_NULL(arena);

  uint32_t cap =
      list_value->content.u32[_cel_MutableListValueContent_kCapOffset];
  if (size <= cap) {
    return true;
  }

  if (size <= 8) {
    size = 8;
  }
  cel_Value* ptr =
      (cel_Value*)
          list_value->content.ptr[_cel_MutableListValueContent_kPtrOffset];
  size_t new_cap_bytes;
  if (_cel_ckd_mul(&new_cap_bytes, (size_t)size, sizeof(cel_Value))) {
    return false;
  }
  cel_Value* new_ptr = (cel_Value*)cel_Arena_Realloc(
      arena, ptr, (size_t)cap * sizeof(cel_Value), new_cap_bytes, cel_nullptr);
  if (CEL_UNLIKELY(new_ptr == cel_nullptr)) {
    return false;
  }
  list_value->content.ptr[_cel_MutableListValueContent_kPtrOffset] = new_ptr;
  list_value->content.u32[_cel_MutableListValueContent_kCapOffset] = size;
  return true;
}

extern "C" cel_Value* cel_nullable
_cel_MutableListValue_AddN(cel_ListValue* cel_nonnull list_value, uint32_t size,
                           cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(list_value);
  CEL_ASSERT(_cel_ListValue_IsMutable(list_value));
  CEL_ASSERT_NOT_NULL(arena);

  uint32_t len =
      list_value->content.u32[_cel_MutableListValueContent_kLenOffset];
  uint32_t cap =
      list_value->content.u32[_cel_MutableListValueContent_kCapOffset];
  cel_Value* ptr =
      (cel_Value*)
          list_value->content.ptr[_cel_MutableListValueContent_kPtrOffset];
  if (CEL_UNLIKELY(size > len || len > cap - size)) {
    if (cap == UINT32_MAX) {
      return cel_nullptr;
    }
    uint32_t min_cap;
    if (_cel_ckd_add(&min_cap, len, size)) {
      return cel_nullptr;
    }
    uint32_t new_cap;
    if (_cel_ckd_mul(&new_cap, cap, (uint32_t)2)) {
      new_cap = UINT32_MAX;
    }
    if (new_cap < 8) {
      new_cap = 8;
    }
    while (new_cap < min_cap) {
      if (_cel_ckd_mul(&new_cap, new_cap, (uint32_t)2)) {
        new_cap = UINT32_MAX;
        break;
      }
    }
    size_t new_cap_bytes;
    if (_cel_ckd_mul(&new_cap_bytes, (size_t)new_cap, sizeof(cel_Value))) {
      return cel_nullptr;
    }
    ptr = (cel_Value*)cel_Arena_Realloc(arena, ptr,
                                        (size_t)cap * sizeof(cel_Value),
                                        new_cap_bytes, cel_nullptr);
    if (CEL_UNLIKELY(ptr == cel_nullptr)) {
      return cel_nullptr;
    }
    list_value->content.ptr[_cel_MutableListValueContent_kPtrOffset] = ptr;
    list_value->content.u32[_cel_MutableListValueContent_kCapOffset] = new_cap;
  }
  list_value->content.u32[_cel_MutableListValueContent_kLenOffset] = len + size;
  return ptr + len;
}

static cel_ListValueIterator* cel_nullable _cel_MutableListValue_NewIterator(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_MutableListValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return cel_nullptr;
  }

  uint32_t len = content.u32[_cel_MutableListValueContent_kLenOffset];
  if (len == 0) {
    return &_cel_EmptyListValueIterator;
  }

  _cel_MutableListValueIterator* iter =
      (_cel_MutableListValueIterator*)cel_Allocator_Malloc(
          context->alloc, sizeof(_cel_MutableListValueIterator), cel_nullptr);
  if (CEL_UNLIKELY(iter == cel_nullptr)) {
    cel_OutOfMemoryStatus(status);
    return cel_nullptr;
  }
  iter->super.vtable = &_cel_MutableListValueIteratorVTable;
  iter->alloc = context->alloc;
  iter->ptr = (cel_Value*)content.ptr[_cel_MutableListValueContent_kPtrOffset];
  iter->len = len;
  iter->idx = 0;

  return &iter->super;
}

extern "C" const cel_ListValueVTable _cel_MutableListValueVTable = {
    .Equals = cel_nullptr,
    .FastSize = &_cel_MutableListValue_FastSize,
    .SlowSize = &_cel_MutableListValue_SlowSize,
    .Get = &_cel_MutableListValue_Get,
    .NewIterator = &_cel_MutableListValue_NewIterator,
};

static void _cel_MutableListValueIterator_Delete(
    cel_ValueIterator* cel_nonnull iterator) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(
      iterator->vtable,
      (const cel_ValueIteratorVTable*)&_cel_MutableListValueIteratorVTable);

  _cel_MutableListValueIterator* iter =
      cel_containerof(iterator, _cel_MutableListValueIterator, super);
  cel_Allocator_FreeSized(iter->alloc, iter, sizeof(*iter));
}

static bool _cel_MutableListValueIterator_Next1(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_Value* cel_nonnull key_or_value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_MutableListValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key_or_value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  _cel_MutableListValueIterator* iter =
      cel_containerof(iterator, _cel_MutableListValueIterator, super);
  if (CEL_UNLIKELY(iter->idx >= iter->len)) {
    return false;
  }

  *key_or_value = iter->ptr[iter->idx];
  ++iter->idx;
  return true;
}

static bool _cel_MutableListValueIterator_Next2(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull key,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_MutableListValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  _cel_MutableListValueIterator* iter =
      cel_containerof(iterator, _cel_MutableListValueIterator, super);
  if (CEL_UNLIKELY(iter->idx >= iter->len)) {
    return false;
  }

  cel_Value_SetInt(key, iter->idx);
  *value = iter->ptr[iter->idx];
  ++iter->idx;
  return true;
}

static bool _cel_MutableListValueIterator_Remaining(
    const cel_ValueIterator* cel_nonnull iterator,
    size_t* cel_nonnull remaining) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_MutableListValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(remaining);

  const _cel_MutableListValueIterator* iter =
      cel_containerof(iterator, _cel_MutableListValueIterator, super);

  *remaining = iter->len - iter->idx;
  return true;
}

static bool _cel_MutableListValueIterator_Next(
    cel_ListValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, size_t* cel_nullable index,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_MutableListValueIteratorVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  _cel_MutableListValueIterator* iter =
      cel_containerof(iterator, _cel_MutableListValueIterator, super);
  if (CEL_UNLIKELY(iter->idx >= iter->len)) {
    return false;
  }

  if (index != cel_nullptr) {
    *index = iter->idx;
  }
  *value = iter->ptr[iter->idx];
  ++iter->idx;
  return true;
}

const cel_ListValueIteratorVTable _cel_MutableListValueIteratorVTable = {
    .super =
        {
            .Delete = &_cel_MutableListValueIterator_Delete,
            .Next1 = &_cel_MutableListValueIterator_Next1,
            .Next2 = &_cel_MutableListValueIterator_Next2,
            .Remaining = &_cel_MutableListValueIterator_Remaining,
        },
    .Next = &_cel_MutableListValueIterator_Next,
};
