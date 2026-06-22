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

#include "cel-c/internal/parsed_map_field_value.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/internal/map_value.h"
#include "cel-c/internal/message_equality.h"
#include "cel-c/status.h"
#include "cel-c/status_code.h"
#include "cel-c/string_view.h"
#include "cel-c/value.h"
#include "upb/message/array.h"
#include "upb/message/map.h"
#include "upb/message/message.h"
#include "upb/reflection/def.h"

extern const cel_MapValueIteratorVTable _cel_ParsedMapFieldValueIteratorVTable;

typedef struct {
  cel_MapValueIterator base;
  cel_Allocator* cel_nonnull alloc;
  const upb_Message* cel_nonnull message;
  const upb_FieldDef* cel_nonnull field_key_def;
  const upb_FieldDef* cel_nonnull field_val_def;
  const upb_Map* cel_nullability_unknown field_val;
  size_t iter;
  size_t remain;
} _cel_ParsedMapFieldValueIterator;

static bool _cel_ParsedMapFieldValue_Equals(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedMapFieldValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(other);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  if (other->vtable == &_cel_ParsedMapFieldValueVTable) {
    upb_MessageValue field_val;
    upb_MessageValue other_field_val;
    field_val.map_val = (const upb_Map*)content.ptr[0];
    other_field_val.map_val = (const upb_Map*)other->content.ptr[0];
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

static bool _cel_ParsedMapFieldValue_FastSize(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    size_t* cel_nonnull size) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedMapFieldValueVTable);
  CEL_ASSERT_NOT_NULL(size);

  const upb_Map* field_val = (const upb_Map*)content.ptr[0];
  *size = field_val != cel_nullptr ? upb_Map_Size(field_val) : (size_t)0;
  return true;
}

static bool _cel_ParsedMapFieldValue_SlowSize(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull size,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedMapFieldValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(size);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_Map* field_val = (const upb_Map*)content.ptr[0];
  cel_Value_SetInt(size, field_val != cel_nullptr
                             ? (int64_t)upb_Map_Size(field_val)
                             : (int64_t)0);
  return true;
}

