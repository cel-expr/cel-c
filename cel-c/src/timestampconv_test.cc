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

#include "cel-c/src/timestampconv.h"

#include <cstdint>

#include "gtest/gtest.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"

namespace {

class CelTimestampConvTest : public ::testing::Test {
 protected:
  void SetUp() override { arena_ = cel_Arena_New(cel_DefaultAllocator); }
  void TearDown() override { cel_Arena_Delete(arena_); }

  cel_Arena* arena() { return arena_; }

 private:
  cel_Arena* arena_;
};

TEST_F(CelTimestampConvTest, ToRFC3339WithNanos) {
  // 2023-03-15T13:20:00.500Z
  cel_Timestamp ts = cel_Timestamp_FromUnix(1678886400, 500000000);
  cel_StringView result;
  ASSERT_TRUE(_cel_Timestamp_ToRFC3339(ts, &result, arena()));

  cel_StringView expected = cel_StringView_From("2023-03-15T13:20:00.5Z");
  EXPECT_EQ(result, expected);
}

TEST_F(CelTimestampConvTest, ToRFC3339WithoutFractional) {
  // 2023-03-15T13:20:00Z
  cel_Timestamp ts = cel_Timestamp_FromUnix(1678886400, 0);
  cel_StringView result;
  ASSERT_TRUE(_cel_Timestamp_ToRFC3339(ts, &result, arena()));

  cel_StringView expected = cel_StringView_From("2023-03-15T13:20:00Z");
  EXPECT_EQ(result, expected);
}

TEST_F(CelTimestampConvTest, ToRFC3339Micros) {
  // 2023-03-15T13:20:00.000500Z
  cel_Timestamp ts = cel_Timestamp_FromUnix(1678886400, 500000);
  cel_StringView result;
  ASSERT_TRUE(_cel_Timestamp_ToRFC3339(ts, &result, arena()));

  cel_StringView expected = cel_StringView_From("2023-03-15T13:20:00.0005Z");
  EXPECT_EQ(result, expected);
}

TEST_F(CelTimestampConvTest, ToRFC3339FullNanos) {
  // 2023-03-15T13:20:00.123456789Z
  cel_Timestamp ts = cel_Timestamp_FromUnix(1678886400, 123456789);
  cel_StringView result;
  ASSERT_TRUE(_cel_Timestamp_ToRFC3339(ts, &result, arena()));

  cel_StringView expected =
      cel_StringView_From("2023-03-15T13:20:00.123456789Z");
  EXPECT_EQ(result, expected);
}

TEST_F(CelTimestampConvTest, ToRFC3339Epoch) {
  // 1970-01-01T00:00:00Z
  cel_Timestamp ts = cel_Timestamp_FromUnix(0, 0);
  cel_StringView result;
  ASSERT_TRUE(_cel_Timestamp_ToRFC3339(ts, &result, arena()));

  cel_StringView expected = cel_StringView_From("1970-01-01T00:00:00Z");
  EXPECT_EQ(result, expected);
}

TEST_F(CelTimestampConvTest, ToRFC3339Negative) {
  // 1969-12-31T23:59:59Z
  cel_Timestamp ts = cel_Timestamp_FromUnix(-1, 0);
  cel_StringView result;
  ASSERT_TRUE(_cel_Timestamp_ToRFC3339(ts, &result, arena()));

  cel_StringView expected = cel_StringView_From("1969-12-31T23:59:59Z");
  EXPECT_EQ(result, expected);
}

TEST_F(CelTimestampConvTest, FromRFC3339Valid) {
  cel_Timestamp ts;
  EXPECT_TRUE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00Z")));
  int64_t seconds;
  int32_t nanos;
  cel_Timestamp_ToUnix(ts, &seconds, &nanos);
  EXPECT_EQ(seconds, 1678886400);
  EXPECT_EQ(nanos, 0);

  EXPECT_TRUE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00.1Z")));
  cel_Timestamp_ToUnix(ts, &seconds, &nanos);
  EXPECT_EQ(seconds, 1678886400);
  EXPECT_EQ(nanos, 100000000);

  EXPECT_TRUE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00.123Z")));
  cel_Timestamp_ToUnix(ts, &seconds, &nanos);
  EXPECT_EQ(seconds, 1678886400);
  EXPECT_EQ(nanos, 123000000);

  EXPECT_TRUE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00.123456Z")));
  cel_Timestamp_ToUnix(ts, &seconds, &nanos);
  EXPECT_EQ(seconds, 1678886400);
  EXPECT_EQ(nanos, 123456000);

  EXPECT_TRUE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00.123456789Z")));
  cel_Timestamp_ToUnix(ts, &seconds, &nanos);
  EXPECT_EQ(seconds, 1678886400);
  EXPECT_EQ(nanos, 123456789);

  EXPECT_TRUE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("1970-01-01T00:00:00Z")));
  cel_Timestamp_ToUnix(ts, &seconds, &nanos);
  EXPECT_EQ(seconds, 0);
  EXPECT_EQ(nanos, 0);

  EXPECT_TRUE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("1969-12-31T23:59:59Z")));
  cel_Timestamp_ToUnix(ts, &seconds, &nanos);
  EXPECT_EQ(seconds, -1);
  EXPECT_EQ(nanos, 0);
}

