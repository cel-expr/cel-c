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

#include "cel-c/src/map_value.h"

#include <cstddef>
#include <cstdint>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/config.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/hash.h"
#include "cel-c/src/empty_map_value.h"
#include "cel-c/src/value_testing.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/value.h"
#include "upb/base/descriptor_constants.h"
#include "upb/message/array.h"

namespace {

using ::testing::Eq;
using ::testing::Gt;
using ::testing::Lt;

TEST_F(ValueTest, SetMapValueKey_Bool) {
  cel_MapValueKey key;
  cel_Value value;

  cel_MapValueKey_SetBool(&key, true);

  _cel_Value_SetMapValueKey(&value, &key);
  EXPECT_TRUE(cel_Value_IsTrue(&value));
}

TEST_F(ValueTest, SetMapValueKey_Int) {
  cel_MapValueKey key;
  cel_Value value;

  cel_MapValueKey_SetInt(&key, 1);

  _cel_Value_SetMapValueKey(&value, &key);
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);
}

TEST_F(ValueTest, SetMapValueKey_Uint) {
  cel_MapValueKey key;
  cel_Value value;

  cel_MapValueKey_SetUint(&key, 1);

  _cel_Value_SetMapValueKey(&value, &key);
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 1);
}

TEST_F(ValueTest, SetMapValueKey_String) {
  cel_MapValueKey key;
  cel_Value value;

  cel_MapValueKey_SetString(&key, cel_StringView_FromString("foo"));

  _cel_Value_SetMapValueKey(&value, &key);
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_FromString("foo"));
}

using MapValueKeyTest = ValueTest;

TEST_F(MapValueKeyTest, Compare_Bool) {
  cel_MapValueKey key;
  cel_MapValueKey other_key;

  cel_MapValueKey_SetBool(&key, false);
  cel_MapValueKey_SetBool(&other_key, false);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Eq(0));

  cel_MapValueKey_SetBool(&other_key, true);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Lt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Gt(0));

  cel_MapValueKey_SetInt(&other_key, 1);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Lt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Gt(0));

  cel_MapValueKey_SetUint(&other_key, 1);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Lt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Gt(0));

  cel_MapValueKey_SetString(&other_key, cel_StringView_FromString("foo"));
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Lt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Gt(0));
}

TEST_F(MapValueKeyTest, Compare_Int) {
  cel_MapValueKey key;
  cel_MapValueKey other_key;

  cel_MapValueKey_SetInt(&key, 1);
  cel_MapValueKey_SetInt(&other_key, 1);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Eq(0));

  cel_MapValueKey_SetBool(&other_key, true);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Gt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Lt(0));

  cel_MapValueKey_SetInt(&other_key, 0);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Gt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Lt(0));

  cel_MapValueKey_SetInt(&other_key, 2);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Lt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Gt(0));

  cel_MapValueKey_SetUint(&other_key, 1);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Eq(0));

  cel_MapValueKey_SetUint(&other_key, 0);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Gt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Lt(0));

  cel_MapValueKey_SetUint(&other_key, 2);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Lt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Gt(0));

  cel_MapValueKey_SetString(&other_key, cel_StringView_FromString("foo"));
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Lt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Gt(0));
}

TEST_F(MapValueKeyTest, Compare_Uint) {
  cel_MapValueKey key;
  cel_MapValueKey other_key;

  cel_MapValueKey_SetUint(&key, 1);
  cel_MapValueKey_SetUint(&other_key, 1);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Eq(0));

  cel_MapValueKey_SetBool(&other_key, true);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Gt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Lt(0));

  cel_MapValueKey_SetInt(&other_key, 0);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Gt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Lt(0));

  cel_MapValueKey_SetInt(&other_key, 1);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Eq(0));

  cel_MapValueKey_SetInt(&other_key, 2);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Lt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Gt(0));

  cel_MapValueKey_SetUint(&other_key, 0);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Gt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Lt(0));

  cel_MapValueKey_SetUint(&other_key, 2);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Lt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Gt(0));

  cel_MapValueKey_SetString(&other_key, cel_StringView_FromString("foo"));
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Lt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Gt(0));
}

