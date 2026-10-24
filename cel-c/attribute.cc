// Copyright 2026 Google LLC
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

#include "cel-c/attribute.h"

#include <stdbool.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/string_view.h"

extern "C" bool cel_AttributeQualifier_Equals(
    CEL_NONNULL(const cel_AttributeQualifier*) lhs,
    CEL_NONNULL(const cel_AttributeQualifier*) rhs) {
  CEL_ASSERT_NOT_NULL(lhs);
  CEL_ASSERT_NOT_NULL(rhs);

  if (lhs == rhs) {
    return true;
  }

  const cel_AttributeQualifierKind kind = cel_AttributeQualifier_Kind(lhs);
  if (kind != cel_AttributeQualifier_Kind(rhs)) {
    return false;
  }

  switch (kind) {
    case cel_AttributeQualifierKind_kUnspecified:
      return true;
    case cel_AttributeQualifierKind_kBool:
      return cel_AttributeQualifier_GetBool(lhs) ==
             cel_AttributeQualifier_GetBool(rhs);
    case cel_AttributeQualifierKind_kInt:
      return cel_AttributeQualifier_GetInt(lhs) ==
             cel_AttributeQualifier_GetInt(rhs);
    case cel_AttributeQualifierKind_kUint:
      return cel_AttributeQualifier_GetUint(lhs) ==
             cel_AttributeQualifier_GetUint(rhs);
    case cel_AttributeQualifierKind_kString:
      return cel_StringView_Equals(cel_AttributeQualifier_GetString(lhs),
                                   cel_AttributeQualifier_GetString(rhs));
    default:
      return false;
  }
}
