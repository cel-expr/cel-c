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

#include "cel-c/type.h"

#include <cstddef>

#include "google/protobuf/any.upbdefs.h"
#include "google/protobuf/api.upbdefs.h"
#include "google/protobuf/duration.upbdefs.h"
#include "google/protobuf/struct.upbdefs.h"
#include "google/protobuf/timestamp.upbdefs.h"
#include "google/protobuf/type.upbdefs.h"
#include "google/protobuf/wrappers.upbdefs.h"
#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/config.h"
#include "cel-c/string_view.h"
#include "cel-c/type_kind.h"
#include "upb/reflection/def.h"

namespace {

class TypeTest : public ::testing::Test {
 public:
  void SetUp() override {
    arena_ = ABSL_DIE_IF_NULL(cel_Arena_New(alloc()));
    def_pool_ = ABSL_DIE_IF_NULL(upb_DefPool_New());
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Api_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_BoolValue_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Int32Value_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Int64Value_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_UInt32Value_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_UInt64Value_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_FloatValue_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_DoubleValue_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_StringValue_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_BytesValue_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Any_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Duration_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Timestamp_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Value_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_ListValue_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Struct_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Enum_getmsgdef(def_pool_)));
  }

  void TearDown() override {
    upb_DefPool_Free(def_pool_);
    def_pool_ = nullptr;
    cel_Arena_Delete(arena_);
    arena_ = nullptr;
  }

 protected:
  CEL_NONNULL(cel_Allocator*) alloc() { return cel_DefaultAllocator; }

  CEL_NONNULL(cel_Arena*) arena() { return ABSL_DIE_IF_NULL(arena_); }

  CEL_NONNULL(upb_DefPool*) def_pool() { return ABSL_DIE_IF_NULL(def_pool_); }

 private:
  CEL_NULLABLE(cel_Arena*) arena_ = nullptr;
  CEL_NULLABLE(upb_DefPool*) def_pool_ = nullptr;
};

TEST_F(TypeTest, IsWellKnownMessageType) {
  EXPECT_TRUE(cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(
      upb_DefPool_FindMessageByName(def_pool(), "google.protobuf.BoolValue"))));
  EXPECT_TRUE(
      cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(upb_DefPool_FindMessageByName(
          def_pool(), "google.protobuf.Int32Value"))));
  EXPECT_TRUE(
      cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(upb_DefPool_FindMessageByName(
          def_pool(), "google.protobuf.Int64Value"))));
  EXPECT_TRUE(
      cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(upb_DefPool_FindMessageByName(
          def_pool(), "google.protobuf.UInt32Value"))));
  EXPECT_TRUE(
      cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(upb_DefPool_FindMessageByName(
          def_pool(), "google.protobuf.UInt64Value"))));
  EXPECT_TRUE(
      cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(upb_DefPool_FindMessageByName(
          def_pool(), "google.protobuf.FloatValue"))));
  EXPECT_TRUE(
      cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(upb_DefPool_FindMessageByName(
          def_pool(), "google.protobuf.DoubleValue"))));
  EXPECT_TRUE(
      cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(upb_DefPool_FindMessageByName(
          def_pool(), "google.protobuf.BytesValue"))));
  EXPECT_TRUE(
      cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(upb_DefPool_FindMessageByName(
          def_pool(), "google.protobuf.StringValue"))));
  EXPECT_TRUE(cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(
      upb_DefPool_FindMessageByName(def_pool(), "google.protobuf.Any"))));
  EXPECT_TRUE(cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(
      upb_DefPool_FindMessageByName(def_pool(), "google.protobuf.Timestamp"))));
  EXPECT_TRUE(cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(
      upb_DefPool_FindMessageByName(def_pool(), "google.protobuf.Duration"))));
  EXPECT_TRUE(cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(
      upb_DefPool_FindMessageByName(def_pool(), "google.protobuf.Value"))));
  EXPECT_TRUE(cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(
      upb_DefPool_FindMessageByName(def_pool(), "google.protobuf.ListValue"))));
  EXPECT_TRUE(cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(
      upb_DefPool_FindMessageByName(def_pool(), "google.protobuf.Struct"))));
  EXPECT_FALSE(cel_IsWellKnownMessageType(ABSL_DIE_IF_NULL(
      upb_DefPool_FindMessageByName(def_pool(), "google.protobuf.Api"))));
}