TEST_F(MapValueKeyTest, Compare_String) {
  cel_MapValueKey key;
  cel_MapValueKey other_key;

  cel_MapValueKey_SetString(&key, cel_StringView_FromString("foo"));
  cel_MapValueKey_SetString(&other_key, cel_StringView_FromString("foo"));
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Eq(0));

  cel_MapValueKey_SetBool(&other_key, true);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Gt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Lt(0));

  cel_MapValueKey_SetInt(&other_key, 1);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Gt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Lt(0));

  cel_MapValueKey_SetUint(&other_key, 1);
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Gt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Lt(0));

  cel_MapValueKey_SetString(&other_key, cel_StringView_FromString("bar"));
  EXPECT_THAT(_cel_MapValueKey_Compare(&key, &other_key), Gt(0));
  EXPECT_THAT(_cel_MapValueKey_Compare(&other_key, &key), Lt(0));
}

TEST_F(MapValueKeyTest, Equals_Bool) {
  cel_MapValueKey key;
  cel_MapValueKey other_key;

  cel_MapValueKey_SetBool(&key, false);
  cel_MapValueKey_SetBool(&other_key, false);

  EXPECT_TRUE(_cel_MapValueKey_Equals(&key, &other_key));

  cel_MapValueKey_SetBool(&other_key, true);
  EXPECT_FALSE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&other_key, &key));

  cel_MapValueKey_SetInt(&other_key, 1);
  EXPECT_FALSE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&other_key, &key));

  cel_MapValueKey_SetUint(&other_key, 1);
  EXPECT_FALSE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&other_key, &key));

  cel_MapValueKey_SetString(&other_key, cel_StringView_FromString("foo"));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&other_key, &key));
}

TEST_F(MapValueKeyTest, Equals_Int) {
  cel_MapValueKey key;
  cel_MapValueKey other_key;

  cel_MapValueKey_SetInt(&key, 1);
  cel_MapValueKey_SetInt(&other_key, 1);

  EXPECT_TRUE(_cel_MapValueKey_Equals(&key, &other_key));

  cel_MapValueKey_SetInt(&other_key, 2);
  EXPECT_FALSE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&other_key, &key));

  cel_MapValueKey_SetUint(&other_key, 1);
  EXPECT_TRUE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_TRUE(_cel_MapValueKey_Equals(&other_key, &key));

  cel_MapValueKey_SetInt(&key, -1);
  cel_MapValueKey_SetUint(&other_key, UINT64_MAX);
  EXPECT_FALSE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&other_key, &key));

  cel_MapValueKey_SetString(&other_key, cel_StringView_FromString("foo"));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&other_key, &key));
}

TEST_F(MapValueKeyTest, Equals_Uint) {
  cel_MapValueKey key;
  cel_MapValueKey other_key;

  cel_MapValueKey_SetUint(&key, 1);
  cel_MapValueKey_SetUint(&other_key, 1);

  EXPECT_TRUE(_cel_MapValueKey_Equals(&key, &other_key));

  cel_MapValueKey_SetUint(&other_key, 2);
  EXPECT_FALSE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&other_key, &key));

  cel_MapValueKey_SetInt(&other_key, 1);
  EXPECT_TRUE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_TRUE(_cel_MapValueKey_Equals(&other_key, &key));

  cel_MapValueKey_SetUint(&key, UINT64_MAX);
  cel_MapValueKey_SetInt(&other_key, -1);
  EXPECT_FALSE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&other_key, &key));

  cel_MapValueKey_SetString(&other_key, cel_StringView_FromString("foo"));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&other_key, &key));
}

