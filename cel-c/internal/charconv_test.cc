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

#include "cel-c/internal/charconv.h"

#include <clocale>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "gtest/gtest.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

namespace {

class ScopedLocale {
 public:
  ScopedLocale() = delete;
  ScopedLocale(const ScopedLocale&) = delete;
  ScopedLocale& operator=(const ScopedLocale&) = delete;
  ScopedLocale& operator=(ScopedLocale&&) = delete;

  explicit ScopedLocale(const char* locale)
      : locale_(setlocale(LC_ALL, locale)) {}

  ScopedLocale(ScopedLocale&& other)
      : locale_(std::exchange(other.locale_, nullptr)) {}

  ~ScopedLocale() {
    if (locale_ != nullptr) {
      setlocale(LC_ALL, locale_);
    }
  }

  explicit operator bool() const noexcept { return locale_ != nullptr; }

 private:
  char* locale_;
};

template <typename T>
std::enable_if_t<std::is_integral_v<T>, int> FromChars(std::string_view* string,
                                                       T* val, int base = 10) {
  _cel_FromCharsResult result = _cel_FromChars(
      string->data(), string->data() + string->size(), val, base);
  if (result.ptr != nullptr) {
    string->remove_prefix(result.ptr - string->data());
  }
  return result.ec;
}

template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, int> FromChars(
    std::string_view* string, T* val) {
  _cel_FromCharsResult result =
      _cel_FromChars(string->data(), string->data() + string->size(), val);
  if (result.ptr != nullptr) {
    string->remove_prefix(result.ptr - string->data());
  }
  return result.ec;
}

template <typename T>
std::enable_if_t<std::is_integral_v<T>, std::string_view> ToChars(
    std::string* string, T val, int base) {
  string->resize(_CEL_MAX_INT_CHARS);
  string->resize(_cel_ToChars(string->data(), val, base));
  return *string;
}

template <typename T>
std::enable_if_t<std::is_same_v<T, float>, std::string_view> ToChars(
    std::string* string, T val) {
  string->resize(_CEL_MAX_FLOAT_CHARS);
  string->resize(_cel_ToChars(string->data(), val));
  return *string;
}

template <typename T>
std::enable_if_t<std::is_same_v<T, double>, std::string_view> ToChars(
    std::string* string, T val) {
  string->resize(_CEL_MAX_DOUBLE_CHARS);
  string->resize(_cel_ToChars(string->data(), val));
  return *string;
}

TEST(FromChars, ShortBase10) {
  short to;
  std::string_view from = "42";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, ShortBase16) {
  short to;
  std::string_view from = "2a";
  ASSERT_EQ(FromChars(&from, &to, 16), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, UnsignedShortBase10) {
  unsigned short to;
  std::string_view from = "42";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, UnsignedShortBase16) {
  unsigned short to;
  std::string_view from = "2a";
  ASSERT_EQ(FromChars(&from, &to, 16), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, IntBase10) {
  int to;
  std::string_view from = "42";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, IntBase16) {
  int to;
  std::string_view from = "2a";
  ASSERT_EQ(FromChars(&from, &to, 16), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, UnsignedIntBase10) {
  unsigned int to;
  std::string_view from = "42";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, UnsignedIntBase16) {
  unsigned int to;
  std::string_view from = "2a";
  ASSERT_EQ(FromChars(&from, &to, 16), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, LongBase10) {
  long to;
  std::string_view from = "42";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, LongBase16) {
  long to;
  std::string_view from = "2a";
  ASSERT_EQ(FromChars(&from, &to, 16), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, UnsignedLongBase10) {
  unsigned long to;
  std::string_view from = "42";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, UnsignedLongBase16) {
  unsigned long to;
  std::string_view from = "2a";
  ASSERT_EQ(FromChars(&from, &to, 16), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, LongLongBase10) {
  long long to;
  std::string_view from = "42";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, LongLongBase16) {
  long long to;
  std::string_view from = "2a";
  ASSERT_EQ(FromChars(&from, &to, 16), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, UnsignedLongLongBase10) {
  unsigned long long to;
  std::string_view from = "42";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, UnsignedLongLongBase16) {
  unsigned long long to;
  std::string_view from = "2a";
  ASSERT_EQ(FromChars(&from, &to, 16), 0);
  EXPECT_EQ(to, 42);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, FloatDot) {
  ScopedLocale scoped_locale("C");
  ASSERT_TRUE(scoped_locale);
  float to;
  std::string_view from;

  from = "1.5";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, 1.5f);
  EXPECT_TRUE(from.empty());

  from = "-1.5";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, -1.5f);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, DoubleDot) {
  ScopedLocale scoped_locale("C");
  ASSERT_TRUE(scoped_locale);
  double to;
  std::string_view from;

  from = "1.5";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, 1.5);
  EXPECT_TRUE(from.empty());

  from = "-1.5";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, -1.5);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, FloatComma) {
  ScopedLocale scoped_locale("de_DE.UTF-8");
  if (!scoped_locale) {
    GTEST_SKIP() << "locale de_DE.UTF-8 unavailable";
  }
  float to;
  std::string_view from;

  from = "1.5";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, 1.5);
  EXPECT_TRUE(from.empty());

  from = "-1.5";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, -1.5);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, DoubleComma) {
  ScopedLocale scoped_locale("de_DE.UTF-8");
  if (!scoped_locale) {
    GTEST_SKIP() << "locale de_DE.UTF-8 unavailable";
  }
  double to;
  std::string_view from;

  from = "1.5";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, 1.5);
  EXPECT_TRUE(from.empty());

  from = "-1.5";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, -1.5);
  EXPECT_TRUE(from.empty());
}

