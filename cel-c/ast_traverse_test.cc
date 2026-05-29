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

#include "cel-c/ast_traverse.h"

#include <memory>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/ast.h"
#include "cel-c/ast_visitor.h"
#include "cel-c/config.h"
#include "cel-c/status.h"

namespace {

using ::testing::InSequence;
using ::testing::NotNull;
using ::testing::Return;

struct _cel_AstTraverserDeleter {
  void operator()(const cel_AstTraverser* ast_traverser) const {
    if (ast_traverser != cel_nullptr) {
      cel_AstTraverser_Delete(const_cast<cel_AstTraverser*>(ast_traverser));
    }
  }
};

using _cel_AstTraverserPtr =
    std::unique_ptr<cel_AstTraverser, _cel_AstTraverserDeleter>;

class AstVisitor : public cel_AstVisitor {
 public:
  explicit AstVisitor(const cel_AstVisitorVTable* vtable_ptr) {
    this->vtable = vtable_ptr;
  }

  virtual ~AstVisitor() = default;

  virtual void PreVisitExpr(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitExpr(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void VisitUnspecifiedExpr(
      CEL_NONNULL(const cel_UnspecifiedExpr*)) = 0;
  virtual void VisitIdentExpr(CEL_NONNULL(const cel_IdentExpr*)) = 0;
  virtual void VisitConstExpr(CEL_NONNULL(const cel_ConstExpr*)) = 0;
  virtual void PreVisitSelectExpr(CEL_NONNULL(const cel_SelectExpr*)) = 0;
  virtual void PostVisitSelectExpr(CEL_NONNULL(const cel_SelectExpr*)) = 0;
  virtual void PreVisitSelectExprOperand(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitSelectExprOperand(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitCallArgExpr(CEL_NONNULL(const cel_CallArgExpr*)) = 0;
  virtual void PostVisitCallArgExpr(CEL_NONNULL(const cel_CallArgExpr*)) = 0;
  virtual void PreVisitCallArgExprValue(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitCallArgExprValue(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitCallExpr(CEL_NONNULL(const cel_CallExpr*)) = 0;
  virtual void PostVisitCallExpr(CEL_NONNULL(const cel_CallExpr*)) = 0;
  virtual void PreVisitCallExprTarget(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitCallExprTarget(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitListElementExpr(
      CEL_NONNULL(const cel_ListElementExpr*)) = 0;
  virtual void PostVisitListElementExpr(
      CEL_NONNULL(const cel_ListElementExpr*)) = 0;
  virtual void PreVisitListElementExprValue(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitListElementExprValue(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitListExpr(CEL_NONNULL(const cel_ListExpr*)) = 0;
  virtual void PostVisitListExpr(CEL_NONNULL(const cel_ListExpr*)) = 0;
  virtual void PreVisitMapEntryExpr(CEL_NONNULL(const cel_MapEntryExpr*)) = 0;
  virtual void PostVisitMapEntryExpr(CEL_NONNULL(const cel_MapEntryExpr*)) = 0;
  virtual void PreVisitMapEntryExprKey(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitMapEntryExprKey(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitMapEntryExprValue(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitMapEntryExprValue(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitMapExpr(CEL_NONNULL(const cel_MapExpr*)) = 0;
  virtual void PostVisitMapExpr(CEL_NONNULL(const cel_MapExpr*)) = 0;
  virtual void PreVisitStructFieldExpr(
      CEL_NONNULL(const cel_StructFieldExpr*)) = 0;
  virtual void PostVisitStructFieldExpr(
      CEL_NONNULL(const cel_StructFieldExpr*)) = 0;
  virtual void PreVisitStructFieldExprValue(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitStructFieldExprValue(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitStructExpr(CEL_NONNULL(const cel_StructExpr*)) = 0;
  virtual void PostVisitStructExpr(CEL_NONNULL(const cel_StructExpr*)) = 0;
  virtual void PreVisitComprehensionExpr(
      CEL_NONNULL(const cel_ComprehensionExpr*)) = 0;
  virtual void PostVisitComprehensionExpr(
      CEL_NONNULL(const cel_ComprehensionExpr*)) = 0;
  virtual void PreVisitComprehensionExprIterRange(
      CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitComprehensionExprIterRange(
      CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitComprehensionExprAccuInit(
      CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitComprehensionExprAccuInit(
      CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitComprehensionExprLoopCondition(
      CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitComprehensionExprLoopCondition(
      CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitComprehensionExprLoopStep(
      CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitComprehensionExprLoopStep(
      CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitComprehensionExprResult(
      CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitComprehensionExprResult(
      CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitUnaryExpr(CEL_NONNULL(const cel_UnaryExpr*)) = 0;
  virtual void PostVisitUnaryExpr(CEL_NONNULL(const cel_UnaryExpr*)) = 0;
  virtual void PreVisitUnaryExprArg(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitUnaryExprArg(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitBinaryExpr(CEL_NONNULL(const cel_BinaryExpr*)) = 0;
  virtual void PostVisitBinaryExpr(CEL_NONNULL(const cel_BinaryExpr*)) = 0;
  virtual void PreVisitBinaryExprLeft(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitBinaryExprLeft(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitBinaryExprRight(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitBinaryExprRight(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitTernaryExpr(CEL_NONNULL(const cel_TernaryExpr*)) = 0;
  virtual void PostVisitTernaryExpr(CEL_NONNULL(const cel_TernaryExpr*)) = 0;
  virtual void PreVisitTernaryExprCondition(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitTernaryExprCondition(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitTernaryExprIfTrue(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitTernaryExprIfTrue(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PreVisitTernaryExprIfFalse(CEL_NONNULL(const cel_Expr*)) = 0;
  virtual void PostVisitTernaryExprIfFalse(CEL_NONNULL(const cel_Expr*)) = 0;

  bool Traverse(CEL_NONNULL(const cel_Ast*) ast,
                CEL_NONNULL(cel_Status*) status) {
    if (!cel_Status_Ok(status)) {
      return false;
    }
    _cel_AstTraverserPtr ast_traverser(cel_AstTraverser_New(ast, this));
    if (ast_traverser == cel_nullptr) {
      cel_OutOfMemoryStatus(status);
      return false;
    }
    return Traverse(ast_traverser.get(), status);
  }

  bool Traverse(CEL_NONNULL(cel_AstTraverser*) ast_traverser,
                CEL_NONNULL(cel_Status*) status) {
    if (!cel_Status_Ok(status)) {
      return false;
    }
    while (cel_AstTraverser_Traverse(ast_traverser, status)) {
    }
    return cel_Status_Ok(status);
  }

  static AstVisitor* ContainerOf(cel_AstVisitor* visitor) {
    return static_cast<AstVisitor*>(visitor);
  }
};

template <typename T>
const cel_AstVisitorVTable* MakeAstVisitorVTable() {
  static const cel_AstVisitorVTable kVtable = {
      .PreVisitExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                         CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitExpr(expr);
      },
      .PostVisitExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                          CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitExpr(expr);
      },
      .VisitUnspecifiedExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                 CEL_NONNULL(const cel_UnspecifiedExpr*)
                                     expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->VisitUnspecifiedExpr(expr);
      },
      .VisitIdentExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                           CEL_NONNULL(const cel_IdentExpr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->VisitIdentExpr(expr);
      },
      .VisitConstExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                           CEL_NONNULL(const cel_ConstExpr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->VisitConstExpr(expr);
      },
      .PreVisitSelectExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                               CEL_NONNULL(const cel_SelectExpr*)
                                   expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitSelectExpr(expr);
      },
      .PostVisitSelectExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                CEL_NONNULL(const cel_SelectExpr*)
                                    expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitSelectExpr(expr);
      },
      .PreVisitSelectExprOperand = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                      CEL_NONNULL(const cel_Expr*)
                                          expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitSelectExprOperand(expr);
      },
      .PostVisitSelectExprOperand = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                       CEL_NONNULL(const cel_Expr*)
                                           expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitSelectExprOperand(expr);
      },
      .PreVisitUnaryExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                              CEL_NONNULL(const cel_UnaryExpr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitUnaryExpr(expr);
      },
      .PostVisitUnaryExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                               CEL_NONNULL(const cel_UnaryExpr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitUnaryExpr(expr);
      },
      .PreVisitUnaryExprArg = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                 CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitUnaryExprArg(expr);
      },
      .PostVisitUnaryExprArg = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                  CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitUnaryExprArg(expr);
      },
      .PreVisitBinaryExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                               CEL_NONNULL(const cel_BinaryExpr*)
                                   expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitBinaryExpr(expr);
      },
      .PostVisitBinaryExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                CEL_NONNULL(const cel_BinaryExpr*)
                                    expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitBinaryExpr(expr);
      },
      .PreVisitBinaryExprLeft = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                   CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitBinaryExprLeft(expr);
      },
      .PostVisitBinaryExprLeft = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                    CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitBinaryExprLeft(expr);
      },
      .PreVisitBinaryExprRight = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                    CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitBinaryExprRight(expr);
      },
      .PostVisitBinaryExprRight = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                     CEL_NONNULL(const cel_Expr*)
                                         expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitBinaryExprRight(expr);
      },
      .PreVisitTernaryExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                CEL_NONNULL(const cel_TernaryExpr*)
                                    expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitTernaryExpr(expr);
      },
      .PostVisitTernaryExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                 CEL_NONNULL(const cel_TernaryExpr*)
                                     expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitTernaryExpr(expr);
      },
      .PreVisitTernaryExprCondition = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                         CEL_NONNULL(const cel_Expr*)
                                             expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitTernaryExprCondition(expr);
      },
      .PostVisitTernaryExprCondition = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                          CEL_NONNULL(const cel_Expr*)
                                              expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitTernaryExprCondition(expr);
      },
      .PreVisitTernaryExprIfTrue = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                      CEL_NONNULL(const cel_Expr*)
                                          expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitTernaryExprIfTrue(expr);
      },
      .PostVisitTernaryExprIfTrue = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                       CEL_NONNULL(const cel_Expr*)
                                           expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitTernaryExprIfTrue(expr);
      },
      .PreVisitTernaryExprIfFalse = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                       CEL_NONNULL(const cel_Expr*)
                                           expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitTernaryExprIfFalse(expr);
      },
      .PostVisitTernaryExprIfFalse = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                        CEL_NONNULL(const cel_Expr*)
                                            expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitTernaryExprIfFalse(expr);
      },
      .PreVisitCallArgExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                CEL_NONNULL(const cel_CallArgExpr*)
                                    expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitCallArgExpr(expr);
      },
      .PostVisitCallArgExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                 CEL_NONNULL(const cel_CallArgExpr*)
                                     expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitCallArgExpr(expr);
      },
      .PreVisitCallArgExprValue = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                     CEL_NONNULL(const cel_Expr*)
                                         expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitCallArgExprValue(expr);
      },
      .PostVisitCallArgExprValue = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                      CEL_NONNULL(const cel_Expr*)
                                          expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitCallArgExprValue(expr);
      },
      .PreVisitCallExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                             CEL_NONNULL(const cel_CallExpr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitCallExpr(expr);
      },
      .PostVisitCallExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                              CEL_NONNULL(const cel_CallExpr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitCallExpr(expr);
      },
      .PreVisitCallExprTarget = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                   CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitCallExprTarget(expr);
      },
      .PostVisitCallExprTarget = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                    CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitCallExprTarget(expr);
      },
      .PreVisitListElementExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                    CEL_NONNULL(const cel_ListElementExpr*)
                                        expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitListElementExpr(expr);
      },
      .PostVisitListElementExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                     CEL_NONNULL(const cel_ListElementExpr*)
                                         expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitListElementExpr(expr);
      },
      .PreVisitListElementExprValue = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                         CEL_NONNULL(const cel_Expr*)
                                             expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitListElementExprValue(expr);
      },
      .PostVisitListElementExprValue = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                          CEL_NONNULL(const cel_Expr*)
                                              expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitListElementExprValue(expr);
      },
      .PreVisitListExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                             CEL_NONNULL(const cel_ListExpr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitListExpr(expr);
      },
      .PostVisitListExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                              CEL_NONNULL(const cel_ListExpr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitListExpr(expr);
      },
      .PreVisitMapEntryExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                 CEL_NONNULL(const cel_MapEntryExpr*)
                                     expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitMapEntryExpr(expr);
      },
      .PostVisitMapEntryExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                  CEL_NONNULL(const cel_MapEntryExpr*)
                                      expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitMapEntryExpr(expr);
      },
      .PreVisitMapEntryExprKey = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                    CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitMapEntryExprKey(expr);
      },
      .PostVisitMapEntryExprKey = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                     CEL_NONNULL(const cel_Expr*)
                                         expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitMapEntryExprKey(expr);
      },
      .PreVisitMapEntryExprValue = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                      CEL_NONNULL(const cel_Expr*)
                                          expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitMapEntryExprValue(expr);
      },
      .PostVisitMapEntryExprValue = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                       CEL_NONNULL(const cel_Expr*)
                                           expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitMapEntryExprValue(expr);
      },
      .PreVisitMapExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                            CEL_NONNULL(const cel_MapExpr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitMapExpr(expr);
      },
      .PostVisitMapExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                             CEL_NONNULL(const cel_MapExpr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitMapExpr(expr);
      },
      .PreVisitStructFieldExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                    CEL_NONNULL(const cel_StructFieldExpr*)
                                        expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitStructFieldExpr(expr);
      },
      .PostVisitStructFieldExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                     CEL_NONNULL(const cel_StructFieldExpr*)
                                         expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitStructFieldExpr(expr);
      },
      .PreVisitStructFieldExprValue = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                         CEL_NONNULL(const cel_Expr*)
                                             expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitStructFieldExprValue(expr);
      },
      .PostVisitStructFieldExprValue = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                          CEL_NONNULL(const cel_Expr*)
                                              expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitStructFieldExprValue(expr);
      },
      .PreVisitStructExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                               CEL_NONNULL(const cel_StructExpr*)
                                   expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitStructExpr(expr);
      },
      .PostVisitStructExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                CEL_NONNULL(const cel_StructExpr*)
                                    expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitStructExpr(expr);
      },
      .PreVisitComprehensionExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                      CEL_NONNULL(const cel_ComprehensionExpr*)
                                          expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitComprehensionExpr(expr);
      },
      .PostVisitComprehensionExpr = [](CEL_NONNULL(cel_AstVisitor*) visitor,
                                       CEL_NONNULL(const cel_ComprehensionExpr*)
                                           expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitComprehensionExpr(expr);
      },
      .PreVisitComprehensionExprIterRange =
          [](CEL_NONNULL(cel_AstVisitor*) visitor,
             CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitComprehensionExprIterRange(expr);
      },
      .PostVisitComprehensionExprIterRange =
          [](CEL_NONNULL(cel_AstVisitor*) visitor,
             CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitComprehensionExprIterRange(expr);
      },
      .PreVisitComprehensionExprAccuInit =
          [](CEL_NONNULL(cel_AstVisitor*) visitor,
             CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitComprehensionExprAccuInit(expr);
      },
      .PostVisitComprehensionExprAccuInit =
          [](CEL_NONNULL(cel_AstVisitor*) visitor,
             CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitComprehensionExprAccuInit(expr);
      },
      .PreVisitComprehensionExprLoopCondition =
          [](CEL_NONNULL(cel_AstVisitor*) visitor,
             CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitComprehensionExprLoopCondition(expr);
      },
      .PostVisitComprehensionExprLoopCondition =
          [](CEL_NONNULL(cel_AstVisitor*) visitor,
             CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitComprehensionExprLoopCondition(expr);
      },
      .PreVisitComprehensionExprLoopStep =
          [](CEL_NONNULL(cel_AstVisitor*) visitor,
             CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitComprehensionExprLoopStep(expr);
      },
      .PostVisitComprehensionExprLoopStep =
          [](CEL_NONNULL(cel_AstVisitor*) visitor,
             CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitComprehensionExprLoopStep(expr);
      },
      .PreVisitComprehensionExprResult =
          [](CEL_NONNULL(cel_AstVisitor*) visitor,
             CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PreVisitComprehensionExprResult(expr);
      },
      .PostVisitComprehensionExprResult =
          [](CEL_NONNULL(cel_AstVisitor*) visitor,
             CEL_NONNULL(const cel_Expr*) expr) -> void {
        return static_cast<T*>(AstVisitor::ContainerOf(visitor))
            ->PostVisitComprehensionExprResult(expr);
      },
  };
  return &kVtable;
}

