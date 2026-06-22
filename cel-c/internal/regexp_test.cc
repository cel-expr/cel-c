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

#include "cel-c/internal/regexp.h"

#include <memory>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/config.h"
#include "cel-c/status.h"
#include "cel-c/status_code.h"
#include "cel-c/string_view.h"

namespace {

using ::testing::IsNull;
using ::testing::NotNull;

struct RegExpDeleter {
  void operator()(_cel_RegExp* cel_nullable regexp) const {
    _cel_RegExp_Delete(regexp);
  }
};

using RegExpPtr = std::unique_ptr<_cel_RegExp, RegExpDeleter>;

class RegExpTest : public ::testing::Test {
 public:
  void SetUp() override { cel_Status_Construct(&status_); }

  void TearDown() override { cel_Status_Destruct(&status_); }

 protected:
  CEL_NONNULL(cel_Status*) status() { return &status_; }

 private:
  cel_Status status_;
};

TEST_F(RegExpTest, FullMatch) {
  _cel_RegExpOptions options;
  _cel_RegExpOptions_Construct(&options);

  RegExpPtr regexp(
      _cel_RegExp_New(cel_StringView_From("(\\s*)Hello(\\s+)World!(\\s*)"),
                      &options, status()));
  ASSERT_THAT(regexp, NotNull());

  cel_StringView captures[3];

  EXPECT_TRUE(_cel_RegExp_FullMatch(
      regexp.get(), cel_StringView_From(" Hello  World!   "), status()));

  ASSERT_TRUE(_cel_RegExp_FullMatch(regexp.get(),
                                    cel_StringView_From(" Hello  World!   "),
                                    status(), &captures[0]));
  EXPECT_TRUE(cel_StringView_Equals(captures[0], cel_StringView_From(" ")));

  ASSERT_TRUE(_cel_RegExp_FullMatch(regexp.get(),
                                    cel_StringView_From(" Hello  World!   "),
                                    status(), &captures[0], &captures[1]));
  EXPECT_TRUE(cel_StringView_Equals(captures[0], cel_StringView_From(" ")));
  EXPECT_TRUE(cel_StringView_Equals(captures[1], cel_StringView_From("  ")));

  ASSERT_TRUE(_cel_RegExp_FullMatch(
      regexp.get(), cel_StringView_From(" Hello  World!   "), status(),
      &captures[0], &captures[1], &captures[2]));
  EXPECT_TRUE(cel_StringView_Equals(captures[0], cel_StringView_From(" ")));
  EXPECT_TRUE(cel_StringView_Equals(captures[1], cel_StringView_From("  ")));
  EXPECT_TRUE(cel_StringView_Equals(captures[2], cel_StringView_From("   ")));

  EXPECT_FALSE(_cel_RegExp_FullMatch(
      regexp.get(), cel_StringView_From(" Hello  World!   Combo breaker!"),
      status()));
}

TEST_F(RegExpTest, PartialMatch) {
  _cel_RegExpOptions options;
  _cel_RegExpOptions_Construct(&options);

  RegExpPtr regexp(
      _cel_RegExp_New(cel_StringView_From("(\\s*)Hello(\\s+)World!(\\s*)"),
                      &options, status()));
  ASSERT_THAT(regexp, NotNull());

  cel_StringView captures[3];

  EXPECT_TRUE(_cel_RegExp_PartialMatch(
      regexp.get(), cel_StringView_From(" Hello  World!   "), status()));

  ASSERT_TRUE(_cel_RegExp_PartialMatch(regexp.get(),
                                       cel_StringView_From(" Hello  World!   "),
                                       status(), &captures[0]));
  EXPECT_TRUE(cel_StringView_Equals(captures[0], cel_StringView_From(" ")));

  ASSERT_TRUE(_cel_RegExp_PartialMatch(regexp.get(),
                                       cel_StringView_From(" Hello  World!   "),
                                       status(), &captures[0], &captures[1]));
  EXPECT_TRUE(cel_StringView_Equals(captures[0], cel_StringView_From(" ")));
  EXPECT_TRUE(cel_StringView_Equals(captures[1], cel_StringView_From("  ")));

  ASSERT_TRUE(_cel_RegExp_PartialMatch(
      regexp.get(), cel_StringView_From(" Hello  World!   "), status(),
      &captures[0], &captures[1], &captures[2]));
  EXPECT_TRUE(cel_StringView_Equals(captures[0], cel_StringView_From(" ")));
  EXPECT_TRUE(cel_StringView_Equals(captures[1], cel_StringView_From("  ")));
  EXPECT_TRUE(cel_StringView_Equals(captures[2], cel_StringView_From("   ")));

  EXPECT_TRUE(_cel_RegExp_PartialMatch(
      regexp.get(), cel_StringView_From(" Hello  World!   Combo breaker!"),
      status()));

  EXPECT_TRUE(_cel_RegExp_PartialMatch(
      regexp.get(), cel_StringView_From("Combo breaker! Hello  World!   "),
      status()));

  EXPECT_FALSE(_cel_RegExp_PartialMatch(
      regexp.get(), cel_StringView_From(" Hello Combo breaker! World!   "),
      status()));
}

TEST_F(RegExpTest, BadPattern) {
  _cel_RegExpOptions options;
  _cel_RegExpOptions_Construct(&options);

  RegExpPtr regexp(
      _cel_RegExp_New(cel_StringView_From("("), &options, status()));
  ASSERT_THAT(regexp, IsNull());
  EXPECT_EQ(cel_Status_CanonicalCode(status()),
            cel_StatusCode_kInvalidArgument);
  EXPECT_FALSE(cel_StringView_Empty(cel_Status_Message(status())));
}

TEST_F(RegExpTest, Matches) {
  _cel_RegExpOptions options;
  _cel_RegExpOptions_Construct(&options);

  EXPECT_TRUE(_cel_RegExp_Matches(
      cel_StringView_From("(\\s*)Hello(\\s+)World!(\\s*)"), &options,
      cel_StringView_From(" Hello  World!   "), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(_cel_RegExp_Matches(
      cel_StringView_From("(\\s*)Hello(\\s+)World!(\\s*)"), &options,
      cel_StringView_From(" Hello  World!   "), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(_cel_RegExp_Matches(
      cel_StringView_From("(\\s*)Hello(\\s+)World!(\\s*)"), &options,
      cel_StringView_From(" Hello  World!   "), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(_cel_RegExp_Matches(
      cel_StringView_From("(\\s*)Hello(\\s+)World!(\\s*)"), &options,
      cel_StringView_From(" Hello  World!   "), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(_cel_RegExp_Matches(
      cel_StringView_From("(\\s*)Hello(\\s+)World!(\\s*)"), &options,
      cel_StringView_From(" Hello  World!   Combo breaker!"), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(_cel_RegExp_Matches(
      cel_StringView_From("(\\s*)Hello(\\s+)World!(\\s*)"), &options,
      cel_StringView_From("Combo breaker! Hello  World!   "), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_FALSE(_cel_RegExp_Matches(
      cel_StringView_From("(\\s*)Hello(\\s+)World!(\\s*)"), &options,
      cel_StringView_From(" Hello Combo breaker! World!   "), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
}

}  // namespace