TEST(FromChars, FloatArabic) {
  ScopedLocale scoped_locale("ar_SA.UTF-8");
  if (!scoped_locale) {
    GTEST_SKIP() << "locale ar_SA.UTF-8 unavailable";
  }
  float to;
  std::string_view from;

  from = "1.5";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, 1.5) << from;
  EXPECT_TRUE(from.empty()) << from;

  from = "-1.5";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, -1.5) << from;
  EXPECT_TRUE(from.empty()) << from;
}

TEST(FromChars, DoubleArabic) {
  ScopedLocale scoped_locale("ar_SA.UTF-8");
  if (!scoped_locale) {
    GTEST_SKIP() << "locale ar_SA.UTF-8 unavailable";
  }
  double to;
  std::string_view from;

  from = "1.5";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, 1.5) << from;
  EXPECT_TRUE(from.empty()) << from;

  from = "-1.5";
  ASSERT_EQ(FromChars(&from, &to), 0);
  EXPECT_EQ(to, -1.5) << from;
  EXPECT_TRUE(from.empty()) << from;
}

TEST(ToChars, ShortBase10) {
  std::string to;
  ASSERT_EQ(ToChars(&to, short{42}, 10), "42");
  ASSERT_EQ(ToChars(&to, short{-42}, 10), "-42");
}

TEST(ToChars, ShortBase16) {
  std::string to;
  ASSERT_EQ(ToChars(&to, short{42}, 16), "2a");
  ASSERT_EQ(ToChars(&to, short{-42}, 16), "-2a");
}

TEST(ToChars, UnsignedShortBase10) {
  std::string to;
  ASSERT_EQ(ToChars(&to, static_cast<unsigned short>(42), 10), "42");
}

TEST(ToChars, UnsignedShortBase16) {
  std::string to;
  ASSERT_EQ(ToChars(&to, static_cast<unsigned short>(42), 16), "2a");
}

TEST(ToChars, IntBase10) {
  std::string to;
  ASSERT_EQ(ToChars(&to, 42, 10), "42");
  ASSERT_EQ(ToChars(&to, -42, 10), "-42");
}

