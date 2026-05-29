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

#include "cel-c/ast_proto_v1alpha1.h"

#include <memory>
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
#include "cel-c/ast.h"
#include "cel-c/config.h"
#include "cel-c/constant.h"
#include "cel-c/duration.h"
#include "cel-c/operators.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"
#include "google/protobuf/text_format.h"

namespace {

using ::testing::IsNull;
using ::testing::NotNull;

struct AstDeleter {
  void operator()(CEL_NULLABLE(cel_Ast*) ast) const { cel_Ast_Delete(ast); }
};

using AstPtr = std::unique_ptr<cel_Ast, AstDeleter>;

class AstFromProtoTest : public ::testing::Test {
 public:
  void SetUp() override {
    cel_Status_Construct(&status_);
    arena_ = ABSL_DIE_IF_NULL(cel_Arena_New(alloc()));
  }

  void TearDown() override {
    cel_Status_Destruct(&status_);
    cel_Arena_Delete(arena_);
    arena_ = nullptr;
  }

 protected:
  CEL_NONNULL(google_api_expr_v1alpha1_CheckedExpr*)
  ParseTextProto(absl::string_view text) {
    google::api::expr::v1alpha1::CheckedExpr proto;
    ABSL_CHECK(google::protobuf::TextFormat::ParseFromString(text, &proto));
    std::string proto_bytes;
    ABSL_CHECK(proto.SerializePartialToString(&proto_bytes));
    return ABSL_DIE_IF_NULL(google_api_expr_v1alpha1_CheckedExpr_parse(
        proto_bytes.data(), proto_bytes.size(), arena()));
  }

  CEL_NONNULL(cel_Allocator*) alloc() { return cel_DefaultAllocator; }

  CEL_NONNULL(cel_Status*) status() { return &status_; }

  CEL_NONNULL(cel_Arena*) arena() { return ABSL_DIE_IF_NULL(arena_); }

 private:
  cel_Status status_;
  CEL_NULLABILITY_UNKNOWN(cel_Arena*) arena_ = nullptr;
};

TEST_F(AstFromProtoTest, Empty) {
  AstPtr ast(
      cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb()pb"), arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 0);
  EXPECT_THAT(cel_Ast_Expr(ast.get()), IsNull());
}

TEST_F(AstFromProtoTest, Unspecified) {
  AstPtr ast(
      cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                  source_info: { positions { key: 1 value: 1 } }
                                  expr: { id: 1 }
                                )pb"),
                                arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 1);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kUnspecified);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_UnspecifiedExpr* unspecified_expr = cel_UnspecifiedExpr_DownCast(expr);
  ASSERT_THAT(unspecified_expr, NotNull());
}

TEST_F(AstFromProtoTest, Ident) {
  AstPtr ast(
      cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                  source_info: { positions { key: 1 value: 1 } }
                                  expr: {
                                    id: 1
                                    ident_expr: { name: "foo" }
                                  }
                                )pb"),
                                arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 1);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_IdentExpr* ident_expr = cel_IdentExpr_DownCast(expr);
  ASSERT_THAT(ident_expr, NotNull());
  EXPECT_TRUE(cel_StringView_Equals(cel_IdentExpr_Name(ident_expr),
                                    cel_StringView_From("foo")));
}

TEST_F(AstFromProtoTest, ConstUnspecified) {
  AstPtr ast(
      cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                  source_info: { positions { key: 1 value: 1 } }
                                  expr: {
                                    id: 1
                                    const_expr: {}
                                  }
                                )pb"),
                                arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 1);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kConst);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_ConstExpr* const_expr = cel_ConstExpr_DownCast(expr);
  ASSERT_THAT(const_expr, NotNull());
  ASSERT_EQ(cel_Constant_Kind(cel_ConstExpr_Value(const_expr)),
            cel_ConstantKind_kUnspecified);
}

