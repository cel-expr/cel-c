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

#include "cel-c/src/map_value.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/src/array.h"
#include "cel-c/src/sort.h"
#include "cel-c/status.h"
#include "cel-c/value.h"
#include "cel-c/value_kind.h"

static void _cel_MapValueKey_SetValue(cel_MapValueKey* cel_nonnull
                                          map_value_key,
                                      const cel_Value* cel_nonnull value) {
  switch (cel_Value_Kind(value)) {
    case cel_ValueKind_kBool:
      cel_MapValueKey_SetBool(map_value_key, cel_Value_GetBool(value));
      return;
    case cel_ValueKind_kInt:
      cel_MapValueKey_SetInt(map_value_key, cel_Value_GetInt(value));
      return;
    case cel_ValueKind_kUint:
      cel_MapValueKey_SetUint(map_value_key, cel_Value_GetUint(value));
      return;
    case cel_ValueKind_kString:
      cel_MapValueKey_SetString(map_value_key, cel_Value_GetString(value));
      return;
    default:
      CEL_UNREACHABLE();
  }
}

typedef struct {
  cel_MapValueKey key;
  cel_Value value;
} _cel_MapValueEntry;

static int _cel_MapValueEntry_Compare(const void* cel_nullability_unknown lhs,
                                      const void* cel_nullability_unknown rhs) {
  return _cel_MapValueKey_Compare(&((const _cel_MapValueEntry*)lhs)->key,
                                  &((const _cel_MapValueEntry*)rhs)->key);
}

static bool _cel_MapValue_EqualsFast(
    const cel_MapValue* cel_nonnull lhs, size_t lhs_size,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValue* cel_nonnull rhs, size_t rhs_size,
    cel_Value* cel_nonnull result, cel_Status* cel_nonnull status) {
  if (lhs_size != rhs_size) {
    cel_Value_SetFalse(result);
    return true;
  }
  if (lhs_size == 0) {
    cel_Value_SetTrue(result);
    return true;
  }
  // Both have size. We can iterate through one and check if the other has the
  // same key and value.
  cel_MapValueIterator* lhs_iter =
      cel_MapValue_NewIterator(lhs, context, status);
  if (CEL_UNLIKELY(lhs_iter == cel_nullptr)) {
    CEL_ASSERT(!cel_Status_Ok(status));
    return false;
  }
  bool ok = true;
  while (true) {
    cel_Value lhs_key;
    cel_Value lhs_value;
    cel_MapValueKey rhs_key;
    cel_Value rhs_value;
    if (!cel_MapValueIterator_Next2(lhs_iter, context, &lhs_key, &lhs_value,
                                    status)) {
      if (!cel_Status_Ok(status)) {
        ok = false;
        break;
      }
      cel_Value_SetTrue(result);
      break;
    }
    if (cel_Value_IsError(&lhs_key) || cel_Value_IsError(&lhs_value)) {
      cel_Value_SetFalse(result);
      ok = false;
      break;
    }
    _cel_MapValueKey_SetValue(&rhs_key, &lhs_key);
    if (cel_MapValue_Find(rhs, context, &rhs_key, &rhs_value, status)) {
      if (!cel_Value_Equals(&lhs_value, context, &rhs_value, result, status)) {
        ok = cel_Status_Ok(status);
        if (ok) {
          cel_Value_SetFalse(result);
        }
        break;
      }
    } else {
      ok = cel_Status_Ok(status);
      if (ok) {
        cel_Value_SetFalse(result);
      }
      break;
    }
  }
  cel_MapValueIterator_Delete(lhs_iter);
  return ok;
}