TEST(ToChars, IntBase16) {
  std::string to;
  ASSERT_EQ(ToChars(&to, 42, 16), "2a");
  ASSERT_EQ(ToChars(&to, -42, 16), "-2a");
}

TEST(ToChars, UnsignedIntBase10) {
  std::string to;
  ASSERT_EQ(ToChars(&to, 42u, 10), "42");
}

TEST(ToChars, UnsignedIntBase16) {
  std::string to;
  ASSERT_EQ(ToChars(&to, 42u, 16), "2a");
}

TEST(ToChars, LongBase10) {
  std::string to;
  ASSERT_EQ(ToChars(&to, 42l, 10), "42");
  ASSERT_EQ(ToChars(&to, -42l, 10), "-42");
}

TEST(ToChars, LongBase16) {
  std::string to;
  ASSERT_EQ(ToChars(&to, 42l, 16), "2a");
  ASSERT_EQ(ToChars(&to, -42l, 16), "-2a");
}

TEST(ToChars, UnsignedLongBase10) {
  std::string to;
  ASSERT_EQ(ToChars(&to, 42ul, 10), "42");
}

TEST(ToChars, UnsignedLongBase16) {
  std::string to;
  ASSERT_EQ(ToChars(&to, 42ul, 16), "2a");
}

TEST(ToChars, LongLongBase10) {
  std::string to;
  ASSERT_EQ(ToChars(&to, 42ll, 10), "42");
  ASSERT_EQ(ToChars(&to, -42ll, 10), "-42");
}

TEST(ToChars, LongLongBase16) {
  std::string to;
  ASSERT_EQ(ToChars(&to, 42ll, 16), "2a");
  ASSERT_EQ(ToChars(&to, -42ll, 16), "-2a");
}

TEST(ToChars, UnsignedLongLongBase10) {
  std::string to;
  ASSERT_EQ(ToChars(&to, 42ull, 10), "42");
}

TEST(ToChars, UnsignedLongLongBase16) {
  std::string to;
  ASSERT_EQ(ToChars(&to, 42ull, 16), "2a");
}

TEST(ToChars, FloatDot) {
  ScopedLocale scoped_locale("C");
  ASSERT_TRUE(scoped_locale);
  std::string to;
  ASSERT_EQ(ToChars(&to, 1.5f), "1.5");
}

TEST(ToChars, DoubleDot) {
  ScopedLocale scoped_locale("C");
  ASSERT_TRUE(scoped_locale);
  std::string to;
  ASSERT_EQ(ToChars(&to, 1.5), "1.5");
}

TEST(ToChars, FloatComma) {
  ScopedLocale scoped_locale("de_DE.UTF-8");
  if (!scoped_locale) {
    GTEST_SKIP() << "locale de_DE.UTF-8 unavailable";
  }
  std::string to;
  ASSERT_EQ(ToChars(&to, 1.5f), "1.5");
}

TEST(ToChars, DoubleComma) {
  ScopedLocale scoped_locale("de_DE.UTF-8");
  if (!scoped_locale) {
    GTEST_SKIP() << "locale de_DE.UTF-8 unavailable";
  }
  std::string to;
  ASSERT_EQ(ToChars(&to, 1.5), "1.5");
}

TEST(ToChars, FloatArabic) {
  ScopedLocale scoped_locale("ar_SA.UTF-8");
  if (!scoped_locale) {
    GTEST_SKIP() << "locale ar_SA.UTF-8 unavailable";
  }
  std::string to;
  ASSERT_EQ(ToChars(&to, 1.5f), "1.5");
}

TEST(ToChars, DoubleArabic) {
  ScopedLocale scoped_locale("ar_SA.UTF-8");
  if (!scoped_locale) {
    GTEST_SKIP() << "locale ar_SA.UTF-8 unavailable";
  }
  std::string to;
  ASSERT_EQ(ToChars(&to, 1.5), "1.5");
}

}  // namespace

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)
