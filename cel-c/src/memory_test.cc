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

#include "cel-c/src/memory.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/string_view.h"

namespace {

using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsNull;
using ::testing::Lt;
using ::testing::PrintToStringParamName;
using ::testing::TestWithParam;
using ::testing::ValuesIn;

struct EqualsTestCase {
  absl::string_view name;
  absl::string_view lhs;
  absl::string_view rhs;
  bool result;

  template <typename S>
  friend void AbslStringify(S& sink, const EqualsTestCase& test_case) {
    sink.Append(test_case.name);
  }
};

using EqualsTest = TestWithParam<EqualsTestCase>;

TEST_P(EqualsTest, Equals) {
  const auto& test_case = GetParam();
  EXPECT_THAT(_cel_Memory_Equals(test_case.lhs.data(), test_case.lhs.size(),
                                 test_case.rhs.data(), test_case.rhs.size()),
              Eq(test_case.result))
      << test_case.lhs << " == " << test_case.rhs;
}

INSTANTIATE_TEST_SUITE_P(
    EqualsTest, EqualsTest,
    ValuesIn<EqualsTestCase>({
        {.name = "Empty", .lhs = "", .rhs = "", .result = true},
        {// Force different memory addresses by using string literals with
         // different suffixes and trimming the suffix.
         .name = "SameContent",
         .lhs = absl::string_view("Hello World! foo").substr(0, 12),
         .rhs = absl::string_view("Hello World! bar").substr(0, 12),
         .result = true},
        {.name = "SameAddress",
         .lhs = "Hello World!",
         .rhs = "Hello World!",
         .result = true},
        {.name = "LeftEmpty",
         .lhs = "",
         .rhs = "Hello World!",
         .result = false},
        {.name = "RightEmpty",
         .lhs = "Hello World!",
         .rhs = "",
         .result = false},
        {.name = "SameLength", .lhs = "foo", .rhs = "bar", .result = false},
        {.name = "CommonPrefix",
         .lhs = "foofoo",
         .rhs = "foo",
         .result = false},
    }),
    PrintToStringParamName());

struct CompareTestCase {
  absl::string_view name;
  absl::string_view lhs;
  absl::string_view rhs;
  int result;

  template <typename S>
  friend void AbslStringify(S& sink, const CompareTestCase& test_case) {
    sink.Append(test_case.name);
  }
};

using CompareTest = TestWithParam<CompareTestCase>;

TEST_P(CompareTest, Compare) {
  const auto& test_case = GetParam();
  if (test_case.result < 0) {
    EXPECT_THAT(_cel_Memory_Compare(test_case.lhs.data(), test_case.lhs.size(),
                                    test_case.rhs.data(), test_case.rhs.size()),
                Lt(0))
        << test_case.lhs << " < " << test_case.rhs;
  } else if (test_case.result > 0) {
    EXPECT_THAT(_cel_Memory_Compare(test_case.lhs.data(), test_case.lhs.size(),
                                    test_case.rhs.data(), test_case.rhs.size()),
                Gt(0))
        << test_case.lhs << " > " << test_case.rhs;
  } else {
    EXPECT_THAT(_cel_Memory_Compare(test_case.lhs.data(), test_case.lhs.size(),
                                    test_case.rhs.data(), test_case.rhs.size()),
                Eq(0))
        << test_case.lhs << " == " << test_case.rhs;
  }
}

INSTANTIATE_TEST_SUITE_P(
    CompareTest, CompareTest,
    ValuesIn<CompareTestCase>({
        {.name = "Empty", .lhs = "", .rhs = "", .result = 0},
        {// Force different memory addresses by using string literals with
         // different suffixes and trimming the suffix.
         .name = "SameContent",
         .lhs = absl::string_view("Hello World! foo").substr(0, 12),
         .rhs = absl::string_view("Hello World! bar").substr(0, 12),
         .result = 0},
        {.name = "SameAddress",
         .lhs = "Hello World!",
         .rhs = "Hello World!",
         .result = 0},
        {.name = "LeftEmpty", .lhs = "", .rhs = "Hello World!", .result = -1},
        {.name = "RightEmpty", .lhs = "Hello World!", .rhs = "", .result = 1},
        {.name = "Greater", .lhs = "foo", .rhs = "bar", .result = 1},
        {.name = "Less", .lhs = "bar", .rhs = "foo", .result = -1},
        {.name = "CommonPrefixGreater",
         .lhs = "foofoo",
         .rhs = "foo",
         .result = 1},
        {.name = "CommonPrefixLess",
         .lhs = "foo",
         .rhs = "foofoo",
         .result = -1},
    }),
    PrintToStringParamName());

struct StartsWithTestCase {
  absl::string_view name;
  absl::string_view haystack;
  absl::string_view needle;
  bool result;