TEST_F(TypeTest, IsWellKnownMessageTypeName) {
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.BoolValue")));
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.Int32Value")));
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.Int64Value")));
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.UInt32Value")));
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.UInt64Value")));
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.FloatValue")));
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.DoubleValue")));
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.BytesValue")));
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.StringValue")));
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.Any")));
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.Timestamp")));
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.Duration")));
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.Value")));
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.ListValue")));
  EXPECT_TRUE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.Struct")));
  EXPECT_FALSE(cel_IsWellKnownMessageTypeName(
      cel_StringView_From("google.protobuf.Api")));
}

TEST_F(TypeTest, IsWellKnownEnumType) {
  EXPECT_TRUE(cel_IsWellKnownEnumType(ABSL_DIE_IF_NULL(
      upb_DefPool_FindEnumByName(def_pool(), "google.protobuf.NullValue"))));
  EXPECT_FALSE(cel_IsWellKnownEnumType(ABSL_DIE_IF_NULL(
      upb_DefPool_FindEnumByName(def_pool(), "google.protobuf.Syntax"))));
}

TEST_F(TypeTest, IsWellKnownEnumTypeName) {
  EXPECT_TRUE(cel_IsWellKnownEnumTypeName(
      cel_StringView_From("google.protobuf.NullValue")));
  EXPECT_FALSE(cel_IsWellKnownEnumTypeName(
      cel_StringView_From("google.protobuf.Syntax")));
}

TEST_F(TypeTest, Dyn) {
  EXPECT_EQ(cel_Type_Kind(cel_DynType), cel_TypeKind_kDyn);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_DynType),
                                    cel_StringView_From("dyn")));
  EXPECT_TRUE(cel_Type_IsDyn(cel_DynType));
  EXPECT_TRUE(cel_Type_Equals(cel_DynType, cel_DynType));
  EXPECT_TRUE(cel_Type_Equals(cel_DynType, cel_DynType));
  EXPECT_FALSE(cel_Type_Equals(cel_DynType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_DynType));
}

TEST_F(TypeTest, Null) {
  EXPECT_EQ(cel_Type_Kind(cel_NullType), cel_TypeKind_kNull);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_NullType),
                                    cel_StringView_From("null_type")));
  EXPECT_TRUE(cel_Type_IsNull(cel_NullType));
  EXPECT_TRUE(cel_Type_Equals(cel_NullType, cel_NullType));
  EXPECT_TRUE(cel_Type_Equals(cel_NullType, cel_NullType));
  EXPECT_FALSE(cel_Type_Equals(cel_NullType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_NullType));
}

TEST_F(TypeTest, Bool) {
  EXPECT_EQ(cel_Type_Kind(cel_BoolType), cel_TypeKind_kBool);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_BoolType),
                                    cel_StringView_From("bool")));
  EXPECT_TRUE(cel_Type_IsBool(cel_BoolType));
  EXPECT_TRUE(cel_Type_Equals(cel_BoolType, cel_BoolType));
  EXPECT_TRUE(cel_Type_Equals(cel_BoolType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_DynType));
  EXPECT_FALSE(cel_Type_Equals(cel_DynType, cel_BoolType));
}

TEST_F(TypeTest, Int) {
  EXPECT_EQ(cel_Type_Kind(cel_IntType), cel_TypeKind_kInt);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_IntType),
                                    cel_StringView_From("int")));
  EXPECT_TRUE(cel_Type_IsInt(cel_IntType));
  EXPECT_TRUE(cel_Type_Equals(cel_IntType, cel_IntType));
  EXPECT_TRUE(cel_Type_Equals(cel_IntType, cel_IntType));
  EXPECT_FALSE(cel_Type_Equals(cel_IntType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_IntType));
}

TEST_F(TypeTest, Uint) {
  EXPECT_EQ(cel_Type_Kind(cel_UintType), cel_TypeKind_kUint);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_UintType),
                                    cel_StringView_From("uint")));
  EXPECT_TRUE(cel_Type_IsUint(cel_UintType));
  EXPECT_TRUE(cel_Type_Equals(cel_UintType, cel_UintType));
  EXPECT_TRUE(cel_Type_Equals(cel_UintType, cel_UintType));
  EXPECT_FALSE(cel_Type_Equals(cel_UintType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_UintType));
}

