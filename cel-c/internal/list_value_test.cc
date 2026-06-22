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

#include <cstddef>

#include "gtest/gtest.h"
#include "cel-c/config.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/internal/empty_list_value.h"
#include "cel-c/internal/value_testing.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/value.h"

namespace {

struct TestListValueIterator : cel_ListValueIterator {
  size_t index = 0;
};

void cel_TestListValueIterator_Delete(cel_ValueIterator* cel_nonnull iterator) {
  delete reinterpret_cast<TestListValueIterator*>(iterator);
}

bool cel_TestListValueIteratorNext1(cel_ValueIterator* cel_nonnull iterator,
                                    const cel_ValueContext* cel_nonnull context,
                                    cel_Value* cel_nonnull key_or_value,
                                    cel_Status* cel_nonnull status) {
  TestListValueIterator* iter =
      reinterpret_cast<TestListValueIterator*>(iterator);
  switch (iter->index) {
    case 0:
      cel_Value_SetNull(key_or_value);
      ++iter->index;
      return true;
    case 1:
      cel_Value_SetTrue(key_or_value);
      ++iter->index;
      return true;
    case 2:
      cel_Value_SetDouble(key_or_value, 1.0);
      ++iter->index;
      return true;
    case 3:
      cel_Value_SetString(key_or_value, cel_StringView_FromString("foo"));
      ++iter->index;
      return true;
    default:
      return false;
  }
}

bool cel_TestListValueIterator_Next2(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull key,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status) {
  TestListValueIterator* iter =
      reinterpret_cast<TestListValueIterator*>(iterator);
  switch (iter->index) {
    case 0:
      cel_Value_SetInt(key, 0);
      cel_Value_SetNull(value);
      ++iter->index;
      return true;
    case 1:
      cel_Value_SetInt(key, 1);
      cel_Value_SetTrue(value);
      ++iter->index;
      return true;
    case 2:
      cel_Value_SetInt(key, 2);
      cel_Value_SetDouble(value, 1.0);
      ++iter->index;
      return true;
    case 3:
      cel_Value_SetInt(key, 3);
      cel_Value_SetString(value, cel_StringView_FromString("foo"));
      ++iter->index;
      return true;
    default:
      return false;
  }
}

bool cel_TestListValueIterator_Remaining(const cel_ValueIterator* cel_nonnull
                                             iterator,
                                         size_t* cel_nonnull remaining) {
  const TestListValueIterator* iter =
      reinterpret_cast<const TestListValueIterator*>(iterator);
  *remaining = 4 - iter->index;
  return true;
}

bool cel_TestListValueIterator_Next(cel_ListValueIterator* cel_nonnull iterator,
                                    const cel_ValueContext* cel_nonnull context,
                                    size_t* cel_nullable index,
                                    cel_Value* cel_nonnull value,
                                    cel_Status* cel_nonnull status) {
  TestListValueIterator* iter =
      reinterpret_cast<TestListValueIterator*>(iterator);
  switch (iter->index) {
    case 0:
      if (index != nullptr) {
        *index = 0;
      }
      cel_Value_SetNull(value);
      ++iter->index;
      return true;
    case 1:
      if (index != nullptr) {
        *index = 1;
      }
      cel_Value_SetTrue(value);
      ++iter->index;
      return true;
    case 2:
      if (index != nullptr) {
        *index = 2;
      }
      cel_Value_SetDouble(value, 1.0);
      ++iter->index;
      return true;
    case 3:
      if (index != nullptr) {
        *index = 3;
      }
      cel_Value_SetString(value, cel_StringView_FromString("foo"));
      ++iter->index;
      return true;
    default:
      return false;
  }
}

const cel_ListValueIteratorVTable fast_test_list_value_iterator_vtable = {
    .super =
        {
            .Delete = &cel_TestListValueIterator_Delete,
            .Next1 = &cel_TestListValueIteratorNext1,
            .Next2 = &cel_TestListValueIterator_Next2,
            .Remaining = &cel_TestListValueIterator_Remaining,
        },
    .Next = &cel_TestListValueIterator_Next,
};

const cel_ListValueIteratorVTable slow_test_list_value_iterator_vtable = {
    .super =
        {
            .Delete = &cel_TestListValueIterator_Delete,
            .Next1 = &cel_TestListValueIteratorNext1,
            .Next2 = &cel_TestListValueIterator_Next2,
            .Remaining = cel_nullptr,
        },
    .Next = &cel_TestListValueIterator_Next,
};

bool cel_TestListValue_Equals(const cel_ListValueVTable* cel_nonnull vtable,
                              cel_ValueContent content,
                              const cel_ValueContext* cel_nonnull context,
                              const cel_ListValue* cel_nonnull other,
                              cel_Value* cel_nonnull result,
                              cel_Status* cel_nonnull status) {
  return false;
}

bool cel_TestListValue_FastSize(const cel_ListValueVTable* cel_nonnull vtable,
                                cel_ValueContent content,
                                size_t* cel_nonnull size) {
  *size = 4;
  return true;
}

bool cel_TestListValue_SlowSize(const cel_ListValueVTable* cel_nonnull vtable,
                                cel_ValueContent content,
                                const cel_ValueContext* cel_nonnull context,
                                cel_Value* cel_nonnull size,
                                cel_Status* cel_nonnull status) {
  cel_Value_SetInt(size, 4);
  return true;
}

bool cel_TestListValue_Get(const cel_ListValueVTable* cel_nonnull vtable,
                           cel_ValueContent content,
                           const cel_ValueContext* cel_nonnull context,
                           size_t index, cel_Value* cel_nonnull element,
                           cel_Status* cel_nonnull status) {
  switch (index) {
    case 0:
      cel_Value_SetNull(element);
      return true;
    case 1:
      cel_Value_SetTrue(element);
      return true;
    case 2:
      cel_Value_SetDouble(element, 1.0);
      return true;
    case 3:
      cel_Value_SetString(element, cel_StringView_FromString("foo"));
      return true;
    default: {
      cel_Error* error = cel_Error_New(context->arena);
      if (error == nullptr) {
        cel_OutOfMemoryStatus(status);
        return false;
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kOutOfRange);
      cel_Value_SetError(element, error);
      return true;
    }
  }
}

cel_ListValueIterator* cel_nullable cel_TestListValue_NewIterator(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    cel_Status* cel_nonnull status) {
  TestListValueIterator* iter = new TestListValueIterator();
  if (vtable->FastSize != nullptr) {
    iter->vtable = &fast_test_list_value_iterator_vtable;
  } else {
    iter->vtable = &slow_test_list_value_iterator_vtable;
  }
  return iter;
}

const cel_ListValueVTable fast_test_list_value_vtable = {
    .Equals = &cel_TestListValue_Equals,
    .FastSize = &cel_TestListValue_FastSize,
    .SlowSize = &cel_TestListValue_SlowSize,
    .Get = &cel_TestListValue_Get,
    .NewIterator = &cel_TestListValue_NewIterator,
};

const cel_ListValueVTable slow_test_list_value_vtable = {
    .Equals = &cel_TestListValue_Equals,
    .FastSize = cel_nullptr,
    .SlowSize = &cel_TestListValue_SlowSize,
    .Get = &cel_TestListValue_Get,
    .NewIterator = &cel_TestListValue_NewIterator,
};

using ListValueTest = ValueTest;

TEST_F(ListValueTest, Equals) {
  cel_ListValue list_value;
  cel_ListValue other_list_value;
  cel_Value result;

  list_value.vtable = &fast_test_list_value_vtable;
  other_list_value.vtable = &fast_test_list_value_vtable;

  ASSERT_TRUE(cel_ListValue_Equals(&list_value, ctx(), &other_list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
  ASSERT_TRUE(cel_ListValue_Equals(&other_list_value, ctx(), &list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  list_value.vtable = &fast_test_list_value_vtable;
  other_list_value.vtable = &slow_test_list_value_vtable;

  ASSERT_TRUE(cel_ListValue_Equals(&list_value, ctx(), &other_list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
  ASSERT_TRUE(cel_ListValue_Equals(&other_list_value, ctx(), &list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  list_value.vtable = &slow_test_list_value_vtable;
  other_list_value.vtable = &slow_test_list_value_vtable;

  ASSERT_TRUE(cel_ListValue_Equals(&list_value, ctx(), &other_list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
  ASSERT_TRUE(cel_ListValue_Equals(&other_list_value, ctx(), &list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  list_value.vtable = &fast_test_list_value_vtable;
  _cel_EmptyListValue_Set(&other_list_value);
  ASSERT_TRUE(cel_ListValue_Equals(&list_value, ctx(), &other_list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
  ASSERT_TRUE(cel_ListValue_Equals(&other_list_value, ctx(), &list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));

  list_value.vtable = &slow_test_list_value_vtable;
  _cel_EmptyListValue_Set(&other_list_value);
  ASSERT_TRUE(cel_ListValue_Equals(&list_value, ctx(), &other_list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
  ASSERT_TRUE(cel_ListValue_Equals(&other_list_value, ctx(), &list_value,
                                   &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

}  // namespace
