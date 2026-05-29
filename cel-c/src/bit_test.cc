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

#include "cel-c/src/bit.h"

#include <limits>

#include "gtest/gtest.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

namespace {

template <typename T>
class BitTest : public ::testing::Test {};

using BitTestTypes =
    ::testing::Types<unsigned char, unsigned short, unsigned int, unsigned long,
                     unsigned long long>;

TYPED_TEST_SUITE(BitTest, BitTestTypes);

TYPED_TEST(BitTest, CountLeadingZeros) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  EXPECT_EQ(_cel_leading_zeros(TypeParam{0}), TypeParamLimits::digits);
  EXPECT_EQ(_cel_leading_zeros(TypeParam{1 << 0}), TypeParamLimits::digits - 1);
  EXPECT_EQ(_cel_leading_zeros(TypeParam{1 << 1}), TypeParamLimits::digits - 2);
  EXPECT_EQ(_cel_leading_zeros(
                TypeParam{TypeParam{1} << (TypeParamLimits::digits - 1)}),
            0);
  EXPECT_EQ(_cel_leading_zeros(
                TypeParam{TypeParam{1} << (TypeParamLimits::digits - 2)}),
            1);
}

TYPED_TEST(BitTest, CountTrailingZeros) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  EXPECT_EQ(_cel_trailing_zeros(TypeParam{0}), TypeParamLimits::digits);
  EXPECT_EQ(_cel_trailing_zeros(TypeParam{1 << 0}), 0);
  EXPECT_EQ(_cel_trailing_zeros(TypeParam{1 << 1}), 1);
  EXPECT_EQ(_cel_trailing_zeros(
                TypeParam{TypeParam{1} << (TypeParamLimits::digits - 1)}),
            TypeParamLimits::digits - 1);
  EXPECT_EQ(_cel_trailing_zeros(
                TypeParam{TypeParam{1} << (TypeParamLimits::digits - 2)}),
            TypeParamLimits::digits - 2);
}

TYPED_TEST(BitTest, BitWidth) {
  EXPECT_EQ(_cel_bit_width(TypeParam{0}), 0);
  EXPECT_EQ(_cel_bit_width(TypeParam{1}), 1);
  EXPECT_EQ(_cel_bit_width(TypeParam{2}), 2);
  EXPECT_EQ(_cel_bit_width(TypeParam{3}), 2);
}

TYPED_TEST(BitTest, BitCeil) {
  EXPECT_EQ(_cel_bit_ceil(TypeParam{0}), 1);
  EXPECT_EQ(_cel_bit_ceil(TypeParam{1}), 1);
  EXPECT_EQ(_cel_bit_ceil(TypeParam{2}), 2);
  EXPECT_EQ(_cel_bit_ceil(TypeParam{3}), 4);
}

TYPED_TEST(BitTest, HasSingleBit) {
  EXPECT_FALSE(_cel_has_single_bit(TypeParam{0}));
  EXPECT_FALSE(_cel_has_single_bit(TypeParam{3}));
  EXPECT_TRUE(_cel_has_single_bit(TypeParam{1 << 0}));
  EXPECT_TRUE(_cel_has_single_bit(TypeParam{1 << 1}));
}

TYPED_TEST(BitTest, RotateRight) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  const TypeParam bit = TypeParam{1};
  EXPECT_EQ(_cel_rotr(bit, 0), bit);
  EXPECT_EQ(_cel_rotr(bit, 1),
            TypeParam{TypeParam{1} << (TypeParamLimits::digits - 1)});
  EXPECT_EQ(_cel_rotr(bit, TypeParamLimits::digits + 1),
            TypeParam{TypeParam{1} << (TypeParamLimits::digits - 1)});
  EXPECT_EQ(_cel_rotr(bit, -1), TypeParam{2});
  EXPECT_EQ(_cel_rotr(bit, -TypeParamLimits::digits + -1), TypeParam{2});
}

TYPED_TEST(BitTest, RotateLeft) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  const TypeParam bit =
      TypeParam{TypeParam{1} << (TypeParamLimits::digits - 1)};
  EXPECT_EQ(_cel_rotl(bit, 0), bit);
  EXPECT_EQ(_cel_rotl(bit, 1), TypeParam{1});
  EXPECT_EQ(_cel_rotl(bit, TypeParamLimits::digits + 1), TypeParam{1});
  EXPECT_EQ(_cel_rotl(bit, -1),
            TypeParam{TypeParam{1} << (TypeParamLimits::digits - 2)});
  EXPECT_EQ(_cel_rotl(bit, -TypeParamLimits::digits + -1),
            TypeParam{TypeParam{1} << (TypeParamLimits::digits - 2)});
}

TYPED_TEST(BitTest, FindFirstSet) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  EXPECT_EQ(_cel_ffs(TypeParam{0}), 0);
  EXPECT_EQ(_cel_ffs(TypeParam{1 << 0}), 1);
  EXPECT_EQ(_cel_ffs(TypeParam{1 << 1}), 2);
  EXPECT_EQ(_cel_ffs(TypeParam{TypeParam{1} << (TypeParamLimits::digits - 1)}),
            TypeParamLimits::digits);
  EXPECT_EQ(_cel_ffs(TypeParam{TypeParam{1} << (TypeParamLimits::digits - 2)}),
            TypeParamLimits::digits - 1);
}

}  // namespace

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)
