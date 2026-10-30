// Copyright 2026 Google LLC
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

#include "cel-c/string_pool.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/string_view.h"

namespace {

using ::testing::NotNull;

TEST(StringPool, InternEmpty) {
  cel_ArenaPtr arena(cel_Arena_New(cel_DefaultAllocator));
  ASSERT_THAT(arena, NotNull());
  cel_StringPoolPtr string_pool(cel_StringPool_New(arena.get()));
  ASSERT_THAT(string_pool, NotNull());
  cel_StringView str1 = cel_StringView_From("");
  cel_StringView str2 = cel_StringView_From("");
  ASSERT_TRUE(cel_StringPool_Intern(string_pool.get(), str1, &str1));
  ASSERT_TRUE(cel_StringPool_Intern(string_pool.get(), str2, &str2));
  EXPECT_EQ(cel_StringView_Data(str1), cel_StringView_Data(str2));
  EXPECT_EQ(cel_StringView_Size(str1), cel_StringView_Size(str2));
  EXPECT_TRUE(cel_StringView_Equals(str1, cel_StringView_From("")));
  EXPECT_TRUE(cel_StringView_Equals(str2, cel_StringView_From("")));
}

TEST(StringPool, Intern) {
  cel_ArenaPtr arena(cel_Arena_New(cel_DefaultAllocator));
  ASSERT_THAT(arena, NotNull());
  cel_StringPoolPtr string_pool(cel_StringPool_New(arena.get()));
  ASSERT_THAT(string_pool, NotNull());
  cel_StringView str1 = cel_StringView_From("Hello, World!");
  cel_StringView str2 = cel_StringView_From("Hello, World!");
  ASSERT_TRUE(cel_StringPool_Intern(string_pool.get(), str1, &str1));
  ASSERT_TRUE(cel_StringPool_Intern(string_pool.get(), str2, &str2));
  EXPECT_EQ(cel_StringView_Data(str1), cel_StringView_Data(str2));
  EXPECT_EQ(cel_StringView_Size(str1), cel_StringView_Size(str2));
  EXPECT_TRUE(
      cel_StringView_Equals(str1, cel_StringView_From("Hello, World!")));
  EXPECT_TRUE(
      cel_StringView_Equals(str2, cel_StringView_From("Hello, World!")));
}

}  // namespace
