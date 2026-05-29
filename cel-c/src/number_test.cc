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

#include "cel-c/src/number.h"

#include <cmath>
#include <cstdint>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/src/compare.h"

namespace {

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;

TEST(Number, Compare) {
  // Equal
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(1), _cel_IntNumber(1)),
              Eq(_cel_PartialOrdering_kEquivalent));
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(1), _cel_UintNumber(1)),
              Eq(_cel_PartialOrdering_kEquivalent));
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(1), _cel_DoubleNumber(1)),
              Eq(_cel_PartialOrdering_kEquivalent));
  EXPECT_THAT(_cel_Number_Compare(_cel_IntNumber(1), _cel_IntNumber(1)),
              Eq(_cel_PartialOrdering_kEquivalent));
  EXPECT_THAT(_cel_Number_Compare(_cel_IntNumber(1), _cel_UintNumber(1)),
              Eq(_cel_PartialOrdering_kEquivalent));
  EXPECT_THAT(_cel_Number_Compare(_cel_IntNumber(1), _cel_DoubleNumber(1)),
              Eq(_cel_PartialOrdering_kEquivalent));
  EXPECT_THAT(_cel_Number_Compare(_cel_UintNumber(1), _cel_DoubleNumber(1)),
              Eq(_cel_PartialOrdering_kEquivalent));
  EXPECT_THAT(_cel_Number_Compare(_cel_UintNumber(1), _cel_IntNumber(1)),
              Eq(_cel_PartialOrdering_kEquivalent));
  EXPECT_THAT(_cel_Number_Compare(_cel_UintNumber(1), _cel_UintNumber(1)),
              Eq(_cel_PartialOrdering_kEquivalent));

  // Greater
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(1.5), _cel_IntNumber(1)),
              Eq(_cel_PartialOrdering_kGreater));
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(1.5), _cel_UintNumber(1)),
              Eq(_cel_PartialOrdering_kGreater));
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(1.5), _cel_DoubleNumber(1)),
              Eq(_cel_PartialOrdering_kGreater));
  EXPECT_THAT(_cel_Number_Compare(_cel_IntNumber(2), _cel_IntNumber(1)),
              Eq(_cel_PartialOrdering_kGreater));
  EXPECT_THAT(_cel_Number_Compare(_cel_IntNumber(2), _cel_UintNumber(1)),
              Eq(_cel_PartialOrdering_kGreater));
  EXPECT_THAT(_cel_Number_Compare(_cel_IntNumber(2), _cel_DoubleNumber(1.5)),
              Eq(_cel_PartialOrdering_kGreater));
  EXPECT_THAT(_cel_Number_Compare(_cel_UintNumber(2), _cel_IntNumber(1)),
              Eq(_cel_PartialOrdering_kGreater));
  EXPECT_THAT(_cel_Number_Compare(_cel_UintNumber(2), _cel_UintNumber(1)),
              Eq(_cel_PartialOrdering_kGreater));
  EXPECT_THAT(_cel_Number_Compare(_cel_UintNumber(2), _cel_DoubleNumber(1.5)),
              Eq(_cel_PartialOrdering_kGreater));

  // Less
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(1), _cel_IntNumber(2)),
              Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(1), _cel_UintNumber(2)),
              Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(1), _cel_DoubleNumber(1.5)),
              Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(_cel_Number_Compare(_cel_IntNumber(1), _cel_IntNumber(2)),
              Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(_cel_Number_Compare(_cel_IntNumber(1), _cel_UintNumber(2)),
              Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(_cel_Number_Compare(_cel_IntNumber(1), _cel_DoubleNumber(1.5)),
              Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(_cel_Number_Compare(_cel_UintNumber(1), _cel_IntNumber(2)),
              Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(_cel_Number_Compare(_cel_UintNumber(1), _cel_UintNumber(2)),
              Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(_cel_Number_Compare(_cel_UintNumber(1), _cel_DoubleNumber(1.5)),
              Eq(_cel_PartialOrdering_kLess));

  // NaN
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(NAN), _cel_IntNumber(2)),
              Eq(_cel_PartialOrdering_kUnordered));
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(NAN), _cel_UintNumber(2)),
              Eq(_cel_PartialOrdering_kUnordered));
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(1), _cel_DoubleNumber(NAN)),
              Eq(_cel_PartialOrdering_kUnordered));
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(NAN), _cel_DoubleNumber(1)),
              Eq(_cel_PartialOrdering_kUnordered));
  EXPECT_THAT(_cel_Number_Compare(_cel_IntNumber(1), _cel_DoubleNumber(NAN)),
              Eq(_cel_PartialOrdering_kUnordered));
  EXPECT_THAT(_cel_Number_Compare(_cel_UintNumber(1), _cel_DoubleNumber(NAN)),
              Eq(_cel_PartialOrdering_kUnordered));

  // +Infinity
  EXPECT_THAT(
      _cel_Number_Compare(_cel_DoubleNumber(INFINITY), _cel_IntNumber(0)),
      Eq(_cel_PartialOrdering_kGreater));
  EXPECT_THAT(
      _cel_Number_Compare(_cel_DoubleNumber(INFINITY), _cel_UintNumber(0)),
      Eq(_cel_PartialOrdering_kGreater));
  EXPECT_THAT(
      _cel_Number_Compare(_cel_DoubleNumber(INFINITY), _cel_DoubleNumber(0)),
      Eq(_cel_PartialOrdering_kGreater));
  EXPECT_THAT(
      _cel_Number_Compare(_cel_IntNumber(0), _cel_DoubleNumber(INFINITY)),
      Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(
      _cel_Number_Compare(_cel_UintNumber(0), _cel_DoubleNumber(INFINITY)),
      Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(
      _cel_Number_Compare(_cel_DoubleNumber(0), _cel_DoubleNumber(INFINITY)),
      Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(INFINITY),
                                  _cel_DoubleNumber(INFINITY)),
              Eq(_cel_PartialOrdering_kEquivalent));

  // -Infinity
  EXPECT_THAT(
      _cel_Number_Compare(_cel_DoubleNumber(-INFINITY), _cel_IntNumber(0)),
      Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(
      _cel_Number_Compare(_cel_DoubleNumber(-INFINITY), _cel_UintNumber(0)),
      Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(
      _cel_Number_Compare(_cel_DoubleNumber(-INFINITY), _cel_DoubleNumber(0)),
      Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(
      _cel_Number_Compare(_cel_IntNumber(0), _cel_DoubleNumber(-INFINITY)),
      Eq(_cel_PartialOrdering_kGreater));
  EXPECT_THAT(
      _cel_Number_Compare(_cel_UintNumber(0), _cel_DoubleNumber(-INFINITY)),
      Eq(_cel_PartialOrdering_kGreater));
  EXPECT_THAT(
      _cel_Number_Compare(_cel_DoubleNumber(0), _cel_DoubleNumber(-INFINITY)),
      Eq(_cel_PartialOrdering_kGreater));
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(-INFINITY),
                                  _cel_DoubleNumber(-INFINITY)),
              Eq(_cel_PartialOrdering_kEquivalent));

  // [+-]Infinity
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(-INFINITY),
                                  _cel_DoubleNumber(INFINITY)),
              Eq(_cel_PartialOrdering_kLess));
  EXPECT_THAT(_cel_Number_Compare(_cel_DoubleNumber(INFINITY),
                                  _cel_DoubleNumber(-INFINITY)),
              Eq(_cel_PartialOrdering_kGreater));
}

