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

#include "cel-c/internal/durationconv.h"

#include <cstdint>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/duration.h"
#include "cel-c/string_view.h"

namespace {

using ::testing::Eq;

TEST(DurationConv, ToStringView) {
  cel_Arena* arena = cel_Arena_New(cel_DefaultAllocator);
  cel_Duration d = cel_Duration_FromUnix(60, 100000000);
  cel_StringView s;
  EXPECT_TRUE(_cel_Duration_ToStringView(d, arena, &s));
  EXPECT_THAT(s.size, Eq(7));
  EXPECT_THAT(std::string(s.data, s.size), Eq("60.100s"));
  cel_Arena_Delete(arena);
}

TEST(DurationConv, FromStringViewValid) {
  cel_Duration d;
  EXPECT_TRUE(
      _cel_Duration_FromStringView(cel_StringView_From("60.100000000s"), &d));
  int64_t seconds;
  int32_t nanos;
  cel_Duration_ToUnix(d, &seconds, &nanos);
  EXPECT_THAT(seconds, Eq(60));
  EXPECT_THAT(nanos, Eq(100000000));
}

TEST(DurationConv, ToStringViewZero) {
  cel_Arena* arena = cel_Arena_New(cel_DefaultAllocator);
  cel_Duration d = cel_Duration_FromUnix(0, 0);
  cel_StringView s;
  EXPECT_TRUE(_cel_Duration_ToStringView(d, arena, &s));
  EXPECT_THAT(s.size, Eq(2));
  EXPECT_THAT(std::string(s.data, s.size), Eq("0s"));
  cel_Arena_Delete(arena);
}

TEST(DurationConv, ToStringViewMicroseconds) {
  cel_Arena* arena = cel_Arena_New(cel_DefaultAllocator);
  cel_Duration d = cel_Duration_FromUnix(1, 123000);
  cel_StringView s;
  EXPECT_TRUE(_cel_Duration_ToStringView(d, arena, &s));
  EXPECT_THAT(s.size, Eq(9));
  EXPECT_THAT(std::string(s.data, s.size), Eq("1.000123s"));
  cel_Arena_Delete(arena);
}

TEST(DurationConv, ToStringViewNanoseconds) {
  cel_Arena* arena = cel_Arena_New(cel_DefaultAllocator);
  cel_Duration d = cel_Duration_FromUnix(1, 123);
  cel_StringView s;
  EXPECT_TRUE(_cel_Duration_ToStringView(d, arena, &s));
  EXPECT_THAT(s.size, Eq(12));
  EXPECT_THAT(std::string(s.data, s.size), Eq("1.000000123s"));
  cel_Arena_Delete(arena);
}

TEST(DurationConv, ToStringViewNegative) {
  cel_Arena* arena = cel_Arena_New(cel_DefaultAllocator);
  cel_Duration d = cel_Duration_FromUnix(-1, -123000000);
  cel_StringView s;
  EXPECT_TRUE(_cel_Duration_ToStringView(d, arena, &s));
  EXPECT_THAT(s.size, Eq(7));
  EXPECT_THAT(std::string(s.data, s.size), Eq("-1.123s"));
  cel_Arena_Delete(arena);
}

TEST(DurationConv, FromStringViewNanos) {
  cel_Duration d;
  EXPECT_TRUE(_cel_Duration_FromStringView(cel_StringView_From("1ns"), &d));
  int64_t seconds;
  int32_t nanos;
  cel_Duration_ToUnix(d, &seconds, &nanos);
  EXPECT_THAT(seconds, Eq(0));
  EXPECT_THAT(nanos, Eq(1));
}

TEST(DurationConv, FromStringViewMicros) {
  cel_Duration d;
  EXPECT_TRUE(_cel_Duration_FromStringView(cel_StringView_From("1us"), &d));
  int64_t seconds;
  int32_t nanos;
  cel_Duration_ToUnix(d, &seconds, &nanos);
  EXPECT_THAT(seconds, Eq(0));
  EXPECT_THAT(nanos, Eq(1000));
}

TEST(DurationConv, FromStringViewMillis) {
  cel_Duration d;
  EXPECT_TRUE(_cel_Duration_FromStringView(cel_StringView_From("1ms"), &d));
  int64_t seconds;
  int32_t nanos;
  cel_Duration_ToUnix(d, &seconds, &nanos);
  EXPECT_THAT(seconds, Eq(0));
  EXPECT_THAT(nanos, Eq(1000000));
}

TEST(DurationConv, FromStringViewMinutes) {
  cel_Duration d;
  EXPECT_TRUE(_cel_Duration_FromStringView(cel_StringView_From("1m"), &d));
  int64_t seconds;
  int32_t nanos;
  cel_Duration_ToUnix(d, &seconds, &nanos);
  EXPECT_THAT(seconds, Eq(60));
  EXPECT_THAT(nanos, Eq(0));
}

TEST(DurationConv, FromStringViewHours) {
  cel_Duration d;
  EXPECT_TRUE(_cel_Duration_FromStringView(cel_StringView_From("1h"), &d));
  int64_t seconds;
  int32_t nanos;
  cel_Duration_ToUnix(d, &seconds, &nanos);
  EXPECT_THAT(seconds, Eq(3600));
  EXPECT_THAT(nanos, Eq(0));
}

TEST(DurationConv, FromStringViewCompound) {
  cel_Duration d;
  EXPECT_TRUE(_cel_Duration_FromStringView(cel_StringView_From("1h30m"), &d));
  int64_t seconds;
  int32_t nanos;
  cel_Duration_ToUnix(d, &seconds, &nanos);
  EXPECT_THAT(seconds, Eq(5400));
  EXPECT_THAT(nanos, Eq(0));
}

TEST(DurationConv, FromStringViewNegativeCompound) {
  cel_Duration d;
  EXPECT_TRUE(_cel_Duration_FromStringView(cel_StringView_From("-1.5h"), &d));
  int64_t seconds;
  int32_t nanos;
  cel_Duration_ToUnix(d, &seconds, &nanos);
  EXPECT_THAT(seconds, Eq(-5400));
  EXPECT_THAT(nanos, Eq(0));
}

TEST(DurationConv, FromStringViewZero) {
  cel_Duration d;
  EXPECT_TRUE(_cel_Duration_FromStringView(cel_StringView_From("0"), &d));
  int64_t seconds;
  int32_t nanos;
  cel_Duration_ToUnix(d, &seconds, &nanos);
  EXPECT_THAT(seconds, Eq(0));
  EXPECT_THAT(nanos, Eq(0));
}

TEST(DurationConv, FromStringViewOutOfRange) {
  cel_Duration d;
  EXPECT_FALSE(
      _cel_Duration_FromStringView(cel_StringView_From("87660001h"), &d));
  EXPECT_FALSE(
      _cel_Duration_FromStringView(cel_StringView_From("-87660001h"), &d));
}

TEST(DurationConv, FromStringViewInvalid) {
  cel_Duration d;
  EXPECT_FALSE(
      _cel_Duration_FromStringView(cel_StringView_From("invalid"), &d));
}

}  // namespace
