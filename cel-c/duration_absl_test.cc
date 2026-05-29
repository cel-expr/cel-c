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

#include "cel-c/duration_absl.h"

#include "gtest/gtest.h"
#include "absl/time/time.h"
#include "cel-c/duration.h"

namespace {

TEST(Duration, FromAbsl) {
  EXPECT_TRUE(cel_Duration_Equals(
      cel_Duration_FromAbsl(absl::InfiniteDuration()), cel_Duration_kMax));
  EXPECT_TRUE(cel_Duration_Equals(
      cel_Duration_FromAbsl(-absl::InfiniteDuration()), cel_Duration_kMin));
  EXPECT_TRUE(cel_Duration_Equals(cel_Duration_FromAbsl(absl::ZeroDuration()),
                                  cel_Duration_kZero));
}

TEST(Duration, ToAbsl) {
  EXPECT_EQ(cel_Duration_ToAbsl(cel_Duration_kMax),
            absl::Seconds(cel_Duration_kMaxSeconds) +
                absl::Nanoseconds(cel_Duration_kMaxNanos));
  EXPECT_EQ(cel_Duration_ToAbsl(cel_Duration_kMin),
            absl::Seconds(cel_Duration_kMinSeconds) +
                absl::Nanoseconds(cel_Duration_kMinNanos));
  EXPECT_EQ(cel_Duration_ToAbsl(cel_Duration_kZero), absl::ZeroDuration());
}

}  // namespace
