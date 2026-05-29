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

#include "cel-c/well_known_types.h"

#include <cstring>

#include "google/protobuf/any.upbdefs.h"
#include "google/protobuf/duration.upbdefs.h"
#include "google/protobuf/field_mask.upbdefs.h"
#include "google/protobuf/struct.upbdefs.h"
#include "google/protobuf/timestamp.upbdefs.h"
#include "google/protobuf/wrappers.upbdefs.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "cel-c/config.h"
#include "cel-c/status.h"
#include "upb/reflection/def.h"

namespace {

using ::testing::NotNull;

class WellKnownTypesTest : public ::testing::Test {
 public:
  void SetUp() override {
    def_pool_ = ABSL_DIE_IF_NULL(upb_DefPool_New());
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
        ABSL_DIE_IF_NULL(google_protobuf_BytesValue_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_StringValue_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Duration_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Timestamp_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Any_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Value_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Struct_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_ListValue_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_FieldMask_getmsgdef(def_pool_)));
    cel_Status_Construct(&status_);
  }

  void TearDown() override {
    cel_Status_Destruct(&status_);
    upb_DefPool_Free(def_pool_);
    def_pool_ = nullptr;
  }

 protected:
  CEL_NONNULL(upb_DefPool*) def_pool() { return ABSL_DIE_IF_NULL(def_pool_); }

  CEL_NONNULL(cel_Status*) status() { return &status_; }

 private:
  CEL_NULLABLE(upb_DefPool*) def_pool_ = nullptr;
  cel_Status status_;
};

TEST_F(WellKnownTypesTest, NullValue) {
  cel_WellKnownTypes wkts;
  cel_NullValueWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(
      cel_NullValueWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.value_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.null_value, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, BoolValue) {
  cel_WellKnownTypes wkts;
  cel_BoolValueWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(
      cel_BoolValueWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.value_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.bool_value, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, Int32Value) {
  cel_WellKnownTypes wkts;
  cel_Int32ValueWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(
      cel_Int32ValueWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.value_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.int32_value, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, UInt32Value) {
  cel_WellKnownTypes wkts;
  cel_UInt32ValueWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(
      cel_UInt32ValueWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.value_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.uint32_value, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, Int64Value) {
  cel_WellKnownTypes wkts;
  cel_Int64ValueWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(
      cel_Int64ValueWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.value_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.int64_value, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, UInt64Value) {
  cel_WellKnownTypes wkts;
  cel_UInt64ValueWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(
      cel_UInt64ValueWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.value_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.uint64_value, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, FloatValue) {
  cel_WellKnownTypes wkts;
  cel_FloatValueWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(
      cel_FloatValueWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.value_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.float_value, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, DoubleValue) {
  cel_WellKnownTypes wkts;
  cel_DoubleValueWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(
      cel_DoubleValueWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.value_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.double_value, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, BytesValue) {
  cel_WellKnownTypes wkts;
  cel_BytesValueWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(
      cel_BytesValueWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.value_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.bytes_value, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, StringValue) {
  cel_WellKnownTypes wkts;
  cel_StringValueWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(
      cel_StringValueWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.value_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.string_value, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, Duration) {
  cel_WellKnownTypes wkts;
  cel_DurationWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(cel_DurationWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.seconds_def, NotNull());
  EXPECT_THAT(wkt.nanos_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.duration, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, Timestamp) {
  cel_WellKnownTypes wkts;
  cel_TimestampWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(
      cel_TimestampWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.seconds_def, NotNull());
  EXPECT_THAT(wkt.nanos_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.timestamp, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, Any) {
  cel_WellKnownTypes wkts;
  cel_AnyWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(cel_AnyWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.type_url_def, NotNull());
  EXPECT_THAT(wkt.value_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.any, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, Value) {
  cel_WellKnownTypes wkts;
  cel_ValueWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(cel_ValueWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.kind_def, NotNull());
  EXPECT_THAT(wkt.null_value_def, NotNull());
  EXPECT_THAT(wkt.number_value_def, NotNull());
  EXPECT_THAT(wkt.string_value_def, NotNull());
  EXPECT_THAT(wkt.struct_value_def, NotNull());
  EXPECT_THAT(wkt.list_value_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.value, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, Struct) {
  cel_WellKnownTypes wkts;
  cel_StructWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(cel_StructWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.fields_def, NotNull());
  EXPECT_THAT(wkt.fields_key_def, NotNull());
  EXPECT_THAT(wkt.fields_value_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.struct_value, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, ListValue) {
  cel_WellKnownTypes wkts;
  cel_ListValueWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(
      cel_ListValueWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.values_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.list_value, &wkt, sizeof(wkt)), 0);
}

TEST_F(WellKnownTypesTest, FieldMask) {
  cel_WellKnownTypes wkts;
  cel_FieldMaskWellKnownType wkt;
  ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts, def_pool(), status()));
  ASSERT_TRUE(
      cel_FieldMaskWellKnownType_Initialize(&wkt, def_pool(), status()));
  EXPECT_THAT(wkt.def, NotNull());
  EXPECT_THAT(wkt.paths_def, NotNull());
  EXPECT_EQ(std::memcmp(&wkts.field_mask, &wkt, sizeof(wkt)), 0);
}

}  // namespace