  template <typename S>
  friend void AbslStringify(S& sink, const StartsWithTestCase& test_case) {
    sink.Append(test_case.name);
  }
};

using StartsWithTest = TestWithParam<StartsWithTestCase>;

TEST_P(StartsWithTest, StartsWith) {
  const auto& test_case = GetParam();
  EXPECT_THAT(_cel_Memory_StartsWith(
                  test_case.haystack.data(), test_case.haystack.size(),
                  test_case.needle.data(), test_case.needle.size()),
              Eq(test_case.result));
}

INSTANTIATE_TEST_SUITE_P(
    StartsWithTest, StartsWithTest,
    ValuesIn<StartsWithTestCase>({
        {.name = "Empty", .haystack = "", .needle = "", .result = true},
        {.name = "EmptyHaystack",
         .haystack = "",
         .needle = "foo",
         .result = false},
        {.name = "EmptyNeedle",
         .haystack = "foo",
         .needle = "",
         .result = true},
        {.name = "LargerNeedle",
         .haystack = "foo",
         .needle = "Hello World!",
         .result = false},
        {// Force different memory addresses by using string literals with
         // different suffixes and trimming the suffix.
         .name = "SameContent",
         .haystack = absl::string_view("Hello World! foo").substr(0, 12),
         .needle = absl::string_view("Hello World! bar").substr(0, 12),
         .result = true},
        {.name = "SameAddress",
         .haystack = "Hello World!",
         .needle = "Hello World!",
         .result = true},
        {.name = "Prefix",
         .haystack = "foofoo",
         .needle = "foo",
         .result = true},
    }),
    PrintToStringParamName());

struct EndsWithTestCase {
  absl::string_view name;
  absl::string_view haystack;
  absl::string_view needle;
  bool result;

  template <typename S>
  friend void AbslStringify(S& sink, const EndsWithTestCase& test_case) {
    sink.Append(test_case.name);
  }
};

using EndsWithTest = TestWithParam<EndsWithTestCase>;

TEST_P(EndsWithTest, EndsWith) {
  const auto& test_case = GetParam();
  EXPECT_THAT(
      _cel_Memory_EndsWith(test_case.haystack.data(), test_case.haystack.size(),
                           test_case.needle.data(), test_case.needle.size()),
      Eq(test_case.result));
}

INSTANTIATE_TEST_SUITE_P(
    EndsWithTest, EndsWithTest,
    ValuesIn<EndsWithTestCase>({
        {.name = "Empty", .haystack = "", .needle = "", .result = true},
        {.name = "EmptyHaystack",
         .haystack = "",
         .needle = "foo",
         .result = false},
        {.name = "EmptyNeedle",
         .haystack = "foo",
         .needle = "",
         .result = true},
        {.name = "LargerNeedle",
         .haystack = "foo",
         .needle = "Hello World!",
         .result = false},
        {// Force different memory addresses by using string literals with
         // different suffixes and trimming the suffix.
         .name = "SameContent",
         .haystack = absl::string_view("Hello World! foo").substr(0, 12),
         .needle = absl::string_view("Hello World! bar").substr(0, 12),
         .result = true},
        {.name = "SameAddress",
         .haystack = "Hello World!",
         .needle = "Hello World!",
         .result = true},
        {.name = "Suffix",
         .haystack = "foofoo",
         .needle = "foo",
         .result = true},
    }),
    PrintToStringParamName());

struct FindFirstTestCase {
  absl::string_view name;
  absl::string_view haystack;
  absl::string_view needle;
  absl::string_view::size_type result;

