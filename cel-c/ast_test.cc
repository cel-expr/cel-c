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

#include "cel-c/ast.h"

#include <memory>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/config.h"
#include "cel-c/operators.h"
#include "cel-c/source.h"
#include "cel-c/string_view.h"

namespace {

using ::testing::_;
using ::testing::IsNull;
using ::testing::NotNull;

struct AstDeleter {
  void operator()(CEL_NULLABLE(cel_Ast*) ast) const { cel_Ast_Delete(ast); }
};

using AstPtr = std::unique_ptr<cel_Ast, AstDeleter>;

class AstTest : public ::testing::Test {
 public:
  void SetUp() override { arena_ = ABSL_DIE_IF_NULL(cel_Arena_New(alloc())); }

  void TearDown() override {
    cel_Arena_Delete(arena_);
    arena_ = nullptr;
  }

 protected:
  CEL_NONNULL(cel_Allocator*) alloc() { return cel_DefaultAllocator; }

  CEL_NONNULL(cel_Arena*) arena() { return ABSL_DIE_IF_NULL(arena_); }

 private:
  CEL_NULLABILITY_UNKNOWN(cel_Arena*) arena_ = nullptr;
};

TEST_F(AstTest, Id) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 0);
  EXPECT_EQ(cel_Ast_NextId(ast.get()), 1);
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 1);
}

TEST_F(AstTest, Expr) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());
  EXPECT_THAT(cel_Ast_Expr(ast.get()), IsNull());
  EXPECT_THAT(cel_Ast_SetExpr(ast.get(), nullptr), IsNull());
  EXPECT_THAT(cel_Ast_Expr(ast.get()), IsNull());
  EXPECT_THAT(cel_Ast_ReleaseExpr(ast.get()), IsNull());
  EXPECT_THAT(cel_Ast_Expr(ast.get()), IsNull());

  cel_Expr* expr = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(expr), cel_ExprKind_kUnspecified);
  EXPECT_EQ(cel_Expr_Id(expr), 0);
  cel_Expr_SetId(expr, 1);
  EXPECT_EQ(cel_Expr_Id(expr), 1);
  EXPECT_EQ(cel_Ast_MaxId(ast.get()), 1);
  EXPECT_EQ(cel_Expr_SourcePosition(expr), -1);
  EXPECT_TRUE(cel_SourceRange_Equals(cel_Expr_SourceRange(expr),
                                     cel_SourceRange(-1, -1)));
  cel_Expr_SetSourceRange(expr, cel_SourceRange(1, 2));
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 1);
  EXPECT_TRUE(cel_SourceRange_Equals(cel_Expr_SourceRange(expr),
                                     cel_SourceRange(1, 2)));
  cel_Expr_SetSourcePosition(expr, 2);
  EXPECT_THAT(cel_Ast_SetExpr(ast.get(), expr), IsNull());
  EXPECT_EQ(cel_Expr_SourcePosition(expr), 2);
  EXPECT_TRUE(cel_SourceRange_Equals(cel_Expr_SourceRange(expr),
                                     cel_SourceRange(2, -1)));
  EXPECT_EQ(cel_Ast_Expr(ast.get()), expr);
  EXPECT_EQ(cel_Ast_ReleaseExpr(ast.get()), expr);
  EXPECT_THAT(cel_Expr_Parent(expr), IsNull());
  EXPECT_EQ(cel_Expr_Depth(expr), 0);
}

TEST_F(AstTest, Unspecified) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_UnspecifiedExpr* expr = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kUnspecified);
  EXPECT_EQ(cel_UnspecifiedExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(cel_UnspecifiedExpr_DownCast(
                cel_Expr_UpCast((const cel_UnspecifiedExpr*)expr)),
            expr);
}

TEST_F(AstTest, Ident) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_IdentExpr* expr = cel_IdentExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kIdent);
  EXPECT_EQ(cel_IdentExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(cel_IdentExpr_DownCast(cel_Expr_UpCast((const cel_IdentExpr*)expr)),
            expr);

  EXPECT_TRUE(
      cel_StringView_Equals(cel_IdentExpr_Name(expr), cel_StringView_From("")));
  cel_IdentExpr_SetName(expr, cel_StringView_From("foo"));
  EXPECT_TRUE(cel_StringView_Equals(cel_IdentExpr_Name(expr),
                                    cel_StringView_From("foo")));
}

TEST_F(AstTest, Const) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_ConstExpr* expr = cel_ConstExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kConst);
  EXPECT_EQ(cel_ConstExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(cel_ConstExpr_DownCast(cel_Expr_UpCast((const cel_ConstExpr*)expr)),
            expr);

  EXPECT_EQ(cel_ConstExpr_MutableValue(expr), cel_ConstExpr_Value(expr));
}