static bool _cel_ParsedMapFieldValue_Get(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValueKey* cel_nonnull key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedMapFieldValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_FieldDef* field_def = (const upb_FieldDef*)content.ptr[1];
  const upb_Map* field_val = (const upb_Map*)content.ptr[0];

  if (field_val != cel_nullptr && upb_Map_Size(field_val) > 0) {
    const upb_MessageDef* field_entry_def =
        upb_FieldDef_MessageSubDef(field_def);
    const upb_FieldDef* field_key_def = upb_MessageDef_FindFieldByNumber(
        field_entry_def, kUpb_MapEntry_KeyFieldNumber);

    upb_MessageValue message_key;
    if (!_cel_MapValueKey_ToMessageValue(key, upb_FieldDef_CType(field_key_def),
                                         &message_key)) {
      goto no_such_key;
    }

    upb_MessageValue message_val;
    if (!upb_Map_Get(field_val, message_key, &message_val)) {
      goto no_such_key;
    }

    const upb_FieldDef* field_val_def = upb_MessageDef_FindFieldByNumber(
        field_entry_def, kUpb_MapEntry_ValueFieldNumber);
    return cel_Value_FromMapFieldValue(value, context, message_val,
                                       field_val_def, status);
  }

no_such_key: {
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
}

static bool _cel_ParsedMapFieldValue_Find(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValueKey* cel_nonnull key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedMapFieldValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_FieldDef* field_def = (const upb_FieldDef*)content.ptr[1];
  const upb_Map* field_val = (const upb_Map*)content.ptr[0];

  if (field_val != cel_nullptr && upb_Map_Size(field_val) > 0) {
    const upb_MessageDef* field_entry_def =
        upb_FieldDef_MessageSubDef(field_def);
    const upb_FieldDef* field_key_def = upb_MessageDef_FindFieldByNumber(
        field_entry_def, kUpb_MapEntry_KeyFieldNumber);

    upb_MessageValue message_key;
    if (!_cel_MapValueKey_ToMessageValue(key, upb_FieldDef_CType(field_key_def),
                                         &message_key)) {
      return false;
    }

    upb_MessageValue message_val;
    if (!upb_Map_Get(field_val, message_key, &message_val)) {
      return false;
    }

    const upb_FieldDef* field_val_def = upb_MessageDef_FindFieldByNumber(
        field_entry_def, kUpb_MapEntry_ValueFieldNumber);
    return cel_Value_FromMapFieldValue(value, context, message_val,
                                       field_val_def, status);
  }

  return false;
}

static bool _cel_ParsedMapFieldValue_Has(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValueKey* cel_nonnull key, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedMapFieldValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  const upb_FieldDef* field_def = (const upb_FieldDef*)content.ptr[1];
  const upb_Map* field_val = (const upb_Map*)content.ptr[0];

  if (field_val != cel_nullptr && upb_Map_Size(field_val) > 0) {
    const upb_MessageDef* field_entry_def =
        upb_FieldDef_MessageSubDef(field_def);
    const upb_FieldDef* field_key_def = upb_MessageDef_FindFieldByNumber(
        field_entry_def, kUpb_MapEntry_KeyFieldNumber);

    upb_MessageValue message_key;
    if (!_cel_MapValueKey_ToMessageValue(key, upb_FieldDef_CType(field_key_def),
                                         &message_key)) {
      return false;
    }

    upb_MessageValue message_val;
    cel_Value_SetBool(result,
                      upb_Map_Get(field_val, message_key, &message_val));
    return true;
  }

  cel_Value_SetFalse(result);
  return true;
}

static cel_MapValueIterator* cel_nullable _cel_ParsedMapFieldValue_NewIterator(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_EQ(vtable, &_cel_ParsedMapFieldValueVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return cel_nullptr;
  }

  _cel_ParsedMapFieldValueIterator* iter =
      (_cel_ParsedMapFieldValueIterator*)cel_Allocator_Malloc(
          context->alloc, sizeof(_cel_ParsedMapFieldValueIterator),
          cel_nullptr);
  if (CEL_UNLIKELY(iter == cel_nullptr)) {
    cel_OutOfMemoryStatus(status);
    return cel_nullptr;
  }
  memset(iter, 0, sizeof(*iter));
  iter->base.vtable = &_cel_ParsedMapFieldValueIteratorVTable;
  iter->alloc = context->alloc;
  const upb_FieldDef* field_def = (const upb_FieldDef*)content.ptr[1];
  iter->field_val = (const upb_Map*)content.ptr[0];
  const upb_MessageDef* field_entry_def = upb_FieldDef_MessageSubDef(field_def);
  iter->field_key_def = upb_MessageDef_FindFieldByNumber(
      field_entry_def, kUpb_MapEntry_KeyFieldNumber);
  iter->field_val_def = upb_MessageDef_FindFieldByNumber(
      field_entry_def, kUpb_MapEntry_ValueFieldNumber);
  iter->iter = kUpb_Map_Begin;
  iter->remain = iter->field_val != cel_nullptr ? upb_Map_Size(iter->field_val)
                                                : (size_t)0;
  return &iter->base;
}

extern "C" const cel_MapValueVTable _cel_ParsedMapFieldValueVTable = {
    .Equals = &_cel_ParsedMapFieldValue_Equals,
    .FastSize = &_cel_ParsedMapFieldValue_FastSize,
    .SlowSize = &_cel_ParsedMapFieldValue_SlowSize,
    .Get = &_cel_ParsedMapFieldValue_Get,
    .Find = &_cel_ParsedMapFieldValue_Find,
    .Has = &_cel_ParsedMapFieldValue_Has,
    .NewIterator = &_cel_ParsedMapFieldValue_NewIterator,
};

static void _cel_ParsedMapFieldValueIterator_Delete(
    cel_ValueIterator* cel_nonnull iterator) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable,
                &_cel_ParsedMapFieldValueIteratorVTable.super);

  _cel_ParsedMapFieldValueIterator* iter =
      cel_containerof(iterator, _cel_ParsedMapFieldValueIterator, base);
  cel_Allocator_FreeSized(iter->alloc, iter, sizeof(*iter));
}