  template <typename S>
  friend void AbslStringify(S& sink, const FindFirstTestCase& test_case) {
    sink.Append(test_case.name);
  }
};

using FindFirstTest = TestWithParam<FindFirstTestCase>;

TEST_P(FindFirstTest, FindFirst) {
  const auto& test_case = GetParam();
  if (test_case.result != absl::string_view::npos) {
    EXPECT_THAT(_cel_Memory_FindFirst(
                    test_case.haystack.data(), test_case.haystack.size(),
                    test_case.needle.data(), test_case.needle.size()),
                Eq(test_case.haystack.data() + test_case.result))
        << test_case.haystack << "[" << test_case.needle << ":]";
  } else {
    EXPECT_THAT(_cel_Memory_FindFirst(
                    test_case.haystack.data(), test_case.haystack.size(),
                    test_case.needle.data(), test_case.needle.size()),
                IsNull())
        << test_case.haystack << "[" << test_case.needle << ":]";
  }
}

INSTANTIATE_TEST_SUITE_P(
    FindFirstTest, FindFirstTest,
    ValuesIn<FindFirstTestCase>({
        {.name = "Empty", .haystack = "", .needle = "", .result = 0},
        {.name = "EmptyHaystack",
         .haystack = "",
         .needle = "foo",
         .result = absl::string_view::npos},
        {.name = "EmptyNeedle", .haystack = "foo", .needle = "", .result = 0},
        {.name = "LargerNeedle",
         .haystack = "foo",
         .needle = "Hello World!",
         .result = absl::string_view::npos},
        {// Force different memory addresses by using string literals with
         // different suffixes and trimming the suffix.
         .name = "SameContent",
         .haystack = absl::string_view("Hello World! foo").substr(0, 12),
         .needle = absl::string_view("Hello World! bar").substr(0, 12),
         .result = 0},
        {.name = "SameAddress",
         .haystack = "Hello World!",
         .needle = "Hello World!",
         .result = 0},
        {.name = "Prefix", .haystack = "foofoo", .needle = "foo", .result = 0},
        {.name = "CommonPrefixFind",
         .haystack = "foffoofoo",
         .needle = "foo",
         .result = 3},
        {.name = "CommonPrefixMiss",
         .haystack = "fofoffo",
         .needle = "foo",
         .result = absl::string_view::npos},
    }),
    PrintToStringParamName());

struct FindLastTestCase {
  absl::string_view name;
  absl::string_view haystack;
  absl::string_view needle;
  absl::string_view::size_type result;

  template <typename S>
  friend void AbslStringify(S& sink, const FindLastTestCase& test_case) {
    sink.Append(test_case.name);
  }
};

using FindLastTest = TestWithParam<FindLastTestCase>;

TEST_P(FindLastTest, FindLast) {
  const auto& test_case = GetParam();
  if (test_case.result != absl::string_view::npos) {
    EXPECT_THAT(_cel_Memory_FindLast(
                    test_case.haystack.data(), test_case.haystack.size(),
                    test_case.needle.data(), test_case.needle.size()),
                Eq(test_case.haystack.data() + test_case.result))
        << test_case.haystack << "[:" << test_case.needle << "]";
  } else {
    EXPECT_THAT(_cel_Memory_FindLast(
                    test_case.haystack.data(), test_case.haystack.size(),
                    test_case.needle.data(), test_case.needle.size()),
                IsNull())
        << test_case.haystack << "[:" << test_case.needle << "]";
  }
}

INSTANTIATE_TEST_SUITE_P(
    FindLastTest, FindLastTest,
    ValuesIn<FindLastTestCase>({
        {.name = "Empty", .haystack = "", .needle = "", .result = 0},
        {.name = "EmptyHaystack",
         .haystack = "",
         .needle = "foo",
         .result = absl::string_view::npos},
        {.name = "EmptyNeedle", .haystack = "foo", .needle = "", .result = 0},
        {.name = "LargerNeedle",
         .haystack = "foo",
         .needle = "Hello World!",
         .result = absl::string_view::npos},
        {// Force different memory addresses by using string literals with
         // different suffixes and trimming the suffix.
         .name = "SameContent",
         .haystack = absl::string_view("Hello World! foo").substr(0, 12),
         .needle = absl::string_view("Hello World! bar").substr(0, 12),
         .result = 0},
        {.name = "SameAddress",
         .haystack = "Hello World!",
         .needle = "Hello World!",
         .result = 0},
        {.name = "Suffix", .haystack = "foofoo", .needle = "foo", .result = 3},
        {.name = "CommonPrefixFind",
         .haystack = "foofooffo",
         .needle = "foo",
         .result = 3},
        {.name = "CommonPrefixMiss",
         .haystack = "fofoffo",
         .needle = "foo",
         .result = absl::string_view::npos},
    }),
    PrintToStringParamName());

}  // namespace
