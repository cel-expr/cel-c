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

// Compiled as C to exercise the C99 versions of the `cel_*Constant()` and
// `cel_*AttributeQualifier()` macros, which are not visible to C++ code, and
// checks that they agree with the corresponding `*_Set*` functions.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "cel-c/duration.h"
#include "cel-c/internal/attribute.h"
#include "cel-c/internal/constant.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"

static int failures = 0;

#define EXPECT(cond)                                                    \
  do {                                                                  \
    if (!(cond)) {                                                      \
      fprintf(stderr, "%s:%d: EXPECT(%s) failed\n", __FILE__, __LINE__, \
              #cond);                                                   \
      ++failures;                                                       \
    }                                                                   \
  } while (0)

static void TestConstantMacros(void) {
  const cel_StringView str = cel_StringView_FromString("foo");
  const cel_Duration duration = cel_Duration_FromUnix(1, 2);
  const cel_Timestamp timestamp = cel_Timestamp_FromUnix(3, 4);
  cel_Constant set;

  cel_Constant c = cel_UnspecifiedConstant();
  EXPECT(cel_Constant_Kind(&c) == cel_ConstantKind_kUnspecified);

  c = cel_NullConstant();
  cel_Constant_SetNull(&set);
  EXPECT(cel_Constant_Kind(&c) == cel_ConstantKind_kNull);
  EXPECT(cel_Constant_Equals(&c, &set));

  c = cel_BoolConstant(true);
  cel_Constant_SetBool(&set, true);
  EXPECT(cel_Constant_GetBool(&c));
  EXPECT(cel_Constant_Equals(&c, &set));

  c = cel_IntConstant(-42);
  cel_Constant_SetInt(&set, -42);
  EXPECT(cel_Constant_GetInt(&c) == -42);
  EXPECT(cel_Constant_Equals(&c, &set));

  c = cel_UintConstant(42u);
  cel_Constant_SetUint(&set, 42u);
  EXPECT(cel_Constant_GetUint(&c) == 42u);
  EXPECT(cel_Constant_Equals(&c, &set));

  c = cel_DoubleConstant(1.5);
  cel_Constant_SetDouble(&set, 1.5);
  EXPECT(cel_Constant_GetDouble(&c) == 1.5);
  EXPECT(cel_Constant_Equals(&c, &set));

  c = cel_BytesConstant(str);
  cel_Constant_SetBytes(&set, str);
  EXPECT(cel_StringView_Equals(cel_Constant_GetBytes(&c), str));
  EXPECT(cel_Constant_Equals(&c, &set));

  c = cel_StringConstant(str);
  cel_Constant_SetString(&set, str);
  EXPECT(cel_StringView_Equals(cel_Constant_GetString(&c), str));
  EXPECT(cel_Constant_Equals(&c, &set));

  c = cel_DurationConstant(duration);
  cel_Constant_SetDuration(&set, duration);
  EXPECT(cel_Duration_Equals(cel_Constant_GetDuration(&c), duration));
  EXPECT(cel_Constant_Equals(&c, &set));

  c = cel_TimestampConstant(timestamp);
  cel_Constant_SetTimestamp(&set, timestamp);
  EXPECT(cel_Timestamp_Equals(cel_Constant_GetTimestamp(&c), timestamp));
  EXPECT(cel_Constant_Equals(&c, &set));
}

static void TestAttributeQualifierMacros(void) {
  const cel_StringView str = cel_StringView_FromString("bar");
  cel_AttributeQualifier set;

  cel_AttributeQualifier q = cel_UnspecifiedAttributeQualifier();
  EXPECT(cel_AttributeQualifier_Kind(&q) ==
         cel_AttributeQualifierKind_kUnspecified);

  q = cel_BoolAttributeQualifier(true);
  cel_AttributeQualifier_SetBool(&set, true);
  EXPECT(cel_AttributeQualifier_Kind(&q) == cel_AttributeQualifier_Kind(&set));
  EXPECT(cel_AttributeQualifier_GetBool(&q) ==
         cel_AttributeQualifier_GetBool(&set));

  q = cel_IntAttributeQualifier(-7);
  cel_AttributeQualifier_SetInt(&set, -7);
  EXPECT(cel_AttributeQualifier_Kind(&q) == cel_AttributeQualifier_Kind(&set));
  EXPECT(cel_AttributeQualifier_GetInt(&q) == -7);
  EXPECT(cel_AttributeQualifier_GetInt(&set) == -7);

  q = cel_UintAttributeQualifier(7u);
  cel_AttributeQualifier_SetUint(&set, 7u);
  EXPECT(cel_AttributeQualifier_Kind(&q) == cel_AttributeQualifier_Kind(&set));
  EXPECT(cel_AttributeQualifier_GetUint(&q) == 7u);
  EXPECT(cel_AttributeQualifier_GetUint(&set) == 7u);

  q = cel_StringAttributeQualifier(str);
  cel_AttributeQualifier_SetString(&set, str);
  EXPECT(cel_AttributeQualifier_Kind(&q) == cel_AttributeQualifier_Kind(&set));
  EXPECT(cel_StringView_Equals(cel_AttributeQualifier_GetString(&q), str));
  EXPECT(cel_StringView_Equals(cel_AttributeQualifier_GetString(&set), str));
}

int main(void) {
  TestConstantMacros();
  TestAttributeQualifierMacros();
  if (failures != 0) {
    fprintf(stderr, "%d failure(s)\n", failures);
    return 1;
  }
  return 0;
}