TEST_F(AstTest, Select) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_SelectExpr* expr = cel_SelectExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kSelect);
  EXPECT_EQ(cel_SelectExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(
      cel_SelectExpr_DownCast(cel_Expr_UpCast((const cel_SelectExpr*)expr)),
      expr);

  EXPECT_THAT(cel_SelectExpr_Operand(expr), IsNull());
  EXPECT_THAT(cel_SelectExpr_SetOperand(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_SelectExpr_Operand(expr), IsNull());
  EXPECT_THAT(cel_SelectExpr_ReleaseOperand(expr), IsNull());
  EXPECT_THAT(cel_SelectExpr_Operand(expr), IsNull());

  cel_Expr* operand = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(operand, NotNull());
  EXPECT_THAT(cel_SelectExpr_SetOperand(expr, operand), IsNull());
  EXPECT_EQ(cel_SelectExpr_Operand(expr), operand);
  EXPECT_EQ(cel_Expr_Parent(operand), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(operand), 1);
  EXPECT_EQ(cel_SelectExpr_ReleaseOperand(expr), operand);
  EXPECT_THAT(cel_SelectExpr_Operand(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(operand), IsNull());
  EXPECT_EQ(cel_Expr_Depth(operand), 0);

  EXPECT_TRUE(cel_StringView_Equals(cel_SelectExpr_Field(expr),
                                    cel_StringView_From("")));
  cel_SelectExpr_SetField(expr, cel_StringView_From("foo"));
  EXPECT_TRUE(cel_StringView_Equals(cel_SelectExpr_Field(expr),
                                    cel_StringView_From("foo")));

  EXPECT_FALSE(cel_SelectExpr_TestOnly(expr));
  cel_SelectExpr_SetTestOnly(expr, true);
  EXPECT_TRUE(cel_SelectExpr_TestOnly(expr));
}

TEST_F(AstTest, CallArg) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_CallArgExpr* expr = cel_CallArgExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kCallArg);
  EXPECT_EQ(cel_CallArgExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(
      cel_CallArgExpr_DownCast(cel_Expr_UpCast((const cel_CallArgExpr*)expr)),
      expr);
  EXPECT_EQ(cel_CallArgExpr_Index(expr), -1);

  EXPECT_THAT(cel_CallArgExpr_Value(expr), IsNull());
  EXPECT_THAT(cel_CallArgExpr_SetValue(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_CallArgExpr_Value(expr), IsNull());
  EXPECT_THAT(cel_CallArgExpr_ReleaseValue(expr), IsNull());
  EXPECT_THAT(cel_CallArgExpr_Value(expr), IsNull());

  cel_Expr* value = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(value, NotNull());
  EXPECT_THAT(cel_CallArgExpr_SetValue(expr, value), IsNull());
  EXPECT_EQ(cel_CallArgExpr_Value(expr), value);
  EXPECT_EQ(cel_Expr_Parent(value), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(value), 1);
  EXPECT_EQ(cel_CallArgExpr_ReleaseValue(expr), value);
  EXPECT_THAT(cel_CallArgExpr_Value(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(value), IsNull());
  EXPECT_EQ(cel_Expr_Depth(value), 0);
}

TEST_F(AstTest, Call) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_CallExpr* expr = cel_CallExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kCall);
  EXPECT_EQ(cel_CallExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(cel_CallExpr_DownCast(cel_Expr_UpCast((const cel_CallExpr*)expr)),
            expr);

  EXPECT_THAT(cel_CallExpr_Target(expr), IsNull());
  EXPECT_THAT(cel_CallExpr_SetTarget(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_CallExpr_Target(expr), IsNull());
  EXPECT_THAT(cel_CallExpr_ReleaseTarget(expr), IsNull());
  EXPECT_THAT(cel_CallExpr_Target(expr), IsNull());

  cel_Expr* target = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(target, NotNull());
  EXPECT_THAT(cel_CallExpr_SetTarget(expr, target), IsNull());
  EXPECT_EQ(cel_CallExpr_Target(expr), target);
  EXPECT_EQ(cel_Expr_Parent(target), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(target), 1);
  EXPECT_EQ(cel_CallExpr_ReleaseTarget(expr), target);
  EXPECT_THAT(cel_CallExpr_Target(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(target), IsNull());
  EXPECT_EQ(cel_Expr_Depth(target), 0);

  EXPECT_TRUE(cel_StringView_Equals(cel_CallExpr_Function(expr),
                                    cel_StringView_From("")));
  cel_CallExpr_SetFunction(expr, cel_StringView_From("foo"));
  EXPECT_TRUE(cel_StringView_Equals(cel_CallExpr_Function(expr),
                                    cel_StringView_From("foo")));

  cel_CallArgExpr* args_head;
  cel_CallArgExpr* args_tail;
  EXPECT_EQ(cel_CallExpr_Args(expr, &args_head, &args_tail), 0);
  EXPECT_THAT(args_head, IsNull());
  EXPECT_THAT(args_tail, IsNull());
  EXPECT_THAT(cel_CallExpr_PrevArg(cel_nullptr), IsNull());
  EXPECT_THAT(cel_CallExpr_NextArg(cel_nullptr), IsNull());

  cel_CallArgExpr* arg0 = cel_CallArgExpr_New(ast.get());
  ASSERT_THAT(arg0, NotNull());

  // Prepend
  cel_CallExpr_PrependArg(expr, arg0);
  EXPECT_EQ(cel_CallArgExpr_Parent(arg0), expr);
  EXPECT_EQ(cel_CallArgExpr_Index(arg0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(arg0)), 1);
  EXPECT_EQ(cel_CallExpr_Args(expr, &args_head, &args_tail), 1);
  EXPECT_EQ(args_head, arg0);
  EXPECT_EQ(args_tail, arg0);
  EXPECT_THAT(cel_CallExpr_PrevArg(arg0), IsNull());
  EXPECT_THAT(cel_CallExpr_NextArg(arg0), IsNull());
  EXPECT_EQ(cel_CallExpr_Arg(expr, 0), arg0);
  EXPECT_EQ(cel_CallExpr_ReleaseArg(expr, arg0), arg0);
  EXPECT_THAT(cel_CallArgExpr_Parent(arg0), IsNull());
  EXPECT_EQ(cel_CallArgExpr_Index(arg0), -1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(arg0)), 0);
  EXPECT_EQ(cel_CallExpr_Args(expr, &args_head, &args_tail), 0);
  EXPECT_THAT(args_head, IsNull());
  EXPECT_THAT(args_tail, IsNull());
  EXPECT_THAT(cel_CallExpr_PrevArg(cel_nullptr), IsNull());
  EXPECT_THAT(cel_CallExpr_NextArg(cel_nullptr), IsNull());

  // Append
  cel_CallExpr_AppendArg(expr, arg0);
  EXPECT_EQ(cel_CallArgExpr_Parent(arg0), expr);
  EXPECT_EQ(cel_CallArgExpr_Index(arg0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(arg0)), 1);
  EXPECT_EQ(cel_CallExpr_Args(expr, &args_head, &args_tail), 1);
  EXPECT_EQ(args_head, arg0);
  EXPECT_EQ(args_tail, arg0);
  EXPECT_THAT(cel_CallExpr_PrevArg(arg0), IsNull());
  EXPECT_THAT(cel_CallExpr_NextArg(arg0), IsNull());
  EXPECT_EQ(cel_CallExpr_Arg(expr, 0), arg0);
  EXPECT_EQ(cel_CallExpr_ReleaseArg(expr, arg0), arg0);
  EXPECT_THAT(cel_CallArgExpr_Parent(arg0), IsNull());
  EXPECT_EQ(cel_CallArgExpr_Index(arg0), -1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(arg0)), 0);
  EXPECT_EQ(cel_CallExpr_Args(expr, &args_head, &args_tail), 0);
  EXPECT_THAT(args_head, IsNull());
  EXPECT_THAT(args_tail, IsNull());
  EXPECT_THAT(cel_CallExpr_PrevArg(cel_nullptr), IsNull());
  EXPECT_THAT(cel_CallExpr_NextArg(cel_nullptr), IsNull());

  cel_CallArgExpr* arg1 = cel_CallArgExpr_New(ast.get());
  ASSERT_THAT(arg1, NotNull());

  // Prepend
  cel_CallExpr_PrependArg(expr, arg1);
  cel_CallExpr_PrependArg(expr, arg0);
  EXPECT_EQ(cel_CallArgExpr_Parent(arg0), expr);
  EXPECT_EQ(cel_CallArgExpr_Index(arg0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(arg0)), 1);
  EXPECT_EQ(cel_CallArgExpr_Parent(arg1), expr);
  EXPECT_EQ(cel_CallArgExpr_Index(arg1), 1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(arg1)), 1);
  EXPECT_EQ(cel_CallExpr_Args(expr, &args_head, &args_tail), 2);
  EXPECT_EQ(args_head, arg0);
  EXPECT_EQ(args_tail, arg1);
  EXPECT_EQ(cel_CallExpr_Arg(expr, 0), arg0);
  EXPECT_EQ(cel_CallExpr_Arg(expr, 1), arg1);
  EXPECT_THAT(cel_CallExpr_PrevArg(arg0), IsNull());
  EXPECT_EQ(cel_CallExpr_PrevArg(arg1), arg0);
  EXPECT_THAT(cel_CallExpr_NextArg(arg1), IsNull());
  EXPECT_EQ(cel_CallExpr_NextArg(arg0), arg1);
  EXPECT_EQ(cel_CallExpr_ReleaseArg(expr, arg0), arg0);
  EXPECT_EQ(cel_CallExpr_ReleaseArg(expr, arg1), arg1);
  EXPECT_EQ(cel_CallExpr_Args(expr, &args_head, &args_tail), 0);
  EXPECT_THAT(args_head, IsNull());
  EXPECT_THAT(args_tail, IsNull());

  // Append
  cel_CallExpr_AppendArg(expr, arg0);
  cel_CallExpr_AppendArg(expr, arg1);
  EXPECT_EQ(cel_CallArgExpr_Parent(arg0), expr);
  EXPECT_EQ(cel_CallArgExpr_Index(arg0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(arg0)), 1);
  EXPECT_EQ(cel_CallArgExpr_Parent(arg1), expr);
  EXPECT_EQ(cel_CallArgExpr_Index(arg1), 1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(arg1)), 1);
  EXPECT_EQ(cel_CallExpr_Args(expr, &args_head, &args_tail), 2);
  EXPECT_EQ(args_head, arg0);
  EXPECT_EQ(args_tail, arg1);
  EXPECT_EQ(cel_CallExpr_Arg(expr, 0), arg0);
  EXPECT_EQ(cel_CallExpr_Arg(expr, 1), arg1);
  EXPECT_THAT(cel_CallExpr_PrevArg(arg0), IsNull());
  EXPECT_EQ(cel_CallExpr_PrevArg(arg1), arg0);
  EXPECT_THAT(cel_CallExpr_NextArg(arg1), IsNull());
  EXPECT_EQ(cel_CallExpr_NextArg(arg0), arg1);
  EXPECT_EQ(cel_CallExpr_ReleaseArg(expr, arg1), arg1);
  EXPECT_EQ(cel_CallExpr_ReleaseArg(expr, arg0), arg0);
  EXPECT_EQ(cel_CallExpr_Args(expr, &args_head, &args_tail), 0);
  EXPECT_THAT(args_head, IsNull());
  EXPECT_THAT(args_tail, IsNull());

  cel_CallArgExpr* arg2 = cel_CallArgExpr_New(ast.get());
  ASSERT_THAT(arg2, NotNull());
  cel_CallExpr_AppendArg(expr, arg0);
  cel_CallExpr_AppendArg(expr, arg2);
  cel_CallExpr_InsertArg(expr, arg2, arg1);
  EXPECT_EQ(cel_CallArgExpr_Parent(arg0), expr);
  EXPECT_EQ(cel_CallArgExpr_Index(arg0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(arg0)), 1);
  EXPECT_EQ(cel_CallArgExpr_Parent(arg1), expr);
  EXPECT_EQ(cel_CallArgExpr_Index(arg1), 1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(arg1)), 1);
  EXPECT_EQ(cel_CallArgExpr_Parent(arg2), expr);
  EXPECT_EQ(cel_CallArgExpr_Index(arg2), 2);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(arg2)), 1);
  EXPECT_EQ(cel_CallExpr_Args(expr, &args_head, &args_tail), 3);
  EXPECT_EQ(args_head, arg0);
  EXPECT_EQ(args_tail, arg2);
  EXPECT_EQ(cel_CallExpr_Arg(expr, 0), arg0);
  EXPECT_EQ(cel_CallExpr_Arg(expr, 1), arg1);
  EXPECT_EQ(cel_CallExpr_Arg(expr, 2), arg2);
  EXPECT_THAT(cel_CallExpr_PrevArg(arg0), IsNull());
  EXPECT_EQ(cel_CallExpr_PrevArg(arg1), arg0);
  EXPECT_EQ(cel_CallExpr_PrevArg(arg2), arg1);
  EXPECT_THAT(cel_CallExpr_NextArg(arg2), IsNull());
  EXPECT_EQ(cel_CallExpr_NextArg(arg1), arg2);
  EXPECT_EQ(cel_CallExpr_NextArg(arg0), arg1);
}

