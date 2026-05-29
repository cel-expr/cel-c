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

#include "cel-c/src/utf8.h"

#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/string_view.h"
#include "upb/base/string_view.h"

namespace {

using ::testing::IsEmpty;
using ::testing::TestWithParam;

struct Utf8CodecTestCase {
  char32_t code_point;
  absl::string_view code_units;
};

using Utf8CodecTest = TestWithParam<Utf8CodecTestCase>;

TEST_P(Utf8CodecTest, RoundTrip) {
  const auto& test_case = GetParam();
  char32_t pnt;
  size_t units;

  EXPECT_EQ(_cel_Utf8_EncodedSize(test_case.code_point),
            test_case.code_units.size());

  _cel_Utf8_Decode(upb_StringView_FromDataAndSize(test_case.code_units.data(),
                                                  test_case.code_units.size()),
                   &pnt, &units);

  EXPECT_EQ(test_case.code_point, pnt);
  EXPECT_THAT(test_case.code_units.substr(units), IsEmpty());

  char buffer[_cel_Utf8_kMaxEncodedSize];

  ASSERT_EQ(_cel_Utf8_Encode(pnt, buffer, sizeof(buffer)), units);

  EXPECT_EQ(absl::string_view(buffer, units), test_case.code_units);
}

INSTANTIATE_TEST_SUITE_P(Utf8CodecTest, Utf8CodecTest,
                         testing::ValuesIn<Utf8CodecTestCase>({
                             {0x0000, absl::string_view("\x00", 1)},
                             {0x0001, "\x01"},
                             {0x007e, "\x7e"},
                             {0x007f, "\x7f"},
                             {0x0080, "\xc2\x80"},
                             {0x0081, "\xc2\x81"},
                             {0x00bf, "\xc2\xbf"},
                             {0x00c0, "\xc3\x80"},
                             {0x00c1, "\xc3\x81"},
                             {0x00c8, "\xc3\x88"},
                             {0x00d0, "\xc3\x90"},
                             {0x00e0, "\xc3\xa0"},
                             {0x00f0, "\xc3\xb0"},
                             {0x00f8, "\xc3\xb8"},
                             {0x00ff, "\xc3\xbf"},
                             {0x0100, "\xc4\x80"},
                             {0x07ff, "\xdf\xbf"},
                             {0x0400, "\xd0\x80"},
                             {0x0800, "\xe0\xa0\x80"},
                             {0x0801, "\xe0\xa0\x81"},
                             {0x1000, "\xe1\x80\x80"},
                             {0xd000, "\xed\x80\x80"},
                             {0xd7ff, "\xed\x9f\xbf"},
                             {0xe000, "\xee\x80\x80"},
                             {0xfffe, "\xef\xbf\xbe"},
                             {0xffff, "\xef\xbf\xbf"},
                             {0x10000, "\xf0\x90\x80\x80"},
                             {0x10001, "\xf0\x90\x80\x81"},
                             {0x40000, "\xf1\x80\x80\x80"},
                             {0x10fffe, "\xf4\x8f\xbf\xbe"},
                             {0x10ffff, "\xf4\x8f\xbf\xbf"},
                             {0xFFFD, "\xef\xbf\xbd"},
                         }));

TEST(Utf8, DecodedSize) {
  EXPECT_EQ(_cel_Utf8_DecodedSize(upb_StringView_FromString("abcd")), 4);
  EXPECT_EQ(_cel_Utf8_DecodedSize(upb_StringView_FromString("1,2,3,4")), 7);
  EXPECT_EQ(_cel_Utf8_DecodedSize(upb_StringView_FromString(
                "\xe2\x98\xba\xe2\x98\xbb\xe2\x98\xb9")),
            3);
  EXPECT_EQ(
      _cel_Utf8_DecodedSize(upb_StringView_FromDataAndSize("\xe2\x00", 2)), 2);
  EXPECT_EQ(_cel_Utf8_DecodedSize(upb_StringView_FromString("\xe2\x80")), 2);
  EXPECT_EQ(_cel_Utf8_DecodedSize(upb_StringView_FromString("a\xe2\x80")), 3);
}

TEST(Utf8, IsValid) {
  EXPECT_TRUE(_cel_Utf8_IsValid(upb_StringView_FromString("")));
  EXPECT_TRUE(_cel_Utf8_IsValid(upb_StringView_FromString("a")));
  EXPECT_TRUE(_cel_Utf8_IsValid(upb_StringView_FromString("abc")));
  EXPECT_TRUE(_cel_Utf8_IsValid(upb_StringView_FromString("\xd0\x96")));
  EXPECT_TRUE(_cel_Utf8_IsValid(upb_StringView_FromString("\xd0\x96\xd0\x96")));
  EXPECT_TRUE(_cel_Utf8_IsValid(upb_StringView_FromString(
      "\xd0\xb1\xd1\x80\xd1\x8d\xd0\xb4-\xd0\x9b\xd0\x93\xd0\xa2\xd0\x9c")));
  EXPECT_TRUE(_cel_Utf8_IsValid(
      upb_StringView_FromString("\xe2\x98\xba\xe2\x98\xbb\xe2\x98\xb9")));
  EXPECT_TRUE(_cel_Utf8_IsValid(upb_StringView_FromString("a\ufffdb")));
  EXPECT_TRUE(_cel_Utf8_IsValid(upb_StringView_FromString("\xf4\x8f\xbf\xbf")));

  EXPECT_FALSE(_cel_Utf8_IsValid(upb_StringView_FromString("\x42\xfa")));
  EXPECT_FALSE(_cel_Utf8_IsValid(upb_StringView_FromString("\x42\xfa\x43")));
  EXPECT_FALSE(
      _cel_Utf8_IsValid(upb_StringView_FromString("\xf4\x90\x80\x80")));
  EXPECT_FALSE(
      _cel_Utf8_IsValid(upb_StringView_FromString("\xf7\xbf\xbf\xbf")));
  EXPECT_FALSE(
      _cel_Utf8_IsValid(upb_StringView_FromString("\xfb\xbf\xbf\xbf\xbf")));
  EXPECT_FALSE(_cel_Utf8_IsValid(upb_StringView_FromString("\xc0\x80")));
  EXPECT_FALSE(_cel_Utf8_IsValid(upb_StringView_FromString("\xed\xa0\x80")));
  EXPECT_FALSE(_cel_Utf8_IsValid(upb_StringView_FromString("\xed\xbf\xbf")));
}

}  // namespace