class MockAstVisitor : public AstVisitor {
 public:
  MockAstVisitor() : AstVisitor(MakeAstVisitorVTable<MockAstVisitor>()) {}

  MOCK_METHOD(void, PreVisitExpr, (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PostVisitExpr, (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, VisitUnspecifiedExpr,
              (CEL_NONNULL(const cel_UnspecifiedExpr*)), (override));
  MOCK_METHOD(void, VisitIdentExpr, (CEL_NONNULL(const cel_IdentExpr*)),
              (override));
  MOCK_METHOD(void, VisitConstExpr, (CEL_NONNULL(const cel_ConstExpr*)),
              (override));
  MOCK_METHOD(void, PreVisitSelectExpr, (CEL_NONNULL(const cel_SelectExpr*)),
              (override));
  MOCK_METHOD(void, PostVisitSelectExpr, (CEL_NONNULL(const cel_SelectExpr*)),
              (override));
  MOCK_METHOD(void, PreVisitSelectExprOperand, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PostVisitSelectExprOperand, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PreVisitCallArgExpr, (CEL_NONNULL(const cel_CallArgExpr*)),
              (override));
  MOCK_METHOD(void, PostVisitCallArgExpr, (CEL_NONNULL(const cel_CallArgExpr*)),
              (override));
  MOCK_METHOD(void, PreVisitCallArgExprValue, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PostVisitCallArgExprValue, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PreVisitCallExpr, (CEL_NONNULL(const cel_CallExpr*)),
              (override));
  MOCK_METHOD(void, PostVisitCallExpr, (CEL_NONNULL(const cel_CallExpr*)),
              (override));
  MOCK_METHOD(void, PreVisitCallExprTarget, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PostVisitCallExprTarget, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PreVisitListElementExpr,
              (CEL_NONNULL(const cel_ListElementExpr*)), (override));
  MOCK_METHOD(void, PostVisitListElementExpr,
              (CEL_NONNULL(const cel_ListElementExpr*)), (override));
  MOCK_METHOD(void, PreVisitListElementExprValue,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PostVisitListElementExprValue,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PreVisitListExpr, (CEL_NONNULL(const cel_ListExpr*)),
              (override));
  MOCK_METHOD(void, PostVisitListExpr, (CEL_NONNULL(const cel_ListExpr*)),
              (override));
  MOCK_METHOD(void, PreVisitMapEntryExpr,
              (CEL_NONNULL(const cel_MapEntryExpr*)), (override));
  MOCK_METHOD(void, PostVisitMapEntryExpr,
              (CEL_NONNULL(const cel_MapEntryExpr*)), (override));
  MOCK_METHOD(void, PreVisitMapEntryExprKey, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PostVisitMapEntryExprKey, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PreVisitMapEntryExprValue, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PostVisitMapEntryExprValue, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PreVisitMapExpr, (CEL_NONNULL(const cel_MapExpr*)),
              (override));
  MOCK_METHOD(void, PostVisitMapExpr, (CEL_NONNULL(const cel_MapExpr*)),
              (override));
  MOCK_METHOD(void, PreVisitStructFieldExpr,
              (CEL_NONNULL(const cel_StructFieldExpr*)), (override));
  MOCK_METHOD(void, PostVisitStructFieldExpr,
              (CEL_NONNULL(const cel_StructFieldExpr*)), (override));
  MOCK_METHOD(void, PreVisitStructFieldExprValue,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PostVisitStructFieldExprValue,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PreVisitStructExpr, (CEL_NONNULL(const cel_StructExpr*)),
              (override));
  MOCK_METHOD(void, PostVisitStructExpr, (CEL_NONNULL(const cel_StructExpr*)),
              (override));
  MOCK_METHOD(void, PreVisitComprehensionExpr,
              (CEL_NONNULL(const cel_ComprehensionExpr*)), (override));
  MOCK_METHOD(void, PostVisitComprehensionExpr,
              (CEL_NONNULL(const cel_ComprehensionExpr*)), (override));
  MOCK_METHOD(void, PreVisitComprehensionExprIterRange,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PostVisitComprehensionExprIterRange,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PreVisitComprehensionExprAccuInit,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PostVisitComprehensionExprAccuInit,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PreVisitComprehensionExprLoopCondition,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PostVisitComprehensionExprLoopCondition,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PreVisitComprehensionExprLoopStep,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PostVisitComprehensionExprLoopStep,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PreVisitComprehensionExprResult,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PostVisitComprehensionExprResult,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PreVisitUnaryExpr, (CEL_NONNULL(const cel_UnaryExpr*)),
              (override));
  MOCK_METHOD(void, PostVisitUnaryExpr, (CEL_NONNULL(const cel_UnaryExpr*)),
              (override));
  MOCK_METHOD(void, PreVisitUnaryExprArg, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PostVisitUnaryExprArg, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PreVisitBinaryExpr, (CEL_NONNULL(const cel_BinaryExpr*)),
              (override));
  MOCK_METHOD(void, PostVisitBinaryExpr, (CEL_NONNULL(const cel_BinaryExpr*)),
              (override));
  MOCK_METHOD(void, PreVisitBinaryExprLeft, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PostVisitBinaryExprLeft, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PreVisitBinaryExprRight, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PostVisitBinaryExprRight, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PreVisitTernaryExpr, (CEL_NONNULL(const cel_TernaryExpr*)),
              (override));
  MOCK_METHOD(void, PostVisitTernaryExpr, (CEL_NONNULL(const cel_TernaryExpr*)),
              (override));
  MOCK_METHOD(void, PreVisitTernaryExprCondition,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PostVisitTernaryExprCondition,
              (CEL_NONNULL(const cel_Expr*)), (override));
  MOCK_METHOD(void, PreVisitTernaryExprIfTrue, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PostVisitTernaryExprIfTrue, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PreVisitTernaryExprIfFalse, (CEL_NONNULL(const cel_Expr*)),
              (override));
  MOCK_METHOD(void, PostVisitTernaryExprIfFalse, (CEL_NONNULL(const cel_Expr*)),
              (override));
};

struct AstDeleter {
  void operator()(cel_Ast* ast) const { cel_Ast_Delete(ast); }
};

using AstPtr = std::unique_ptr<cel_Ast, AstDeleter>;

class AstTraverseTest : public ::testing::Test {
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
  CEL_NONNULL(cel_Status*) status() { return &status_; }

  CEL_NONNULL(cel_Allocator*) alloc() { return cel_DefaultAllocator; }

  CEL_NONNULL(cel_Arena*) arena() { return ABSL_DIE_IF_NULL(arena_); }

 private:
  cel_Status status_;
  CEL_NULLABILITY_UNKNOWN(cel_Arena*) arena_ = nullptr;
};

TEST_F(AstTraverseTest, Empty) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  MockAstVisitor visitor;
  EXPECT_TRUE(visitor.Traverse(ast.get(), status()));
}

TEST_F(AstTraverseTest, Unspecified) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_UnspecifiedExpr* expr = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(expr)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast.get(), status()));
}