static bool _cel_MapValue_EqualsSlowWithSize(
    const cel_MapValue* cel_nonnull lhs, size_t lhs_size,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValue* cel_nonnull rhs, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  cel_MapValueIterator* rhs_iter =
      cel_MapValue_NewIterator(rhs, context, status);
  if (CEL_UNLIKELY(rhs_iter == cel_nullptr)) {
    CEL_ASSERT(!cel_Status_Ok(status));
    return false;
  }
  size_t rhs_size;
  if (cel_MapValueIterator_Remaining(rhs_iter, &rhs_size)) {
    if (rhs_size != lhs_size) {
      cel_Value_SetFalse(result);
      cel_MapValueIterator_Delete(rhs_iter);
      return true;
    }
    if (rhs_size == 0) {
      cel_Value_SetTrue(result);
      cel_MapValueIterator_Delete(rhs_iter);
      return true;
    }
  }
  rhs_size = 0;
  bool ok = true;
  while (true) {
    cel_Value rhs_key;
    cel_Value rhs_value;
    cel_MapValueKey lhs_key;
    cel_Value lhs_value;
    if (!cel_MapValueIterator_Next2(rhs_iter, context, &rhs_key, &rhs_value,
                                    status)) {
      if (!cel_Status_Ok(status)) {
        ok = false;
        break;
      }
      cel_Value_SetBool(result, rhs_size == lhs_size);
      break;
    }
    ++rhs_size;
    if (cel_Value_IsError(&rhs_key) || cel_Value_IsError(&rhs_value)) {
      cel_Value_SetFalse(result);
      ok = false;
      break;
    }
    _cel_MapValueKey_SetValue(&lhs_key, &rhs_key);
    if (cel_MapValue_Find(lhs, context, &lhs_key, &lhs_value, status)) {
      if (!cel_Value_Equals(&rhs_value, context, &lhs_value, result, status)) {
        ok = cel_Status_Ok(status);
        if (ok) {
          cel_Value_SetFalse(result);
        }
        break;
      }
    } else {
      ok = cel_Status_Ok(status);
      if (ok) {
        cel_Value_SetFalse(result);
      }
      break;
    }
  }
  cel_MapValueIterator_Delete(rhs_iter);
  return ok;
}