TEST(Number, Equality) {
  // Equal
  EXPECT_THAT(_cel_Number_Equals(_cel_DoubleNumber(1), _cel_IntNumber(1)),
              IsTrue());
  EXPECT_THAT(_cel_Number_Equals(_cel_DoubleNumber(1), _cel_UintNumber(1)),
              IsTrue());
  EXPECT_THAT(_cel_Number_Equals(_cel_DoubleNumber(1), _cel_DoubleNumber(1)),
              IsTrue());
  EXPECT_THAT(_cel_Number_Equals(_cel_IntNumber(1), _cel_IntNumber(1)),
              IsTrue());
  EXPECT_THAT(_cel_Number_Equals(_cel_IntNumber(1), _cel_UintNumber(1)),
              IsTrue());
  EXPECT_THAT(_cel_Number_Equals(_cel_IntNumber(1), _cel_DoubleNumber(1)),
              IsTrue());
  EXPECT_THAT(_cel_Number_Equals(_cel_UintNumber(1), _cel_DoubleNumber(1)),
              IsTrue());
  EXPECT_THAT(_cel_Number_Equals(_cel_UintNumber(1), _cel_IntNumber(1)),
              IsTrue());
  EXPECT_THAT(_cel_Number_Equals(_cel_UintNumber(1), _cel_UintNumber(1)),
              IsTrue());

  // Greater
  EXPECT_THAT(_cel_Number_Equals(_cel_DoubleNumber(1.5), _cel_IntNumber(1)),
              IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_DoubleNumber(1.5), _cel_UintNumber(1)),
              IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_DoubleNumber(1.5), _cel_DoubleNumber(1)),
              IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_IntNumber(2), _cel_IntNumber(1)),
              IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_IntNumber(2), _cel_UintNumber(1)),
              IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_IntNumber(2), _cel_DoubleNumber(1.5)),
              IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_UintNumber(2), _cel_IntNumber(1)),
              IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_UintNumber(2), _cel_UintNumber(1)),
              IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_UintNumber(2), _cel_DoubleNumber(1.5)),
              IsFalse());

  // +Infinity
  EXPECT_THAT(
      _cel_Number_Equals(_cel_DoubleNumber(INFINITY), _cel_IntNumber(0)),
      IsFalse());
  EXPECT_THAT(
      _cel_Number_Equals(_cel_DoubleNumber(INFINITY), _cel_UintNumber(0)),
      IsFalse());
  EXPECT_THAT(
      _cel_Number_Equals(_cel_DoubleNumber(INFINITY), _cel_DoubleNumber(0)),
      IsFalse());
  EXPECT_THAT(
      _cel_Number_Equals(_cel_IntNumber(0), _cel_DoubleNumber(INFINITY)),
      IsFalse());
  EXPECT_THAT(
      _cel_Number_Equals(_cel_UintNumber(0), _cel_DoubleNumber(INFINITY)),
      IsFalse());
  EXPECT_THAT(
      _cel_Number_Equals(_cel_DoubleNumber(0), _cel_DoubleNumber(INFINITY)),
      IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_DoubleNumber(INFINITY),
                                 _cel_DoubleNumber(INFINITY)),
              IsTrue());

  // -Infinity
  EXPECT_THAT(
      _cel_Number_Equals(_cel_DoubleNumber(-INFINITY), _cel_IntNumber(0)),
      IsFalse());
  EXPECT_THAT(
      _cel_Number_Equals(_cel_DoubleNumber(-INFINITY), _cel_UintNumber(0)),
      IsFalse());
  EXPECT_THAT(
      _cel_Number_Equals(_cel_DoubleNumber(-INFINITY), _cel_DoubleNumber(0)),
      IsFalse());
  EXPECT_THAT(
      _cel_Number_Equals(_cel_IntNumber(0), _cel_DoubleNumber(-INFINITY)),
      IsFalse());
  EXPECT_THAT(
      _cel_Number_Equals(_cel_UintNumber(0), _cel_DoubleNumber(-INFINITY)),
      IsFalse());
  EXPECT_THAT(
      _cel_Number_Equals(_cel_DoubleNumber(0), _cel_DoubleNumber(-INFINITY)),
      IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_DoubleNumber(-INFINITY),
                                 _cel_DoubleNumber(-INFINITY)),
              IsTrue());

  // [+-]Infinity
  EXPECT_THAT(_cel_Number_Equals(_cel_DoubleNumber(-INFINITY),
                                 _cel_DoubleNumber(INFINITY)),
              IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_DoubleNumber(INFINITY),
                                 _cel_DoubleNumber(-INFINITY)),
              IsFalse());

  // NaN
  EXPECT_THAT(
      _cel_Number_Equals(_cel_DoubleNumber(NAN), _cel_DoubleNumber(NAN)),
      IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_DoubleNumber(NAN), _cel_IntNumber(1)),
              IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_DoubleNumber(NAN), _cel_UintNumber(1)),
              IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_IntNumber(1), _cel_DoubleNumber(NAN)),
              IsFalse());
  EXPECT_THAT(_cel_Number_Equals(_cel_UintNumber(1), _cel_DoubleNumber(NAN)),
              IsFalse());
}

