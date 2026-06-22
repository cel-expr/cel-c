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

#include "cel-c/internal/arena_array.h"

#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/config.h"

namespace {

using ::testing::_;
using ::testing::IsNull;
using ::testing::NotNull;
using ::testing::Test;

class ArenaArrayTest : public Test {
 public:
  void SetUp() override {
    arena_ = ABSL_DIE_IF_NULL(cel_Arena_New(cel_DefaultAllocator));
  }

  void TearDown() override {
    cel_Arena_Delete(arena_);
    arena_ = nullptr;
  }

 protected:
  CEL_NONNULL(cel_Arena*) arena() { return ABSL_DIE_IF_NULL(arena_); }

 private:
  CEL_NULLABLE(cel_Arena*) arena_ = nullptr;
};

TEST_F(ArenaArrayTest, Empty) {
  _cel_ArenaArray(int) array;
  _cel_ArenaArray_Construct(&array);

  EXPECT_EQ(_cel_ArenaArray_Size(&array), 0);
  EXPECT_TRUE(_cel_ArenaArray_Empty(&array));
  EXPECT_EQ(_cel_ArenaArray_Capacity(&array), 0);
  EXPECT_THAT(_cel_ArenaArray_Data(&array), IsNull());
  EXPECT_EQ(_cel_ArenaArray_MutableData(&array), _cel_ArenaArray_Data(&array));
}

TEST_F(ArenaArrayTest, PushPop) {
  _cel_ArenaArray(int) array;
  _cel_ArenaArray_Construct(&array);

  auto* pushed = _cel_ArenaArray_Push(&array, arena());
  ASSERT_THAT(pushed, NotNull());
  *pushed = 1;

  EXPECT_EQ(_cel_ArenaArray_Size(&array), 1);
  EXPECT_FALSE(_cel_ArenaArray_Empty(&array));
  EXPECT_GE(_cel_ArenaArray_Capacity(&array), _cel_ArenaArray_Size(&array));
  EXPECT_EQ(_cel_ArenaArray_Data(&array), pushed);
  EXPECT_EQ(_cel_ArenaArray_MutableData(&array), _cel_ArenaArray_Data(&array));
  EXPECT_EQ(_cel_ArenaArray_At(&array, 0), pushed);
  EXPECT_EQ(_cel_ArenaArray_MutableAt(&array, 0), pushed);
  EXPECT_EQ(_cel_ArenaArray_Front(&array), pushed);
  EXPECT_EQ(_cel_ArenaArray_MutableFront(&array), pushed);

  _cel_ArenaArray_Pop(&array);
  EXPECT_EQ(_cel_ArenaArray_Size(&array), 0);
  EXPECT_TRUE(_cel_ArenaArray_Empty(&array));
  EXPECT_GT(_cel_ArenaArray_Capacity(&array), _cel_ArenaArray_Size(&array));
  EXPECT_THAT(_cel_ArenaArray_Data(&array), NotNull());
  EXPECT_EQ(_cel_ArenaArray_MutableData(&array), _cel_ArenaArray_Data(&array));
}

TEST_F(ArenaArrayTest, Erase) {
  _cel_ArenaArray(int) array;
  _cel_ArenaArray_Construct(&array);

  auto* pushed = _cel_ArenaArray_Push(&array, arena());
  ASSERT_THAT(pushed, NotNull());
  *pushed = 1;

  pushed = _cel_ArenaArray_Push(&array, arena());
  ASSERT_THAT(pushed, NotNull());
  *pushed = 2;

  pushed = _cel_ArenaArray_Push(&array, arena());
  ASSERT_THAT(pushed, NotNull());
  *pushed = 3;

  _cel_ArenaArray_Erase(&array, 1);
  EXPECT_FALSE(_cel_ArenaArray_Empty(&array));
  EXPECT_EQ(_cel_ArenaArray_Size(&array), 2);
  EXPECT_EQ(*_cel_ArenaArray_Front(&array), 1);
  EXPECT_EQ(*_cel_ArenaArray_Back(&array), 3);
}

TEST_F(ArenaArrayTest, Append) {
  _cel_ArenaArray(int) array;
  _cel_ArenaArray_Construct(&array);

  auto* appended = _cel_ArenaArray_Append(&array, arena(), 150);
  ASSERT_THAT(appended, NotNull());
  for (size_t i = 0; i < 150; ++i) {
    appended[i] = static_cast<int>(i);
  }

  EXPECT_EQ(_cel_ArenaArray_Size(&array), 150);
  EXPECT_FALSE(_cel_ArenaArray_Empty(&array));
  EXPECT_GE(_cel_ArenaArray_Capacity(&array), _cel_ArenaArray_Size(&array));
  EXPECT_EQ(_cel_ArenaArray_Data(&array), appended);
  EXPECT_EQ(_cel_ArenaArray_MutableData(&array), _cel_ArenaArray_Data(&array));
  EXPECT_EQ(_cel_ArenaArray_At(&array, 0), appended);
  EXPECT_EQ(_cel_ArenaArray_MutableAt(&array, 0), appended);
  EXPECT_EQ(_cel_ArenaArray_Front(&array), appended);
  EXPECT_EQ(_cel_ArenaArray_MutableFront(&array), appended);
  EXPECT_EQ(*_cel_ArenaArray_Back(&array), 149);
  EXPECT_EQ(*_cel_ArenaArray_MutableBack(&array), 149);

  const auto* begin = _cel_ArenaArray_Begin(&array);
  EXPECT_EQ(_cel_ArenaArray_MutableBegin(&array), begin);
  const auto* end = _cel_ArenaArray_End(&array);
  EXPECT_EQ(_cel_ArenaArray_MutableEnd(&array), end);
  EXPECT_EQ(begin, _cel_ArenaArray_Data(&array));
  EXPECT_EQ(end - begin, _cel_ArenaArray_Size(&array));

  appended = _cel_ArenaArray_Append(&array, arena(), 150);
  ASSERT_THAT(appended, NotNull());
  for (size_t i = 0; i < 150; ++i) {
    appended[i] = static_cast<int>(i + 150);
  }

  EXPECT_EQ(appended, _cel_ArenaArray_At(&array, 150));
  EXPECT_EQ(_cel_ArenaArray_MutableAt(&array, 150),
            _cel_ArenaArray_At(&array, 150));
  EXPECT_EQ(_cel_ArenaArray_Size(&array), 300);
  EXPECT_GE(_cel_ArenaArray_Capacity(&array), _cel_ArenaArray_Size(&array));
  begin = _cel_ArenaArray_Begin(&array);
  EXPECT_EQ(_cel_ArenaArray_MutableBegin(&array), begin);
  end = _cel_ArenaArray_End(&array);
  EXPECT_EQ(_cel_ArenaArray_MutableEnd(&array), end);
  EXPECT_EQ(begin, _cel_ArenaArray_Data(&array));
  EXPECT_EQ(end - begin, _cel_ArenaArray_Size(&array));

  _cel_ArenaArray_Clear(&array);
  EXPECT_EQ(_cel_ArenaArray_Size(&array), 0);
  EXPECT_TRUE(_cel_ArenaArray_Empty(&array));
}

TEST_F(ArenaArrayTest, ReserveResize) {
  _cel_ArenaArray(int) array;
  _cel_ArenaArray_Construct(&array);

  ASSERT_TRUE(_cel_ArenaArray_Reserve(&array, arena(), 150));
  EXPECT_GE(_cel_ArenaArray_Capacity(&array), 150);
  EXPECT_TRUE(_cel_ArenaArray_Empty(&array));

  auto* resized = _cel_ArenaArray_Resize(&array, arena(), 300);
  ASSERT_THAT(resized, NotNull());
  for (size_t i = 0; i < 300; ++i) {
    resized[i] = static_cast<int>(i);
  }
  EXPECT_EQ(resized, _cel_ArenaArray_Data(&array));
  EXPECT_EQ(_cel_ArenaArray_Size(&array), 300);
  EXPECT_GE(_cel_ArenaArray_Capacity(&array), _cel_ArenaArray_Size(&array));
  EXPECT_FALSE(_cel_ArenaArray_Empty(&array));
}

using ArenaArrayDeathTest = ArenaArrayTest;

TEST_F(ArenaArrayDeathTest, Empty) {
  _cel_ArenaArray(int) array;
  _cel_ArenaArray_Construct(&array);

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_ArenaArray_At(&array, 0)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_ArenaArray_MutableAt(&array, 0)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_ArenaArray_Front(&array)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_ArenaArray_MutableFront(&array)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_ArenaArray_Back(&array)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_ArenaArray_MutableBack(&array)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_ArenaArray_Pop(&array)), _);
#else
  GTEST_SKIP() << "optimized build or death test is unsupported";
#endif
}

}  // namespace
