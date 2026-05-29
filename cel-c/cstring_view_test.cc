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

#include "cel-c/cstring_view.h"

#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/hash.h"
#include "cel-c/string_view.h"

namespace {

using ::testing::Eq;
using ::testing::Gt;
using ::testing::Lt;

TEST(CStringView, FromString) {
  static const char* const literal = "foo";

  cel_CStringView str = cel_CStringView_FromString(literal);

  EXPECT_EQ(cel_CStringView_Data(str), literal);
  EXPECT_EQ(cel_CStringView_Size(str), 3);
  EXPECT_FALSE(cel_CStringView_Empty(str));
}

TEST(CStringView, Size32) {
  EXPECT_EQ(cel_CStringView_Size32(cel_CStringView_FromString("foo")), 3);
}

TEST(CStringView, SizeInt) {
  EXPECT_EQ(cel_CStringView_SizeInt(cel_CStringView_FromString("foo")), 3);
}

TEST(CStringView, StartsWith) {
  static const char* const haystack = "Hello World!";
  static const char* const needle = "Hello ";

  EXPECT_TRUE(cel_CStringView_StartsWith(cel_CStringView_FromString(haystack),
                                         cel_CStringView_FromString(needle)));
}

TEST(CStringView, ConsumePrefix) {
  static const char* const haystack = "Hello World!";
  static const char* const prefix = "Hello ";
  static const char* const suffix = "World!";

  cel_CStringView subject = cel_CStringView_FromString(haystack);

  EXPECT_TRUE(cel_CStringView_ConsumePrefix(
      &subject, cel_CStringView_FromString(prefix)));
  EXPECT_TRUE(
      cel_CStringView_Equals(subject, cel_CStringView_FromString(suffix)));

  subject = cel_CStringView_FromString(haystack);
  EXPECT_FALSE(cel_CStringView_ConsumePrefix(
      &subject, cel_CStringView_FromString(suffix)));
  EXPECT_TRUE(
      cel_CStringView_Equals(subject, cel_CStringView_FromString(haystack)));
}

TEST(CStringView, FindFirst) {
  cel_CStringView haystack = cel_CStringView_FromString("Hello World!");

  const char* pos =
      cel_CStringView_FindFirst(haystack, cel_CStringView_FromString("l"));
  EXPECT_EQ(pos, haystack.data + 2);

  pos = cel_CStringView_FindFirst(haystack, cel_CStringView_FromString(""));
  EXPECT_EQ(pos, haystack.data);
}

TEST(CStringView, Count) {
  cel_CStringView haystack = cel_CStringView_FromString("Hello World!");

  EXPECT_EQ(cel_CStringView_Count(haystack, cel_CStringView_FromString("l")),
            3);
  EXPECT_EQ(cel_CStringView_Count(haystack, cel_CStringView_FromString("ll")),
            1);
}

TEST(CStringView, Hash) {
  EXPECT_EQ(cel_HashState_Finalize(
                cel_CStringView_Hash(cel_CStringView_FromString("Hello World!"),
                                     cel_HashState_Initialize())),
            cel_HashState_Finalize(cel_HashState_Combine(
                cel_HashState_Initialize(), "Hello World!")));
}

TEST(CStringView, Equals) {
  EXPECT_TRUE(cel_CStringView_Equals(cel_CStringView_FromString("foo"),
                                     cel_CStringView_FromString("foo")));
  EXPECT_FALSE(cel_CStringView_Equals(cel_CStringView_FromString("foo"),
                                      cel_CStringView_FromString("bar")));
  EXPECT_FALSE(cel_CStringView_Equals(cel_CStringView_FromString("bar"),
                                      cel_CStringView_FromString("foo")));
}

TEST(CStringView, EqualsIgnoreCase) {
  EXPECT_TRUE(cel_CStringView_EqualsIgnoreCase(
      cel_CStringView_FromString("foo"), cel_CStringView_FromString("foo")));
  EXPECT_TRUE(cel_CStringView_EqualsIgnoreCase(
      cel_CStringView_FromString("FOO"), cel_CStringView_FromString("foo")));
  EXPECT_TRUE(cel_CStringView_EqualsIgnoreCase(
      cel_CStringView_FromString("foo"), cel_CStringView_FromString("FOO")));
  EXPECT_FALSE(cel_CStringView_EqualsIgnoreCase(
      cel_CStringView_FromString("foo"), cel_CStringView_FromString("bar")));
}

TEST(CStringView, Compare) {
  EXPECT_THAT(cel_CStringView_Compare(cel_CStringView_FromString("foo"),
                                      cel_CStringView_FromString("foo")),
              Eq(0));
  EXPECT_THAT(cel_CStringView_Compare(cel_CStringView_FromString("foo"),
                                      cel_CStringView_FromString("bar")),
              Gt(0));
  EXPECT_THAT(cel_CStringView_Compare(cel_CStringView_FromString("bar"),
                                      cel_CStringView_FromString("foo")),
              Lt(0));
}

TEST(CStringView, Tokenize) {
  cel_CStringView subject = cel_CStringView_FromString("Hello World!");
  cel_CStringView delim = cel_CStringView_FromString("l");
  cel_CStringViewTokenizer tokenizer = cel_CStringView_Tokenize(subject, delim);
  cel_CStringView token;
  size_t token_len;

  ASSERT_TRUE(cel_CStringViewTokenizer_Next(&tokenizer, &token, &token_len));
  EXPECT_EQ(cel_StringView_FromArray(cel_CStringView_Data(token), token_len),
            cel_CStringView_FromString("He"));

  ASSERT_TRUE(cel_CStringViewTokenizer_Next(&tokenizer, &token, &token_len));
  EXPECT_EQ(cel_StringView_FromArray(cel_CStringView_Data(token), token_len),
            cel_CStringView_FromString(""));

  ASSERT_TRUE(cel_CStringViewTokenizer_Next(&tokenizer, &token, &token_len));
  EXPECT_EQ(cel_StringView_FromArray(cel_CStringView_Data(token), token_len),
            cel_CStringView_FromString("o Wor"));

  ASSERT_TRUE(cel_CStringViewTokenizer_Next(&tokenizer, &token, &token_len));
  EXPECT_EQ(cel_StringView_FromArray(cel_CStringView_Data(token), token_len),
            cel_CStringView_FromString("d!"));

  ASSERT_FALSE(cel_CStringViewTokenizer_Next(&tokenizer, &token, &token_len));
}

}  // namespace
