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

#include "cel-c/internal/parsed_repeated_field_value.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/assert.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/message_equality.h"
#include "cel-c/status.h"
#include "cel-c/status_code.h"
#include "cel-c/string_view.h"
#include "cel-c/value.h"
#include "upb/message/array.h"
#include "upb/message/message.h"
#include "upb/reflection/def.h"

extern const cel_ListValueIteratorVTable
    _cel_ParsedRepeatedFieldValueIteratorVTable;

typedef struct {
  cel_ListValueIterator base;
  cel_Allocator* cel_nonnull alloc;
  const upb_Message* cel_nonnull message;
  const upb_FieldDef* cel_nonnull field_def;
  const upb_Array* cel_nullability_unknown field_val;
  size_t index;
  size_t remain;
} _cel_ParsedRepeatedFieldValueIterator;

static bool _cel_ParsedRepeatedFieldValue_Equals(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_ListValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedRepeatedFieldValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(other);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  if (other->vtable == &_cel_ParsedRepeatedFieldValueVTable) {
    upb_MessageValue field_val;
    upb_MessageValue other_field_val;
    field_val.array_val = (const upb_Array*)content.ptr[0];
    other_field_val.array_val = (const upb_Array*)other->content.ptr[0];
    const upb_FieldDef* field_def = (const upb_FieldDef*)content.ptr[1];
    const upb_FieldDef* other_field_def =
        (const upb_FieldDef*)other->content.ptr[1];
    switch (_cel_MessageField_Equals(
        field_val, field_def, other_field_val, other_field_def,
        context->def_pool, context->well_known_types, context->alloc)) {
      case _cel_MessageEquality_kEqual:
        cel_Value_SetTrue(result);
        return true;
      case _cel_MessageEquality_kNotEqual:
        cel_Value_SetFalse(result);
        return true;
      case _cel_MessageEquality_kOutOfMemory:
        cel_OutOfMemoryStatus(status);
        return false;
      case _cel_MessageEquality_kMaxDepthExceeded:
        cel_Status_SetCanonicalCode(status, cel_StatusCode_kInvalidArgument);
        cel_Status_SetMessage(
            status, cel_StringView_From("max message depth exceeded"));
        return false;
    }
  }

  // Fallback to default implementation.
  return false;
}

static bool _cel_ParsedRepeatedFieldValue_FastSize(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    size_t* cel_nonnull size) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedRepeatedFieldValueVTable);
  CEL_ASSERT_NOT_NULL(size);

  const upb_Array* field_val = (const upb_Array*)content.ptr[0];
  *size = field_val != cel_nullptr ? upb_Array_Size(field_val) : (size_t)0;
  return true;
}

static bool _cel_ParsedRepeatedFieldValue_SlowSize(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull size,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedRepeatedFieldValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(size);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_Array* field_val = (const upb_Array*)content.ptr[0];
  cel_Value_SetInt(size, field_val != cel_nullptr
                             ? (int64_t)upb_Array_Size(field_val)
                             : (int64_t)0);
  return true;
}

static bool _cel_ParsedRepeatedFieldValue_Get(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, size_t index,
    cel_Value* cel_nonnull element, cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedRepeatedFieldValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(element);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_FieldDef* field_def = (const upb_FieldDef*)content.ptr[1];
  const upb_Array* field_val = (const upb_Array*)content.ptr[0];

  const size_t size =
      field_val != cel_nullptr ? upb_Array_Size(field_val) : (size_t)0;

  if (CEL_UNLIKELY(index >= size)) {
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

  return cel_Value_FromRepeatedFieldElement(
      element, context, upb_Array_Get(field_val, index), field_def, status);
}

static cel_ListValueIterator* cel_nullable
_cel_ParsedRepeatedFieldValue_NewIterator(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedRepeatedFieldValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return cel_nullptr;
  }

  _cel_ParsedRepeatedFieldValueIterator* iter =
      (_cel_ParsedRepeatedFieldValueIterator*)cel_Allocator_Malloc(
          context->alloc, sizeof(_cel_ParsedRepeatedFieldValueIterator),
          cel_nullptr);
  if (CEL_UNLIKELY(iter == cel_nullptr)) {
    cel_OutOfMemoryStatus(status);
    return cel_nullptr;
  }
  memset(iter, 0, sizeof(*iter));
  iter->base.vtable = &_cel_ParsedRepeatedFieldValueIteratorVTable;
  iter->alloc = context->alloc;
  iter->field_def = (const upb_FieldDef*)content.ptr[1];
  iter->field_val = (const upb_Array*)content.ptr[0];
  iter->index = 0;
  iter->remain = iter->field_val != cel_nullptr
                     ? upb_Array_Size(iter->field_val)
                     : (size_t)0;
  return &iter->base;
}

