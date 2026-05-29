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

#include "cel-c/constant_proto_v1alpha1.h"

#include "google/api/expr/v1alpha1/syntax.upb.h"
#include "google/protobuf/duration.upb.h"
#include "google/protobuf/struct.upb.h"
#include "google/protobuf/timestamp.upb.h"
#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "cel-c/config.h"
#include "cel-c/constant.h"
#include "cel-c/duration.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"
#include "upb/mem/arena.h"

namespace {

using ::testing::Test;

class ConstantProtoV1Alpha1Test : public Test {
 public:
  void SetUp() override {
    cel_Status_Construct(&status_);
    arena_ = ABSL_DIE_IF_NULL(upb_Arena_New());
  }

  void TearDown() override {
    cel_Status_Destruct(&status_);
    upb_Arena_Free(arena_);
    arena_ = nullptr;
  }

 protected:
  CEL_NONNULL(cel_Status*) status() { return &status_; }

  CEL_NONNULL(upb_Arena*) arena() { return ABSL_DIE_IF_NULL(arena_); }

 private:
  cel_Status status_;
  CEL_NULLABILITY_UNKNOWN(upb_Arena*) arena_ = nullptr;
};

TEST_F(ConstantProtoV1Alpha1Test, Unspecified) {
  const cel_Constant want = cel_UnspecifiedConstant();
  cel_Constant in = want;
  CEL_NONNULL(google_api_expr_v1alpha1_Constant*)
  out = ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_Constant_new(arena()));
  ASSERT_TRUE(cel_Constant_ToProtoV1Alpha1(&in, arena(), out, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(google_api_expr_v1alpha1_Constant_constant_kind_case(out),
            google_api_expr_v1alpha1_Constant_constant_kind_NOT_SET);
  ASSERT_TRUE(cel_Constant_FromProtoV1Alpha1(&in, out, arena(), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Constant_Equals(&in, &want));
}

TEST_F(ConstantProtoV1Alpha1Test, Null) {
  const cel_Constant want = cel_NullConstant();
  cel_Constant in = want;
  CEL_NONNULL(google_api_expr_v1alpha1_Constant*)
  out = ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_Constant_new(arena()));
  ASSERT_TRUE(cel_Constant_ToProtoV1Alpha1(&in, arena(), out, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(google_api_expr_v1alpha1_Constant_constant_kind_case(out),
            google_api_expr_v1alpha1_Constant_constant_kind_null_value);
  EXPECT_EQ(google_api_expr_v1alpha1_Constant_null_value(out),
            google_protobuf_NULL_VALUE);
  ASSERT_TRUE(cel_Constant_FromProtoV1Alpha1(&in, out, arena(), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Constant_Equals(&in, &want));
}

TEST_F(ConstantProtoV1Alpha1Test, Bool) {
  const cel_Constant want = cel_BoolConstant(true);
  cel_Constant in = want;
  CEL_NONNULL(google_api_expr_v1alpha1_Constant*)
  out = ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_Constant_new(arena()));
  ASSERT_TRUE(cel_Constant_ToProtoV1Alpha1(&in, arena(), out, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(google_api_expr_v1alpha1_Constant_constant_kind_case(out),
            google_api_expr_v1alpha1_Constant_constant_kind_bool_value);
  EXPECT_TRUE(google_api_expr_v1alpha1_Constant_bool_value(out));
  ASSERT_TRUE(cel_Constant_FromProtoV1Alpha1(&in, out, arena(), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Constant_Equals(&in, &want));
}

TEST_F(ConstantProtoV1Alpha1Test, Int) {
  const cel_Constant want = cel_IntConstant(1);
  cel_Constant in = want;
  CEL_NONNULL(google_api_expr_v1alpha1_Constant*)
  out = ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_Constant_new(arena()));
  ASSERT_TRUE(cel_Constant_ToProtoV1Alpha1(&in, arena(), out, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(google_api_expr_v1alpha1_Constant_constant_kind_case(out),
            google_api_expr_v1alpha1_Constant_constant_kind_int64_value);
  EXPECT_EQ(google_api_expr_v1alpha1_Constant_int64_value(out), 1);
  ASSERT_TRUE(cel_Constant_FromProtoV1Alpha1(&in, out, arena(), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Constant_Equals(&in, &want));
}

TEST_F(ConstantProtoV1Alpha1Test, Uint) {
  const cel_Constant want = cel_UintConstant(1u);
  cel_Constant in = want;
  CEL_NONNULL(google_api_expr_v1alpha1_Constant*)
  out = ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_Constant_new(arena()));
  ASSERT_TRUE(cel_Constant_ToProtoV1Alpha1(&in, arena(), out, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(google_api_expr_v1alpha1_Constant_constant_kind_case(out),
            google_api_expr_v1alpha1_Constant_constant_kind_uint64_value);
  EXPECT_EQ(google_api_expr_v1alpha1_Constant_uint64_value(out), 1u);
  ASSERT_TRUE(cel_Constant_FromProtoV1Alpha1(&in, out, arena(), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Constant_Equals(&in, &want));
}

TEST_F(ConstantProtoV1Alpha1Test, Double) {
  const cel_Constant want = cel_DoubleConstant(1.0);
  cel_Constant in = want;
  CEL_NONNULL(google_api_expr_v1alpha1_Constant*)
  out = ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_Constant_new(arena()));
  ASSERT_TRUE(cel_Constant_ToProtoV1Alpha1(&in, arena(), out, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(google_api_expr_v1alpha1_Constant_constant_kind_case(out),
            google_api_expr_v1alpha1_Constant_constant_kind_double_value);
  EXPECT_EQ(google_api_expr_v1alpha1_Constant_double_value(out), 1.0);
  ASSERT_TRUE(cel_Constant_FromProtoV1Alpha1(&in, out, arena(), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Constant_Equals(&in, &want));
}

TEST_F(ConstantProtoV1Alpha1Test, Bytes) {
  const cel_Constant want = cel_BytesConstant(cel_StringView_From("foo"));
  cel_Constant in = want;
  CEL_NONNULL(google_api_expr_v1alpha1_Constant*)
  out = ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_Constant_new(arena()));
  ASSERT_TRUE(cel_Constant_ToProtoV1Alpha1(&in, arena(), out, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(google_api_expr_v1alpha1_Constant_constant_kind_case(out),
            google_api_expr_v1alpha1_Constant_constant_kind_bytes_value);
  EXPECT_TRUE(
      cel_StringView_Equals(google_api_expr_v1alpha1_Constant_bytes_value(out),
                            cel_StringView_From("foo")));
  ASSERT_TRUE(cel_Constant_FromProtoV1Alpha1(&in, out, arena(), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Constant_Equals(&in, &want));
}

TEST_F(ConstantProtoV1Alpha1Test, String) {
  const cel_Constant want = cel_StringConstant(cel_StringView_From("foo"));
  cel_Constant in = want;
  CEL_NONNULL(google_api_expr_v1alpha1_Constant*)
  out = ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_Constant_new(arena()));
  ASSERT_TRUE(cel_Constant_ToProtoV1Alpha1(&in, arena(), out, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(google_api_expr_v1alpha1_Constant_constant_kind_case(out),
            google_api_expr_v1alpha1_Constant_constant_kind_string_value);
  EXPECT_TRUE(
      cel_StringView_Equals(google_api_expr_v1alpha1_Constant_string_value(out),
                            cel_StringView_From("foo")));
  ASSERT_TRUE(cel_Constant_FromProtoV1Alpha1(&in, out, arena(), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Constant_Equals(&in, &want));
}

TEST_F(ConstantProtoV1Alpha1Test, Duration) {
  const cel_Constant want = cel_DurationConstant(cel_Duration_FromUnix(1, 1));
  cel_Constant in = want;
  CEL_NONNULL(google_api_expr_v1alpha1_Constant*)
  out = ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_Constant_new(arena()));
  ASSERT_TRUE(cel_Constant_ToProtoV1Alpha1(&in, arena(), out, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(google_api_expr_v1alpha1_Constant_constant_kind_case(out),
            google_api_expr_v1alpha1_Constant_constant_kind_duration_value);
  CEL_NONNULL(const google_protobuf_Duration*)
  out_duration =
      ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_Constant_duration_value(out));
  EXPECT_EQ(google_protobuf_Duration_seconds(out_duration), 1);
  EXPECT_EQ(google_protobuf_Duration_nanos(out_duration), 1);
  ASSERT_TRUE(cel_Constant_FromProtoV1Alpha1(&in, out, arena(), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Constant_Equals(&in, &want));
}

TEST_F(ConstantProtoV1Alpha1Test, Timestamp) {
  const cel_Constant want = cel_TimestampConstant(cel_Timestamp_FromUnix(1, 1));
  cel_Constant in = want;
  CEL_NONNULL(google_api_expr_v1alpha1_Constant*)
  out = ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_Constant_new(arena()));
  ASSERT_TRUE(cel_Constant_ToProtoV1Alpha1(&in, arena(), out, status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(google_api_expr_v1alpha1_Constant_constant_kind_case(out),
            google_api_expr_v1alpha1_Constant_constant_kind_timestamp_value);
  CEL_NONNULL(const google_protobuf_Timestamp*)
  out_timestamp =
      ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_Constant_timestamp_value(out));
  EXPECT_EQ(google_protobuf_Timestamp_seconds(out_timestamp), 1);
  EXPECT_EQ(google_protobuf_Timestamp_nanos(out_timestamp), 1);
  ASSERT_TRUE(cel_Constant_FromProtoV1Alpha1(&in, out, arena(), status()));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Constant_Equals(&in, &want));
}

}  // namespace
