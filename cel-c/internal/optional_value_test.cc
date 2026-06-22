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

#include "gtest/gtest.h"
#include "cel-c/duration.h"
#include "cel-c/internal/empty_list_value.h"
#include "cel-c/internal/value_testing.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"
#include "cel-c/value.h"

namespace {

using OptionalValueTest = ValueTest;

TEST_F(OptionalValueTest, Empty) {
  cel_OptionalValue optional_value;
  cel_OptionalValue_Empty(&optional_value);

  cel_OptionalValue other_optional_value;
  cel_OptionalValue_Empty(&other_optional_value);

  cel_Value result;

  ASSERT_TRUE(
      cel_OptionalValue_HasValue(&optional_value, ctx(), &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_OptionalValue_Value(&optional_value, ctx(), &result, status()));
  EXPECT_TRUE(cel_Value_IsError(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &other_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&other_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
}

TEST_F(OptionalValueTest, Null) {
  cel_Value value;
  cel_Value_SetNull(&value);

  cel_OptionalValue optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&optional_value, &value, arena()));

  cel_Value result;

  ASSERT_TRUE(
      cel_OptionalValue_HasValue(&optional_value, ctx(), &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_OptionalValue_Value(&optional_value, ctx(), &result, status()));
  EXPECT_TRUE(cel_Value_IsNull(&result));

  cel_OptionalValue other_optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&other_optional_value, &value, arena()));

  cel_OptionalValue empty_optional_value;
  cel_OptionalValue_Empty(&empty_optional_value);

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &other_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&other_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &empty_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&empty_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(OptionalValueTest, Bool) {
  cel_Value value;
  cel_Value_SetTrue(&value);

  cel_OptionalValue optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&optional_value, &value, arena()));

  cel_Value result;

  ASSERT_TRUE(
      cel_OptionalValue_HasValue(&optional_value, ctx(), &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_OptionalValue_Value(&optional_value, ctx(), &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  cel_OptionalValue other_optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&other_optional_value, &value, arena()));

  cel_OptionalValue empty_optional_value;
  cel_OptionalValue_Empty(&empty_optional_value);

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &other_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&other_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &empty_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&empty_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(OptionalValueTest, Int) {
  cel_Value value;
  cel_Value_SetInt(&value, 1);

  cel_OptionalValue optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&optional_value, &value, arena()));

  cel_Value result;

