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

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/assert.h"
#include "cel-c/ast.h"
#include "cel-c/ast_visitor.h"
#include "cel-c/config.h"
#include "cel-c/src/deque.h"
#include "cel-c/src/malloc.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM {
  _cel_AstVisitorControl_kStop = 0,
  _cel_AstVisitorControl_kContinue,
} _cel_AstVisitorControl;

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM {
  _cel_AstTraverserStep_kStepIn = 0,
  _cel_AstTraverserStep_kStepOver,
  _cel_AstTraverserStep_kStepOut,
} _cel_AstTraverserStep;

#define _cel_AstVisitor_Invoke(visitor, member, ...)              \
  ((((visitor)->vtable->member) != cel_nullptr)                   \
       ? ((*(visitor)->vtable->member)((visitor), ##__VA_ARGS__), \
          _cel_AstVisitorControl_kStop)                           \
       : _cel_AstVisitorControl_kContinue)

#define _cel_AstVisitor_HasVisit(visitor, visit)          \
  (((visitor)->vtable->PreVisit##visit) != cel_nullptr || \
   ((visitor)->vtable->PostVisit##visit) != cel_nullptr)

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM {
  _cel_AstTraverserRecordKind_kExpr = 1,
  _cel_AstTraverserRecordKind_kSelectOperand,
  _cel_AstTraverserRecordKind_kCallTarget,
  _cel_AstTraverserRecordKind_kCallArgValue,
  _cel_AstTraverserRecordKind_kListElementValue,
  _cel_AstTraverserRecordKind_kMapEntryKey,
  _cel_AstTraverserRecordKind_kMapEntryValue,
  _cel_AstTraverserRecordKind_kStructFieldValue,
  _cel_AstTraverserRecordKind_kComprehensionIterRange,
  _cel_AstTraverserRecordKind_kComprehensionAccuInit,
  _cel_AstTraverserRecordKind_kComprehensionLoopCondition,
  _cel_AstTraverserRecordKind_kComprehensionLoopStep,
  _cel_AstTraverserRecordKind_kComprehensionResult,
  _cel_AstTraverserRecordKind_kUnaryArg,
  _cel_AstTraverserRecordKind_kBinaryLeft,
  _cel_AstTraverserRecordKind_kBinaryRight,
  _cel_AstTraverserRecordKind_kTernaryCondition,
  _cel_AstTraverserRecordKind_kTernaryIfTrue,
  _cel_AstTraverserRecordKind_kTernaryIfFalse,
} _cel_AstTraverserRecordKind;

typedef struct {
  CEL_NONNULL(const cel_Expr*) expr;
  bool previsited;
  bool pushed_deps;
  bool postvisited;
  bool previsited_expr;
  bool postvisited_expr;
} _cel_AstTraverserRecordData;

typedef struct {
  _cel_AstTraverserRecordData data;
  _cel_AstTraverserRecordKind kind;
} _cel_AstTraverserRecord;

struct cel_AstTraverser {
  _cel_Deque(_cel_AstTraverserRecord) records;
  CEL_NONNULL(cel_AstVisitor*) visitor;
  _cel_AstTraverserStep step;
  bool stepped;
  // Is the visitor interested in any {Pre,Post}VisitSelectExpr{...} callbacks?
  bool select_operand_callbacks;
  // Is the visitor interested in any {Pre,Post}VisitUnaryExpr{...} callbacks?
  bool unary_arg_callbacks;
  // Is the visitor interested in any {Pre,Post}VisitBinaryExpr{...} callbacks?
  bool binary_left_callbacks;
  bool binary_right_callbacks;
  // Is the visitor interested in any {Pre,Post}VisitTernaryExpr{...} callbacks?
  bool ternary_condition_callbacks;
  bool ternary_if_true_callbacks;
  bool ternary_if_false_callbacks;
  // Is the visitor interested in any {Pre,Post}VisitCallExpr{...} callbacks?
  bool call_target_callbacks;
  // Is the visitor interested in any {Pre,Post}VisitCallArgExpr{...} callbacks?
  bool call_arg_value_callbacks;
  // Is the visitor interested in any {Pre,Post}VisitListElementExpr{...}
  // callbacks?
  bool list_element_value_callbacks;
  // Is the visitor interested in any {Pre,Post}VisitMapEntryExpr{...}
  // callbacks?
  bool map_entry_key_callbacks;
  bool map_entry_value_callbacks;
  // Is the visitor interested in any {Pre,Post}VisitStructFieldExpr{...}
  // callbacks?
  bool struct_field_value_callbacks;
  // Is the visitor interested in any {Pre,Post}VisitComprehensionExpr{...}
  // callbacks?
  bool comprehension_iter_range_callbacks;
  bool comprehension_accu_init_callbacks;
  bool comprehension_loop_condition_callbacks;
  bool comprehension_loop_step_callbacks;
  bool comprehension_result_callbacks;
};

CEL_ATTRIBUTE_NODISCARD
static CEL_NULLABLE(_cel_AstTraverserRecord*)
    _cel_AstTraverser_Push(CEL_NONNULL(cel_AstTraverser*) traverser,
                           CEL_NULLABLE(cel_Status*) status) {
  _cel_AstTraverserRecord* record =
      _cel_Deque_PushBack(&traverser->records, cel_DefaultAllocator);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    if (status != cel_nullptr) {
      cel_OutOfMemoryStatus(status);
    }
    return cel_nullptr;
  }
  memset(record, 0, sizeof(*record));
  return record;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushExpr(CEL_NONNULL(cel_AstTraverser*) traverser,
                                       CEL_NONNULL(const cel_Expr*) expr,
                                       CEL_NULLABLE(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = expr;
  record->kind = _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_NULLABLE(_cel_AstTraverserRecord*)
    _cel_AstTraverser_Peek(CEL_NONNULL(cel_AstTraverser*) traverser) {
  if (_cel_Deque_Empty(&traverser->records)) {
    return cel_nullptr;
  }
  return _cel_Deque_MutablePeekBack(&traverser->records);
}

static void _cel_AstTraverser_Pop(CEL_NONNULL(cel_AstTraverser*) traverser) {
  _cel_Deque_PopBack(&traverser->records, cel_DefaultAllocator);
}

CEL_ATTRIBUTE_NODISCARD
static _cel_AstVisitorControl _cel_AstTraverserRecord_PreVisit(
    CEL_NONNULL(_cel_AstTraverserRecord*) record,
    CEL_NONNULL(cel_AstTraverser*) traverser, CEL_NONNULL(cel_Status*) status) {
  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return _cel_AstVisitorControl_kStop;
  }
  cel_AstVisitor* visitor = traverser->visitor;
  CEL_NONNULL(const cel_Expr*) expr = record->data.expr;
  const _cel_AstTraverserRecordKind record_kind = record->kind;
  switch (record_kind) {
    case _cel_AstTraverserRecordKind_kExpr: {
      _cel_AstVisitorControl control = _cel_AstVisitorControl_kContinue;
      if (!record->data.previsited_expr) {
        control = _cel_AstVisitor_Invoke(visitor, PreVisitExpr, expr);
        record->data.previsited_expr = true;
        if (traverser->stepped) {
          CEL_ASSERT_EQ(control, _cel_AstVisitorControl_kStop);
          switch (traverser->step) {
            case _cel_AstTraverserStep_kStepIn:
              break;
            case _cel_AstTraverserStep_kStepOver:
              record->data.previsited = true;
              record->data.pushed_deps = true;
              record->data.postvisited = true;
              break;
            case _cel_AstTraverserStep_kStepOut:
              record->data.previsited = true;
              record->data.pushed_deps = true;
              record->data.postvisited = true;
              record->data.postvisited_expr = true;
              break;
            default:
              CEL_UNREACHABLE();
          }
        }
      }
      if (control != _cel_AstVisitorControl_kContinue) {
        return control;
      }
      if (record->data.previsited) {
        return _cel_AstVisitorControl_kContinue;
      }
      record->data.previsited = true;
      switch (cel_Expr_Kind(expr)) {
        case cel_ExprKind_kUnspecified:
          control = _cel_AstVisitor_Invoke(visitor, VisitUnspecifiedExpr,
                                           cel_UnspecifiedExpr_DownCast(expr));
          break;
        case cel_ExprKind_kIdent:
          control = _cel_AstVisitor_Invoke(visitor, VisitIdentExpr,
                                           cel_IdentExpr_DownCast(expr));
          break;
        case cel_ExprKind_kConst:
          control = _cel_AstVisitor_Invoke(visitor, VisitConstExpr,
                                           cel_ConstExpr_DownCast(expr));
          break;
        case cel_ExprKind_kSelect:
          control = _cel_AstVisitor_Invoke(visitor, PreVisitSelectExpr,
                                           cel_SelectExpr_DownCast(expr));
          break;
        case cel_ExprKind_kCallArg:
          control = _cel_AstVisitor_Invoke(visitor, PreVisitCallArgExpr,
                                           cel_CallArgExpr_DownCast(expr));
          break;
        case cel_ExprKind_kCall:
          control = _cel_AstVisitor_Invoke(visitor, PreVisitCallExpr,
                                           cel_CallExpr_DownCast(expr));
          break;
        case cel_ExprKind_kListElement:
          control = _cel_AstVisitor_Invoke(visitor, PreVisitListElementExpr,
                                           cel_ListElementExpr_DownCast(expr));
          break;
        case cel_ExprKind_kList:
          control = _cel_AstVisitor_Invoke(visitor, PreVisitListExpr,
                                           cel_ListExpr_DownCast(expr));
          break;
        case cel_ExprKind_kMapEntry:
          control = _cel_AstVisitor_Invoke(visitor, PreVisitMapEntryExpr,
                                           cel_MapEntryExpr_DownCast(expr));
          break;
        case cel_ExprKind_kMap:
          control = _cel_AstVisitor_Invoke(visitor, PreVisitMapExpr,
                                           cel_MapExpr_DownCast(expr));
          break;
        case cel_ExprKind_kStructField:
          control = _cel_AstVisitor_Invoke(visitor, PreVisitStructFieldExpr,
                                           cel_StructFieldExpr_DownCast(expr));
          break;
        case cel_ExprKind_kStruct:
          control = _cel_AstVisitor_Invoke(visitor, PreVisitStructExpr,
                                           cel_StructExpr_DownCast(expr));
          break;
        case cel_ExprKind_kComprehension:
          control =
              _cel_AstVisitor_Invoke(visitor, PreVisitComprehensionExpr,
                                     cel_ComprehensionExpr_DownCast(expr));
          break;
        case cel_ExprKind_kUnary:
          control = _cel_AstVisitor_Invoke(visitor, PreVisitUnaryExpr,
                                           cel_UnaryExpr_DownCast(expr));
          break;
        case cel_ExprKind_kBinary:
          control = _cel_AstVisitor_Invoke(visitor, PreVisitBinaryExpr,
                                           cel_BinaryExpr_DownCast(expr));
          break;
        case cel_ExprKind_kTernary:
          control = _cel_AstVisitor_Invoke(visitor, PreVisitTernaryExpr,
                                           cel_TernaryExpr_DownCast(expr));
          break;
        default:
          cel_InternalStatus(status,
                             cel_StringView_From("unexpected AST node kind"));
          control = _cel_AstVisitorControl_kStop;
          break;
      }
      if (traverser->stepped) {
        CEL_ASSERT_EQ(control, _cel_AstVisitorControl_kStop);
        switch (traverser->step) {
          case _cel_AstTraverserStep_kStepIn:
            break;
          case _cel_AstTraverserStep_kStepOver:
            record->data.pushed_deps = true;
            break;
          case _cel_AstTraverserStep_kStepOut:
            record->data.pushed_deps = true;
            record->data.postvisited = true;
            break;
          default:
            CEL_UNREACHABLE();
        }
      }
      return control;
    }
    default:
      record->data.previsited = record->data.previsited_expr = true;
      break;
  }
  _cel_AstVisitorControl control = _cel_AstVisitorControl_kContinue;
  switch (record_kind) {
    case _cel_AstTraverserRecordKind_kSelectOperand: {
      control =
          _cel_AstVisitor_Invoke(visitor, PreVisitSelectExprOperand, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kCallTarget: {
      control = _cel_AstVisitor_Invoke(visitor, PreVisitCallExprTarget, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kCallArgValue: {
      control = _cel_AstVisitor_Invoke(visitor, PreVisitCallArgExprValue, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kListElementValue: {
      control =
          _cel_AstVisitor_Invoke(visitor, PreVisitListElementExprValue, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kMapEntryKey: {
      control = _cel_AstVisitor_Invoke(visitor, PreVisitMapEntryExprKey, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kMapEntryValue: {
      control =
          _cel_AstVisitor_Invoke(visitor, PreVisitMapEntryExprValue, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kStructFieldValue: {
      control =
          _cel_AstVisitor_Invoke(visitor, PreVisitStructFieldExprValue, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kComprehensionIterRange: {
      control = _cel_AstVisitor_Invoke(
          visitor, PreVisitComprehensionExprIterRange, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kComprehensionAccuInit: {
      control = _cel_AstVisitor_Invoke(visitor,
                                       PreVisitComprehensionExprAccuInit, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kComprehensionLoopCondition: {
      control = _cel_AstVisitor_Invoke(
          visitor, PreVisitComprehensionExprLoopCondition, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kComprehensionLoopStep: {
      control = _cel_AstVisitor_Invoke(visitor,
                                       PreVisitComprehensionExprLoopStep, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kComprehensionResult: {
      control = _cel_AstVisitor_Invoke(visitor, PreVisitComprehensionExprResult,
                                       expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kUnaryArg: {
      control = _cel_AstVisitor_Invoke(visitor, PreVisitUnaryExprArg, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kBinaryLeft: {
      control = _cel_AstVisitor_Invoke(visitor, PreVisitBinaryExprLeft, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kBinaryRight: {
      control = _cel_AstVisitor_Invoke(visitor, PreVisitBinaryExprRight, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kTernaryCondition: {
      control =
          _cel_AstVisitor_Invoke(visitor, PreVisitTernaryExprCondition, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kTernaryIfTrue: {
      control =
          _cel_AstVisitor_Invoke(visitor, PreVisitTernaryExprIfTrue, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kTernaryIfFalse: {
      control =
          _cel_AstVisitor_Invoke(visitor, PreVisitTernaryExprIfFalse, expr);
      break;
    }
    default:
      CEL_UNREACHABLE();
  }
  if (traverser->stepped) {
    CEL_ASSERT_EQ(control, _cel_AstVisitorControl_kStop);
    switch (traverser->step) {
      case _cel_AstTraverserStep_kStepIn:
        break;
      case _cel_AstTraverserStep_kStepOver:
        record->data.pushed_deps = true;
        break;
      case _cel_AstTraverserStep_kStepOut:
        record->data.pushed_deps = true;
        record->data.postvisited = true;
        record->data.postvisited_expr = true;
        break;
      default:
        CEL_UNREACHABLE();
    }
  }
  return control;
}

CEL_ATTRIBUTE_NODISCARD
static _cel_AstVisitorControl _cel_AstTraverserRecord_PostVisit(
    CEL_NONNULL(_cel_AstTraverserRecord*) record,
    CEL_NONNULL(cel_AstTraverser*) traverser, CEL_NONNULL(cel_Status*) status) {
  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return _cel_AstVisitorControl_kStop;
  }
  cel_AstVisitor* visitor = traverser->visitor;
  CEL_NONNULL(const cel_Expr*) expr = record->data.expr;
  const _cel_AstTraverserRecordKind record_kind = record->kind;
  switch (record_kind) {
    case _cel_AstTraverserRecordKind_kExpr: {
      _cel_AstVisitorControl control = _cel_AstVisitorControl_kContinue;
      if (!record->data.postvisited) {
        record->data.postvisited = true;
        switch (cel_Expr_Kind(expr)) {
          case cel_ExprKind_kUnspecified:
            CEL_ATTRIBUTE_FALLTHROUGH;
          case cel_ExprKind_kIdent:
            CEL_ATTRIBUTE_FALLTHROUGH;
          case cel_ExprKind_kConst:
            break;
          case cel_ExprKind_kSelect:
            control = _cel_AstVisitor_Invoke(visitor, PostVisitSelectExpr,
                                             cel_SelectExpr_DownCast(expr));
            break;
          case cel_ExprKind_kCallArg:
            control = _cel_AstVisitor_Invoke(visitor, PostVisitCallArgExpr,
                                             cel_CallArgExpr_DownCast(expr));
            break;
          case cel_ExprKind_kCall:
            control = _cel_AstVisitor_Invoke(visitor, PostVisitCallExpr,
                                             cel_CallExpr_DownCast(expr));
            break;
          case cel_ExprKind_kListElement:
            control =
                _cel_AstVisitor_Invoke(visitor, PostVisitListElementExpr,
                                       cel_ListElementExpr_DownCast(expr));
            break;
          case cel_ExprKind_kList:
            control = _cel_AstVisitor_Invoke(visitor, PostVisitListExpr,
                                             cel_ListExpr_DownCast(expr));
            break;
          case cel_ExprKind_kMapEntry:
            control = _cel_AstVisitor_Invoke(visitor, PostVisitMapEntryExpr,
                                             cel_MapEntryExpr_DownCast(expr));
            break;
          case cel_ExprKind_kMap:
            control = _cel_AstVisitor_Invoke(visitor, PostVisitMapExpr,
                                             cel_MapExpr_DownCast(expr));
            break;
          case cel_ExprKind_kStructField:
            control =
                _cel_AstVisitor_Invoke(visitor, PostVisitStructFieldExpr,
                                       cel_StructFieldExpr_DownCast(expr));
            break;
          case cel_ExprKind_kStruct:
            control = _cel_AstVisitor_Invoke(visitor, PostVisitStructExpr,
                                             cel_StructExpr_DownCast(expr));
            break;
          case cel_ExprKind_kComprehension:
            control =
                _cel_AstVisitor_Invoke(visitor, PostVisitComprehensionExpr,
                                       cel_ComprehensionExpr_DownCast(expr));
            break;
          case cel_ExprKind_kUnary:
            control = _cel_AstVisitor_Invoke(visitor, PostVisitUnaryExpr,
                                             cel_UnaryExpr_DownCast(expr));
            break;
          case cel_ExprKind_kBinary:
            control = _cel_AstVisitor_Invoke(visitor, PostVisitBinaryExpr,
                                             cel_BinaryExpr_DownCast(expr));
            break;
          case cel_ExprKind_kTernary:
            control = _cel_AstVisitor_Invoke(visitor, PostVisitTernaryExpr,
                                             cel_TernaryExpr_DownCast(expr));
            break;
          default:
            record->data.postvisited = false;
            cel_InternalStatus(status,
                               cel_StringView_From("unexpected AST node kind"));
            control = _cel_AstVisitorControl_kStop;
            break;
        }
      }
      if (control != _cel_AstVisitorControl_kContinue) {
        return control;
      }
      if (record->data.postvisited_expr) {
        return _cel_AstVisitorControl_kContinue;
      }
      record->data.postvisited_expr = true;
      return _cel_AstVisitor_Invoke(visitor, PostVisitExpr, expr);
    }
    default:
      if (record->data.postvisited || record->data.postvisited_expr) {
        record->data.postvisited = record->data.postvisited_expr = true;
        return _cel_AstVisitorControl_kContinue;
      }
      record->data.postvisited = record->data.postvisited_expr = true;
      break;
  }
  _cel_AstVisitorControl control = _cel_AstVisitorControl_kContinue;
  switch (record_kind) {
    case _cel_AstTraverserRecordKind_kSelectOperand: {
      control =
          _cel_AstVisitor_Invoke(visitor, PostVisitSelectExprOperand, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kCallTarget: {
      control = _cel_AstVisitor_Invoke(visitor, PostVisitCallExprTarget, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kCallArgValue: {
      control =
          _cel_AstVisitor_Invoke(visitor, PostVisitCallArgExprValue, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kListElementValue: {
      control =
          _cel_AstVisitor_Invoke(visitor, PostVisitListElementExprValue, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kMapEntryKey: {
      control = _cel_AstVisitor_Invoke(visitor, PostVisitMapEntryExprKey, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kMapEntryValue: {
      control =
          _cel_AstVisitor_Invoke(visitor, PostVisitMapEntryExprValue, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kStructFieldValue: {
      control =
          _cel_AstVisitor_Invoke(visitor, PostVisitStructFieldExprValue, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kComprehensionIterRange: {
      control = _cel_AstVisitor_Invoke(
          visitor, PostVisitComprehensionExprIterRange, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kComprehensionAccuInit: {
      control = _cel_AstVisitor_Invoke(
          visitor, PostVisitComprehensionExprAccuInit, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kComprehensionLoopCondition: {
      control = _cel_AstVisitor_Invoke(
          visitor, PostVisitComprehensionExprLoopCondition, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kComprehensionLoopStep: {
      control = _cel_AstVisitor_Invoke(
          visitor, PostVisitComprehensionExprLoopStep, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kComprehensionResult: {
      control = _cel_AstVisitor_Invoke(visitor,
                                       PostVisitComprehensionExprResult, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kUnaryArg: {
      control = _cel_AstVisitor_Invoke(visitor, PostVisitUnaryExprArg, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kBinaryLeft: {
      control = _cel_AstVisitor_Invoke(visitor, PostVisitBinaryExprLeft, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kBinaryRight: {
      control = _cel_AstVisitor_Invoke(visitor, PostVisitBinaryExprRight, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kTernaryCondition: {
      control =
          _cel_AstVisitor_Invoke(visitor, PostVisitTernaryExprCondition, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kTernaryIfFalse: {
      control =
          _cel_AstVisitor_Invoke(visitor, PostVisitTernaryExprIfFalse, expr);
      break;
    }
    case _cel_AstTraverserRecordKind_kTernaryIfTrue: {
      control =
          _cel_AstVisitor_Invoke(visitor, PostVisitTernaryExprIfTrue, expr);
      break;
    }
    default:
      CEL_UNREACHABLE();
  }
  if (traverser->stepped) {
    CEL_ASSERT_EQ(control, _cel_AstVisitorControl_kStop);
    switch (traverser->step) {
      case _cel_AstTraverserStep_kStepIn:
        break;
      case _cel_AstTraverserStep_kStepOver:
        CEL_ATTRIBUTE_FALLTHROUGH;
      case _cel_AstTraverserStep_kStepOut:
        record->data.postvisited = true;
        record->data.postvisited_expr = true;
        break;
      default:
        CEL_UNREACHABLE();
    }
  }
  return control;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushSelectOperand(CEL_NONNULL(cel_AstTraverser*)
                                                    traverser,
                                                CEL_NONNULL(const cel_Expr*)
                                                    select_operand,
                                                CEL_NONNULL(cel_Status*)
                                                    status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = select_operand;
  record->kind = traverser->select_operand_callbacks
                     ? _cel_AstTraverserRecordKind_kSelectOperand
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushCallTarget(CEL_NONNULL(cel_AstTraverser*)
                                                 traverser,
                                             CEL_NONNULL(const cel_Expr*)
                                                 call_target,
                                             CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = call_target;
  record->kind = traverser->call_target_callbacks
                     ? _cel_AstTraverserRecordKind_kCallTarget
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushCallArgValue(CEL_NONNULL(cel_AstTraverser*)
                                                   traverser,
                                               CEL_NONNULL(const cel_Expr*)
                                                   call_arg_value,
                                               CEL_NONNULL(cel_Status*)
                                                   status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = call_arg_value;
  record->kind = traverser->call_arg_value_callbacks
                     ? _cel_AstTraverserRecordKind_kCallArgValue
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushListElementValue(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    CEL_NONNULL(const cel_Expr*) list_element_value,
    CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = list_element_value;
  record->kind = traverser->list_element_value_callbacks
                     ? _cel_AstTraverserRecordKind_kListElementValue
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushMapEntryKey(CEL_NONNULL(cel_AstTraverser*)
                                                  traverser,
                                              CEL_NONNULL(const cel_Expr*)
                                                  map_entry_key,
                                              CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = map_entry_key;
  record->kind = traverser->map_entry_key_callbacks
                     ? _cel_AstTraverserRecordKind_kMapEntryKey
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushMapEntryValue(CEL_NONNULL(cel_AstTraverser*)
                                                    traverser,
                                                CEL_NONNULL(const cel_Expr*)
                                                    map_entry_value,
                                                CEL_NONNULL(cel_Status*)
                                                    status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = map_entry_value;
  record->kind = traverser->map_entry_value_callbacks
                     ? _cel_AstTraverserRecordKind_kMapEntryValue
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushStructFieldValue(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    CEL_NONNULL(const cel_Expr*) struct_field_value,
    CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = struct_field_value;
  record->kind = traverser->struct_field_value_callbacks
                     ? _cel_AstTraverserRecordKind_kStructFieldValue
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushComprehensionIterRange(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    CEL_NONNULL(const cel_Expr*) iter_range, CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = iter_range;
  record->kind = traverser->comprehension_iter_range_callbacks
                     ? _cel_AstTraverserRecordKind_kComprehensionIterRange
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushComprehensionAccuInit(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    CEL_NONNULL(const cel_Expr*) accu_init, CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = accu_init;
  record->kind = traverser->comprehension_accu_init_callbacks
                     ? _cel_AstTraverserRecordKind_kComprehensionAccuInit
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushComprehensionLoopCondition(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    CEL_NONNULL(const cel_Expr*) loop_condition,
    CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = loop_condition;
  record->kind = traverser->comprehension_loop_condition_callbacks
                     ? _cel_AstTraverserRecordKind_kComprehensionLoopCondition
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushComprehensionLoopStep(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    CEL_NONNULL(const cel_Expr*) loop_step, CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = loop_step;
  record->kind = traverser->comprehension_loop_step_callbacks
                     ? _cel_AstTraverserRecordKind_kComprehensionLoopStep
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushComprehensionResult(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    CEL_NONNULL(const cel_Expr*) result, CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = result;
  record->kind = traverser->comprehension_result_callbacks
                     ? _cel_AstTraverserRecordKind_kComprehensionResult
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushUnaryArg(CEL_NONNULL(cel_AstTraverser*)
                                               traverser,
                                           CEL_NONNULL(const cel_Expr*) result,
                                           CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = result;
  record->kind = traverser->unary_arg_callbacks
                     ? _cel_AstTraverserRecordKind_kUnaryArg
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushBinaryLeft(CEL_NONNULL(cel_AstTraverser*)
                                                 traverser,
                                             CEL_NONNULL(const cel_Expr*)
                                                 result,
                                             CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = result;
  record->kind = traverser->binary_left_callbacks
                     ? _cel_AstTraverserRecordKind_kBinaryLeft
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushBinaryRight(CEL_NONNULL(cel_AstTraverser*)
                                                  traverser,
                                              CEL_NONNULL(const cel_Expr*)
                                                  result,
                                              CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = result;
  record->kind = traverser->binary_right_callbacks
                     ? _cel_AstTraverserRecordKind_kBinaryRight
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushTernaryCondition(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    CEL_NONNULL(const cel_Expr*) result, CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = result;
  record->kind = traverser->ternary_condition_callbacks
                     ? _cel_AstTraverserRecordKind_kTernaryCondition
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushTernaryIfTrue(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    CEL_NONNULL(const cel_Expr*) result, CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = result;
  record->kind = traverser->ternary_if_true_callbacks
                     ? _cel_AstTraverserRecordKind_kTernaryIfTrue
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushTernaryIfFalse(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    CEL_NONNULL(const cel_Expr*) result, CEL_NONNULL(cel_Status*) status) {
  _cel_AstTraverserRecord* record = _cel_AstTraverser_Push(traverser, status);
  if (CEL_UNLIKELY(record == cel_nullptr)) {
    return false;
  }
  record->data.expr = result;
  record->kind = traverser->ternary_if_false_callbacks
                     ? _cel_AstTraverserRecordKind_kTernaryIfFalse
                     : _cel_AstTraverserRecordKind_kExpr;
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushSelectDeps(CEL_NONNULL(cel_AstTraverser*)
                                                 traverser,
                                             const cel_SelectExpr* select_expr,
                                             CEL_NONNULL(cel_Status*) status) {
  CEL_NULLABLE(cel_Expr*) operand = cel_SelectExpr_Operand(select_expr);
  if (operand != cel_nullptr) {
    if (!_cel_AstTraverser_PushSelectOperand(traverser, operand, status)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushCallArgDeps(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    const cel_CallArgExpr* call_arg_expr, CEL_NONNULL(cel_Status*) status) {
  CEL_NULLABLE(cel_Expr*) value = cel_CallArgExpr_Value(call_arg_expr);
  if (value != cel_nullptr) {
    if (!_cel_AstTraverser_PushCallArgValue(traverser, value, status)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushCallDeps(CEL_NONNULL(cel_AstTraverser*)
                                               traverser,
                                           const cel_CallExpr* call_expr,
                                           CEL_NONNULL(cel_Status*) status) {
  CEL_NULLABLE(cel_CallArgExpr*) arg;
  size_t call_expr_args_size =
      cel_CallExpr_Args(call_expr, /*head=*/cel_nullptr, &arg);
  for (size_t i = call_expr_args_size; i > 0; --i) {
    if (!_cel_AstTraverser_PushExpr(traverser, cel_Expr_UpCast(arg), status)) {
      return false;
    }
    arg = cel_CallExpr_PrevArg(arg);
  }
  CEL_NULLABLE(cel_Expr*) target = cel_CallExpr_Target(call_expr);
  if (target != cel_nullptr) {
    if (!_cel_AstTraverser_PushCallTarget(traverser, target, status)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushListElementDeps(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    const cel_ListElementExpr* list_element_expr,
    CEL_NONNULL(cel_Status*) status) {
  CEL_NULLABLE(cel_Expr*) value = cel_ListElementExpr_Value(list_element_expr);
  if (value != cel_nullptr) {
    if (!_cel_AstTraverser_PushListElementValue(traverser, value, status)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushListDeps(CEL_NONNULL(cel_AstTraverser*)
                                               traverser,
                                           const cel_ListExpr* list_expr,
                                           CEL_NONNULL(cel_Status*) status) {
  CEL_NULLABLE(cel_ListElementExpr*) elem;
  size_t list_expr_elements_size =
      cel_ListExpr_Elements(list_expr, /*head=*/cel_nullptr, &elem);
  for (size_t i = list_expr_elements_size; i > 0; --i) {
    if (!_cel_AstTraverser_PushExpr(traverser, cel_Expr_UpCast(elem), status)) {
      return false;
    }
    elem = cel_ListExpr_PrevElement(elem);
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushMapEntryDeps(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    const cel_MapEntryExpr* map_entry_expr, CEL_NONNULL(cel_Status*) status) {
  CEL_NULLABLE(cel_Expr*) value = cel_MapEntryExpr_Value(map_entry_expr);
  if (value != cel_nullptr) {
    if (!_cel_AstTraverser_PushMapEntryValue(traverser, value, status)) {
      return false;
    }
  }
  CEL_NULLABLE(cel_Expr*) key = cel_MapEntryExpr_Key(map_entry_expr);
  if (key != cel_nullptr) {
    if (!_cel_AstTraverser_PushMapEntryKey(traverser, key, status)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushMapDeps(CEL_NONNULL(cel_AstTraverser*)
                                              traverser,
                                          const cel_MapExpr* map_expr,
                                          CEL_NONNULL(cel_Status*) status) {
  CEL_NULLABLE(cel_MapEntryExpr*) ent;
  size_t struct_expr_entries_size =
      cel_MapExpr_Entries(map_expr, /*head=*/cel_nullptr, &ent);
  for (size_t i = struct_expr_entries_size; i > 0; --i) {
    if (!_cel_AstTraverser_PushExpr(traverser, cel_Expr_UpCast(ent), status)) {
      return false;
    }
    ent = cel_MapExpr_PrevEntry(ent);
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushStructFieldDeps(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    const cel_StructFieldExpr* struct_field_expr,
    CEL_NONNULL(cel_Status*) status) {
  CEL_NULLABLE(cel_Expr*) value = cel_StructFieldExpr_Value(struct_field_expr);
  if (value != cel_nullptr) {
    if (!_cel_AstTraverser_PushStructFieldValue(traverser, value, status)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushStructDeps(CEL_NONNULL(cel_AstTraverser*)
                                                 traverser,
                                             const cel_StructExpr* struct_expr,
                                             CEL_NONNULL(cel_Status*) status) {
  CEL_NULLABLE(cel_StructFieldExpr*) fld;
  size_t struct_expr_entries_size =
      cel_StructExpr_Fields(struct_expr, /*head=*/cel_nullptr, &fld);
  for (size_t i = struct_expr_entries_size; i > 0; --i) {
    if (!_cel_AstTraverser_PushExpr(traverser, cel_Expr_UpCast(fld), status)) {
      return false;
    }
    fld = cel_StructExpr_PrevField(fld);
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushComprehensionDeps(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    const cel_ComprehensionExpr* comprehension_expr,
    CEL_NONNULL(cel_Status*) status) {
  CEL_NULLABLE(cel_Expr*)
  result = cel_ComprehensionExpr_Result(comprehension_expr);
  if (result != cel_nullptr) {
    if (!_cel_AstTraverser_PushComprehensionResult(traverser, result, status)) {
      return false;
    }
  }
  CEL_NULLABLE(cel_Expr*)
  loop_step = cel_ComprehensionExpr_LoopStep(comprehension_expr);
  if (loop_step != cel_nullptr) {
    if (!_cel_AstTraverser_PushComprehensionLoopStep(traverser, loop_step,
                                                     status)) {
      return false;
    }
  }
  CEL_NULLABLE(cel_Expr*)
  loop_condition = cel_ComprehensionExpr_LoopCondition(comprehension_expr);
  if (loop_condition != cel_nullptr) {
    if (!_cel_AstTraverser_PushComprehensionLoopCondition(
            traverser, loop_condition, status)) {
      return false;
    }
  }
  CEL_NULLABLE(cel_Expr*)
  accu_init = cel_ComprehensionExpr_AccuInit(comprehension_expr);
  if (accu_init != cel_nullptr) {
    if (!_cel_AstTraverser_PushComprehensionAccuInit(traverser, accu_init,
                                                     status)) {
      return false;
    }
  }
  CEL_NULLABLE(cel_Expr*)
  iter_range = cel_ComprehensionExpr_IterRange(comprehension_expr);
  if (iter_range != cel_nullptr) {
    if (!_cel_AstTraverser_PushComprehensionIterRange(traverser, iter_range,
                                                      status)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushUnaryDeps(CEL_NONNULL(cel_AstTraverser*)
                                                traverser,
                                            const cel_UnaryExpr* unary_expr,
                                            CEL_NONNULL(cel_Status*) status) {
  CEL_NULLABLE(cel_Expr*) arg = cel_UnaryExpr_Arg(unary_expr);
  if (arg != cel_nullptr) {
    if (!_cel_AstTraverser_PushUnaryArg(traverser, arg, status)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushBinaryDeps(CEL_NONNULL(cel_AstTraverser*)
                                                 traverser,
                                             const cel_BinaryExpr* binary_expr,
                                             CEL_NONNULL(cel_Status*) status) {
  CEL_NULLABLE(cel_Expr*) right = cel_BinaryExpr_Right(binary_expr);
  if (right != cel_nullptr) {
    if (!_cel_AstTraverser_PushBinaryRight(traverser, right, status)) {
      return false;
    }
  }
  CEL_NULLABLE(cel_Expr*) left = cel_BinaryExpr_Left(binary_expr);
  if (left != cel_nullptr) {
    if (!_cel_AstTraverser_PushBinaryLeft(traverser, left, status)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushTernaryDeps(
    CEL_NONNULL(cel_AstTraverser*) traverser,
    const cel_TernaryExpr* ternary_expr, CEL_NONNULL(cel_Status*) status) {
  CEL_NULLABLE(cel_Expr*) if_false = cel_TernaryExpr_IfFalse(ternary_expr);
  if (if_false != cel_nullptr) {
    if (!_cel_AstTraverser_PushTernaryIfFalse(traverser, if_false, status)) {
      return false;
    }
  }
  CEL_NULLABLE(cel_Expr*) if_true = cel_TernaryExpr_IfTrue(ternary_expr);
  if (if_true != cel_nullptr) {
    if (!_cel_AstTraverser_PushTernaryIfTrue(traverser, if_true, status)) {
      return false;
    }
  }
  CEL_NULLABLE(cel_Expr*) condition = cel_TernaryExpr_Condition(ternary_expr);
  if (condition != cel_nullptr) {
    if (!_cel_AstTraverser_PushTernaryCondition(traverser, condition, status)) {
      return false;
    }
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_AstTraverser_PushDeps(CEL_NONNULL(cel_AstTraverser*) traverser,
                                       CEL_NONNULL(_cel_AstTraverserRecord*)
                                           record,
                                       CEL_NONNULL(cel_Status*) status) {
  CEL_NONNULL(const cel_Expr*) expr = record->data.expr;
  switch (record->kind) {
    case _cel_AstTraverserRecordKind_kExpr: {
      switch (cel_Expr_Kind(expr)) {
        case cel_ExprKind_kUnspecified:
          break;
        case cel_ExprKind_kIdent:
          break;
        case cel_ExprKind_kConst:
          break;
        case cel_ExprKind_kSelect:
          return _cel_AstTraverser_PushSelectDeps(
              traverser, cel_SelectExpr_DownCast(expr), status);
        case cel_ExprKind_kCallArg:
          return _cel_AstTraverser_PushCallArgDeps(
              traverser, cel_CallArgExpr_DownCast(expr), status);
        case cel_ExprKind_kCall:
          return _cel_AstTraverser_PushCallDeps(
              traverser, cel_CallExpr_DownCast(expr), status);
        case cel_ExprKind_kListElement:
          return _cel_AstTraverser_PushListElementDeps(
              traverser, cel_ListElementExpr_DownCast(expr), status);
        case cel_ExprKind_kList:
          return _cel_AstTraverser_PushListDeps(
              traverser, cel_ListExpr_DownCast(expr), status);
        case cel_ExprKind_kMapEntry:
          return _cel_AstTraverser_PushMapEntryDeps(
              traverser, cel_MapEntryExpr_DownCast(expr), status);
        case cel_ExprKind_kMap:
          return _cel_AstTraverser_PushMapDeps(
              traverser, cel_MapExpr_DownCast(expr), status);
        case cel_ExprKind_kStructField:
          return _cel_AstTraverser_PushStructFieldDeps(
              traverser, cel_StructFieldExpr_DownCast(expr), status);
        case cel_ExprKind_kStruct:
          return _cel_AstTraverser_PushStructDeps(
              traverser, cel_StructExpr_DownCast(expr), status);
        case cel_ExprKind_kComprehension:
          return _cel_AstTraverser_PushComprehensionDeps(
              traverser, cel_ComprehensionExpr_DownCast(expr), status);
        case cel_ExprKind_kUnary:
          return _cel_AstTraverser_PushUnaryDeps(
              traverser, cel_UnaryExpr_DownCast(expr), status);
        case cel_ExprKind_kBinary:
          return _cel_AstTraverser_PushBinaryDeps(
              traverser, cel_BinaryExpr_DownCast(expr), status);
        case cel_ExprKind_kTernary:
          return _cel_AstTraverser_PushTernaryDeps(
              traverser, cel_TernaryExpr_DownCast(expr), status);
        default:
          cel_InternalStatus(status,
                             cel_StringView_From("unexpected AST node kind"));
          return false;
      }
      return true;
    }
    case _cel_AstTraverserRecordKind_kSelectOperand:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kCallTarget:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kCallArgValue:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kListElementValue:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kMapEntryKey:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kMapEntryValue:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kStructFieldValue:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kComprehensionIterRange:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kComprehensionAccuInit:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kComprehensionLoopCondition:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kComprehensionLoopStep:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kComprehensionResult:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kUnaryArg:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kBinaryLeft:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kBinaryRight:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kTernaryCondition:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kTernaryIfTrue:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case _cel_AstTraverserRecordKind_kTernaryIfFalse:
      return _cel_AstTraverser_PushExpr(traverser, expr, status);
    default:
      CEL_UNREACHABLE();
  }
}

static void _cel_AstTraverser_Construct(CEL_NONNULL(cel_AstTraverser*)
                                            traverser,
                                        CEL_NONNULL(cel_AstVisitor*) visitor) {
  memset(traverser, '\0', sizeof(*traverser));
  _cel_Deque_Construct(&traverser->records);
  traverser->visitor = visitor;
  traverser->step = _cel_AstTraverserStep_kStepIn;
  traverser->stepped = false;
  traverser->select_operand_callbacks =
      _cel_AstVisitor_HasVisit(visitor, SelectExprOperand);
  traverser->unary_arg_callbacks =
      _cel_AstVisitor_HasVisit(visitor, UnaryExprArg);
  traverser->binary_left_callbacks =
      _cel_AstVisitor_HasVisit(visitor, BinaryExprLeft);
  traverser->binary_right_callbacks =
      _cel_AstVisitor_HasVisit(visitor, BinaryExprRight);
  traverser->ternary_condition_callbacks =
      _cel_AstVisitor_HasVisit(visitor, TernaryExprCondition);
  traverser->ternary_if_true_callbacks =
      _cel_AstVisitor_HasVisit(visitor, TernaryExprIfTrue);
  traverser->ternary_if_false_callbacks =
      _cel_AstVisitor_HasVisit(visitor, TernaryExprIfFalse);
  traverser->call_target_callbacks =
      _cel_AstVisitor_HasVisit(visitor, CallExprTarget);
  traverser->call_arg_value_callbacks =
      _cel_AstVisitor_HasVisit(visitor, CallArgExprValue);
  traverser->list_element_value_callbacks =
      _cel_AstVisitor_HasVisit(visitor, ListElementExprValue);
  traverser->map_entry_key_callbacks =
      _cel_AstVisitor_HasVisit(visitor, MapEntryExprKey);
  traverser->map_entry_value_callbacks =
      _cel_AstVisitor_HasVisit(visitor, MapEntryExprValue);
  traverser->struct_field_value_callbacks =
      _cel_AstVisitor_HasVisit(visitor, StructFieldExprValue);
  traverser->comprehension_iter_range_callbacks =
      _cel_AstVisitor_HasVisit(visitor, ComprehensionExprIterRange);
  traverser->comprehension_accu_init_callbacks =
      _cel_AstVisitor_HasVisit(visitor, ComprehensionExprAccuInit);
  traverser->comprehension_loop_condition_callbacks =
      _cel_AstVisitor_HasVisit(visitor, ComprehensionExprLoopCondition);
  traverser->comprehension_loop_step_callbacks =
      _cel_AstVisitor_HasVisit(visitor, ComprehensionExprLoopStep);
  traverser->comprehension_result_callbacks =
      _cel_AstVisitor_HasVisit(visitor, ComprehensionExprResult);
}

static void _cel_AstTraverser_Destruct(CEL_NONNULL(cel_AstTraverser*)
                                           traverser) {
  _cel_Deque_Destruct(&traverser->records, cel_DefaultAllocator);
}

extern "C" CEL_NULLABLE(cel_AstTraverser*)
    cel_AstTraverser_New(CEL_NONNULL(const cel_Ast*) ast,
                         CEL_NONNULL(cel_AstVisitor*) visitor) {
  CEL_ASSERT_NOT_NULL(ast);
  CEL_ASSERT_NOT_NULL(visitor);

  CEL_NULLABLE(cel_AstTraverser*)
  ast_traverser = (CEL_NULLABLE(cel_AstTraverser*))_cel_Malloc(
      sizeof(cel_AstTraverser), cel_nullptr);
  if (ast_traverser == cel_nullptr) {
    return cel_nullptr;
  }
  _cel_AstTraverser_Construct(ast_traverser, visitor);
  CEL_NULLABLE(cel_Expr*) expr = cel_Ast_Expr(ast);
  if (expr != cel_nullptr) {
    if (!_cel_AstTraverser_PushExpr(ast_traverser, expr,
                                    /*status=*/cel_nullptr)) {
      cel_AstTraverser_Delete(ast_traverser);
      return cel_nullptr;
    }
  }
  return ast_traverser;
}

extern "C" void cel_AstTraverser_Delete(CEL_NULLABLE(cel_AstTraverser*)
                                            ast_traverser) {
  if (ast_traverser == cel_nullptr) {
    return;
  }
  _cel_AstTraverser_Destruct(ast_traverser);
  _cel_FreeSized(ast_traverser, sizeof(cel_AstTraverser));
}

extern "C" void cel_AstTraverser_StepIn(
    cel_AstTraverser* cel_nonnull ast_traverser) {
  CEL_ASSERT_NOT_NULL(ast_traverser);

  ast_traverser->step = _cel_AstTraverserStep_kStepIn;
  ast_traverser->stepped = true;
}

extern "C" void cel_AstTraverser_StepOut(
    cel_AstTraverser* cel_nonnull ast_traverser) {
  CEL_ASSERT_NOT_NULL(ast_traverser);

  ast_traverser->step = _cel_AstTraverserStep_kStepOut;
  ast_traverser->stepped = true;
}

extern "C" void cel_AstTraverser_StepOver(
    cel_AstTraverser* cel_nonnull ast_traverser) {
  CEL_ASSERT_NOT_NULL(ast_traverser);

  ast_traverser->step = _cel_AstTraverserStep_kStepOver;
  ast_traverser->stepped = true;
}

extern "C" bool cel_AstTraverser_Traverse(CEL_NONNULL(cel_AstTraverser*)
                                              ast_traverser,
                                          CEL_NONNULL(cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(ast_traverser);
  CEL_ASSERT(cel_Status_Ok(status));

  while (true) {
    _cel_AstTraverserRecord* record = _cel_AstTraverser_Peek(ast_traverser);
    if (record == cel_nullptr) {
      return false;
    }
    while (true) {
      if (!record->data.previsited || !record->data.previsited_expr) {
        ast_traverser->stepped = false;
        switch (
            _cel_AstTraverserRecord_PreVisit(record, ast_traverser, status)) {
          case _cel_AstVisitorControl_kStop:
            return cel_Status_Ok(status);
          case _cel_AstVisitorControl_kContinue:
            CEL_ASSERT_NOT(ast_traverser->stepped);
            CEL_ASSERT(cel_Status_Ok(status));
            break;
          default:
            CEL_UNREACHABLE();
        }
      } else if (!record->data.pushed_deps) {
        if (!_cel_AstTraverser_PushDeps(ast_traverser, record, status)) {
          return false;
        }
        record->data.pushed_deps = true;
        break;
      } else if (!record->data.postvisited || !record->data.postvisited_expr) {
        ast_traverser->stepped = false;
        switch (
            _cel_AstTraverserRecord_PostVisit(record, ast_traverser, status)) {
          case _cel_AstVisitorControl_kStop:
            return cel_Status_Ok(status);
          case _cel_AstVisitorControl_kContinue:
            CEL_ASSERT_NOT(ast_traverser->stepped);
            CEL_ASSERT(cel_Status_Ok(status));
            break;
          default:
            CEL_UNREACHABLE();
        }
      } else {
        _cel_AstTraverser_Pop(ast_traverser);
        break;
      }
    }
  }
}