TEST_F(TypeTest, Double) {
  EXPECT_EQ(cel_Type_Kind(cel_DoubleType), cel_TypeKind_kDouble);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_DoubleType),
                                    cel_StringView_From("double")));
  EXPECT_TRUE(cel_Type_IsDouble(cel_DoubleType));
  EXPECT_TRUE(cel_Type_Equals(cel_DoubleType, cel_DoubleType));
  EXPECT_TRUE(cel_Type_Equals(cel_DoubleType, cel_DoubleType));
  EXPECT_FALSE(cel_Type_Equals(cel_DoubleType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_DoubleType));
}

TEST_F(TypeTest, String) {
  EXPECT_EQ(cel_Type_Kind(cel_StringType), cel_TypeKind_kString);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_StringType),
                                    cel_StringView_From("string")));
  EXPECT_TRUE(cel_Type_IsString(cel_StringType));
  EXPECT_TRUE(cel_Type_Equals(cel_StringType, cel_StringType));
  EXPECT_TRUE(cel_Type_Equals(cel_StringType, cel_StringType));
  EXPECT_FALSE(cel_Type_Equals(cel_StringType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_StringType));
}

TEST_F(TypeTest, Bytes) {
  EXPECT_EQ(cel_Type_Kind(cel_BytesType), cel_TypeKind_kBytes);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_BytesType),
                                    cel_StringView_From("bytes")));
  EXPECT_TRUE(cel_Type_IsBytes(cel_BytesType));
  EXPECT_TRUE(cel_Type_Equals(cel_BytesType, cel_BytesType));
  EXPECT_TRUE(cel_Type_Equals(cel_BytesType, cel_BytesType));
  EXPECT_FALSE(cel_Type_Equals(cel_BytesType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_BytesType));
}

TEST_F(TypeTest, Struct) {
  const cel_StructType* type = ABSL_DIE_IF_NULL(cel_StructType_New(
      cel_StringView_FromString("google.protobuf.Api"), arena()));
  EXPECT_EQ(cel_Type_Kind(cel_Type_UpCast(type)), cel_TypeKind_kStruct);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_Type_Name(cel_Type_UpCast(type)),
                            cel_StringView_From("google.protobuf.Api")));
  EXPECT_TRUE(cel_StringView_Equals(
      cel_StructType_Name(type), cel_StringView_From("google.protobuf.Api")));
  EXPECT_TRUE(cel_Type_IsStruct(cel_Type_UpCast(type)));
  const cel_StructType* same_type = ABSL_DIE_IF_NULL(cel_StructType_New(
      cel_StringView_FromString("google.protobuf.Api"), arena()));
  const cel_StructType* diff_type = ABSL_DIE_IF_NULL(cel_StructType_New(
      cel_StringView_FromString("google.protobuf.Enum"), arena()));
  EXPECT_TRUE(cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(same_type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(same_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(diff_type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(diff_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(cel_Type_Equals(cel_Type_UpCast(type), cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_Type_UpCast(type)));
}

TEST_F(TypeTest, Duration) {
  EXPECT_EQ(cel_Type_Kind(cel_DurationType), cel_TypeKind_kDuration);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_Type_Name(cel_DurationType),
                            cel_StringView_From("google.protobuf.Duration")));
  EXPECT_TRUE(cel_Type_IsDuration(cel_DurationType));
  EXPECT_TRUE(cel_Type_Equals(cel_DurationType, cel_DurationType));
  EXPECT_TRUE(cel_Type_Equals(cel_DurationType, cel_DurationType));
  EXPECT_FALSE(cel_Type_Equals(cel_DurationType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_DurationType));
}

TEST_F(TypeTest, Timestamp) {
  EXPECT_EQ(cel_Type_Kind(cel_TimestampType), cel_TypeKind_kTimestamp);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_Type_Name(cel_TimestampType),
                            cel_StringView_From("google.protobuf.Timestamp")));
  EXPECT_TRUE(cel_Type_IsTimestamp(cel_TimestampType));
  EXPECT_TRUE(cel_Type_Equals(cel_TimestampType, cel_TimestampType));
  EXPECT_TRUE(cel_Type_Equals(cel_TimestampType, cel_TimestampType));
  EXPECT_FALSE(cel_Type_Equals(cel_TimestampType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_TimestampType));
}

