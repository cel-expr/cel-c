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

#include "cel-c/src/empty_map_value.h"

#include <stddef.h>

#include <memory>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/src/value_testing.h"
#include "cel-c/status.h"
#include "cel-c/value.h"

namespace {

using ::testing::NotNull;

using EmptyMapValueTest = ValueTest;

struct MapValueDeleter {
  void operator()(cel_MapValueIterator* iterator) const {
    cel_MapValueIterator_Delete(iterator);
  }
};

using MapValueIteratorPtr =
    std::unique_ptr<cel_MapValueIterator, MapValueDeleter>;

TEST_F(EmptyMapValueTest, Equals) {
  cel_MapValue map_value;
  _cel_EmptyMapValue_Set(&map_value);
  cel_MapValue other_map_value;
  _cel_EmptyMapValue_Set(&other_map_value);

  cel_Value result;

  ASSERT_TRUE(cel_MapValue_Equals(&map_value, ctx(), &other_map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
}

TEST_F(EmptyMapValueTest, Size) {
  cel_Value result;

  cel_MapValue map_value;
  _cel_EmptyMapValue_Set(&map_value);
  ASSERT_TRUE(cel_MapValue_Size(&map_value, ctx(), &result, status()));
  ASSERT_TRUE(cel_Value_IsInt(&result));
  EXPECT_EQ(cel_Value_GetInt(&result), 0);

  size_t size;
  ASSERT_TRUE((*map_value.vtable->FastSize)(map_value.vtable, map_value.content,
                                            &size));
  EXPECT_EQ(size, 0);
}

TEST_F(EmptyMapValueTest, Get) {
  cel_MapValue map_value;
  _cel_EmptyMapValue_Set(&map_value);

  cel_MapValueKey key;
  cel_Value result;

  cel_MapValueKey_SetInt(&key, 1);
  ASSERT_TRUE(cel_MapValue_Get(&map_value, ctx(), &key, &result, status()));
  EXPECT_TRUE(cel_Value_IsError(&result));
}

TEST_F(EmptyMapValueTest, Find) {
  cel_MapValue map_value;
  _cel_EmptyMapValue_Set(&map_value);

  cel_MapValueKey key;
  cel_Value result;

  cel_MapValueKey_SetInt(&key, 1);
  ASSERT_FALSE(cel_MapValue_Find(&map_value, ctx(), &key, &result, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
}

TEST_F(EmptyMapValueTest, Has) {
  cel_MapValue map_value;
  _cel_EmptyMapValue_Set(&map_value);

  cel_MapValueKey key;
  cel_Value result;

  cel_MapValueKey_SetInt(&key, 1);
  ASSERT_TRUE(cel_MapValue_Has(&map_value, ctx(), &key, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

TEST_F(EmptyMapValueTest, NewIterator) {
  cel_MapValue map_value;
  _cel_EmptyMapValue_Set(&map_value);

  MapValueIteratorPtr iter(
      cel_MapValue_NewIterator(&map_value, ctx(), status()));
  ASSERT_THAT(iter, NotNull());

  size_t remaining;
  ASSERT_TRUE(cel_MapValueIterator_Remaining(iter.get(), &remaining));
  EXPECT_EQ(remaining, 0);

  cel_MapValueKey map_key;
  cel_Value key;
  cel_Value value;

  EXPECT_FALSE(cel_MapValueIterator_Next1(iter.get(), ctx(), &value, status()));
  EXPECT_FALSE(
      cel_MapValueIterator_Next2(iter.get(), ctx(), &key, &value, status()));
  EXPECT_FALSE(
      cel_MapValueIterator_Next(iter.get(), ctx(), &map_key, &value, status()));
}

}  // namespace
