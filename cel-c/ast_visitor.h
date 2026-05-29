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

#ifndef THIRD_PARTY_CEL_C_AST_VISITOR_H_
#define THIRD_PARTY_CEL_C_AST_VISITOR_H_

#include <stddef.h>

#include "cel-c/ast.h"
#include "cel-c/config.h"

CEL_BEGIN_DECLS

typedef struct cel_AstVisitorVTable cel_AstVisitorVTable;
typedef struct cel_AstVisitor cel_AstVisitor;

typedef void cel_AstVisitorVTable_PreVisitExpr(CEL_NONNULL(cel_AstVisitor*),
                                               CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitExpr(CEL_NONNULL(cel_AstVisitor*),
                                                CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_VisitUnspecifiedExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_UnspecifiedExpr*));

typedef void cel_AstVisitorVTable_VisitIdentExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_IdentExpr*));

typedef void cel_AstVisitorVTable_VisitConstExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_ConstExpr*));

typedef void cel_AstVisitorVTable_PreVisitSelectExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_SelectExpr*));
typedef void cel_AstVisitorVTable_PostVisitSelectExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_SelectExpr*));

typedef void cel_AstVisitorVTable_PreVisitSelectExprOperand(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitSelectExprOperand(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitUnaryExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_UnaryExpr*));
typedef void cel_AstVisitorVTable_PostVisitUnaryExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_UnaryExpr*));

typedef void cel_AstVisitorVTable_PreVisitUnaryExprArg(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitUnaryExprArg(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitBinaryExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_BinaryExpr*));
typedef void cel_AstVisitorVTable_PostVisitBinaryExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_BinaryExpr*));

typedef void cel_AstVisitorVTable_PreVisitBinaryExprLeft(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitBinaryExprLeft(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitBinaryExprRight(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitBinaryExprRight(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitTernaryExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_TernaryExpr*));
typedef void cel_AstVisitorVTable_PostVisitTernaryExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_TernaryExpr*));

typedef void cel_AstVisitorVTable_PreVisitTernaryExprCondition(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitTernaryExprCondition(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitTernaryExprIfTrue(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitTernaryExprIfTrue(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitTernaryExprIfFalse(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitTernaryExprIfFalse(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitCallArgExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_CallArgExpr*));
typedef void cel_AstVisitorVTable_PostVisitCallArgExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_CallArgExpr*));

typedef void cel_AstVisitorVTable_PreVisitCallArgExprValue(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitCallArgExprValue(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitCallExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_CallExpr*));
typedef void cel_AstVisitorVTable_PostVisitCallExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_CallExpr*));

typedef void cel_AstVisitorVTable_PreVisitCallExprTarget(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitCallExprTarget(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitListElementExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_ListElementExpr*));
typedef void cel_AstVisitorVTable_PostVisitListElementExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_ListElementExpr*));

typedef void cel_AstVisitorVTable_PreVisitListElementExprValue(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitListElementExprValue(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitListExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_ListExpr*));
typedef void cel_AstVisitorVTable_PostVisitListExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_ListExpr*));

typedef void cel_AstVisitorVTable_PreVisitMapEntryExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_MapEntryExpr*));
typedef void cel_AstVisitorVTable_PostVisitMapEntryExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_MapEntryExpr*));

typedef void cel_AstVisitorVTable_PreVisitMapEntryExprKey(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitMapEntryExprKey(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitMapEntryExprValue(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitMapEntryExprValue(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitMapExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_MapExpr*));
typedef void cel_AstVisitorVTable_PostVisitMapExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_MapExpr*));

typedef void cel_AstVisitorVTable_PreVisitStructFieldExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_StructFieldExpr*));
typedef void cel_AstVisitorVTable_PostVisitStructFieldExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_StructFieldExpr*));

typedef void cel_AstVisitorVTable_PreVisitStructFieldExprValue(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitStructFieldExprValue(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitStructExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_StructExpr*));
typedef void cel_AstVisitorVTable_PostVisitStructExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_StructExpr*));

typedef void cel_AstVisitorVTable_PreVisitComprehensionExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_ComprehensionExpr*));
typedef void cel_AstVisitorVTable_PostVisitComprehensionExpr(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_ComprehensionExpr*));

