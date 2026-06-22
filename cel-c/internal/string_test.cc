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

#include "cel-c/internal/string.h"

#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/alloc.h"
#include "cel-c/cstring_view.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/generic_string.h"
#include "cel-c/string_view.h"

namespace {

using ::testing::NotNull;
using ::testing::Test;

class StringTest : public Test {
 public:
  void SetUp() override { _cel_String_Construct(&str_); }

  void TearDown() override { _cel_String_Destruct(&str_, alloc()); }

 protected:
  CEL_NONNULL(cel_Allocator*) alloc() { return cel_DefaultAllocator; }

  auto* str() { return &str_; }

  _cel_String str_;
};

TEST_F(StringTest, Empty) {
  EXPECT_TRUE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), 0);
  EXPECT_EQ(_cel_String_SizeInt(str()), 0);
  EXPECT_EQ(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Empty(_cel_String_ToStringView(str())));
  EXPECT_TRUE(cel_CStringView_Empty(_cel_String_ToCStringView(str())));

  _cel_String_Clear(str());

  EXPECT_TRUE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), 0);
  EXPECT_EQ(_cel_String_SizeInt(str()), 0);
  EXPECT_EQ(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Empty(_cel_String_ToStringView(str())));
  EXPECT_TRUE(cel_CStringView_Empty(_cel_String_ToCStringView(str())));
}

TEST_F(StringTest, Assign) {
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

  ASSERT_TRUE(_cel_String_Assign(str(), alloc(), cel_StringView_From(small)));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), cel_CStringView_Size(small));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_CStringView_SizeInt(small));
  EXPECT_EQ(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()),
                                    cel_StringView_From(small)));
  EXPECT_TRUE(cel_CStringView_Equals(_cel_String_ToCStringView(str()), small));

  ASSERT_TRUE(_cel_String_Assign(str(), alloc(), cel_StringView_From(large)));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), cel_CStringView_Size(large));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_CStringView_SizeInt(large));
  EXPECT_GE(_cel_String_Capacity(str()), cel_CStringView_Size(large));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()),
                                    cel_StringView_From(large)));
  EXPECT_TRUE(cel_CStringView_Equals(_cel_String_ToCStringView(str()), large));

  ASSERT_TRUE(_cel_String_Assign(str(), alloc(), cel_StringView_From(small)));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), cel_CStringView_Size(small));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_CStringView_SizeInt(small));
  EXPECT_GE(_cel_String_Capacity(str()), cel_CStringView_Size(small));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()),
                                    cel_StringView_From(small)));
  EXPECT_TRUE(cel_CStringView_Equals(_cel_String_ToCStringView(str()), small));

  ASSERT_TRUE(_cel_String_Assign(str(), alloc(), cel_StringView_From(huge)));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), cel_CStringView_Size(huge));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_CStringView_SizeInt(huge));
  EXPECT_GE(_cel_String_Capacity(str()), cel_CStringView_Size(huge));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()),
                                    cel_StringView_From(huge)));
  EXPECT_TRUE(cel_CStringView_Equals(_cel_String_ToCStringView(str()), huge));
}