TEST_F(MapValueKeyTest, Equals_String) {
  cel_MapValueKey key;
  cel_MapValueKey other_key;

  cel_MapValueKey_SetString(&key, cel_StringView_FromString("foo"));
  cel_MapValueKey_SetString(&other_key, cel_StringView_FromString("foo"));

  EXPECT_TRUE(_cel_MapValueKey_Equals(&key, &other_key));

  cel_MapValueKey_SetString(&other_key, cel_StringView_FromString("bar"));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&other_key, &key));

  cel_MapValueKey_SetInt(&other_key, 1);
  EXPECT_FALSE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&other_key, &key));

  cel_MapValueKey_SetUint(&other_key, 1);
  EXPECT_FALSE(_cel_MapValueKey_Equals(&key, &other_key));
  EXPECT_FALSE(_cel_MapValueKey_Equals(&other_key, &key));
}

TEST_F(MapValueKeyTest, Hash_Bool) {
  cel_MapValueKey key;

  cel_MapValueKey_SetBool(&key, true);
  EXPECT_EQ(cel_HashState_Finalize(
                _cel_MapValueKey_Hash(&key, cel_HashState_Initialize())),
            cel_HashState_Finalize(
                _cel_MapValueKey_Hash(&key, cel_HashState_Initialize())));
}

TEST_F(MapValueKeyTest, Hash_Int) {
  cel_MapValueKey int_key;
  cel_MapValueKey uint_key;

  cel_MapValueKey_SetInt(&int_key, 1);
  cel_MapValueKey_SetUint(&uint_key, 1);
  EXPECT_EQ(cel_HashState_Finalize(
                _cel_MapValueKey_Hash(&int_key, cel_HashState_Initialize())),
            cel_HashState_Finalize(
                _cel_MapValueKey_Hash(&uint_key, cel_HashState_Initialize())));
}

TEST_F(MapValueKeyTest, Hash_String) {
  cel_MapValueKey key;

  cel_MapValueKey_SetString(&key, cel_StringView_FromString("foo"));
  EXPECT_EQ(cel_HashState_Finalize(
                _cel_MapValueKey_Hash(&key, cel_HashState_Initialize())),
            cel_HashState_Finalize(
                _cel_MapValueKey_Hash(&key, cel_HashState_Initialize())));
}

TEST_F(MapValueKeyTest, ToMessageValue_Bool) {
  cel_MapValueKey key;
  upb_MessageValue message_val;

  cel_MapValueKey_SetBool(&key, true);
  ASSERT_TRUE(
      _cel_MapValueKey_ToMessageValue(&key, kUpb_CType_Bool, &message_val));
  EXPECT_TRUE(message_val.bool_val);

  EXPECT_FALSE(
      _cel_MapValueKey_ToMessageValue(&key, kUpb_CType_Int32, &message_val));
}

TEST_F(MapValueKeyTest, ToMessageValue_Int) {
  cel_MapValueKey key;
  upb_MessageValue message_val;

  cel_MapValueKey_SetInt(&key, 1);
  ASSERT_TRUE(
      _cel_MapValueKey_ToMessageValue(&key, kUpb_CType_Int32, &message_val));
  EXPECT_EQ(message_val.int32_val, 1);
  ASSERT_TRUE(
      _cel_MapValueKey_ToMessageValue(&key, kUpb_CType_Int64, &message_val));
  EXPECT_EQ(message_val.int64_val, 1);
  ASSERT_TRUE(
      _cel_MapValueKey_ToMessageValue(&key, kUpb_CType_UInt32, &message_val));
  EXPECT_EQ(message_val.uint32_val, 1);
  ASSERT_TRUE(
      _cel_MapValueKey_ToMessageValue(&key, kUpb_CType_UInt64, &message_val));
  EXPECT_EQ(message_val.uint64_val, 1);

  EXPECT_FALSE(
      _cel_MapValueKey_ToMessageValue(&key, kUpb_CType_Bool, &message_val));
}

