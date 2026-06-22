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

#include "cel-c/internal/array.h"

#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/alloc.h"
#include "cel-c/config.h"

namespace {

using ::testing::_;
using ::testing::IsNull;
using ::testing::NotNull;
using ::testing::Test;

class ArrayTest : public Test {
 public:
  void SetUp() override { _cel_Array_Construct(&array_); }

  void TearDown() override { _cel_Array_Destruct(&array_, alloc()); }

 protected:
  CEL_NONNULL(cel_Allocator*) alloc() { return cel_DefaultAllocator; }

  auto* array() { return &array_; }

 private:
  _cel_Array(int) array_;
};

TEST_F(ArrayTest, Empty) {
  EXPECT_EQ(_cel_Array_Size(array()), 0);
  EXPECT_TRUE(_cel_Array_Empty(array()));
  EXPECT_EQ(_cel_Array_Capacity(array()), 0);
  EXPECT_THAT(_cel_Array_Data(array()), IsNull());
  EXPECT_EQ(_cel_Array_MutableData(array()), _cel_Array_Data(array()));
}

TEST_F(ArrayTest, PushPop) {
  auto* pushed = _cel_Array_Push(array(), alloc());
  ASSERT_THAT(pushed, NotNull());
  *pushed = 1;

  EXPECT_EQ(_cel_Array_Size(array()), 1);
  EXPECT_FALSE(_cel_Array_Empty(array()));
  EXPECT_GE(_cel_Array_Capacity(array()), _cel_Array_Size(array()));
  EXPECT_EQ(_cel_Array_Data(array()), pushed);
  EXPECT_EQ(_cel_Array_MutableData(array()), _cel_Array_Data(array()));
  EXPECT_EQ(_cel_Array_At(array(), 0), pushed);
  EXPECT_EQ(_cel_Array_MutableAt(array(), 0), pushed);
  EXPECT_EQ(_cel_Array_Front(array()), pushed);
  EXPECT_EQ(_cel_Array_MutableFront(array()), pushed);

  _cel_Array_ShrinkToFit(array(), alloc());
  EXPECT_EQ(_cel_Array_Size(array()), 1);
  EXPECT_FALSE(_cel_Array_Empty(array()));
  EXPECT_GE(_cel_Array_Capacity(array()), _cel_Array_Size(array()));

  _cel_Array_Pop(array());
  EXPECT_EQ(_cel_Array_Size(array()), 0);
  EXPECT_TRUE(_cel_Array_Empty(array()));
  EXPECT_GT(_cel_Array_Capacity(array()), _cel_Array_Size(array()));
  EXPECT_THAT(_cel_Array_Data(array()), NotNull());
  EXPECT_EQ(_cel_Array_MutableData(array()), _cel_Array_Data(array()));

  _cel_Array_ShrinkToFit(array(), alloc());
  EXPECT_EQ(_cel_Array_Size(array()), 0);
  EXPECT_TRUE(_cel_Array_Empty(array()));
  EXPECT_GE(_cel_Array_Capacity(array()), _cel_Array_Size(array()));
}

TEST_F(ArrayTest, Erase) {
  auto* pushed = _cel_Array_Push(array(), alloc());
  ASSERT_THAT(pushed, NotNull());
  *pushed = 1;

  pushed = _cel_Array_Push(array(), alloc());
  ASSERT_THAT(pushed, NotNull());
  *pushed = 2;

  pushed = _cel_Array_Push(array(), alloc());
  ASSERT_THAT(pushed, NotNull());
  *pushed = 3;

  _cel_Array_Erase(array(), 1);
  EXPECT_FALSE(_cel_Array_Empty(array()));
  EXPECT_EQ(_cel_Array_Size(array()), 2);
  EXPECT_EQ(*_cel_Array_Front(array()), 1);
  EXPECT_EQ(*_cel_Array_Back(array()), 3);
}

TEST_F(ArrayTest, Append) {
  auto* appended = _cel_Array_Append(array(), alloc(), 150);
  ASSERT_THAT(appended, NotNull());
  for (size_t i = 0; i < 150; ++i) {
    appended[i] = static_cast<int>(i);
  }

  EXPECT_EQ(_cel_Array_Size(array()), 150);
  EXPECT_FALSE(_cel_Array_Empty(array()));
  EXPECT_GE(_cel_Array_Capacity(array()), _cel_Array_Size(array()));
  EXPECT_EQ(_cel_Array_Data(array()), appended);
  EXPECT_EQ(_cel_Array_MutableData(array()), _cel_Array_Data(array()));
  EXPECT_EQ(_cel_Array_At(array(), 0), appended);
  EXPECT_EQ(_cel_Array_MutableAt(array(), 0), appended);
  EXPECT_EQ(_cel_Array_Front(array()), appended);
  EXPECT_EQ(_cel_Array_MutableFront(array()), appended);
  EXPECT_EQ(*_cel_Array_Back(array()), 149);
  EXPECT_EQ(*_cel_Array_MutableBack(array()), 149);

  const auto* begin = _cel_Array_Begin(array());
  EXPECT_EQ(_cel_Array_MutableBegin(array()), begin);
  const auto* end = _cel_Array_End(array());
  EXPECT_EQ(_cel_Array_MutableEnd(array()), end);
  EXPECT_EQ(begin, _cel_Array_Data(array()));
  EXPECT_EQ(end - begin, _cel_Array_Size(array()));

  appended = _cel_Array_Append(array(), alloc(), 150);
  ASSERT_THAT(appended, NotNull());
  for (size_t i = 0; i < 150; ++i) {
    appended[i] = static_cast<int>(i + 150);
  }

  EXPECT_EQ(appended, _cel_Array_At(array(), 150));
  EXPECT_EQ(_cel_Array_MutableAt(array(), 150), _cel_Array_At(array(), 150));
  EXPECT_EQ(_cel_Array_Size(array()), 300);
  EXPECT_GE(_cel_Array_Capacity(array()), _cel_Array_Size(array()));
  begin = _cel_Array_Begin(array());
  EXPECT_EQ(_cel_Array_MutableBegin(array()), begin);
  end = _cel_Array_End(array());
  EXPECT_EQ(_cel_Array_MutableEnd(array()), end);
  EXPECT_EQ(begin, _cel_Array_Data(array()));
  EXPECT_EQ(end - begin, _cel_Array_Size(array()));

  _cel_Array_Clear(array());
  EXPECT_EQ(_cel_Array_Size(array()), 0);
  EXPECT_TRUE(_cel_Array_Empty(array()));
}

TEST_F(ArrayTest, ReserveResize) {
  ASSERT_TRUE(_cel_Array_Reserve(array(), alloc(), 150));
  EXPECT_GE(_cel_Array_Capacity(array()), 150);
  EXPECT_TRUE(_cel_Array_Empty(array()));

  auto* resized = _cel_Array_Resize(array(), alloc(), 300);
  ASSERT_THAT(resized, NotNull());
  for (size_t i = 0; i < 300; ++i) {
    resized[i] = static_cast<int>(i);
  }
  EXPECT_EQ(resized, _cel_Array_Data(array()));
  EXPECT_EQ(_cel_Array_Size(array()), 300);
  EXPECT_GE(_cel_Array_Capacity(array()), _cel_Array_Size(array()));
  EXPECT_FALSE(_cel_Array_Empty(array()));

  _cel_Array_Reset(array(), alloc());
  EXPECT_EQ(_cel_Array_Size(array()), 0);
  EXPECT_TRUE(_cel_Array_Empty(array()));
}

using ArrayDeathTest = ArrayTest;

TEST_F(ArrayDeathTest, Empty) {
#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_Array_At(array(), 0)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_Array_MutableAt(array(), 0)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_Array_Front(array())), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_Array_MutableFront(array())), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_Array_Back(array())), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_Array_MutableBack(array())), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_Array_Pop(array())), _);
#else
  GTEST_SKIP() << "optimized build or death test is unsupported";
#endif
}

}  // namespace
