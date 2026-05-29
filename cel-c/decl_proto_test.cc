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

#include "cel-c/decl_proto.h"

#include <string>

#include "cel/expr/checked.pb.h"
#include "cel/expr/checked.upb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/absl_check.h"
#include "absl/log/die_if_null.h"
#include "absl/strings/string_view.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/constant.h"
#include "cel-c/decl.h"
#include "cel-c/function_scope.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/type.h"
#include "google/protobuf/text_format.h"

namespace {

using ::testing::NotNull;

class DeclFromProtoTest : public ::testing::Test {
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

  cel_expr_Decl* ParseTextProto(absl::string_view text) {
    cel::expr::Decl proto;
    ABSL_CHECK(google::protobuf::TextFormat::ParseFromString(text, &proto));
    std::string proto_bytes;
    ABSL_CHECK(proto.SerializePartialToString(&proto_bytes));
    return ABSL_DIE_IF_NULL(cel_expr_Decl_parse(
        proto_bytes.data(), proto_bytes.size(), arena()));
  }

 private:
  cel_Status status_;
  cel_Arena* arena_ = nullptr;
};

TEST_F(DeclFromProtoTest, Ident) {
  cel_Decl* decl =
      cel_Decl_FromProto(ParseTextProto(R"pb(name: "foo"
                                             ident: {
                                               type: { primitive: BOOL }
                                               value: { bool_value: true }
                                               doc: "doc"
                                             })pb"),
                         arena(), status());
  ASSERT_THAT(decl, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Decl_IsIdent(decl));
  EXPECT_EQ(cel_Decl_Name(decl), cel_StringView_FromString("foo"));
  EXPECT_EQ(cel_Decl_Doc(decl), cel_StringView_FromString("doc"));
  EXPECT_EQ(*cel_IdentDecl_Type(cel_IdentDecl_DownCast(decl)), *cel_BoolType);
  EXPECT_EQ(*cel_IdentDecl_Value(cel_IdentDecl_DownCast(decl)),
            cel_BoolConstant(true));
}

TEST_F(DeclFromProtoTest, Function) {
  cel_Decl* decl = cel_Decl_FromProto(
      ParseTextProto(R"pb(name: "foo"
                          function: {
                            overloads: {
                              overload_id: "bar"
                              params: { primitive: STRING }
                              result_type: { primitive: INT64 }
                              is_instance_function: true
                              doc: "doc"
                            }
                            doc: "doc"
                          })pb"),
      arena(), status());
  ASSERT_THAT(decl, NotNull());
  EXPECT_TRUE(cel_Status_Ok(status()));

  EXPECT_TRUE(cel_Decl_IsFunction(decl));
  EXPECT_EQ(cel_Decl_Name(decl), cel_StringView_FromString("foo"));
  EXPECT_EQ(cel_Decl_Doc(decl), cel_StringView_FromString("doc"));

  cel_FunctionOverloadDecl* overload_head;
  cel_FunctionOverloadDecl* overload_tail;
  ASSERT_EQ(cel_FunctionDecl_Overloads(cel_FunctionDecl_DownCast(decl),
                                       &overload_head, &overload_tail),
            1);
  ASSERT_THAT(overload_head, NotNull());
  EXPECT_EQ(overload_head, overload_tail);
  EXPECT_EQ(cel_FunctionOverloadDecl_Id(overload_head),
            cel_StringView_FromString("bar"));
  EXPECT_EQ(cel_FunctionOverloadDecl_Scope(overload_head),
            cel_FunctionScope_kMember);
  const cel_FunctionType* overload_type =
      cel_FunctionOverloadDecl_Type(overload_head);
  ASSERT_THAT(overload_type, NotNull());
  EXPECT_EQ(*cel_FunctionType_Result(overload_type), *cel_IntType);
  ASSERT_EQ(*cel_FunctionType_Args(overload_type, nullptr)[0], *cel_StringType);
  EXPECT_EQ(cel_FunctionOverloadDecl_Doc(overload_head),
            cel_StringView_FromString("doc"));
}

}  // namespace
