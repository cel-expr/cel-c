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

#include "cel-c/internal/empty_list_value.h"

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

extern const cel_ListValueIteratorVTable _cel_EmptyListValueIteratorVTable;

static bool _cel_EmptyListValue_FastSize(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    size_t* cel_nonnull size) {
  CEL_ASSERT_EQ(vtable, &_cel_EmptyListValueVTable);
  CEL_ASSERT_NOT_NULL(size);

  *size = 0;
  return true;
}

static bool _cel_EmptyListValue_SlowSize(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull size,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_EmptyListValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(size);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Value_SetInt(size, 0);
  return true;
}

static bool _cel_EmptyListValue_Get(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, size_t index,
    cel_Value* cel_nonnull element, cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_EmptyListValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(element);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
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

static cel_ListValueIterator* cel_nullable _cel_EmptyListValue_NewIterator(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_EmptyListValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return cel_nullptr;
  }

  return &_cel_EmptyListValueIterator;
}

const cel_ListValueVTable _cel_EmptyListValueVTable = {
    .Equals = cel_nullptr,
    .FastSize = &_cel_EmptyListValue_FastSize,
    .SlowSize = &_cel_EmptyListValue_SlowSize,
    .Get = &_cel_EmptyListValue_Get,
    .NewIterator = &_cel_EmptyListValue_NewIterator,
};

static bool _cel_EmptyListValueIterator_Next1(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_Value* cel_nonnull key_or_value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_EmptyListValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key_or_value);
  CEL_ASSERT_NOT_NULL(status);

  return false;
}

static bool _cel_EmptyListValueIterator_Next2(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull key,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_EmptyListValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  return false;
}

static bool _cel_EmptyListValueIterator_Remaining(
    const cel_ValueIterator* cel_nonnull iterator,
    size_t* cel_nonnull remaining) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_EmptyListValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(remaining);

  *remaining = 0;
  return true;
}

static bool _cel_EmptyListValueIterator_Next(
    cel_ListValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, size_t* cel_nullable index,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_EmptyListValueIteratorVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  return false;
}

const cel_ListValueIteratorVTable _cel_EmptyListValueIteratorVTable = {
    .super =
        {
            .Delete = cel_nullptr,
            .Next1 = &_cel_EmptyListValueIterator_Next1,
            .Next2 = &_cel_EmptyListValueIterator_Next2,
            .Remaining = &_cel_EmptyListValueIterator_Remaining,
        },
    .Next = &_cel_EmptyListValueIterator_Next,
};

extern "C" cel_ListValueIterator _cel_EmptyListValueIterator = {
    .vtable = &_cel_EmptyListValueIteratorVTable,
};
