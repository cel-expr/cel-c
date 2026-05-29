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

#include "cel-c/value.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/string_view.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"
#include "cel-c/error.h"
#include "cel-c/src/empty_list_value.h"
#include "cel-c/src/empty_map_value.h"
#include "cel-c/src/mutable_list_value.h"
#include "cel-c/src/mutable_map_value.h"
#include "cel-c/src/parsed_message_value.h"
#include "cel-c/src/value_testing.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"
#include "cel-c/value_kind.h"

namespace {

using ::testing::NotNull;
using ::testing::Pointee;

TEST_F(ValueTest, Null) {
  cel_Value value;
  cel_Value_SetNull(&value);
  EXPECT_EQ(cel_Value_Kind(&value), cel_ValueKind_kNull);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&value),
                                    cel_StringView_FromString("null_type")));
  EXPECT_TRUE(cel_Value_IsNull(&value));

  cel_Value result;
  ASSERT_TRUE(
      cel_Value_Equals(&value, ctx(), &cel_NullValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&value, ctx(), &cel_FalseValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(ValueTest, Bool) {
  cel_Value false_value;
  cel_Value_SetFalse(&false_value);
  EXPECT_EQ(cel_Value_Kind(&false_value), cel_ValueKind_kBool);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&false_value),
                                    cel_StringView_FromString("bool")));
  EXPECT_TRUE(cel_Value_IsBool(&false_value));
  EXPECT_TRUE(cel_Value_IsFalse(&false_value));
  EXPECT_FALSE(cel_Value_IsTrue(&false_value));
  EXPECT_THAT(cel_Value_AsBool(&false_value), Pointee(false));

  cel_Value true_value;
  cel_Value_SetTrue(&true_value);
  EXPECT_EQ(cel_Value_Kind(&true_value), cel_ValueKind_kBool);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&true_value),
                                    cel_StringView_FromString("bool")));
  EXPECT_TRUE(cel_Value_IsBool(&true_value));
  EXPECT_FALSE(cel_Value_IsFalse(&true_value));
  EXPECT_TRUE(cel_Value_IsTrue(&true_value));
  EXPECT_THAT(cel_Value_AsBool(&true_value), Pointee(true));

  cel_Value value;
  cel_Value result;

  cel_Value_SetFalse(&value);
  ASSERT_TRUE(
      cel_Value_Equals(&value, ctx(), &cel_FalseValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&value, ctx(), &cel_TrueValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value_SetTrue(&value);
  ASSERT_TRUE(
      cel_Value_Equals(&value, ctx(), &cel_FalseValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&value, ctx(), &cel_TrueValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&value, ctx(), &cel_NullValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value string_value;
  cel_Value_SetString(&string_value, cel_StringView_FromString(""));

  ASSERT_TRUE(
      cel_Value_Equals(&value, ctx(), &string_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(ValueTest, Int) {
  cel_Value zero_value;
  cel_Value_SetInt(&zero_value, 0);
  EXPECT_EQ(cel_Value_Kind(&zero_value), cel_ValueKind_kInt);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&zero_value),
                                    cel_StringView_FromString("int")));
  EXPECT_TRUE(cel_Value_IsInt(&zero_value));
  EXPECT_EQ(cel_Value_GetInt(&zero_value), 0);
  EXPECT_THAT(cel_Value_AsInt(&zero_value), Pointee(0));

  cel_Value one_value;
  cel_Value_SetInt(&one_value, 1);
  EXPECT_EQ(cel_Value_Kind(&one_value), cel_ValueKind_kInt);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&one_value),
                                    cel_StringView_FromString("int")));
  EXPECT_TRUE(cel_Value_IsInt(&one_value));
  EXPECT_EQ(cel_Value_GetInt(&one_value), 1);
  EXPECT_THAT(cel_Value_AsInt(&one_value), Pointee(1));

  cel_Value other_zero_value = zero_value;
  cel_Value other_one_value = one_value;
  cel_Value zero_uint_value;
  cel_Value one_uint_value;
  cel_Value zero_double_value;
  cel_Value one_double_value;
  cel_Value result;

  cel_Value_SetUint(&zero_uint_value, 0);
  cel_Value_SetUint(&one_uint_value, 1);
  cel_Value_SetDouble(&zero_double_value, 0);
  cel_Value_SetDouble(&one_double_value, 1);

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &zero_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&zero_value, ctx(), &other_zero_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&zero_value, ctx(), &zero_uint_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&zero_value, ctx(), &zero_double_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &one_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&one_value, ctx(), &zero_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&one_value, ctx(), &one_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&one_value, ctx(), &other_one_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&one_value, ctx(), &one_uint_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&one_value, ctx(), &one_double_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &cel_NullValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value string_value;
  cel_Value_SetString(&string_value, cel_StringView_FromString(""));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &string_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(ValueTest, Uint) {
  cel_Value zero_value;
  cel_Value_SetUint(&zero_value, 0);
  EXPECT_EQ(cel_Value_Kind(&zero_value), cel_ValueKind_kUint);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&zero_value),
                                    cel_StringView_FromString("uint")));
  EXPECT_TRUE(cel_Value_IsUint(&zero_value));
  EXPECT_EQ(cel_Value_GetUint(&zero_value), 0);
  EXPECT_THAT(cel_Value_AsUint(&zero_value), Pointee(0));

  cel_Value one_value;
  cel_Value_SetUint(&one_value, 1);
  EXPECT_EQ(cel_Value_Kind(&one_value), cel_ValueKind_kUint);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&one_value),
                                    cel_StringView_FromString("uint")));
  EXPECT_TRUE(cel_Value_IsUint(&one_value));
  EXPECT_EQ(cel_Value_GetUint(&one_value), 1);
  EXPECT_THAT(cel_Value_AsUint(&one_value), Pointee(1));

  cel_Value other_zero_value = zero_value;
  cel_Value other_one_value = one_value;
  cel_Value zero_int_value;
  cel_Value one_int_value;
  cel_Value zero_double_value;
  cel_Value one_double_value;
  cel_Value result;

  cel_Value_SetInt(&zero_int_value, 0);
  cel_Value_SetInt(&one_int_value, 1);
  cel_Value_SetDouble(&zero_double_value, 0);
  cel_Value_SetDouble(&one_double_value, 1);

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &zero_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&zero_value, ctx(), &other_zero_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &zero_int_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&zero_value, ctx(), &zero_double_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &one_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&one_value, ctx(), &zero_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&one_value, ctx(), &one_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&one_value, ctx(), &other_one_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&one_value, ctx(), &one_int_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&one_value, ctx(), &one_double_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &cel_NullValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value string_value;
  cel_Value_SetString(&string_value, cel_StringView_FromString(""));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &string_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(ValueTest, Double) {
  cel_Value zero_value;
  cel_Value_SetDouble(&zero_value, 0);
  EXPECT_EQ(cel_Value_Kind(&zero_value), cel_ValueKind_kDouble);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&zero_value),
                                    cel_StringView_FromString("double")));
  EXPECT_TRUE(cel_Value_IsDouble(&zero_value));
  EXPECT_EQ(cel_Value_GetDouble(&zero_value), 0);
  EXPECT_THAT(cel_Value_AsDouble(&zero_value), Pointee(0));

  cel_Value one_value;
  cel_Value_SetDouble(&one_value, 1);
  EXPECT_EQ(cel_Value_Kind(&one_value), cel_ValueKind_kDouble);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&one_value),
                                    cel_StringView_FromString("double")));
  EXPECT_TRUE(cel_Value_IsDouble(&one_value));
  EXPECT_EQ(cel_Value_GetDouble(&one_value), 1);
  EXPECT_THAT(cel_Value_AsDouble(&one_value), Pointee(1));

  cel_Value other_zero_value = zero_value;
  cel_Value other_one_value = one_value;
  cel_Value zero_int_value;
  cel_Value one_int_value;
  cel_Value zero_uint_value;
  cel_Value one_uint_value;
  cel_Value result;

  cel_Value_SetInt(&zero_int_value, 0);
  cel_Value_SetInt(&one_int_value, 1);
  cel_Value_SetUint(&zero_uint_value, 0);
  cel_Value_SetUint(&one_uint_value, 1);

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &zero_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&zero_value, ctx(), &other_zero_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &zero_int_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&zero_value, ctx(), &zero_uint_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &one_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&one_value, ctx(), &zero_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&one_value, ctx(), &one_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&one_value, ctx(), &other_one_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&one_value, ctx(), &one_int_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&one_value, ctx(), &one_uint_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &cel_NullValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value string_value;
  cel_Value_SetString(&string_value, cel_StringView_FromString(""));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &string_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(ValueTest, Bytes) {
  cel_Value empty_value;
  cel_Value_SetBytes(&empty_value, cel_StringView_FromString(""));
  EXPECT_EQ(cel_Value_Kind(&empty_value), cel_ValueKind_kBytes);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&empty_value),
                                    cel_StringView_FromString("bytes")));
  EXPECT_TRUE(cel_Value_IsBytes(&empty_value));
  EXPECT_EQ(cel_Value_GetBytes(&empty_value), cel_StringView_FromString(""));
  EXPECT_THAT(cel_Value_AsBytes(&empty_value),
              Pointee(cel_StringView_FromString("")));

  cel_Value nonempty_value;
  cel_Value_SetBytes(&nonempty_value, cel_StringView_FromString("foo"));
  EXPECT_EQ(cel_Value_Kind(&nonempty_value), cel_ValueKind_kBytes);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&nonempty_value),
                                    cel_StringView_FromString("bytes")));
  EXPECT_TRUE(cel_Value_IsBytes(&nonempty_value));
  EXPECT_EQ(cel_Value_GetBytes(&nonempty_value),
            cel_StringView_FromString("foo"));
  EXPECT_THAT(cel_Value_AsBytes(&nonempty_value),
              Pointee(cel_StringView_FromString("foo")));

  cel_Value other_empty_value = empty_value;
  cel_Value other_nonempty_value = nonempty_value;
  cel_Value result;

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &empty_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&empty_value, ctx(), &other_empty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&empty_value, ctx(), &nonempty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &nonempty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &other_nonempty_value,
                               &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &empty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &cel_NullValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value string_value;
  cel_Value_SetString(&string_value, cel_StringView_FromString(""));

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &string_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(ValueTest, String) {
  cel_Value empty_value;
  cel_Value_SetString(&empty_value, cel_StringView_FromString(""));
  EXPECT_EQ(cel_Value_Kind(&empty_value), cel_ValueKind_kString);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&empty_value),
                                    cel_StringView_FromString("string")));
  EXPECT_TRUE(cel_Value_IsString(&empty_value));
  EXPECT_EQ(cel_Value_GetString(&empty_value), cel_StringView_FromString(""));
  EXPECT_THAT(cel_Value_AsString(&empty_value),
              Pointee(cel_StringView_FromString("")));

  cel_Value nonempty_value;
  cel_Value_SetString(&nonempty_value, cel_StringView_FromString("foo"));
  EXPECT_EQ(cel_Value_Kind(&nonempty_value), cel_ValueKind_kString);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&nonempty_value),
                                    cel_StringView_FromString("string")));
  EXPECT_TRUE(cel_Value_IsString(&nonempty_value));
  EXPECT_EQ(cel_Value_GetString(&nonempty_value),
            cel_StringView_FromString("foo"));
  EXPECT_THAT(cel_Value_AsString(&nonempty_value),
              Pointee(cel_StringView_FromString("foo")));

  cel_Value other_empty_value = empty_value;
  cel_Value other_nonempty_value = nonempty_value;
  cel_Value result;

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &empty_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&empty_value, ctx(), &other_empty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&empty_value, ctx(), &nonempty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &nonempty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &other_nonempty_value,
                               &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &empty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &cel_NullValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value bytes_value;
  cel_Value_SetBytes(&bytes_value, cel_StringView_FromString(""));

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &bytes_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(ValueTest, Duration) {
  cel_Value zero_value;
  cel_Value_SetDuration(&zero_value, cel_Duration_kZero);
  EXPECT_EQ(cel_Value_Kind(&zero_value), cel_ValueKind_kDuration);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_Value_TypeName(&zero_value),
      cel_StringView_FromString("google.protobuf.Duration")));
  EXPECT_TRUE(cel_Value_IsDuration(&zero_value));
  EXPECT_EQ(cel_Value_GetDuration(&zero_value), cel_Duration_kZero);
  EXPECT_THAT(cel_Value_AsDuration(&zero_value), Pointee(cel_Duration_kZero));

  cel_Value max_value;
  cel_Value_SetDuration(&max_value, cel_Duration_kMax);
  EXPECT_EQ(cel_Value_Kind(&max_value), cel_ValueKind_kDuration);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_Value_TypeName(&max_value),
      cel_StringView_FromString("google.protobuf.Duration")));
  EXPECT_TRUE(cel_Value_IsDuration(&max_value));
  EXPECT_EQ(cel_Value_GetDuration(&max_value), cel_Duration_kMax);
  EXPECT_THAT(cel_Value_AsDuration(&max_value), Pointee(cel_Duration_kMax));

  cel_Value other_zero_value = zero_value;
  cel_Value other_max_value = max_value;
  cel_Value result;

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &zero_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&zero_value, ctx(), &other_zero_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &max_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&max_value, ctx(), &zero_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&max_value, ctx(), &max_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&max_value, ctx(), &other_max_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &cel_NullValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value string_value;
  cel_Value_SetString(&string_value, cel_StringView_FromString(""));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &string_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(ValueTest, Timestamp) {
  cel_Value zero_value;
  cel_Value_SetTimestamp(&zero_value, cel_Timestamp_kUnixEpoch);
  EXPECT_EQ(cel_Value_Kind(&zero_value), cel_ValueKind_kTimestamp);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_Value_TypeName(&zero_value),
      cel_StringView_FromString("google.protobuf.Timestamp")));
  EXPECT_TRUE(cel_Value_IsTimestamp(&zero_value));
  EXPECT_EQ(cel_Value_GetTimestamp(&zero_value), cel_Timestamp_kUnixEpoch);
  EXPECT_THAT(cel_Value_AsTimestamp(&zero_value),
              Pointee(cel_Timestamp_kUnixEpoch));

  cel_Value max_value;
  cel_Value_SetTimestamp(&max_value, cel_Timestamp_kMax);
  EXPECT_EQ(cel_Value_Kind(&max_value), cel_ValueKind_kTimestamp);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_Value_TypeName(&max_value),
      cel_StringView_FromString("google.protobuf.Timestamp")));
  EXPECT_TRUE(cel_Value_IsTimestamp(&max_value));
  EXPECT_EQ(cel_Value_GetTimestamp(&max_value), cel_Timestamp_kMax);
  EXPECT_THAT(cel_Value_AsTimestamp(&max_value), Pointee(cel_Timestamp_kMax));

  cel_Value other_zero_value = zero_value;
  cel_Value other_max_value = max_value;
  cel_Value result;

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &zero_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&zero_value, ctx(), &other_zero_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &max_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&max_value, ctx(), &zero_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&max_value, ctx(), &max_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&max_value, ctx(), &other_max_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &cel_NullValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value string_value;
  cel_Value_SetString(&string_value, cel_StringView_FromString(""));

  ASSERT_TRUE(
      cel_Value_Equals(&zero_value, ctx(), &string_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(ValueTest, List) {
  cel_Value result;

  cel_Value empty_value;
  _cel_EmptyListValue_Set(cel_Value_SetList(&empty_value));
  EXPECT_EQ(cel_Value_Kind(&empty_value), cel_ValueKind_kList);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&empty_value),
                                    cel_StringView_FromString("list")));
  EXPECT_TRUE(cel_Value_IsList(&empty_value));
  EXPECT_THAT(cel_Value_GetList(&empty_value), NotNull());
  EXPECT_THAT(cel_Value_AsList(&empty_value), cel_Value_GetList(&empty_value));

  cel_Value nonempty_value;
  cel_ListValue* mutable_nonempty_value = cel_Value_SetList(&nonempty_value);
  _cel_MutableListValue_Set(mutable_nonempty_value);
  EXPECT_TRUE(
      _cel_MutableListValue_Reserve(mutable_nonempty_value, 1, arena()));
  {
    cel_Value* nonempty_value_elem =
        _cel_MutableListValue_Add(mutable_nonempty_value, arena());
    ASSERT_THAT(nonempty_value_elem, NotNull());
    cel_Value_SetInt(nonempty_value_elem, 1);
  }
  EXPECT_EQ(cel_Value_Kind(&nonempty_value), cel_ValueKind_kList);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&nonempty_value),
                                    cel_StringView_FromString("list")));
  EXPECT_TRUE(cel_Value_IsList(&nonempty_value));
  EXPECT_EQ(cel_Value_GetList(&nonempty_value), mutable_nonempty_value);
  EXPECT_EQ(cel_Value_AsList(&nonempty_value), mutable_nonempty_value);

  cel_Value other_empty_value = empty_value;
  cel_Value other_nonempty_value = nonempty_value;

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &empty_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&empty_value, ctx(), &other_empty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&empty_value, ctx(), &nonempty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &nonempty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &other_nonempty_value,
                               &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &empty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &cel_NullValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value string_value;
  cel_Value_SetString(&string_value, cel_StringView_FromString(""));

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &string_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(ValueTest, Map) {
  cel_Value result;

  cel_Value empty_value;
  _cel_EmptyMapValue_Set(cel_Value_SetMap(&empty_value));
  EXPECT_EQ(cel_Value_Kind(&empty_value), cel_ValueKind_kMap);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&empty_value),
                                    cel_StringView_FromString("map")));
  EXPECT_TRUE(cel_Value_IsMap(&empty_value));
  EXPECT_THAT(cel_Value_GetMap(&empty_value), NotNull());
  EXPECT_THAT(cel_Value_AsMap(&empty_value), cel_Value_GetMap(&empty_value));

  cel_Value nonempty_value;
  cel_MapValue* mutable_nonempty_value = cel_Value_SetMap(&nonempty_value);
  _cel_MutableMapValue_Set(mutable_nonempty_value);
  EXPECT_TRUE(_cel_MutableMapValue_Reserve(mutable_nonempty_value, 1, arena()));
  {
    cel_MapValueKey key;
    cel_Value* value;
    cel_MapValueKey_SetInt(&key, 1);
    ASSERT_EQ(_cel_MutableMapValue_Insert(mutable_nonempty_value, &key,
                                          cel_nullptr, &value, arena()),
              _cel_MutableMapValueInsertResult_kInserted);
    cel_Value_SetInt(value, 2);
  }
  EXPECT_EQ(cel_Value_Kind(&nonempty_value), cel_ValueKind_kMap);
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&nonempty_value),
                                    cel_StringView_FromString("map")));
  EXPECT_TRUE(cel_Value_IsMap(&nonempty_value));
  EXPECT_EQ(cel_Value_GetMap(&nonempty_value), mutable_nonempty_value);
  EXPECT_EQ(cel_Value_AsMap(&nonempty_value), mutable_nonempty_value);

  cel_Value other_empty_value = empty_value;
  cel_Value other_nonempty_value = nonempty_value;

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &empty_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&empty_value, ctx(), &other_empty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&empty_value, ctx(), &nonempty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &nonempty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &other_nonempty_value,
                               &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &empty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &cel_NullValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value string_value;
  cel_Value_SetString(&string_value, cel_StringView_FromString(""));

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &string_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(ValueTest, Struct) {
  cel_Value result;

  cel_Value empty_value;
  _cel_ParsedMessageValue_Set(
      cel_Value_SetStruct(&empty_value),
      ParseProto("cel.expr.conformance.proto3.TestAllTypes", "", ""),
      TestAllTypesDef());
  EXPECT_EQ(cel_Value_Kind(&empty_value), cel_ValueKind_kStruct);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_Value_TypeName(&empty_value),
      cel_StringView_FromString("cel.expr.conformance.proto3.TestAllTypes")));
  EXPECT_TRUE(cel_Value_IsStruct(&empty_value));
  EXPECT_THAT(cel_Value_GetStruct(&empty_value), NotNull());
  EXPECT_THAT(cel_Value_AsStruct(&empty_value),
              cel_Value_GetStruct(&empty_value));

  cel_Value nonempty_value;
  _cel_ParsedMessageValue_Set(
      cel_Value_SetStruct(&nonempty_value),
      ParseProto("cel.expr.conformance.proto3.TestAllTypes",
                 R"pb(single_int32: 1)pb", ""),
      TestAllTypesDef());
  EXPECT_EQ(cel_Value_Kind(&nonempty_value), cel_ValueKind_kStruct);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_Value_TypeName(&nonempty_value),
      cel_StringView_FromString("cel.expr.conformance.proto3.TestAllTypes")));
  EXPECT_TRUE(cel_Value_IsStruct(&nonempty_value));
  EXPECT_THAT(cel_Value_GetStruct(&nonempty_value), NotNull());
  EXPECT_THAT(cel_Value_AsStruct(&nonempty_value),
              cel_Value_GetStruct(&nonempty_value));

  cel_Value other_empty_value = empty_value;
  cel_Value other_nonempty_value = nonempty_value;

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &empty_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&empty_value, ctx(), &other_empty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&empty_value, ctx(), &nonempty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &nonempty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &other_nonempty_value,
                               &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &empty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &cel_NullValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value string_value;
  cel_Value_SetString(&string_value, cel_StringView_FromString(""));

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &string_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(ValueTest, Optional) {
  cel_Value result;

  cel_Value empty_value;
  cel_OptionalValue_Empty(cel_Value_SetOptional(&empty_value));
  EXPECT_EQ(cel_Value_Kind(&empty_value), cel_ValueKind_kOpaque);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_Value_TypeName(&empty_value),
                            cel_StringView_FromString("optional_type")));
  EXPECT_TRUE(cel_Value_IsOpaque(&empty_value));
  EXPECT_TRUE(cel_Value_IsOptional(&empty_value));
  EXPECT_THAT(cel_Value_GetOptional(&empty_value), NotNull());
  EXPECT_THAT(cel_Value_AsOptional(&empty_value),
              cel_Value_GetOptional(&empty_value));
  EXPECT_THAT(cel_Value_GetOpaque(&empty_value), NotNull());
  EXPECT_THAT(cel_Value_AsOpaque(&empty_value),
              cel_Value_GetOpaque(&empty_value));
  EXPECT_TRUE(cel_OptionalValue_HasValue(cel_Value_GetOptional(&empty_value),
                                         ctx(), &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
  EXPECT_TRUE(cel_OptionalValue_Value(cel_Value_GetOptional(&empty_value),
                                      ctx(), &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsError(&result));

  cel_Value nonempty_value;
  ASSERT_TRUE(cel_OptionalValue_Of(cel_Value_SetOptional(&nonempty_value),
                                   &cel_NullValue, arena()));
  EXPECT_EQ(cel_Value_Kind(&nonempty_value), cel_ValueKind_kOpaque);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_Value_TypeName(&nonempty_value),
                            cel_StringView_FromString("optional_type")));
  EXPECT_TRUE(cel_Value_IsOpaque(&nonempty_value));
  EXPECT_TRUE(cel_Value_IsOptional(&nonempty_value));
  EXPECT_THAT(cel_Value_GetOptional(&nonempty_value), NotNull());
  EXPECT_THAT(cel_Value_AsOptional(&nonempty_value),
              cel_Value_GetOptional(&nonempty_value));
  EXPECT_THAT(cel_Value_GetOpaque(&nonempty_value), NotNull());
  EXPECT_THAT(cel_Value_AsOpaque(&nonempty_value),
              cel_Value_GetOpaque(&nonempty_value));
  EXPECT_TRUE(cel_OptionalValue_HasValue(cel_Value_GetOptional(&nonempty_value),
                                         ctx(), &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
  EXPECT_TRUE(cel_OptionalValue_Value(cel_Value_GetOptional(&nonempty_value),
                                      ctx(), &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsNull(&result));

  cel_Value other_empty_value = empty_value;
  cel_Value other_nonempty_value = nonempty_value;

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &empty_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&empty_value, ctx(), &other_empty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&empty_value, ctx(), &nonempty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &nonempty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &other_nonempty_value,
                               &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&nonempty_value, ctx(), &empty_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &cel_NullValue, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value bytes_value;
  cel_Value_SetBytes(&bytes_value, cel_StringView_FromString(""));

  ASSERT_TRUE(
      cel_Value_Equals(&empty_value, ctx(), &bytes_value, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(ValueTest, Type) {
  cel_Value int_type_value;
  cel_Value_SetType(&int_type_value, cel_StringView_From("int"));
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&int_type_value),
                                    cel_StringView_FromString("type")));
  EXPECT_TRUE(cel_Value_IsType(&int_type_value));
  EXPECT_EQ(cel_Value_GetType(&int_type_value), cel_StringView_From("int"));
  EXPECT_THAT(cel_Value_AsType(&int_type_value),
              Pointee(cel_StringView_From("int")));

  cel_Value uint_type_value;
  cel_Value_SetType(&uint_type_value, cel_StringView_From("uint"));
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_TypeName(&uint_type_value),
                                    cel_StringView_FromString("type")));
  EXPECT_TRUE(cel_Value_IsType(&uint_type_value));
  EXPECT_EQ(cel_Value_GetType(&uint_type_value), cel_StringView_From("uint"));
  EXPECT_THAT(cel_Value_AsType(&uint_type_value),
              Pointee(cel_StringView_From("uint")));

  cel_Value other_int_type_value = int_type_value;
  cel_Value other_uint_type_value = uint_type_value;
  cel_Value result;

  ASSERT_TRUE(cel_Value_Equals(&int_type_value, ctx(), &int_type_value, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&int_type_value, ctx(), &other_int_type_value,
                               &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&int_type_value, ctx(), &uint_type_value,
                               &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_Value_Equals(&uint_type_value, ctx(), &int_type_value,
                               &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_TRUE(cel_Value_Equals(&uint_type_value, ctx(), &uint_type_value,
                               &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&uint_type_value, ctx(), &other_uint_type_value,
                               &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_Value_Equals(&int_type_value, ctx(), &cel_NullValue, &result,
                               status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value string_value;
  cel_Value_SetString(&string_value, cel_StringView_FromString(""));

  ASSERT_TRUE(cel_Value_Equals(&int_type_value, ctx(), &string_value, &result,
                               status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(ValueTest, Error) {
  cel_Error* error = cel_Error_New(arena());
  ASSERT_THAT(error, NotNull());

  cel_Value value;
  cel_Value_SetError(&value, error);
  EXPECT_EQ(cel_Value_Kind(&value), cel_ValueKind_kError);
  EXPECT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Value_GetError(&value), error);
  EXPECT_EQ(cel_Value_AsError(&value), error);

  cel_Error* other_error = cel_Error_New(arena());
  ASSERT_THAT(other_error, NotNull());

  cel_Value other_value;
  cel_Value_SetError(&other_value, other_error);

  cel_Value result;

  ASSERT_TRUE(cel_Value_Equals(&value, ctx(), &value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_THAT(cel_Value_AsError(&result), error);

  ASSERT_TRUE(cel_Value_Equals(&value, ctx(), &other_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_THAT(cel_Value_AsError(&result), error);

  ASSERT_TRUE(cel_Value_Equals(&other_value, ctx(), &value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_THAT(cel_Value_AsError(&result), other_error);

  cel_Value string_value;
  cel_Value_SetString(&string_value, cel_StringView_FromString(""));

  ASSERT_TRUE(
      cel_Value_Equals(&value, ctx(), &string_value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_THAT(cel_Value_AsError(&result), error);

  ASSERT_TRUE(
      cel_Value_Equals(&string_value, ctx(), &value, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_THAT(cel_Value_AsError(&result), error);
}

using ValueFromMessageTest = ValueTest;

TEST_F(ValueFromMessageTest, BoolValue) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.BoolValue", R"pb(value: true)pb", ""),
      MessageDef("google.protobuf.BoolValue"), status()));
  EXPECT_TRUE(cel_Value_IsTrue(&value));
}

TEST_F(ValueFromMessageTest, Int32Value) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.Int32Value", R"pb(value: 1)pb", ""),
      MessageDef("google.protobuf.Int32Value"), status()));
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);
}

TEST_F(ValueFromMessageTest, Int64Value) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.Int64Value", R"pb(value: 1)pb", ""),
      MessageDef("google.protobuf.Int64Value"), status()));
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);
}

TEST_F(ValueFromMessageTest, UInt32Value) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.UInt32Value", R"pb(value: 1)pb", ""),
      MessageDef("google.protobuf.UInt32Value"), status()));
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 1);
}

TEST_F(ValueFromMessageTest, UInt64Value) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.UInt64Value", R"pb(value: 1)pb", ""),
      MessageDef("google.protobuf.UInt64Value"), status()));
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 1);
}