TEST(Number, Conversions) {
  int64_t i;
  uint64_t u;

  ASSERT_THAT(_cel_Number_ToIntLossless(_cel_DoubleNumber(1), &i), IsTrue());
  EXPECT_EQ(i, 1);
  ASSERT_THAT(_cel_Number_ToIntLossless(_cel_IntNumber(1), &i), IsTrue());
  EXPECT_EQ(i, 1);
  ASSERT_THAT(_cel_Number_ToIntLossless(_cel_UintNumber(1), &i), IsTrue());
  EXPECT_EQ(i, 1);
  ASSERT_THAT(_cel_Number_ToIntLossless(_cel_DoubleNumber(-0.0), &i), IsTrue());
  EXPECT_EQ(i, 0);

  ASSERT_THAT(_cel_Number_ToIntLossless(_cel_DoubleNumber(1.5), &i), IsFalse());
  ASSERT_THAT(_cel_Number_ToIntLossless(_cel_DoubleNumber(INFINITY), &i),
              IsFalse());
  ASSERT_THAT(_cel_Number_ToIntLossless(_cel_DoubleNumber(-INFINITY), &i),
              IsFalse());
  ASSERT_THAT(_cel_Number_ToIntLossless(_cel_DoubleNumber(NAN), &i), IsFalse());
  ASSERT_THAT(_cel_Number_ToIntLossless(_cel_UintNumber(UINT64_MAX), &i),
              IsFalse());
  ASSERT_THAT(
      _cel_Number_ToIntLossless(_cel_DoubleNumber((double)INT64_MAX + 1.0), &i),
      IsFalse());

  ASSERT_THAT(_cel_Number_ToInt(_cel_DoubleNumber(1.5), &i), IsTrue());
  EXPECT_EQ(i, 1);
  ASSERT_THAT(_cel_Number_ToInt(_cel_IntNumber(1), &i), IsTrue());
  EXPECT_EQ(i, 1);
  ASSERT_THAT(_cel_Number_ToInt(_cel_UintNumber(1), &i), IsTrue());
  EXPECT_EQ(i, 1);
  ASSERT_THAT(_cel_Number_ToInt(_cel_DoubleNumber(-1.5), &i), IsTrue());
  EXPECT_EQ(i, -1);

  ASSERT_THAT(_cel_Number_ToInt(_cel_UintNumber(UINT64_MAX), &i), IsFalse());
  ASSERT_THAT(_cel_Number_ToInt(_cel_DoubleNumber(INFINITY), &i), IsFalse());
  ASSERT_THAT(_cel_Number_ToInt(_cel_DoubleNumber(-INFINITY), &i), IsFalse());
  ASSERT_THAT(_cel_Number_ToInt(_cel_DoubleNumber(NAN), &i), IsFalse());
  ASSERT_THAT(_cel_Number_ToInt(_cel_DoubleNumber((double)INT64_MAX * 2.0), &i),
              IsFalse());

  ASSERT_THAT(_cel_Number_ToUint(_cel_DoubleNumber(1), &u), IsTrue());
  EXPECT_EQ(u, 1);
  ASSERT_THAT(_cel_Number_ToUint(_cel_IntNumber(1), &u), IsTrue());
  EXPECT_EQ(u, 1);
  ASSERT_THAT(_cel_Number_ToUint(_cel_UintNumber(1), &u), IsTrue());
  EXPECT_EQ(u, 1);
  ASSERT_THAT(_cel_Number_ToUint(_cel_DoubleNumber(-0.0), &u), IsTrue());
  EXPECT_EQ(u, 0);
  ASSERT_THAT(_cel_Number_ToUint(_cel_DoubleNumber(1.5), &u), IsTrue());
  EXPECT_EQ(u, 1);

  ASSERT_THAT(_cel_Number_ToUint(_cel_DoubleNumber(INFINITY), &u), IsFalse());
  ASSERT_THAT(_cel_Number_ToUint(_cel_DoubleNumber(-INFINITY), &u), IsFalse());
  ASSERT_THAT(_cel_Number_ToUint(_cel_DoubleNumber(NAN), &u), IsFalse());
  ASSERT_THAT(_cel_Number_ToUint(_cel_IntNumber(INT64_MIN), &u), IsFalse());
  ASSERT_THAT(_cel_Number_ToUint(_cel_IntNumber(-1), &u), IsFalse());
  ASSERT_THAT(
      _cel_Number_ToUint(_cel_DoubleNumber((double)UINT64_MAX * 2.0), &u),
      IsFalse());

  EXPECT_EQ(_cel_Number_ToDouble(_cel_DoubleNumber(1.5)), 1.5);
  EXPECT_EQ(_cel_Number_ToDouble(_cel_IntNumber(1)), 1.0);
  EXPECT_EQ(_cel_Number_ToDouble(_cel_UintNumber(1)), 1.0);
}

}  // namespace
