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

#include "cel-c/internal/arena_string.h"

#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/config.h"
#include "cel-c/cstring_view.h"
#include "cel-c/internal/generic_string.h"
#include "cel-c/string_view.h"

namespace {

using ::testing::NotNull;
using ::testing::Test;

class ArenaStringTest : public Test {
 public:
  void SetUp() override {
    arena_ = ABSL_DIE_IF_NULL(cel_Arena_New(cel_DefaultAllocator));
  }

  void TearDown() override {
    cel_Arena_Delete(arena_);
    arena_ = nullptr;
  }

 protected:
  CEL_NONNULL(cel_Arena*) arena() { return ABSL_DIE_IF_NULL(arena_); }

 private:
  CEL_NULLABLE(cel_Arena*) arena_ = nullptr;
};

TEST_F(ArenaStringTest, Empty) {
  _cel_ArenaString str;
  _cel_ArenaString_Construct(&str);

  EXPECT_TRUE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), 0);
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), 0);
  EXPECT_EQ(_cel_ArenaString_Capacity(&str), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_Empty(_cel_ArenaString_ToStringView(&str)));
  EXPECT_TRUE(cel_CStringView_Empty(_cel_ArenaString_ToCStringView(&str)));

  _cel_ArenaString_Clear(&str);

  EXPECT_TRUE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), 0);
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), 0);
  EXPECT_EQ(_cel_ArenaString_Capacity(&str), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_Empty(_cel_ArenaString_ToStringView(&str)));
  EXPECT_TRUE(cel_CStringView_Empty(_cel_ArenaString_ToCStringView(&str)));
}

TEST_F(ArenaStringTest, Assign) {
  cel_CStringView small = cel_CStringView_FromString("Hello World!");
  cel_CStringView large = cel_CStringView_FromString(
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
  cel_CStringView huge = cel_CStringView_FromString(
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
      "fermentum ac massa nec sollicitudin. Fusce sed ultrices tellus."
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
      "fermentum ac massa nec sollicitudin. Fusce sed ultrices tellus."
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

  _cel_ArenaString str;
  _cel_ArenaString_Construct(&str);

  ASSERT_TRUE(
      _cel_ArenaString_Assign(&str, arena(), cel_StringView_From(small)));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), cel_CStringView_Size(small));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), cel_CStringView_SizeInt(small));
  EXPECT_EQ(_cel_ArenaString_Capacity(&str), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_Equals(_cel_ArenaString_ToStringView(&str),
                                    cel_StringView_From(small)));
  EXPECT_TRUE(
      cel_CStringView_Equals(_cel_ArenaString_ToCStringView(&str), small));

  ASSERT_TRUE(
      _cel_ArenaString_Assign(&str, arena(), cel_StringView_From(large)));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), cel_CStringView_Size(large));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), cel_CStringView_SizeInt(large));
  EXPECT_GE(_cel_ArenaString_Capacity(&str), cel_CStringView_Size(large));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_Equals(_cel_ArenaString_ToStringView(&str),
                                    cel_StringView_From(large)));
  EXPECT_TRUE(
      cel_CStringView_Equals(_cel_ArenaString_ToCStringView(&str), large));

  ASSERT_TRUE(
      _cel_ArenaString_Assign(&str, arena(), cel_StringView_From(small)));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), cel_CStringView_Size(small));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), cel_CStringView_SizeInt(small));
  EXPECT_GE(_cel_ArenaString_Capacity(&str), cel_CStringView_Size(small));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_Equals(_cel_ArenaString_ToStringView(&str),
                                    cel_StringView_From(small)));
  EXPECT_TRUE(
      cel_CStringView_Equals(_cel_ArenaString_ToCStringView(&str), small));

  ASSERT_TRUE(
      _cel_ArenaString_Assign(&str, arena(), cel_StringView_From(huge)));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), cel_CStringView_Size(huge));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), cel_CStringView_SizeInt(huge));
  EXPECT_GE(_cel_ArenaString_Capacity(&str), cel_CStringView_Size(huge));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_Equals(_cel_ArenaString_ToStringView(&str),
                                    cel_StringView_From(huge)));
  EXPECT_TRUE(
      cel_CStringView_Equals(_cel_ArenaString_ToCStringView(&str), huge));
}