static bool _cel_ParsedMapFieldValueIterator_Next1(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_Value* cel_nonnull key_or_value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable,
                &_cel_ParsedMapFieldValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key_or_value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  _cel_ParsedMapFieldValueIterator* iter =
      cel_containerof(iterator, _cel_ParsedMapFieldValueIterator, base);

  upb_MessageValue entry_key;
  upb_MessageValue entry_val;
  size_t iter_next = iter->iter;
  if (iter->field_val != cel_nullptr &&
      upb_Map_Next(iter->field_val, &entry_key, &entry_val, &iter_next)) {
    const bool ok = cel_Value_FromMapFieldKey(key_or_value, context, entry_key,
                                              iter->field_key_def, status);
    if (ok) {
      iter->iter = iter_next;
      --iter->remain;
    }
    return ok;
  }

  return false;
}

static bool _cel_ParsedMapFieldValueIterator_Next2(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull key,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable,
                &_cel_ParsedMapFieldValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  _cel_ParsedMapFieldValueIterator* iter =
      cel_containerof(iterator, _cel_ParsedMapFieldValueIterator, base);

  upb_MessageValue entry_key;
  upb_MessageValue entry_val;
  size_t iter_next = iter->iter;
  if (iter->field_val != cel_nullptr &&
      upb_Map_Next(iter->field_val, &entry_key, &entry_val, &iter_next)) {
    bool ok = cel_Value_FromMapFieldKey(key, context, entry_key,
                                        iter->field_key_def, status);
    if (ok) {
      ok = cel_Value_FromMapFieldValue(value, context, entry_val,
                                       iter->field_val_def, status);
      if (ok) {
        iter->iter = iter_next;
        --iter->remain;
      }
    }
    return ok;
  }

  return false;
}

static bool _cel_ParsedMapFieldValueIterator_Remaining(
    const cel_ValueIterator* cel_nonnull iterator,
    size_t* cel_nonnull remaining) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable,
                &_cel_ParsedMapFieldValueIteratorVTable.super);
  CEL_ASSERT_NOT_NULL(remaining);

  _cel_ParsedMapFieldValueIterator* iter =
      cel_containerof(iterator, _cel_ParsedMapFieldValueIterator, base);
  *remaining = iter->remain;
  return true;
}

static bool _cel_ParsedMapFieldValueIterator_Next(
    cel_MapValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_MapValueKey* cel_nullable key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_EQ(iterator->vtable, &_cel_ParsedMapFieldValueIteratorVTable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  _cel_ParsedMapFieldValueIterator* iter =
      cel_containerof(iterator, _cel_ParsedMapFieldValueIterator, base);

  upb_MessageValue entry_key;
  upb_MessageValue entry_val;
  size_t iter_next = iter->iter;
  if (iter->field_val != cel_nullptr &&
      upb_Map_Next(iter->field_val, &entry_key, &entry_val, &iter_next)) {
    _cel_MapValueKey_FromMessageValue(
        key, upb_FieldDef_CType(iter->field_key_def), entry_key);
    bool ok = cel_Value_FromMapFieldValue(value, context, entry_val,
                                          iter->field_val_def, status);
    if (ok) {
      iter->iter = iter_next;
      --iter->remain;
    }
    return ok;
  }

  return false;
}

const cel_MapValueIteratorVTable _cel_ParsedMapFieldValueIteratorVTable = {
    .super =
        {
            .Delete = &_cel_ParsedMapFieldValueIterator_Delete,
            .Next1 = &_cel_ParsedMapFieldValueIterator_Next1,
            .Next2 = &_cel_ParsedMapFieldValueIterator_Next2,
            .Remaining = &_cel_ParsedMapFieldValueIterator_Remaining,
        },
    .Next = &_cel_ParsedMapFieldValueIterator_Next,
};