TEST_F(StringTest, AppendF) {
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

  ASSERT_EQ(_cel_String_AppendF(str(), alloc(), CEL_CSTRINGVIEW_FMT,
                                CEL_CSTRINGVIEW_ARGS(small)),
            cel_CStringView_Size(small));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), cel_CStringView_Size(small));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_CStringView_SizeInt(small));
  EXPECT_EQ(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()),
                                    cel_StringView_From(small)));
  EXPECT_TRUE(cel_CStringView_Equals(_cel_String_ToCStringView(str()), small));

  ASSERT_EQ(_cel_String_AppendF(str(), alloc(), "%s", ""), 0);

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), cel_CStringView_Size(small));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_CStringView_SizeInt(small));
  EXPECT_EQ(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()),
                                    cel_StringView_From(small)));
  EXPECT_TRUE(cel_CStringView_Equals(_cel_String_ToCStringView(str()), small));

  ASSERT_EQ(_cel_String_AppendF(str(), alloc(), CEL_CSTRINGVIEW_FMT,
                                CEL_CSTRINGVIEW_ARGS(large)),
            cel_CStringView_Size(large));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()),
            cel_CStringView_Size(small) + cel_CStringView_Size(large));
  EXPECT_EQ(_cel_String_SizeInt(str()),
            cel_CStringView_SizeInt(small) + cel_CStringView_SizeInt(large));
  EXPECT_GE(_cel_String_Capacity(str()),
            cel_CStringView_Size(small) + cel_CStringView_Size(large));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_StartsWith(_cel_String_ToStringView(str()),
                                        cel_StringView_From(small)));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_String_ToStringView(str()),
                                      cel_StringView_From(large)));
  EXPECT_TRUE(
      cel_CStringView_StartsWith(_cel_String_ToCStringView(str()), small));

  ASSERT_EQ(_cel_String_AppendF(str(), alloc(), "%s", ""), 0);

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()),
            cel_CStringView_Size(small) + cel_CStringView_Size(large));
  EXPECT_EQ(_cel_String_SizeInt(str()),
            cel_CStringView_SizeInt(small) + cel_CStringView_SizeInt(large));
  EXPECT_GE(_cel_String_Capacity(str()),
            cel_CStringView_Size(small) + cel_CStringView_Size(large));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_StartsWith(_cel_String_ToStringView(str()),
                                        cel_StringView_From(small)));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_String_ToStringView(str()),
                                      cel_StringView_From(large)));
  EXPECT_TRUE(
      cel_CStringView_StartsWith(_cel_String_ToCStringView(str()), small));

  ASSERT_EQ(_cel_String_AppendF(str(), alloc(), CEL_CSTRINGVIEW_FMT,
                                CEL_CSTRINGVIEW_ARGS(small)),
            cel_CStringView_Size(small));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()),
            cel_CStringView_Size(small) * 2 + cel_CStringView_Size(large));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_CStringView_SizeInt(small) * 2 +
                                            cel_CStringView_SizeInt(large));
  EXPECT_GE(_cel_String_Capacity(str()),
            cel_CStringView_Size(small) * 2 + cel_CStringView_Size(large));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_StartsWith(_cel_String_ToStringView(str()),
                                        cel_StringView_From(small)));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_String_ToStringView(str()),
                                      cel_StringView_From(small)));
  EXPECT_TRUE(
      cel_CStringView_StartsWith(_cel_String_ToCStringView(str()), small));

  EXPECT_TRUE(_cel_String_ShrinkToFit(str(), alloc()));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()),
            cel_CStringView_Size(small) * 2 + cel_CStringView_Size(large));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_CStringView_SizeInt(small) * 2 +
                                            cel_CStringView_SizeInt(large));
  EXPECT_GE(_cel_String_Capacity(str()),
            cel_CStringView_Size(small) * 2 + cel_CStringView_Size(large));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_StartsWith(_cel_String_ToStringView(str()),
                                        cel_StringView_From(small)));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_String_ToStringView(str()),
                                      cel_StringView_From(small)));
  EXPECT_TRUE(
      cel_CStringView_StartsWith(_cel_String_ToCStringView(str()), small));

  _cel_String_Clear(str());

  EXPECT_TRUE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), 0);
  EXPECT_EQ(_cel_String_SizeInt(str()), 0);
  EXPECT_GE(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Empty(_cel_String_ToStringView(str())));
  EXPECT_TRUE(cel_CStringView_Empty(_cel_String_ToCStringView(str())));

  ASSERT_EQ(_cel_String_AppendF(str(), alloc(), CEL_CSTRINGVIEW_FMT,
                                CEL_CSTRINGVIEW_ARGS(huge)),
            cel_CStringView_Size(huge));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), cel_CStringView_Size(huge));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_CStringView_SizeInt(huge));
  EXPECT_GE(_cel_String_Capacity(str()), _cel_String_Size(str()));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()),
                                    cel_StringView_From(huge)));
  EXPECT_TRUE(cel_CStringView_Equals(_cel_String_ToCStringView(str()), huge));

  _cel_String_Clear(str());

  EXPECT_TRUE(_cel_String_ShrinkToFit(str(), alloc()));

  EXPECT_TRUE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), 0);
  EXPECT_EQ(_cel_String_SizeInt(str()), 0);
  EXPECT_GE(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Empty(_cel_String_ToStringView(str())));
  EXPECT_TRUE(cel_CStringView_Empty(_cel_String_ToCStringView(str())));

  _cel_String_Reset(str(), alloc());
}

