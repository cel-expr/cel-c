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

#include "cel-c/internal/empty_list_value.h"

#include <stddef.h>

#include <memory>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/internal/value_testing.h"
#include "cel-c/value.h"

namespace {

using ::testing::NotNull;

using EmptyListValueTest = ValueTest;

struct ListValueDeleter {
  void operator()(cel_ListValueIterator* iterator) const {
    cel_ListValueIterator_Delete(iterator);
  }
};

using ListValueIteratorPtr =
    std::unique_ptr<cel_ListValueIterator, ListValueDeleter>;

TEST_F(EmptyListValueTest, Equals) {
  cel_ListValue list_value;
  _cel_EmptyListValue_Set(&list_value);
  cel_ListValue other_list_value;
  _cel_EmptyListValue_Set(&other_list_value);

  cel_Value result;

  ASSERT_TRUE(cel_ListValue_Equals(&list_value, ctx(), &other_list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
}

TEST_F(EmptyListValueTest, Size) {
  cel_Value result;

  cel_ListValue list_value;
  _cel_EmptyListValue_Set(&list_value);
  ASSERT_TRUE(cel_ListValue_Size(&list_value, ctx(), &result, status()));
  ASSERT_TRUE(cel_Value_IsInt(&result));
  EXPECT_EQ(cel_Value_GetInt(&result), 0);

  size_t size;
  ASSERT_TRUE((*list_value.vtable->FastSize)(list_value.vtable,
                                             list_value.content, &size));
  EXPECT_EQ(size, 0);
}

TEST_F(EmptyListValueTest, Get) {
  cel_ListValue list_value;
  _cel_EmptyListValue_Set(&list_value);

  cel_Value result;

  ASSERT_TRUE(cel_ListValue_Get(&list_value, ctx(), 0, &result, status()));
  EXPECT_TRUE(cel_Value_IsError(&result));
}

TEST_F(EmptyListValueTest, NewIterator) {
  cel_ListValue list_value;
  _cel_EmptyListValue_Set(&list_value);

  ListValueIteratorPtr iter(
      cel_ListValue_NewIterator(&list_value, ctx(), status()));
  ASSERT_THAT(iter, NotNull());

  size_t remaining;
  ASSERT_TRUE(cel_ListValueIterator_Remaining(iter.get(), &remaining));
  EXPECT_EQ(remaining, 0);

  size_t index;
  cel_Value key;
  cel_Value value;

  EXPECT_FALSE(
      cel_ListValueIterator_Next1(iter.get(), ctx(), &value, status()));
  EXPECT_FALSE(
      cel_ListValueIterator_Next2(iter.get(), ctx(), &key, &value, status()));
  EXPECT_FALSE(
      cel_ListValueIterator_Next(iter.get(), ctx(), &index, &value, status()));
}

}  // namespace