TEST_F(AstFromProtoTest, ConstNull) {
  AstPtr ast(
      cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                  source_info: { positions { key: 1 value: 1 } }
                                  expr: {
                                    id: 1
                                    const_expr: { null_value: NULL_VALUE }
                                  }
                                )pb"),
                                arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 1);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kConst);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_ConstExpr* const_expr = cel_ConstExpr_DownCast(expr);
  ASSERT_THAT(const_expr, NotNull());
  ASSERT_EQ(cel_Constant_Kind(cel_ConstExpr_Value(const_expr)),
            cel_ConstantKind_kNull);
}

TEST_F(AstFromProtoTest, ConstBool) {
  AstPtr ast(
      cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                  source_info: { positions { key: 1 value: 1 } }
                                  expr: {
                                    id: 1
                                    const_expr: { bool_value: true }
                                  }
                                )pb"),
                                arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 1);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kConst);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_ConstExpr* const_expr = cel_ConstExpr_DownCast(expr);
  ASSERT_THAT(const_expr, NotNull());
  ASSERT_EQ(cel_Constant_Kind(cel_ConstExpr_Value(const_expr)),
            cel_ConstantKind_kBool);
  EXPECT_TRUE(cel_Constant_GetBool(cel_ConstExpr_Value(const_expr)));
}

TEST_F(AstFromProtoTest, ConstInt) {
  AstPtr ast(
      cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                  source_info: { positions { key: 1 value: 1 } }
                                  expr: {
                                    id: 1
                                    const_expr: { int64_value: 1 }
                                  }
                                )pb"),
                                arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 1);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kConst);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_ConstExpr* const_expr = cel_ConstExpr_DownCast(expr);
  ASSERT_THAT(const_expr, NotNull());
  ASSERT_EQ(cel_Constant_Kind(cel_ConstExpr_Value(const_expr)),
            cel_ConstantKind_kInt);
  EXPECT_EQ(cel_Constant_GetInt(cel_ConstExpr_Value(const_expr)), 1);
}

TEST_F(AstFromProtoTest, ConstUint) {
  AstPtr ast(
      cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                  source_info: { positions { key: 1 value: 1 } }
                                  expr: {
                                    id: 1
                                    const_expr: { uint64_value: 1 }
                                  }
                                )pb"),
                                arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 1);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kConst);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_ConstExpr* const_expr = cel_ConstExpr_DownCast(expr);
  ASSERT_THAT(const_expr, NotNull());
  ASSERT_EQ(cel_Constant_Kind(cel_ConstExpr_Value(const_expr)),
            cel_ConstantKind_kUint);
  EXPECT_EQ(cel_Constant_GetUint(cel_ConstExpr_Value(const_expr)), 1);
}

TEST_F(AstFromProtoTest, ConstDouble) {
  AstPtr ast(
      cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                  source_info: { positions { key: 1 value: 1 } }
                                  expr: {
                                    id: 1
                                    const_expr: { double_value: 1 }
                                  }
                                )pb"),
                                arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 1);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kConst);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_ConstExpr* const_expr = cel_ConstExpr_DownCast(expr);
  ASSERT_THAT(const_expr, NotNull());
  ASSERT_EQ(cel_Constant_Kind(cel_ConstExpr_Value(const_expr)),
            cel_ConstantKind_kDouble);
  EXPECT_EQ(cel_Constant_GetDouble(cel_ConstExpr_Value(const_expr)), 1.0);
}

TEST_F(AstFromProtoTest, ConstBytes) {
  AstPtr ast(
      cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                  source_info: { positions { key: 1 value: 1 } }
                                  expr: {
                                    id: 1
                                    const_expr: { bytes_value: "foo" }
                                  }
                                )pb"),
                                arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 1);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kConst);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_ConstExpr* const_expr = cel_ConstExpr_DownCast(expr);
  ASSERT_THAT(const_expr, NotNull());
  ASSERT_EQ(cel_Constant_Kind(cel_ConstExpr_Value(const_expr)),
            cel_ConstantKind_kBytes);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_Constant_GetBytes(cel_ConstExpr_Value(const_expr)),
      cel_StringView_From("foo")));
}

