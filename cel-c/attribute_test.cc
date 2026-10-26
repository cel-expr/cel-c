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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/config.h"
#include "cel-c/string_view.h"

namespace {

using ::testing::_;

TEST(AttributeQualifier, Unspecified) {
  cel_AttributeQualifier attribute_qualifier =
      cel_UnspecifiedAttributeQualifier();
  EXPECT_EQ(cel_AttributeQualifier_Kind(&attribute_qualifier),
            cel_AttributeQualifierKind_kUnspecified);

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetBool(&attribute_qualifier)), _);
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetInt(&attribute_qualifier)), _);
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetUint(&attribute_qualifier)), _);
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetString(&attribute_qualifier)), _);
#endif
}

TEST(AttributeQualifier, Bool) {
  cel_AttributeQualifier attribute_qualifier;
  cel_AttributeQualifier_SetBool(&attribute_qualifier, true);
  EXPECT_EQ(cel_AttributeQualifier_Kind(&attribute_qualifier),
            cel_AttributeQualifierKind_kBool);
  EXPECT_TRUE(cel_AttributeQualifier_GetBool(&attribute_qualifier));

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetInt(&attribute_qualifier)), _);
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetUint(&attribute_qualifier)), _);
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetString(&attribute_qualifier)), _);
#endif
}

TEST(AttributeQualifier, Int) {
  cel_AttributeQualifier attribute_qualifier;
  cel_AttributeQualifier_SetInt(&attribute_qualifier, 1);
  EXPECT_EQ(cel_AttributeQualifier_Kind(&attribute_qualifier),
            cel_AttributeQualifierKind_kInt);
  EXPECT_EQ(cel_AttributeQualifier_GetInt(&attribute_qualifier), 1);

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetBool(&attribute_qualifier)), _);
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetUint(&attribute_qualifier)), _);
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetString(&attribute_qualifier)), _);
#endif
}

TEST(AttributeQualifier, Uint) {
  cel_AttributeQualifier attribute_qualifier;
  cel_AttributeQualifier_SetUint(&attribute_qualifier, 1);
  EXPECT_EQ(cel_AttributeQualifier_Kind(&attribute_qualifier),
            cel_AttributeQualifierKind_kUint);
  EXPECT_EQ(cel_AttributeQualifier_GetUint(&attribute_qualifier), 1);

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetBool(&attribute_qualifier)), _);
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetInt(&attribute_qualifier)), _);
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetString(&attribute_qualifier)), _);
#endif
}

TEST(AttributeQualifier, String) {
  cel_AttributeQualifier attribute_qualifier;
  cel_AttributeQualifier_SetString(&attribute_qualifier,
                                   cel_StringView_FromString("foo"));
  EXPECT_EQ(cel_AttributeQualifier_Kind(&attribute_qualifier),
            cel_AttributeQualifierKind_kString);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_AttributeQualifier_GetString(&attribute_qualifier),
      cel_StringView_FromString("foo")));

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetBool(&attribute_qualifier)), _);
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetInt(&attribute_qualifier)), _);
  EXPECT_DEATH_IF_SUPPORTED(
      CEL_USED(cel_AttributeQualifier_GetUint(&attribute_qualifier)), _);
#endif
}

}  // namespace