TEST_F(ValueFromMessageTest, FloatValue) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.FloatValue", R"pb(value: 1)pb", ""),
      MessageDef("google.protobuf.FloatValue"), status()));
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1);
}

TEST_F(ValueFromMessageTest, DoubleValue) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.DoubleValue", R"pb(value: 1)pb", ""),
      MessageDef("google.protobuf.DoubleValue"), status()));
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1);
}

TEST_F(ValueFromMessageTest, BytesValue) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.BytesValue", R"pb(value: "foo")pb", ""),
      MessageDef("google.protobuf.BytesValue"), status()));
  ASSERT_TRUE(cel_Value_IsBytes(&value));
  EXPECT_EQ(cel_Value_GetBytes(&value), cel_StringView_FromString("foo"));
}

TEST_F(ValueFromMessageTest, StringValue) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.StringValue", R"pb(value: "foo")pb", ""),
      MessageDef("google.protobuf.StringValue"), status()));
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_FromString("foo"));
}

TEST_F(ValueFromMessageTest, Duration) {
  cel_Value value;

  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.Duration",
                 R"pb(seconds: 315576000000 nanos: 999999999)pb", ""),
      MessageDef("google.protobuf.Duration"), status()));
  ASSERT_TRUE(cel_Value_IsDuration(&value));
  EXPECT_EQ(cel_Value_GetDuration(&value), cel_Duration_kMax);

  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.Duration",
                 R"pb(seconds: 315576000000 nanos: 1000000000)pb", ""),
      MessageDef("google.protobuf.Duration"), status()));
  EXPECT_TRUE(cel_Value_IsError(&value));
}