TEST_F(AstFromProtoTest, ConstString) {
  AstPtr ast(
      cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                  source_info: { positions { key: 1 value: 1 } }
                                  expr: {
                                    id: 1
                                    const_expr: { string_value: "foo" }
                                  }
                                )pb"),
                                arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 1);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kConst);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_ConstExpr* const_expr = cel_ConstExpr_DownCast(expr);
  ASSERT_THAT(const_expr, NotNull());
  ASSERT_EQ(cel_Constant_Kind(cel_ConstExpr_Value(const_expr)),
            cel_ConstantKind_kString);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_Constant_GetString(cel_ConstExpr_Value(const_expr)),
      cel_StringView_From("foo")));
}

TEST_F(AstFromProtoTest, ConstDuration) {
  AstPtr ast(cel_Ast_FromProtoV1Alpha1(
      ParseTextProto(R"pb(
        source_info: { positions { key: 1 value: 1 } }
        expr: {
          id: 1
          const_expr: { duration_value: { seconds: 1 nanos: 1 } }
        }
      )pb"),
      arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 1);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kConst);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_ConstExpr* const_expr = cel_ConstExpr_DownCast(expr);
  ASSERT_THAT(const_expr, NotNull());
  ASSERT_EQ(cel_Constant_Kind(cel_ConstExpr_Value(const_expr)),
            cel_ConstantKind_kDuration);
  EXPECT_TRUE(cel_Duration_Equals(
      cel_Constant_GetDuration(cel_ConstExpr_Value(const_expr)),
      cel_Duration_FromUnix(1, 1)));
}

TEST_F(AstFromProtoTest, ConstTimestamp) {
  AstPtr ast(cel_Ast_FromProtoV1Alpha1(
      ParseTextProto(R"pb(
        source_info: { positions { key: 1 value: 1 } }
        expr: {
          id: 1
          const_expr: { timestamp_value: { seconds: 1 nanos: 1 } }
        }
      )pb"),
      arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 1);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kConst);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_ConstExpr* const_expr = cel_ConstExpr_DownCast(expr);
  ASSERT_THAT(const_expr, NotNull());
  ASSERT_EQ(cel_Constant_Kind(cel_ConstExpr_Value(const_expr)),
            cel_ConstantKind_kTimestamp);
  EXPECT_TRUE(cel_Timestamp_Equals(
      cel_Constant_GetTimestamp(cel_ConstExpr_Value(const_expr)),
      cel_Timestamp_FromUnix(1, 1)));
}

TEST_F(AstFromProtoTest, Select) {
  AstPtr ast(cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                         source_info: {
                                           positions { key: 1 value: 1 }
                                           positions { key: 2 value: 2 }
                                         }
                                         expr: {
                                           id: 1
                                           select_expr: {
                                             operand: {
                                               id: 2
                                               ident_expr: { name: "foo" }
                                             }
                                             field: "bar"
                                           }
                                         }
                                       )pb"),
                                       arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 2);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kSelect);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_SelectExpr* select_expr = cel_SelectExpr_DownCast(expr);
  ASSERT_THAT(select_expr, NotNull());
  EXPECT_TRUE(cel_StringView_Equals(cel_SelectExpr_Field(select_expr),
                                    cel_StringView_From("bar")));
  EXPECT_FALSE(cel_SelectExpr_TestOnly(select_expr));

  cel_Expr* operand = cel_SelectExpr_Operand(select_expr);
  ASSERT_THAT(operand, NotNull());
  ASSERT_EQ(cel_Expr_Kind(operand), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(operand), 2);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_IdentExpr_Name(cel_IdentExpr_DownCast(operand)),
                            cel_StringView_From("foo")));
}

