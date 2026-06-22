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

#include "cel-c/internal/align.h"

#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/internal/config.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

namespace {

using ::testing::_;

template <typename T>
class AlignTest : public ::testing::Test {};

using AlignTestTypes =
    ::testing::Types<unsigned char, unsigned short, unsigned int, unsigned long,
                     unsigned long long>;

TYPED_TEST_SUITE(AlignTest, AlignTestTypes);

TYPED_TEST(AlignTest, AlignUp) {
  EXPECT_EQ(_cel_align_up(TypeParam{0}, size_t{1}), TypeParam{0});
  EXPECT_EQ(_cel_align_up(TypeParam{0}, size_t{2}), TypeParam{0});
  EXPECT_EQ(_cel_align_up(TypeParam{3}, size_t{2}), TypeParam{4});
  EXPECT_EQ(_cel_align_up(TypeParam{1}, size_t{2}), TypeParam{2});
  EXPECT_EQ(_cel_align_up(TypeParam{1}, size_t{1}), TypeParam{1});
  EXPECT_EQ(_cel_align_up(TypeParam{1}, alignof(std::max_align_t)),
            alignof(std::max_align_t));
  EXPECT_DEBUG_DEATH(CEL_USED(_cel_align_up(TypeParam{0}, size_t{0})), _);
}

TYPED_TEST(AlignTest, AlignDown) {
  EXPECT_EQ(_cel_align_down(TypeParam{0}, size_t{1}), TypeParam{0});
  EXPECT_EQ(_cel_align_down(TypeParam{0}, size_t{2}), TypeParam{0});
  EXPECT_EQ(_cel_align_down(TypeParam{3}, size_t{2}), TypeParam{2});
  EXPECT_EQ(_cel_align_down(TypeParam{1}, size_t{1}), TypeParam{1});
  EXPECT_EQ(_cel_align_down(TypeParam{alignof(std::max_align_t)}, size_t{1}),
            TypeParam{alignof(std::max_align_t)});
  EXPECT_DEBUG_DEATH(CEL_USED(_cel_align_down(TypeParam{0}, size_t{0})), _);
}

TYPED_TEST(AlignTest, IsAligned) {
  EXPECT_TRUE(_cel_is_aligned(TypeParam{0}, size_t{1}));
  EXPECT_TRUE(_cel_is_aligned(TypeParam{0}, size_t{2}));
  EXPECT_FALSE(_cel_is_aligned(TypeParam{3}, size_t{2}));
  EXPECT_FALSE(_cel_is_aligned(TypeParam{1}, size_t{2}));
  EXPECT_TRUE(_cel_is_aligned(TypeParam{1}, size_t{1}));
  EXPECT_TRUE(_cel_is_aligned(TypeParam{128}, alignof(std::max_align_t)));
  EXPECT_DEBUG_DEATH(CEL_USED(_cel_is_aligned(TypeParam{0}, size_t{0})), _);
}

template <typename T>
class AlignPtrTest : public ::testing::Test {};

using AlignPtrTestTypes =
    ::testing::Types<void*, const void*, char*, const char*, unsigned char*,
                     const unsigned char*>;

TYPED_TEST_SUITE(AlignPtrTest, AlignPtrTestTypes);

TYPED_TEST(AlignPtrTest, AlignUp) {
  TypeParam addr = reinterpret_cast<TypeParam>(&addr);
  EXPECT_GE(_cel_align_up(addr, alignof(std::max_align_t)), addr);
  EXPECT_DEBUG_DEATH(CEL_USED(_cel_align_up(addr, size_t{0})), _);
}

TYPED_TEST(AlignPtrTest, AlignDown) {
  TypeParam addr = reinterpret_cast<TypeParam>(&addr);
  EXPECT_LE(_cel_align_down(addr, alignof(std::max_align_t)), addr);
  EXPECT_DEBUG_DEATH(CEL_USED(_cel_align_down(addr, size_t{0})), _);
}

TYPED_TEST(AlignPtrTest, IsAligned) {
  alignas(std::max_align_t) char slot[alignof(std::max_align_t)];
  TypeParam addr = reinterpret_cast<TypeParam>(&slot[0]);
  EXPECT_TRUE(_cel_is_aligned(addr, alignof(std::max_align_t)));
  EXPECT_DEBUG_DEATH(CEL_USED(_cel_is_aligned(addr, size_t{0})), _);
}

}  // namespace

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)