typedef void cel_AstVisitorVTable_PreVisitComprehensionExprIterRange(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitComprehensionExprIterRange(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitComprehensionExprAccuInit(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitComprehensionExprAccuInit(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitComprehensionExprLoopCondition(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitComprehensionExprLoopCondition(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitComprehensionExprLoopStep(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitComprehensionExprLoopStep(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

typedef void cel_AstVisitorVTable_PreVisitComprehensionExprResult(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));
typedef void cel_AstVisitorVTable_PostVisitComprehensionExprResult(
    CEL_NONNULL(cel_AstVisitor*), CEL_NONNULL(const cel_Expr*));

struct cel_AstVisitorVTable {
  // NOLINTBEGIN(google3-readability-class-member-naming)
  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitExpr*) PreVisitExpr;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitExpr*) PostVisitExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_VisitUnspecifiedExpr*) VisitUnspecifiedExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_VisitIdentExpr*) VisitIdentExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_VisitConstExpr*) VisitConstExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitSelectExpr*) PreVisitSelectExpr;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitSelectExpr*) PostVisitSelectExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitSelectExprOperand*)
  PreVisitSelectExprOperand;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitSelectExprOperand*)
  PostVisitSelectExprOperand;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitUnaryExpr*) PreVisitUnaryExpr;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitUnaryExpr*) PostVisitUnaryExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitUnaryExprArg*) PreVisitUnaryExprArg;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitUnaryExprArg*)
  PostVisitUnaryExprArg;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitBinaryExpr*) PreVisitBinaryExpr;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitBinaryExpr*) PostVisitBinaryExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitBinaryExprLeft*)
  PreVisitBinaryExprLeft;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitBinaryExprLeft*)
  PostVisitBinaryExprLeft;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitBinaryExprRight*)
  PreVisitBinaryExprRight;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitBinaryExprRight*)
  PostVisitBinaryExprRight;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitTernaryExpr*) PreVisitTernaryExpr;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitTernaryExpr*) PostVisitTernaryExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitTernaryExprCondition*)
  PreVisitTernaryExprCondition;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitTernaryExprCondition*)
  PostVisitTernaryExprCondition;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitTernaryExprIfTrue*)
  PreVisitTernaryExprIfTrue;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitTernaryExprIfTrue*)
  PostVisitTernaryExprIfTrue;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitTernaryExprIfFalse*)
  PreVisitTernaryExprIfFalse;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitTernaryExprIfFalse*)
  PostVisitTernaryExprIfFalse;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitCallArgExpr*) PreVisitCallArgExpr;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitCallArgExpr*) PostVisitCallArgExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitCallArgExprValue*)
  PreVisitCallArgExprValue;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitCallArgExprValue*)
  PostVisitCallArgExprValue;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitCallExpr*) PreVisitCallExpr;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitCallExpr*) PostVisitCallExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitCallExprTarget*)
  PreVisitCallExprTarget;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitCallExprTarget*)
  PostVisitCallExprTarget;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitListElementExpr*)
  PreVisitListElementExpr;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitListElementExpr*)
  PostVisitListElementExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitListElementExprValue*)
  PreVisitListElementExprValue;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitListElementExprValue*)
  PostVisitListElementExprValue;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitListExpr*) PreVisitListExpr;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitListExpr*) PostVisitListExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitMapEntryExpr*) PreVisitMapEntryExpr;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitMapEntryExpr*)
  PostVisitMapEntryExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitMapEntryExprKey*)
  PreVisitMapEntryExprKey;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitMapEntryExprKey*)
  PostVisitMapEntryExprKey;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitMapEntryExprValue*)
  PreVisitMapEntryExprValue;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitMapEntryExprValue*)
  PostVisitMapEntryExprValue;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitMapExpr*) PreVisitMapExpr;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitMapExpr*) PostVisitMapExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitStructFieldExpr*)
  PreVisitStructFieldExpr;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitStructFieldExpr*)
  PostVisitStructFieldExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitStructFieldExprValue*)
  PreVisitStructFieldExprValue;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitStructFieldExprValue*)
  PostVisitStructFieldExprValue;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitStructExpr*) PreVisitStructExpr;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitStructExpr*) PostVisitStructExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitComprehensionExpr*)
  PreVisitComprehensionExpr;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitComprehensionExpr*)
  PostVisitComprehensionExpr;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitComprehensionExprIterRange*)
  PreVisitComprehensionExprIterRange;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitComprehensionExprIterRange*)
  PostVisitComprehensionExprIterRange;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitComprehensionExprAccuInit*)
  PreVisitComprehensionExprAccuInit;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitComprehensionExprAccuInit*)
  PostVisitComprehensionExprAccuInit;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitComprehensionExprLoopCondition*)
  PreVisitComprehensionExprLoopCondition;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitComprehensionExprLoopCondition*)
  PostVisitComprehensionExprLoopCondition;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitComprehensionExprLoopStep*)
  PreVisitComprehensionExprLoopStep;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitComprehensionExprLoopStep*)
  PostVisitComprehensionExprLoopStep;

  CEL_NULLABLE(cel_AstVisitorVTable_PreVisitComprehensionExprResult*)
  PreVisitComprehensionExprResult;
  CEL_NULLABLE(cel_AstVisitorVTable_PostVisitComprehensionExprResult*)
  PostVisitComprehensionExprResult;
  // NOLINTEND(google3-readability-class-member-naming)
};

struct cel_AstVisitor {
  CEL_NONNULL(const cel_AstVisitorVTable*) vtable;
};

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_AST_VISITOR_H_