TEST_F(AstFromProtoTest, SelectTestOnly) {
  AstPtr ast(cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                         source_info: {
                                           positions { key: 1 value: 1 }
                                           positions { key: 2 value: 2 }
                                         }
                                         expr: {
                                           id: 1
                                           select_expr: {
                                             operand: {
                                               id: 2
                                               ident_expr: { name: "foo" }
                                             }
                                             field: "bar"
                                             test_only: true
                                           }
                                         }
                                       )pb"),
                                       arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 2);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kSelect);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_SelectExpr* select_expr = cel_SelectExpr_DownCast(expr);
  ASSERT_THAT(select_expr, NotNull());
  EXPECT_TRUE(cel_StringView_Equals(cel_SelectExpr_Field(select_expr),
                                    cel_StringView_From("bar")));
  EXPECT_TRUE(cel_SelectExpr_TestOnly(select_expr));

  cel_Expr* operand = cel_SelectExpr_Operand(select_expr);
  ASSERT_THAT(operand, NotNull());
  ASSERT_EQ(cel_Expr_Kind(operand), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(operand), 2);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_IdentExpr_Name(cel_IdentExpr_DownCast(operand)),
                            cel_StringView_From("foo")));
}

TEST_F(AstFromProtoTest, Unary) {
  AstPtr ast(cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                         source_info: {
                                           positions { key: 1 value: 1 }
                                           positions { key: 2 value: 2 }
                                         }
                                         expr: {
                                           id: 1
                                           call_expr: {
                                             function: "-_"
                                             args {
                                               id: 2
                                               ident_expr: { name: "foo" }
                                             }
                                           }
                                         }
                                       )pb"),
                                       arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 2);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kUnary);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_UnaryExpr* unary_expr = cel_UnaryExpr_DownCast(expr);
  ASSERT_THAT(unary_expr, NotNull());
  EXPECT_EQ(cel_UnaryExpr_Op(unary_expr), cel_UnaryOp_kNegate);

  cel_Expr* arg = cel_UnaryExpr_Arg(unary_expr);
  ASSERT_THAT(arg, NotNull());
  ASSERT_EQ(cel_Expr_Kind(arg), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(arg), 2);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_IdentExpr_Name(cel_IdentExpr_DownCast(arg)),
                            cel_StringView_From("foo")));
}

TEST_F(AstFromProtoTest, Binary) {
  AstPtr ast(cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                         source_info: {
                                           positions { key: 1 value: 1 }
                                           positions { key: 2 value: 2 }
                                           positions { key: 3 value: 3 }
                                         }
                                         expr: {
                                           id: 1
                                           call_expr: {
                                             function: "_+_"
                                             args {
                                               id: 2
                                               ident_expr: { name: "foo" }
                                             }
                                             args {
                                               id: 3
                                               ident_expr: { name: "bar" }
                                             }
                                           }
                                         }
                                       )pb"),
                                       arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 3);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kBinary);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_BinaryExpr* binary_expr = cel_BinaryExpr_DownCast(expr);
  ASSERT_THAT(binary_expr, NotNull());
  EXPECT_EQ(cel_BinaryExpr_Op(binary_expr), cel_BinaryOp_kAdd);

  cel_Expr* left = cel_BinaryExpr_Left(binary_expr);
  ASSERT_THAT(left, NotNull());
  ASSERT_EQ(cel_Expr_Kind(left), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(left), 2);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_IdentExpr_Name(cel_IdentExpr_DownCast(left)),
                            cel_StringView_From("foo")));

  cel_Expr* right = cel_BinaryExpr_Right(binary_expr);
  ASSERT_THAT(left, NotNull());
  ASSERT_EQ(cel_Expr_Kind(right), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(right), 3);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_IdentExpr_Name(cel_IdentExpr_DownCast(right)),
                            cel_StringView_From("bar")));
}