TEST_F(AstTraverseTest, Ident) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_IdentExpr* expr = cel_IdentExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());
  EXPECT_CALL(visitor, VisitIdentExpr(expr)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast.get(), status()));
}

TEST_F(AstTraverseTest, Const) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_ConstExpr* expr = cel_ConstExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());
  EXPECT_CALL(visitor, VisitConstExpr(expr)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast.get(), status()));
}

TEST_F(AstTraverseTest, Select) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_SelectExpr* expr = cel_SelectExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  cel_UnspecifiedExpr* operand = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(operand, NotNull());
  cel_SelectExpr_SetOperand(expr, cel_Expr_UpCast(operand));
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitSelectExpr(expr)).WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitSelectExprOperand(cel_Expr_UpCast(operand)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(operand)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(operand)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(operand)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitSelectExprOperand(cel_Expr_UpCast(operand)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PostVisitSelectExpr(expr)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast.get(), status()));
}

TEST_F(AstTraverseTest, Unary) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_UnaryExpr* expr = cel_UnaryExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  cel_UnspecifiedExpr* arg = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(arg, NotNull());
  cel_UnaryExpr_SetArg(expr, cel_Expr_UpCast(arg));
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitUnaryExpr(expr)).WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitUnaryExprArg(cel_Expr_UpCast(arg)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(arg))).WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(arg)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(arg))).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitUnaryExprArg(cel_Expr_UpCast(arg)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PostVisitUnaryExpr(expr)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast.get(), status()));
}

