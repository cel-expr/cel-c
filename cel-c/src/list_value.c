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
#include "cel-c/value.h"

bool cel_ListValue_Equals(const cel_ListValue* cel_nonnull list_value,
                          const cel_ValueContext* cel_nonnull context,
                          const cel_ListValue* cel_nonnull other,
                          cel_Value* cel_nonnull result,
                          cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(list_value);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(other);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  if (list_value->vtable->Equals != cel_nullptr) {
    if ((*list_value->vtable->Equals)(list_value->vtable, list_value->content,
                                      context, other, result, status)) {
      return true;
    }
    if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
      return false;
    }
  }
  if (other->vtable->Equals != cel_nullptr &&
      other->vtable->Equals != list_value->vtable->Equals) {
    if ((*other->vtable->Equals)(other->vtable, other->content, context,
                                 list_value, result, status)) {
      return true;
    }
    if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
      return false;
    }
  }
  if (list_value->vtable->FastSize != cel_nullptr &&
      other->vtable->FastSize != cel_nullptr) {
    size_t lhs_size;
    size_t rhs_size;
    if ((*list_value->vtable->FastSize)(list_value->vtable, list_value->content,
                                        &lhs_size) &&
        (*other->vtable->FastSize)(other->vtable, other->content, &rhs_size)) {
      if (lhs_size != rhs_size) {
        cel_Value_SetFalse(result);
        return true;
      }
      if (lhs_size == 0) {
        cel_Value_SetTrue(result);
        return true;
      }
    }
  }
  cel_ListValueIterator* lhs_iter =
      cel_ListValue_NewIterator(list_value, context, status);
  if (CEL_UNLIKELY(lhs_iter == cel_nullptr)) {
    CEL_ASSERT(!cel_Status_Ok(status));
    return false;
  }
  CEL_ASSERT(cel_Status_Ok(status));
  cel_ListValueIterator* rhs_iter =
      cel_ListValue_NewIterator(other, context, status);
  if (CEL_UNLIKELY(lhs_iter == cel_nullptr)) {
    cel_ListValueIterator_Delete(lhs_iter);
    CEL_ASSERT(!cel_Status_Ok(status));
    return false;
  }
  size_t lhs_size;
  size_t rhs_size;
  bool lhs_has_size;
  bool rhs_has_size;
  lhs_has_size = cel_ListValueIterator_Remaining(lhs_iter, &lhs_size);
  rhs_has_size = cel_ListValueIterator_Remaining(rhs_iter, &rhs_size);
  if (lhs_has_size && rhs_has_size) {
    if (lhs_size != rhs_size) {
      cel_ListValueIterator_Delete(rhs_iter);
      cel_ListValueIterator_Delete(lhs_iter);
      cel_Value_SetFalse(result);
      return true;
    }
    if (lhs_size == 0) {
      cel_ListValueIterator_Delete(rhs_iter);
      cel_ListValueIterator_Delete(lhs_iter);
      cel_Value_SetTrue(result);
      return true;
    }
  }
  CEL_ASSERT(cel_Status_Ok(status));
  bool ok;
  while (true) {
    cel_Value lhs_ele;
    cel_Value rhs_ele;
    bool lhs_has_ele;
    bool rhs_has_ele;
    if (!(lhs_has_ele = cel_ListValueIterator_Next1(lhs_iter, context, &lhs_ele,
                                                    status))) {
      if (!cel_Status_Ok(status)) {
        ok = false;
        break;
      }
    }
    CEL_ASSERT(cel_Status_Ok(status));
    if (!(rhs_has_ele = cel_ListValueIterator_Next1(rhs_iter, context, &rhs_ele,
                                                    status))) {
      if (!cel_Status_Ok(status)) {
        ok = false;
        break;
      }
    }
    CEL_ASSERT(cel_Status_Ok(status));
    if (lhs_has_ele != rhs_has_ele) {
      cel_Value_SetFalse(result);
      ok = true;
      break;
    }
    if (!lhs_has_ele) {
      cel_Value_SetTrue(result);
      ok = true;
      break;
    }
    if (!cel_Value_Equals(&lhs_ele, context, &rhs_ele, result, status)) {
      if (!cel_Status_Ok(status)) {
        ok = false;
        break;
      }
    }
    CEL_ASSERT(cel_Status_Ok(status));
  }
  cel_ListValueIterator_Delete(rhs_iter);
  cel_ListValueIterator_Delete(lhs_iter);
  return ok;
}