TEST_F(TypeTest, List) {
  const cel_ListType* type =
      ABSL_DIE_IF_NULL(cel_ListType_New(cel_DynType, arena()));
  EXPECT_EQ(cel_Type_Kind(cel_Type_UpCast(type)), cel_TypeKind_kList);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_Type_UpCast(type)),
                                    cel_StringView_From("list")));
  EXPECT_TRUE(cel_Type_IsDyn(cel_ListType_Element(type)));
  EXPECT_TRUE(cel_Type_IsList(cel_Type_UpCast(type)));
  const cel_ListType* same_type =
      ABSL_DIE_IF_NULL(cel_ListType_New(cel_DynType, arena()));
  const cel_ListType* diff_type =
      ABSL_DIE_IF_NULL(cel_ListType_New(cel_BoolType, arena()));
  EXPECT_TRUE(cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(same_type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(same_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(diff_type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(diff_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(cel_Type_Equals(cel_Type_UpCast(type), cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_Type_UpCast(type)));
}

TEST_F(TypeTest, Map) {
  const cel_MapType* type =
      ABSL_DIE_IF_NULL(cel_MapType_New(cel_IntType, cel_DynType, arena()));
  EXPECT_EQ(cel_Type_Kind(cel_Type_UpCast(type)), cel_TypeKind_kMap);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_Type_UpCast(type)),
                                    cel_StringView_From("map")));
  EXPECT_TRUE(cel_Type_IsInt(cel_MapType_Key(type)));
  EXPECT_TRUE(cel_Type_IsDyn(cel_MapType_Value(type)));
  EXPECT_TRUE(cel_Type_IsMap(cel_Type_UpCast(type)));
  const cel_MapType* same_type =
      ABSL_DIE_IF_NULL(cel_MapType_New(cel_IntType, cel_DynType, arena()));
  const cel_MapType* diff_type =
      ABSL_DIE_IF_NULL(cel_MapType_New(cel_StringType, cel_DynType, arena()));
  EXPECT_TRUE(cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(same_type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(same_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(diff_type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(diff_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(cel_Type_Equals(cel_Type_UpCast(type), cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_Type_UpCast(type)));
}

TEST_F(TypeTest, Unknown) {
  EXPECT_EQ(cel_Type_Kind(cel_UnknownType), cel_TypeKind_kUnknown);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_UnknownType),
                                    cel_StringView_From("**unknown**")));
  EXPECT_TRUE(cel_Type_IsUnknown(cel_UnknownType));
  EXPECT_TRUE(cel_Type_Equals(cel_UnknownType, cel_UnknownType));
  EXPECT_TRUE(cel_Type_Equals(cel_UnknownType, cel_UnknownType));
  EXPECT_FALSE(cel_Type_Equals(cel_UnknownType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_UnknownType));
}

TEST_F(TypeTest, Type) {
  const cel_TypeType* type =
      ABSL_DIE_IF_NULL(cel_TypeType_New(cel_DynType, arena()));
  EXPECT_EQ(cel_Type_Kind(cel_Type_UpCast(type)), cel_TypeKind_kType);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_Type_UpCast(type)),
                                    cel_StringView_From("type")));
  EXPECT_TRUE(cel_Type_IsDyn(cel_TypeType_Type(type)));
  EXPECT_TRUE(cel_Type_IsType(cel_Type_UpCast(type)));
  const cel_TypeType* same_type =
      ABSL_DIE_IF_NULL(cel_TypeType_New(cel_DynType, arena()));
  const cel_TypeType* diff_type =
      ABSL_DIE_IF_NULL(cel_TypeType_New(cel_BoolType, arena()));
  EXPECT_TRUE(cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(same_type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(same_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(diff_type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(diff_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(cel_Type_Equals(cel_Type_UpCast(type), cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_Type_UpCast(type)));
}

TEST_F(TypeTest, Error) {
  EXPECT_EQ(cel_Type_Kind(cel_ErrorType), cel_TypeKind_kError);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_ErrorType),
                                    cel_StringView_From("**error**")));
  EXPECT_TRUE(cel_Type_IsError(cel_ErrorType));
  EXPECT_TRUE(cel_Type_Equals(cel_ErrorType, cel_ErrorType));
  EXPECT_TRUE(cel_Type_Equals(cel_ErrorType, cel_ErrorType));
  EXPECT_FALSE(cel_Type_Equals(cel_ErrorType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_ErrorType));
}

TEST_F(TypeTest, Any) {
  EXPECT_EQ(cel_Type_Kind(cel_AnyType), cel_TypeKind_kAny);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_Type_Name(cel_AnyType), cel_StringView_From("google.protobuf.Any")));
  EXPECT_TRUE(cel_Type_IsAny(cel_AnyType));
  EXPECT_TRUE(cel_Type_Equals(cel_AnyType, cel_AnyType));
  EXPECT_TRUE(cel_Type_Equals(cel_AnyType, cel_AnyType));
  EXPECT_FALSE(cel_Type_Equals(cel_AnyType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_AnyType));
}

