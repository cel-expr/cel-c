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

#include "cel-c/internal/mutable_map_value.h"

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

using ::testing::NotNull;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;

struct MapValueIteratorDeleter {
  void operator()(cel_MapValueIterator* iterator) const {
    cel_MapValueIterator_Delete(iterator);
  }
};

using MapValueIteratorPtr =
    std::unique_ptr<cel_MapValueIterator, MapValueIteratorDeleter>;

using MutableMapValueTest = ValueTest;

TEST_F(MutableMapValueTest, Equals) {
  cel_MapValue map_value;
  _cel_MutableMapValue_Set(&map_value);

  cel_MapValue other_map_value;
  _cel_MutableMapValue_Set(&other_map_value);

  cel_Value result;

  ASSERT_TRUE(cel_MapValue_Equals(&map_value, ctx(), &other_map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
  ASSERT_TRUE(cel_MapValue_Equals(&other_map_value, ctx(), &map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  cel_MapValueKey key;
  cel_MapValueKey_SetInt(&key, 1);

  cel_Value* value;

  ASSERT_EQ(
      _cel_MutableMapValue_Insert(&map_value, &key, nullptr, &value, arena()),
      _cel_MutableMapValueInsertResult_kInserted);
  ASSERT_THAT(value, NotNull());
  cel_Value_SetInt(value, 2);

  ASSERT_TRUE(cel_MapValue_Equals(&map_value, ctx(), &other_map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
  ASSERT_TRUE(cel_MapValue_Equals(&other_map_value, ctx(), &map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  ASSERT_EQ(_cel_MutableMapValue_Insert(&other_map_value, &key, nullptr, &value,
                                        arena()),
            _cel_MutableMapValueInsertResult_kInserted);
  ASSERT_THAT(value, NotNull());
  cel_Value_SetInt(value, 2);

  ASSERT_TRUE(cel_MapValue_Equals(&map_value, ctx(), &other_map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
  ASSERT_TRUE(cel_MapValue_Equals(&other_map_value, ctx(), &map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
}

TEST_F(MutableMapValueTest, Size) {
  cel_MapValue map_value;
  _cel_MutableMapValue_Set(&map_value);

  cel_Value result;
  ASSERT_TRUE(cel_MapValue_Size(&map_value, ctx(), &result, status()));
  ASSERT_TRUE(cel_Value_IsInt(&result));
  EXPECT_EQ(cel_Value_GetInt(&result), 0);

  size_t size;
  ASSERT_TRUE((*map_value.vtable->FastSize)(map_value.vtable, map_value.content,
                                            &size));
  EXPECT_EQ(size, 0);

  cel_MapValueKey key;
  cel_MapValueKey_SetInt(&key, 1);

  cel_Value* value;

  ASSERT_EQ(
      _cel_MutableMapValue_Insert(&map_value, &key, nullptr, &value, arena()),
      _cel_MutableMapValueInsertResult_kInserted);
  ASSERT_THAT(value, NotNull());
  cel_Value_SetInt(value, 2);

  ASSERT_TRUE(cel_MapValue_Size(&map_value, ctx(), &result, status()));
  ASSERT_TRUE(cel_Value_IsInt(&result));
  EXPECT_EQ(cel_Value_GetInt(&result), 1);

  ASSERT_TRUE((*map_value.vtable->FastSize)(map_value.vtable, map_value.content,
                                            &size));
  EXPECT_EQ(size, 1);
}

TEST_F(MutableMapValueTest, Get) {
  cel_MapValue map_value;
  _cel_MutableMapValue_Set(&map_value);

  cel_MapValueKey key;
  cel_MapValueKey_SetInt(&key, 1);

  cel_Value result;
  ASSERT_TRUE(cel_MapValue_Get(&map_value, ctx(), &key, &result, status()));
  EXPECT_TRUE(cel_Value_IsError(&result));

  cel_Value* value;

  ASSERT_EQ(
      _cel_MutableMapValue_Insert(&map_value, &key, nullptr, &value, arena()),
      _cel_MutableMapValueInsertResult_kInserted);
  ASSERT_THAT(value, NotNull());
  cel_Value_SetInt(value, 2);

  ASSERT_TRUE(cel_MapValue_Get(&map_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsInt(&result));
  EXPECT_EQ(cel_Value_GetInt(&result), 2);
}

TEST_F(MutableMapValueTest, Find) {
  cel_MapValue map_value;
  _cel_MutableMapValue_Set(&map_value);

  cel_MapValueKey key;
  cel_MapValueKey_SetInt(&key, 1);

  cel_Value result;
  ASSERT_FALSE(cel_MapValue_Find(&map_value, ctx(), &key, &result, status()));

  cel_Value* value;

  ASSERT_EQ(
      _cel_MutableMapValue_Insert(&map_value, &key, nullptr, &value, arena()),
      _cel_MutableMapValueInsertResult_kInserted);
  ASSERT_THAT(value, NotNull());
  cel_Value_SetInt(value, 2);

  ASSERT_TRUE(cel_MapValue_Find(&map_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsInt(&result));
  EXPECT_EQ(cel_Value_GetInt(&result), 2);
}

TEST_F(MutableMapValueTest, Has) {
  cel_MapValue map_value;
  _cel_MutableMapValue_Set(&map_value);

  cel_MapValueKey key;
  cel_MapValueKey_SetInt(&key, 1);

  cel_Value result;
  ASSERT_TRUE(cel_MapValue_Has(&map_value, ctx(), &key, &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  cel_Value* value;

  ASSERT_EQ(
      _cel_MutableMapValue_Insert(&map_value, &key, nullptr, &value, arena()),
      _cel_MutableMapValueInsertResult_kInserted);
  ASSERT_THAT(value, NotNull());
  cel_Value_SetInt(value, 2);

  ASSERT_TRUE(cel_MapValue_Has(&map_value, ctx(), &key, &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
}

TEST_F(MutableMapValueTest, NewIterator) {
  cel_MapValue map_value;
  _cel_MutableMapValue_Set(&map_value);

  cel_MapValueKey key;
  cel_Value* value;

  cel_MapValueKey_SetInt(&key, 1);
  ASSERT_EQ(
      _cel_MutableMapValue_Insert(&map_value, &key, nullptr, &value, arena()),
      _cel_MutableMapValueInsertResult_kInserted);
  cel_Value_SetString(value, cel_StringView_FromString("foo"));

  cel_MapValueKey_SetInt(&key, 2);
  ASSERT_EQ(
      _cel_MutableMapValue_Insert(&map_value, &key, nullptr, &value, arena()),
      _cel_MutableMapValueInsertResult_kInserted);
  cel_Value_SetString(value, cel_StringView_FromString("bar"));

  {
    MapValueIteratorPtr iter(
        cel_MapValue_NewIterator(&map_value, ctx(), status()));
    ASSERT_THAT(iter, NotNull());

    size_t remaining;
    ASSERT_TRUE(cel_MapValueIterator_Remaining(iter.get(), &remaining));
    EXPECT_EQ(remaining, 2);

    cel_Value key;

    std::vector<int64_t> keys;

    ASSERT_TRUE(cel_MapValueIterator_Next1(iter.get(), ctx(), &key, status()));
    ASSERT_TRUE(cel_Value_IsInt(&key));
    keys.push_back(cel_Value_GetInt(&key));

    ASSERT_TRUE(cel_MapValueIterator_Next1(iter.get(), ctx(), &key, status()));
    ASSERT_TRUE(cel_Value_IsInt(&key));
    keys.push_back(cel_Value_GetInt(&key));

    ASSERT_FALSE(cel_MapValueIterator_Next1(iter.get(), ctx(), &key, status()));

    EXPECT_THAT(keys, UnorderedElementsAre(1, 2));
  }

  {
    MapValueIteratorPtr iter(
        cel_MapValue_NewIterator(&map_value, ctx(), status()));
    ASSERT_THAT(iter, NotNull());

    size_t remaining;
    ASSERT_TRUE(cel_MapValueIterator_Remaining(iter.get(), &remaining));
    EXPECT_EQ(remaining, 2);

    cel_Value key;
    cel_Value value;

    std::vector<std::pair<int64_t, std::string>> entries;

    ASSERT_TRUE(
        cel_MapValueIterator_Next2(iter.get(), ctx(), &key, &value, status()));
    ASSERT_TRUE(cel_Value_IsInt(&key));
    ASSERT_TRUE(cel_Value_IsString(&value));
    entries.emplace_back(cel_Value_GetInt(&key),
                         cel_StringView_ToAbsl(cel_Value_GetString(&value)));

    ASSERT_TRUE(
        cel_MapValueIterator_Next2(iter.get(), ctx(), &key, &value, status()));
    ASSERT_TRUE(cel_Value_IsInt(&key));
    ASSERT_TRUE(cel_Value_IsString(&value));
    entries.emplace_back(cel_Value_GetInt(&key),
                         cel_StringView_ToAbsl(cel_Value_GetString(&value)));

    ASSERT_FALSE(
        cel_MapValueIterator_Next2(iter.get(), ctx(), &key, &value, status()));

    EXPECT_THAT(entries, UnorderedElementsAre(Pair(1, "foo"), Pair(2, "bar")));
  }

  {
    MapValueIteratorPtr iter(
        cel_MapValue_NewIterator(&map_value, ctx(), status()));
    ASSERT_THAT(iter, NotNull());

    size_t remaining;
    ASSERT_TRUE(cel_MapValueIterator_Remaining(iter.get(), &remaining));
    EXPECT_EQ(remaining, 2);

    cel_MapValueKey key;
    cel_Value value;

    std::vector<std::pair<int64_t, std::string>> entries;

    ASSERT_TRUE(
        cel_MapValueIterator_Next(iter.get(), ctx(), &key, &value, status()));
    ASSERT_TRUE(cel_MapValueKey_IsInt(&key));
    ASSERT_TRUE(cel_Value_IsString(&value));
    entries.emplace_back(cel_MapValueKey_GetInt(&key),
                         cel_StringView_ToAbsl(cel_Value_GetString(&value)));

    ASSERT_TRUE(
        cel_MapValueIterator_Next(iter.get(), ctx(), &key, &value, status()));
    ASSERT_TRUE(cel_MapValueKey_IsInt(&key));
    ASSERT_TRUE(cel_Value_IsString(&value));
    entries.emplace_back(cel_MapValueKey_GetInt(&key),
                         cel_StringView_ToAbsl(cel_Value_GetString(&value)));

    ASSERT_FALSE(
        cel_MapValueIterator_Next(iter.get(), ctx(), &key, &value, status()));

    EXPECT_THAT(entries, UnorderedElementsAre(Pair(1, "foo"), Pair(2, "bar")));
  }
}

}  // namespace
