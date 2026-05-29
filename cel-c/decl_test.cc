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

#include "cel-c/decl.h"

#include <initializer_list>
#include <memory>

#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "absl/strings/string_view.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/function_scope.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"
#include "cel-c/type.h"

namespace {

class DeclTest : public ::testing::Test {
 public:
  void SetUp() override { arena_ = ABSL_DIE_IF_NULL(cel_Arena_New(alloc())); }

  void TearDown() override {
    cel_Arena_Delete(arena_);
    arena_ = nullptr;
  }

  const cel_FunctionType* NewFunctionType(
      const cel_Type* result, std::initializer_list<const cel_Type*> args) {
    const cel_Type** args_ptr;
    cel_FunctionType* type = ABSL_DIE_IF_NULL(
        cel_FunctionType_New(result, args.size(), &args_ptr, arena()));
    std::uninitialized_copy_n(args.begin(), args.size(), args_ptr);
    return type;
  }

  const cel_ListType* NewListType(const cel_Type* elem) {
    return ABSL_DIE_IF_NULL(cel_ListType_New(elem, arena()));
  }

  const cel_MapType* NewMapType(const cel_Type* key, const cel_Type* val) {
    return ABSL_DIE_IF_NULL(cel_MapType_New(key, val, arena()));
  }

  const cel_OpaqueType* NewOpaqueType(
      absl::string_view name, std::initializer_list<const cel_Type*> params) {
    const cel_Type** params_ptr;
    cel_OpaqueType* type = ABSL_DIE_IF_NULL(cel_OpaqueType_New(
        cel_StringView_FromAbsl(name), params.size(), &params_ptr, arena()));
    std::uninitialized_copy_n(params.begin(), params.size(), params_ptr);
    return type;
  }

 protected:
  cel_Allocator* alloc() { return cel_DefaultAllocator; }

  cel_Arena* arena() { return ABSL_DIE_IF_NULL(arena_); }

 private:
  cel_Arena* arena_ = nullptr;
};

TEST_F(DeclTest, FunctionOverloadCollision_BoolWrapper_Null) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_BoolWrapperType}), arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_NullType}), arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_BoolWrapper_Bool) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_BoolWrapperType}), arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_BoolType}), arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_IntWrapper_Null) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_IntWrapperType}), arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_NullType}), arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_IntWrapper_Int) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_IntWrapperType}), arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_IntType}), arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_UintWrapper_Null) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_UintWrapperType}), arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_NullType}), arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_UintWrapper_Uint) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_UintWrapperType}), arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_UintType}), arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_DoubleWrapper_Null) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_DoubleWrapperType}), arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_NullType}), arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_DoubleWrapper_Double) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_DoubleWrapperType}), arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_DoubleType}), arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_BytesWrapper_Null) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_BytesWrapperType}), arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_NullType}), arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_BytesWrapper_Bytes) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_BytesWrapperType}), arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_BytesType}), arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_StringWrapper_Null) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_StringWrapperType}), arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_NullType}), arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_StringWrapper_String) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_StringWrapperType}), arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_StringType}), arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_Bool_Bool) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_BoolType}), arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_BoolType}), arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_List_List) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType,
                          {cel_Type_UpCast(NewListType(cel_BoolType))}),
          arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType,
                          {cel_Type_UpCast(NewListType(cel_DynType))}),
          arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_Map_Map) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_Type_UpCast(NewMapType(
                                           cel_IntType, cel_BoolType))}),
          arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_Type_UpCast(NewMapType(
                                           cel_DynType, cel_DynType))}),
          arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

TEST_F(DeclTest, FunctionOverloadCollision_Opaque_Opaque) {
  cel_FunctionDecl* function = ABSL_DIE_IF_NULL(
      cel_FunctionDecl_New(cel_StringView_From("foo"), arena()));
  cel_FunctionOverloadDecl* overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo0"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_Type_UpCast(NewOpaqueType(
                                           "optional_type", {cel_BoolType}))}),
          arena()));
  ASSERT_TRUE(cel_FunctionDecl_AddOverload(function, overload));
  cel_FunctionOverloadDecl* other_overload =
      ABSL_DIE_IF_NULL(cel_FunctionOverloadDecl_New(
          cel_StringView_From("foo1"), cel_FunctionScope_kGlobal,
          NewFunctionType(cel_DynType, {cel_Type_UpCast(NewOpaqueType(
                                           "optional_type", {cel_DynType}))}),
          arena()));
  ASSERT_FALSE(cel_FunctionDecl_AddOverload(function, other_overload));
}

}  // namespace
