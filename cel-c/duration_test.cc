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

#include "cel-c/duration.h"

#include <cstdint>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/config.h"

namespace {

using ::testing::_;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::Lt;

TEST(Duration, Zero) {
  int64_t sec;
  int32_t nsec;

  cel_Duration_ToUnix(cel_Duration_kZero, &sec, &nsec);
  EXPECT_EQ(sec, 0);
  EXPECT_EQ(nsec, 0);
}

TEST(Duration, Min) {
  int64_t sec;
  int32_t nsec;

  cel_Duration_ToUnix(cel_Duration_kMin, &sec, &nsec);
  EXPECT_EQ(sec, cel_Duration_kMinSeconds);
  EXPECT_EQ(nsec, cel_Duration_kMinNanos);
}

TEST(Duration, Max) {
  int64_t sec;
  int32_t nsec;

  cel_Duration_ToUnix(cel_Duration_kMax, &sec, &nsec);
  EXPECT_EQ(sec, cel_Duration_kMaxSeconds);
  EXPECT_EQ(nsec, cel_Duration_kMaxNanos);
}

TEST(Duration, Unix) {
  cel_Duration d;
  int64_t sec;
  int32_t nsec;

  d = cel_Duration_FromUnix(1, 1);
  cel_Duration_ToUnix(d, &sec, &nsec);
  EXPECT_EQ(sec, 1);
  EXPECT_EQ(nsec, 1);

  d = cel_Duration_FromUnix(0, -1);
  cel_Duration_ToUnix(d, &sec, &nsec);
  EXPECT_EQ(sec, 0);
  EXPECT_EQ(nsec, -1);

  d = cel_Duration_FromUnix(-1, 0);
  cel_Duration_ToUnix(d, &sec, &nsec);
  EXPECT_EQ(sec, -1);
  EXPECT_EQ(nsec, 0);

  d = cel_Duration_FromUnix(0, 1);
  cel_Duration_ToUnix(d, &sec, &nsec);
  EXPECT_EQ(sec, 0);
  EXPECT_EQ(nsec, 1);

  d = cel_Duration_FromUnix(1, 0);
  cel_Duration_ToUnix(d, &sec, &nsec);
  EXPECT_EQ(sec, 1);
  EXPECT_EQ(nsec, 0);
}

TEST(Duration, Equals) {
  EXPECT_TRUE(cel_Duration_Equals(cel_Duration_FromUnix(1, 1),
                                  cel_Duration_FromUnix(1, 1)));
  EXPECT_FALSE(cel_Duration_Equals(cel_Duration_FromUnix(1, 1),
                                   cel_Duration_FromUnix(0, 0)));
  EXPECT_FALSE(cel_Duration_Equals(cel_Duration_FromUnix(0, 0),
                                   cel_Duration_FromUnix(1, 1)));
}

TEST(Duration, Compare) {
  EXPECT_THAT(cel_Duration_Compare(cel_Duration_FromUnix(1, 1),
                                   cel_Duration_FromUnix(1, 1)),
              Eq(0));
  EXPECT_THAT(cel_Duration_Compare(cel_Duration_FromUnix(1, 1),
                                   cel_Duration_FromUnix(0, 0)),
              Gt(0));
  EXPECT_THAT(cel_Duration_Compare(cel_Duration_FromUnix(0, 0),
                                   cel_Duration_FromUnix(1, 1)),
              Lt(0));
}

TEST(Duration, Add) {
  cel_Duration d;

  ASSERT_TRUE(cel_Duration_Add(
      &d,
      cel_Duration_FromUnix(cel_Duration_kMinSeconds, cel_Duration_kMinNanos),
      cel_Duration_FromUnix(cel_Duration_kMaxSeconds, cel_Duration_kMaxNanos)));
  EXPECT_TRUE(cel_Duration_Equals(d, cel_Duration_FromUnix(0, 0)));

  ASSERT_TRUE(cel_Duration_Add(
      &d,
      cel_Duration_FromUnix(cel_Duration_kMaxSeconds, cel_Duration_kMaxNanos),
      cel_Duration_FromUnix(cel_Duration_kMinSeconds, cel_Duration_kMinNanos)));
  EXPECT_TRUE(cel_Duration_Equals(d, cel_Duration_FromUnix(0, 0)));

  ASSERT_TRUE(
      cel_Duration_Add(&d, cel_Duration_FromUnix(1, cel_Duration_kMaxNanos),
                       cel_Duration_FromUnix(1, cel_Duration_kMaxNanos)));
  EXPECT_TRUE(cel_Duration_Equals(d, cel_Duration_FromUnix(3, 999999998)));

  ASSERT_TRUE(
      cel_Duration_Add(&d, cel_Duration_FromUnix(0, 1),
                       cel_Duration_FromUnix(0, cel_Duration_kMinNanos)));
  EXPECT_TRUE(cel_Duration_Equals(d, cel_Duration_FromUnix(0, -999999998)));

  ASSERT_TRUE(cel_Duration_Add(&d,
                               cel_Duration_FromUnix(0, cel_Duration_kMinNanos),
                               cel_Duration_FromUnix(0, 1)));
  EXPECT_TRUE(cel_Duration_Equals(d, cel_Duration_FromUnix(0, -999999998)));

  ASSERT_TRUE(cel_Duration_Add(&d, cel_Duration_FromUnix(2, 1000),
                               cel_Duration_FromUnix(-4, 0)));
  EXPECT_TRUE(cel_Duration_Equals(d, cel_Duration_FromUnix(-1, -999999000)));

  ASSERT_TRUE(cel_Duration_Add(&d, cel_Duration_FromUnix(-4, 0),
                               cel_Duration_FromUnix(2, 1000)));
  EXPECT_TRUE(cel_Duration_Equals(d, cel_Duration_FromUnix(-1, -999999000)));

  EXPECT_FALSE(cel_Duration_Add(
      &d,
      cel_Duration_FromUnix(cel_Duration_kMaxSeconds, cel_Duration_kMaxNanos),
      cel_Duration_FromUnix(0, 1)));
  EXPECT_FALSE(cel_Duration_Add(
      &d, cel_Duration_FromUnix(0, 1),
      cel_Duration_FromUnix(cel_Duration_kMaxSeconds, cel_Duration_kMaxNanos)));
  EXPECT_FALSE(cel_Duration_Add(
      &d,
      cel_Duration_FromUnix(cel_Duration_kMinSeconds, cel_Duration_kMinNanos),
      cel_Duration_FromUnix(0, -1)));
  EXPECT_FALSE(cel_Duration_Add(
      &d, cel_Duration_FromUnix(0, -1),
      cel_Duration_FromUnix(cel_Duration_kMinSeconds, cel_Duration_kMinNanos)));
}

TEST(Duration, Sub) {
  cel_Duration d;

  ASSERT_TRUE(cel_Duration_Sub(
      &d,
      cel_Duration_FromUnix(cel_Duration_kMaxSeconds, cel_Duration_kMaxNanos),
      cel_Duration_FromUnix(cel_Duration_kMaxSeconds, cel_Duration_kMaxNanos)));
  EXPECT_TRUE(cel_Duration_Equals(d, cel_Duration_FromUnix(0, 0)));

  ASSERT_TRUE(cel_Duration_Sub(
      &d, cel_Duration_FromUnix(0, 0),
      cel_Duration_FromUnix(cel_Duration_kMinSeconds, cel_Duration_kMinNanos)));
  EXPECT_TRUE(cel_Duration_Equals(
      d,
      cel_Duration_FromUnix(cel_Duration_kMaxSeconds, cel_Duration_kMaxNanos)));

  ASSERT_TRUE(cel_Duration_Sub(
      &d, cel_Duration_FromUnix(0, 0),
      cel_Duration_FromUnix(cel_Duration_kMaxSeconds, cel_Duration_kMaxNanos)));
  EXPECT_TRUE(cel_Duration_Equals(
      d,
      cel_Duration_FromUnix(cel_Duration_kMinSeconds, cel_Duration_kMinNanos)));

  ASSERT_TRUE(
      cel_Duration_Sub(&d, cel_Duration_FromUnix(0, 1),
                       cel_Duration_FromUnix(0, cel_Duration_kMinNanos)));
  EXPECT_TRUE(cel_Duration_Equals(d, cel_Duration_FromUnix(1, 0)));

  ASSERT_TRUE(cel_Duration_Sub(&d, cel_Duration_FromUnix(2, 1000),
                               cel_Duration_FromUnix(-4, 0)));
  EXPECT_TRUE(cel_Duration_Equals(d, cel_Duration_FromUnix(6, 1000)));

  ASSERT_TRUE(cel_Duration_Sub(&d, cel_Duration_FromUnix(-4, 0),
                               cel_Duration_FromUnix(2, 1000)));
  EXPECT_TRUE(cel_Duration_Equals(d, cel_Duration_FromUnix(-6, -1000)));

  EXPECT_FALSE(cel_Duration_Sub(
      &d,
      cel_Duration_FromUnix(cel_Duration_kMaxSeconds, cel_Duration_kMaxNanos),
      cel_Duration_FromUnix(0, -1)));
  EXPECT_FALSE(cel_Duration_Sub(
      &d, cel_Duration_FromUnix(0, -1),
      cel_Duration_FromUnix(cel_Duration_kMaxSeconds, cel_Duration_kMaxNanos)));
  EXPECT_FALSE(cel_Duration_Sub(
      &d,
      cel_Duration_FromUnix(cel_Duration_kMinSeconds, cel_Duration_kMinNanos),
      cel_Duration_FromUnix(0, 1)));
  EXPECT_FALSE(cel_Duration_Sub(
      &d, cel_Duration_FromUnix(0, 1),
      cel_Duration_FromUnix(cel_Duration_kMinSeconds, cel_Duration_kMinNanos)));
}

TEST(DurationDeath, Unix) {
  EXPECT_DEBUG_DEATH(
      CEL_USED(cel_Duration_FromUnix(cel_Duration_kMinSeconds - 1, 0)), _);
  EXPECT_DEBUG_DEATH(
      CEL_USED(cel_Duration_FromUnix(cel_Duration_kMaxSeconds + 1, 0)), _);
  EXPECT_DEBUG_DEATH(
      CEL_USED(cel_Duration_FromUnix(0, cel_Duration_kMinNanos - 1)), _);
  EXPECT_DEBUG_DEATH(
      CEL_USED(cel_Duration_FromUnix(0, cel_Duration_kMaxNanos + 1)), _);
  EXPECT_DEBUG_DEATH(CEL_USED(cel_Duration_FromUnix(1, -1)), _);
  EXPECT_DEBUG_DEATH(CEL_USED(cel_Duration_FromUnix(-1, 1)), _);
}

}  // namespace
