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

#include "cel-c/internal/mutable_list_value.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/internal/value_testing.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"
#include "cel-c/value.h"

namespace {

using ::testing::ElementsAre;
using ::testing::NotNull;
using ::testing::Pair;

struct ListValueDeleter {
  void operator()(cel_ListValueIterator* iterator) const {
    cel_ListValueIterator_Delete(iterator);
  }
};

using ListValueIteratorPtr =
    std::unique_ptr<cel_ListValueIterator, ListValueDeleter>;

using MutableListValueTest = ValueTest;

TEST_F(MutableListValueTest, Equals) {
  cel_ListValue list_value;
  _cel_MutableListValue_Set(&list_value);

  cel_ListValue other_list_value;
  _cel_MutableListValue_Set(&other_list_value);

  cel_Value result;

  ASSERT_TRUE(cel_ListValue_Equals(&list_value, ctx(), &other_list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
  ASSERT_TRUE(cel_ListValue_Equals(&other_list_value, ctx(), &list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  cel_Value* element = _cel_MutableListValue_Add(&list_value, arena());
  ASSERT_THAT(element, NotNull());
  cel_Value_SetInt(element, 1);

  ASSERT_TRUE(cel_ListValue_Equals(&list_value, ctx(), &other_list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
  ASSERT_TRUE(cel_ListValue_Equals(&other_list_value, ctx(), &list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value* other_element =
      _cel_MutableListValue_Add(&other_list_value, arena());
  ASSERT_THAT(other_element, NotNull());
  cel_Value_SetInt(other_element, 1);

  ASSERT_TRUE(cel_ListValue_Equals(&list_value, ctx(), &other_list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
  ASSERT_TRUE(cel_ListValue_Equals(&other_list_value, ctx(), &list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
}

TEST_F(MutableListValueTest, Size) {
  cel_ListValue list_value;
  _cel_MutableListValue_Set(&list_value);

  cel_Value result;
  ASSERT_TRUE(cel_ListValue_Size(&list_value, ctx(), &result, status()));
  ASSERT_TRUE(cel_Value_IsInt(&result));
  EXPECT_EQ(cel_Value_GetInt(&result), 0);

  size_t size;
  ASSERT_TRUE((*list_value.vtable->FastSize)(list_value.vtable,
                                             list_value.content, &size));
  EXPECT_EQ(size, 0);

  cel_Value* element = _cel_MutableListValue_Add(&list_value, arena());
  ASSERT_THAT(element, NotNull());
  cel_Value_SetInt(element, 1);

  ASSERT_TRUE(cel_ListValue_Size(&list_value, ctx(), &result, status()));
  ASSERT_TRUE(cel_Value_IsInt(&result));
  EXPECT_EQ(cel_Value_GetInt(&result), 1);

  ASSERT_TRUE((*list_value.vtable->FastSize)(list_value.vtable,
                                             list_value.content, &size));
  EXPECT_EQ(size, 1);
}

TEST_F(MutableListValueTest, Get) {
  cel_ListValue list_value;
  _cel_MutableListValue_Set(&list_value);

  cel_Value result;
  ASSERT_TRUE(cel_ListValue_Get(&list_value, ctx(), 0, &result, status()));
  EXPECT_TRUE(cel_Value_IsError(&result));

  cel_Value* element = _cel_MutableListValue_Add(&list_value, arena());
  ASSERT_THAT(element, NotNull());
  cel_Value_SetInt(element, 1);

  ASSERT_TRUE(cel_ListValue_Get(&list_value, ctx(), 0, &result, status()));
  ASSERT_TRUE(cel_Value_IsInt(&result));
  EXPECT_EQ(cel_Value_GetInt(&result), 1);
}

TEST_F(MutableListValueTest, NewIterator) {
  cel_ListValue list_value;
  _cel_MutableListValue_Set(&list_value);

  cel_Value* elements = _cel_MutableListValue_AddN(&list_value, 2, arena());
  ASSERT_THAT(elements, NotNull());
  cel_Value_SetString(elements, cel_StringView_FromString("foo"));
  cel_Value_SetString(elements + 1, cel_StringView_FromString("bar"));

  ListValueIteratorPtr iter(
      cel_ListValue_NewIterator(&list_value, ctx(), status()));
  ASSERT_THAT(iter, NotNull());

  size_t remaining;
  ASSERT_TRUE(cel_ListValueIterator_Remaining(iter.get(), &remaining));
  EXPECT_EQ(remaining, 2);

  {
    ListValueIteratorPtr iter(
        cel_ListValue_NewIterator(&list_value, ctx(), status()));
    ASSERT_THAT(iter, NotNull());

    size_t remaining;
    ASSERT_TRUE(cel_ListValueIterator_Remaining(iter.get(), &remaining));
    EXPECT_EQ(remaining, 2);

    cel_Value value;

    std::vector<std::string> values;

    ASSERT_TRUE(
        cel_ListValueIterator_Next1(iter.get(), ctx(), &value, status()));
    ASSERT_TRUE(cel_Value_IsString(&value));
    values.emplace_back(cel_StringView_ToAbsl(cel_Value_GetString(&value)));

    ASSERT_TRUE(
        cel_ListValueIterator_Next1(iter.get(), ctx(), &value, status()));
    ASSERT_TRUE(cel_Value_IsString(&value));
    values.emplace_back(cel_StringView_ToAbsl(cel_Value_GetString(&value)));

    ASSERT_FALSE(
        cel_ListValueIterator_Next1(iter.get(), ctx(), &value, status()));

    EXPECT_THAT(values, ElementsAre("foo", "bar"));
  }

  {
    ListValueIteratorPtr iter(
        cel_ListValue_NewIterator(&list_value, ctx(), status()));
    ASSERT_THAT(iter, NotNull());

    size_t remaining;
    ASSERT_TRUE(cel_ListValueIterator_Remaining(iter.get(), &remaining));
    EXPECT_EQ(remaining, 2);

    cel_Value key;
    cel_Value value;

    std::vector<std::pair<int64_t, std::string>> elements;

    ASSERT_TRUE(
        cel_ListValueIterator_Next2(iter.get(), ctx(), &key, &value, status()));
    ASSERT_TRUE(cel_Value_IsInt(&key));
    ASSERT_TRUE(cel_Value_IsString(&value));
    elements.emplace_back(cel_Value_GetInt(&key),
                          cel_StringView_ToAbsl(cel_Value_GetString(&value)));

    ASSERT_TRUE(
        cel_ListValueIterator_Next2(iter.get(), ctx(), &key, &value, status()));
    ASSERT_TRUE(cel_Value_IsInt(&key));
    ASSERT_TRUE(cel_Value_IsString(&value));
    elements.emplace_back(cel_Value_GetInt(&key),
                          cel_StringView_ToAbsl(cel_Value_GetString(&value)));

    ASSERT_FALSE(
        cel_ListValueIterator_Next2(iter.get(), ctx(), &key, &value, status()));

    EXPECT_THAT(elements, ElementsAre(Pair(0, "foo"), Pair(1, "bar")));
  }

  {
    ListValueIteratorPtr iter(
        cel_ListValue_NewIterator(&list_value, ctx(), status()));
    ASSERT_THAT(iter, NotNull());

    size_t remaining;
    ASSERT_TRUE(cel_ListValueIterator_Remaining(iter.get(), &remaining));
    EXPECT_EQ(remaining, 2);

    size_t index;
    cel_Value value;

    std::vector<std::pair<size_t, std::string>> elements;

    ASSERT_TRUE(cel_ListValueIterator_Next(iter.get(), ctx(), &index, &value,
                                           status()));
    ASSERT_TRUE(cel_Value_IsString(&value));
    elements.emplace_back(index,
                          cel_StringView_ToAbsl(cel_Value_GetString(&value)));

    ASSERT_TRUE(cel_ListValueIterator_Next(iter.get(), ctx(), &index, &value,
                                           status()));
    ASSERT_TRUE(cel_Value_IsString(&value));
    elements.emplace_back(index,
                          cel_StringView_ToAbsl(cel_Value_GetString(&value)));

    ASSERT_FALSE(cel_ListValueIterator_Next(iter.get(), ctx(), &index, &value,
                                            status()));

    EXPECT_THAT(elements, ElementsAre(Pair(0, "foo"), Pair(1, "bar")));
  }
}

}  // namespace