TEST_F(ValueFromMessageTest, Timestamp) {
  cel_Value value;

  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.Timestamp",
                 R"pb(seconds: 253402300799 nanos: 999999999)pb", ""),
      MessageDef("google.protobuf.Timestamp"), status()));
  ASSERT_TRUE(cel_Value_IsTimestamp(&value));
  EXPECT_EQ(cel_Value_GetTimestamp(&value), cel_Timestamp_kMax);

  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.Timestamp",
                 R"pb(seconds: 253402300799 nanos: 1000000000)pb", ""),
      MessageDef("google.protobuf.Timestamp"), status()));
  EXPECT_TRUE(cel_Value_IsError(&value));
}

TEST_F(ValueFromMessageTest, Any) {
  cel_Value value;

  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto(
          "google.protobuf.Any",
          R"pb(type_url: "type.googleapis.com/google.protobuf.BoolValue")pb",
          ""),
      MessageDef("google.protobuf.Any"), status()));
  EXPECT_TRUE(cel_Value_IsFalse(&value));

  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto(
          "google.protobuf.Any",
          R"pb(type_url: "type.googleapis.com/message.that.does.not.Exist")pb",
          ""),
      MessageDef("google.protobuf.Any"), status()));
  EXPECT_TRUE(cel_Value_IsError(&value));
}

