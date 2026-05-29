// Copyright 2024 Google LLC
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

#include "cel-c/src/malloc.h"

#include <algorithm>
#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/config.h"
#include "cel-c/src/align.h"

namespace {

using ::testing::IsNull;
using ::testing::NotNull;

inline constexpr size_t kOveraligned =
    std::max(alignof(std::max_align_t),
             static_cast<size_t>(__STDCPP_DEFAULT_NEW_ALIGNMENT__)) *
    2;

TEST(Malloc, Malloc) {
  size_t actual_size;
  ASSERT_THAT(_cel_Malloc(0, &actual_size), IsNull());
  EXPECT_THAT(actual_size, 0);

  CEL_NULLABLE(void*)
  addr = _cel_Malloc(16, &actual_size);
  EXPECT_THAT(addr, NotNull());
  EXPECT_GE(actual_size, 16);
  _cel_Free(addr);
}

TEST(Malloc, Calloc) {
  size_t actual_num;
  ASSERT_THAT(_cel_Calloc(0, 0, &actual_num), IsNull());
  EXPECT_THAT(actual_num, 0);
  ASSERT_THAT(_cel_Calloc(1, 0, &actual_num), IsNull());
  EXPECT_THAT(actual_num, 0);
  ASSERT_THAT(_cel_Calloc(0, 1, &actual_num), IsNull());
  EXPECT_THAT(actual_num, 0);

  CEL_NULLABLE(void*)
  addr = _cel_Calloc(1, 16, &actual_num);
  EXPECT_THAT(addr, NotNull());
  EXPECT_GE(actual_num, 1);
  _cel_FreeSized(addr, actual_num * 16);
}

TEST(Malloc, Realloc) {
  size_t actual_size;

  CEL_NULLABLE(void*)
  addr = _cel_Realloc(cel_nullptr, 0, 16, &actual_size);
  EXPECT_THAT(addr, NotNull());
  EXPECT_GE(actual_size, 16);

  addr = _cel_Realloc(addr, 16, 32, &actual_size);
  ASSERT_THAT(addr, NotNull());
  EXPECT_GE(actual_size, 32);

  EXPECT_THAT(_cel_Realloc(addr, 32, 0, &actual_size), IsNull());
  EXPECT_EQ(actual_size, 0);
}

}  // namespace