TEST_F(AstTraverseTest, Binary) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_BinaryExpr* expr = cel_BinaryExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  cel_UnspecifiedExpr* left = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(left, NotNull());
  cel_BinaryExpr_SetLeft(expr, cel_Expr_UpCast(left));
  cel_UnspecifiedExpr* right = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(right, NotNull());
  cel_BinaryExpr_SetRight(expr, cel_Expr_UpCast(right));
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitBinaryExpr(expr)).WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitBinaryExprLeft(cel_Expr_UpCast(left)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(left))).WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(left)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(left))).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitBinaryExprLeft(cel_Expr_UpCast(left)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitBinaryExprRight(cel_Expr_UpCast(right)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(right))).WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(right)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(right)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitBinaryExprRight(cel_Expr_UpCast(right)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PostVisitBinaryExpr(expr)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast.get(), status()));
}

TEST_F(AstTraverseTest, Ternary) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_TernaryExpr* expr = cel_TernaryExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  cel_UnspecifiedExpr* condition = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(condition, NotNull());
  cel_TernaryExpr_SetCondition(expr, cel_Expr_UpCast(condition));
  cel_UnspecifiedExpr* if_true = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(if_true, NotNull());
  cel_TernaryExpr_SetIfTrue(expr, cel_Expr_UpCast(if_true));
  cel_UnspecifiedExpr* if_false = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(if_false, NotNull());
  cel_TernaryExpr_SetIfFalse(expr, cel_Expr_UpCast(if_false));
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitTernaryExpr(expr)).WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitTernaryExprCondition(cel_Expr_UpCast(condition)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(condition)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(condition)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(condition)))
      .WillOnce(Return());
  EXPECT_CALL(visitor,
              PostVisitTernaryExprCondition(cel_Expr_UpCast(condition)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitTernaryExprIfTrue(cel_Expr_UpCast(if_true)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(if_true)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(if_true)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(if_true)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitTernaryExprIfTrue(cel_Expr_UpCast(if_true)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitTernaryExprIfFalse(cel_Expr_UpCast(if_false)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(if_false)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(if_false)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(if_false)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitTernaryExprIfFalse(cel_Expr_UpCast(if_false)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PostVisitTernaryExpr(expr)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast.get(), status()));
}

TEST_F(AstTraverseTest, Call) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_CallExpr* expr = cel_CallExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  cel_UnspecifiedExpr* target = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(target, NotNull());
  cel_CallExpr_SetTarget(expr, cel_Expr_UpCast(target));
  cel_CallArgExpr* arg0 = cel_CallArgExpr_New(ast.get());
  ASSERT_THAT(arg0, NotNull());
  cel_UnspecifiedExpr* arg0_value = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(arg0_value, NotNull());
  cel_CallArgExpr_SetValue(arg0, cel_Expr_UpCast(arg0_value));
  cel_CallExpr_AppendArg(expr, arg0);
  cel_CallArgExpr* arg1 = cel_CallArgExpr_New(ast.get());
  ASSERT_THAT(arg1, NotNull());
  cel_UnspecifiedExpr* arg1_value = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(arg1_value, NotNull());
  cel_CallArgExpr_SetValue(arg1, cel_Expr_UpCast(arg1_value));
  cel_CallExpr_AppendArg(expr, arg1);
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitCallExpr(expr)).WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitCallExprTarget(cel_Expr_UpCast(target)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(target)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(target)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(target)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitCallExprTarget(cel_Expr_UpCast(target)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(arg0))).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitCallArgExpr(arg0)).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitCallArgExprValue(cel_Expr_UpCast(arg0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(arg0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(arg0_value)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(arg0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitCallArgExprValue(cel_Expr_UpCast(arg0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitCallArgExpr(arg0)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(arg0))).WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(arg1))).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitCallArgExpr(arg1)).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitCallArgExprValue(cel_Expr_UpCast(arg1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(arg1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(arg1_value)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(arg1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitCallArgExprValue(cel_Expr_UpCast(arg1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitCallArgExpr(arg1)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(arg1))).WillOnce(Return());

  EXPECT_CALL(visitor, PostVisitCallExpr(expr)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast.get(), status()));
}

TEST_F(AstTraverseTest, List) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_ListExpr* expr = cel_ListExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  cel_ListElementExpr* element0 = cel_ListElementExpr_New(ast.get());
  ASSERT_THAT(element0, NotNull());
  cel_UnspecifiedExpr* element0_value = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(element0_value, NotNull());
  cel_ListElementExpr_SetValue(element0, cel_Expr_UpCast(element0_value));
  cel_ListExpr_AppendElement(expr, element0);
  cel_ListElementExpr* element1 = cel_ListElementExpr_New(ast.get());
  ASSERT_THAT(element1, NotNull());
  cel_UnspecifiedExpr* element1_value = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(element1_value, NotNull());
  cel_ListElementExpr_SetValue(element1, cel_Expr_UpCast(element1_value));
  cel_ListExpr_AppendElement(expr, element1);
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitListExpr(expr)).WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(element0)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitListElementExpr(element0)).WillOnce(Return());
  EXPECT_CALL(visitor,
              PreVisitListElementExprValue(cel_Expr_UpCast(element0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(element0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(element0_value)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(element0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor,
              PostVisitListElementExprValue(cel_Expr_UpCast(element0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitListElementExpr(element0)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(element0)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(element1)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitListElementExpr(element1)).WillOnce(Return());
  EXPECT_CALL(visitor,
              PreVisitListElementExprValue(cel_Expr_UpCast(element1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(element1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(element1_value)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(element1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor,
              PostVisitListElementExprValue(cel_Expr_UpCast(element1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitListElementExpr(element1)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(element1)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PostVisitListExpr(expr)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast.get(), status()));
}

TEST_F(AstTraverseTest, Map) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_MapExpr* expr = cel_MapExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  cel_MapEntryExpr* entry0 = cel_MapEntryExpr_New(ast.get());
  ASSERT_THAT(entry0, NotNull());
  cel_UnspecifiedExpr* entry0_key = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(entry0_key, NotNull());
  cel_MapEntryExpr_SetKey(entry0, cel_Expr_UpCast(entry0_key));
  cel_UnspecifiedExpr* entry0_value = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(entry0_value, NotNull());
  cel_MapEntryExpr_SetValue(entry0, cel_Expr_UpCast(entry0_value));
  cel_MapExpr_AppendEntry(expr, entry0);
  cel_MapEntryExpr* entry1 = cel_MapEntryExpr_New(ast.get());
  ASSERT_THAT(entry1, NotNull());
  cel_UnspecifiedExpr* entry1_key = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(entry1_key, NotNull());
  cel_MapEntryExpr_SetKey(entry1, cel_Expr_UpCast(entry1_key));
  cel_UnspecifiedExpr* entry1_value = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(entry1_value, NotNull());
  cel_MapEntryExpr_SetValue(entry1, cel_Expr_UpCast(entry1_value));
  cel_MapExpr_AppendEntry(expr, entry1);
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitMapExpr(expr)).WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(entry0)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitMapEntryExpr(entry0)).WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitMapEntryExprKey(cel_Expr_UpCast(entry0_key)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(entry0_key)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(entry0_key)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(entry0_key)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitMapEntryExprKey(cel_Expr_UpCast(entry0_key)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitMapEntryExprValue(cel_Expr_UpCast(entry0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(entry0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(entry0_value)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(entry0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor,
              PostVisitMapEntryExprValue(cel_Expr_UpCast(entry0_value)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PostVisitMapEntryExpr(entry0)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(entry0)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(entry1)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitMapEntryExpr(entry1)).WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitMapEntryExprKey(cel_Expr_UpCast(entry1_key)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(entry1_key)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(entry1_key)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(entry1_key)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitMapEntryExprKey(cel_Expr_UpCast(entry1_key)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitMapEntryExprValue(cel_Expr_UpCast(entry1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(entry1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(entry1_value)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(entry1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor,
              PostVisitMapEntryExprValue(cel_Expr_UpCast(entry1_value)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PostVisitMapEntryExpr(entry1)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(entry1)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PostVisitMapExpr(expr)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast.get(), status()));
}

TEST_F(AstTraverseTest, Struct) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_StructExpr* expr = cel_StructExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  cel_StructFieldExpr* field0 = cel_StructFieldExpr_New(ast.get());
  ASSERT_THAT(field0, NotNull());
  cel_UnspecifiedExpr* field0_value = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(field0_value, NotNull());
  cel_StructFieldExpr_SetValue(field0, cel_Expr_UpCast(field0_value));
  cel_StructExpr_AppendField(expr, field0);
  cel_StructFieldExpr* field1 = cel_StructFieldExpr_New(ast.get());
  ASSERT_THAT(field1, NotNull());
  cel_UnspecifiedExpr* field1_value = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(field1_value, NotNull());
  cel_StructFieldExpr_SetValue(field1, cel_Expr_UpCast(field1_value));
  cel_StructExpr_AppendField(expr, field1);
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitStructExpr(expr)).WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(field0)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitStructFieldExpr(field0)).WillOnce(Return());
  EXPECT_CALL(visitor,
              PreVisitStructFieldExprValue(cel_Expr_UpCast(field0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(field0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(field0_value)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(field0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor,
              PostVisitStructFieldExprValue(cel_Expr_UpCast(field0_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitStructFieldExpr(field0)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(field0)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(field1)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitStructFieldExpr(field1)).WillOnce(Return());
  EXPECT_CALL(visitor,
              PreVisitStructFieldExprValue(cel_Expr_UpCast(field1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(field1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(field1_value)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(field1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor,
              PostVisitStructFieldExprValue(cel_Expr_UpCast(field1_value)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitStructFieldExpr(field1)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(field1)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PostVisitStructExpr(expr)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast.get(), status()));
}

TEST_F(AstTraverseTest, Comprehension) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_ComprehensionExpr* expr = cel_ComprehensionExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  cel_UnspecifiedExpr* iter_range = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(iter_range, NotNull());
  cel_ComprehensionExpr_SetIterRange(expr, cel_Expr_UpCast(iter_range));
  cel_UnspecifiedExpr* accu_init = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(accu_init, NotNull());
  cel_ComprehensionExpr_SetAccuInit(expr, cel_Expr_UpCast(accu_init));
  cel_UnspecifiedExpr* loop_condition = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(loop_condition, NotNull());
  cel_ComprehensionExpr_SetLoopCondition(expr, cel_Expr_UpCast(loop_condition));
  cel_UnspecifiedExpr* loop_step = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(loop_step, NotNull());
  cel_ComprehensionExpr_SetLoopStep(expr, cel_Expr_UpCast(loop_step));
  cel_UnspecifiedExpr* result = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(result, NotNull());
  cel_ComprehensionExpr_SetResult(expr, cel_Expr_UpCast(result));
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitComprehensionExpr(expr)).WillOnce(Return());

  EXPECT_CALL(visitor,
              PreVisitComprehensionExprIterRange(cel_Expr_UpCast(iter_range)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(iter_range)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(iter_range)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(iter_range)))
      .WillOnce(Return());
  EXPECT_CALL(visitor,
              PostVisitComprehensionExprIterRange(cel_Expr_UpCast(iter_range)))
      .WillOnce(Return());

  EXPECT_CALL(visitor,
              PreVisitComprehensionExprAccuInit(cel_Expr_UpCast(accu_init)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(accu_init)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(accu_init)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(accu_init)))
      .WillOnce(Return());
  EXPECT_CALL(visitor,
              PostVisitComprehensionExprAccuInit(cel_Expr_UpCast(accu_init)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitComprehensionExprLoopCondition(
                           cel_Expr_UpCast(loop_condition)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(loop_condition)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(loop_condition)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(loop_condition)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitComprehensionExprLoopCondition(
                           cel_Expr_UpCast(loop_condition)))
      .WillOnce(Return());

  EXPECT_CALL(visitor,
              PreVisitComprehensionExprLoopStep(cel_Expr_UpCast(loop_step)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(loop_step)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(loop_step)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(loop_step)))
      .WillOnce(Return());
  EXPECT_CALL(visitor,
              PostVisitComprehensionExprLoopStep(cel_Expr_UpCast(loop_step)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PreVisitComprehensionExprResult(cel_Expr_UpCast(result)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(result)))
      .WillOnce(Return());
  EXPECT_CALL(visitor, VisitUnspecifiedExpr(result)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(result)))
      .WillOnce(Return());
  EXPECT_CALL(visitor,
              PostVisitComprehensionExprResult(cel_Expr_UpCast(result)))
      .WillOnce(Return());

  EXPECT_CALL(visitor, PostVisitComprehensionExpr(expr)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast.get(), status()));
}

class StepOut {
 public:
  explicit StepOut(cel_AstTraverser* traverser)
      : traverser_(ABSL_DIE_IF_NULL(traverser)) {}

  template <typename... Args>
  void operator()(Args&&... args) const {
    cel_AstTraverser_StepOut(traverser_);
  }

 private:
  cel_AstTraverser* traverser_;
};

class StepOver {
 public:
  explicit StepOver(cel_AstTraverser* traverser)
      : traverser_(ABSL_DIE_IF_NULL(traverser)) {}

  template <typename... Args>
  void operator()(Args&&... args) const {
    cel_AstTraverser_StepOver(traverser_);
  }

 private:
  cel_AstTraverser* traverser_;
};

TEST_F(AstTraverseTest, PreVisitExpr_StepOut) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_UnspecifiedExpr* expr = cel_UnspecifiedExpr_New(ast.get());
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  _cel_AstTraverserPtr ast_traverser(cel_AstTraverser_New(ast.get(), &visitor));
  ASSERT_THAT(ast_traverser, NotNull());

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr)))
      .WillOnce(StepOut(ast_traverser.get()));

  EXPECT_TRUE(visitor.Traverse(ast_traverser.get(), status()));
}

TEST_F(AstTraverseTest, PreVisitExpr_StepOver) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_UnspecifiedExpr* expr = cel_UnspecifiedExpr_New(ast.get());
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  _cel_AstTraverserPtr ast_traverser(cel_AstTraverser_New(ast.get(), &visitor));
  ASSERT_THAT(ast_traverser, NotNull());

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr)))
      .WillOnce(StepOver(ast_traverser.get()));
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast_traverser.get(), status()));
}

TEST_F(AstTraverseTest, PreVisitSelectExpr_StepOut) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_SelectExpr* expr = cel_SelectExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  cel_UnspecifiedExpr* operand = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(operand, NotNull());
  cel_SelectExpr_SetOperand(expr, cel_Expr_UpCast(operand));
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  _cel_AstTraverserPtr ast_traverser(cel_AstTraverser_New(ast.get(), &visitor));
  ASSERT_THAT(ast_traverser, NotNull());

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitSelectExpr(expr))
      .WillOnce(StepOut(ast_traverser.get()));
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast_traverser.get(), status()));
}

TEST_F(AstTraverseTest, PreVisitSelectExpr_StepOver) {
  AstPtr ast(cel_Ast_New(arena()));
  ASSERT_THAT(ast, NotNull());

  cel_SelectExpr* expr = cel_SelectExpr_New(ast.get());
  ASSERT_THAT(expr, NotNull());
  cel_UnspecifiedExpr* operand = cel_UnspecifiedExpr_New(ast.get());
  ASSERT_THAT(operand, NotNull());
  cel_SelectExpr_SetOperand(expr, cel_Expr_UpCast(operand));
  cel_Ast_SetExpr(ast.get(), cel_Expr_UpCast(expr));

  InSequence seq;

  MockAstVisitor visitor;

  _cel_AstTraverserPtr ast_traverser(cel_AstTraverser_New(ast.get(), &visitor));
  ASSERT_THAT(ast_traverser, NotNull());

  EXPECT_CALL(visitor, PreVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());
  EXPECT_CALL(visitor, PreVisitSelectExpr(expr))
      .WillOnce(StepOver(ast_traverser.get()));
  EXPECT_CALL(visitor, PostVisitSelectExpr(expr)).WillOnce(Return());
  EXPECT_CALL(visitor, PostVisitExpr(cel_Expr_UpCast(expr))).WillOnce(Return());

  EXPECT_TRUE(visitor.Traverse(ast_traverser.get(), status()));
}

}  // namespace
