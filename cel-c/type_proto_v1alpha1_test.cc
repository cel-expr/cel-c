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

#include "cel-c/type_proto_v1alpha1.h"

#include <cstddef>
#include <string>

#include "google/api/expr/v1alpha1/checked.pb.h"
#include "google/api/expr/v1alpha1/checked.upb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/absl_check.h"
#include "absl/log/die_if_null.h"
#include "absl/strings/string_view.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/type.h"
#include "google/protobuf/text_format.h"

namespace {

using ::testing::NotNull;

class TypeFromProtoV1Alpha1Test : public ::testing::Test {
 public:
  void SetUp() override {
    cel_Status_Construct(&status_);
    arena_ = ABSL_DIE_IF_NULL(cel_Arena_New(alloc()));
  }

  void TearDown() override {
    cel_Arena_Delete(arena_);
    arena_ = nullptr;
    cel_Status_Destruct(&status_);
  }

 protected:
  cel_Allocator* alloc() { return cel_DefaultAllocator; }

  cel_Status* status() { return &status_; }

  cel_Arena* arena() { return ABSL_DIE_IF_NULL(arena_); }

  google_api_expr_v1alpha1_Type* ParseTextProto(absl::string_view text) {
    google::api::expr::v1alpha1::Type proto;
    ABSL_CHECK(google::protobuf::TextFormat::ParseFromString(text, &proto));
    std::string proto_bytes;
    ABSL_CHECK(proto.SerializePartialToString(&proto_bytes));
    return ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_Type_parse(
        proto_bytes.data(), proto_bytes.size(), arena()));
  }

 private:
  cel_Status status_;
  cel_Arena* arena_ = nullptr;
};

TEST_F(TypeFromProtoV1Alpha1Test, Dyn) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(dyn: {})pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsDyn(type));
  EXPECT_EQ(*type, *cel_DynType);
}

TEST_F(TypeFromProtoV1Alpha1Test, Null) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(null: NULL_VALUE)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsNull(type));
  EXPECT_EQ(*type, *cel_NullType);
}

TEST_F(TypeFromProtoV1Alpha1Test, Bool) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(primitive: BOOL)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsBool(type));
  EXPECT_EQ(*type, *cel_BoolType);
}

TEST_F(TypeFromProtoV1Alpha1Test, Int) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(primitive: INT64)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsInt(type));
  EXPECT_EQ(*type, *cel_IntType);
}

TEST_F(TypeFromProtoV1Alpha1Test, Uint) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(primitive: UINT64)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsUint(type));
  EXPECT_EQ(*type, *cel_UintType);
}

TEST_F(TypeFromProtoV1Alpha1Test, Double) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(primitive: DOUBLE)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsDouble(type));
  EXPECT_EQ(*type, *cel_DoubleType);
}

TEST_F(TypeFromProtoV1Alpha1Test, String) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(primitive: STRING)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsString(type));
  EXPECT_EQ(*type, *cel_StringType);
}

TEST_F(TypeFromProtoV1Alpha1Test, Bytes) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(primitive: BYTES)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsBytes(type));
  EXPECT_EQ(*type, *cel_BytesType);
}

TEST_F(TypeFromProtoV1Alpha1Test, BoolWrapper) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(wrapper: BOOL)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsBoolWrapper(type));
  EXPECT_EQ(*type, *cel_BoolWrapperType);
}

TEST_F(TypeFromProtoV1Alpha1Test, IntWrapper) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(wrapper: INT64)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsIntWrapper(type));
  EXPECT_EQ(*type, *cel_IntWrapperType);
}

TEST_F(TypeFromProtoV1Alpha1Test, UintWrapper) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(wrapper: UINT64)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsUintWrapper(type));
  EXPECT_EQ(*type, *cel_UintWrapperType);
}

TEST_F(TypeFromProtoV1Alpha1Test, DoubleWrapper) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(wrapper: DOUBLE)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsDoubleWrapper(type));
  EXPECT_EQ(*type, *cel_DoubleWrapperType);
}

TEST_F(TypeFromProtoV1Alpha1Test, StringWrapper) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(wrapper: STRING)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsStringWrapper(type));
  EXPECT_EQ(*type, *cel_StringWrapperType);
}

TEST_F(TypeFromProtoV1Alpha1Test, BytesWrapper) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(wrapper: BYTES)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsBytesWrapper(type));
  EXPECT_EQ(*type, *cel_BytesWrapperType);
}

