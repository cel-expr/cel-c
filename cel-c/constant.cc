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

#include "cel-c/constant.h"

#include <stdbool.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"

extern "C" bool cel_Constant_Equals(CEL_NONNULL(const cel_Constant*) lhs,
                                    CEL_NONNULL(const cel_Constant*) rhs) {
  CEL_ASSERT_NOT_NULL(lhs);
  CEL_ASSERT_NOT_NULL(rhs);

  if (lhs == rhs) {
    return true;
  }

  const cel_ConstantKind kind = cel_Constant_Kind(lhs);
  if (kind != cel_Constant_Kind(rhs)) {
    return false;
  }

  switch (kind) {
    case cel_ConstantKind_kUnspecified:
      return true;
    case cel_ConstantKind_kNull:
      return true;
    case cel_ConstantKind_kBool:
      return cel_Constant_GetBool(lhs) == cel_Constant_GetBool(rhs);
    case cel_ConstantKind_kInt:
      return cel_Constant_GetInt(lhs) == cel_Constant_GetInt(rhs);
    case cel_ConstantKind_kUint:
      return cel_Constant_GetUint(lhs) == cel_Constant_GetUint(rhs);
    case cel_ConstantKind_kDouble:
      return cel_Constant_GetDouble(lhs) == cel_Constant_GetDouble(rhs);
    case cel_ConstantKind_kBytes:
      return cel_StringView_Equals(cel_Constant_GetBytes(lhs),
                                   cel_Constant_GetBytes(rhs));
    case cel_ConstantKind_kString:
      return cel_StringView_Equals(cel_Constant_GetString(lhs),
                                   cel_Constant_GetString(rhs));
    case cel_ConstantKind_kDuration:
      return cel_Duration_Equals(cel_Constant_GetDuration(lhs),
                                 cel_Constant_GetDuration(rhs));
    case cel_ConstantKind_kTimestamp:
      return cel_Timestamp_Equals(cel_Constant_GetTimestamp(lhs),
                                  cel_Constant_GetTimestamp(rhs));
    default:
      return false;
  }
}