TEST_F(ArenaStringTest, AppendF) {
  cel_CStringView small = cel_CStringView_FromString("Hello World!");
  cel_CStringView large = cel_CStringView_FromString(
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
  cel_CStringView huge = cel_CStringView_FromString(
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
      "fermentum ac massa nec sollicitudin. Fusce sed ultrices tellus."
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
      "fermentum ac massa nec sollicitudin. Fusce sed ultrices tellus."
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

  _cel_ArenaString str;
  _cel_ArenaString_Construct(&str);

  ASSERT_EQ(_cel_ArenaString_AppendF(&str, arena(), CEL_CSTRINGVIEW_FMT,
                                     CEL_CSTRINGVIEW_ARGS(small)),
            cel_CStringView_Size(small));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), cel_CStringView_Size(small));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), cel_CStringView_SizeInt(small));
  EXPECT_EQ(_cel_ArenaString_Capacity(&str), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_Equals(_cel_ArenaString_ToStringView(&str),
                                    cel_StringView_From(small)));
  EXPECT_TRUE(
      cel_CStringView_Equals(_cel_ArenaString_ToCStringView(&str), small));

  ASSERT_EQ(_cel_ArenaString_AppendF(&str, arena(), "%s", ""), 0);

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), cel_CStringView_Size(small));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), cel_CStringView_SizeInt(small));
  EXPECT_EQ(_cel_ArenaString_Capacity(&str), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_Equals(_cel_ArenaString_ToStringView(&str),
                                    cel_StringView_From(small)));
  EXPECT_TRUE(
      cel_CStringView_Equals(_cel_ArenaString_ToCStringView(&str), small));

  ASSERT_EQ(_cel_ArenaString_AppendF(&str, arena(), CEL_CSTRINGVIEW_FMT,
                                     CEL_CSTRINGVIEW_ARGS(large)),
            cel_CStringView_Size(large));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str),
            cel_CStringView_Size(small) + cel_CStringView_Size(large));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str),
            cel_CStringView_SizeInt(small) + cel_CStringView_SizeInt(large));
  EXPECT_GE(_cel_ArenaString_Capacity(&str),
            cel_CStringView_Size(small) + cel_CStringView_Size(large));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str),
                                        cel_StringView_From(small)));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_ArenaString_ToStringView(&str),
                                      cel_StringView_From(large)));
  EXPECT_TRUE(
      cel_CStringView_StartsWith(_cel_ArenaString_ToCStringView(&str), small));

  ASSERT_EQ(_cel_ArenaString_AppendF(&str, arena(), "%s", ""), 0);

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str),
            cel_CStringView_Size(small) + cel_CStringView_Size(large));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str),
            cel_CStringView_SizeInt(small) + cel_CStringView_SizeInt(large));
  EXPECT_GE(_cel_ArenaString_Capacity(&str),
            cel_CStringView_Size(small) + cel_CStringView_Size(large));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str),
                                        cel_StringView_From(small)));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_ArenaString_ToStringView(&str),
                                      cel_StringView_From(large)));
  EXPECT_TRUE(
      cel_CStringView_StartsWith(_cel_ArenaString_ToCStringView(&str), small));

  ASSERT_EQ(_cel_ArenaString_AppendF(&str, arena(), CEL_CSTRINGVIEW_FMT,
                                     CEL_CSTRINGVIEW_ARGS(small)),
            cel_CStringView_Size(small));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str),
            cel_CStringView_Size(small) * 2 + cel_CStringView_Size(large));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), cel_CStringView_SizeInt(small) * 2 +
                                                cel_CStringView_SizeInt(large));
  EXPECT_GE(_cel_ArenaString_Capacity(&str),
            cel_CStringView_Size(small) * 2 + cel_CStringView_Size(large));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str),
                                        cel_StringView_From(small)));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_ArenaString_ToStringView(&str),
                                      cel_StringView_From(small)));
  EXPECT_TRUE(
      cel_CStringView_StartsWith(_cel_ArenaString_ToCStringView(&str), small));

  _cel_ArenaString_Clear(&str);

  EXPECT_TRUE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), 0);
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), 0);
  EXPECT_GE(_cel_ArenaString_Capacity(&str), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_Empty(_cel_ArenaString_ToStringView(&str)));
  EXPECT_TRUE(cel_CStringView_Empty(_cel_ArenaString_ToCStringView(&str)));

  ASSERT_EQ(_cel_ArenaString_AppendF(&str, arena(), CEL_CSTRINGVIEW_FMT,
                                     CEL_CSTRINGVIEW_ARGS(huge)),
            cel_CStringView_Size(huge));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), cel_CStringView_Size(huge));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), cel_CStringView_SizeInt(huge));
  EXPECT_GE(_cel_ArenaString_Capacity(&str), _cel_ArenaString_Size(&str));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_Equals(_cel_ArenaString_ToStringView(&str),
                                    cel_StringView_From(huge)));
  EXPECT_TRUE(
      cel_CStringView_Equals(_cel_ArenaString_ToCStringView(&str), huge));
}