TEST_F(TypeTest, Opaque) {
  const cel_Type** params;
  const cel_OpaqueType* type = ABSL_DIE_IF_NULL(cel_OpaqueType_New(
      cel_StringView_FromString("opaque_type"), 2, &params, arena()));
  params[0] = cel_IntType;
  params[1] = cel_DynType;
  EXPECT_EQ(cel_Type_Kind(cel_Type_UpCast(type)), cel_TypeKind_kOpaque);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_Type_UpCast(type)),
                                    cel_StringView_From("opaque_type")));
  size_t params_len;
  EXPECT_TRUE(cel_Type_IsInt(cel_OpaqueType_Params(type, &params_len)[0]));
  EXPECT_EQ(params_len, 2);
  EXPECT_TRUE(cel_Type_IsDyn(cel_OpaqueType_Params(type, &params_len)[1]));
  EXPECT_EQ(params_len, 2);
  EXPECT_TRUE(cel_Type_IsOpaque(cel_Type_UpCast(type)));
  const cel_OpaqueType* same_type = ABSL_DIE_IF_NULL(cel_OpaqueType_New(
      cel_StringView_FromString("opaque_type"), 2, &params, arena()));
  params[0] = cel_IntType;
  params[1] = cel_DynType;
  const cel_OpaqueType* diff_type = ABSL_DIE_IF_NULL(cel_OpaqueType_New(
      cel_StringView_FromString("opaque_type"), 2, &params, arena()));
  params[0] = cel_StringType;
  params[1] = cel_DynType;
  EXPECT_TRUE(cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(same_type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(same_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(diff_type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(diff_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(cel_Type_Equals(cel_Type_UpCast(type), cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_Type_UpCast(type)));
}

TEST_F(TypeTest, Optional) {
  const cel_OptionalType* type =
      ABSL_DIE_IF_NULL(cel_OptionalType_New(cel_DynType, arena()));
  EXPECT_EQ(cel_Type_Kind(cel_Type_UpCast(type)), cel_TypeKind_kOpaque);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_Type_UpCast(type)),
                                    cel_StringView_From("optional_type")));
  EXPECT_TRUE(
      cel_StringView_Equals(cel_OpaqueType_Name(cel_OpaqueType_UpCast(type)),
                            cel_StringView_From("optional_type")));
  EXPECT_TRUE(cel_Type_IsDyn(cel_OptionalType_Param(type)));
  EXPECT_TRUE(cel_Type_IsOptional(cel_Type_UpCast(type)));
  const cel_OptionalType* same_type =
      ABSL_DIE_IF_NULL(cel_OptionalType_New(cel_DynType, arena()));
  const cel_OptionalType* diff_type =
      ABSL_DIE_IF_NULL(cel_OptionalType_New(cel_BoolType, arena()));
  EXPECT_TRUE(cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(same_type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(same_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(diff_type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(diff_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(cel_Type_Equals(cel_Type_UpCast(type), cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_Type_UpCast(type)));
}

TEST_F(TypeTest, BoolWrapper) {
  EXPECT_EQ(cel_Type_Kind(cel_BoolWrapperType), cel_TypeKind_kBoolWrapper);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_Type_Name(cel_BoolWrapperType),
                            cel_StringView_From("google.protobuf.BoolValue")));
  EXPECT_TRUE(cel_Type_IsBoolWrapper(cel_BoolWrapperType));
  EXPECT_TRUE(cel_Type_Equals(cel_BoolWrapperType, cel_BoolWrapperType));
  EXPECT_TRUE(cel_Type_Equals(cel_BoolWrapperType, cel_BoolWrapperType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolWrapperType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_BoolWrapperType));
}

TEST_F(TypeTest, IntWrapper) {
  EXPECT_EQ(cel_Type_Kind(cel_IntWrapperType), cel_TypeKind_kIntWrapper);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_Type_Name(cel_IntWrapperType),
                            cel_StringView_From("google.protobuf.Int64Value")));
  EXPECT_TRUE(cel_Type_IsIntWrapper(cel_IntWrapperType));
  EXPECT_TRUE(cel_Type_Equals(cel_IntWrapperType, cel_IntWrapperType));
  EXPECT_TRUE(cel_Type_Equals(cel_IntWrapperType, cel_IntWrapperType));
  EXPECT_FALSE(cel_Type_Equals(cel_IntWrapperType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_IntWrapperType));
}

TEST_F(TypeTest, UintWrapper) {
  EXPECT_EQ(cel_Type_Kind(cel_UintWrapperType), cel_TypeKind_kUintWrapper);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_Type_Name(cel_UintWrapperType),
      cel_StringView_From("google.protobuf.UInt64Value")));
  EXPECT_TRUE(cel_Type_IsUintWrapper(cel_UintWrapperType));
  EXPECT_TRUE(cel_Type_Equals(cel_UintWrapperType, cel_UintWrapperType));
  EXPECT_TRUE(cel_Type_Equals(cel_UintWrapperType, cel_UintWrapperType));
  EXPECT_FALSE(cel_Type_Equals(cel_UintWrapperType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_UintWrapperType));
}