TEST_F(MapValueKeyTest, ToMessageValue_Uint) {
  cel_MapValueKey key;
  upb_MessageValue message_val;

  cel_MapValueKey_SetUint(&key, 1);
  ASSERT_TRUE(
      _cel_MapValueKey_ToMessageValue(&key, kUpb_CType_Int32, &message_val));
  EXPECT_EQ(message_val.int32_val, 1);
  ASSERT_TRUE(
      _cel_MapValueKey_ToMessageValue(&key, kUpb_CType_Int64, &message_val));
  EXPECT_EQ(message_val.int64_val, 1);
  ASSERT_TRUE(
      _cel_MapValueKey_ToMessageValue(&key, kUpb_CType_UInt32, &message_val));
  EXPECT_EQ(message_val.uint32_val, 1);
  ASSERT_TRUE(
      _cel_MapValueKey_ToMessageValue(&key, kUpb_CType_UInt64, &message_val));
  EXPECT_EQ(message_val.uint64_val, 1);

  EXPECT_FALSE(
      _cel_MapValueKey_ToMessageValue(&key, kUpb_CType_Bool, &message_val));
}

TEST_F(MapValueKeyTest, ToMessageValue_String) {
  cel_MapValueKey key;
  upb_MessageValue message_val;

  cel_MapValueKey_SetString(&key, cel_StringView_FromString("foo"));
  ASSERT_TRUE(
      _cel_MapValueKey_ToMessageValue(&key, kUpb_CType_String, &message_val));
  EXPECT_EQ(message_val.str_val, cel_StringView_FromString("foo"));

  EXPECT_FALSE(
      _cel_MapValueKey_ToMessageValue(&key, kUpb_CType_Bool, &message_val));
}

TEST_F(MapValueKeyTest, FromMessageValue_Bool) {
  upb_MessageValue message_val;
  cel_MapValueKey key;

  message_val.bool_val = true;
  _cel_MapValueKey_FromMessageValue(&key, kUpb_CType_Bool, message_val);
  ASSERT_TRUE(cel_MapValueKey_IsBool(&key));
  EXPECT_TRUE(cel_MapValueKey_GetBool(&key));
}

TEST_F(MapValueKeyTest, FromMessageValue_Int32) {
  upb_MessageValue message_val;
  cel_MapValueKey key;

  message_val.int32_val = 1;
  _cel_MapValueKey_FromMessageValue(&key, kUpb_CType_Int32, message_val);
  ASSERT_TRUE(cel_MapValueKey_IsInt(&key));
  EXPECT_EQ(cel_MapValueKey_GetInt(&key), 1);
}

TEST_F(MapValueKeyTest, FromMessageValue_Int64) {
  upb_MessageValue message_val;
  cel_MapValueKey key;

  message_val.int64_val = 1;
  _cel_MapValueKey_FromMessageValue(&key, kUpb_CType_Int64, message_val);
  ASSERT_TRUE(cel_MapValueKey_IsInt(&key));
  EXPECT_EQ(cel_MapValueKey_GetInt(&key), 1);
}

TEST_F(MapValueKeyTest, FromMessageValue_UInt32) {
  upb_MessageValue message_val;
  cel_MapValueKey key;

  message_val.uint32_val = 1;
  _cel_MapValueKey_FromMessageValue(&key, kUpb_CType_UInt32, message_val);
  ASSERT_TRUE(cel_MapValueKey_IsUint(&key));
  EXPECT_EQ(cel_MapValueKey_GetUint(&key), 1);
}

TEST_F(MapValueKeyTest, FromMessageValue_UInt64) {
  upb_MessageValue message_val;
  cel_MapValueKey key;

  message_val.uint64_val = 1;
  _cel_MapValueKey_FromMessageValue(&key, kUpb_CType_UInt64, message_val);
  ASSERT_TRUE(cel_MapValueKey_IsUint(&key));
  EXPECT_EQ(cel_MapValueKey_GetUint(&key), 1);
}

TEST_F(MapValueKeyTest, FromMessageValue_String) {
  upb_MessageValue message_val;
  cel_MapValueKey key;

  message_val.str_val = cel_StringView_FromString("foo");
  _cel_MapValueKey_FromMessageValue(&key, kUpb_CType_String, message_val);
  ASSERT_TRUE(cel_MapValueKey_IsString(&key));
  EXPECT_EQ(cel_MapValueKey_GetString(&key), cel_StringView_FromString("foo"));
}

struct TestMapValueIterator : cel_MapValueIterator {
  size_t index = 0;
};