TEST_F(AstTest, Unary) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_UnaryExpr* expr = cel_UnaryExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kUnary);
  EXPECT_EQ(cel_UnaryExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(cel_UnaryExpr_DownCast(cel_Expr_UpCast((const cel_UnaryExpr*)expr)),
            expr);

  EXPECT_EQ(cel_UnaryExpr_Op(expr), cel_UnaryOp_kUnspecified);
  cel_UnaryExpr_SetOp(expr, cel_UnaryOp_kLogicalNot);
  EXPECT_EQ(cel_UnaryExpr_Op(expr), cel_UnaryOp_kLogicalNot);

  EXPECT_THAT(cel_UnaryExpr_Arg(expr), IsNull());
  EXPECT_THAT(cel_UnaryExpr_SetArg(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_UnaryExpr_Arg(expr), IsNull());
  EXPECT_THAT(cel_UnaryExpr_ReleaseArg(expr), IsNull());
  EXPECT_THAT(cel_UnaryExpr_Arg(expr), IsNull());

  cel_Expr* arg = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(arg, NotNull());
  EXPECT_THAT(cel_UnaryExpr_SetArg(expr, arg), IsNull());
  EXPECT_EQ(cel_UnaryExpr_Arg(expr), arg);
  EXPECT_EQ(cel_Expr_Parent(arg), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(arg), 1);
  EXPECT_EQ(cel_UnaryExpr_ReleaseArg(expr), arg);
  EXPECT_THAT(cel_UnaryExpr_Arg(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(arg), IsNull());
  EXPECT_EQ(cel_Expr_Depth(arg), 0);
}

TEST_F(AstTest, Binary) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_BinaryExpr* expr = cel_BinaryExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kBinary);
  EXPECT_EQ(cel_BinaryExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(
      cel_BinaryExpr_DownCast(cel_Expr_UpCast((const cel_BinaryExpr*)expr)),
      expr);

  EXPECT_EQ(cel_BinaryExpr_Op(expr), cel_BinaryOp_kUnspecified);
  cel_BinaryExpr_SetOp(expr, cel_BinaryOp_kAdd);
  EXPECT_EQ(cel_BinaryExpr_Op(expr), cel_BinaryOp_kAdd);

  EXPECT_THAT(cel_BinaryExpr_Left(expr), IsNull());
  EXPECT_THAT(cel_BinaryExpr_SetLeft(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_BinaryExpr_Left(expr), IsNull());
  EXPECT_THAT(cel_BinaryExpr_ReleaseLeft(expr), IsNull());
  EXPECT_THAT(cel_BinaryExpr_Left(expr), IsNull());

  cel_Expr* left = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(left, NotNull());
  EXPECT_THAT(cel_BinaryExpr_SetLeft(expr, left), IsNull());
  EXPECT_EQ(cel_BinaryExpr_Left(expr), left);
  EXPECT_EQ(cel_Expr_Parent(left), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(left), 1);
  EXPECT_EQ(cel_BinaryExpr_ReleaseLeft(expr), left);
  EXPECT_THAT(cel_BinaryExpr_Left(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(left), IsNull());
  EXPECT_EQ(cel_Expr_Depth(left), 0);

  EXPECT_THAT(cel_BinaryExpr_Right(expr), IsNull());
  EXPECT_THAT(cel_BinaryExpr_SetRight(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_BinaryExpr_Right(expr), IsNull());
  EXPECT_THAT(cel_BinaryExpr_ReleaseRight(expr), IsNull());
  EXPECT_THAT(cel_BinaryExpr_Right(expr), IsNull());

  cel_Expr* right = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(right, NotNull());
  EXPECT_THAT(cel_BinaryExpr_SetRight(expr, right), IsNull());
  EXPECT_EQ(cel_BinaryExpr_Right(expr), right);
  EXPECT_EQ(cel_Expr_Parent(right), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(right), 1);
  EXPECT_EQ(cel_BinaryExpr_ReleaseRight(expr), right);
  EXPECT_THAT(cel_BinaryExpr_Right(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(right), IsNull());
  EXPECT_EQ(cel_Expr_Depth(right), 0);
}

TEST_F(AstTest, Ternary) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_TernaryExpr* expr = cel_TernaryExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kTernary);
  EXPECT_EQ(cel_TernaryExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(
      cel_TernaryExpr_DownCast(cel_Expr_UpCast((const cel_TernaryExpr*)expr)),
      expr);

  EXPECT_EQ(cel_TernaryExpr_Op(expr), cel_TernaryOp_kUnspecified);
  cel_TernaryExpr_SetOp(expr, cel_TernaryOp_kConditional);
  EXPECT_EQ(cel_TernaryExpr_Op(expr), cel_TernaryOp_kConditional);

  EXPECT_THAT(cel_TernaryExpr_Condition(expr), IsNull());
  EXPECT_THAT(cel_TernaryExpr_SetCondition(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_TernaryExpr_Condition(expr), IsNull());
  EXPECT_THAT(cel_TernaryExpr_ReleaseCondition(expr), IsNull());
  EXPECT_THAT(cel_TernaryExpr_Condition(expr), IsNull());

  cel_Expr* condition = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(condition, NotNull());
  EXPECT_THAT(cel_TernaryExpr_SetCondition(expr, condition), IsNull());
  EXPECT_EQ(cel_TernaryExpr_Condition(expr), condition);
  EXPECT_EQ(cel_Expr_Parent(condition), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(condition), 1);
  EXPECT_EQ(cel_TernaryExpr_ReleaseCondition(expr), condition);
  EXPECT_THAT(cel_TernaryExpr_Condition(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(condition), IsNull());
  EXPECT_EQ(cel_Expr_Depth(condition), 0);

  EXPECT_THAT(cel_TernaryExpr_IfTrue(expr), IsNull());
  EXPECT_THAT(cel_TernaryExpr_SetIfTrue(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_TernaryExpr_IfTrue(expr), IsNull());
  EXPECT_THAT(cel_TernaryExpr_ReleaseIfTrue(expr), IsNull());
  EXPECT_THAT(cel_TernaryExpr_IfTrue(expr), IsNull());

  cel_Expr* if_true = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(if_true, NotNull());
  EXPECT_THAT(cel_TernaryExpr_SetIfTrue(expr, if_true), IsNull());
  EXPECT_EQ(cel_TernaryExpr_IfTrue(expr), if_true);
  EXPECT_EQ(cel_Expr_Parent(if_true), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(if_true), 1);
  EXPECT_EQ(cel_TernaryExpr_ReleaseIfTrue(expr), if_true);
  EXPECT_THAT(cel_TernaryExpr_IfTrue(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(if_true), IsNull());
  EXPECT_EQ(cel_Expr_Depth(if_true), 0);

  EXPECT_THAT(cel_TernaryExpr_IfFalse(expr), IsNull());
  EXPECT_THAT(cel_TernaryExpr_SetIfFalse(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_TernaryExpr_IfFalse(expr), IsNull());
  EXPECT_THAT(cel_TernaryExpr_ReleaseIfFalse(expr), IsNull());
  EXPECT_THAT(cel_TernaryExpr_IfFalse(expr), IsNull());

  cel_Expr* if_false = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(if_false, NotNull());
  EXPECT_THAT(cel_TernaryExpr_SetIfFalse(expr, if_false), IsNull());
  EXPECT_EQ(cel_TernaryExpr_IfFalse(expr), if_false);
  EXPECT_EQ(cel_Expr_Parent(if_false), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(if_false), 1);
  EXPECT_EQ(cel_TernaryExpr_ReleaseIfFalse(expr), if_false);
  EXPECT_THAT(cel_TernaryExpr_IfFalse(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(if_false), IsNull());
  EXPECT_EQ(cel_Expr_Depth(if_false), 0);
}

TEST_F(AstTest, ListElement) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_ListElementExpr* expr = cel_ListElementExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kListElement);
  EXPECT_EQ(cel_ListElementExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(cel_ListElementExpr_DownCast(
                cel_Expr_UpCast((const cel_ListElementExpr*)expr)),
            expr);
  EXPECT_EQ(cel_ListElementExpr_Index(expr), -1);

  EXPECT_THAT(cel_ListElementExpr_Value(expr), IsNull());
  EXPECT_THAT(cel_ListElementExpr_SetValue(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_ListElementExpr_Value(expr), IsNull());
  EXPECT_THAT(cel_ListElementExpr_ReleaseValue(expr), IsNull());
  EXPECT_THAT(cel_ListElementExpr_Value(expr), IsNull());

  cel_Expr* value = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(value, NotNull());
  EXPECT_THAT(cel_ListElementExpr_SetValue(expr, value), IsNull());
  EXPECT_EQ(cel_ListElementExpr_Value(expr), value);
  EXPECT_EQ(cel_Expr_Parent(value), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(value), 1);
  EXPECT_EQ(cel_ListElementExpr_ReleaseValue(expr), value);
  EXPECT_THAT(cel_ListElementExpr_Value(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(value), IsNull());
  EXPECT_EQ(cel_Expr_Depth(value), 0);

  EXPECT_FALSE(cel_ListElementExpr_Optional(expr));
  cel_ListElementExpr_SetOptional(expr, true);
  EXPECT_TRUE(cel_ListElementExpr_Optional(expr));
}

TEST_F(AstTest, List) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_ListExpr* expr = cel_ListExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kList);
  EXPECT_EQ(cel_ListExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(cel_ListExpr_DownCast(cel_Expr_UpCast((const cel_ListExpr*)expr)),
            expr);

  cel_ListElementExpr* elements_head;
  cel_ListElementExpr* elements_tail;
  EXPECT_EQ(cel_ListExpr_Elements(expr, &elements_head, &elements_tail), 0);
  EXPECT_THAT(elements_head, IsNull());
  EXPECT_THAT(elements_tail, IsNull());
  EXPECT_THAT(cel_ListExpr_PrevElement(cel_nullptr), IsNull());
  EXPECT_THAT(cel_ListExpr_NextElement(cel_nullptr), IsNull());

  cel_ListElementExpr* element0 = cel_ListElementExpr_New(ast.get());
  ASSERT_THAT(element0, NotNull());

  // Prepend
  cel_ListExpr_PrependElement(expr, element0);
  EXPECT_EQ(cel_ListElementExpr_Parent(element0), expr);
  EXPECT_EQ(cel_ListElementExpr_Index(element0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(element0)), 1);
  EXPECT_EQ(cel_ListExpr_Elements(expr, &elements_head, &elements_tail), 1);
  EXPECT_EQ(elements_head, element0);
  EXPECT_EQ(elements_tail, element0);
  EXPECT_THAT(cel_ListExpr_PrevElement(element0), IsNull());
  EXPECT_THAT(cel_ListExpr_NextElement(element0), IsNull());
  EXPECT_EQ(cel_ListExpr_Element(expr, 0), element0);
  EXPECT_EQ(cel_ListExpr_ReleaseElement(expr, element0), element0);
  EXPECT_THAT(cel_ListElementExpr_Parent(element0), IsNull());
  EXPECT_EQ(cel_ListElementExpr_Index(element0), -1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(element0)), 0);
  EXPECT_EQ(cel_ListExpr_Elements(expr, &elements_head, &elements_tail), 0);
  EXPECT_THAT(elements_head, IsNull());
  EXPECT_THAT(elements_tail, IsNull());
  EXPECT_THAT(cel_ListExpr_PrevElement(cel_nullptr), IsNull());
  EXPECT_THAT(cel_ListExpr_NextElement(cel_nullptr), IsNull());

  // Append
  cel_ListExpr_AppendElement(expr, element0);
  EXPECT_EQ(cel_ListElementExpr_Parent(element0), expr);
  EXPECT_EQ(cel_ListElementExpr_Index(element0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(element0)), 1);
  EXPECT_EQ(cel_ListExpr_Elements(expr, &elements_head, &elements_tail), 1);
  EXPECT_EQ(elements_head, element0);
  EXPECT_EQ(elements_tail, element0);
  EXPECT_THAT(cel_ListExpr_PrevElement(element0), IsNull());
  EXPECT_THAT(cel_ListExpr_NextElement(element0), IsNull());
  EXPECT_EQ(cel_ListExpr_Element(expr, 0), element0);
  EXPECT_EQ(cel_ListExpr_ReleaseElement(expr, element0), element0);
  EXPECT_THAT(cel_ListElementExpr_Parent(element0), IsNull());
  EXPECT_EQ(cel_ListElementExpr_Index(element0), -1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(element0)), 0);
  EXPECT_EQ(cel_ListExpr_Elements(expr, &elements_head, &elements_tail), 0);
  EXPECT_THAT(elements_head, IsNull());
  EXPECT_THAT(elements_tail, IsNull());
  EXPECT_THAT(cel_ListExpr_PrevElement(cel_nullptr), IsNull());
  EXPECT_THAT(cel_ListExpr_NextElement(cel_nullptr), IsNull());

  cel_ListElementExpr* element1 = cel_ListElementExpr_New(ast.get());
  ASSERT_THAT(element1, NotNull());

  // Prepend
  cel_ListExpr_PrependElement(expr, element1);
  cel_ListExpr_PrependElement(expr, element0);
  EXPECT_EQ(cel_ListElementExpr_Parent(element0), expr);
  EXPECT_EQ(cel_ListElementExpr_Index(element0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(element0)), 1);
  EXPECT_EQ(cel_ListElementExpr_Parent(element1), expr);
  EXPECT_EQ(cel_ListElementExpr_Index(element1), 1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(element1)), 1);
  EXPECT_EQ(cel_ListExpr_Elements(expr, &elements_head, &elements_tail), 2);
  EXPECT_EQ(elements_head, element0);
  EXPECT_EQ(elements_tail, element1);
  EXPECT_EQ(cel_ListExpr_Element(expr, 0), element0);
  EXPECT_EQ(cel_ListExpr_Element(expr, 1), element1);
  EXPECT_THAT(cel_ListExpr_PrevElement(element0), IsNull());
  EXPECT_EQ(cel_ListExpr_PrevElement(element1), element0);
  EXPECT_THAT(cel_ListExpr_NextElement(element1), IsNull());
  EXPECT_EQ(cel_ListExpr_NextElement(element0), element1);
  EXPECT_EQ(cel_ListExpr_ReleaseElement(expr, element0), element0);
  EXPECT_EQ(cel_ListExpr_ReleaseElement(expr, element1), element1);
  EXPECT_EQ(cel_ListExpr_Elements(expr, &elements_head, &elements_tail), 0);
  EXPECT_THAT(elements_head, IsNull());
  EXPECT_THAT(elements_tail, IsNull());

  // Append
  cel_ListExpr_AppendElement(expr, element0);
  cel_ListExpr_AppendElement(expr, element1);
  EXPECT_EQ(cel_ListElementExpr_Parent(element0), expr);
  EXPECT_EQ(cel_ListElementExpr_Index(element0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(element0)), 1);
  EXPECT_EQ(cel_ListElementExpr_Parent(element1), expr);
  EXPECT_EQ(cel_ListElementExpr_Index(element1), 1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(element1)), 1);
  EXPECT_EQ(cel_ListExpr_Elements(expr, &elements_head, &elements_tail), 2);
  EXPECT_EQ(elements_head, element0);
  EXPECT_EQ(elements_tail, element1);
  EXPECT_EQ(cel_ListExpr_Element(expr, 0), element0);
  EXPECT_EQ(cel_ListExpr_Element(expr, 1), element1);
  EXPECT_THAT(cel_ListExpr_PrevElement(element0), IsNull());
  EXPECT_EQ(cel_ListExpr_PrevElement(element1), element0);
  EXPECT_THAT(cel_ListExpr_NextElement(element1), IsNull());
  EXPECT_EQ(cel_ListExpr_NextElement(element0), element1);
  EXPECT_EQ(cel_ListExpr_ReleaseElement(expr, element1), element1);
  EXPECT_EQ(cel_ListExpr_ReleaseElement(expr, element0), element0);
  EXPECT_EQ(cel_ListExpr_Elements(expr, &elements_head, &elements_tail), 0);
  EXPECT_THAT(elements_head, IsNull());
  EXPECT_THAT(elements_tail, IsNull());

  cel_ListElementExpr* element2 = cel_ListElementExpr_New(ast.get());
  ASSERT_THAT(element2, NotNull());
  cel_ListExpr_AppendElement(expr, element0);
  cel_ListExpr_AppendElement(expr, element2);
  cel_ListExpr_InsertElement(expr, element2, element1);
  EXPECT_EQ(cel_ListElementExpr_Parent(element0), expr);
  EXPECT_EQ(cel_ListElementExpr_Index(element0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(element0)), 1);
  EXPECT_EQ(cel_ListElementExpr_Parent(element1), expr);
  EXPECT_EQ(cel_ListElementExpr_Index(element1), 1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(element1)), 1);
  EXPECT_EQ(cel_ListElementExpr_Parent(element2), expr);
  EXPECT_EQ(cel_ListElementExpr_Index(element2), 2);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(element2)), 1);
  EXPECT_EQ(cel_ListExpr_Elements(expr, &elements_head, &elements_tail), 3);
  EXPECT_EQ(elements_head, element0);
  EXPECT_EQ(elements_tail, element2);
  EXPECT_EQ(cel_ListExpr_Element(expr, 0), element0);
  EXPECT_EQ(cel_ListExpr_Element(expr, 1), element1);
  EXPECT_EQ(cel_ListExpr_Element(expr, 2), element2);
  EXPECT_THAT(cel_ListExpr_PrevElement(element0), IsNull());
  EXPECT_EQ(cel_ListExpr_PrevElement(element1), element0);
  EXPECT_EQ(cel_ListExpr_PrevElement(element2), element1);
  EXPECT_THAT(cel_ListExpr_NextElement(element2), IsNull());
  EXPECT_EQ(cel_ListExpr_NextElement(element1), element2);
  EXPECT_EQ(cel_ListExpr_NextElement(element0), element1);
}

TEST_F(AstTest, MapEntry) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_MapEntryExpr* expr = cel_MapEntryExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kMapEntry);
  EXPECT_EQ(cel_MapEntryExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(
      cel_MapEntryExpr_DownCast(cel_Expr_UpCast((const cel_MapEntryExpr*)expr)),
      expr);
  EXPECT_EQ(cel_MapEntryExpr_Index(expr), -1);

  EXPECT_THAT(cel_MapEntryExpr_Key(expr), IsNull());
  EXPECT_THAT(cel_MapEntryExpr_SetKey(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_MapEntryExpr_Key(expr), IsNull());
  EXPECT_THAT(cel_MapEntryExpr_ReleaseKey(expr), IsNull());
  EXPECT_THAT(cel_MapEntryExpr_Key(expr), IsNull());

  cel_Expr* key = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(key, NotNull());
  EXPECT_THAT(cel_MapEntryExpr_SetKey(expr, key), IsNull());
  EXPECT_EQ(cel_MapEntryExpr_Key(expr), key);
  EXPECT_EQ(cel_Expr_Parent(key), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(key), 1);
  EXPECT_EQ(cel_MapEntryExpr_ReleaseKey(expr), key);
  EXPECT_THAT(cel_MapEntryExpr_Key(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(key), IsNull());
  EXPECT_EQ(cel_Expr_Depth(key), 0);

  EXPECT_THAT(cel_MapEntryExpr_Value(expr), IsNull());
  EXPECT_THAT(cel_MapEntryExpr_SetValue(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_MapEntryExpr_Value(expr), IsNull());
  EXPECT_THAT(cel_MapEntryExpr_ReleaseValue(expr), IsNull());
  EXPECT_THAT(cel_MapEntryExpr_Value(expr), IsNull());

  cel_Expr* value = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(value, NotNull());
  EXPECT_THAT(cel_MapEntryExpr_SetValue(expr, value), IsNull());
  EXPECT_EQ(cel_MapEntryExpr_Value(expr), value);
  EXPECT_EQ(cel_Expr_Parent(value), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(value), 1);
  EXPECT_EQ(cel_MapEntryExpr_ReleaseValue(expr), value);
  EXPECT_THAT(cel_MapEntryExpr_Value(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(value), IsNull());
  EXPECT_EQ(cel_Expr_Depth(value), 0);

  EXPECT_FALSE(cel_MapEntryExpr_Optional(expr));
  cel_MapEntryExpr_SetOptional(expr, true);
  EXPECT_TRUE(cel_MapEntryExpr_Optional(expr));
}

TEST_F(AstTest, Map) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_MapExpr* expr = cel_MapExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kMap);
  EXPECT_EQ(cel_MapExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(cel_MapExpr_DownCast(cel_Expr_UpCast((const cel_MapExpr*)expr)),
            expr);

  cel_MapEntryExpr* entries_head;
  cel_MapEntryExpr* entries_tail;
  EXPECT_EQ(cel_MapExpr_Entries(expr, &entries_head, &entries_tail), 0);
  EXPECT_THAT(entries_head, IsNull());
  EXPECT_THAT(entries_tail, IsNull());
  EXPECT_THAT(cel_MapExpr_PrevEntry(cel_nullptr), IsNull());
  EXPECT_THAT(cel_MapExpr_NextEntry(cel_nullptr), IsNull());

  cel_MapEntryExpr* entry0 = cel_MapEntryExpr_New(ast.get());
  ASSERT_THAT(entry0, NotNull());

  // Prepend
  cel_MapExpr_PrependEntry(expr, entry0);
  EXPECT_EQ(cel_MapEntryExpr_Parent(entry0), expr);
  EXPECT_EQ(cel_MapEntryExpr_Index(entry0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(entry0)), 1);
  EXPECT_EQ(cel_MapExpr_Entries(expr, &entries_head, &entries_tail), 1);
  EXPECT_EQ(entries_head, entry0);
  EXPECT_EQ(entries_tail, entry0);
  EXPECT_THAT(cel_MapExpr_PrevEntry(entry0), IsNull());
  EXPECT_THAT(cel_MapExpr_NextEntry(entry0), IsNull());
  EXPECT_EQ(cel_MapExpr_Entry(expr, 0), entry0);
  EXPECT_EQ(cel_MapExpr_ReleaseEntry(expr, entry0), entry0);
  EXPECT_THAT(cel_MapEntryExpr_Parent(entry0), IsNull());
  EXPECT_EQ(cel_MapEntryExpr_Index(entry0), -1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(entry0)), 0);
  EXPECT_EQ(cel_MapExpr_Entries(expr, &entries_head, &entries_tail), 0);
  EXPECT_THAT(entries_head, IsNull());
  EXPECT_THAT(entries_tail, IsNull());
  EXPECT_THAT(cel_MapExpr_PrevEntry(cel_nullptr), IsNull());
  EXPECT_THAT(cel_MapExpr_NextEntry(cel_nullptr), IsNull());

  // Append
  cel_MapExpr_AppendEntry(expr, entry0);
  EXPECT_EQ(cel_MapEntryExpr_Parent(entry0), expr);
  EXPECT_EQ(cel_MapEntryExpr_Index(entry0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(entry0)), 1);
  EXPECT_EQ(cel_MapExpr_Entries(expr, &entries_head, &entries_tail), 1);
  EXPECT_EQ(entries_head, entry0);
  EXPECT_EQ(entries_tail, entry0);
  EXPECT_THAT(cel_MapExpr_PrevEntry(entry0), IsNull());
  EXPECT_THAT(cel_MapExpr_NextEntry(entry0), IsNull());
  EXPECT_EQ(cel_MapExpr_Entry(expr, 0), entry0);
  EXPECT_EQ(cel_MapExpr_ReleaseEntry(expr, entry0), entry0);
  EXPECT_THAT(cel_MapEntryExpr_Parent(entry0), IsNull());
  EXPECT_EQ(cel_MapEntryExpr_Index(entry0), -1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(entry0)), 0);
  EXPECT_EQ(cel_MapExpr_Entries(expr, &entries_head, &entries_tail), 0);
  EXPECT_THAT(entries_head, IsNull());
  EXPECT_THAT(entries_tail, IsNull());
  EXPECT_THAT(cel_MapExpr_PrevEntry(cel_nullptr), IsNull());
  EXPECT_THAT(cel_MapExpr_NextEntry(cel_nullptr), IsNull());

  cel_MapEntryExpr* entry1 = cel_MapEntryExpr_New(ast.get());
  ASSERT_THAT(entry1, NotNull());

  // Prepend
  cel_MapExpr_PrependEntry(expr, entry1);
  cel_MapExpr_PrependEntry(expr, entry0);
  EXPECT_EQ(cel_MapEntryExpr_Parent(entry0), expr);
  EXPECT_EQ(cel_MapEntryExpr_Index(entry0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(entry0)), 1);
  EXPECT_EQ(cel_MapEntryExpr_Parent(entry1), expr);
  EXPECT_EQ(cel_MapEntryExpr_Index(entry1), 1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(entry1)), 1);
  EXPECT_EQ(cel_MapExpr_Entries(expr, &entries_head, &entries_tail), 2);
  EXPECT_EQ(entries_head, entry0);
  EXPECT_EQ(entries_tail, entry1);
  EXPECT_EQ(cel_MapExpr_Entry(expr, 0), entry0);
  EXPECT_EQ(cel_MapExpr_Entry(expr, 1), entry1);
  EXPECT_THAT(cel_MapExpr_PrevEntry(entry0), IsNull());
  EXPECT_EQ(cel_MapExpr_PrevEntry(entry1), entry0);
  EXPECT_THAT(cel_MapExpr_NextEntry(entry1), IsNull());
  EXPECT_EQ(cel_MapExpr_NextEntry(entry0), entry1);
  EXPECT_EQ(cel_MapExpr_ReleaseEntry(expr, entry0), entry0);
  EXPECT_EQ(cel_MapExpr_ReleaseEntry(expr, entry1), entry1);
  EXPECT_EQ(cel_MapExpr_Entries(expr, &entries_head, &entries_tail), 0);
  EXPECT_THAT(entries_head, IsNull());
  EXPECT_THAT(entries_tail, IsNull());

  // Append
  cel_MapExpr_AppendEntry(expr, entry0);
  cel_MapExpr_AppendEntry(expr, entry1);
  EXPECT_EQ(cel_MapEntryExpr_Parent(entry0), expr);
  EXPECT_EQ(cel_MapEntryExpr_Index(entry0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(entry0)), 1);
  EXPECT_EQ(cel_MapEntryExpr_Parent(entry1), expr);
  EXPECT_EQ(cel_MapEntryExpr_Index(entry1), 1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(entry1)), 1);
  EXPECT_EQ(cel_MapExpr_Entries(expr, &entries_head, &entries_tail), 2);
  EXPECT_EQ(entries_head, entry0);
  EXPECT_EQ(entries_tail, entry1);
  EXPECT_EQ(cel_MapExpr_Entry(expr, 0), entry0);
  EXPECT_EQ(cel_MapExpr_Entry(expr, 1), entry1);
  EXPECT_THAT(cel_MapExpr_PrevEntry(entry0), IsNull());
  EXPECT_EQ(cel_MapExpr_PrevEntry(entry1), entry0);
  EXPECT_THAT(cel_MapExpr_NextEntry(entry1), IsNull());
  EXPECT_EQ(cel_MapExpr_NextEntry(entry0), entry1);
  EXPECT_EQ(cel_MapExpr_ReleaseEntry(expr, entry1), entry1);
  EXPECT_EQ(cel_MapExpr_ReleaseEntry(expr, entry0), entry0);
  EXPECT_EQ(cel_MapExpr_Entries(expr, &entries_head, &entries_tail), 0);
  EXPECT_THAT(entries_head, IsNull());
  EXPECT_THAT(entries_tail, IsNull());

  cel_MapEntryExpr* entry2 = cel_MapEntryExpr_New(ast.get());
  ASSERT_THAT(entry2, NotNull());
  cel_MapExpr_AppendEntry(expr, entry0);
  cel_MapExpr_AppendEntry(expr, entry2);
  cel_MapExpr_InsertEntry(expr, entry2, entry1);
  EXPECT_EQ(cel_MapEntryExpr_Parent(entry0), expr);
  EXPECT_EQ(cel_MapEntryExpr_Index(entry0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(entry0)), 1);
  EXPECT_EQ(cel_MapEntryExpr_Parent(entry1), expr);
  EXPECT_EQ(cel_MapEntryExpr_Index(entry1), 1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(entry1)), 1);
  EXPECT_EQ(cel_MapEntryExpr_Parent(entry2), expr);
  EXPECT_EQ(cel_MapEntryExpr_Index(entry2), 2);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(entry2)), 1);
  EXPECT_EQ(cel_MapExpr_Entries(expr, &entries_head, &entries_tail), 3);
  EXPECT_EQ(entries_head, entry0);
  EXPECT_EQ(entries_tail, entry2);
  EXPECT_EQ(cel_MapExpr_Entry(expr, 0), entry0);
  EXPECT_EQ(cel_MapExpr_Entry(expr, 1), entry1);
  EXPECT_EQ(cel_MapExpr_Entry(expr, 2), entry2);
  EXPECT_THAT(cel_MapExpr_PrevEntry(entry0), IsNull());
  EXPECT_EQ(cel_MapExpr_PrevEntry(entry1), entry0);
  EXPECT_EQ(cel_MapExpr_PrevEntry(entry2), entry1);
  EXPECT_THAT(cel_MapExpr_NextEntry(entry2), IsNull());
  EXPECT_EQ(cel_MapExpr_NextEntry(entry1), entry2);
  EXPECT_EQ(cel_MapExpr_NextEntry(entry0), entry1);
}