TEST_F(TypeTest, DoubleWrapper) {
  EXPECT_EQ(cel_Type_Kind(cel_DoubleWrapperType), cel_TypeKind_kDoubleWrapper);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_Type_Name(cel_DoubleWrapperType),
      cel_StringView_From("google.protobuf.DoubleValue")));
  EXPECT_TRUE(cel_Type_IsDoubleWrapper(cel_DoubleWrapperType));
  EXPECT_TRUE(cel_Type_Equals(cel_DoubleWrapperType, cel_DoubleWrapperType));
  EXPECT_TRUE(cel_Type_Equals(cel_DoubleWrapperType, cel_DoubleWrapperType));
  EXPECT_FALSE(cel_Type_Equals(cel_DoubleWrapperType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_DoubleWrapperType));
}

TEST_F(TypeTest, StringWrapper) {
  EXPECT_EQ(cel_Type_Kind(cel_StringWrapperType), cel_TypeKind_kStringWrapper);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_Type_Name(cel_StringWrapperType),
      cel_StringView_From("google.protobuf.StringValue")));
  EXPECT_TRUE(cel_Type_IsStringWrapper(cel_StringWrapperType));
  EXPECT_TRUE(cel_Type_Equals(cel_StringWrapperType, cel_StringWrapperType));
  EXPECT_TRUE(cel_Type_Equals(cel_StringWrapperType, cel_StringWrapperType));
  EXPECT_FALSE(cel_Type_Equals(cel_StringWrapperType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_StringWrapperType));
}

TEST_F(TypeTest, BytesWrapper) {
  EXPECT_EQ(cel_Type_Kind(cel_BytesWrapperType), cel_TypeKind_kBytesWrapper);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_Type_Name(cel_BytesWrapperType),
                            cel_StringView_From("google.protobuf.BytesValue")));
  EXPECT_TRUE(cel_Type_IsBytesWrapper(cel_BytesWrapperType));
  EXPECT_TRUE(cel_Type_Equals(cel_BytesWrapperType, cel_BytesWrapperType));
  EXPECT_TRUE(cel_Type_Equals(cel_BytesWrapperType, cel_BytesWrapperType));
  EXPECT_FALSE(cel_Type_Equals(cel_BytesWrapperType, cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_BytesWrapperType));
}

