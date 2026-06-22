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

#include "cel-c/internal/ctype.h"

#include <cstring>

#include "gtest/gtest.h"

namespace {

TEST(CType, Properties) {
  for (int i = 0; i < 256; ++i) {
    const unsigned char c = static_cast<unsigned char>(i);
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
      EXPECT_TRUE(_cel_isalpha(c));
    } else {
      EXPECT_FALSE(_cel_isalpha(c));
    }
    if (_cel_isalpha(c) || _cel_isdigit(c)) {
      EXPECT_TRUE(_cel_isalnum(c));
    } else {
      EXPECT_FALSE(_cel_isalnum(c));
    }
    if (c != '\0' && strchr(" \r\n\t\v\f", i)) {
      EXPECT_TRUE(_cel_isspace(c));
    } else {
      EXPECT_FALSE(_cel_isspace(c));
    }
    if (_cel_isprint(c) && !_cel_isspace(c) && !_cel_isalnum(c)) {
      EXPECT_TRUE(_cel_ispunct(c));
    } else {
      EXPECT_FALSE(_cel_ispunct(c));
    }
    if (c == ' ' || c == '\t') {
      EXPECT_TRUE(_cel_isblank(c));
    } else {
      EXPECT_FALSE(_cel_isblank(c));
    }
    if (c < 32 || c == 127) {
      EXPECT_TRUE(_cel_iscntrl(c));
    } else {
      EXPECT_FALSE(_cel_iscntrl(c));
    }
    if (_cel_isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
      EXPECT_TRUE(_cel_isxdigit(c));
    } else {
      EXPECT_FALSE(_cel_isxdigit(c));
    }
    if (c >= '0' && c <= '9') {
      EXPECT_TRUE(_cel_isdigit(c));
    } else {
      EXPECT_FALSE(_cel_isdigit(c));
    }
    if (c >= 32 && c < 127) {
      EXPECT_TRUE(_cel_isprint(c));
    } else {
      EXPECT_FALSE(_cel_isprint(c));
    }
    if (c > 32 && c < 127) {
      EXPECT_TRUE(_cel_isgraph(c));
    } else {
      EXPECT_FALSE(_cel_isgraph(c));
    }
    if (c >= 'a' && c <= 'z') {
      EXPECT_TRUE(_cel_islower(c));
    } else {
      EXPECT_FALSE(_cel_islower(c));
    }
    if (c >= 'A' && c <= 'Z') {
      EXPECT_TRUE(_cel_isupper(c));
    } else {
      EXPECT_FALSE(_cel_isupper(c));
    }
    if (c < 128) {
      EXPECT_TRUE(_cel_isascii(c));
    } else {
      EXPECT_FALSE(_cel_isascii(c));
    }
    if (_cel_isupper(c)) {
      EXPECT_EQ(_cel_tolower(c), static_cast<char>((c - 'A') + 'a'));
    } else {
      EXPECT_EQ(_cel_tolower(c), static_cast<char>(c));
    }
    if (_cel_islower(c)) {
      EXPECT_EQ(_cel_toupper(c), static_cast<char>((c - 'a') + 'A'));
    } else {
      EXPECT_EQ(_cel_toupper(c), static_cast<char>(c));
    }
  }
}

}  // namespace
