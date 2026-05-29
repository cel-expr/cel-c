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

#include "cel-c/src/uint128.h"

#include <cstdint>
#include <limits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

namespace {

using ::testing::Eq;
using ::testing::Gt;
using ::testing::Lt;

template <typename T>
void TestFromTo() {
  using Limits = std::numeric_limits<T>;
  EXPECT_EQ(_cel_Uint128_To(T, _cel_Uint128_From(Limits::min())),
            Limits::min());
  EXPECT_EQ(_cel_Uint128_To(T, _cel_Uint128_From(Limits::max())),
            Limits::max());
  EXPECT_EQ(_cel_Uint128_To(T, _cel_Uint128_From(static_cast<T>(-1))),
            static_cast<T>(-1));
}

TEST(Uint128, FromTo) {
  TestFromTo<char>();
  TestFromTo<signed char>();
  TestFromTo<unsigned char>();
  TestFromTo<short>();
  TestFromTo<unsigned short>();
  TestFromTo<int>();
  TestFromTo<unsigned int>();
  TestFromTo<long>();
  TestFromTo<unsigned long>();
  TestFromTo<long long>();
  TestFromTo<unsigned long long>();
}

TEST(Uint128, Less) {
  EXPECT_FALSE(_cel_Uint128_Less(_cel_Uint128_From(0), _cel_Uint128_From(0)));
  EXPECT_FALSE(_cel_Uint128_Less(_cel_Uint128_From(1), _cel_Uint128_From(0)));
  EXPECT_FALSE(_cel_Uint128_Less(_cel_Uint128_From(-1), _cel_Uint128_From(0)));
  EXPECT_TRUE(_cel_Uint128_Less(_cel_Uint128_From(0), _cel_Uint128_From(1)));
  EXPECT_TRUE(_cel_Uint128_Less(_cel_Uint128_From(0), _cel_Uint128_From(-1)));
}

TEST(Uint128, ShiftLeft) {
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_ShiftLeft(_cel_Uint128_From(0), 0), _cel_Uint128_From(0)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_ShiftLeft(_cel_Uint128_From(1), 0), _cel_Uint128_From(1)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_ShiftLeft(_cel_Uint128_From(1), 1), _cel_Uint128_From(2)));
  EXPECT_TRUE(
      _cel_Uint128_Equals(_cel_Uint128_ShiftLeft(_cel_Uint128_From(1), 64),
                          _cel_Uint128_Make(1, 0)));
}

TEST(Uint128, ShiftRight) {
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_ShiftRight(_cel_Uint128_From(0), 0), _cel_Uint128_From(0)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_ShiftRight(_cel_Uint128_From(1), 0), _cel_Uint128_From(1)));
  EXPECT_TRUE(
      _cel_Uint128_Equals(_cel_Uint128_ShiftRight(_cel_Uint128_Make(2, 0), 1),
                          _cel_Uint128_Make(1, 0)));
  EXPECT_TRUE(
      _cel_Uint128_Equals(_cel_Uint128_ShiftRight(_cel_Uint128_Make(1, 0), 64),
                          _cel_Uint128_From(1)));
}

TEST(Uint128, Add) {
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Add(_cel_Uint128_From(0), _cel_Uint128_From(0)),
      _cel_Uint128_From(0)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Add(_cel_Uint128_From(1), _cel_Uint128_From(0)),
      _cel_Uint128_From(1)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Add(_cel_Uint128_From(0), _cel_Uint128_From(1)),
      _cel_Uint128_From(1)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Add(_cel_Uint128_Make(1, 0), _cel_Uint128_From(1)),
      _cel_Uint128_Make(1, 1)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Add(_cel_Uint128_From(1), _cel_Uint128_Make(1, 0)),
      _cel_Uint128_Make(1, 1)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Add(_cel_Uint128_Make(0, UINT64_MAX), _cel_Uint128_From(1)),
      _cel_Uint128_Make(1, 0)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Add(_cel_Uint128_From(1), _cel_Uint128_Make(0, UINT64_MAX)),
      _cel_Uint128_Make(1, 0)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Add(_cel_Uint128_Make(UINT64_MAX, UINT64_MAX),
                       _cel_Uint128_From(1)),
      _cel_Uint128_From(0)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Add(_cel_Uint128_From(1),
                       _cel_Uint128_Make(UINT64_MAX, UINT64_MAX)),
      _cel_Uint128_From(0)));
}

TEST(Uint128, Mul) {
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Mul(_cel_Uint128_From(0), _cel_Uint128_From(0)),
      _cel_Uint128_From(0)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Mul(_cel_Uint128_From(1), _cel_Uint128_From(0)),
      _cel_Uint128_From(0)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Mul(_cel_Uint128_From(0), _cel_Uint128_From(1)),
      _cel_Uint128_From(0)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Mul(_cel_Uint128_Make(1, 0), _cel_Uint128_From(1)),
      _cel_Uint128_Make(1, 0)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Mul(_cel_Uint128_From(1), _cel_Uint128_Make(1, 0)),
      _cel_Uint128_Make(1, 0)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Mul(_cel_Uint128_Make(0, UINT64_MAX), _cel_Uint128_From(2)),
      _cel_Uint128_ShiftLeft(_cel_Uint128_Make(0, UINT64_MAX), 1)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Mul(_cel_Uint128_From(2), _cel_Uint128_Make(0, UINT64_MAX)),
      _cel_Uint128_ShiftLeft(_cel_Uint128_Make(0, UINT64_MAX), 1)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Mul(_cel_Uint128_Make(UINT64_C(1) << 63, 0),
                       _cel_Uint128_From(2)),
      _cel_Uint128_From(0)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Mul(_cel_Uint128_From(2),
                       _cel_Uint128_Make(UINT64_C(1) << 63, 0)),
      _cel_Uint128_From(0)));
}