TEST_F(AstFromProtoTest, Ternary) {
  AstPtr ast(cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                         source_info: {
                                           positions { key: 1 value: 1 }
                                           positions { key: 2 value: 2 }
                                           positions { key: 3 value: 3 }
                                           positions { key: 4 value: 4 }
                                         }
                                         expr: {
                                           id: 1
                                           call_expr: {
                                             function: "_?_:_"
                                             args {
                                               id: 2
                                               ident_expr: { name: "foo" }
                                             }
                                             args {
                                               id: 3
                                               ident_expr: { name: "bar" }
                                             }
                                             args {
                                               id: 4
                                               ident_expr: { name: "baz" }
                                             }
                                           }
                                         }
                                       )pb"),
                                       arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 4);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kTernary);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_TernaryExpr* ternary_expr = cel_TernaryExpr_DownCast(expr);
  ASSERT_THAT(ternary_expr, NotNull());
  EXPECT_EQ(cel_TernaryExpr_Op(ternary_expr), cel_TernaryOp_kConditional);

  cel_Expr* condition = cel_TernaryExpr_Condition(ternary_expr);
  ASSERT_THAT(condition, NotNull());
  ASSERT_EQ(cel_Expr_Kind(condition), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(condition), 2);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(condition)),
      cel_StringView_From("foo")));

  cel_Expr* if_true = cel_TernaryExpr_IfTrue(ternary_expr);
  ASSERT_THAT(if_true, NotNull());
  ASSERT_EQ(cel_Expr_Kind(if_true), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(if_true), 3);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_IdentExpr_Name(cel_IdentExpr_DownCast(if_true)),
                            cel_StringView_From("bar")));

  cel_Expr* if_false = cel_TernaryExpr_IfFalse(ternary_expr);
  ASSERT_THAT(if_false, NotNull());
  ASSERT_EQ(cel_Expr_Kind(if_false), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(if_false), 4);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(if_false)),
      cel_StringView_From("baz")));
}

TEST_F(AstFromProtoTest, Call) {
  AstPtr ast(cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                         source_info: {
                                           positions { key: 1 value: 1 }
                                           positions { key: 2 value: 2 }
                                           positions { key: 3 value: 3 }
                                         }
                                         expr: {
                                           id: 1
                                           call_expr: {
                                             target: {
                                               id: 2
                                               ident_expr: { name: "foo" }
                                             }
                                             function: "bar"
                                             args {
                                               id: 3
                                               ident_expr: { name: "baz" }
                                             }
                                           }
                                         }
                                       )pb"),
                                       arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 3);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kCall);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_CallExpr* call_expr = cel_CallExpr_DownCast(expr);
  ASSERT_THAT(call_expr, NotNull());
  EXPECT_TRUE(cel_StringView_Equals(cel_CallExpr_Function(call_expr),
                                    cel_StringView_From("bar")));

  cel_Expr* target = cel_CallExpr_Target(call_expr);
  ASSERT_THAT(target, NotNull());
  ASSERT_EQ(cel_Expr_Kind(target), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(target), 2);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_IdentExpr_Name(cel_IdentExpr_DownCast(target)),
                            cel_StringView_From("foo")));

  ASSERT_EQ(cel_CallExpr_Args(call_expr, cel_nullptr, cel_nullptr), 1);
  cel_CallArgExpr* arg = cel_CallExpr_Arg(call_expr, 0);
  EXPECT_EQ(cel_Expr_SourcePosition(cel_Expr_UpCast(arg)), 3);
  ASSERT_THAT(arg, NotNull());

  cel_Expr* arg_value = cel_CallArgExpr_Value(arg);
  ASSERT_THAT(arg_value, NotNull());
  ASSERT_EQ(cel_Expr_Kind(arg_value), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(arg_value), 3);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(arg_value)),
      cel_StringView_From("baz")));
}

