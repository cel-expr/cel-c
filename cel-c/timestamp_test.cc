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

#include "cel-c/timestamp.h"

#include <cstdint>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"

namespace {

using ::testing::_;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::Lt;

TEST(Timestamp, UnixEpoch) {
  int64_t sec;
  int32_t nsec;

  cel_Timestamp_ToUnix(cel_Timestamp_kUnixEpoch, &sec, &nsec);
  EXPECT_EQ(sec, 0);
  EXPECT_EQ(nsec, 0);
}

TEST(Timestamp, Min) {
  int64_t sec;
  int32_t nsec;

  cel_Timestamp_ToUnix(cel_Timestamp_kMin, &sec, &nsec);
  EXPECT_EQ(sec, cel_Timestamp_kMinSeconds);
  EXPECT_EQ(nsec, cel_Timestamp_kMinNanos);
}

TEST(Timestamp, Max) {
  int64_t sec;
  int32_t nsec;

  cel_Timestamp_ToUnix(cel_Timestamp_kMax, &sec, &nsec);
  EXPECT_EQ(sec, cel_Timestamp_kMaxSeconds);
  EXPECT_EQ(nsec, cel_Timestamp_kMaxNanos);
}

TEST(Timestamp, Unix) {
  cel_Timestamp d;
  int64_t sec;
  int32_t nsec;

  d = cel_Timestamp_FromUnix(1, 1);
  cel_Timestamp_ToUnix(d, &sec, &nsec);
  EXPECT_EQ(sec, 1);
  EXPECT_EQ(nsec, 1);

  d = cel_Timestamp_FromUnix(-1, 0);
  cel_Timestamp_ToUnix(d, &sec, &nsec);
  EXPECT_EQ(sec, -1);
  EXPECT_EQ(nsec, 0);

  d = cel_Timestamp_FromUnix(0, 1);
  cel_Timestamp_ToUnix(d, &sec, &nsec);
  EXPECT_EQ(sec, 0);
  EXPECT_EQ(nsec, 1);

  d = cel_Timestamp_FromUnix(1, 0);
  cel_Timestamp_ToUnix(d, &sec, &nsec);
  EXPECT_EQ(sec, 1);
  EXPECT_EQ(nsec, 0);
}

TEST(Timestamp, Equals) {
  EXPECT_TRUE(cel_Timestamp_Equals(cel_Timestamp_FromUnix(1, 1),
                                   cel_Timestamp_FromUnix(1, 1)));
  EXPECT_FALSE(cel_Timestamp_Equals(cel_Timestamp_FromUnix(1, 1),
                                    cel_Timestamp_FromUnix(0, 0)));
  EXPECT_FALSE(cel_Timestamp_Equals(cel_Timestamp_FromUnix(0, 0),
                                    cel_Timestamp_FromUnix(1, 1)));
}

TEST(Timestamp, Compare) {
  EXPECT_THAT(cel_Timestamp_Compare(cel_Timestamp_FromUnix(1, 1),
                                    cel_Timestamp_FromUnix(1, 1)),
              Eq(0));
  EXPECT_THAT(cel_Timestamp_Compare(cel_Timestamp_FromUnix(1, 1),
                                    cel_Timestamp_FromUnix(0, 0)),
              Gt(0));
  EXPECT_THAT(cel_Timestamp_Compare(cel_Timestamp_FromUnix(0, 0),
                                    cel_Timestamp_FromUnix(1, 1)),
              Lt(0));
}

TEST(Timestamp, Add) {
  cel_Timestamp t;

  ASSERT_TRUE(cel_Timestamp_Add(&t, cel_Timestamp_FromUnix(0, 0),
                                cel_Duration_FromUnix(1, 0)));
  EXPECT_TRUE(cel_Timestamp_Equals(t, cel_Timestamp_FromUnix(1, 0)));

  ASSERT_TRUE(cel_Timestamp_Add(&t, cel_Timestamp_FromUnix(1, 0),
                                cel_Duration_FromUnix(0, 0)));
  EXPECT_TRUE(cel_Timestamp_Equals(t, cel_Timestamp_FromUnix(1, 0)));

  ASSERT_TRUE(cel_Timestamp_Add(&t, cel_Timestamp_FromUnix(0, 0),
                                cel_Duration_FromUnix(0, 1)));
  EXPECT_TRUE(cel_Timestamp_Equals(t, cel_Timestamp_FromUnix(0, 1)));

  ASSERT_TRUE(cel_Timestamp_Add(&t, cel_Timestamp_FromUnix(0, 1),
                                cel_Duration_FromUnix(0, 0)));
  EXPECT_TRUE(cel_Timestamp_Equals(t, cel_Timestamp_FromUnix(0, 1)));

  ASSERT_TRUE(cel_Timestamp_Add(&t, cel_Timestamp_FromUnix(2, 1000),
                                cel_Duration_FromUnix(-4, 0)));
  EXPECT_TRUE(cel_Timestamp_Equals(t, cel_Timestamp_FromUnix(-2, 1000)));

  ASSERT_TRUE(cel_Timestamp_Add(&t, cel_Timestamp_FromUnix(-4, 0),
                                cel_Duration_FromUnix(2, 1000)));
  EXPECT_TRUE(cel_Timestamp_Equals(t, cel_Timestamp_FromUnix(-2, 1000)));

  EXPECT_FALSE(
      cel_Timestamp_Add(&t,
                        cel_Timestamp_FromUnix(cel_Timestamp_kMaxSeconds,
                                               cel_Timestamp_kMaxNanos),
                        cel_Duration_FromUnix(0, 1)));
  EXPECT_FALSE(
      cel_Timestamp_Add(&t, cel_Timestamp_FromUnix(0, 1),
                        cel_Duration_FromUnix(cel_Timestamp_kMaxSeconds,
                                              cel_Timestamp_kMaxNanos)));
  EXPECT_FALSE(
      cel_Timestamp_Add(&t,
                        cel_Timestamp_FromUnix(cel_Timestamp_kMinSeconds,
                                               cel_Timestamp_kMinNanos),
                        cel_Duration_FromUnix(0, -1)));
}

TEST(Timestamp, Sub) {
  cel_Timestamp t;

  ASSERT_TRUE(cel_Timestamp_Sub(&t, cel_Timestamp_FromUnix(1, 0),
                                cel_Duration_FromUnix(0, 0)));
  EXPECT_TRUE(cel_Timestamp_Equals(t, cel_Timestamp_FromUnix(1, 0)));

  ASSERT_TRUE(cel_Timestamp_Sub(&t, cel_Timestamp_FromUnix(0, 1),
                                cel_Duration_FromUnix(0, 0)));
  EXPECT_TRUE(cel_Timestamp_Equals(t, cel_Timestamp_FromUnix(0, 1)));

  ASSERT_TRUE(cel_Timestamp_Sub(&t, cel_Timestamp_FromUnix(2, 1000),
                                cel_Duration_FromUnix(-4, 0)));
  EXPECT_TRUE(cel_Timestamp_Equals(t, cel_Timestamp_FromUnix(6, 1000)));

  ASSERT_TRUE(cel_Timestamp_Sub(&t, cel_Timestamp_FromUnix(-4, 0),
                                cel_Duration_FromUnix(2, 1000)));
  EXPECT_TRUE(cel_Timestamp_Equals(t, cel_Timestamp_FromUnix(-7, 999999000)));

  EXPECT_FALSE(
      cel_Timestamp_Sub(&t,
                        cel_Timestamp_FromUnix(cel_Timestamp_kMaxSeconds,
                                               cel_Timestamp_kMaxNanos),
                        cel_Duration_FromUnix(0, -1)));
  EXPECT_FALSE(
      cel_Timestamp_Sub(&t,
                        cel_Timestamp_FromUnix(cel_Timestamp_kMinSeconds,
                                               cel_Timestamp_kMinNanos),
                        cel_Duration_FromUnix(0, 1)));
}

TEST(Timestamp, Diff) {
  cel_Duration d;

  ASSERT_TRUE(
      cel_Timestamp_Diff(&d,
                         cel_Timestamp_FromUnix(cel_Timestamp_kMaxSeconds,
                                                cel_Timestamp_kMaxNanos),
                         cel_Timestamp_FromUnix(cel_Timestamp_kMaxSeconds,
                                                cel_Timestamp_kMaxNanos)));
  EXPECT_TRUE(cel_Duration_Equals(d, cel_Duration_FromUnix(0, 0)));

  ASSERT_TRUE(
      cel_Timestamp_Diff(&d, cel_Timestamp_FromUnix(0, 0),
                         cel_Timestamp_FromUnix(cel_Timestamp_kMaxSeconds,
                                                cel_Timestamp_kMinNanos)));
  EXPECT_TRUE(
      cel_Duration_Equals(d, cel_Duration_FromUnix(-cel_Timestamp_kMaxSeconds,
                                                   cel_Timestamp_kMinNanos)));
}

TEST(TimestampDeath, Unix) {
  EXPECT_DEBUG_DEATH(
      CEL_USED(cel_Timestamp_FromUnix(cel_Timestamp_kMinSeconds - 1, 0)), _);
  EXPECT_DEBUG_DEATH(
      CEL_USED(cel_Timestamp_FromUnix(cel_Timestamp_kMaxSeconds + 1, 0)), _);
  EXPECT_DEBUG_DEATH(
      CEL_USED(cel_Timestamp_FromUnix(0, cel_Timestamp_kMinNanos - 1)), _);
  EXPECT_DEBUG_DEATH(
      CEL_USED(cel_Timestamp_FromUnix(0, cel_Timestamp_kMaxNanos + 1)), _);
  EXPECT_DEBUG_DEATH(CEL_USED(cel_Timestamp_FromUnix(0, -1)), _);
}

}  // namespace
