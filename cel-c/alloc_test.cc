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

#include "cel-c/alloc.h"

#include <cstddef>
#include <cstring>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/config.h"
#include "cel-c/src/align.h"

namespace {

using ::testing::IsNull;
using ::testing::NotNull;

TEST(Allocator, Default) { EXPECT_THAT(cel_DefaultAllocator, NotNull()); }

TEST(DefaultAllocator, Malloc) {
  size_t actual_size;
  ASSERT_THAT(cel_Allocator_Malloc(cel_DefaultAllocator, 0, &actual_size),
              IsNull());
  EXPECT_THAT(actual_size, 0);

  CEL_NULLABLE(void*)
  addr = cel_Allocator_Malloc(cel_DefaultAllocator, 16, &actual_size);
  EXPECT_THAT(addr, NotNull());
  EXPECT_TRUE(_cel_is_aligned(addr, cel_Allocator_kMaxAlign));
  EXPECT_GE(actual_size, 16);
  cel_Allocator_Free(cel_DefaultAllocator, addr);
}

TEST(DefaultAllocator, Calloc) {
  size_t actual_num;
  ASSERT_THAT(cel_Allocator_Calloc(cel_DefaultAllocator, 0, 0, &actual_num),
              IsNull());
  EXPECT_THAT(actual_num, 0);
  ASSERT_THAT(cel_Allocator_Calloc(cel_DefaultAllocator, 1, 0, &actual_num),
              IsNull());
  EXPECT_THAT(actual_num, 0);
  ASSERT_THAT(cel_Allocator_Calloc(cel_DefaultAllocator, 0, 1, &actual_num),
              IsNull());
  EXPECT_THAT(actual_num, 0);

  CEL_NULLABLE(void*)
  addr = cel_Allocator_Calloc(cel_DefaultAllocator, 1, 16, &actual_num);
  EXPECT_THAT(addr, NotNull());
  EXPECT_TRUE(_cel_is_aligned(addr, cel_Allocator_kMaxAlign));
  EXPECT_GE(actual_num, 1);
  cel_Allocator_FreeSized(cel_DefaultAllocator, addr, actual_num * 16);
}

TEST(DefaultAllocator, Realloc) {
  size_t actual_size;

  CEL_NULLABLE(void*)
  addr = cel_Allocator_Realloc(cel_DefaultAllocator, cel_nullptr, 0, 16,
                               &actual_size);
  EXPECT_THAT(addr, NotNull());
  EXPECT_TRUE(_cel_is_aligned(addr, cel_Allocator_kMaxAlign));
  EXPECT_GE(actual_size, 16);

  addr =
      cel_Allocator_Realloc(cel_DefaultAllocator, addr, 16, 32, &actual_size);
  ASSERT_THAT(addr, NotNull());
  EXPECT_TRUE(_cel_is_aligned(addr, cel_Allocator_kMaxAlign));
  EXPECT_GE(actual_size, 32);

  EXPECT_THAT(
      cel_Allocator_Realloc(cel_DefaultAllocator, addr, 32, 0, &actual_size),
      IsNull());
  EXPECT_EQ(actual_size, 0);
}

TEST(DefaultAllocator, StrDup) {
  size_t actual_size;
  CEL_NULLABLE(char*)
  str = cel_Allocator_StrDup(cel_DefaultAllocator, &actual_size, "");
  ASSERT_THAT(str, NotNull());
  EXPECT_THAT(std::strlen(str), 0);
  EXPECT_GT(actual_size, 0);
  cel_Allocator_FreeSized(cel_DefaultAllocator, str, actual_size);

  str =
      cel_Allocator_StrDup(cel_DefaultAllocator, &actual_size, "Hello World!");
  ASSERT_THAT(str, NotNull());
  EXPECT_TRUE(_cel_is_aligned(str, cel_Allocator_kMaxAlign));
  EXPECT_THAT(std::strlen(str), std::strlen("Hello World!"));
  EXPECT_THAT(std::strcmp(str, "Hello World!"), 0);
  EXPECT_GT(actual_size, std::strlen("Hello World!"));
  cel_Allocator_FreeSized(cel_DefaultAllocator, str, actual_size);
}

TEST(DefaultAllocator, PrintF) {
  size_t actual_size;
  CEL_NULLABLE(char*)
  str = cel_Allocator_PrintF(cel_DefaultAllocator, &actual_size, "%s",
                             "Hello World!");
  ASSERT_THAT(str, NotNull());
  EXPECT_TRUE(_cel_is_aligned(str, cel_Allocator_kMaxAlign));
  EXPECT_EQ(std::strlen(str), std::strlen("Hello World!"));
  EXPECT_THAT(std::strcmp(str, "Hello World!"), 0);
  EXPECT_GT(actual_size, std::strlen("Hello World!"));
  cel_Allocator_FreeSized(cel_DefaultAllocator, str, actual_size);
}

}  // namespace
