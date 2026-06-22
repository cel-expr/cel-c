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
#include <cstdlib>

#include "gtest/gtest.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/value_testing.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/value.h"

namespace {

struct TestStructValueIterator : cel_StructValueIterator {
  size_t index = 0;
};

void cel_TestStructValueIterator_Delete(
    cel_StructValueIterator* cel_nonnull iterator) {
  delete static_cast<TestStructValueIterator*>(iterator);
}

bool cel_TestStructValueIterator_Next(
    cel_StructValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_StructValueKey* cel_nonnull key, cel_Value* cel_nullable value,
    cel_Status* cel_nonnull status) {
  TestStructValueIterator* iter =
      static_cast<TestStructValueIterator*>(iterator);
  switch (iter->index) {
    case 0:
      cel_StructValueKey_SetName(key, cel_StringView_FromString("b"));
      if (value != nullptr) {
        cel_Value_SetInt(value, 1);
      }
      ++iter->index;
      return true;
    case 1:
      cel_StructValueKey_SetName(key, cel_StringView_FromString("d"));
      if (value != nullptr) {
        cel_Value_SetString(value, cel_StringView_FromString("foo"));
      }
      ++iter->index;
      return true;
    case 2:
      cel_StructValueKey_SetName(key, cel_StringView_FromString("a"));
      if (value != nullptr) {
        cel_Value_SetNull(value);
      }
      ++iter->index;
      return true;
    case 3:
      cel_StructValueKey_SetName(key, cel_StringView_FromString("c"));
      if (value != nullptr) {
        cel_Value_SetUint(value, 2);
      }
      ++iter->index;
      return true;
    default:
      return false;
  }
}

bool cel_TestStructValueIterator_Remaining(
    const cel_StructValueIterator* cel_nonnull iterator,
    size_t* cel_nonnull remaining) {
  const TestStructValueIterator* iter =
      static_cast<const TestStructValueIterator*>(iterator);
  *remaining = 4 - iter->index;
  return true;
}

const cel_StructValueIteratorVTable fast_test_struct_value_iterator_vtable = {
    .Delete = &cel_TestStructValueIterator_Delete,
    .Next = &cel_TestStructValueIterator_Next,
    .Remaining = &cel_TestStructValueIterator_Remaining,
};

const cel_StructValueIteratorVTable slow_test_struct_value_iterator_vtable = {
    .Delete = &cel_TestStructValueIterator_Delete,
    .Next = &cel_TestStructValueIterator_Next,
    .Remaining = cel_nullptr,
};

bool cel_TestStructValue_Equals(const cel_StructValueVTable* cel_nonnull vtable,
                                cel_ValueContent content,
                                const cel_ValueContext* cel_nonnull context,
                                const cel_StructValue* cel_nonnull other,
                                cel_Value* cel_nonnull result,
                                cel_Status* cel_nonnull status) {
  return false;
}

cel_StringView cel_TestStructValue_TypeName(
    const cel_StructValueVTable* cel_nonnull vtable, cel_ValueContent content) {
  return cel_StringView_FromString("test.Struct");
}

bool cel_TestStructValue_Get(const cel_StructValueVTable* cel_nonnull vtable,
                             cel_ValueContent content,
                             const cel_ValueContext* cel_nonnull context,
                             const cel_StructValueKey* cel_nonnull key,
                             cel_Value* cel_nonnull value,
                             cel_Status* cel_nonnull status) {
  if (cel_StringView_Equals(cel_StructValueKey_GetName(key),
                            cel_StringView_FromString("a"))) {
    cel_Value_SetNull(value);
    return true;
  }
  if (cel_StringView_Equals(cel_StructValueKey_GetName(key),
                            cel_StringView_FromString("b"))) {
    cel_Value_SetInt(value, 1);
    return true;
  }
  if (cel_StringView_Equals(cel_StructValueKey_GetName(key),
                            cel_StringView_FromString("c"))) {
    cel_Value_SetUint(value, 2);
    return true;
  }
  if (cel_StringView_Equals(cel_StructValueKey_GetName(key),
                            cel_StringView_FromString("d"))) {
    cel_Value_SetString(value, cel_StringView_FromString("foo"));
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

bool cel_TestStructValue_Has(const cel_StructValueVTable* cel_nonnull vtable,
                             cel_ValueContent content,
                             const cel_ValueContext* cel_nonnull context,
                             const cel_StructValueKey* cel_nonnull key,
                             cel_Value* cel_nonnull result,
                             cel_Status* cel_nonnull status) {
  if (cel_StringView_Equals(cel_StructValueKey_GetName(key),
                            cel_StringView_FromString("a"))) {
    cel_Value_SetTrue(result);
    return true;
  }
  if (cel_StringView_Equals(cel_StructValueKey_GetName(key),
                            cel_StringView_FromString("b"))) {
    cel_Value_SetTrue(result);
    return true;
  }
  if (cel_StringView_Equals(cel_StructValueKey_GetName(key),
                            cel_StringView_FromString("c"))) {
    cel_Value_SetTrue(result);
    return true;
  }
  if (cel_StringView_Equals(cel_StructValueKey_GetName(key),
                            cel_StringView_FromString("d"))) {
    cel_Value_SetTrue(result);
    return true;
  }

  cel_Value_SetFalse(result);
  return true;
}

cel_StructValueIterator* cel_nullable cel_TestStructValue_NewIterator(
    const cel_StructValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    cel_Status* cel_nonnull status);

const cel_StructValueVTable fast_test_struct_value_vtable = {
    .Equals = &cel_TestStructValue_Equals,
    .TypeName = &cel_TestStructValue_TypeName,
    .Get = &cel_TestStructValue_Get,
    .Has = &cel_TestStructValue_Has,
    .NewIterator = &cel_TestStructValue_NewIterator,
};

const cel_StructValueVTable slow_test_struct_value_vtable = {
    .Equals = &cel_TestStructValue_Equals,
    .TypeName = &cel_TestStructValue_TypeName,
    .Get = &cel_TestStructValue_Get,
    .Has = &cel_TestStructValue_Has,
    .NewIterator = &cel_TestStructValue_NewIterator,
};

cel_StructValueIterator* cel_nullable cel_TestStructValue_NewIterator(
    const cel_StructValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    cel_Status* cel_nonnull status) {
  TestStructValueIterator* iter = new TestStructValueIterator();
  if (vtable == &fast_test_struct_value_vtable) {
    iter->vtable = &fast_test_struct_value_iterator_vtable;
  } else {
    iter->vtable = &slow_test_struct_value_iterator_vtable;
  }
  return iter;
}

using StructValueTest = ValueTest;

TEST_F(StructValueTest, Equals) {
  cel_StructValue struct_value;
  cel_StructValue other_struct_value;
  cel_Value result;

  struct_value.vtable = &fast_test_struct_value_vtable;
  other_struct_value.vtable = &fast_test_struct_value_vtable;

  ASSERT_TRUE(cel_StructValue_Equals(&struct_value, ctx(), &other_struct_value,
                                     &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
  ASSERT_TRUE(cel_StructValue_Equals(&other_struct_value, ctx(), &struct_value,
                                     &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  struct_value.vtable = &fast_test_struct_value_vtable;
  other_struct_value.vtable = &slow_test_struct_value_vtable;

  ASSERT_TRUE(cel_StructValue_Equals(&struct_value, ctx(), &other_struct_value,
                                     &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
  ASSERT_TRUE(cel_StructValue_Equals(&other_struct_value, ctx(), &struct_value,
                                     &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  struct_value.vtable = &slow_test_struct_value_vtable;
  other_struct_value.vtable = &slow_test_struct_value_vtable;

  ASSERT_TRUE(cel_StructValue_Equals(&struct_value, ctx(), &other_struct_value,
                                     &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
  ASSERT_TRUE(cel_StructValue_Equals(&other_struct_value, ctx(), &struct_value,
                                     &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
}

}  // namespace