static bool _cel_MapValue_EqualsSlow(
    const cel_MapValue* cel_nonnull lhs,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValue* cel_nonnull rhs, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  cel_MapValueIterator* lhs_iter =
      cel_MapValue_NewIterator(lhs, context, status);
  if (CEL_UNLIKELY(lhs_iter == cel_nullptr)) {
    CEL_ASSERT(!cel_Status_Ok(status));
    return false;
  }
  cel_MapValueIterator* rhs_iter =
      cel_MapValue_NewIterator(rhs, context, status);
  if (CEL_UNLIKELY(rhs_iter == cel_nullptr)) {
    CEL_ASSERT(!cel_Status_Ok(status));
    cel_MapValueIterator_Delete(lhs_iter);
    return false;
  }
  size_t lhs_size;
  size_t rhs_size;
  bool lhs_has_size;
  bool rhs_has_size;
  lhs_has_size = cel_MapValueIterator_Remaining(lhs_iter, &lhs_size);
  rhs_has_size = cel_MapValueIterator_Remaining(rhs_iter, &rhs_size);
  if (lhs_has_size && rhs_has_size) {
    if (lhs_size != rhs_size) {
      cel_Value_SetFalse(result);
      cel_MapValueIterator_Delete(rhs_iter);
      cel_MapValueIterator_Delete(lhs_iter);
      return true;
    }
    if (lhs_size == 0) {
      cel_Value_SetTrue(result);
      cel_MapValueIterator_Delete(rhs_iter);
      cel_MapValueIterator_Delete(lhs_iter);
      return true;
    }
  }
  _cel_Array(_cel_MapValueEntry) lhs_entries;
  _cel_Array(_cel_MapValueEntry) rhs_entries;
  _cel_Array_Construct(&lhs_entries);
  if (lhs_has_size) {
    _cel_Array_Reserve(&lhs_entries, context->alloc, lhs_size);
  }
  _cel_Array_Construct(&rhs_entries);
  if (rhs_has_size) {
    _cel_Array_Reserve(&rhs_entries, context->alloc, rhs_size);
  }
  bool ok = true;
  bool lhs_next = true;
  bool rhs_next = true;
  while (true) {
    if (lhs_next) {
      _cel_MapValueEntry lhs_entry;
      lhs_next = cel_MapValueIterator_Next(lhs_iter, context, &lhs_entry.key,
                                           &lhs_entry.value, status);
      if (!lhs_next && !cel_Status_Ok(status)) {
        ok = false;
        break;
      }
      if (lhs_next) {
        _cel_MapValueEntry* lhs_entry_ptr =
            _cel_Array_Push(&lhs_entries, context->alloc);
        if (CEL_UNLIKELY(lhs_entry_ptr == cel_nullptr)) {
          cel_OutOfMemoryStatus(status);
          ok = false;
          break;
        }
        *lhs_entry_ptr = lhs_entry;
      }
    }
    if (rhs_next) {
      _cel_MapValueEntry rhs_entry;
      rhs_next = cel_MapValueIterator_Next(rhs_iter, context, &rhs_entry.key,
                                           &rhs_entry.value, status);
      if (!rhs_next && !cel_Status_Ok(status)) {
        ok = false;
        break;
      }
      if (rhs_next) {
        _cel_MapValueEntry* rhs_entry_ptr =
            _cel_Array_Push(&rhs_entries, context->alloc);
        if (CEL_UNLIKELY(rhs_entry_ptr == cel_nullptr)) {
          cel_OutOfMemoryStatus(status);
          ok = false;
          break;
        }
        *rhs_entry_ptr = rhs_entry;
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
      const size_t lhs_entries_size = _cel_Array_Size(&lhs_entries);
      const size_t rhs_entries_size = _cel_Array_Size(&rhs_entries);
      if (lhs_entries_size != rhs_entries_size) {
        cel_Value_SetFalse(result);
      } else {
        _cel_MapValueEntry* lhs_entries_data =
            _cel_Array_MutableData(&lhs_entries);
        _cel_MapValueEntry* rhs_entries_data =
            _cel_Array_MutableData(&rhs_entries);
        _cel_Sort(lhs_entries_data, lhs_entries_size,
                  sizeof(_cel_MapValueEntry), &_cel_MapValueEntry_Compare);
        _cel_Sort(rhs_entries_data, rhs_entries_size,
                  sizeof(_cel_MapValueEntry), &_cel_MapValueEntry_Compare);
        for (size_t i = 0; i < lhs_entries_size; ++i) {
          if (!_cel_MapValueKey_Equals(&lhs_entries_data[i].key,
                                       &rhs_entries_data[i].key)) {
            cel_Value_SetFalse(result);
            goto done;
          }
          if (!cel_Value_Equals(&lhs_entries_data[i].value, context,
                                &rhs_entries_data[i].value, result, status)) {
            ok = false;
            goto done;
          }
          if (!cel_Value_IsTrue(result)) {
            cel_Value_SetFalse(result);
            goto done;
          }
        }
        cel_Value_SetTrue(result);
      }
    }
  }
done:
  _cel_Array_Destruct(&rhs_entries, context->alloc);
  _cel_Array_Destruct(&lhs_entries, context->alloc);
  cel_MapValueIterator_Delete(rhs_iter);
  cel_MapValueIterator_Delete(lhs_iter);
  return ok;
}

extern "C" bool cel_MapValue_Equals(const cel_MapValue* cel_nonnull map_value,
                                    const cel_ValueContext* cel_nonnull context,
                                    const cel_MapValue* cel_nonnull other,
                                    cel_Value* cel_nonnull result,
                                    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(map_value);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(other);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  if (map_value->vtable->Equals != cel_nullptr) {
    if ((*map_value->vtable->Equals)(map_value->vtable, map_value->content,
                                     context, other, result, status)) {
      return true;
    }
    if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
      return false;
    }
  }
  if (other->vtable->Equals != cel_nullptr &&
      other->vtable->Equals != map_value->vtable->Equals) {
    if ((*other->vtable->Equals)(other->vtable, other->content, context,
                                 map_value, result, status)) {
      return true;
    }
    if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
      return false;
    }
  }
  size_t lhs_size;
  size_t rhs_size;
  bool lhs_has_size;
  bool rhs_has_size;
  if (map_value->vtable->FastSize != cel_nullptr) {
    lhs_has_size = (*map_value->vtable->FastSize)(
        map_value->vtable, map_value->content, &lhs_size);
  } else {
    lhs_has_size = false;
  }
  if (other->vtable->FastSize != cel_nullptr) {
    rhs_has_size =
        (*other->vtable->FastSize)(other->vtable, other->content, &rhs_size);
  } else {
    rhs_has_size = false;
  }
  if (lhs_has_size && rhs_has_size) {
    return _cel_MapValue_EqualsFast(map_value, lhs_size, context, other,
                                    rhs_size, result, status);
  }
  // Unfortunately one or both of the map values does not know its size in
  // constant time. We have no choice but to collect the key value pairs into
  // arrays, sort, and then compare.
  if (lhs_has_size) {
    return _cel_MapValue_EqualsSlowWithSize(map_value, lhs_size, context, other,
                                            result, status);
  }
  if (rhs_has_size) {
    return _cel_MapValue_EqualsSlowWithSize(other, rhs_size, context, map_value,
                                            result, status);
  }
  return _cel_MapValue_EqualsSlow(map_value, context, other, result, status);
}
