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

#include "cel-c/timestamp_proto.h"

#include "google/protobuf/timestamp.upb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/cleanup/cleanup.h"
#include "cel-c/timestamp.h"
#include "upb/mem/arena.h"

namespace {

using ::testing::NotNull;

TEST(Timestamp, ToProto) {
  upb_Arena* arena = upb_Arena_New();
  ASSERT_THAT(arena, NotNull());

  auto arena_cleanup =
      absl::MakeCleanup([&]() -> void { upb_Arena_Free(arena); });

  google_protobuf_Timestamp* timestamp_proto =
      google_protobuf_Timestamp_new(arena);
  ASSERT_THAT(timestamp_proto, NotNull());

  cel_Timestamp_ToProto(cel_Timestamp_FromUnix(1, 2), timestamp_proto);
  EXPECT_EQ(google_protobuf_Timestamp_seconds(timestamp_proto), 1);
  EXPECT_EQ(google_protobuf_Timestamp_nanos(timestamp_proto), 2);
}

TEST(Timestamp, FromProto) {
  upb_Arena* arena = upb_Arena_New();
  ASSERT_THAT(arena, NotNull());

  auto arena_cleanup =
      absl::MakeCleanup([&]() -> void { upb_Arena_Free(arena); });

  google_protobuf_Timestamp* timestamp_proto =
      google_protobuf_Timestamp_new(arena);
  ASSERT_THAT(timestamp_proto, NotNull());
  google_protobuf_Timestamp_set_seconds(timestamp_proto, 1);
  google_protobuf_Timestamp_set_nanos(timestamp_proto, 2);

  cel_Timestamp timestamp;
  ASSERT_TRUE(cel_Timestamp_FromProto(&timestamp, timestamp_proto));

  EXPECT_TRUE(cel_Timestamp_Equals(cel_Timestamp_FromUnix(1, 2), timestamp));
}

}  // namespace
