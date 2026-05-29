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

#include "cel-c/src/binary_search.h"

#include <array>
#include <cstdint>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::IsNull;

int Int32Compare(const void* lhs, const void* rhs) {
  const int32_t l = *((const int32_t*)lhs);
  const int32_t r = *((const int32_t*)rhs);
  return l < r ? -1 : l > r ? 1 : 0;
}

TEST(BinarySearch, NotFound) {
  std::array<int32_t, 7> values = {-127, -63, -1, 0, 1, 63, 127};

  int32_t key = 128;
  EXPECT_THAT(_cel_BinarySearch(&key, values.data(), values.size(),
                                sizeof(int32_t), &Int32Compare),
              IsNull());

  key = -128;
  EXPECT_THAT(_cel_BinarySearch(&key, values.data(), values.size(),
                                sizeof(int32_t), &Int32Compare),
              IsNull());

  key = 2;
  EXPECT_THAT(_cel_BinarySearch(&key, values.data(), values.size(),
                                sizeof(int32_t), &Int32Compare),
              IsNull());

  key = -2;
  EXPECT_THAT(_cel_BinarySearch(&key, values.data(), values.size(),
                                sizeof(int32_t), &Int32Compare),
              IsNull());
}

TEST(BinarySearch, First) {
  std::array<int32_t, 7> values = {-127, -63, -1, 0, 1, 63, 127};

  int32_t key = -127;
  EXPECT_EQ(_cel_BinarySearch(&key, values.data(), values.size(),
                              sizeof(int32_t), &Int32Compare),
            &values.front());
}

TEST(BinarySearch, Last) {
  std::array<int32_t, 7> values = {-127, -63, -1, 0, 1, 63, 127};

  int32_t key = 127;
  EXPECT_EQ(_cel_BinarySearch(&key, values.data(), values.size(),
                              sizeof(int32_t), &Int32Compare),
            &values.back());
}

TEST(BinarySearch, Mid) {
  std::array<int32_t, 7> values = {-127, -63, -1, 0, 1, 63, 127};

  int32_t key = 0;
  EXPECT_EQ(_cel_BinarySearch(&key, values.data(), values.size(),
                              sizeof(int32_t), &Int32Compare),
            &values[3]);
}

}  // namespace