TEST_F(AstFromProtoTest, List) {
  AstPtr ast(cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                         source_info: {
                                           positions { key: 1 value: 1 }
                                           positions { key: 2 value: 2 }
                                           positions { key: 3 value: 3 }
                                         }
                                         expr: {
                                           id: 1
                                           list_expr: {
                                             elements {
                                               id: 2
                                               ident_expr: { name: "foo" }
                                             }
                                             elements {
                                               id: 3
                                               ident_expr: { name: "bar" }
                                             }
                                             optional_indices: 1
                                           }
                                         }
                                       )pb"),
                                       arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 3);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kList);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_ListExpr* list_expr = cel_ListExpr_DownCast(expr);
  ASSERT_THAT(list_expr, NotNull());
  ASSERT_EQ(cel_ListExpr_Elements(list_expr, cel_nullptr, cel_nullptr), 2);

  cel_ListElementExpr* element = cel_ListExpr_Element(list_expr, 0);
  EXPECT_EQ(cel_Expr_SourcePosition(cel_Expr_UpCast(element)), 2);
  ASSERT_THAT(element, NotNull());
  EXPECT_FALSE(cel_ListElementExpr_Optional(element));

  cel_Expr* element_value = cel_ListElementExpr_Value(element);
  ASSERT_THAT(element_value, NotNull());
  ASSERT_EQ(cel_Expr_Kind(element_value), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(element_value), 2);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(element_value)),
      cel_StringView_From("foo")));

  element = cel_ListExpr_Element(list_expr, 1);
  EXPECT_EQ(cel_Expr_SourcePosition(cel_Expr_UpCast(element)), 3);
  ASSERT_THAT(element, NotNull());
  EXPECT_TRUE(cel_ListElementExpr_Optional(element));

  element_value = cel_ListElementExpr_Value(element);
  ASSERT_THAT(element_value, NotNull());
  ASSERT_EQ(cel_Expr_Kind(element_value), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(element_value), 3);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(element_value)),
      cel_StringView_From("bar")));
}

TEST_F(AstFromProtoTest, Map) {
  AstPtr ast(cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                         source_info: {
                                           positions { key: 1 value: 1 }
                                           positions { key: 2 value: 2 }
                                           positions { key: 3 value: 3 }
                                           positions { key: 4 value: 4 }
                                           positions { key: 5 value: 5 }
                                           positions { key: 6 value: 6 }
                                           positions { key: 7 value: 7 }
                                         }
                                         expr: {
                                           id: 1
                                           struct_expr: {
                                             entries {
                                               id: 2
                                               map_key: {
                                                 id: 3
                                                 ident_expr: { name: "foo" }
                                               }
                                               value: {
                                                 id: 4
                                                 ident_expr: { name: "bar" }
                                               }
                                             }
                                             entries {
                                               id: 5
                                               map_key: {
                                                 id: 6
                                                 ident_expr: { name: "baz" }
                                               }
                                               value: {
                                                 id: 7
                                                 ident_expr: { name: "bux" }
                                               }
                                               optional_entry: true
                                             }
                                           }
                                         }
                                       )pb"),
                                       arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 7);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kMap);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_MapExpr* map_expr = cel_MapExpr_DownCast(expr);
  ASSERT_THAT(map_expr, NotNull());
  ASSERT_EQ(cel_MapExpr_Entries(map_expr, cel_nullptr, cel_nullptr), 2);

  cel_MapEntryExpr* entry = cel_MapExpr_Entry(map_expr, 0);
  ASSERT_THAT(entry, NotNull());
  EXPECT_EQ(cel_Expr_SourcePosition(cel_Expr_UpCast(entry)), 2);
  EXPECT_FALSE(cel_MapEntryExpr_Optional(entry));

  cel_Expr* entry_key = cel_MapEntryExpr_Key(entry);
  ASSERT_THAT(entry_key, NotNull());
  ASSERT_EQ(cel_Expr_Kind(entry_key), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(entry_key), 3);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(entry_key)),
      cel_StringView_From("foo")));

  cel_Expr* entry_value = cel_MapEntryExpr_Value(entry);
  ASSERT_THAT(entry_value, NotNull());
  ASSERT_EQ(cel_Expr_Kind(entry_value), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(entry_value), 4);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(entry_value)),
      cel_StringView_From("bar")));

  entry = cel_MapExpr_Entry(map_expr, 1);
  ASSERT_THAT(entry, NotNull());
  EXPECT_EQ(cel_Expr_SourcePosition(cel_Expr_UpCast(entry)), 5);

  entry_key = cel_MapEntryExpr_Key(entry);
  ASSERT_THAT(entry_key, NotNull());
  ASSERT_EQ(cel_Expr_Kind(entry_key), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(entry_key), 6);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(entry_key)),
      cel_StringView_From("baz")));
  EXPECT_TRUE(cel_MapEntryExpr_Optional(entry));

  entry_value = cel_MapEntryExpr_Value(entry);
  ASSERT_THAT(entry_value, NotNull());
  ASSERT_EQ(cel_Expr_Kind(entry_value), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(entry_value), 7);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(entry_value)),
      cel_StringView_From("bux")));
}