TEST_F(TypeTest, TypeParam) {
  const cel_TypeParamType* type = ABSL_DIE_IF_NULL(
      cel_TypeParamType_New(cel_StringView_FromString("T"), arena()));
  EXPECT_EQ(cel_Type_Kind(cel_Type_UpCast(type)), cel_TypeKind_kTypeParam);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_Type_UpCast(type)),
                                    cel_StringView_From("T")));
  EXPECT_TRUE(cel_StringView_Equals(cel_TypeParamType_Name(type),
                                    cel_StringView_From("T")));
  EXPECT_TRUE(cel_Type_IsTypeParam(cel_Type_UpCast(type)));
  const cel_TypeParamType* same_type = ABSL_DIE_IF_NULL(
      cel_TypeParamType_New(cel_StringView_FromString("T"), arena()));
  const cel_TypeParamType* diff_type = ABSL_DIE_IF_NULL(
      cel_TypeParamType_New(cel_StringView_FromString("U"), arena()));
  EXPECT_TRUE(cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(same_type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(same_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(diff_type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(diff_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(cel_Type_Equals(cel_Type_UpCast(type), cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_Type_UpCast(type)));
}

TEST_F(TypeTest, Function) {
  const cel_Type** args;
  const cel_FunctionType* type =
      ABSL_DIE_IF_NULL(cel_FunctionType_New(cel_DynType, 2, &args, arena()));
  args[0] = cel_IntType;
  args[1] = cel_DynType;
  EXPECT_EQ(cel_Type_Kind(cel_Type_UpCast(type)), cel_TypeKind_kFunction);
  EXPECT_TRUE(cel_StringView_Equals(cel_Type_Name(cel_Type_UpCast(type)),
                                    cel_StringView_From("**function**")));
  size_t args_len;
  EXPECT_TRUE(cel_Type_IsInt(cel_FunctionType_Args(type, &args_len)[0]));
  EXPECT_EQ(args_len, 2);
  EXPECT_TRUE(cel_Type_IsDyn(cel_FunctionType_Args(type, &args_len)[1]));
  EXPECT_EQ(args_len, 2);
  EXPECT_TRUE(cel_Type_IsFunction(cel_Type_UpCast(type)));
  const cel_FunctionType* same_type =
      ABSL_DIE_IF_NULL(cel_FunctionType_New(cel_DynType, 2, &args, arena()));
  args[0] = cel_IntType;
  args[1] = cel_DynType;
  const cel_FunctionType* diff_type =
      ABSL_DIE_IF_NULL(cel_FunctionType_New(cel_DynType, 2, &args, arena()));
  args[0] = cel_StringType;
  args[1] = cel_DynType;
  EXPECT_TRUE(cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(same_type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(same_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(diff_type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(diff_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(cel_Type_Equals(cel_Type_UpCast(type), cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_Type_UpCast(type)));
}

TEST_F(TypeTest, Enum) {
  const cel_EnumType* type = ABSL_DIE_IF_NULL(cel_EnumType_New(
      cel_StringView_FromString("google.protobuf.Syntax"), arena()));
  EXPECT_EQ(cel_Type_Kind(cel_Type_UpCast(type)), cel_TypeKind_kEnum);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_Type_Name(cel_Type_UpCast(type)),
                            cel_StringView_From("google.protobuf.Syntax")));
  EXPECT_TRUE(cel_StringView_Equals(
      cel_EnumType_Name(type), cel_StringView_From("google.protobuf.Syntax")));
  EXPECT_TRUE(cel_Type_IsEnum(cel_Type_UpCast(type)));
  const cel_EnumType* same_type = ABSL_DIE_IF_NULL(cel_EnumType_New(
      cel_StringView_FromString("google.protobuf.Syntax"), arena()));
  const cel_EnumType* diff_type = ABSL_DIE_IF_NULL(cel_EnumType_New(
      cel_StringView_FromString("google.protobuf.Field.Kind"), arena()));
  EXPECT_TRUE(cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(same_type)));
  EXPECT_TRUE(
      cel_Type_Equals(cel_Type_UpCast(same_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(type), cel_Type_UpCast(diff_type)));
  EXPECT_FALSE(
      cel_Type_Equals(cel_Type_UpCast(diff_type), cel_Type_UpCast(type)));
  EXPECT_FALSE(cel_Type_Equals(cel_Type_UpCast(type), cel_BoolType));
  EXPECT_FALSE(cel_Type_Equals(cel_BoolType, cel_Type_UpCast(type)));
}

}  // namespace
