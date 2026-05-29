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

#include "cel-c/timestamp_absl.h"

#include "gtest/gtest.h"
#include "absl/time/time.h"
#include "cel-c/timestamp.h"

namespace {

TEST(Timestamp, FromAbsl) {
  EXPECT_TRUE(cel_Timestamp_Equals(
      cel_Timestamp_FromAbsl(absl::InfiniteFuture()), cel_Timestamp_kMax));
  EXPECT_TRUE(cel_Timestamp_Equals(cel_Timestamp_FromAbsl(absl::InfinitePast()),
                                   cel_Timestamp_kMin));
  EXPECT_TRUE(cel_Timestamp_Equals(cel_Timestamp_FromAbsl(absl::UnixEpoch()),
                                   cel_Timestamp_kUnixEpoch));
}

TEST(Timestamp, ToAbsl) {
  EXPECT_EQ(cel_Timestamp_ToAbsl(cel_Timestamp_kMax),
            absl::UnixEpoch() + absl::Seconds(cel_Timestamp_kMaxSeconds) +
                absl::Nanoseconds(cel_Timestamp_kMaxNanos));
  EXPECT_EQ(cel_Timestamp_ToAbsl(cel_Timestamp_kMin),
            absl::UnixEpoch() + absl::Seconds(cel_Timestamp_kMinSeconds) +
                absl::Nanoseconds(cel_Timestamp_kMinNanos));
  EXPECT_EQ(cel_Timestamp_ToAbsl(cel_Timestamp_kUnixEpoch), absl::UnixEpoch());
}

}  // namespace
