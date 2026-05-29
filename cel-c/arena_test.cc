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

#include "cel-c/arena.h"

#include <cstddef>
#include <cstring>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/alloc.h"
#include "cel-c/config.h"
#include "upb/base/string_view.h"

namespace {

using ::testing::IsNull;
using ::testing::NotNull;

TEST(Arena, Malloc) {
  CEL_NULLABLE(cel_Arena*) arena = cel_Arena_New(cel_DefaultAllocator);
  ASSERT_THAT(arena, NotNull());

  size_t actual_size;
  EXPECT_THAT(cel_Arena_Malloc(arena, 0, &actual_size), IsNull());
  EXPECT_EQ(actual_size, 0);

  CEL_NULLABLE(void*)
  addr = cel_Arena_Malloc(arena, 16, &actual_size);
  EXPECT_THAT(addr, NotNull());
  EXPECT_GE(actual_size, 16);

  cel_Arena_FreeSized(arena, cel_nullptr, 0);

  cel_Arena_Delete(arena);
}

TEST(Arena, Calloc) {
  CEL_NULLABLE(cel_Arena*) arena = cel_Arena_New(cel_DefaultAllocator);
  ASSERT_THAT(arena, NotNull());

  size_t actual_num;
  EXPECT_THAT(cel_Arena_Calloc(arena, 0, 0, &actual_num), IsNull());
  EXPECT_EQ(actual_num, 0);
  EXPECT_THAT(cel_Arena_Calloc(arena, 1, 0, &actual_num), IsNull());
  EXPECT_EQ(actual_num, 0);
  EXPECT_THAT(cel_Arena_Calloc(arena, 0, 1, &actual_num), IsNull());
  EXPECT_EQ(actual_num, 0);

  CEL_NULLABLE(void*)
  addr = cel_Arena_Calloc(arena, 1, 16, &actual_num);
  EXPECT_THAT(addr, NotNull());
  EXPECT_EQ(actual_num, 1);

  cel_Arena_FreeSized(arena, cel_nullptr, 0);

  cel_Arena_Delete(arena);
}

TEST(Arena, Realloc) {
  CEL_NULLABLE(cel_Arena*) arena = cel_Arena_New(cel_DefaultAllocator);
  ASSERT_THAT(arena, NotNull());

  size_t actual_size;
  CEL_NULLABLE(void*)
  addr = cel_Arena_Realloc(arena, cel_nullptr, 0, 16, &actual_size);
  EXPECT_THAT(addr, NotNull());
  EXPECT_GE(actual_size, 16);

  addr = cel_Arena_Realloc(arena, addr, 16, 32, &actual_size);
  EXPECT_THAT(addr, NotNull());
  EXPECT_GE(actual_size, 32);

  cel_Arena_FreeSized(arena, cel_nullptr, 0);

  EXPECT_THAT(cel_Arena_Realloc(arena, addr, 32, 0, &actual_size), IsNull());
  EXPECT_EQ(actual_size, 0);

  cel_Arena_Delete(arena);
}

TEST(Arena, StrDup) {
  CEL_NULLABLE(cel_Arena*) arena = cel_Arena_New(cel_DefaultAllocator);
  ASSERT_THAT(arena, NotNull());

  upb_StringView out;
  ASSERT_TRUE(cel_Arena_StrDup(arena, &out, upb_StringView_FromString("")));
  EXPECT_THAT(out.data, NotNull());
  EXPECT_THAT(out.size, 0);

  ASSERT_TRUE(
      cel_Arena_StrDup(arena, &out, upb_StringView_FromString("Hello World!")));
  EXPECT_THAT(out.data, NotNull());
  EXPECT_THAT(out.size, std::strlen("Hello World!"));
  EXPECT_TRUE(
      upb_StringView_IsEqual(out, upb_StringView_FromString("Hello World!")));

  cel_Arena_Delete(arena);
}

TEST(Arena, PrintF) {
  CEL_NULLABLE(cel_Arena*) arena = cel_Arena_New(cel_DefaultAllocator);
  ASSERT_THAT(arena, NotNull());

  upb_StringView out;
  ASSERT_TRUE(cel_Arena_PrintF(arena, &out, "%s", "Hello World!"));
  EXPECT_THAT(out.data, NotNull());
  EXPECT_THAT(out.size, std::strlen("Hello World!"));
  EXPECT_TRUE(
      upb_StringView_IsEqual(out, upb_StringView_FromString("Hello World!")));

  cel_Arena_Delete(arena);
}

}  // namespace