void cel_TestMapValueIterator_Delete(cel_ValueIterator* cel_nonnull iterator) {
  delete reinterpret_cast<const TestMapValueIterator*>(iterator);
}

bool cel_TestMapValueIterator_Next1(cel_ValueIterator* cel_nonnull iterator,
                                    const cel_ValueContext* cel_nonnull context,
                                    cel_Value* cel_nonnull key_or_value,
                                    cel_Status* cel_nonnull status) {
  TestMapValueIterator* iter =
      reinterpret_cast<TestMapValueIterator*>(iterator);
  switch (iter->index) {
    case 0:
      cel_Value_SetUint(key_or_value, 1);
      ++iter->index;
      return true;
    case 1:
      cel_Value_SetString(key_or_value, cel_StringView_FromString("foo"));
      ++iter->index;
      return true;
    case 2:
      cel_Value_SetTrue(key_or_value);
      ++iter->index;
      return true;
    case 3:
      cel_Value_SetInt(key_or_value, -1);
      ++iter->index;
      return true;
    default:
      return false;
  }
}

bool cel_TestMapValueIterator_Next2(cel_ValueIterator* cel_nonnull iterator,
                                    const cel_ValueContext* cel_nonnull context,
                                    cel_Value* cel_nonnull key,
                                    cel_Value* cel_nonnull value,
                                    cel_Status* cel_nonnull status) {
  TestMapValueIterator* iter =
      reinterpret_cast<TestMapValueIterator*>(iterator);
  switch (iter->index) {
    case 0:
      cel_Value_SetUint(key, 1);
      cel_Value_SetInt(value, -2);
      ++iter->index;
      return true;
    case 1:
      cel_Value_SetString(key, cel_StringView_FromString("foo"));
      cel_Value_SetUint(value, 2);
      ++iter->index;
      return true;
    case 2:
      cel_Value_SetTrue(key);
      cel_Value_SetNull(value);
      ++iter->index;
      return true;
    case 3:
      cel_Value_SetInt(key, -1);
      cel_Value_SetTrue(value);
      ++iter->index;
      return true;
    default:
      return false;
  }
}

bool cel_TestMapValueIterator_Remaining(const cel_ValueIterator* cel_nonnull
                                            iterator,
                                        size_t* cel_nonnull remaining) {
  const TestMapValueIterator* iter =
      reinterpret_cast<const TestMapValueIterator*>(iterator);
  *remaining = 4 - iter->index;
  return true;
}

bool cel_TestMapValueIterator_Next(cel_MapValueIterator* cel_nonnull iterator,
                                   const cel_ValueContext* cel_nonnull context,
                                   cel_MapValueKey* cel_nonnull key,
                                   cel_Value* cel_nullable value,
                                   cel_Status* cel_nonnull status) {
  TestMapValueIterator* iter =
      reinterpret_cast<TestMapValueIterator*>(iterator);
  switch (iter->index) {
    case 0:
      cel_MapValueKey_SetUint(key, 1);
      if (value != cel_nullptr) {
        cel_Value_SetInt(value, -2);
      }
      ++iter->index;
      return true;
    case 1:
      cel_MapValueKey_SetString(key, cel_StringView_FromString("foo"));
      if (value != cel_nullptr) {
        cel_Value_SetUint(value, 2);
      }
      ++iter->index;
      return true;
    case 2:
      cel_MapValueKey_SetBool(key, true);
      if (value != cel_nullptr) {
        cel_Value_SetNull(value);
      }
      ++iter->index;
      return true;
    case 3:
      cel_MapValueKey_SetInt(key, -1);
      if (value != cel_nullptr) {
        cel_Value_SetTrue(value);
      }
      ++iter->index;
      return true;
    default:
      return false;
  }
}

const cel_MapValueIteratorVTable fast_test_map_value_iterator_vtable = {
    .super =
        {
            .Delete = &cel_TestMapValueIterator_Delete,
            .Next1 = &cel_TestMapValueIterator_Next1,
            .Next2 = &cel_TestMapValueIterator_Next2,
            .Remaining = &cel_TestMapValueIterator_Remaining,
        },
    .Next = &cel_TestMapValueIterator_Next,
};