TEST_F(ValueFromMessageTest, Value) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(), ParseProto("google.protobuf.Value", R"pb()pb", ""),
      MessageDef("google.protobuf.Value"), status()));
  EXPECT_TRUE(cel_Value_IsNull(&value));
}

TEST_F(ValueFromMessageTest, Value_Null) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.Value", R"pb(null_value: NULL_VALUE)pb", ""),
      MessageDef("google.protobuf.Value"), status()));
  EXPECT_TRUE(cel_Value_IsNull(&value));
}

TEST_F(ValueFromMessageTest, Value_Bool) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.Value", R"pb(bool_value: true)pb", ""),
      MessageDef("google.protobuf.Value"), status()));
  EXPECT_TRUE(cel_Value_IsTrue(&value));
}

TEST_F(ValueFromMessageTest, Value_Number) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.Value", R"pb(number_value: 1)pb", ""),
      MessageDef("google.protobuf.Value"), status()));
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1);
}

TEST_F(ValueFromMessageTest, Value_String) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.Value", R"pb(string_value: "foo")pb", ""),
      MessageDef("google.protobuf.Value"), status()));
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_FromString("foo"));
}

TEST_F(ValueFromMessageTest, Value_List) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.Value", R"pb(list_value: {})pb", ""),
      MessageDef("google.protobuf.Value"), status()));
  EXPECT_TRUE(cel_Value_IsList(&value));
}