TEST_F(TypeFromProtoV1Alpha1Test, Duration) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(well_known: DURATION)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsDuration(type));
  EXPECT_EQ(*type, *cel_DurationType);
}

TEST_F(TypeFromProtoV1Alpha1Test, Timestamp) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(well_known: TIMESTAMP)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsTimestamp(type));
  EXPECT_EQ(*type, *cel_TimestampType);
}

TEST_F(TypeFromProtoV1Alpha1Test, Any) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(well_known: ANY)pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Type_IsAny(type));
  EXPECT_EQ(*type, *cel_AnyType);
}

TEST_F(TypeFromProtoV1Alpha1Test, List) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(list_type: { elem_type: { primitive: BOOL } })pb"),
      arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  ASSERT_TRUE(cel_Type_IsList(type));
  const cel_ListType* list_type = cel_ListType_DownCast(type);
  EXPECT_EQ(*cel_ListType_Element(list_type), *cel_BoolType);
}

TEST_F(TypeFromProtoV1Alpha1Test, Map) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(map_type: {
                            key_type: { primitive: STRING }
                            value_type: { primitive: BOOL }
                          })pb"),
      arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  ASSERT_TRUE(cel_Type_IsMap(type));
  const cel_MapType* map_type = cel_MapType_DownCast(type);
  EXPECT_EQ(*cel_MapType_Key(map_type), *cel_StringType);
  EXPECT_EQ(*cel_MapType_Value(map_type), *cel_BoolType);
}

TEST_F(TypeFromProtoV1Alpha1Test, Function) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(function: {
                            result_type: { primitive: BOOL }
                            arg_types: { primitive: STRING }
                            arg_types: { primitive: INT64 }
                          })pb"),
      arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  ASSERT_TRUE(cel_Type_IsFunction(type));
  const cel_FunctionType* function_type = cel_FunctionType_DownCast(type);
  EXPECT_EQ(*cel_FunctionType_Result(function_type), *cel_BoolType);
  size_t arg_types_len;
  const cel_Type* const* arg_types =
      cel_FunctionType_Args(function_type, &arg_types_len);
  ASSERT_EQ(arg_types_len, 2);
  EXPECT_EQ(*arg_types[0], *cel_StringType);
  EXPECT_EQ(*arg_types[1], *cel_IntType);
}

TEST_F(TypeFromProtoV1Alpha1Test, Struct) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(message_type: "test.Struct")pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  ASSERT_TRUE(cel_Type_IsStruct(type));
  const cel_StructType* struct_type = cel_StructType_DownCast(type);
  EXPECT_EQ(cel_StructType_Name(struct_type),
            cel_StringView_FromString("test.Struct"));
}

TEST_F(TypeFromProtoV1Alpha1Test, TypeParam) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(type_param: "T")pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  ASSERT_TRUE(cel_Type_IsTypeParam(type));
  const cel_TypeParamType* type_param_type = cel_TypeParamType_DownCast(type);
  EXPECT_EQ(cel_TypeParamType_Name(type_param_type),
            cel_StringView_FromString("T"));
}

TEST_F(TypeFromProtoV1Alpha1Test, Type) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(type: { primitive: INT64 })pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  ASSERT_TRUE(cel_Type_IsType(type));
  const cel_TypeType* type_type = cel_TypeType_DownCast(type);
  EXPECT_EQ(*cel_TypeType_Type(type_type), *cel_IntType);
}

TEST_F(TypeFromProtoV1Alpha1Test, Error) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(error: {})pb"), arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  ASSERT_TRUE(cel_Type_IsError(type));
}

TEST_F(TypeFromProtoV1Alpha1Test, Opaque) {
  const cel_Type* type = cel_Type_FromProtoV1Alpha1(
      ParseTextProto(R"pb(abstract_type: {
                            name: "optional_type"
                            parameter_types: { primitive: STRING }
                          })pb"),
      arena(), status());
  ASSERT_THAT(type, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  ASSERT_TRUE(cel_Type_IsOpaque(type));
  ASSERT_TRUE(cel_Type_IsOptional(type));
  const cel_OptionalType* optional_type = cel_OptionalType_DownCast(type);
  EXPECT_EQ(*cel_OptionalType_Param(optional_type), *cel_StringType);
}

}  // namespace