const cel_MapValueIteratorVTable slow_test_map_value_iterator_vtable = {
    .super =
        {
            .Delete = &cel_TestMapValueIterator_Delete,
            .Next1 = &cel_TestMapValueIterator_Next1,
            .Next2 = &cel_TestMapValueIterator_Next2,
            .Remaining = cel_nullptr,
        },
    .Next = &cel_TestMapValueIterator_Next,
};

bool cel_TestMapValue_Equals(const cel_MapValueVTable* cel_nonnull vtable,
                             cel_ValueContent content,
                             const cel_ValueContext* cel_nonnull context,
                             const cel_MapValue* cel_nonnull other,
                             cel_Value* cel_nonnull result,
                             cel_Status* cel_nonnull status) {
  return false;
}

bool cel_TestMapValue_FastSize(const cel_MapValueVTable* cel_nonnull vtable,
                               cel_ValueContent content,
                               size_t* cel_nonnull size) {
  *size = 4;
  return true;
}

bool cel_TestMapValue_SlowSize(const cel_MapValueVTable* cel_nonnull vtable,
                               cel_ValueContent content,
                               const cel_ValueContext* cel_nonnull context,
                               cel_Value* cel_nonnull size,
                               cel_Status* cel_nonnull status) {
  cel_Value_SetInt(size, 4);
  return true;
}

bool cel_TestMapValue_Get(const cel_MapValueVTable* cel_nonnull vtable,
                          cel_ValueContent content,
                          const cel_ValueContext* cel_nonnull context,
                          const cel_MapValueKey* cel_nonnull key,
                          cel_Value* cel_nonnull value,
                          cel_Status* cel_nonnull status) {
  if (cel_MapValueKey_IsUint(key) && cel_MapValueKey_GetUint(key) == 1) {
    cel_Value_SetInt(value, -2);
    return true;
  }
  if (cel_MapValueKey_IsString(key) &&
      cel_StringView_Equals(cel_MapValueKey_GetString(key),
                            cel_StringView_FromString("foo"))) {
    cel_Value_SetUint(value, 2);
    return true;
  }
  if (cel_MapValueKey_IsBool(key) && cel_MapValueKey_GetBool(key)) {
    cel_Value_SetNull(value);
    return true;
  }
  if (cel_MapValueKey_IsInt(key) && cel_MapValueKey_GetInt(key) == -1) {
    cel_Value_SetTrue(value);
    return true;
  }

  cel_Error* error = cel_Error_New(context->arena);
  if (error == nullptr) {
    cel_OutOfMemoryStatus(status);
    return false;
  }
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kNotFound);
  cel_Value_SetError(value, error);
  return true;
}

bool cel_TestMapValue_Find(const cel_MapValueVTable* cel_nonnull vtable,
                           cel_ValueContent content,
                           const cel_ValueContext* cel_nonnull context,
                           const cel_MapValueKey* cel_nonnull key,
                           cel_Value* cel_nonnull value,
                           cel_Status* cel_nonnull status) {
  if (cel_MapValueKey_IsUint(key) && cel_MapValueKey_GetUint(key) == 1) {
    cel_Value_SetInt(value, -2);
    return true;
  }
  if (cel_MapValueKey_IsString(key) &&
      cel_StringView_Equals(cel_MapValueKey_GetString(key),
                            cel_StringView_FromString("foo"))) {
    cel_Value_SetUint(value, 2);
    return true;
  }
  if (cel_MapValueKey_IsBool(key) && cel_MapValueKey_GetBool(key)) {
    cel_Value_SetNull(value);
    return true;
  }
  if (cel_MapValueKey_IsInt(key) && cel_MapValueKey_GetInt(key) == -1) {
    cel_Value_SetTrue(value);
    return true;
  }

  return false;
}