TEST_F(CelTimestampConvTest, FromRFC3339ValidWithOffset) {
  cel_Timestamp ts;
  int64_t seconds;
  int32_t nanos;

  // 2023-03-15T13:20:00+00:00 is equivalent to Z
  EXPECT_TRUE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00+00:00")));
  cel_Timestamp_ToUnix(ts, &seconds, &nanos);
  EXPECT_EQ(seconds, 1678886400);
  EXPECT_EQ(nanos, 0);

  // 2023-03-15T13:20:00+01:00 is one hour before UTC
  EXPECT_TRUE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00+01:00")));
  cel_Timestamp_ToUnix(ts, &seconds, &nanos);
  EXPECT_EQ(seconds, 1678886400 - 3600);
  EXPECT_EQ(nanos, 0);

  // 2023-03-15T13:20:00-01:00 is one hour after UTC
  EXPECT_TRUE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00-01:00")));
  cel_Timestamp_ToUnix(ts, &seconds, &nanos);
  EXPECT_EQ(seconds, 1678886400 + 3600);
  EXPECT_EQ(nanos, 0);

  // 2023-03-15T13:20:00-01:30 is 1 hour 30 minutes after UTC
  EXPECT_TRUE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00-01:30")));
  cel_Timestamp_ToUnix(ts, &seconds, &nanos);
  EXPECT_EQ(seconds, 1678886400 + 5400);
  EXPECT_EQ(nanos, 0);
}

TEST_F(CelTimestampConvTest, FromRFC3339Invalid) {
  cel_Timestamp ts;
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(&ts, cel_StringView_From("")));
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(&ts, cel_StringView_From("invalid")));
  EXPECT_FALSE(
      _cel_Timestamp_FromRFC3339(&ts, cel_StringView_From("2023-03-15")));
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00")));
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00.Z")));
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00+")));
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00+00:")));
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00+00:0")));
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00+00:000")));
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00-")));
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00-00:")));
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00-00:0")));
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00-00:000")));
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-13-15T13:20:00Z")));  // Invalid month
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-32T13:20:00Z")));  // Invalid day
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T24:20:00Z")));  // Invalid hour
  EXPECT_FALSE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:60:00Z")));  // Invalid minute
  // Leap seconds are allowed, so 60 is valid for seconds.
}

TEST_F(CelTimestampConvTest, FromRFC3339EdgeCases) {
  cel_Timestamp ts;
  int64_t seconds;
  int32_t nanos;

  // +00 and -00 are valid representations of UTC.
  EXPECT_TRUE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00+00")));
  cel_Timestamp_ToUnix(ts, &seconds, &nanos);
  EXPECT_EQ(seconds, 1678886400);
  EXPECT_EQ(nanos, 0);

  EXPECT_TRUE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:00-00")));
  cel_Timestamp_ToUnix(ts, &seconds, &nanos);
  EXPECT_EQ(seconds, 1678886400);
  EXPECT_EQ(nanos, 0);

  // Leap second is valid.
  EXPECT_TRUE(_cel_Timestamp_FromRFC3339(
      &ts, cel_StringView_From("2023-03-15T13:20:60Z")));
  cel_Timestamp_ToUnix(ts, &seconds, &nanos);
  // The exact second value for a leap second depends on when it's applied.
  // When the second is 60, it rolls over to the next minute.
  EXPECT_EQ(seconds, 1678886400 + 60);
  EXPECT_EQ(nanos, 0);
}

}  // namespace