TEST_F(AstTest, StructField) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_StructFieldExpr* expr = cel_StructFieldExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kStructField);
  EXPECT_EQ(cel_StructFieldExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(cel_StructFieldExpr_DownCast(
                cel_Expr_UpCast((const cel_StructFieldExpr*)expr)),
            expr);
  EXPECT_EQ(cel_StructFieldExpr_Index(expr), -1);

  EXPECT_TRUE(cel_StringView_Equals(cel_StructFieldExpr_Name(expr),
                                    cel_StringView_From("")));
  cel_StructFieldExpr_SetName(expr, cel_StringView_From("foo"));
  EXPECT_TRUE(cel_StringView_Equals(cel_StructFieldExpr_Name(expr),
                                    cel_StringView_From("foo")));

  EXPECT_THAT(cel_StructFieldExpr_Value(expr), IsNull());
  EXPECT_THAT(cel_StructFieldExpr_SetValue(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_StructFieldExpr_Value(expr), IsNull());
  EXPECT_THAT(cel_StructFieldExpr_ReleaseValue(expr), IsNull());
  EXPECT_THAT(cel_StructFieldExpr_Value(expr), IsNull());

  cel_Expr* value = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(value, NotNull());
  EXPECT_THAT(cel_StructFieldExpr_SetValue(expr, value), IsNull());
  EXPECT_EQ(cel_StructFieldExpr_Value(expr), value);
  EXPECT_EQ(cel_Expr_Parent(value), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(value), 1);
  EXPECT_EQ(cel_StructFieldExpr_ReleaseValue(expr), value);
  EXPECT_THAT(cel_StructFieldExpr_Value(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(value), IsNull());
  EXPECT_EQ(cel_Expr_Depth(value), 0);

  EXPECT_FALSE(cel_StructFieldExpr_Optional(expr));
  cel_StructFieldExpr_SetOptional(expr, true);
  EXPECT_TRUE(cel_StructFieldExpr_Optional(expr));
}

TEST_F(AstTest, Struct) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_StructExpr* expr = cel_StructExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kStruct);
  EXPECT_EQ(cel_StructExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(
      cel_StructExpr_DownCast(cel_Expr_UpCast((const cel_StructExpr*)expr)),
      expr);

  EXPECT_TRUE(cel_StringView_Equals(cel_StructExpr_Name(expr),
                                    cel_StringView_From("")));
  cel_StructExpr_SetName(expr, cel_StringView_From("foo"));
  EXPECT_TRUE(cel_StringView_Equals(cel_StructExpr_Name(expr),
                                    cel_StringView_From("foo")));

  cel_StructFieldExpr* fields_head;
  cel_StructFieldExpr* fields_tail;
  EXPECT_EQ(cel_StructExpr_Fields(expr, &fields_head, &fields_tail), 0);
  EXPECT_THAT(fields_head, IsNull());
  EXPECT_THAT(fields_tail, IsNull());
  EXPECT_THAT(cel_StructExpr_PrevField(cel_nullptr), IsNull());
  EXPECT_THAT(cel_StructExpr_NextField(cel_nullptr), IsNull());

  cel_StructFieldExpr* field0 = cel_StructFieldExpr_New(ast.get());
  ASSERT_THAT(field0, NotNull());

  // Prepend
  cel_StructExpr_PrependField(expr, field0);
  EXPECT_EQ(cel_StructFieldExpr_Parent(field0), expr);
  EXPECT_EQ(cel_StructFieldExpr_Index(field0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(field0)), 1);
  EXPECT_EQ(cel_StructExpr_Fields(expr, &fields_head, &fields_tail), 1);
  EXPECT_EQ(fields_head, field0);
  EXPECT_EQ(fields_tail, field0);
  EXPECT_THAT(cel_StructExpr_PrevField(field0), IsNull());
  EXPECT_THAT(cel_StructExpr_NextField(field0), IsNull());
  EXPECT_EQ(cel_StructExpr_Field(expr, 0), field0);
  EXPECT_EQ(cel_StructExpr_ReleaseField(expr, field0), field0);
  EXPECT_THAT(cel_StructFieldExpr_Parent(field0), IsNull());
  EXPECT_EQ(cel_StructFieldExpr_Index(field0), -1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(field0)), 0);
  EXPECT_EQ(cel_StructExpr_Fields(expr, &fields_head, &fields_tail), 0);
  EXPECT_THAT(fields_head, IsNull());
  EXPECT_THAT(fields_tail, IsNull());
  EXPECT_THAT(cel_StructExpr_PrevField(cel_nullptr), IsNull());
  EXPECT_THAT(cel_StructExpr_NextField(cel_nullptr), IsNull());

  // Append
  cel_StructExpr_AppendField(expr, field0);
  EXPECT_EQ(cel_StructFieldExpr_Parent(field0), expr);
  EXPECT_EQ(cel_StructFieldExpr_Index(field0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(field0)), 1);
  EXPECT_EQ(cel_StructExpr_Fields(expr, &fields_head, &fields_tail), 1);
  EXPECT_EQ(fields_head, field0);
  EXPECT_EQ(fields_tail, field0);
  EXPECT_THAT(cel_StructExpr_PrevField(field0), IsNull());
  EXPECT_THAT(cel_StructExpr_NextField(field0), IsNull());
  EXPECT_EQ(cel_StructExpr_Field(expr, 0), field0);
  EXPECT_EQ(cel_StructExpr_ReleaseField(expr, field0), field0);
  EXPECT_THAT(cel_StructFieldExpr_Parent(field0), IsNull());
  EXPECT_EQ(cel_StructFieldExpr_Index(field0), -1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(field0)), 0);
  EXPECT_EQ(cel_StructExpr_Fields(expr, &fields_head, &fields_tail), 0);
  EXPECT_THAT(fields_head, IsNull());
  EXPECT_THAT(fields_tail, IsNull());
  EXPECT_THAT(cel_StructExpr_PrevField(cel_nullptr), IsNull());
  EXPECT_THAT(cel_StructExpr_NextField(cel_nullptr), IsNull());

  cel_StructFieldExpr* field1 = cel_StructFieldExpr_New(ast.get());
  ASSERT_THAT(field1, NotNull());

  // Prepend
  cel_StructExpr_PrependField(expr, field1);
  cel_StructExpr_PrependField(expr, field0);
  EXPECT_EQ(cel_StructFieldExpr_Parent(field0), expr);
  EXPECT_EQ(cel_StructFieldExpr_Index(field0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(field0)), 1);
  EXPECT_EQ(cel_StructFieldExpr_Parent(field1), expr);
  EXPECT_EQ(cel_StructFieldExpr_Index(field1), 1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(field1)), 1);
  EXPECT_EQ(cel_StructExpr_Fields(expr, &fields_head, &fields_tail), 2);
  EXPECT_EQ(fields_head, field0);
  EXPECT_EQ(fields_tail, field1);
  EXPECT_EQ(cel_StructExpr_Field(expr, 0), field0);
  EXPECT_EQ(cel_StructExpr_Field(expr, 1), field1);
  EXPECT_THAT(cel_StructExpr_PrevField(field0), IsNull());
  EXPECT_EQ(cel_StructExpr_PrevField(field1), field0);
  EXPECT_THAT(cel_StructExpr_NextField(field1), IsNull());
  EXPECT_EQ(cel_StructExpr_NextField(field0), field1);
  EXPECT_EQ(cel_StructExpr_ReleaseField(expr, field0), field0);
  EXPECT_EQ(cel_StructExpr_ReleaseField(expr, field1), field1);
  EXPECT_EQ(cel_StructExpr_Fields(expr, &fields_head, &fields_tail), 0);
  EXPECT_THAT(fields_head, IsNull());
  EXPECT_THAT(fields_tail, IsNull());

  // Append
  cel_StructExpr_AppendField(expr, field0);
  cel_StructExpr_AppendField(expr, field1);
  EXPECT_EQ(cel_StructFieldExpr_Parent(field0), expr);
  EXPECT_EQ(cel_StructFieldExpr_Index(field0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(field0)), 1);
  EXPECT_EQ(cel_StructFieldExpr_Parent(field1), expr);
  EXPECT_EQ(cel_StructFieldExpr_Index(field1), 1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(field1)), 1);
  EXPECT_EQ(cel_StructExpr_Fields(expr, &fields_head, &fields_tail), 2);
  EXPECT_EQ(fields_head, field0);
  EXPECT_EQ(fields_tail, field1);
  EXPECT_EQ(cel_StructExpr_Field(expr, 0), field0);
  EXPECT_EQ(cel_StructExpr_Field(expr, 1), field1);
  EXPECT_THAT(cel_StructExpr_PrevField(field0), IsNull());
  EXPECT_EQ(cel_StructExpr_PrevField(field1), field0);
  EXPECT_THAT(cel_StructExpr_NextField(field1), IsNull());
  EXPECT_EQ(cel_StructExpr_NextField(field0), field1);
  EXPECT_EQ(cel_StructExpr_ReleaseField(expr, field1), field1);
  EXPECT_EQ(cel_StructExpr_ReleaseField(expr, field0), field0);
  EXPECT_EQ(cel_StructExpr_Fields(expr, &fields_head, &fields_tail), 0);
  EXPECT_THAT(fields_head, IsNull());
  EXPECT_THAT(fields_tail, IsNull());

  cel_StructFieldExpr* field2 = cel_StructFieldExpr_New(ast.get());
  ASSERT_THAT(field2, NotNull());
  cel_StructExpr_AppendField(expr, field0);
  cel_StructExpr_AppendField(expr, field2);
  cel_StructExpr_InsertField(expr, field2, field1);
  EXPECT_EQ(cel_StructFieldExpr_Parent(field0), expr);
  EXPECT_EQ(cel_StructFieldExpr_Index(field0), 0);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(field0)), 1);
  EXPECT_EQ(cel_StructFieldExpr_Parent(field1), expr);
  EXPECT_EQ(cel_StructFieldExpr_Index(field1), 1);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(field1)), 1);
  EXPECT_EQ(cel_StructFieldExpr_Parent(field2), expr);
  EXPECT_EQ(cel_StructFieldExpr_Index(field2), 2);
  EXPECT_EQ(cel_Expr_Depth(cel_Expr_UpCast(field2)), 1);
  EXPECT_EQ(cel_StructExpr_Fields(expr, &fields_head, &fields_tail), 3);
  EXPECT_EQ(fields_head, field0);
  EXPECT_EQ(fields_tail, field2);
  EXPECT_EQ(cel_StructExpr_Field(expr, 0), field0);
  EXPECT_EQ(cel_StructExpr_Field(expr, 1), field1);
  EXPECT_EQ(cel_StructExpr_Field(expr, 2), field2);
  EXPECT_THAT(cel_StructExpr_PrevField(field0), IsNull());
  EXPECT_EQ(cel_StructExpr_PrevField(field1), field0);
  EXPECT_EQ(cel_StructExpr_PrevField(field2), field1);
  EXPECT_THAT(cel_StructExpr_NextField(field2), IsNull());
  EXPECT_EQ(cel_StructExpr_NextField(field1), field2);
  EXPECT_EQ(cel_StructExpr_NextField(field0), field1);
}

