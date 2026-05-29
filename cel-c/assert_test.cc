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

#include "cel-c/assert.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/config.h"

namespace {

using ::testing::_;

TEST(Assert, Death) {
  EXPECT_DEATH_IF_SUPPORTED(
      cel_AssertionFailed(__FILE__, __LINE__, "EXPECT_DEATH_IF_SUPPORTED"), _);
}

TEST(Assert, True) { EXPECT_DEBUG_DEATH(CEL_ASSERT(false), _); }

TEST(Assert, False) { EXPECT_DEBUG_DEATH(CEL_ASSERT_NOT(true), _); }

TEST(Assert, Null) {
  EXPECT_DEBUG_DEATH(CEL_ASSERT_NULL(&cel_AssertionFailed), _);
}

TEST(Assert, NotNull) {
  EXPECT_DEBUG_DEATH(CEL_ASSERT_NOT_NULL(cel_nullptr), _);
}

TEST(Assert, EqualTo) { EXPECT_DEBUG_DEATH(CEL_ASSERT_EQ(0, 1), _); }

TEST(Assert, NotEqualTo) { EXPECT_DEBUG_DEATH(CEL_ASSERT_NE(0, 0), _); }

TEST(Assert, LessThan) { EXPECT_DEBUG_DEATH(CEL_ASSERT_LT(1, 0), _); }

TEST(Assert, LessThanEqualTo) { EXPECT_DEBUG_DEATH(CEL_ASSERT_LE(1, 0), _); }

TEST(Assert, GreaterThan) { EXPECT_DEBUG_DEATH(CEL_ASSERT_GT(0, 1), _); }

TEST(Assert, GreaterThanEqualTo) { EXPECT_DEBUG_DEATH(CEL_ASSERT_GE(0, 1), _); }

}  // namespace
