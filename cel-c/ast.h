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

#ifndef THIRD_PARTY_CEL_C_AST_H_
#define THIRD_PARTY_CEL_C_AST_H_

#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>
#include <stdint.h>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/constant.h"
#include "cel-c/operators.h"
#include "cel-c/ref.h"
#include "cel-c/source.h"
#include "cel-c/string_view.h"
#include "cel-c/type.h"

CEL_BEGIN_DECLS

typedef enum CEL_ATTRIBUTE_OPEN_ENUM {
  cel_ExprKind_kUnspecified = 0,
  cel_ExprKind_kIdent,
  cel_ExprKind_kConst,
  cel_ExprKind_kSelect,
  cel_ExprKind_kCall,
  cel_ExprKind_kCallArg,
  cel_ExprKind_kUnary,
  cel_ExprKind_kBinary,
  cel_ExprKind_kTernary,
  cel_ExprKind_kList,
  cel_ExprKind_kListElement,
  cel_ExprKind_kMapEntry,
  cel_ExprKind_kMap,
  cel_ExprKind_kStructField,
  cel_ExprKind_kStruct,
  cel_ExprKind_kComprehension,
} cel_ExprKind;

typedef int64_t cel_ExprId;

typedef struct cel_Ast cel_Ast;

typedef struct cel_Expr cel_Expr;
typedef struct cel_UnspecifiedExpr cel_UnspecifiedExpr;
typedef struct cel_IdentExpr cel_IdentExpr;
typedef struct cel_ConstExpr cel_ConstExpr;
typedef struct cel_SelectExpr cel_SelectExpr;
typedef struct cel_CallArgExpr cel_CallArgExpr;
typedef struct cel_CallExpr cel_CallExpr;
typedef struct cel_UnaryExpr cel_UnaryExpr;
typedef struct cel_BinaryExpr cel_BinaryExpr;
typedef struct cel_TernaryExpr cel_TernaryExpr;
typedef struct cel_ListElementExpr cel_ListElementExpr;
typedef struct cel_ListExpr cel_ListExpr;
typedef struct cel_MapEntryExpr cel_MapEntryExpr;
typedef struct cel_MapExpr cel_MapExpr;
typedef struct cel_StructFieldExpr cel_StructFieldExpr;
typedef struct cel_StructExpr cel_StructExpr;
typedef struct cel_ComprehensionExpr cel_ComprehensionExpr;

