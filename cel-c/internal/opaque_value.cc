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

#include "cel-c/assert.h"
#include "cel-c/internal/config.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/value.h"

extern "C" bool cel_OpaqueValue_Equals(
    const cel_OpaqueValue* cel_nonnull opaque_value,
    const cel_ValueContext* cel_nonnull context,
    const cel_OpaqueValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(opaque_value);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(other);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  if (cel_StringView_Equals(cel_OpaqueValue_TypeName(opaque_value),
                            cel_OpaqueValue_TypeName(other))) {
    if (opaque_value->vtable->Equals != cel_nullptr) {
      if ((*opaque_value->vtable->Equals)(opaque_value->vtable,
                                          opaque_value->content, context, other,
                                          result, status)) {
        return true;
      }
      if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
        return false;
      }
    }
    if (other->vtable->Equals != cel_nullptr &&
        other->vtable->Equals != opaque_value->vtable->Equals) {
      if ((*other->vtable->Equals)(other->vtable, other->content, context,
                                   opaque_value, result, status)) {
        return true;
      }
      if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
        return false;
      }
    }
  }

  cel_Value_SetFalse(result);
  return true;
}
