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

#include "cel-c/ref_proto_v1alpha1.h"

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
#include "cel-c/constant.h"
#include "cel-c/ref.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "google/protobuf/text_format.h"

namespace {

using ::testing::IsNull;
using ::testing::NotNull;

class RefFromProtoV1Alpha1Test : public ::testing::Test {
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

  google_api_expr_v1alpha1_Reference* ParseTextProto(absl::string_view text) {
    google::api::expr::v1alpha1::Reference proto;
    ABSL_CHECK(google::protobuf::TextFormat::ParseFromString(text, &proto));
    std::string proto_bytes;
    ABSL_CHECK(proto.SerializePartialToString(&proto_bytes));
    return ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_Reference_parse(
        proto_bytes.data(), proto_bytes.size(), arena()));
  }

 private:
  cel_Status status_;
  cel_Arena* arena_ = nullptr;
};

TEST_F(RefFromProtoV1Alpha1Test, Ident) {
  cel_Ref* ref = cel_Ref_FromProtoV1Alpha1(
      ParseTextProto(R"pb(name: "foo"
                          value: { bool_value: true })pb"),
      arena(), status());
  ASSERT_THAT(ref, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Ref_IsIdent(ref));
  EXPECT_EQ(cel_Ref_Name(ref), cel_StringView_FromString("foo"));
  EXPECT_EQ(*cel_IdentRef_Value(cel_IdentRef_DownCast(ref)),
            cel_BoolConstant(true));
}

TEST_F(RefFromProtoV1Alpha1Test, Function) {
  cel_Ref* ref = cel_Ref_FromProtoV1Alpha1(
      ParseTextProto(R"pb(name: "foo"
                          overload_id: [ "bar", "baz" ])pb"),
      arena(), status());
  ASSERT_THAT(ref, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Ref_IsFunction(ref));
  EXPECT_EQ(cel_Ref_Name(ref), cel_StringView_FromString("foo"));

  cel_FunctionOverloadRef* overload_head;
  cel_FunctionOverloadRef* overload_tail;
  EXPECT_EQ(cel_FunctionRef_Overloads(cel_FunctionRef_DownCast(ref),
                                      &overload_head, &overload_tail),
            2);
  ASSERT_THAT(overload_head, NotNull());
  ASSERT_THAT(overload_tail, NotNull());
  EXPECT_NE(overload_head, overload_tail);

  EXPECT_THAT(cel_FunctionOverloadRef_Prev(overload_head), IsNull());
  EXPECT_EQ(cel_FunctionOverloadRef_Next(overload_head), overload_tail);
  EXPECT_EQ(cel_FunctionOverloadRef_Id(overload_head),
            cel_StringView_FromString("bar"));
  EXPECT_EQ(cel_FunctionOverloadRef_Prev(overload_tail), overload_head);
  EXPECT_THAT(cel_FunctionOverloadRef_Next(overload_tail), IsNull());
  EXPECT_EQ(cel_FunctionOverloadRef_Id(overload_tail),
            cel_StringView_FromString("baz"));
}

}  // namespace