TEST_F(StringTest, Append) {
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

  ASSERT_TRUE(_cel_String_Append(str(), alloc(), small));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), cel_StringView_Size(small));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_StringView_SizeInt(small));
  EXPECT_EQ(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()), small));

  ASSERT_TRUE(_cel_String_Append(str(), alloc(), cel_StringView_From("")));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), cel_StringView_Size(small));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_StringView_SizeInt(small));
  EXPECT_EQ(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()), small));

  ASSERT_TRUE(_cel_String_Append(str(), alloc(), large));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_EQ(_cel_String_SizeInt(str()),
            cel_StringView_SizeInt(small) + cel_StringView_SizeInt(large));
  EXPECT_GE(_cel_String_Capacity(str()),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_String_ToStringView(str()), large));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));

  ASSERT_TRUE(_cel_String_Append(str(), alloc(), cel_StringView_From("")));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_EQ(_cel_String_SizeInt(str()),
            cel_StringView_SizeInt(small) + cel_StringView_SizeInt(large));
  EXPECT_GE(_cel_String_Capacity(str()),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_String_ToStringView(str()), large));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));

  ASSERT_TRUE(_cel_String_Append(str(), alloc(), small));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()),
            cel_StringView_Size(small) * 2 + cel_StringView_Size(large));
  EXPECT_EQ(_cel_String_SizeInt(str()),
            cel_StringView_SizeInt(small) * 2 + cel_StringView_SizeInt(large));
  EXPECT_GE(_cel_String_Capacity(str()),
            cel_StringView_Size(small) * 2 + cel_StringView_Size(large));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));

  EXPECT_TRUE(_cel_String_ShrinkToFit(str(), alloc()));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()),
            cel_StringView_Size(small) * 2 + cel_StringView_Size(large));
  EXPECT_EQ(_cel_String_SizeInt(str()),
            cel_StringView_SizeInt(small) * 2 + cel_StringView_SizeInt(large));
  EXPECT_GE(_cel_String_Capacity(str()),
            cel_StringView_Size(small) * 2 + cel_StringView_Size(large));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));

  _cel_String_Clear(str());

  EXPECT_TRUE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), 0);
  EXPECT_EQ(_cel_String_SizeInt(str()), 0);
  EXPECT_GE(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Empty(_cel_String_ToStringView(str())));
  EXPECT_TRUE(cel_StringView_Empty(_cel_String_ToStringView(str())));

  ASSERT_TRUE(_cel_String_Append(str(), alloc(), huge));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), cel_StringView_Size(huge));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_StringView_SizeInt(huge));
  EXPECT_GE(_cel_String_Capacity(str()), _cel_String_Size(str()));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()), huge));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()), huge));

  _cel_String_Clear(str());

  EXPECT_TRUE(_cel_String_ShrinkToFit(str(), alloc()));

  EXPECT_TRUE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), 0);
  EXPECT_EQ(_cel_String_SizeInt(str()), 0);
  EXPECT_GE(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Empty(_cel_String_ToStringView(str())));
  EXPECT_TRUE(cel_StringView_Empty(_cel_String_ToStringView(str())));

  _cel_String_Reset(str(), alloc());
}

void DoAppendViaPushBack(_cel_String* str, cel_Allocator* alloc,
                         cel_StringView to_append) {
  const char* data = cel_StringView_Data(to_append);
  size_t size = cel_StringView_Size(to_append);
  for (size_t i = 0; i < size; ++i) {
    ASSERT_TRUE(_cel_String_PushBack(str, alloc, data[i]));
  }
}