TEST_F(ValueFromMessageTest, Value_Map) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(),
      ParseProto("google.protobuf.Value", R"pb(struct_value: {})pb", ""),
      MessageDef("google.protobuf.Value"), status()));
  EXPECT_TRUE(cel_Value_IsMap(&value));
}

TEST_F(ValueFromMessageTest, ListValue) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(), ParseProto("google.protobuf.ListValue", R"pb()pb", ""),
      MessageDef("google.protobuf.ListValue"), status()));
  EXPECT_TRUE(cel_Value_IsList(&value));
}

TEST_F(ValueFromMessageTest, Struct) {
  cel_Value value;
  ASSERT_TRUE(cel_Value_FromMessage(
      &value, ctx(), ParseProto("google.protobuf.Struct", R"pb()pb", ""),
      MessageDef("google.protobuf.Struct"), status()));
  EXPECT_TRUE(cel_Value_IsMap(&value));
}

using ValueFromEnumTest = ValueTest;

TEST_F(ValueFromEnumTest, Null) {
  cel_Value value;

  ASSERT_TRUE(cel_Value_FromEnum(
      &value, ctx(), 0, EnumDef("google.protobuf.NullValue"), status()));
  EXPECT_TRUE(cel_Value_IsNull(&value));

  ASSERT_TRUE(cel_Value_FromEnumValue(
      &value, ctx(), EnumValueDef("google.protobuf.NullValue", "NULL_VALUE"),
      status()));
  EXPECT_TRUE(cel_Value_IsNull(&value));
}

TEST_F(ValueFromEnumTest, NotNull) {
  cel_Value value;

  ASSERT_TRUE(cel_Value_FromEnum(
      &value, ctx(), 0,
      EnumDef("cel.expr.conformance.proto3.TestAllTypes.NestedEnum"),
      status()));
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  ASSERT_TRUE(cel_Value_FromEnumValue(
      &value, ctx(),
      EnumValueDef("cel.expr.conformance.proto3.TestAllTypes.NestedEnum",
                   "FOO"),
      status()));
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);
}

}  // namespace