TEST(Uint128, Or) {
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Or(_cel_Uint128_Make(0, 1), _cel_Uint128_Make(1, 0)),
      _cel_Uint128_Make(1, 1)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Or(_cel_Uint128_Make(1, 0), _cel_Uint128_Make(0, 1)),
      _cel_Uint128_Make(1, 1)));
}

TEST(Uint128, Xor) {
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Xor(_cel_Uint128_Make(1, 1), _cel_Uint128_Make(1, 0)),
      _cel_Uint128_Make(0, 1)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Xor(_cel_Uint128_Make(1, 0), _cel_Uint128_Make(1, 1)),
      _cel_Uint128_Make(0, 1)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Xor(_cel_Uint128_Make(1, 1), _cel_Uint128_Make(0, 1)),
      _cel_Uint128_Make(1, 0)));
  EXPECT_TRUE(_cel_Uint128_Equals(
      _cel_Uint128_Xor(_cel_Uint128_Make(0, 1), _cel_Uint128_Make(1, 1)),
      _cel_Uint128_Make(1, 0)));
}

TEST(Uint128, Equals) {
  EXPECT_TRUE(
      _cel_Uint128_Equals(_cel_Uint128_Make(0, 0), _cel_Uint128_Make(0, 0)));
  EXPECT_TRUE(
      _cel_Uint128_Equals(_cel_Uint128_Make(1, 0), _cel_Uint128_Make(1, 0)));
  EXPECT_TRUE(
      _cel_Uint128_Equals(_cel_Uint128_Make(0, 1), _cel_Uint128_Make(0, 1)));
  EXPECT_TRUE(
      _cel_Uint128_Equals(_cel_Uint128_Make(1, 1), _cel_Uint128_Make(1, 1)));
  EXPECT_FALSE(
      _cel_Uint128_Equals(_cel_Uint128_Make(1, 0), _cel_Uint128_Make(0, 0)));
  EXPECT_FALSE(
      _cel_Uint128_Equals(_cel_Uint128_Make(0, 0), _cel_Uint128_Make(1, 0)));
  EXPECT_FALSE(
      _cel_Uint128_Equals(_cel_Uint128_Make(0, 1), _cel_Uint128_Make(0, 0)));
  EXPECT_FALSE(
      _cel_Uint128_Equals(_cel_Uint128_Make(0, 0), _cel_Uint128_Make(0, 1)));
  EXPECT_FALSE(
      _cel_Uint128_Equals(_cel_Uint128_Make(1, 0), _cel_Uint128_Make(0, 1)));
  EXPECT_FALSE(
      _cel_Uint128_Equals(_cel_Uint128_Make(0, 1), _cel_Uint128_Make(1, 0)));
}

TEST(Uint128, Compare) {
  EXPECT_THAT(
      _cel_Uint128_Compare(_cel_Uint128_Make(0, 0), _cel_Uint128_Make(0, 0)),
      Eq(0));
  EXPECT_THAT(
      _cel_Uint128_Compare(_cel_Uint128_Make(0, 1), _cel_Uint128_Make(0, 1)),
      Eq(0));
  EXPECT_THAT(
      _cel_Uint128_Compare(_cel_Uint128_Make(1, 0), _cel_Uint128_Make(1, 0)),
      Eq(0));
  EXPECT_THAT(
      _cel_Uint128_Compare(_cel_Uint128_Make(1, 1), _cel_Uint128_Make(1, 1)),
      Eq(0));
  EXPECT_THAT(
      _cel_Uint128_Compare(_cel_Uint128_Make(0, 1), _cel_Uint128_Make(0, 0)),
      Gt(0));
  EXPECT_THAT(
      _cel_Uint128_Compare(_cel_Uint128_Make(0, 0), _cel_Uint128_Make(0, 1)),
      Lt(0));
  EXPECT_THAT(
      _cel_Uint128_Compare(_cel_Uint128_Make(1, 0), _cel_Uint128_Make(0, 0)),
      Gt(0));
  EXPECT_THAT(
      _cel_Uint128_Compare(_cel_Uint128_Make(0, 0), _cel_Uint128_Make(1, 0)),
      Lt(0));
  EXPECT_THAT(
      _cel_Uint128_Compare(_cel_Uint128_Make(1, 1), _cel_Uint128_Make(0, 1)),
      Gt(0));
  EXPECT_THAT(
      _cel_Uint128_Compare(_cel_Uint128_Make(0, 1), _cel_Uint128_Make(1, 1)),
      Lt(0));
  EXPECT_THAT(
      _cel_Uint128_Compare(_cel_Uint128_Make(1, 1), _cel_Uint128_Make(1, 0)),
      Gt(0));
  EXPECT_THAT(
      _cel_Uint128_Compare(_cel_Uint128_Make(1, 0), _cel_Uint128_Make(1, 1)),
      Lt(0));
}

}  // namespace

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)