// cel_Ast

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Ast*) cel_Ast_New(CEL_NONNULL(cel_Arena*) arena);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Ast_Delete(CEL_NULLABLE(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_ExprId cel_Ast_NextId(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_ExprId cel_Ast_MaxId(CEL_NONNULL(const cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_Ast_Expr(CEL_NONNULL(const cel_Ast*) ast);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_Ast_SetExpr(CEL_NONNULL(cel_Ast*) ast, CEL_NULLABLE(cel_Expr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_Ast_ReleaseExpr(CEL_NONNULL(cel_Ast*) ast);

// cel_Expr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_ExprKind cel_Expr_Kind(CEL_NONNULL(const cel_Expr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_ExprId cel_Expr_Id(CEL_NONNULL(const cel_Expr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Expr_SetId(CEL_NONNULL(cel_Expr*) expr, cel_ExprId id);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN int32_t cel_Expr_SourcePosition(CEL_NONNULL(const cel_Expr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_SourceRange cel_Expr_SourceRange(CEL_NONNULL(const cel_Expr*)
                                                    expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Expr_SetSourcePosition(CEL_NONNULL(cel_Expr*) expr,
                                           int32_t source_position);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Expr_SetSourceRange(CEL_NONNULL(cel_Expr*) expr,
                                        cel_SourceRange source_range);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_Expr_Parent(CEL_NONNULL(const cel_Expr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN size_t cel_Expr_Depth(CEL_NONNULL(const cel_Expr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(const cel_Ref*)
    cel_Expr_Ref(CEL_NONNULL(const cel_Expr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Expr_SetRef(CEL_NONNULL(cel_Expr*) expr,
                                CEL_NULLABLE(const cel_Ref*) ref);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(const cel_Type*)
    cel_Expr_Type(CEL_NONNULL(const cel_Expr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Expr_SetType(CEL_NONNULL(cel_Expr*) expr,
                                 CEL_NONNULL(const cel_Type*) type);

// cel_UnspecifiedExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_UnspecifiedExpr*)
    cel_UnspecifiedExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_UnspecifiedExpr_MutableUpCast(
        CEL_NULLABILITY_UNKNOWN(cel_UnspecifiedExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_UnspecifiedExpr_ConstUpCast(
        CEL_NULLABILITY_UNKNOWN(const cel_UnspecifiedExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_UnspecifiedExpr*)
    cel_UnspecifiedExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*)
                                            expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kUnspecified);

  return (CEL_NULLABILITY_UNKNOWN(cel_UnspecifiedExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_UnspecifiedExpr*)
    cel_UnspecifiedExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
                                          expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kUnspecified);

  return (CEL_NULLABILITY_UNKNOWN(const cel_UnspecifiedExpr*))expr;
}

// cel_IdentExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_IdentExpr*)
    cel_IdentExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_IdentExpr_Name(CEL_NONNULL(const cel_IdentExpr*)
                                                 expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_IdentExpr_SetName(CEL_NONNULL(cel_IdentExpr*) expr,
                                      cel_StringView name);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_IdentExpr_MutableUpCast(CEL_NULLABILITY_UNKNOWN(cel_IdentExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_IdentExpr_ConstUpCast(CEL_NULLABILITY_UNKNOWN(const cel_IdentExpr*)
                                  expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_IdentExpr*)
    cel_IdentExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr || cel_Expr_Kind(expr) == cel_ExprKind_kIdent);

  return (CEL_NULLABILITY_UNKNOWN(cel_IdentExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_IdentExpr*)
    cel_IdentExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr || cel_Expr_Kind(expr) == cel_ExprKind_kIdent);

  return (CEL_NULLABILITY_UNKNOWN(const cel_IdentExpr*))expr;
}

// cel_ConstExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_ConstExpr*)
    cel_ConstExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(const cel_Constant*)
    cel_ConstExpr_Value(CEL_NONNULL(const cel_ConstExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(cel_Constant*)
    cel_ConstExpr_MutableValue(CEL_NONNULL(cel_ConstExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_ConstExpr_MutableUpCast(CEL_NULLABILITY_UNKNOWN(cel_ConstExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_ConstExpr_ConstUpCast(CEL_NULLABILITY_UNKNOWN(const cel_ConstExpr*)
                                  expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_ConstExpr*)
    cel_ConstExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr || cel_Expr_Kind(expr) == cel_ExprKind_kConst);

  return (CEL_NULLABILITY_UNKNOWN(cel_ConstExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_ConstExpr*)
    cel_ConstExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr || cel_Expr_Kind(expr) == cel_ExprKind_kConst);

  return (CEL_NULLABILITY_UNKNOWN(const cel_ConstExpr*))expr;
}

// cel_SelectExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_SelectExpr*)
    cel_SelectExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_SelectExpr_Operand(CEL_NONNULL(const cel_SelectExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_SelectExpr_SetOperand(CEL_NONNULL(cel_SelectExpr*) expr,
                              CEL_NULLABLE(cel_Expr*) operand);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_SelectExpr_ReleaseOperand(CEL_NONNULL(cel_SelectExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView
    cel_SelectExpr_Field(CEL_NONNULL(const cel_SelectExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_SelectExpr_SetField(CEL_NONNULL(cel_SelectExpr*) expr,
                                        cel_StringView field);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_SelectExpr_TestOnly(CEL_NONNULL(const cel_SelectExpr*)
                                            expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_SelectExpr_SetTestOnly(CEL_NONNULL(cel_SelectExpr*) expr,
                                           bool test_only);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_SelectExpr_MutableUpCast(CEL_NULLABILITY_UNKNOWN(cel_SelectExpr*)
                                     expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_SelectExpr_ConstUpCast(CEL_NULLABILITY_UNKNOWN(const cel_SelectExpr*)
                                   expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_SelectExpr*)
    cel_SelectExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kSelect);

  return (CEL_NULLABILITY_UNKNOWN(cel_SelectExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_SelectExpr*)
    cel_SelectExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
                                     expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kSelect);

  return (CEL_NULLABILITY_UNKNOWN(const cel_SelectExpr*))expr;
}

// cel_CallArgExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_CallArgExpr*)
    cel_CallArgExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_CallExpr*)
    cel_CallArgExpr_Parent(CEL_NONNULL(const cel_CallArgExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN ptrdiff_t cel_CallArgExpr_Index(CEL_NONNULL(const cel_CallArgExpr*)
                                               expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_CallArgExpr_Value(CEL_NONNULL(const cel_CallArgExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_CallArgExpr_SetValue(CEL_NONNULL(cel_CallArgExpr*) expr,
                             CEL_NULLABLE(cel_Expr*) value);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_CallArgExpr_ReleaseValue(CEL_NONNULL(cel_CallArgExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_CallArgExpr_MutableUpCast(CEL_NULLABILITY_UNKNOWN(cel_CallArgExpr*)
                                      expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_CallArgExpr_ConstUpCast(CEL_NULLABILITY_UNKNOWN(const cel_CallArgExpr*)
                                    expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_CallArgExpr*)
    cel_CallArgExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kCallArg);

  return (CEL_NULLABILITY_UNKNOWN(cel_CallArgExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_CallArgExpr*)
    cel_CallArgExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
                                      expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kCallArg);

  return (CEL_NULLABILITY_UNKNOWN(const cel_CallArgExpr*))expr;
}

// cel_CallExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_CallExpr*)
    cel_CallExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_CallExpr_Target(CEL_NONNULL(const cel_CallExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_CallExpr_SetTarget(CEL_NONNULL(cel_CallExpr*) expr,
                           CEL_NULLABLE(cel_Expr*) target);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_CallExpr_ReleaseTarget(CEL_NONNULL(cel_CallExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_CallExpr_Function(CEL_NONNULL(const cel_CallExpr*)
                                                    expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_CallExpr_SetFunction(CEL_NONNULL(cel_CallExpr*) expr,
                                         cel_StringView function);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_CallExpr_AppendArg(CEL_NONNULL(cel_CallExpr*) expr,
                                       CEL_NONNULL(cel_CallArgExpr*) arg);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_CallExpr_PrependArg(CEL_NONNULL(cel_CallExpr*) expr,
                                        CEL_NONNULL(cel_CallArgExpr*) arg);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_CallExpr_InsertArg(CEL_NONNULL(cel_CallExpr*) expr,
                                       CEL_NULLABLE(cel_CallArgExpr*) before,
                                       CEL_NONNULL(cel_CallArgExpr*) arg);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(cel_CallArgExpr*)
    cel_CallExpr_ReleaseArg(CEL_NONNULL(cel_CallExpr*) expr,
                            CEL_NONNULL(cel_CallArgExpr*) arg);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(cel_CallArgExpr*)
    cel_CallExpr_Arg(CEL_NONNULL(const cel_CallExpr*) expr, size_t index);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN size_t cel_CallExpr_Args(CEL_NONNULL(const cel_CallExpr*) expr,
                                    CEL_NULLABLE(cel_CallArgExpr*) *
                                        cel_nullable head,
                                    CEL_NULLABLE(cel_CallArgExpr*) *
                                        cel_nullable tail);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_CallArgExpr*)
    cel_CallExpr_PrevArg(CEL_NULLABLE(const cel_CallArgExpr*) arg);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_CallArgExpr*)
    cel_CallExpr_NextArg(CEL_NULLABLE(const cel_CallArgExpr*) arg);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_CallExpr_MutableUpCast(CEL_NULLABILITY_UNKNOWN(cel_CallExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_CallExpr_ConstUpCast(CEL_NULLABILITY_UNKNOWN(const cel_CallExpr*)
                                 expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_CallExpr*)
    cel_CallExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr || cel_Expr_Kind(expr) == cel_ExprKind_kCall);

  return (CEL_NULLABILITY_UNKNOWN(cel_CallExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_CallExpr*)
    cel_CallExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr || cel_Expr_Kind(expr) == cel_ExprKind_kCall);

  return (CEL_NULLABILITY_UNKNOWN(const cel_CallExpr*))expr;
}

// cel_UnaryExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_UnaryExpr*)
    cel_UnaryExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_UnaryOp cel_UnaryExpr_Op(CEL_NONNULL(const cel_UnaryExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_UnaryExpr_SetOp(CEL_NONNULL(cel_UnaryExpr*) expr,
                                    cel_UnaryOp op);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_UnaryExpr_Arg(CEL_NONNULL(const cel_UnaryExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_UnaryExpr_SetArg(CEL_NONNULL(cel_UnaryExpr*) expr,
                         CEL_NULLABLE(cel_Expr*) arg);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_UnaryExpr_ReleaseArg(CEL_NONNULL(cel_UnaryExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_UnaryExpr_MutableUpCast(CEL_NULLABILITY_UNKNOWN(cel_UnaryExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_UnaryExpr_ConstUpCast(CEL_NULLABILITY_UNKNOWN(const cel_UnaryExpr*)
                                  expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_UnaryExpr*)
    cel_UnaryExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr || cel_Expr_Kind(expr) == cel_ExprKind_kUnary);

  return (CEL_NULLABILITY_UNKNOWN(cel_UnaryExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_UnaryExpr*)
    cel_UnaryExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr || cel_Expr_Kind(expr) == cel_ExprKind_kUnary);

  return (CEL_NULLABILITY_UNKNOWN(const cel_UnaryExpr*))expr;
}

// cel_BinaryExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_BinaryExpr*)
    cel_BinaryExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_BinaryOp cel_BinaryExpr_Op(CEL_NONNULL(const cel_BinaryExpr*)
                                              expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_BinaryExpr_SetOp(CEL_NONNULL(cel_BinaryExpr*) expr,
                                     cel_BinaryOp op);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_BinaryExpr_Left(CEL_NONNULL(const cel_BinaryExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_BinaryExpr_SetLeft(CEL_NONNULL(cel_BinaryExpr*) expr,
                           CEL_NULLABLE(cel_Expr*) left);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_BinaryExpr_ReleaseLeft(CEL_NONNULL(cel_BinaryExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_BinaryExpr_Right(CEL_NONNULL(const cel_BinaryExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_BinaryExpr_SetRight(CEL_NONNULL(cel_BinaryExpr*) expr,
                            CEL_NULLABLE(cel_Expr*) right);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_BinaryExpr_ReleaseRight(CEL_NONNULL(cel_BinaryExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_BinaryExpr_MutableUpCast(CEL_NULLABILITY_UNKNOWN(cel_BinaryExpr*)
                                     expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_BinaryExpr_ConstUpCast(CEL_NULLABILITY_UNKNOWN(const cel_BinaryExpr*)
                                   expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_BinaryExpr*)
    cel_BinaryExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kBinary);

  return (CEL_NULLABILITY_UNKNOWN(cel_BinaryExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_BinaryExpr*)
    cel_BinaryExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
                                     expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kBinary);

  return (CEL_NULLABILITY_UNKNOWN(const cel_BinaryExpr*))expr;
}

// cel_TernaryExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_TernaryExpr*)
    cel_TernaryExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_TernaryOp cel_TernaryExpr_Op(CEL_NONNULL(const cel_TernaryExpr*)
                                                expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_TernaryExpr_SetOp(CEL_NONNULL(cel_TernaryExpr*) expr,
                                      cel_TernaryOp op);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_TernaryExpr_Condition(CEL_NONNULL(const cel_TernaryExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_TernaryExpr_SetCondition(CEL_NONNULL(cel_TernaryExpr*) expr,
                                 CEL_NULLABLE(cel_Expr*) condition);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_TernaryExpr_ReleaseCondition(CEL_NONNULL(cel_TernaryExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_TernaryExpr_IfTrue(CEL_NONNULL(const cel_TernaryExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_TernaryExpr_SetIfTrue(CEL_NONNULL(cel_TernaryExpr*) expr,
                              CEL_NULLABLE(cel_Expr*) if_true);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_TernaryExpr_ReleaseIfTrue(CEL_NONNULL(cel_TernaryExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_TernaryExpr_IfFalse(CEL_NONNULL(const cel_TernaryExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_TernaryExpr_SetIfFalse(CEL_NONNULL(cel_TernaryExpr*) expr,
                               CEL_NULLABLE(cel_Expr*) if_false);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_TernaryExpr_ReleaseIfFalse(CEL_NONNULL(cel_TernaryExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_TernaryExpr_MutableUpCast(CEL_NULLABILITY_UNKNOWN(cel_TernaryExpr*)
                                      expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_TernaryExpr_ConstUpCast(CEL_NULLABILITY_UNKNOWN(const cel_TernaryExpr*)
                                    expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_TernaryExpr*)
    cel_TernaryExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kTernary);

  return (CEL_NULLABILITY_UNKNOWN(cel_TernaryExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_TernaryExpr*)
    cel_TernaryExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
                                      expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kTernary);

  return (CEL_NULLABILITY_UNKNOWN(const cel_TernaryExpr*))expr;
}

// cel_ListElementExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_ListElementExpr*)
    cel_ListElementExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_ListExpr*)
    cel_ListElementExpr_Parent(CEL_NONNULL(const cel_ListElementExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN ptrdiff_t
    cel_ListElementExpr_Index(CEL_NONNULL(const cel_ListElementExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ListElementExpr_Value(CEL_NONNULL(const cel_ListElementExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ListElementExpr_SetValue(CEL_NONNULL(cel_ListElementExpr*) expr,
                                 CEL_NULLABLE(cel_Expr*) value);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ListElementExpr_ReleaseValue(CEL_NONNULL(cel_ListElementExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_ListElementExpr_Optional(
    CEL_NONNULL(const cel_ListElementExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_ListElementExpr_SetOptional(
    CEL_NONNULL(cel_ListElementExpr*) expr, bool optional);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_ListElementExpr_MutableUpCast(
        CEL_NULLABILITY_UNKNOWN(cel_ListElementExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_ListElementExpr_ConstUpCast(
        CEL_NULLABILITY_UNKNOWN(const cel_ListElementExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_ListElementExpr*)
    cel_ListElementExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*)
                                            expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kListElement);

  return (CEL_NULLABILITY_UNKNOWN(cel_ListElementExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_ListElementExpr*)
    cel_ListElementExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
                                          expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kListElement);

  return (CEL_NULLABILITY_UNKNOWN(const cel_ListElementExpr*))expr;
}

// cel_ListExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_ListExpr*)
    cel_ListExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_ListExpr_AppendElement(CEL_NONNULL(cel_ListExpr*) expr,
                                           CEL_NONNULL(cel_ListElementExpr*)
                                               element);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_ListExpr_PrependElement(CEL_NONNULL(cel_ListExpr*) expr,
                                            CEL_NONNULL(cel_ListElementExpr*)
                                                element);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_ListExpr_InsertElement(CEL_NONNULL(cel_ListExpr*) expr,
                                           CEL_NULLABLE(cel_ListElementExpr*)
                                               before,
                                           CEL_NONNULL(cel_ListElementExpr*)
                                               element);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(cel_ListElementExpr*)
    cel_ListExpr_ReleaseElement(CEL_NONNULL(cel_ListExpr*) expr,
                                CEL_NONNULL(cel_ListElementExpr*) element);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(cel_ListElementExpr*)
    cel_ListExpr_Element(CEL_NONNULL(const cel_ListExpr*) expr, size_t index);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN size_t cel_ListExpr_Elements(CEL_NONNULL(const cel_ListExpr*) expr,
                                        CEL_NULLABLE(cel_ListElementExpr*) *
                                            cel_nullable head,
                                        CEL_NULLABLE(cel_ListElementExpr*) *
                                            cel_nullable tail);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_ListElementExpr*)
    cel_ListExpr_PrevElement(CEL_NULLABLE(const cel_ListElementExpr*) element);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_ListElementExpr*)
    cel_ListExpr_NextElement(CEL_NULLABLE(const cel_ListElementExpr*) element);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_ListExpr_MutableUpCast(CEL_NULLABILITY_UNKNOWN(cel_ListExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_ListExpr_ConstUpCast(CEL_NULLABILITY_UNKNOWN(const cel_ListExpr*)
                                 expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_ListExpr*)
    cel_ListExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr || cel_Expr_Kind(expr) == cel_ExprKind_kList);

  return (CEL_NULLABILITY_UNKNOWN(cel_ListExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_ListExpr*)
    cel_ListExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr || cel_Expr_Kind(expr) == cel_ExprKind_kList);

  return (CEL_NULLABILITY_UNKNOWN(const cel_ListExpr*))expr;
}

// cel_MapEntryExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_MapEntryExpr*)
    cel_MapEntryExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_MapExpr*)
    cel_MapEntryExpr_Parent(CEL_NONNULL(const cel_MapEntryExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN ptrdiff_t cel_MapEntryExpr_Index(CEL_NONNULL(const cel_MapEntryExpr*)
                                                expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_MapEntryExpr_Key(CEL_NONNULL(const cel_MapEntryExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_MapEntryExpr_SetKey(CEL_NONNULL(cel_MapEntryExpr*) expr,
                            CEL_NULLABLE(cel_Expr*) key);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_MapEntryExpr_ReleaseKey(CEL_NONNULL(cel_MapEntryExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_MapEntryExpr_Value(CEL_NONNULL(const cel_MapEntryExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_MapEntryExpr_SetValue(CEL_NONNULL(cel_MapEntryExpr*) expr,
                              CEL_NULLABLE(cel_Expr*) value);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_MapEntryExpr_ReleaseValue(CEL_NONNULL(cel_MapEntryExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_MapEntryExpr_Optional(CEL_NONNULL(const cel_MapEntryExpr*)
                                              expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_MapEntryExpr_SetOptional(CEL_NONNULL(cel_MapEntryExpr*)
                                                 expr,
                                             bool optional);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_MapEntryExpr_MutableUpCast(CEL_NULLABILITY_UNKNOWN(cel_MapEntryExpr*)
                                       expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_MapEntryExpr_ConstUpCast(
        CEL_NULLABILITY_UNKNOWN(const cel_MapEntryExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_MapEntryExpr*)
    cel_MapEntryExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kMapEntry);

  return (CEL_NULLABILITY_UNKNOWN(cel_MapEntryExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_MapEntryExpr*)
    cel_MapEntryExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
                                       expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kMapEntry);

  return (CEL_NULLABILITY_UNKNOWN(const cel_MapEntryExpr*))expr;
}

// cel_MapExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_MapExpr*)
    cel_MapExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_MapExpr_AppendEntry(CEL_NONNULL(cel_MapExpr*) expr,
                                        CEL_NONNULL(cel_MapEntryExpr*) entry);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_MapExpr_PrependEntry(CEL_NONNULL(cel_MapExpr*) expr,
                                         CEL_NONNULL(cel_MapEntryExpr*) entry);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_MapExpr_InsertEntry(CEL_NONNULL(cel_MapExpr*) expr,
                                        CEL_NULLABLE(cel_MapEntryExpr*) before,
                                        CEL_NONNULL(cel_MapEntryExpr*) entry);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(cel_MapEntryExpr*)
    cel_MapExpr_ReleaseEntry(CEL_NONNULL(cel_MapExpr*) expr,
                             CEL_NONNULL(cel_MapEntryExpr*) entry);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(cel_MapEntryExpr*)
    cel_MapExpr_Entry(CEL_NONNULL(const cel_MapExpr*) expr, size_t index);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN size_t cel_MapExpr_Entries(CEL_NONNULL(const cel_MapExpr*) expr,
                                      CEL_NULLABLE(cel_MapEntryExpr*) *
                                          cel_nullable head,
                                      CEL_NULLABLE(cel_MapEntryExpr*) *
                                          cel_nullable tail);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_MapEntryExpr*)
    cel_MapExpr_PrevEntry(CEL_NULLABLE(const cel_MapEntryExpr*) entry);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_MapEntryExpr*)
    cel_MapExpr_NextEntry(CEL_NULLABLE(const cel_MapEntryExpr*) entry);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_MapExpr_MutableUpCast(CEL_NULLABILITY_UNKNOWN(cel_MapExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_MapExpr_ConstUpCast(CEL_NULLABILITY_UNKNOWN(const cel_MapExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_MapExpr*)
    cel_MapExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr || cel_Expr_Kind(expr) == cel_ExprKind_kMap);

  return (CEL_NULLABILITY_UNKNOWN(cel_MapExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_MapExpr*)
    cel_MapExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr || cel_Expr_Kind(expr) == cel_ExprKind_kMap);

  return (CEL_NULLABILITY_UNKNOWN(const cel_MapExpr*))expr;
}

// cel_StructFieldExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_StructFieldExpr*)
    cel_StructFieldExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_StructExpr*)
    cel_StructFieldExpr_Parent(CEL_NONNULL(const cel_StructFieldExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN ptrdiff_t
    cel_StructFieldExpr_Index(CEL_NONNULL(const cel_StructFieldExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView
    cel_StructFieldExpr_Name(CEL_NONNULL(const cel_StructFieldExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_StructFieldExpr_SetName(CEL_NONNULL(cel_StructFieldExpr*)
                                                expr,
                                            cel_StringView name);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_StructFieldExpr_Value(CEL_NONNULL(const cel_StructFieldExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_StructFieldExpr_SetValue(CEL_NONNULL(cel_StructFieldExpr*) expr,
                                 CEL_NULLABLE(cel_Expr*) value);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_StructFieldExpr_ReleaseValue(CEL_NONNULL(cel_StructFieldExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_StructFieldExpr_Optional(
    CEL_NONNULL(const cel_StructFieldExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_StructFieldExpr_SetOptional(
    CEL_NONNULL(cel_StructFieldExpr*) expr, bool optional);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_StructFieldExpr_MutableUpCast(
        CEL_NULLABILITY_UNKNOWN(cel_StructFieldExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_StructFieldExpr_ConstUpCast(
        CEL_NULLABILITY_UNKNOWN(const cel_StructFieldExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_StructFieldExpr*)
    cel_StructFieldExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*)
                                            expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kStructField);

  return (CEL_NULLABILITY_UNKNOWN(cel_StructFieldExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_StructFieldExpr*)
    cel_StructFieldExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
                                          expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kStructField);

  return (CEL_NULLABILITY_UNKNOWN(const cel_StructFieldExpr*))expr;
}

// cel_StructExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_StructExpr*)
    cel_StructExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_StructExpr_Name(CEL_NONNULL(const cel_StructExpr*)
                                                  expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_StructExpr_SetName(CEL_NONNULL(cel_StructExpr*) expr,
                                       cel_StringView name);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_StructExpr_AppendField(CEL_NONNULL(cel_StructExpr*) expr,
                                           CEL_NONNULL(cel_StructFieldExpr*)
                                               field);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_StructExpr_PrependField(CEL_NONNULL(cel_StructExpr*) expr,
                                            CEL_NONNULL(cel_StructFieldExpr*)
                                                field);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_StructExpr_InsertField(CEL_NONNULL(cel_StructExpr*) expr,
                                           CEL_NULLABLE(cel_StructFieldExpr*)
                                               before,
                                           CEL_NONNULL(cel_StructFieldExpr*)
                                               field);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(cel_StructFieldExpr*)
    cel_StructExpr_ReleaseField(CEL_NONNULL(cel_StructExpr*) expr,
                                CEL_NONNULL(cel_StructFieldExpr*) field);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(cel_StructFieldExpr*)
    cel_StructExpr_Field(CEL_NONNULL(const cel_StructExpr*) expr, size_t index);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN size_t cel_StructExpr_Fields(CEL_NONNULL(const cel_StructExpr*) expr,
                                        CEL_NULLABLE(cel_StructFieldExpr*) *
                                            cel_nullable head,
                                        CEL_NULLABLE(cel_StructFieldExpr*) *
                                            cel_nullable tail);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_StructFieldExpr*)
    cel_StructExpr_PrevField(CEL_NULLABLE(const cel_StructFieldExpr*) field);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_StructFieldExpr*)
    cel_StructExpr_NextField(CEL_NULLABLE(const cel_StructFieldExpr*) field);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_StructExpr_MutableUpCast(CEL_NULLABILITY_UNKNOWN(cel_StructExpr*)
                                     expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_StructExpr_ConstUpCast(CEL_NULLABILITY_UNKNOWN(const cel_StructExpr*)
                                   expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_StructExpr*)
    cel_StructExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*) expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kStruct);

  return (CEL_NULLABILITY_UNKNOWN(cel_StructExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_StructExpr*)
    cel_StructExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
                                     expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kStruct);

  return (CEL_NULLABILITY_UNKNOWN(const cel_StructExpr*))expr;
}

// cel_ComprehensionExpr

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_ComprehensionExpr*)
    cel_ComprehensionExpr_New(CEL_NONNULL(cel_Ast*) ast);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_ComprehensionExpr_IterVar(
    CEL_NONNULL(const cel_ComprehensionExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_ComprehensionExpr_SetIterVar(
    CEL_NONNULL(cel_ComprehensionExpr*) expr, cel_StringView iter_var);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_ComprehensionExpr_IterVar2(
    CEL_NONNULL(const cel_ComprehensionExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_ComprehensionExpr_SetIterVar2(
    CEL_NONNULL(cel_ComprehensionExpr*) expr, cel_StringView iter_var2);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ComprehensionExpr_IterRange(CEL_NONNULL(const cel_ComprehensionExpr*)
                                        expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ComprehensionExpr_SetIterRange(CEL_NONNULL(cel_ComprehensionExpr*) expr,
                                       CEL_NULLABLE(cel_Expr*) iter_range);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ComprehensionExpr_ReleaseIterRange(CEL_NONNULL(cel_ComprehensionExpr*)
                                               expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_ComprehensionExpr_AccuVar(
    CEL_NONNULL(const cel_ComprehensionExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_ComprehensionExpr_SetAccuVar(
    CEL_NONNULL(cel_ComprehensionExpr*) expr, cel_StringView accu_var);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ComprehensionExpr_AccuInit(CEL_NONNULL(const cel_ComprehensionExpr*)
                                       expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ComprehensionExpr_SetAccuInit(CEL_NONNULL(cel_ComprehensionExpr*) expr,
                                      CEL_NULLABLE(cel_Expr*) accu_init);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ComprehensionExpr_ReleaseAccuInit(CEL_NONNULL(cel_ComprehensionExpr*)
                                              expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*) cel_ComprehensionExpr_LoopCondition(
    CEL_NONNULL(const cel_ComprehensionExpr*) expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ComprehensionExpr_SetLoopCondition(CEL_NONNULL(cel_ComprehensionExpr*)
                                               expr,
                                           CEL_NULLABLE(cel_Expr*)
                                               loop_condition);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*) cel_ComprehensionExpr_ReleaseLoopCondition(
    CEL_NONNULL(cel_ComprehensionExpr*) expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ComprehensionExpr_LoopStep(CEL_NONNULL(const cel_ComprehensionExpr*)
                                       expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ComprehensionExpr_SetLoopStep(CEL_NONNULL(cel_ComprehensionExpr*) expr,
                                      CEL_NULLABLE(cel_Expr*) loop_step);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ComprehensionExpr_ReleaseLoopStep(CEL_NONNULL(cel_ComprehensionExpr*)
                                              expr);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ComprehensionExpr_Result(CEL_NONNULL(const cel_ComprehensionExpr*)
                                     expr);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ComprehensionExpr_SetResult(CEL_NONNULL(cel_ComprehensionExpr*) expr,
                                    CEL_NULLABLE(cel_Expr*) result);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Expr*)
    cel_ComprehensionExpr_ReleaseResult(CEL_NONNULL(cel_ComprehensionExpr*)
                                            expr);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_Expr*)
    cel_ComprehensionExpr_MutableUpCast(
        CEL_NULLABILITY_UNKNOWN(cel_ComprehensionExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
    cel_ComprehensionExpr_ConstUpCast(
        CEL_NULLABILITY_UNKNOWN(const cel_ComprehensionExpr*) expr) {
  return (CEL_NULLABILITY_UNKNOWN(const cel_Expr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(cel_ComprehensionExpr*)
    cel_ComprehensionExpr_MutableDownCast(CEL_NULLABILITY_UNKNOWN(cel_Expr*)
                                              expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kComprehension);

  return (CEL_NULLABILITY_UNKNOWN(cel_ComprehensionExpr*))expr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABILITY_UNKNOWN(const cel_ComprehensionExpr*)
    cel_ComprehensionExpr_ConstDownCast(CEL_NULLABILITY_UNKNOWN(const cel_Expr*)
                                            expr) {
  CEL_ASSERT(expr == cel_nullptr ||
             cel_Expr_Kind(expr) == cel_ExprKind_kComprehension);

  return (CEL_NULLABILITY_UNKNOWN(const cel_ComprehensionExpr*))expr;
}

CEL_END_DECLS

#ifndef __cplusplus
#define cel_Expr_UpCast(expr)                                       \
  (_Generic((expr),                                                 \
       cel_UnspecifiedExpr*: cel_UnspecifiedExpr_MutableUpCast,     \
       const cel_UnspecifiedExpr*: cel_UnspecifiedExpr_ConstUpCast, \
       cel_IdentExpr*: cel_IdentExpr_MutableUpCast,                 \
       const cel_IdentExpr*: cel_IdentExpr_ConstUpCast,             \
       cel_ConstExpr*: cel_ConstExpr_MutableUpCast,                 \
       const cel_ConstExpr*: cel_ConstExpr_ConstUpCast,             \
       cel_SelectExpr*: cel_SelectExpr_MutableUpCast,               \
       const cel_SelectExpr*: cel_SelectExpr_ConstUpCast,           \
       cel_CallExpr*: cel_CallExpr_MutableUpCast,                   \
       const cel_CallExpr*: cel_CallExpr_ConstUpCast,               \
       cel_CallArgExpr*: cel_CallArgExpr_MutableUpCast,             \
       const cel_CallArgExpr*: cel_CallArgExpr_ConstUpCast,         \
       cel_UnaryExpr*: cel_UnaryExpr_MutableUpCast,                 \
       const cel_UnaryExpr*: cel_UnaryExpr_ConstUpCast,             \
       cel_BinaryExpr*: cel_BinaryExpr_MutableUpCast,               \
       const cel_BinaryExpr*: cel_BinaryExpr_ConstUpCast,           \
       cel_TernaryExpr*: cel_TernaryExpr_MutableUpCast,             \
       const cel_TernaryExpr*: cel_TernaryExpr_ConstUpCast,         \
       cel_ListElementExpr*: cel_ListElementExpr_MutableUpCast,     \
       const cel_ListElementExpr*: cel_ListElementExpr_ConstUpCast, \
       cel_ListExpr*: cel_ListExpr_MutableUpCast,                   \
       const cel_ListExpr*: cel_ListExpr_ConstUpCast,               \
       cel_MapEntryExpr*: cel_MapEntryExpr_MutableUpCast,           \
       const cel_MapEntryExpr*: cel_MapEntryExpr_ConstUpCast,       \
       cel_MapExpr*: cel_MapExpr_MutableUpCast,                     \
       const cel_MapExpr*: cel_MapExpr_ConstUpCast,                 \
       cel_StructFieldExpr*: cel_StructFieldExpr_MutableUpCast,     \
       const cel_StructFieldExpr*: cel_StructFieldExpr_ConstUpCast, \
       cel_StructExpr*: cel_StructExpr_MutableUpCast,               \
       const cel_StructExpr*: cel_StructExpr_ConstUpCast,           \
       cel_ComprehensionExpr*: cel_ComprehensionExpr_MutableUpCast, \
       const cel_ComprehensionExpr*: cel_ComprehensionExpr_ConstUpCast)(expr))
#define cel_UnspecifiedExpr_DownCast(expr)             \
  (_Generic((expr),                                    \
       cel_Expr*: cel_UnspecifiedExpr_MutableDownCast, \
       const cel_Expr*: cel_UnspecifiedExpr_ConstDownCast)(expr))
#define cel_IdentExpr_DownCast(expr)             \
  (_Generic((expr),                              \
       cel_Expr*: cel_IdentExpr_MutableDownCast, \
       const cel_Expr*: cel_IdentExpr_ConstDownCast)(expr))
#define cel_ConstExpr_DownCast(expr)             \
  (_Generic((expr),                              \
       cel_Expr*: cel_ConstExpr_MutableDownCast, \
       const cel_Expr*: cel_ConstExpr_ConstDownCast)(expr))
#define cel_SelectExpr_DownCast(expr)             \
  (_Generic((expr),                               \
       cel_Expr*: cel_SelectExpr_MutableDownCast, \
       const cel_Expr*: cel_SelectExpr_ConstDownCast)(expr))
#define cel_CallArgExpr_DownCast(expr)             \
  (_Generic((expr),                                \
       cel_Expr*: cel_CallArgExpr_MutableDownCast, \
       const cel_Expr*: cel_CallArgExpr_ConstDownCast)(expr))
#define cel_CallExpr_DownCast(expr)             \
  (_Generic((expr),                             \
       cel_Expr*: cel_CallExpr_MutableDownCast, \
       const cel_Expr*: cel_CallExpr_ConstDownCast)(expr))
#define cel_UnaryExpr_DownCast(expr)             \
  (_Generic((expr),                              \
       cel_Expr*: cel_UnaryExpr_MutableDownCast, \
       const cel_Expr*: cel_UnaryExpr_ConstDownCast)(expr))
#define cel_BinaryExpr_DownCast(expr)             \
  (_Generic((expr),                               \
       cel_Expr*: cel_BinaryExpr_MutableDownCast, \
       const cel_Expr*: cel_BinaryExpr_ConstDownCast)(expr))
#define cel_TernaryExpr_DownCast(expr)             \
  (_Generic((expr),                                \
       cel_Expr*: cel_TernaryExpr_MutableDownCast, \
       const cel_Expr*: cel_TernaryExpr_ConstDownCast)(expr))
#define cel_ListElementExpr_DownCast(expr)             \
  (_Generic((expr),                                    \
       cel_Expr*: cel_ListElementExpr_MutableDownCast, \
       const cel_Expr*: cel_ListElementExpr_ConstDownCast)(expr))
#define cel_ListExpr_DownCast(expr)             \
  (_Generic((expr),                             \
       cel_Expr*: cel_ListExpr_MutableDownCast, \
       const cel_Expr*: cel_ListExpr_ConstDownCast)(expr))
#define cel_MapEntryExpr_DownCast(expr)             \
  (_Generic((expr),                                 \
       cel_Expr*: cel_MapEntryExpr_MutableDownCast, \
       const cel_Expr*: cel_MapEntryExpr_ConstDownCast)(expr))
#define cel_MapExpr_DownCast(expr)             \
  (_Generic((expr),                            \
       cel_Expr*: cel_MapExpr_MutableDownCast, \
       const cel_Expr*: cel_MapExpr_ConstDownCast)(expr))
#define cel_StructFieldExpr_DownCast(expr)             \
  (_Generic((expr),                                    \
       cel_Expr*: cel_StructFieldExpr_MutableDownCast, \
       const cel_Expr*: cel_StructFieldExpr_ConstDownCast)(expr))
#define cel_StructExpr_DownCast(expr)             \
  (_Generic((expr),                               \
       cel_Expr*: cel_StructExpr_MutableDownCast, \
       const cel_Expr*: cel_StructExpr_ConstDownCast)(expr))
#define cel_ComprehensionExpr_DownCast(expr)             \
  (_Generic((expr),                                      \
       cel_Expr*: cel_ComprehensionExpr_MutableDownCast, \
       const cel_Expr*: cel_ComprehensionExpr_ConstDownCast)(expr))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_UnspecifiedExpr* cel_nullability_unknown expr) {
  return cel_UnspecifiedExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_UnspecifiedExpr* cel_nullability_unknown expr) {
  return cel_UnspecifiedExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_IdentExpr* cel_nullability_unknown expr) {
  return cel_IdentExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_IdentExpr* cel_nullability_unknown expr) {
  return cel_IdentExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_ConstExpr* cel_nullability_unknown expr) {
  return cel_ConstExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_ConstExpr* cel_nullability_unknown expr) {
  return cel_ConstExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_SelectExpr* cel_nullability_unknown expr) {
  return cel_SelectExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_SelectExpr* cel_nullability_unknown expr) {
  return cel_SelectExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_CallExpr* cel_nullability_unknown expr) {
  return cel_CallExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_CallExpr* cel_nullability_unknown expr) {
  return cel_CallExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_CallArgExpr* cel_nullability_unknown expr) {
  return cel_CallArgExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_CallArgExpr* cel_nullability_unknown expr) {
  return cel_CallArgExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_UnaryExpr* cel_nullability_unknown expr) {
  return cel_UnaryExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_UnaryExpr* cel_nullability_unknown expr) {
  return cel_UnaryExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_BinaryExpr* cel_nullability_unknown expr) {
  return cel_BinaryExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_BinaryExpr* cel_nullability_unknown expr) {
  return cel_BinaryExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_TernaryExpr* cel_nullability_unknown expr) {
  return cel_TernaryExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_TernaryExpr* cel_nullability_unknown expr) {
  return cel_TernaryExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_ListElementExpr* cel_nullability_unknown expr) {
  return cel_ListElementExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_ListElementExpr* cel_nullability_unknown expr) {
  return cel_ListElementExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_ListExpr* cel_nullability_unknown expr) {
  return cel_ListExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_ListExpr* cel_nullability_unknown expr) {
  return cel_ListExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_MapEntryExpr* cel_nullability_unknown expr) {
  return cel_MapEntryExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_MapEntryExpr* cel_nullability_unknown expr) {
  return cel_MapEntryExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_MapExpr* cel_nullability_unknown expr) {
  return cel_MapExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_MapExpr* cel_nullability_unknown expr) {
  return cel_MapExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_StructFieldExpr* cel_nullability_unknown expr) {
  return cel_StructFieldExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_StructFieldExpr* cel_nullability_unknown expr) {
  return cel_StructFieldExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_StructExpr* cel_nullability_unknown expr) {
  return cel_StructExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_StructExpr* cel_nullability_unknown expr) {
  return cel_StructExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(const cel_ComprehensionExpr* cel_nullability_unknown expr) {
  return cel_ComprehensionExpr_ConstUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_Expr* cel_nullability_unknown
cel_Expr_UpCast(cel_ComprehensionExpr* cel_nullability_unknown expr) {
  return cel_ComprehensionExpr_MutableUpCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_UnspecifiedExpr* cel_nullability_unknown
cel_UnspecifiedExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_UnspecifiedExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_UnspecifiedExpr* cel_nullability_unknown
cel_UnspecifiedExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_UnspecifiedExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_IdentExpr* cel_nullability_unknown
cel_IdentExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_IdentExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_IdentExpr* cel_nullability_unknown
cel_IdentExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_IdentExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_ConstExpr* cel_nullability_unknown
cel_ConstExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_ConstExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_ConstExpr* cel_nullability_unknown
cel_ConstExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_ConstExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_SelectExpr* cel_nullability_unknown
cel_SelectExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_SelectExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_SelectExpr* cel_nullability_unknown
cel_SelectExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_SelectExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_CallExpr* cel_nullability_unknown
cel_CallExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_CallExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_CallExpr* cel_nullability_unknown
cel_CallExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_CallExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_CallArgExpr* cel_nullability_unknown
cel_CallArgExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_CallArgExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_CallArgExpr* cel_nullability_unknown
cel_CallArgExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_CallArgExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_UnaryExpr* cel_nullability_unknown
cel_UnaryExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_UnaryExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_UnaryExpr* cel_nullability_unknown
cel_UnaryExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_UnaryExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_BinaryExpr* cel_nullability_unknown
cel_BinaryExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_BinaryExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_BinaryExpr* cel_nullability_unknown
cel_BinaryExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_BinaryExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_TernaryExpr* cel_nullability_unknown
cel_TernaryExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_TernaryExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_TernaryExpr* cel_nullability_unknown
cel_TernaryExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_TernaryExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_ListElementExpr* cel_nullability_unknown
cel_ListElementExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_ListElementExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_ListElementExpr* cel_nullability_unknown
cel_ListElementExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_ListElementExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_ListExpr* cel_nullability_unknown
cel_ListExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_ListExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_ListExpr* cel_nullability_unknown
cel_ListExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_ListExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_MapEntryExpr* cel_nullability_unknown
cel_MapEntryExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_MapEntryExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_MapEntryExpr* cel_nullability_unknown
cel_MapEntryExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_MapEntryExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_MapExpr* cel_nullability_unknown
cel_MapExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_MapExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_MapExpr* cel_nullability_unknown
cel_MapExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_MapExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_StructFieldExpr* cel_nullability_unknown
cel_StructFieldExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_StructFieldExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_StructFieldExpr* cel_nullability_unknown
cel_StructFieldExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_StructFieldExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_StructExpr* cel_nullability_unknown
cel_StructExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_StructExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_StructExpr* cel_nullability_unknown
cel_StructExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_StructExpr_MutableDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const cel_ComprehensionExpr* cel_nullability_unknown
cel_ComprehensionExpr_DownCast(const cel_Expr* cel_nullability_unknown expr) {
  return cel_ComprehensionExpr_ConstDownCast(expr);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_ComprehensionExpr* cel_nullability_unknown
cel_ComprehensionExpr_DownCast(cel_Expr* cel_nullability_unknown expr) {
  return cel_ComprehensionExpr_MutableDownCast(expr);
}
#endif

#endif  // THIRD_PARTY_CEL_C_AST_H_