bool cel_TestMapValue_Has(const cel_MapValueVTable* cel_nonnull vtable,
                          cel_ValueContent content,
                          const cel_ValueContext* cel_nonnull context,
                          const cel_MapValueKey* cel_nonnull key,
                          cel_Value* cel_nonnull result,
                          cel_Status* cel_nonnull status) {
  if (cel_MapValueKey_IsUint(key) && cel_MapValueKey_GetUint(key) == 1) {
    cel_Value_SetTrue(result);
    return true;
  }
  if (cel_MapValueKey_IsString(key) &&
      cel_StringView_Equals(cel_MapValueKey_GetString(key),
                            cel_StringView_FromString("foo"))) {
    cel_Value_SetTrue(result);
    return true;
  }
  if (cel_MapValueKey_IsBool(key) && cel_MapValueKey_GetBool(key)) {
    cel_Value_SetTrue(result);
    return true;
  }
  if (cel_MapValueKey_IsInt(key) && cel_MapValueKey_GetInt(key) == -1) {
    cel_Value_SetTrue(result);
    return true;
  }

  cel_Value_SetFalse(result);
  return true;
}

cel_MapValueIterator* cel_nullable cel_TestMapValue_NewIterator(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    cel_Status* cel_nonnull status) {
  TestMapValueIterator* iter = new TestMapValueIterator();
  if (vtable->FastSize != nullptr) {
    iter->vtable = &fast_test_map_value_iterator_vtable;
  } else {
    iter->vtable = &slow_test_map_value_iterator_vtable;
  }
  return iter;
}

const cel_MapValueVTable fast_test_map_value_vtable = {
    .Equals = &cel_TestMapValue_Equals,
    .FastSize = &cel_TestMapValue_FastSize,
    .SlowSize = &cel_TestMapValue_SlowSize,
    .Get = &cel_TestMapValue_Get,
    .Find = &cel_TestMapValue_Find,
    .Has = &cel_TestMapValue_Has,
    .NewIterator = &cel_TestMapValue_NewIterator,
};

const cel_MapValueVTable slow_test_map_value_vtable = {
    .Equals = &cel_TestMapValue_Equals,
    .FastSize = cel_nullptr,
    .SlowSize = &cel_TestMapValue_SlowSize,
    .Get = &cel_TestMapValue_Get,
    .Find = &cel_TestMapValue_Find,
    .Has = &cel_TestMapValue_Has,
    .NewIterator = &cel_TestMapValue_NewIterator,
};

using MapValueTest = ValueTest;

TEST_F(MapValueTest, Equals) {
  cel_MapValue map_value;
  cel_MapValue other_map_value;
  cel_Value result;

  map_value.vtable = &fast_test_map_value_vtable;
  other_map_value.vtable = &fast_test_map_value_vtable;

  ASSERT_TRUE(cel_MapValue_Equals(&map_value, ctx(), &other_map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
  ASSERT_TRUE(cel_MapValue_Equals(&other_map_value, ctx(), &map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  map_value.vtable = &fast_test_map_value_vtable;
  other_map_value.vtable = &slow_test_map_value_vtable;

  ASSERT_TRUE(cel_MapValue_Equals(&map_value, ctx(), &other_map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
  ASSERT_TRUE(cel_MapValue_Equals(&other_map_value, ctx(), &map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  map_value.vtable = &slow_test_map_value_vtable;
  other_map_value.vtable = &slow_test_map_value_vtable;

  ASSERT_TRUE(cel_MapValue_Equals(&map_value, ctx(), &other_map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
  ASSERT_TRUE(cel_MapValue_Equals(&other_map_value, ctx(), &map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  map_value.vtable = &fast_test_map_value_vtable;
  _cel_EmptyMapValue_Set(&other_map_value);
  ASSERT_TRUE(cel_MapValue_Equals(&map_value, ctx(), &other_map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
  ASSERT_TRUE(cel_MapValue_Equals(&other_map_value, ctx(), &map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  map_value.vtable = &slow_test_map_value_vtable;
  _cel_EmptyMapValue_Set(&other_map_value);
  ASSERT_TRUE(cel_MapValue_Equals(&map_value, ctx(), &other_map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
  ASSERT_TRUE(cel_MapValue_Equals(&other_map_value, ctx(), &map_value, &result,
                                  status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

}  // namespace
