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

#include "cel-c/duration_proto.h"

#include "google/protobuf/duration.upb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/cleanup/cleanup.h"
#include "cel-c/duration.h"
#include "upb/mem/arena.h"

namespace {

using ::testing::NotNull;

TEST(Duration, ToProto) {
  upb_Arena* arena = upb_Arena_New();
  ASSERT_THAT(arena, NotNull());

  auto arena_cleanup =
      absl::MakeCleanup([&]() -> void { upb_Arena_Free(arena); });

  google_protobuf_Duration* duration_proto =
      google_protobuf_Duration_new(arena);
  ASSERT_THAT(duration_proto, NotNull());

  cel_Duration_ToProto(cel_Duration_FromUnix(1, 2), duration_proto);
  EXPECT_EQ(google_protobuf_Duration_seconds(duration_proto), 1);
  EXPECT_EQ(google_protobuf_Duration_nanos(duration_proto), 2);
}

TEST(Duration, FromProto) {
  upb_Arena* arena = upb_Arena_New();
  ASSERT_THAT(arena, NotNull());

  auto arena_cleanup =
      absl::MakeCleanup([&]() -> void { upb_Arena_Free(arena); });

  google_protobuf_Duration* duration_proto =
      google_protobuf_Duration_new(arena);
  ASSERT_THAT(duration_proto, NotNull());
  google_protobuf_Duration_set_seconds(duration_proto, 1);
  google_protobuf_Duration_set_nanos(duration_proto, 2);

  cel_Duration duration;
  ASSERT_TRUE(cel_Duration_FromProto(&duration, duration_proto));

  EXPECT_TRUE(cel_Duration_Equals(cel_Duration_FromUnix(1, 2), duration));
}

}  // namespace