TEST_F(AstFromProtoTest, Struct) {
  AstPtr ast(cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                         source_info: {
                                           positions { key: 1 value: 1 }
                                           positions { key: 2 value: 2 }
                                           positions { key: 3 value: 3 }
                                           positions { key: 4 value: 4 }
                                           positions { key: 5 value: 5 }
                                         }
                                         expr: {
                                           id: 1
                                           struct_expr: {
                                             message_name: "Message"
                                             entries {
                                               id: 2
                                               field_key: "foo"
                                               value: {
                                                 id: 3
                                                 ident_expr: { name: "bar" }
                                               }
                                             }
                                             entries {
                                               id: 4
                                               field_key: "baz"
                                               value: {
                                                 id: 5
                                                 ident_expr: { name: "bux" }
                                               }
                                               optional_entry: true
                                             }
                                           }
                                         }
                                       )pb"),
                                       arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 5);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kStruct);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_StructExpr* struct_expr = cel_StructExpr_DownCast(expr);
  ASSERT_THAT(struct_expr, NotNull());
  ASSERT_EQ(cel_StructExpr_Fields(struct_expr, cel_nullptr, cel_nullptr), 2);
  EXPECT_TRUE(cel_StringView_Equals(cel_StructExpr_Name(struct_expr),
                                    cel_StringView_From("Message")));

  cel_StructFieldExpr* field = cel_StructExpr_Field(struct_expr, 0);
  ASSERT_THAT(field, NotNull());
  EXPECT_EQ(cel_Expr_SourcePosition(cel_Expr_UpCast(field)), 2);
  EXPECT_TRUE(cel_StringView_Equals(cel_StructFieldExpr_Name(field),
                                    cel_StringView_From("foo")));
  EXPECT_FALSE(cel_StructFieldExpr_Optional(field));

  cel_Expr* field_value = cel_StructFieldExpr_Value(field);
  ASSERT_THAT(field_value, NotNull());
  ASSERT_EQ(cel_Expr_Kind(field_value), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(field_value), 3);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(field_value)),
      cel_StringView_From("bar")));

  field = cel_StructExpr_Field(struct_expr, 1);
  ASSERT_THAT(field, NotNull());
  EXPECT_EQ(cel_Expr_SourcePosition(cel_Expr_UpCast(field)), 4);
  EXPECT_TRUE(cel_StringView_Equals(cel_StructFieldExpr_Name(field),
                                    cel_StringView_From("baz")));
  EXPECT_TRUE(cel_StructFieldExpr_Optional(field));

  field_value = cel_StructFieldExpr_Value(field);
  ASSERT_THAT(field_value, NotNull());
  ASSERT_EQ(cel_Expr_Kind(field_value), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(field_value), 5);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(field_value)),
      cel_StringView_From("bux")));
}

