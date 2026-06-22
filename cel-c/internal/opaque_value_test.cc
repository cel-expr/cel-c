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

#include "gtest/gtest.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/value_testing.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/value.h"

namespace {

using OpaqueValueTest = ValueTest;

bool _cel_TestOpaqueValue_Equals(
    const cel_OpaqueValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_OpaqueValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  return false;
}

cel_StringView _cel_TestOpaqueValue_TypeName(
    const cel_OpaqueValueVTable* cel_nonnull vtable, cel_ValueContent content) {
  return cel_StringView_FromString("optional_type");
}

TEST_F(OpaqueValueTest, Equals) {
  cel_OpaqueValueVTable opaque_value_vtable = {
      .Equals = &_cel_TestOpaqueValue_Equals,
      .TypeName = &_cel_TestOpaqueValue_TypeName,
  };

  cel_OpaqueValueVTable other_opaque_value_vtable = opaque_value_vtable;
  other_opaque_value_vtable.Equals = nullptr;

  cel_OpaqueValue opaque_value;
  opaque_value.vtable = &opaque_value_vtable;

  cel_OpaqueValue other_opaque_value;
  other_opaque_value.vtable = &other_opaque_value_vtable;

  cel_Value result;

  ASSERT_TRUE(cel_OpaqueValue_Equals(&opaque_value, ctx(), &other_opaque_value,
                                     &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
  ASSERT_TRUE(cel_OpaqueValue_Equals(&other_opaque_value, ctx(), &opaque_value,
                                     &result, status()));
  EXPECT_TRUE(cel_Value_IsFalse(&result));
}

}  // namespace