TEST_F(StringTest, PushBack) {
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

  DoAppendViaPushBack(str(), alloc(), small);

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), cel_StringView_Size(small));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_StringView_SizeInt(small));
  EXPECT_EQ(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()), small));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), cel_StringView_Size(small));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_StringView_SizeInt(small));
  EXPECT_EQ(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()), small));

  DoAppendViaPushBack(str(), alloc(), large);

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_EQ(_cel_String_SizeInt(str()),
            cel_StringView_SizeInt(small) + cel_StringView_SizeInt(large));
  EXPECT_GE(_cel_String_Capacity(str()),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_String_ToStringView(str()), large));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_EQ(_cel_String_SizeInt(str()),
            cel_StringView_SizeInt(small) + cel_StringView_SizeInt(large));
  EXPECT_GE(_cel_String_Capacity(str()),
            cel_StringView_Size(small) + cel_StringView_Size(large));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_String_ToStringView(str()), large));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));

  DoAppendViaPushBack(str(), alloc(), small);

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()),
            cel_StringView_Size(small) * 2 + cel_StringView_Size(large));
  EXPECT_EQ(_cel_String_SizeInt(str()),
            cel_StringView_SizeInt(small) * 2 + cel_StringView_SizeInt(large));
  EXPECT_GE(_cel_String_Capacity(str()),
            cel_StringView_Size(small) * 2 + cel_StringView_Size(large));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));

  EXPECT_TRUE(_cel_String_ShrinkToFit(str(), alloc()));

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()),
            cel_StringView_Size(small) * 2 + cel_StringView_Size(large));
  EXPECT_EQ(_cel_String_SizeInt(str()),
            cel_StringView_SizeInt(small) * 2 + cel_StringView_SizeInt(large));
  EXPECT_GE(_cel_String_Capacity(str()),
            cel_StringView_Size(small) * 2 + cel_StringView_Size(large));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(cel_StringView_EndsWith(_cel_String_ToStringView(str()), small));
  EXPECT_TRUE(
      cel_StringView_StartsWith(_cel_String_ToStringView(str()), small));

  _cel_String_Clear(str());

  EXPECT_TRUE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), 0);
  EXPECT_EQ(_cel_String_SizeInt(str()), 0);
  EXPECT_GE(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Empty(_cel_String_ToStringView(str())));
  EXPECT_TRUE(cel_StringView_Empty(_cel_String_ToStringView(str())));

  DoAppendViaPushBack(str(), alloc(), huge);

  EXPECT_FALSE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), cel_StringView_Size(huge));
  EXPECT_EQ(_cel_String_SizeInt(str()), cel_StringView_SizeInt(huge));
  EXPECT_GE(_cel_String_Capacity(str()), _cel_String_Size(str()));
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()), huge));
  EXPECT_TRUE(cel_StringView_Equals(_cel_String_ToStringView(str()), huge));

  _cel_String_Clear(str());

  EXPECT_TRUE(_cel_String_ShrinkToFit(str(), alloc()));

  EXPECT_TRUE(_cel_String_Empty(str()));
  EXPECT_EQ(_cel_String_Size(str()), 0);
  EXPECT_EQ(_cel_String_SizeInt(str()), 0);
  EXPECT_GE(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  EXPECT_THAT(_cel_String_Data(str()), NotNull());
  EXPECT_EQ(_cel_String_MutableData(str()), _cel_String_Data(str()));
  EXPECT_TRUE(cel_StringView_Empty(_cel_String_ToStringView(str())));
  EXPECT_TRUE(cel_StringView_Empty(_cel_String_ToStringView(str())));

  _cel_String_Reset(str(), alloc());
}

TEST_F(StringTest, Reserve) {
  EXPECT_EQ(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  ASSERT_TRUE(
      _cel_String_Reserve(str(), alloc(), _cel_GenericStringSmall_kCapacity));
  EXPECT_EQ(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity);
  ASSERT_TRUE(_cel_String_Reserve(str(), alloc(),
                                  _cel_GenericStringSmall_kCapacity + 1));
  EXPECT_GE(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity + 1);
  ASSERT_TRUE(_cel_String_Reserve(str(), alloc(),
                                  _cel_GenericStringSmall_kCapacity + 2));
  EXPECT_GE(_cel_String_Capacity(str()), _cel_GenericStringSmall_kCapacity + 2);
}

TEST_F(StringTest, Stability) {
  cel_StringView small = cel_StringView_FromString("Hello World!");
  ASSERT_TRUE(_cel_String_Append(str(), alloc(), small));
  _cel_String other_str = *str();
  EXPECT_NE(_cel_String_Data(&other_str), _cel_String_Data(str()));
  ASSERT_TRUE(_cel_String_Stabilize(str(), alloc()));
  other_str = *str();
  EXPECT_EQ(_cel_String_Data(&other_str), _cel_String_Data(str()));
  _cel_String_Destabilize(str(), alloc());
  other_str = *str();
  EXPECT_NE(_cel_String_Data(&other_str), _cel_String_Data(str()));
}

}  // namespace
