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

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/value.h"
#include "cel-c/src/array.h"
#include "cel-c/src/sort.h"
#include "upb/reflection/def.h"

typedef struct {
  cel_StringView key;
  cel_Value value;
} _cel_StructValueField;

static int _cel_StructValueField_Compare(
    const void* cel_nullability_unknown lhs,
    const void* cel_nullability_unknown rhs) {
  return cel_StringView_Compare(((const _cel_StructValueField*)lhs)->key,
                                ((const _cel_StructValueField*)rhs)->key);
}

static bool _cel_StructValue_EqualsSlow(
    const cel_StructValue* cel_nonnull struct_value,
    const cel_ValueContext* cel_nonnull context,
    const cel_StructValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  cel_StructValueIterator* lhs_iter =
      cel_StructValue_NewIterator(struct_value, context, status);
  if (CEL_UNLIKELY(lhs_iter == cel_nullptr)) {
    CEL_ASSERT(!cel_Status_Ok(status));
    return false;
  }
  cel_StructValueIterator* rhs_iter =
      cel_StructValue_NewIterator(other, context, status);
  if (CEL_UNLIKELY(rhs_iter == cel_nullptr)) {
    CEL_ASSERT(!cel_Status_Ok(status));
    cel_StructValueIterator_Delete(lhs_iter);
    return false;
  }
  size_t lhs_size;
  size_t rhs_size;
  bool lhs_has_size;
  bool rhs_has_size;
  lhs_has_size = cel_StructValueIterator_Remaining(lhs_iter, &lhs_size);
  rhs_has_size = cel_StructValueIterator_Remaining(rhs_iter, &rhs_size);
  if (lhs_has_size && rhs_has_size) {
    if (lhs_size != rhs_size) {
      cel_Value_SetFalse(result);
      cel_StructValueIterator_Delete(rhs_iter);
      cel_StructValueIterator_Delete(lhs_iter);
      return true;
    }
    if (lhs_size == 0) {
      cel_Value_SetTrue(result);
      cel_StructValueIterator_Delete(rhs_iter);
      cel_StructValueIterator_Delete(lhs_iter);
      return true;
    }
  }
  _cel_Array(_cel_StructValueField) lhs_fields;
  _cel_Array_Construct(&lhs_fields);
  if (lhs_has_size) {
    _cel_Array_Reserve(&lhs_fields, context->alloc, lhs_size);
  }
  _cel_Array(_cel_StructValueField) rhs_fields;
  _cel_Array_Construct(&rhs_fields);
  if (rhs_has_size) {
    _cel_Array_Reserve(&rhs_fields, context->alloc, rhs_size);
  }
  bool ok = true;
  bool lhs_next = true;
  bool rhs_next = true;
  while (true) {
    if (lhs_next) {
      cel_StructValueKey lhs_field_key;
      _cel_StructValueField lhs_field;
      lhs_next = cel_StructValueIterator_Next(lhs_iter, context, &lhs_field_key,
                                              &lhs_field.value, status);
      if (!lhs_next && !cel_Status_Ok(status)) {
        ok = false;
        break;
      }
      if (lhs_next) {
        _cel_StructValueField* lhs_field_ptr =
            _cel_Array_Push(&lhs_fields, context->alloc);
        if (CEL_UNLIKELY(lhs_field_ptr == cel_nullptr)) {
          cel_OutOfMemoryStatus(status);
          ok = false;
          break;
        }
        lhs_field_ptr->value = lhs_field.value;
        switch (cel_StructValueKey_Kind(&lhs_field_key)) {
          case cel_StructValueKeyKind_kName:
            lhs_field_ptr->key = cel_StructValueKey_GetName(&lhs_field_key);
            break;
          case cel_StructValueKeyKind_kDef:
            lhs_field_ptr->key = cel_StringView_FromString(
                upb_FieldDef_Name(cel_StructValueKey_GetDef(&lhs_field_key)));
            break;
          default:
            CEL_UNREACHABLE();
        }
      }
    }
    if (rhs_next) {
      cel_StructValueKey rhs_field_key;
      _cel_StructValueField rhs_field;
      rhs_next = cel_StructValueIterator_Next(rhs_iter, context, &rhs_field_key,
                                              &rhs_field.value, status);
      if (!rhs_next && !cel_Status_Ok(status)) {
        ok = false;
        break;
      }
      if (rhs_next) {
        _cel_StructValueField* rhs_field_ptr =
            _cel_Array_Push(&rhs_fields, context->alloc);
        if (CEL_UNLIKELY(rhs_field_ptr == cel_nullptr)) {
          cel_OutOfMemoryStatus(status);
          ok = false;
          break;
        }
        rhs_field_ptr->value = rhs_field.value;
        switch (cel_StructValueKey_Kind(&rhs_field_key)) {
          case cel_StructValueKeyKind_kName:
            rhs_field_ptr->key = cel_StructValueKey_GetName(&rhs_field_key);
            break;
          case cel_StructValueKeyKind_kDef:
            rhs_field_ptr->key = cel_StringView_FromString(
                upb_FieldDef_Name(cel_StructValueKey_GetDef(&rhs_field_key)));
            break;
          default:
            CEL_UNREACHABLE();
        }
      }
    }
    if (!lhs_next || !rhs_next) {
      break;
    }
  }
  if (ok) {
    if (lhs_next || rhs_next) {
      cel_Value_SetFalse(result);
    } else {
      const size_t lhs_fields_size = _cel_Array_Size(&lhs_fields);
      const size_t rhs_fields_size = _cel_Array_Size(&rhs_fields);
      if (lhs_fields_size != rhs_fields_size) {
        cel_Value_SetFalse(result);
      } else {
        _cel_StructValueField* lhs_fields_data =
            _cel_Array_MutableData(&lhs_fields);
        _cel_StructValueField* rhs_fields_data =
            _cel_Array_MutableData(&rhs_fields);
        _cel_Sort(lhs_fields_data, lhs_fields_size,
                  sizeof(_cel_StructValueField),
                  &_cel_StructValueField_Compare);
        _cel_Sort(rhs_fields_data, rhs_fields_size,
                  sizeof(_cel_StructValueField),
                  &_cel_StructValueField_Compare);
        for (size_t i = 0; i < lhs_fields_size; ++i) {
          if (!cel_StringView_Equals(lhs_fields_data[i].key,
                                     rhs_fields_data[i].key)) {
            cel_Value_SetFalse(result);
            goto done;
          }
          if (!cel_Value_Equals(&lhs_fields_data[i].value, context,
                                &rhs_fields_data[i].value, result, status)) {
            ok = false;
            goto done;
          }
          if (!cel_Value_IsTrue(result)) {
            goto done;
          }
        }
        cel_Value_SetTrue(result);
      }
    }
  }
done:
  _cel_Array_Destruct(&rhs_fields, context->alloc);
  _cel_Array_Destruct(&lhs_fields, context->alloc);
  cel_StructValueIterator_Delete(rhs_iter);
  cel_StructValueIterator_Delete(lhs_iter);
  return ok;
}

bool cel_StructValue_Equals(const cel_StructValue* cel_nonnull struct_value,
                            const cel_ValueContext* cel_nonnull context,
                            const cel_StructValue* cel_nonnull other,
                            cel_Value* cel_nonnull result,
                            cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(struct_value);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(other);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  if (cel_StringView_Equals(cel_StructValue_TypeName(struct_value),
                            cel_StructValue_TypeName(other))) {
    if (struct_value->vtable->Equals != cel_nullptr) {
      if ((*struct_value->vtable->Equals)(struct_value->vtable,
                                          struct_value->content, context, other,
                                          result, status)) {
        return true;
      }
      if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
        return false;
      }
    }
    if (other->vtable->Equals != cel_nullptr &&
        other->vtable->Equals != struct_value->vtable->Equals) {
      if ((*other->vtable->Equals)(other->vtable, other->content, context,
                                   struct_value, result, status)) {
        return true;
      }
      if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
        return false;
      }
    }
    return _cel_StructValue_EqualsSlow(struct_value, context, other, result,
                                       status);
  }

  cel_Value_SetFalse(result);
  return true;
}