  ASSERT_TRUE(
      cel_OptionalValue_HasValue(&optional_value, ctx(), &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_OptionalValue_Value(&optional_value, ctx(), &result, status()));
  ASSERT_TRUE(cel_Value_IsInt(&result));
  EXPECT_EQ(cel_Value_GetInt(&result), 1);

  cel_Value_SetUint(&value, 1);

  cel_OptionalValue other_optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&other_optional_value, &value, arena()));

  cel_OptionalValue empty_optional_value;
  cel_OptionalValue_Empty(&empty_optional_value);

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &other_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&other_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &empty_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&empty_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(OptionalValueTest, Uint) {
  cel_Value value;
  cel_Value_SetUint(&value, 1);

  cel_OptionalValue optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&optional_value, &value, arena()));

  cel_Value result;

  ASSERT_TRUE(
      cel_OptionalValue_HasValue(&optional_value, ctx(), &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_OptionalValue_Value(&optional_value, ctx(), &result, status()));
  ASSERT_TRUE(cel_Value_IsUint(&result));
  EXPECT_EQ(cel_Value_GetUint(&result), 1);

  cel_Value_SetInt(&value, 1);

  cel_OptionalValue other_optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&other_optional_value, &value, arena()));

  cel_OptionalValue empty_optional_value;
  cel_OptionalValue_Empty(&empty_optional_value);

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &other_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&other_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &empty_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&empty_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(OptionalValueTest, Double) {
  cel_Value value;
  cel_Value_SetDouble(&value, 1);

  cel_OptionalValue optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&optional_value, &value, arena()));

  cel_Value result;

  ASSERT_TRUE(
      cel_OptionalValue_HasValue(&optional_value, ctx(), &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_OptionalValue_Value(&optional_value, ctx(), &result, status()));
  ASSERT_TRUE(cel_Value_IsDouble(&result));
  EXPECT_EQ(cel_Value_GetDouble(&result), 1);

  cel_Value_SetInt(&value, 1);

  cel_OptionalValue other_optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&other_optional_value, &value, arena()));

  cel_OptionalValue empty_optional_value;
  cel_OptionalValue_Empty(&empty_optional_value);

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &other_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&other_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &empty_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&empty_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(OptionalValueTest, Bytes) {
  cel_Value value;
  cel_Value_SetBytes(&value, cel_StringView_FromString("foo"));

  cel_OptionalValue optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&optional_value, &value, arena()));

  cel_Value result;

  ASSERT_TRUE(
      cel_OptionalValue_HasValue(&optional_value, ctx(), &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_OptionalValue_Value(&optional_value, ctx(), &result, status()));
  ASSERT_TRUE(cel_Value_IsBytes(&result));
  EXPECT_EQ(cel_Value_GetBytes(&result), cel_StringView_FromString("foo"));

  cel_OptionalValue other_optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&other_optional_value, &value, arena()));

  cel_OptionalValue empty_optional_value;
  cel_OptionalValue_Empty(&empty_optional_value);

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &other_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&other_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &empty_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&empty_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(OptionalValueTest, String) {
  cel_Value value;
  cel_Value_SetString(&value, cel_StringView_FromString("foo"));

  cel_OptionalValue optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&optional_value, &value, arena()));

  cel_Value result;

  ASSERT_TRUE(
      cel_OptionalValue_HasValue(&optional_value, ctx(), &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_OptionalValue_Value(&optional_value, ctx(), &result, status()));
  ASSERT_TRUE(cel_Value_IsString(&result));
  EXPECT_EQ(cel_Value_GetString(&result), cel_StringView_FromString("foo"));

  cel_OptionalValue other_optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&other_optional_value, &value, arena()));

  cel_OptionalValue empty_optional_value;
  cel_OptionalValue_Empty(&empty_optional_value);

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &other_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&other_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &empty_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&empty_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(OptionalValueTest, Duration) {
  cel_Value value;
  cel_Value_SetDuration(&value, cel_Duration_kMax);

  cel_OptionalValue optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&optional_value, &value, arena()));

  cel_Value result;

  ASSERT_TRUE(
      cel_OptionalValue_HasValue(&optional_value, ctx(), &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_OptionalValue_Value(&optional_value, ctx(), &result, status()));
  ASSERT_TRUE(cel_Value_IsDuration(&result));
  EXPECT_EQ(cel_Value_GetDuration(&result), cel_Duration_kMax);

  cel_OptionalValue other_optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&other_optional_value, &value, arena()));

  cel_OptionalValue empty_optional_value;
  cel_OptionalValue_Empty(&empty_optional_value);

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &other_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&other_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &empty_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&empty_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(OptionalValueTest, Timestamp) {
  cel_Value value;
  cel_Value_SetTimestamp(&value, cel_Timestamp_kMax);

  cel_OptionalValue optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&optional_value, &value, arena()));

  cel_Value result;

  ASSERT_TRUE(
      cel_OptionalValue_HasValue(&optional_value, ctx(), &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_OptionalValue_Value(&optional_value, ctx(), &result, status()));
  ASSERT_TRUE(cel_Value_IsTimestamp(&result));
  EXPECT_EQ(cel_Value_GetTimestamp(&result), cel_Timestamp_kMax);

  cel_OptionalValue other_optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&other_optional_value, &value, arena()));

  cel_OptionalValue empty_optional_value;
  cel_OptionalValue_Empty(&empty_optional_value);

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &other_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&other_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &empty_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&empty_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(OptionalValueTest, Other) {
  cel_Value value;
  _cel_EmptyListValue_Set(cel_Value_SetList(&value));

  cel_OptionalValue optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&optional_value, &value, arena()));

  cel_Value result;

  ASSERT_TRUE(
      cel_OptionalValue_HasValue(&optional_value, ctx(), &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_OptionalValue_Value(&optional_value, ctx(), &result, status()));
  ASSERT_TRUE(cel_Value_IsList(&result));
  EXPECT_EQ(cel_Value_GetList(&result)->vtable, &_cel_EmptyListValueVTable);

  cel_OptionalValue other_optional_value;
  ASSERT_TRUE(cel_OptionalValue_Of(&other_optional_value, &value, arena()));

  cel_OptionalValue empty_optional_value;
  cel_OptionalValue_Empty(&empty_optional_value);

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &other_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&other_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(
      &optional_value, ctx(), &empty_optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_OptionalValue_Equals(&empty_optional_value, ctx(),
                                       &optional_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

}  // namespace