TEST_F(AstFromProtoTest, Comprehension) {
  AstPtr ast(
      cel_Ast_FromProtoV1Alpha1(ParseTextProto(R"pb(
                                  source_info: {
                                    positions { key: 1 value: 1 }
                                    positions { key: 2 value: 2 }
                                    positions { key: 3 value: 3 }
                                    positions { key: 4 value: 4 }
                                    positions { key: 5 value: 5 }
                                    positions { key: 6 value: 6 }
                                  }
                                  expr: {
                                    id: 1
                                    comprehension_expr: {
                                      iter_range: {
                                        id: 2
                                        ident_expr: { name: "foo" }
                                      }
                                      iter_var: "a"
                                      iter_var2: "b"
                                      accu_var: "__result__"
                                      accu_init: {
                                        id: 3
                                        ident_expr: { name: "bar" }
                                      }
                                      loop_condition: {
                                        id: 4
                                        ident_expr: { name: "a" }
                                      }
                                      loop_step: {
                                        id: 5
                                        ident_expr: { name: "b" }
                                      }
                                      result: {
                                        id: 6
                                        ident_expr: { name: "__result__" }
                                      }
                                    }
                                  }
                                )pb"),
                                arena(), status()));
  ASSERT_THAT(ast, NotNull());
  ASSERT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 6);

  cel_Expr* expr = cel_Ast_Expr(ast.get());
  ASSERT_THAT(expr, NotNull());
  ASSERT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kComprehension);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);

  cel_ComprehensionExpr* comprehension_expr =
      cel_ComprehensionExpr_DownCast(expr);
  ASSERT_THAT(comprehension_expr, NotNull());
  EXPECT_TRUE(
      cel_StringView_Equals(cel_ComprehensionExpr_IterVar(comprehension_expr),
                            cel_StringView_From("a")));
  EXPECT_TRUE(
      cel_StringView_Equals(cel_ComprehensionExpr_IterVar2(comprehension_expr),
                            cel_StringView_From("b")));
  EXPECT_TRUE(
      cel_StringView_Equals(cel_ComprehensionExpr_AccuVar(comprehension_expr),
                            cel_StringView_From("__result__")));

  cel_Expr* iter_range = cel_ComprehensionExpr_IterRange(comprehension_expr);
  ASSERT_THAT(iter_range, NotNull());
  ASSERT_EQ(cel_Expr_Kind(iter_range), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(iter_range), 2);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(iter_range)),
      cel_StringView_From("foo")));

  cel_Expr* accu_init = cel_ComprehensionExpr_AccuInit(comprehension_expr);
  ASSERT_THAT(accu_init, NotNull());
  ASSERT_EQ(cel_Expr_Kind(accu_init), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(accu_init), 3);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(accu_init)),
      cel_StringView_From("bar")));

  cel_Expr* loop_condition =
      cel_ComprehensionExpr_LoopCondition(comprehension_expr);
  ASSERT_THAT(loop_condition, NotNull());
  ASSERT_EQ(cel_Expr_Kind(loop_condition), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(loop_condition), 4);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(loop_condition)),
      cel_StringView_From("a")));

  cel_Expr* loop_step = cel_ComprehensionExpr_LoopStep(comprehension_expr);
  ASSERT_THAT(loop_step, NotNull());
  ASSERT_EQ(cel_Expr_Kind(loop_step), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(loop_step), 5);
  EXPECT_TRUE(cel_StringView_Equals(
      cel_IdentExpr_Name(cel_IdentExpr_DownCast(loop_step)),
      cel_StringView_From("b")));

  cel_Expr* result = cel_ComprehensionExpr_Result(comprehension_expr);
  ASSERT_THAT(result, NotNull());
  ASSERT_EQ(cel_Expr_Kind(result), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_Expr_SourcePosition(result), 6);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_IdentExpr_Name(cel_IdentExpr_DownCast(result)),
                            cel_StringView_From("__result__")));
}

}  // namespace
