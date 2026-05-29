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

#ifndef THIRD_PARTY_CEL_C_SRC_PARSED_REPEATED_FIELD_VALUE_H_
#define THIRD_PARTY_CEL_C_SRC_PARSED_REPEATED_FIELD_VALUE_H_

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/src/empty_list_value.h"
#include "cel-c/value.h"
#include "upb/message/array.h"
#include "upb/reflection/def.h"

CEL_BEGIN_DECLS

extern const cel_ListValueVTable _cel_ParsedRepeatedFieldValueVTable;

static CEL_INLINE void _cel_ParsedRepeatedFieldValue_Set(
    cel_ListValue* cel_nonnull list_value,
    const upb_Array* cel_nullable field_val,
    const upb_FieldDef* cel_nonnull field_def) {
  CEL_ASSERT_NOT_NULL(list_value);
  CEL_ASSERT_NOT_NULL(field_def);
  CEL_ASSERT(upb_FieldDef_IsRepeated(field_def));
  CEL_ASSERT_NOT(upb_FieldDef_IsMap(field_def));

  if (field_val == cel_nullptr || upb_Array_Size(field_val) == 0) {
    _cel_EmptyListValue_Set(list_value);
  } else {
    list_value->vtable = &_cel_ParsedRepeatedFieldValueVTable;
    list_value->content.ptr[0] = field_val;
    list_value->content.ptr[1] = field_def;
  }
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_SRC_PARSED_REPEATED_FIELD_VALUE_H_
