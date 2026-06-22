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

#include "cel-c/config.h"

#include <cstdint>
#include <type_traits>

#include "gtest/gtest.h"

namespace {

#define COUNT_TRAILING_ARGS(ignored, ...) _CEL_NARGS(__VA_ARGS__)

TEST(NArgs, Correct) {
  EXPECT_EQ(_CEL_NARGS(), 0);
  EXPECT_EQ(COUNT_TRAILING_ARGS(ignored), 0);
  EXPECT_EQ(COUNT_TRAILING_ARGS(ignored, foo), 1);
  EXPECT_EQ(COUNT_TRAILING_ARGS(ignored, foo, bar), 2);
  EXPECT_EQ(COUNT_TRAILING_ARGS(ignored, foo, bar, baz), 3);
}

#undef COUNT_TRAILING_ARGS

template <typename Got, typename Want>
constexpr bool DoTypeOfIsSame() {
  return std::is_same<cel_typeof(std::declval<Got>()), Want>::value;
}

template <typename Got, typename Want>
constexpr bool DoTypeOfUnqualIsSame() {
  return std::is_same<cel_typeof_unqual(std::declval<Got>()), Want>::value;
}

#if defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
using cel_char8_t = unsigned char;
#endif

#if defined(__GNUC__) || defined(__clang__)
using cel_wchar_t = __WCHAR_TYPE__;
#elif defined(_MSC_VER)
using cel_wchar_t = unsigned short;  // NOLINT(runtime/int)
#else
#error Unexpected compiler.
#endif

using cel_char16_t = uint_least16_t;

using cel_char32_t = uint_least32_t;

enum class IntEnum : int {};

enum class WcharEnum : wchar_t {};

#if defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
enum class Char8Enum : char8_t {};
#endif

enum class Char16Enum : char16_t {};

enum class Char32Enum : char32_t {};

TEST(TypeOf, Correct) {
  EXPECT_TRUE((DoTypeOfIsSame<wchar_t, cel_wchar_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<const wchar_t, const cel_wchar_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<volatile wchar_t, volatile cel_wchar_t>()));
  EXPECT_TRUE(
      (DoTypeOfIsSame<const volatile wchar_t, const volatile cel_wchar_t>()));

  EXPECT_TRUE((DoTypeOfIsSame<WcharEnum, cel_wchar_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<const WcharEnum, const cel_wchar_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<volatile WcharEnum, volatile cel_wchar_t>()));
  EXPECT_TRUE(
      (DoTypeOfIsSame<const volatile WcharEnum, const volatile cel_wchar_t>()));

  EXPECT_TRUE((DoTypeOfIsSame<char16_t, cel_char16_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<const char16_t, const cel_char16_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<volatile char16_t, volatile cel_char16_t>()));
  EXPECT_TRUE(
      (DoTypeOfIsSame<const volatile char16_t, const volatile cel_char16_t>()));

  EXPECT_TRUE((DoTypeOfIsSame<Char16Enum, cel_char16_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<const Char16Enum, const cel_char16_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<volatile Char16Enum, volatile cel_char16_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<const volatile Char16Enum,
                              const volatile cel_char16_t>()));

  EXPECT_TRUE((DoTypeOfIsSame<char32_t, cel_char32_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<const char32_t, const cel_char32_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<volatile char32_t, volatile cel_char32_t>()));
  EXPECT_TRUE(
      (DoTypeOfIsSame<const volatile char32_t, const volatile cel_char32_t>()));

  EXPECT_TRUE((DoTypeOfIsSame<Char32Enum, cel_char32_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<const Char32Enum, const cel_char32_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<volatile Char32Enum, volatile cel_char32_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<const volatile Char32Enum,
                              const volatile cel_char32_t>()));

#if defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
  EXPECT_TRUE((DoTypeOfIsSame<char8_t, cel_char8_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<const char8_t, const cel_char8_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<volatile char8_t, volatile cel_char8_t>()));
  EXPECT_TRUE(
      (DoTypeOfIsSame<const volatile char8_t, const volatile cel_char8_t>()));

  EXPECT_TRUE((DoTypeOfIsSame<Char8Enum, cel_char8_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<const Char8Enum, const cel_char8_t>()));
  EXPECT_TRUE((DoTypeOfIsSame<volatile Char8Enum, volatile cel_char8_t>()));
  EXPECT_TRUE(
      (DoTypeOfIsSame<const volatile Char8Enum, const volatile cel_char8_t>()));
#endif

  EXPECT_TRUE((DoTypeOfIsSame<int, int>()));
  EXPECT_TRUE((DoTypeOfIsSame<const int, const int>()));
  EXPECT_TRUE((DoTypeOfIsSame<volatile int, volatile int>()));
  EXPECT_TRUE((DoTypeOfIsSame<const volatile int, const volatile int>()));

  EXPECT_TRUE((DoTypeOfIsSame<IntEnum, int>()));
  EXPECT_TRUE((DoTypeOfIsSame<const IntEnum, const int>()));
  EXPECT_TRUE((DoTypeOfIsSame<volatile IntEnum, volatile int>()));
  EXPECT_TRUE((DoTypeOfIsSame<const volatile IntEnum, const volatile int>()));

  EXPECT_TRUE((DoTypeOfIsSame<int&, int>()));
  EXPECT_TRUE((DoTypeOfIsSame<const int&, const int>()));
  EXPECT_TRUE((DoTypeOfIsSame<volatile int&, volatile int>()));
  EXPECT_TRUE((DoTypeOfIsSame<const volatile int&, const volatile int>()));

  EXPECT_TRUE((DoTypeOfIsSame<int&&, int>()));
  EXPECT_TRUE((DoTypeOfIsSame<const int&&, const int>()));
  EXPECT_TRUE((DoTypeOfIsSame<volatile int&&, volatile int>()));
  EXPECT_TRUE((DoTypeOfIsSame<const volatile int&&, const volatile int>()));

  EXPECT_TRUE((DoTypeOfIsSame<int*, int*>()));
  EXPECT_TRUE((DoTypeOfIsSame<int* const, int* const>()));
  EXPECT_TRUE((DoTypeOfIsSame<int* volatile, int* volatile>()));
  EXPECT_TRUE((DoTypeOfIsSame<int* const volatile, int* const volatile>()));

  EXPECT_TRUE((DoTypeOfIsSame<const int*, const int*>()));
  EXPECT_TRUE((DoTypeOfIsSame<volatile int*, volatile int*>()));
  EXPECT_TRUE((DoTypeOfIsSame<const volatile int*, const volatile int*>()));

  EXPECT_TRUE((DoTypeOfIsSame<int*&, int*>()));
  EXPECT_TRUE((DoTypeOfIsSame<int* const&, int* const>()));
  EXPECT_TRUE((DoTypeOfIsSame<int* volatile&, int* volatile>()));
  EXPECT_TRUE((DoTypeOfIsSame<int* const volatile&, int* const volatile>()));
}

TEST(TypeOfUnqual, Correct) {
  EXPECT_TRUE((DoTypeOfUnqualIsSame<wchar_t, cel_wchar_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const wchar_t, cel_wchar_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<volatile wchar_t, cel_wchar_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const volatile wchar_t, cel_wchar_t>()));

  EXPECT_TRUE((DoTypeOfUnqualIsSame<WcharEnum, cel_wchar_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const WcharEnum, cel_wchar_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<volatile WcharEnum, cel_wchar_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const volatile WcharEnum, cel_wchar_t>()));

  EXPECT_TRUE((DoTypeOfUnqualIsSame<char16_t, cel_char16_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const char16_t, cel_char16_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<volatile char16_t, cel_char16_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const volatile char16_t, cel_char16_t>()));

  EXPECT_TRUE((DoTypeOfUnqualIsSame<Char16Enum, cel_char16_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const Char16Enum, cel_char16_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<volatile Char16Enum, cel_char16_t>()));
  EXPECT_TRUE(
      (DoTypeOfUnqualIsSame<const volatile Char16Enum, cel_char16_t>()));

  EXPECT_TRUE((DoTypeOfUnqualIsSame<char32_t, cel_char32_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const char32_t, cel_char32_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<volatile char32_t, cel_char32_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const volatile char32_t, cel_char32_t>()));

  EXPECT_TRUE((DoTypeOfUnqualIsSame<Char32Enum, cel_char32_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const Char32Enum, cel_char32_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<volatile Char32Enum, cel_char32_t>()));
  EXPECT_TRUE(
      (DoTypeOfUnqualIsSame<const volatile Char32Enum, cel_char32_t>()));

#if defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
  EXPECT_TRUE((DoTypeOfUnqualIsSame<char8_t, cel_char8_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const char8_t, cel_char8_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<volatile char8_t, cel_char8_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const volatile char8_t, cel_char8_t>()));

  EXPECT_TRUE((DoTypeOfUnqualIsSame<Char8Enum, cel_char8_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const Char8Enum, cel_char8_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<volatile Char8Enum, cel_char8_t>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const volatile Char8Enum, cel_char8_t>()));
#endif

  EXPECT_TRUE((DoTypeOfUnqualIsSame<int, int>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const int, int>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<volatile int, int>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const volatile int, int>()));

  EXPECT_TRUE((DoTypeOfUnqualIsSame<IntEnum, int>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const IntEnum, int>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<volatile IntEnum, int>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const volatile IntEnum, int>()));

  EXPECT_TRUE((DoTypeOfUnqualIsSame<int&, int>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const int&, int>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<volatile int&, int>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const volatile int&, int>()));

  EXPECT_TRUE((DoTypeOfUnqualIsSame<int&&, int>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const int&&, int>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<volatile int&&, int>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<const volatile int&&, int>()));

  EXPECT_TRUE((DoTypeOfUnqualIsSame<int*, int*>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<int* const, int*>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<int* volatile, int*>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<int* const volatile, int*>()));

  EXPECT_TRUE((DoTypeOfUnqualIsSame<const int*, const int*>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<volatile int*, volatile int*>()));
  EXPECT_TRUE(
      (DoTypeOfUnqualIsSame<const volatile int*, const volatile int*>()));

  EXPECT_TRUE((DoTypeOfUnqualIsSame<int*&, int*>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<int* const&, int*>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<int* volatile&, int*>()));
  EXPECT_TRUE((DoTypeOfUnqualIsSame<int* const volatile&, int*>()));
}

template <typename Type1, typename Type2>
constexpr bool DoTypesCompatible() {
  return _CEL_TYPES_COMPATIBLE(Type1, Type2);
}

TEST(TypesCompatible, Correct) {
  EXPECT_TRUE((DoTypesCompatible<const int, int>()));
  EXPECT_TRUE((DoTypesCompatible<int, const int>()));
  EXPECT_TRUE((DoTypesCompatible<volatile int, int>()));
  EXPECT_TRUE((DoTypesCompatible<int, volatile int>()));
  EXPECT_TRUE((DoTypesCompatible<const volatile int, int>()));
  EXPECT_TRUE((DoTypesCompatible<int, const volatile int>()));

  EXPECT_TRUE((DoTypesCompatible<const IntEnum, int>()));
  EXPECT_TRUE((DoTypesCompatible<IntEnum, const int>()));
  EXPECT_TRUE((DoTypesCompatible<volatile IntEnum, int>()));
  EXPECT_TRUE((DoTypesCompatible<IntEnum, volatile int>()));
  EXPECT_TRUE((DoTypesCompatible<const volatile IntEnum, int>()));
  EXPECT_TRUE((DoTypesCompatible<IntEnum, const volatile int>()));

  EXPECT_TRUE((DoTypesCompatible<int*, int*>()));
  EXPECT_TRUE((DoTypesCompatible<int* const, int*>()));
  EXPECT_TRUE((DoTypesCompatible<int*, int* const>()));
  EXPECT_TRUE((DoTypesCompatible<int* volatile, int*>()));
  EXPECT_TRUE((DoTypesCompatible<int*, int* volatile>()));
  EXPECT_TRUE((DoTypesCompatible<int* const volatile, int*>()));
  EXPECT_TRUE((DoTypesCompatible<int*, int* const volatile>()));

  EXPECT_FALSE((DoTypesCompatible<const int*, int*>()));
  EXPECT_FALSE((DoTypesCompatible<int*, const int*>()));
  EXPECT_FALSE((DoTypesCompatible<volatile int*, int*>()));
  EXPECT_FALSE((DoTypesCompatible<int*, volatile int*>()));
  EXPECT_FALSE((DoTypesCompatible<const volatile int*, int*>()));
  EXPECT_FALSE((DoTypesCompatible<int*, const volatile int*>()));

  EXPECT_TRUE((DoTypesCompatible<int[], int[]>()));
  EXPECT_TRUE((DoTypesCompatible<int[2], int[]>()));
  EXPECT_TRUE((DoTypesCompatible<int[], int[2]>()));

  // According to GCC 14.2 and Clang 19.1 these are compatible. Which is odd,
  // because if you decay the types they are not. Oh well.
  EXPECT_TRUE((DoTypesCompatible<const int[], int[]>()));
  EXPECT_TRUE((DoTypesCompatible<int[], const int[]>()));
  EXPECT_TRUE((DoTypesCompatible<volatile int[], int[]>()));
  EXPECT_TRUE((DoTypesCompatible<int[], volatile int[]>()));
  EXPECT_TRUE((DoTypesCompatible<const volatile int[], int[]>()));
  EXPECT_TRUE((DoTypesCompatible<int[], const volatile int[]>()));
}

struct Child {
  int field;
};

struct Parent {
  Child child2;
  int field;
  Child child1;
};

TEST(ContainerOf, Correct) {
  Parent parent;
  EXPECT_EQ(cel_containerof(&parent.child1, Parent, child1), &parent);
  EXPECT_EQ(cel_containerof(&parent.child2, Parent, child2), &parent);
}

TEST(ArraySize, Correct) {
  int array[5];
  EXPECT_EQ(cel_arraysize(array), 5);
}

}  // namespace
