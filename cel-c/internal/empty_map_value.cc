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

#include "cel-c/internal/empty_map_value.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/value.h"

extern const cel_MapValueIteratorVTable _cel_EmptyMapValueIteratorVTable;

static bool _cel_EmptyMapValue_FastSize(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    size_t* cel_nonnull size) {
  CEL_ASSERT_EQ(vtable, &_cel_EmptyMapValueVTable);
  CEL_ASSERT_NOT_NULL(size);

  *size = 0;
  return true;
}

static bool _cel_EmptyMapValue_SlowSize(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull size,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_EmptyMapValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(size);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Value_SetInt(size, 0);
  return true;
}

static bool _cel_EmptyMapValue_Get(const cel_MapValueVTable* cel_nonnull vtable,
                                   cel_ValueContent content,
                                   const cel_ValueContext* cel_nonnull context,
                                   const cel_MapValueKey* cel_nonnull key,
                                   cel_Value* cel_nonnull value,
                                   cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_EmptyMapValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
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

static bool _cel_EmptyMapValue_Find(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValueKey* cel_nonnull key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_EmptyMapValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  return false;
}

static bool _cel_EmptyMapValue_Has(const cel_MapValueVTable* cel_nonnull vtable,
                                   cel_ValueContent content,
                                   const cel_ValueContext* cel_nonnull context,
                                   const cel_MapValueKey* cel_nonnull key,
                                   cel_Value* cel_nonnull result,
                                   cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_EmptyMapValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Value_SetFalse(result);
  return true;
}

static cel_MapValueIterator* cel_nullable _cel_EmptyMapValue_NewIterator(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_EmptyMapValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return cel_nullptr;
  }

  return &_cel_EmptyMapValueIterator;
}

const cel_MapValueVTable _cel_EmptyMapValueVTable = {
    .Equals = cel_nullptr,
    .FastSize = &_cel_EmptyMapValue_FastSize,
    .SlowSize = &_cel_EmptyMapValue_SlowSize,
    .Get = &_cel_EmptyMapValue_Get,
    .Find = &_cel_EmptyMapValue_Find,
    .Has = &_cel_EmptyMapValue_Has,
    .NewIterator = &_cel_EmptyMapValue_NewIterator,
};

static bool _cel_EmptyMapValueIterator_Next1(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_Value* cel_nonnull key_or_value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_EmptyMapValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key_or_value);
  CEL_ASSERT_NOT_NULL(status);

  return false;
}

static bool _cel_EmptyMapValueIterator_Next2(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull key,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_EmptyMapValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  return false;
}

static bool _cel_EmptyMapValueIterator_Remaining(
    const cel_ValueIterator* cel_nonnull iterator,
    size_t* cel_nonnull remaining) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_EmptyMapValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(remaining);

  *remaining = 0;
  return true;
}

static bool _cel_EmptyMapValueIterator_Next(
    cel_MapValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_MapValueKey* cel_nullable key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_EmptyMapValueIteratorVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  return false;
}

const cel_MapValueIteratorVTable _cel_EmptyMapValueIteratorVTable = {
    .super =
        {
            .Delete = cel_nullptr,
            .Next1 = &_cel_EmptyMapValueIterator_Next1,
            .Next2 = &_cel_EmptyMapValueIterator_Next2,
            .Remaining = &_cel_EmptyMapValueIterator_Remaining,
        },
    .Next = &_cel_EmptyMapValueIterator_Next,
};

extern "C" cel_MapValueIterator _cel_EmptyMapValueIterator = {
    .vtable = &_cel_EmptyMapValueIteratorVTable,
};