extern "C" const cel_ListValueVTable _cel_ParsedRepeatedFieldValueVTable = {
    .Equals = &_cel_ParsedRepeatedFieldValue_Equals,
    .FastSize = &_cel_ParsedRepeatedFieldValue_FastSize,
    .SlowSize = &_cel_ParsedRepeatedFieldValue_SlowSize,
    .Get = &_cel_ParsedRepeatedFieldValue_Get,
    .NewIterator = &_cel_ParsedRepeatedFieldValue_NewIterator,
};

static void _cel_ParsedRepeatedFieldValueIterator_Delete(
    cel_ValueIterator* cel_nonnull iterator) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(
      iterator->vtable,
      (const cel_ValueIteratorVTable*)&_cel_ParsedRepeatedFieldValueIteratorVTable);

  _cel_ParsedRepeatedFieldValueIterator* iter =
      cel_containerof(iterator, _cel_ParsedRepeatedFieldValueIterator, base);
  cel_Allocator_FreeSized(iter->alloc, iter, sizeof(*iter));
}

static bool _cel_ParsedRepeatedFieldValueIterator_Next1(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_Value* cel_nonnull key_or_value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(
      iterator->vtable,
      (const cel_ValueIteratorVTable*)&_cel_ParsedRepeatedFieldValueIteratorVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key_or_value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  _cel_ParsedRepeatedFieldValueIterator* iter =
      cel_containerof(iterator, _cel_ParsedRepeatedFieldValueIterator, base);
  if (iter->remain == 0) {
    return false;
  }

  const bool ok = cel_Value_FromRepeatedFieldElement(
      key_or_value, context, upb_Array_Get(iter->field_val, iter->index),
      iter->field_def, status);
  if (ok) {
    ++iter->index;
    --iter->remain;
  }
  return ok;
}

static bool _cel_ParsedRepeatedFieldValueIterator_Next2(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull key,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(
      iterator->vtable,
      (const cel_ValueIteratorVTable*)&_cel_ParsedRepeatedFieldValueIteratorVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  _cel_ParsedRepeatedFieldValueIterator* iter =
      cel_containerof(iterator, _cel_ParsedRepeatedFieldValueIterator, base);
  if (iter->remain == 0) {
    return false;
  }

  const bool ok = cel_Value_FromRepeatedFieldElement(
      value, context, upb_Array_Get(iter->field_val, iter->index),
      iter->field_def, status);
  if (ok) {
    cel_Value_SetInt(key, (int64_t)iter->index);
    ++iter->index;
    --iter->remain;
  }
  return ok;
}

static bool _cel_ParsedRepeatedFieldValueIterator_Remaining(
    const cel_ValueIterator* cel_nonnull iterator,
    size_t* cel_nonnull remaining) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(
      iterator->vtable,
      (const cel_ValueIteratorVTable*)&_cel_ParsedRepeatedFieldValueIteratorVTable);
  CEL_ASSERT_NOT_NULL(remaining);

  _cel_ParsedRepeatedFieldValueIterator* iter =
      cel_containerof(iterator, _cel_ParsedRepeatedFieldValueIterator, base);
  *remaining = iter->remain;
  return true;
}

static bool _cel_ParsedRepeatedFieldValueIterator_Next(
    cel_ListValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, size_t* cel_nullable index,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_ParsedRepeatedFieldValueIteratorVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  _cel_ParsedRepeatedFieldValueIterator* iter =
      cel_containerof(iterator, _cel_ParsedRepeatedFieldValueIterator, base);
  if (iter->remain == 0) {
    return false;
  }

  const bool ok = cel_Value_FromRepeatedFieldElement(
      value, context, upb_Array_Get(iter->field_val, iter->index),
      iter->field_def, status);
  if (ok) {
    if (index != cel_nullptr) {
      *index = iter->index;
    }
    ++iter->index;
    --iter->remain;
  }
  return ok;
}

const cel_ListValueIteratorVTable _cel_ParsedRepeatedFieldValueIteratorVTable =
    {
        .super =
            {
                .Delete = &_cel_ParsedRepeatedFieldValueIterator_Delete,
                .Next1 = &_cel_ParsedRepeatedFieldValueIterator_Next1,
                .Next2 = &_cel_ParsedRepeatedFieldValueIterator_Next2,
                .Remaining = &_cel_ParsedRepeatedFieldValueIterator_Remaining,
            },
        .Next = &_cel_ParsedRepeatedFieldValueIterator_Next,
    };
