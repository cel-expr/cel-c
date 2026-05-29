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

#include "cel-c/trilean.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/config.h"

namespace {

using ::testing::_;

TEST(Trilean, Bool) {
  EXPECT_EQ(static_cast<int>(false), static_cast<int>(cel_Trilean_kFalse));
  EXPECT_EQ(static_cast<int>(true), static_cast<int>(cel_Trilean_kTrue));
}

TEST(Trilean, FromBool) {
  EXPECT_EQ(cel_Trilean_FromBool(false), cel_Trilean_kFalse);
  EXPECT_EQ(cel_Trilean_FromBool(true), cel_Trilean_kTrue);
}

TEST(Trilean, ToBool) {
  EXPECT_FALSE(cel_Trilean_ToBool(cel_Trilean_kFalse));
  EXPECT_TRUE(cel_Trilean_ToBool(cel_Trilean_kTrue));

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Trilean_ToBool(cel_Trilean_kUnknown)),
                            _);
#endif
}

}  // namespace
