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

#include "cel-c/hash.h"

#include <limits>

#include "gtest/gtest.h"
#include "upb/base/string_view.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

inline bool operator==(cel_HashState lhs, cel_HashState rhs) {
  return lhs.val == rhs.val;
}

inline bool operator!=(cel_HashState lhs, cel_HashState rhs) {
  return !operator==(lhs, rhs);
}

namespace {

TEST(Hash, Empty) {
  EXPECT_EQ(cel_HashState_Initialize(), cel_HashState_Initialize());
  EXPECT_EQ(cel_HashState_Finalize(cel_HashState_Initialize()),
            cel_HashState_Finalize(cel_HashState_Initialize()));
}

TEST(Hash, Bool) {
  EXPECT_EQ(cel_HashState_Combine(cel_HashState_Initialize(), false),
            cel_HashState_Combine(cel_HashState_Initialize(), false));
  EXPECT_EQ(cel_HashState_Combine(cel_HashState_Initialize(), true),
            cel_HashState_Combine(cel_HashState_Initialize(), true));
}

template <typename T>
class HashIntTest : public ::testing::Test {};

using HashIntTestTypes =
    ::testing::Types<char, char16_t, char32_t, signed char, unsigned char,
                     short, unsigned short, int, unsigned int, long,
                     unsigned long, long long, unsigned long long, wchar_t>;

TYPED_TEST_SUITE(HashIntTest, HashIntTestTypes);

TYPED_TEST(HashIntTest, Hash) {
  using TypeParamLimits = std::numeric_limits<TypeParam>;
  EXPECT_EQ(cel_HashState_Combine(cel_HashState_Initialize(), TypeParam{0}),
            cel_HashState_Combine(cel_HashState_Initialize(), TypeParam{0}));
  EXPECT_EQ(cel_HashState_Combine(cel_HashState_Initialize(), TypeParam{1}),
            cel_HashState_Combine(cel_HashState_Initialize(), TypeParam{1}));
  EXPECT_EQ(
      cel_HashState_Combine(cel_HashState_Initialize(), TypeParamLimits::min()),
      cel_HashState_Combine(cel_HashState_Initialize(),
                            TypeParamLimits::min()));
  EXPECT_EQ(
      cel_HashState_Combine(cel_HashState_Initialize(), TypeParamLimits::max()),
      cel_HashState_Combine(cel_HashState_Initialize(),
                            TypeParamLimits::max()));
}

template <typename T>
class HashFloatTest : public ::testing::Test {};

using HashFloatTestTypes = ::testing::Types<float, double, long double>;

TYPED_TEST_SUITE(HashFloatTest, HashFloatTestTypes);

TYPED_TEST(HashFloatTest, Hash) {
  using Limits = std::numeric_limits<TypeParam>;
  EXPECT_EQ(cel_HashState_Combine(cel_HashState_Initialize(), TypeParam{0.0}),
            cel_HashState_Combine(cel_HashState_Initialize(), TypeParam{0.0}));
  EXPECT_EQ(
      cel_HashState_Combine(cel_HashState_Initialize(), Limits::infinity()),
      cel_HashState_Combine(cel_HashState_Initialize(), Limits::infinity()));
  EXPECT_EQ(
      cel_HashState_Combine(cel_HashState_Initialize(), -Limits::infinity()),
      cel_HashState_Combine(cel_HashState_Initialize(), -Limits::infinity()));
  EXPECT_EQ(
      cel_HashState_Combine(cel_HashState_Initialize(), Limits::quiet_NaN()),
      cel_HashState_Combine(cel_HashState_Initialize(), Limits::quiet_NaN()));
  EXPECT_EQ(cel_HashState_Combine(cel_HashState_Initialize(), TypeParam{-0.0}),
            cel_HashState_Combine(cel_HashState_Initialize(), TypeParam{0.0}));
  EXPECT_EQ(cel_HashState_Combine(cel_HashState_Initialize(), TypeParam{1.0}),
            cel_HashState_Combine(cel_HashState_Initialize(), TypeParam{1.0}));
}

TEST(Hash, Str) {
  EXPECT_EQ(cel_HashState_Combine(cel_HashState_Initialize(), "Hello World!"),
            cel_HashState_Combine(cel_HashState_Initialize(), "Hello World!"));
  EXPECT_EQ(cel_HashState_Combine(cel_HashState_Initialize(), "Hello World!"),
            cel_HashState_Combine(cel_HashState_Initialize(),
                                  upb_StringView_FromString("Hello World!")));
  EXPECT_EQ(
      cel_HashState_Combine(cel_HashState_Initialize(), (const char*)nullptr),
      cel_HashState_Combine(cel_HashState_Initialize(), ""));
}

TEST(Hash, Mem) {
  upb_StringView small = upb_StringView_FromString("Hello World!");
  EXPECT_EQ(cel_HashState_Combine(cel_HashState_Initialize(), small),
            cel_HashState_Combine(cel_HashState_Initialize(), small));
  EXPECT_EQ(cel_HashState_Combine(cel_HashState_Initialize(), small),
            cel_HashState_Combine(cel_HashState_Initialize(), small));
  upb_StringView large = upb_StringView_FromString(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus "
      "condimentum rhoncus est volutpat venenatis. Fusce semper, sapien ut "
      "venenatis pellentesque, lorem dui aliquam sapien, non pharetra diam "
      "neque id mi. Suspendisse sollicitudin, metus ut gravida semper, nunc "
      "ipsum ullamcorper nunc, ut maximus nulla nunc dignissim justo. Duis nec "
      "nisi leo. Proin tristique massa mi, imperdiet tempus nibh vulputate "
      "quis. Morbi sagittis eget neque sagittis egestas. Quisque viverra arcu "
      "a cursus dignissim. In aliquam, mi ut laoreet varius, ex ante posuere "
      "justo, eget aliquam magna metus id purus. Aenean convallis sem ac purus "
      "bibendum, sit amet mattis augue fermentum. Quisque cursus posuere mi, "
      "vitae vestibulum purus egestas eget. Nunc eu sagittis est, at elementum "
      "leo. Pellentesque habitant morbi tristique senectus et netus et "
      "malesuada fames ac turpis egestas. Nulla interdum lacus a turpis "
      "maximus, scelerisque aliquam magna ultricies. Aliquam erat volutpat. "
      "Mauris eget tellus sed velit aliquet venenatis ut nec eros. Nulla "
      "facilisi. Sed posuere nisi quis felis varius, ut maximus elit placerat. "
      "Sed vulputate quam augue, ac lacinia diam ullamcorper at. Fusce "
      "efficitur fermentum mi, sed imperdiet ipsum dignissim eu. Nam semper "
      "quis ex eget iaculis. In a diam dolor. Integer fermentum in arcu "
      "commodo tempus. Phasellus ultricies gravida ante ac tristique. "
      "Vestibulum volutpat id neque a porttitor. Donec ullamcorper sed augue "
      "eget efficitur. Ut vitae aliquet lorem, quis ultrices neque. Fusce "
      "blandit ac mauris vel porttitor. Pellentesque eget ultrices augue. "
      "Pellentesque tempor maximus nunc sit amet ullamcorper. Nulla non "
      "vehicula magna. In eget condimentum nisi, sit amet auctor purus. Proin "
      "venenatis lorem non mattis interdum. Aenean metus leo, lobortis non "
      "auctor eget, bibendum at mi. Aenean mauris est, convallis efficitur "
      "molestie et, suscipit in dolor. Duis eget volutpat est. Fusce id "
      "maximus arcu, et tincidunt purus. In efficitur volutpat est nec "
      "eleifend. Proin malesuada pharetra mattis. Donec sed arcu sit amet nisi "
      "vulputate ornare. Integer accumsan ullamcorper mauris, eget semper "
      "ipsum. Suspendisse sagittis pellentesque molestie. Duis at venenatis "
      "erat. Quisque congue turpis id erat egestas pellentesque. Nulla "
      "fermentum ac massa nec sollicitudin. Fusce sed ultrices tellus.");
  EXPECT_EQ(cel_HashState_Combine(cel_HashState_Initialize(), large),
            cel_HashState_Combine(cel_HashState_Initialize(), large));
  EXPECT_EQ(cel_HashState_Combine(cel_HashState_Initialize(), large),
            cel_HashState_Combine(cel_HashState_Initialize(), large));
}

}  // namespace

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)
