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

#include "cel-c/string_view.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/cstring_view.h"
#include "cel-c/hash.h"

namespace {

using ::testing::Eq;
using ::testing::Gt;
using ::testing::Lt;

TEST(StringView, FromString) {
  static const char* const literal = "foo";

  cel_StringView str = cel_StringView_FromString(literal);

  EXPECT_EQ(cel_StringView_Data(str), literal);
  EXPECT_EQ(cel_StringView_Size(str), 3);
  EXPECT_EQ(cel_StringView_SizeInt(str), 3);
  EXPECT_FALSE(cel_StringView_Empty(str));
}

TEST(StringView, FromCString) {
  static const char* const literal = "foo";

  EXPECT_TRUE(cel_StringView_Equals(
      cel_StringView_FromCString(cel_CStringView_FromString(literal)),
      cel_StringView_FromString(literal)));
}

TEST(CStringView, Size32) {
  EXPECT_EQ(cel_StringView_Size32(cel_StringView_FromString("foo")), 3);
}

TEST(StringView, SizeInt) {
  EXPECT_EQ(cel_StringView_SizeInt(cel_StringView_FromString("foo")), 3);
}

TEST(StringView, StartsWith) {
  static const char* const haystack = "Hello World!";
  static const char* const needle = "Hello ";

  EXPECT_TRUE(cel_StringView_StartsWith(cel_StringView_FromString(haystack),
                                        cel_StringView_FromString(needle)));
}

TEST(StringView, EndsWith) {
  static const char* const haystack = "Hello World!";
  static const char* const needle = " World!";

  EXPECT_TRUE(cel_StringView_EndsWith(cel_StringView_FromString(haystack),
                                      cel_StringView_FromString(needle)));
}

TEST(StringView, ConsumePrefix) {
  static const char* const haystack = "Hello World!";
  static const char* const prefix = "Hello ";
  static const char* const suffix = "World!";

  cel_StringView subject = cel_StringView_FromString(haystack);

  EXPECT_TRUE(cel_StringView_ConsumePrefix(&subject,
                                           cel_StringView_FromString(prefix)));
  EXPECT_TRUE(
      cel_StringView_Equals(subject, cel_StringView_FromString(suffix)));

  subject = cel_StringView_FromString(haystack);
  EXPECT_FALSE(cel_StringView_ConsumePrefix(&subject,
                                            cel_StringView_FromString(suffix)));
  EXPECT_TRUE(
      cel_StringView_Equals(subject, cel_StringView_FromString(haystack)));
}

TEST(StringView, ConsumeSuffix) {
  static const char* const haystack = "Hello World!";
  static const char* const prefix = "Hello ";
  static const char* const suffix = "World!";

  cel_StringView subject = cel_StringView_FromString(haystack);

  EXPECT_TRUE(cel_StringView_ConsumeSuffix(&subject,
                                           cel_StringView_FromString(suffix)));
  EXPECT_TRUE(
      cel_StringView_Equals(subject, cel_StringView_FromString(prefix)));

  subject = cel_StringView_FromString(haystack);
  EXPECT_FALSE(cel_StringView_ConsumeSuffix(&subject,
                                            cel_StringView_FromString(prefix)));
  EXPECT_TRUE(
      cel_StringView_Equals(subject, cel_StringView_FromString(haystack)));
}

TEST(StringView, FindFirst) {
  cel_StringView haystack = cel_StringView_FromString("Hello World!");

  const char* pos =
      cel_StringView_FindFirst(haystack, cel_StringView_FromString("l"));
  EXPECT_EQ(pos, haystack.data + 2);

  pos = cel_StringView_FindFirst(haystack, cel_StringView_FromString(""));
  EXPECT_EQ(pos, haystack.data);
}

TEST(StringView, FindLast) {
  cel_StringView haystack = cel_StringView_FromString("Hello World!");

  const char* pos =
      cel_StringView_FindLast(haystack, cel_StringView_FromString("l"));
  EXPECT_EQ(pos, haystack.data + 9);

  pos = cel_StringView_FindLast(haystack, cel_StringView_FromString(""));
  EXPECT_EQ(pos, haystack.data);
}

TEST(StringView, Count) {
  cel_StringView haystack = cel_StringView_FromString("Hello World!");

  EXPECT_EQ(cel_StringView_Count(haystack, cel_StringView_FromString("l")), 3);
  EXPECT_EQ(cel_StringView_Count(haystack, cel_StringView_FromString("ll")), 1);
}

TEST(StringView, Hash) {
  EXPECT_EQ(cel_HashState_Finalize(
                cel_StringView_Hash(cel_StringView_FromString("Hello World!"),
                                    cel_HashState_Initialize())),
            cel_HashState_Finalize(cel_HashState_Combine(
                cel_HashState_Initialize(), "Hello World!")));
}

TEST(StringView, Equals) {
  EXPECT_TRUE(cel_StringView_Equals(cel_StringView_FromString("foo"),
                                    cel_StringView_FromString("foo")));
  EXPECT_FALSE(cel_StringView_Equals(cel_StringView_FromString("foo"),
                                     cel_StringView_FromString("bar")));
  EXPECT_FALSE(cel_StringView_Equals(cel_StringView_FromString("bar"),
                                     cel_StringView_FromString("foo")));
}

TEST(StringView, EqualsIgnoreCase) {
  EXPECT_TRUE(cel_StringView_EqualsIgnoreCase(
      cel_StringView_FromString("foo"), cel_StringView_FromString("foo")));
  EXPECT_TRUE(cel_StringView_EqualsIgnoreCase(
      cel_StringView_FromString("FOO"), cel_StringView_FromString("foo")));
  EXPECT_TRUE(cel_StringView_EqualsIgnoreCase(
      cel_StringView_FromString("foo"), cel_StringView_FromString("FOO")));
  EXPECT_FALSE(cel_StringView_EqualsIgnoreCase(
      cel_StringView_FromString("foo"), cel_StringView_FromString("bar")));
}

TEST(StringView, Compare) {
  EXPECT_THAT(cel_StringView_Compare(cel_StringView_FromString("foo"),
                                     cel_StringView_FromString("foo")),
              Eq(0));
  EXPECT_THAT(cel_StringView_Compare(cel_StringView_FromString("foo"),
                                     cel_StringView_FromString("bar")),
              Gt(0));
  EXPECT_THAT(cel_StringView_Compare(cel_StringView_FromString("bar"),
                                     cel_StringView_FromString("foo")),
              Lt(0));
}

TEST(StringView, Tokenize) {
  cel_StringView subject = cel_StringView_FromString("Hello World!");
  cel_StringView delim = cel_StringView_FromString("l");
  cel_StringViewTokenizer tokenizer = cel_StringView_Tokenize(subject, delim);
  cel_StringView token;

  ASSERT_TRUE(cel_StringViewTokenizer_Next(&tokenizer, &token));
  EXPECT_EQ(token, cel_StringView_FromString("He"));

  ASSERT_TRUE(cel_StringViewTokenizer_Next(&tokenizer, &token));
  EXPECT_EQ(token, cel_StringView_FromString(""));

  ASSERT_TRUE(cel_StringViewTokenizer_Next(&tokenizer, &token));
  EXPECT_EQ(token, cel_StringView_FromString("o Wor"));

  ASSERT_TRUE(cel_StringViewTokenizer_Next(&tokenizer, &token));
  EXPECT_EQ(token, cel_StringView_FromString("d!"));

  ASSERT_FALSE(cel_StringViewTokenizer_Next(&tokenizer, &token));
}

}  // namespace
