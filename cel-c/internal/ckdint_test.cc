// Copyright 2024 Google LLC
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

#include "cel-c/internal/ckdint.h"

#include <limits>

#include "gtest/gtest.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

namespace {

template <typename T>
class CheckedTest : public ::testing::Test {};

using CheckedTestTypes =
    ::testing::Types<signed char, unsigned char, short, unsigned short, int,
                     unsigned int, long, unsigned long, long long,
                     unsigned long long>;

TYPED_TEST_SUITE(CheckedTest, CheckedTestTypes);

TYPED_TEST(CheckedTest, OneAddOne) {
  TypeParam out;
  EXPECT_FALSE(_cel_ckd_add(&out, TypeParam{1}, TypeParam{1}));
  EXPECT_EQ(out, TypeParam{2});
}

TYPED_TEST(CheckedTest, ZeroAddOne) {
  TypeParam out;
  EXPECT_FALSE(_cel_ckd_add(&out, TypeParam{0}, TypeParam{1}));
  EXPECT_EQ(out, TypeParam{1});
}

TYPED_TEST(CheckedTest, OneAddZero) {
  TypeParam out;
  EXPECT_FALSE(_cel_ckd_add(&out, TypeParam{1}, TypeParam{0}));
  EXPECT_EQ(out, TypeParam{1});
}

TYPED_TEST(CheckedTest, MaxAddOne) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  TypeParam out;
  EXPECT_TRUE(_cel_ckd_add(&out, TypeParamLimits::max(), TypeParam{1}));
}

TYPED_TEST(CheckedTest, OneAddMax) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  TypeParam out;
  EXPECT_TRUE(_cel_ckd_add(&out, TypeParam{1}, TypeParamLimits::max()));
}

TYPED_TEST(CheckedTest, ZeroAddMax) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  TypeParam out;
  EXPECT_FALSE(_cel_ckd_add(&out, TypeParam{0}, TypeParamLimits::max()));
  EXPECT_EQ(out, TypeParamLimits::max());
}

TYPED_TEST(CheckedTest, MaxAddZero) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  TypeParam out;
  EXPECT_FALSE(_cel_ckd_add(&out, TypeParamLimits::max(), TypeParam{0}));
  EXPECT_EQ(out, TypeParamLimits::max());
}

TYPED_TEST(CheckedTest, MinAddMax) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  TypeParam out;
  EXPECT_FALSE(
      _cel_ckd_add(&out, TypeParamLimits::min(), TypeParamLimits::max()));
  EXPECT_EQ(out, static_cast<TypeParam>(-1));
}

TYPED_TEST(CheckedTest, MaxAddMin) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  TypeParam out;
  EXPECT_FALSE(
      _cel_ckd_add(&out, TypeParamLimits::max(), TypeParamLimits::min()));
  EXPECT_EQ(out, static_cast<TypeParam>(-1));
}

TYPED_TEST(CheckedTest, OneSubOne) {
  TypeParam out;
  EXPECT_FALSE(_cel_ckd_sub(&out, TypeParam{1}, TypeParam{1}));
  EXPECT_EQ(out, TypeParam{0});
}

TYPED_TEST(CheckedTest, OneSubZero) {
  TypeParam out;
  EXPECT_FALSE(_cel_ckd_sub(&out, TypeParam{1}, TypeParam{0}));
  EXPECT_EQ(out, TypeParam{1});
}

TYPED_TEST(CheckedTest, MinSubOne) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  TypeParam out;
  EXPECT_TRUE(_cel_ckd_sub(&out, TypeParamLimits::min(), TypeParam{1}));
}

TYPED_TEST(CheckedTest, MinSubZero) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  TypeParam out;
  EXPECT_FALSE(_cel_ckd_sub(&out, TypeParamLimits::min(), TypeParam{0}));
  EXPECT_EQ(out, TypeParamLimits::min());
}

TYPED_TEST(CheckedTest, MaxSubOne) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  TypeParam out;
  EXPECT_FALSE(_cel_ckd_sub(&out, TypeParamLimits::max(), TypeParam{1}));
  EXPECT_EQ(out, TypeParamLimits::max() - 1);
}

TYPED_TEST(CheckedTest, MaxSubZero) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  TypeParam out;
  EXPECT_FALSE(_cel_ckd_sub(&out, TypeParamLimits::max(), TypeParam{0}));
  EXPECT_EQ(out, TypeParamLimits::max());
}

TYPED_TEST(CheckedTest, ZeroMulZero) {
  TypeParam out;
  EXPECT_FALSE(_cel_ckd_mul(&out, TypeParam{0}, TypeParam{0}));
  EXPECT_EQ(out, TypeParam{0});
}

TYPED_TEST(CheckedTest, ZeroMulOne) {
  TypeParam out;
  EXPECT_FALSE(_cel_ckd_mul(&out, TypeParam{0}, TypeParam{1}));
  EXPECT_EQ(out, TypeParam{0});
}

TYPED_TEST(CheckedTest, OneMulZero) {
  TypeParam out;
  EXPECT_FALSE(_cel_ckd_mul(&out, TypeParam{1}, TypeParam{0}));
  EXPECT_EQ(out, TypeParam{0});
}

TYPED_TEST(CheckedTest, OneMulOne) {
  TypeParam out;
  EXPECT_FALSE(_cel_ckd_mul(&out, TypeParam{1}, TypeParam{1}));
  EXPECT_EQ(out, TypeParam{1});
}

TYPED_TEST(CheckedTest, MaxMulTwo) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  TypeParam out;
  EXPECT_TRUE(_cel_ckd_mul(&out, TypeParamLimits::max(), TypeParam{2}));
}

TYPED_TEST(CheckedTest, MinMulMinusOne) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  if constexpr (std::is_signed_v<TypeParam>) {
    TypeParam out;
    EXPECT_TRUE(
        _cel_ckd_mul(&out, TypeParamLimits::min(), static_cast<TypeParam>(-1)));
  } else {
    TypeParam out;
    EXPECT_FALSE(
        _cel_ckd_mul(&out, TypeParamLimits::min(), static_cast<TypeParam>(-1)));
    EXPECT_EQ(out, TypeParam{0});
  }
}

TYPED_TEST(CheckedTest, MaxMulMinusOne) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  if constexpr (std::is_signed_v<TypeParam>) {
    TypeParam out;
    EXPECT_FALSE(
        _cel_ckd_mul(&out, TypeParamLimits::max(), static_cast<TypeParam>(-1)));
    EXPECT_EQ(out, TypeParamLimits::min() + 1);
  } else {
    TypeParam out;
    EXPECT_TRUE(
        _cel_ckd_mul(&out, TypeParamLimits::max(), static_cast<TypeParam>(-1)));
  }
}

}  // namespace

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)