TEST_F(ArenaStringTest, Append) {
  cel_StringView small = cel_StringView_FromString("Hello World!");
  cel_StringView large = cel_StringView_FromString(
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
  cel_StringView huge = cel_StringView_FromString(
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
      "fermentum ac massa nec sollicitudin. Fusce sed ultrices tellus."
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
      "fermentum ac massa nec sollicitudin. Fusce sed ultrices tellus."
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

  _cel_ArenaString str;
  _cel_ArenaString_Construct(&str);

  ASSERT_TRUE(_cel_ArenaString_Append(&str, arena(), small));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), cel_StringView_Size(small));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), cel_StringView_SizeInt(small));
  EXPECT_EQ(_cel_ArenaString_Capacity(&str), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(
      cel_StringView_Equals(_cel_ArenaString_ToStringView(&str), small));
  EXPECT_TRUE(
      cel_StringView_Equals(_cel_ArenaString_ToStringView(&str), small));

  ASSERT_TRUE(_cel_ArenaString_Append(&str, arena(), cel_StringView_From("")));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), cel_StringView_Size(small));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), cel_StringView_SizeInt(small));
  EXPECT_EQ(_cel_ArenaString_Capacity(&str), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(
      cel_StringView_Equals(_cel_ArenaString_ToStringView(&str), small));
  EXPECT_TRUE(
      cel_StringView_Equals(_cel_ArenaString_ToStringView(&str), small));

  ASSERT_TRUE(_cel_ArenaString_Append(&str, arena(), large));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str),
            cel_StringView_SizeInt(small) + cel_StringView_SizeInt(large));
  EXPECT_GE(_cel_ArenaString_Capacity(&str),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str), small));
  EXPECT_TRUE(
      cel_StringView_EndsWith(_cel_ArenaString_ToStringView(&str), large));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str), small));

  ASSERT_TRUE(_cel_ArenaString_Append(&str, arena(), cel_StringView_From("")));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str),
            cel_StringView_SizeInt(small) + cel_StringView_SizeInt(large));
  EXPECT_GE(_cel_ArenaString_Capacity(&str),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str), small));
  EXPECT_TRUE(
      cel_StringView_EndsWith(_cel_ArenaString_ToStringView(&str), large));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str), small));

  ASSERT_TRUE(_cel_ArenaString_Append(&str, arena(), small));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str),
            cel_StringView_Size(small) * 2 + cel_StringView_Size(large));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str),
            cel_StringView_SizeInt(small) * 2 + cel_StringView_SizeInt(large));
  EXPECT_GE(_cel_ArenaString_Capacity(&str),
            cel_StringView_Size(small) * 2 + cel_StringView_Size(large));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str), small));
  EXPECT_TRUE(
      cel_StringView_EndsWith(_cel_ArenaString_ToStringView(&str), small));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str), small));

  _cel_ArenaString_Clear(&str);

  EXPECT_TRUE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), 0);
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), 0);
  EXPECT_GE(_cel_ArenaString_Capacity(&str), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_Empty(_cel_ArenaString_ToStringView(&str)));
  EXPECT_TRUE(cel_StringView_Empty(_cel_ArenaString_ToStringView(&str)));

  ASSERT_TRUE(_cel_ArenaString_Append(&str, arena(), huge));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), cel_StringView_Size(huge));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), cel_StringView_SizeInt(huge));
  EXPECT_GE(_cel_ArenaString_Capacity(&str), _cel_ArenaString_Size(&str));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_Equals(_cel_ArenaString_ToStringView(&str), huge));
  EXPECT_TRUE(cel_StringView_Equals(_cel_ArenaString_ToStringView(&str), huge));

  _cel_ArenaString_Clear(&str);
}

void DoAppendViaPushBack(_cel_ArenaString* str, cel_Arena* arena,
                         cel_StringView to_append) {
  const char* data = cel_StringView_Data(to_append);
  size_t size = cel_StringView_Size(to_append);
  for (size_t i = 0; i < size; ++i) {
    ASSERT_TRUE(_cel_ArenaString_PushBack(str, arena, data[i]));
  }
}

