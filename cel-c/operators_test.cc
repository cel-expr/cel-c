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

#include "cel-c/operators.h"

#include "gtest/gtest.h"
#include "cel-c/string_view.h"

namespace {

TEST(UnaryOp, FromString) {
  cel_UnaryOp op = cel_UnaryOp_kUnspecified;

  ASSERT_TRUE(cel_UnaryOp_FromString(cel_StringView_From("!_"), &op));
  EXPECT_EQ(op, cel_UnaryOp_kLogicalNot);

  op = cel_UnaryOp_kUnspecified;

  ASSERT_TRUE(cel_UnaryOp_FromString(cel_StringView_From("-_"), &op));
  EXPECT_EQ(op, cel_UnaryOp_kNegate);

  op = cel_UnaryOp_kUnspecified;

  ASSERT_TRUE(
      cel_UnaryOp_FromString(cel_StringView_From("@not_strictly_false"), &op));
  EXPECT_EQ(op, cel_UnaryOp_kNotStrictlyFalse);

  op = cel_UnaryOp_kUnspecified;

  ASSERT_TRUE(cel_UnaryOp_FromString(
      cel_StringView_From("__not_strictly_false__"), &op));
  EXPECT_EQ(op, cel_UnaryOp_kNotStrictlyFalse);

  op = cel_UnaryOp_kUnspecified;

  ASSERT_FALSE(cel_UnaryOp_FromString(
      cel_StringView_From("@unknown_unary_operator"), &op));
}

TEST(UnaryOp, ToString) {
  EXPECT_TRUE(
      cel_StringView_Equals(cel_UnaryOp_ToString(cel_UnaryOp_kLogicalNot),
                            cel_StringView_From("!_")));

  EXPECT_TRUE(cel_StringView_Equals(cel_UnaryOp_ToString(cel_UnaryOp_kNegate),
                                    cel_StringView_From("-_")));

  EXPECT_TRUE(
      cel_StringView_Equals(cel_UnaryOp_ToString(cel_UnaryOp_kNotStrictlyFalse),
                            cel_StringView_From("@not_strictly_false")));

  EXPECT_TRUE(cel_StringView_Equals(
      cel_UnaryOp_ToString(cel_UnaryOp_kUnspecified), cel_StringView_From("")));
}

TEST(UnaryOp, Precedence) {
  EXPECT_EQ(cel_UnaryOp_Precedence(cel_UnaryOp_kLogicalNot), 2);

  EXPECT_EQ(cel_UnaryOp_Precedence(cel_UnaryOp_kNegate), 2);

  EXPECT_EQ(cel_UnaryOp_Precedence(cel_UnaryOp_kNotStrictlyFalse), 0);

  EXPECT_EQ(cel_UnaryOp_Precedence(cel_UnaryOp_kUnspecified), 0);
}

TEST(BinaryOp, FromString) {
  cel_BinaryOp op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_&&_"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kLogicalAnd);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_||_"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kLogicalOr);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_==_"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kEquals);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_!=_"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kNotEquals);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_<_"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kLess);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_<=_"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kLessEquals);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_>_"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kGreater);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_>=_"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kGreaterEquals);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_+_"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kAdd);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_-_"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kSubtract);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_*_"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kMultiply);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_/_"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kDivide);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_%_"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kModulo);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_[_]"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kIndex);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("@in"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kIn);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_in_"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kIn);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_[?_]"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kOptIndex);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_TRUE(cel_BinaryOp_FromString(cel_StringView_From("_?._"), &op));
  EXPECT_EQ(op, cel_BinaryOp_kOptSelect);

  op = cel_BinaryOp_kUnspecified;

  ASSERT_FALSE(cel_BinaryOp_FromString(
      cel_StringView_From("@unknown_binary_operator"), &op));
}

TEST(BinaryOp, ToString) {
  EXPECT_TRUE(
      cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kLogicalAnd),
                            cel_StringView_From("_&&_")));

  EXPECT_TRUE(
      cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kLogicalOr),
                            cel_StringView_From("_||_")));

  EXPECT_TRUE(cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kEquals),
                                    cel_StringView_From("_==_")));

  EXPECT_TRUE(
      cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kNotEquals),
                            cel_StringView_From("_!=_")));

  EXPECT_TRUE(cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kLess),
                                    cel_StringView_From("_<_")));

  EXPECT_TRUE(
      cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kLessEquals),
                            cel_StringView_From("_<=_")));

  EXPECT_TRUE(
      cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kGreater),
                            cel_StringView_From("_>_")));

  EXPECT_TRUE(
      cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kGreaterEquals),
                            cel_StringView_From("_>=_")));

  EXPECT_TRUE(cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kAdd),
                                    cel_StringView_From("_+_")));

  EXPECT_TRUE(
      cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kSubtract),
                            cel_StringView_From("_-_")));

  EXPECT_TRUE(
      cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kMultiply),
                            cel_StringView_From("_*_")));

  EXPECT_TRUE(cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kDivide),
                                    cel_StringView_From("_/_")));

  EXPECT_TRUE(cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kModulo),
                                    cel_StringView_From("_%_")));

  EXPECT_TRUE(cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kIndex),
                                    cel_StringView_From("_[_]")));

  EXPECT_TRUE(cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kIn),
                                    cel_StringView_From("@in")));

  EXPECT_TRUE(
      cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kOptIndex),
                            cel_StringView_From("_[?_]")));

  EXPECT_TRUE(
      cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kOptSelect),
                            cel_StringView_From("_?._")));

  EXPECT_TRUE(
      cel_StringView_Equals(cel_BinaryOp_ToString(cel_BinaryOp_kUnspecified),
                            cel_StringView_From("")));
}

TEST(BinaryOp, Precedence) {
  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kLogicalAnd), 6);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kLogicalOr), 7);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kEquals), 5);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kNotEquals), 5);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kLess), 5);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kLessEquals), 5);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kGreater), 5);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kGreaterEquals), 5);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kAdd), 4);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kSubtract), 4);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kMultiply), 3);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kDivide), 3);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kModulo), 3);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kIndex), 1);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kIn), 5);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kOptIndex), 0);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kOptSelect), 0);

  EXPECT_EQ(cel_BinaryOp_Precedence(cel_BinaryOp_kUnspecified), 0);
}

TEST(TernaryOp, FromString) {
  cel_TernaryOp op = cel_TernaryOp_kUnspecified;

  ASSERT_TRUE(cel_TernaryOp_FromString(cel_StringView_From("_?_:_"), &op));
  EXPECT_EQ(op, cel_TernaryOp_kConditional);

  op = cel_TernaryOp_kUnspecified;

  ASSERT_FALSE(cel_TernaryOp_FromString(
      cel_StringView_From("@unknown_ternary_operator"), &op));
}

TEST(TernaryOp, ToString) {
  EXPECT_TRUE(
      cel_StringView_Equals(cel_TernaryOp_ToString(cel_TernaryOp_kConditional),
                            cel_StringView_From("_?_:_")));

  EXPECT_TRUE(
      cel_StringView_Equals(cel_TernaryOp_ToString(cel_TernaryOp_kUnspecified),
                            cel_StringView_From("")));
}

TEST(TernaryOp, Precedence) {
  EXPECT_EQ(cel_TernaryOp_Precedence(cel_TernaryOp_kConditional), 8);

  EXPECT_EQ(cel_TernaryOp_Precedence(cel_TernaryOp_kUnspecified), 0);
}

}  // namespace