TEST_F(AstTest, Comprehension) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_ComprehensionExpr* expr = cel_ComprehensionExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  EXPECT_EQ(cel_Expr_Kind(cel_Expr_UpCast(expr)), cel_ExprKind_kComprehension);
  EXPECT_EQ(cel_ComprehensionExpr_DownCast(cel_Expr_UpCast(expr)), expr);
  EXPECT_EQ(cel_ComprehensionExpr_DownCast(
                cel_Expr_UpCast((const cel_ComprehensionExpr*)expr)),
            expr);

  EXPECT_TRUE(cel_StringView_Equals(cel_ComprehensionExpr_IterVar(expr),
                                    cel_StringView_From("")));
  cel_ComprehensionExpr_SetIterVar(expr, cel_StringView_From("foo"));
  EXPECT_TRUE(cel_StringView_Equals(cel_ComprehensionExpr_IterVar(expr),
                                    cel_StringView_From("foo")));

  EXPECT_TRUE(cel_StringView_Equals(cel_ComprehensionExpr_IterVar2(expr),
                                    cel_StringView_From("")));
  cel_ComprehensionExpr_SetIterVar2(expr, cel_StringView_From("foo"));
  EXPECT_TRUE(cel_StringView_Equals(cel_ComprehensionExpr_IterVar2(expr),
                                    cel_StringView_From("foo")));

  EXPECT_TRUE(cel_StringView_Equals(cel_ComprehensionExpr_AccuVar(expr),
                                    cel_StringView_From("")));
  cel_ComprehensionExpr_SetAccuVar(expr, cel_StringView_From("foo"));
  EXPECT_TRUE(cel_StringView_Equals(cel_ComprehensionExpr_AccuVar(expr),
                                    cel_StringView_From("foo")));

  EXPECT_THAT(cel_ComprehensionExpr_IterRange(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_SetIterRange(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_IterRange(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_ReleaseIterRange(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_IterRange(expr), IsNull());

  cel_Expr* iter_range = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(iter_range, NotNull());
  EXPECT_THAT(cel_ComprehensionExpr_SetIterRange(expr, iter_range), IsNull());
  EXPECT_EQ(cel_ComprehensionExpr_IterRange(expr), iter_range);
  EXPECT_EQ(cel_Expr_Parent(iter_range), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(iter_range), 1);
  EXPECT_EQ(cel_ComprehensionExpr_ReleaseIterRange(expr), iter_range);
  EXPECT_THAT(cel_ComprehensionExpr_IterRange(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(iter_range), IsNull());
  EXPECT_EQ(cel_Expr_Depth(iter_range), 0);

  EXPECT_THAT(cel_ComprehensionExpr_AccuInit(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_SetAccuInit(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_AccuInit(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_ReleaseAccuInit(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_AccuInit(expr), IsNull());

  cel_Expr* accu_init = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(accu_init, NotNull());
  EXPECT_THAT(cel_ComprehensionExpr_SetAccuInit(expr, accu_init), IsNull());
  EXPECT_EQ(cel_ComprehensionExpr_AccuInit(expr), accu_init);
  EXPECT_EQ(cel_Expr_Parent(accu_init), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(accu_init), 1);
  EXPECT_EQ(cel_ComprehensionExpr_ReleaseAccuInit(expr), accu_init);
  EXPECT_THAT(cel_ComprehensionExpr_AccuInit(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(accu_init), IsNull());
  EXPECT_EQ(cel_Expr_Depth(accu_init), 0);

  EXPECT_THAT(cel_ComprehensionExpr_LoopCondition(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_SetLoopCondition(expr, cel_nullptr),
              IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_LoopCondition(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_ReleaseLoopCondition(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_LoopCondition(expr), IsNull());

  cel_Expr* loop_condition =
      cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(loop_condition, NotNull());
  EXPECT_THAT(cel_ComprehensionExpr_SetLoopCondition(expr, loop_condition),
              IsNull());
  EXPECT_EQ(cel_ComprehensionExpr_LoopCondition(expr), loop_condition);
  EXPECT_EQ(cel_Expr_Parent(loop_condition), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(loop_condition), 1);
  EXPECT_EQ(cel_ComprehensionExpr_ReleaseLoopCondition(expr), loop_condition);
  EXPECT_THAT(cel_ComprehensionExpr_LoopCondition(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(loop_condition), IsNull());
  EXPECT_EQ(cel_Expr_Depth(loop_condition), 0);

  EXPECT_THAT(cel_ComprehensionExpr_LoopStep(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_SetLoopStep(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_LoopStep(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_ReleaseLoopStep(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_LoopStep(expr), IsNull());

  cel_Expr* loop_step = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(loop_step, NotNull());
  EXPECT_THAT(cel_ComprehensionExpr_SetLoopStep(expr, loop_step), IsNull());
  EXPECT_EQ(cel_ComprehensionExpr_LoopStep(expr), loop_step);
  EXPECT_EQ(cel_Expr_Parent(loop_step), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(loop_step), 1);
  EXPECT_EQ(cel_ComprehensionExpr_ReleaseLoopStep(expr), loop_step);
  EXPECT_THAT(cel_ComprehensionExpr_LoopStep(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(loop_step), IsNull());
  EXPECT_EQ(cel_Expr_Depth(loop_step), 0);

  EXPECT_THAT(cel_ComprehensionExpr_Result(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_SetResult(expr, cel_nullptr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_Result(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_ReleaseResult(expr), IsNull());
  EXPECT_THAT(cel_ComprehensionExpr_Result(expr), IsNull());

  cel_Expr* result = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast.get()));
  ASSERT_THAT(result, NotNull());
  EXPECT_THAT(cel_ComprehensionExpr_SetResult(expr, result), IsNull());
  EXPECT_EQ(cel_ComprehensionExpr_Result(expr), result);
  EXPECT_EQ(cel_Expr_Parent(result), cel_Expr_UpCast(expr));
  EXPECT_EQ(cel_Expr_Depth(result), 1);
  EXPECT_EQ(cel_ComprehensionExpr_ReleaseResult(expr), result);
  EXPECT_THAT(cel_ComprehensionExpr_Result(expr), IsNull());
  EXPECT_THAT(cel_Expr_Parent(result), IsNull());
  EXPECT_EQ(cel_Expr_Depth(result), 0);
}

using AstDeathTest = AstTest;

TEST_F(AstDeathTest, Expr) {
#ifndef NDEBUG
  AstPtr ast0(cel_Ast_New(arena()));
  ASSERT_THAT(ast0, NotNull());
  AstPtr ast1(cel_Ast_New(arena()));
  ASSERT_THAT(ast1, NotNull());
  cel_Expr* expr0 = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast0.get()));
  ASSERT_THAT(expr0, NotNull());
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Ast_SetExpr(ast1.get(), expr0)), _);
#else
  GTEST_SKIP() << "optimized build or death test is unsupported";
#endif
}

TEST_F(AstDeathTest, Select) {
#ifndef NDEBUG
  AstPtr ast0(cel_Ast_New(arena()));
  ASSERT_THAT(ast0, NotNull());
  AstPtr ast1(cel_Ast_New(arena()));
  ASSERT_THAT(ast1, NotNull());
  cel_Expr* expr0 = cel_Expr_UpCast(cel_UnspecifiedExpr_New(ast0.get()));
  ASSERT_THAT(expr0, NotNull());
  cel_SelectExpr* expr1 = cel_SelectExpr_New(ast1.get());
  ASSERT_THAT(expr1, NotNull());
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_SelectExpr_SetOperand(expr1, expr0)),
                            _);
#else
  GTEST_SKIP() << "optimized build or death test is unsupported";
#endif
}

}  // namespace