TEST_F(ArenaStringTest, PushBack) {
  cel_StringView small = cel_StringView_FromString("Hello World!");
  cel_StringView large = cel_StringView_FromString(
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
  cel_StringView huge = cel_StringView_FromString(
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
      "fermentum ac massa nec sollicitudin. Fusce sed ultrices tellus."
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
      "fermentum ac massa nec sollicitudin. Fusce sed ultrices tellus."
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

  _cel_ArenaString str;
  _cel_ArenaString_Construct(&str);

  DoAppendViaPushBack(&str, arena(), small);

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), cel_StringView_Size(small));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), cel_StringView_SizeInt(small));
  EXPECT_EQ(_cel_ArenaString_Capacity(&str), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(
      cel_StringView_Equals(_cel_ArenaString_ToStringView(&str), small));
  EXPECT_TRUE(
      cel_StringView_Equals(_cel_ArenaString_ToStringView(&str), small));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), cel_StringView_Size(small));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), cel_StringView_SizeInt(small));
  EXPECT_EQ(_cel_ArenaString_Capacity(&str), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(
      cel_StringView_Equals(_cel_ArenaString_ToStringView(&str), small));
  EXPECT_TRUE(
      cel_StringView_Equals(_cel_ArenaString_ToStringView(&str), small));

  DoAppendViaPushBack(&str, arena(), large);

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str),
            cel_StringView_SizeInt(small) + cel_StringView_SizeInt(large));
  EXPECT_GE(_cel_ArenaString_Capacity(&str),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str), small));
  EXPECT_TRUE(
      cel_StringView_EndsWith(_cel_ArenaString_ToStringView(&str), large));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str), small));

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str),
            cel_StringView_SizeInt(small) + cel_StringView_SizeInt(large));
  EXPECT_GE(_cel_ArenaString_Capacity(&str),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str), small));
  EXPECT_TRUE(
      cel_StringView_EndsWith(_cel_ArenaString_ToStringView(&str), large));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str), small));

  DoAppendViaPushBack(&str, arena(), small);

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str),
            cel_StringView_Size(small) * 2 + cel_StringView_Size(large));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str),
            cel_StringView_SizeInt(small) * 2 + cel_StringView_SizeInt(large));
  EXPECT_GE(_cel_ArenaString_Capacity(&str),
            cel_StringView_Size(small) * 2 + cel_StringView_Size(large));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str), small));
  EXPECT_TRUE(
      cel_StringView_EndsWith(_cel_ArenaString_ToStringView(&str), small));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_ArenaString_ToStringView(&str), small));

  _cel_ArenaString_Clear(&str);

  EXPECT_TRUE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), 0);
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), 0);
  EXPECT_GE(_cel_ArenaString_Capacity(&str), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_Empty(_cel_ArenaString_ToStringView(&str)));
  EXPECT_TRUE(cel_StringView_Empty(_cel_ArenaString_ToStringView(&str)));

  DoAppendViaPushBack(&str, arena(), huge);

  EXPECT_FALSE(_cel_ArenaString_Empty(&str));
  EXPECT_EQ(_cel_ArenaString_Size(&str), cel_StringView_Size(huge));
  EXPECT_EQ(_cel_ArenaString_SizeInt(&str), cel_StringView_SizeInt(huge));
  EXPECT_GE(_cel_ArenaString_Capacity(&str), _cel_ArenaString_Size(&str));
  EXPECT_THAT(_cel_ArenaString_Data(&str), NotNull());
  EXPECT_EQ(_cel_ArenaString_MutableData(&str), _cel_ArenaString_Data(&str));
  EXPECT_TRUE(cel_StringView_Equals(_cel_ArenaString_ToStringView(&str), huge));
  EXPECT_TRUE(cel_StringView_Equals(_cel_ArenaString_ToStringView(&str), huge));

  _cel_ArenaString_Clear(&str);
}

TEST_F(ArenaStringTest, Reserve) {
  _cel_ArenaString str;
  _cel_ArenaString_Construct(&str);

  EXPECT_EQ(_cel_ArenaString_Capacity(&str), _cel_GenericStringSmall_kCapacity);
  ASSERT_TRUE(_cel_ArenaString_Reserve(&str, arena(),
                                       _cel_GenericStringSmall_kCapacity));
  EXPECT_EQ(_cel_ArenaString_Capacity(&str), _cel_GenericStringSmall_kCapacity);
  ASSERT_TRUE(_cel_ArenaString_Reserve(&str, arena(),
                                       _cel_GenericStringSmall_kCapacity + 1));
  EXPECT_GE(_cel_ArenaString_Capacity(&str),
            _cel_GenericStringSmall_kCapacity + 1);
  ASSERT_TRUE(_cel_ArenaString_Reserve(&str, arena(),
                                       _cel_GenericStringSmall_kCapacity + 2));
  EXPECT_GE(_cel_ArenaString_Capacity(&str),
            _cel_GenericStringSmall_kCapacity + 2);
}

TEST_F(ArenaStringTest, Stability) {
  cel_StringView small = cel_StringView_FromString("Hello World!");

  _cel_ArenaString str;
  _cel_ArenaString_Construct(&str);

  ASSERT_TRUE(_cel_ArenaString_Append(&str, arena(), small));
  _cel_ArenaString other_str = *&str;
  EXPECT_NE(_cel_ArenaString_Data(&other_str), _cel_ArenaString_Data(&str));
  ASSERT_TRUE(_cel_ArenaString_Stabilize(&str, arena()));
  other_str = *&str;
  EXPECT_EQ(_cel_ArenaString_Data(&other_str), _cel_ArenaString_Data(&str));
  _cel_ArenaString_Destabilize(&str, arena());
  other_str = *&str;
  EXPECT_NE(_cel_ArenaString_Data(&other_str), _cel_ArenaString_Data(&str));
}

}  // namespace
