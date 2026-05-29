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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/ast.h"
#include "cel-c/config.h"
#include "cel-c/constant.h"
#include "cel-c/operators.h"
#include "cel-c/ref.h"
#include "cel-c/source.h"
#include "cel-c/string_view.h"
#include "cel-c/type.h"

typedef struct _cel_ExprLink _cel_ExprLink;

struct _cel_ExprLink {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  CEL_NULLABLE(_cel_ExprLink*) left;
  CEL_NULLABLE(_cel_ExprLink*) right;
  ptrdiff_t index;
};

struct cel_UnspecifiedExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;
};

struct cel_IdentExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  cel_StringView name;
};

struct cel_ConstExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  cel_Constant value;
};

struct cel_SelectExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  CEL_NULLABLE(cel_Expr*) operand;
  cel_StringView field;
  bool test_only;
};

struct cel_CallExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  cel_StringView function;
  CEL_NULLABLE(cel_Expr*) target;
  CEL_NULLABLE(_cel_ExprLink*) args_head;
  CEL_NULLABLE(_cel_ExprLink*) args_tail;
  size_t args;
};

struct cel_CallArgExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  CEL_NULLABLE(_cel_ExprLink*) left;
  CEL_NULLABLE(_cel_ExprLink*) right;
  ptrdiff_t index;
  CEL_NULLABLE(cel_Expr*) value;
};

struct cel_UnaryExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  cel_UnaryOp op;
  CEL_NULLABLE(cel_Expr*) args[1];
};

struct cel_BinaryExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  cel_BinaryOp op;
  CEL_NULLABLE(cel_Expr*) args[2];
};

struct cel_TernaryExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  cel_TernaryOp op;
  CEL_NULLABLE(cel_Expr*) args[3];
};

struct cel_ListElementExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  CEL_NULLABLE(_cel_ExprLink*) left;
  CEL_NULLABLE(_cel_ExprLink*) right;
  ptrdiff_t index;
  CEL_NULLABLE(cel_Expr*) value;
  bool optional;
};

struct cel_ListExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  CEL_NULLABLE(_cel_ExprLink*) elems_head;
  CEL_NULLABLE(_cel_ExprLink*) elems_tail;
  size_t elems;
};

struct cel_MapEntryExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  CEL_NULLABLE(_cel_ExprLink*) left;
  CEL_NULLABLE(_cel_ExprLink*) right;
  ptrdiff_t index;
  CEL_NULLABLE(cel_Expr*) key;
  CEL_NULLABLE(cel_Expr*) value;
  bool optional;
};

struct cel_MapExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  CEL_NULLABLE(_cel_ExprLink*) ents_head;
  CEL_NULLABLE(_cel_ExprLink*) ents_tail;
  size_t ents;
};

struct cel_StructFieldExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  CEL_NULLABLE(_cel_ExprLink*) left;
  CEL_NULLABLE(_cel_ExprLink*) right;
  ptrdiff_t index;
  cel_StringView name;
  CEL_NULLABLE(cel_Expr*) value;
  bool optional;
};

struct cel_StructExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  cel_StringView name;
  CEL_NULLABLE(_cel_ExprLink*) flds_head;
  CEL_NULLABLE(_cel_ExprLink*) flds_tail;
  size_t flds;
};

struct cel_ComprehensionExpr {
  cel_ExprId id;
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NULLABLE(cel_Expr*) parent;
  cel_ExprKind kind;
  cel_SourceRange range;
  CEL_NULLABLE(const cel_Ref *) ref;
  CEL_NULLABLE(cel_Type *) type;

  cel_StringView iter_var;
  cel_StringView iter_var2;
  CEL_NULLABLE(cel_Expr*) iter_range;
  cel_StringView accu_var;
  CEL_NULLABLE(cel_Expr*) accu_init;
  CEL_NULLABLE(cel_Expr*) loop_condition;
  CEL_NULLABLE(cel_Expr*) loop_step;
  CEL_NULLABLE(cel_Expr*) result;
};

struct cel_Expr {
  union {
    struct {
      cel_ExprId id;
      CEL_NONNULL(cel_Ast*) ast;
      CEL_NULLABLE(cel_Expr*) parent;
      cel_ExprKind kind;
      cel_SourceRange range;
      CEL_NULLABLE(const cel_Ref *) ref;
      CEL_NULLABLE(const cel_Type *) type;
    };
    _cel_ExprLink link;
    cel_UnspecifiedExpr unspecified_expr;
    cel_IdentExpr ident_expr;
    cel_ConstExpr const_expr;
    cel_SelectExpr select_expr;
    cel_CallExpr call_expr;
    cel_UnaryExpr unary_expr;
    cel_BinaryExpr binary_expr;
    cel_TernaryExpr ternary_expr;
    cel_ListElementExpr optional_expr;
    cel_ListExpr list_expr;
    cel_MapEntryExpr map_entry_expr;
    cel_MapExpr map_expr;
    cel_StructFieldExpr struct_field_expr;
    cel_StructExpr struct_expr;
    cel_ComprehensionExpr comprehension_expr;
  };
};

struct cel_Ast {
  CEL_NONNULL(cel_Arena*) arena;
  cel_ExprId next_id;
  CEL_NULLABLE(cel_Expr*) expr;
};

cel_ExprId cel_Ast_NextId(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  return ++ast->next_id;
}

cel_ExprId cel_Ast_MaxId(CEL_NONNULL(const cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  return ast->next_id;
}

CEL_NULLABLE(cel_Expr*) cel_Ast_Expr(CEL_NONNULL(const cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);
  CEL_ASSERT(ast->expr == cel_nullptr ||
             (ast->expr->ast == ast && ast->expr->parent == cel_nullptr));

  return ast->expr;
}

CEL_NULLABLE(cel_Expr*)
cel_Ast_SetExpr(CEL_NONNULL(cel_Ast*) ast, CEL_NULLABLE(cel_Expr*) expr) {
  CEL_ASSERT_NOT_NULL(ast);
  CEL_ASSERT(expr == cel_nullptr ||
             (expr->ast == ast && expr->parent == cel_nullptr));
  CEL_ASSERT(ast->expr == cel_nullptr ||
             (ast->expr->ast == ast && ast->expr->parent == cel_nullptr));

  CEL_NULLABLE(cel_Expr*) old_expr = ast->expr;
  ast->expr = expr;
  return old_expr;
}

CEL_NULLABLE(cel_Expr*)
cel_Ast_ReleaseExpr(CEL_NONNULL(cel_Ast*) ast) {
  return cel_Ast_SetExpr(ast, cel_nullptr);
}

CEL_NULLABLE(cel_Ast*) cel_Ast_New(CEL_NONNULL(cel_Arena*) arena) {
  CEL_ASSERT_NOT_NULL(arena);

  CEL_NULLABLE(cel_Ast*)
  ast = (CEL_NULLABLE(cel_Ast*))cel_Arena_Malloc(arena, sizeof(cel_Ast),
                                                 cel_nullptr);
  if (ast != cel_nullptr) {
    memset(ast, 0, sizeof(*ast));
    ast->arena = arena;
  }
  return ast;
}

void cel_Ast_Delete(CEL_NULLABLE(cel_Ast*) ast) {}

CEL_ATTRIBUTE_NODISCARD
static CEL_NULLABLE(cel_Expr*)
    _cel_Expr_ReleaseChild(CEL_NONNULL(cel_Expr*) expr,
                           cel_Expr* cel_nullable* cel_nonnull child_ptr) {
  CEL_NULLABLE(cel_Expr*) child = *child_ptr;
  if (child != cel_nullptr) {
    CEL_ASSERT_EQ(child->parent, expr);
    *child_ptr = cel_nullptr;
    child->parent = cel_nullptr;
  }
  return child;
}

static CEL_NULLABLE(cel_Expr*)
    _cel_Expr_SetChild(CEL_NONNULL(cel_Expr*) expr,
                       cel_Expr* cel_nullable* cel_nonnull child_ptr,
                       CEL_NULLABLE(cel_Expr*) child) {
  CEL_ASSERT(child == cel_nullptr ||
             (child->ast == expr->ast && child->parent == cel_nullptr));

  CEL_NULLABLE(cel_Expr*) old_child = _cel_Expr_ReleaseChild(expr, child_ptr);
  *child_ptr = child;
  if (child != cel_nullptr) {
    child->parent = expr;
  }
  return old_child;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_NULLABLE(cel_Expr*)
    _cel_Expr_Child(CEL_NONNULL(const cel_Expr*) expr,
                    CEL_NULLABLE(cel_Expr*) child) {
  CEL_ASSERT(child == cel_nullptr ||
             (child->ast == expr->ast && child->parent == expr));

  return child;
}

static void _cel_Expr_LinkChild(
    CEL_NONNULL(cel_Expr*) expr,
    _cel_ExprLink* cel_nullable* cel_nonnull head_ptr,
    _cel_ExprLink* cel_nullable* cel_nonnull tail_ptr,
    CEL_NONNULL(size_t*) size_ptr, CEL_NULLABLE(_cel_ExprLink*) next,
    CEL_NONNULL(_cel_ExprLink*) child) {
  CEL_ASSERT(child != cel_nullptr && child->ast == expr->ast &&
             child->parent == cel_nullptr && child->left == cel_nullptr &&
             child->right == cel_nullptr &&
             (next == cel_nullptr || next->parent == expr));

  if (next == cel_nullptr) {
    // nullptr is end
    CEL_NULLABLE(_cel_ExprLink*) head = *head_ptr;
    CEL_NULLABLE(_cel_ExprLink*) tail = *tail_ptr;
    child->left = tail;
    if (tail != cel_nullptr) {
      CEL_ASSERT_NOT_NULL(head);
      tail->right = child;
      child->index = tail->index + 1;
    } else {
      CEL_ASSERT_NULL(head);
      *head_ptr = child;
      child->index = 0;
    }
    *tail_ptr = child;
  } else {
    CEL_NULLABLE(_cel_ExprLink*) prev = next->left;
    child->right = next;
    child->index = next->index;
    child->left = prev;
    next->left = child;
    if (prev != cel_nullptr) {
      prev->right = child;
    } else {
      CEL_ASSERT_EQ(*head_ptr, next);
      *head_ptr = child;
    }
    // Fixup indices.
    do {
      ++next->index;
      next = next->right;
    } while (next != cel_nullptr);
  }

  child->parent = expr;
  ++(*size_ptr);
}

static void _cel_Expr_UnlinkChild(
    CEL_NONNULL(cel_Expr*) expr,
    _cel_ExprLink* cel_nullable* cel_nonnull head_ptr,
    _cel_ExprLink* cel_nullable* cel_nonnull tail_ptr,
    CEL_NONNULL(size_t*) size_ptr, CEL_NONNULL(_cel_ExprLink*) child) {
  CEL_ASSERT(child != cel_nullptr && child->ast == expr->ast &&
             child->parent == expr);

  if (child->left != cel_nullptr) {
    child->left->right = child->right;
  } else {
    CEL_ASSERT_EQ(*head_ptr, child);
    *head_ptr = child->right;
  }

  CEL_NULLABLE(_cel_ExprLink*)
  right = child->right;
  if (right != cel_nullptr) {
    right->left = child->left;
    // Fixup indices.
    do {
      --right->index;
      right = right->right;
    } while (right != cel_nullptr);
  } else {
    CEL_ASSERT_EQ(*tail_ptr, child);
    *tail_ptr = child->left;
  }

  --(*size_ptr);

  child->parent = cel_nullptr;
  child->left = child->right = cel_nullptr;
  child->index = -1;
}

cel_ExprKind cel_Expr_Kind(CEL_NONNULL(const cel_Expr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->kind;
}

cel_ExprId cel_Expr_Id(CEL_NONNULL(const cel_Expr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->id;
}

void cel_Expr_SetId(CEL_NONNULL(cel_Expr*) expr, cel_ExprId id) {
  CEL_ASSERT_NOT_NULL(expr);
  CEL_ASSERT_GE(id, 0);

  if (id < 0) {
    id = 0;
  }

  if (id > expr->ast->next_id) {
    expr->ast->next_id = id;
  }

  expr->id = id;
}

int32_t cel_Expr_SourcePosition(CEL_NONNULL(const cel_Expr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->range.begin;
}

cel_SourceRange cel_Expr_SourceRange(CEL_NONNULL(const cel_Expr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->range;
}

void cel_Expr_SetSourcePosition(CEL_NONNULL(cel_Expr*) expr,
                                int32_t source_position) {
  CEL_ASSERT_NOT_NULL(expr);
  CEL_ASSERT_GE(source_position, -1);

  if (source_position < -1) {
    source_position = -1;
  }

  expr->range.begin = source_position;
  expr->range.end = -1;
}

void cel_Expr_SetSourceRange(CEL_NONNULL(cel_Expr*) expr,
                             cel_SourceRange source_range) {
  CEL_ASSERT_NOT_NULL(expr);
  CEL_ASSERT_GE(source_range.begin, -1);
  CEL_ASSERT_GE(source_range.end, -1);
  CEL_ASSERT(source_range.begin <= source_range.end || source_range.end == -1);

  if (source_range.begin < -1) {
    source_range.begin = -1;
  }
  if (source_range.end < -1) {
    source_range.end = -1;
  }

  expr->range = source_range;
}

CEL_NULLABLE(cel_Expr*)
cel_Expr_Parent(CEL_NONNULL(const cel_Expr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->parent;
}

size_t cel_Expr_Depth(CEL_NONNULL(const cel_Expr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  CEL_NULLABLE(const cel_Expr*) current = expr;
  current = cel_Expr_Parent(current);
  size_t depth = 0;
  while (current != cel_nullptr) {
    ++depth;
    current = cel_Expr_Parent(current);
  }
  return depth;
}

CEL_NULLABLE(const cel_Ref *)
cel_Expr_Ref(CEL_NONNULL(const cel_Expr *) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->ref;
}

void cel_Expr_SetRef(CEL_NONNULL(cel_Expr *) expr,
                     CEL_NULLABLE(const cel_Ref *) ref) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->ref = ref;
}

CEL_NONNULL(const cel_Type *)
cel_Expr_Type(CEL_NONNULL(const cel_Expr *) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  if (expr->type == cel_nullptr) {
    return cel_DynType;
  }
  return expr->type;
}

void cel_Expr_SetType(CEL_NONNULL(cel_Expr *) expr,
                      CEL_NONNULL(const cel_Type *) type) {
  CEL_ASSERT_NOT_NULL(expr);
  CEL_ASSERT_NOT_NULL(type);

  expr->type = type;
}

CEL_NULLABLE(cel_UnspecifiedExpr*)
cel_UnspecifiedExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_UnspecifiedExpr*)
  expr = (CEL_NULLABLE(cel_UnspecifiedExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_UnspecifiedExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kUnspecified;
    expr->range = cel_SourceRange(-1, -1);
  }
  return expr;
}

CEL_NULLABLE(cel_IdentExpr*)
cel_IdentExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_IdentExpr*)
  expr = (CEL_NULLABLE(cel_IdentExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_IdentExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kIdent;
    expr->range = cel_SourceRange(-1, -1);
  }
  return expr;
}

cel_StringView cel_IdentExpr_Name(CEL_NONNULL(const cel_IdentExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->name;
}

void cel_IdentExpr_SetName(CEL_NONNULL(cel_IdentExpr*) expr,
                           cel_StringView name) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->name = name;
}

CEL_NULLABLE(cel_ConstExpr*)
cel_ConstExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_ConstExpr*)
  expr = (CEL_NULLABLE(cel_ConstExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_ConstExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kConst;
    expr->range = cel_SourceRange(-1, -1);
  }
  return expr;
}

CEL_NONNULL(const cel_Constant*)
cel_ConstExpr_Value(CEL_NONNULL(const cel_ConstExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return &expr->value;
}

CEL_NONNULL(cel_Constant*)
cel_ConstExpr_MutableValue(CEL_NONNULL(cel_ConstExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return &expr->value;
}

CEL_NULLABLE(cel_SelectExpr*)
cel_SelectExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_SelectExpr*)
  expr = (CEL_NULLABLE(cel_SelectExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_SelectExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kSelect;
    expr->range = cel_SourceRange(-1, -1);
  }
  return expr;
}

CEL_NULLABLE(cel_Expr*)
cel_SelectExpr_Operand(CEL_NONNULL(const cel_SelectExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->operand);
}

CEL_NULLABLE(cel_Expr*)
cel_SelectExpr_SetOperand(CEL_NONNULL(cel_SelectExpr*) expr,
                          CEL_NULLABLE(cel_Expr*) operand) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->operand, operand);
}

CEL_NULLABLE(cel_Expr*)
cel_SelectExpr_ReleaseOperand(CEL_NONNULL(cel_SelectExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->operand);
}

cel_StringView cel_SelectExpr_Field(CEL_NONNULL(const cel_SelectExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->field;
}

void cel_SelectExpr_SetField(CEL_NONNULL(cel_SelectExpr*) expr,
                             cel_StringView field) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->field = field;
}

bool cel_SelectExpr_TestOnly(CEL_NONNULL(const cel_SelectExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->test_only;
}

void cel_SelectExpr_SetTestOnly(CEL_NONNULL(cel_SelectExpr*) expr,
                                bool test_only) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->test_only = test_only;
}

CEL_NULLABLE(cel_CallArgExpr*)
cel_CallArgExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_CallArgExpr*)
  expr = (CEL_NULLABLE(cel_CallArgExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_CallArgExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kCallArg;
    expr->range = cel_SourceRange(-1, -1);
    expr->index = -1;
  }
  return expr;
}

CEL_NULLABLE(cel_CallExpr*)
cel_CallArgExpr_Parent(CEL_NONNULL(const cel_CallArgExpr*) expr) {
  CEL_NULLABLE(cel_Expr*) parent = cel_Expr_Parent(cel_Expr_UpCast(expr));
  return parent != cel_nullptr ? cel_CallExpr_DownCast(parent) : cel_nullptr;
}

ptrdiff_t cel_CallArgExpr_Index(CEL_NONNULL(const cel_CallArgExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->index;
}

CEL_NULLABLE(cel_Expr*)
cel_CallArgExpr_Value(CEL_NONNULL(const cel_CallArgExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->value);
}

CEL_NULLABLE(cel_Expr*)
cel_CallArgExpr_SetValue(CEL_NONNULL(cel_CallArgExpr*) expr,
                         CEL_NULLABLE(cel_Expr*) value) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->value, value);
}

CEL_NULLABLE(cel_Expr*)
cel_CallArgExpr_ReleaseValue(CEL_NONNULL(cel_CallArgExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->value);
}

CEL_NULLABLE(cel_CallExpr*)
cel_CallExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_CallExpr*)
  expr = (CEL_NULLABLE(cel_CallExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_CallExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kCall;
    expr->range = cel_SourceRange(-1, -1);
  }
  return expr;
}

CEL_NULLABLE(cel_Expr*)
cel_CallExpr_Target(CEL_NONNULL(const cel_CallExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->target);
}

CEL_NULLABLE(cel_Expr*)
cel_CallExpr_SetTarget(CEL_NONNULL(cel_CallExpr*) expr,
                       CEL_NULLABLE(cel_Expr*) target) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->target, target);
}

CEL_NULLABLE(cel_Expr*)
cel_CallExpr_ReleaseTarget(CEL_NONNULL(cel_CallExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->target);
}

cel_StringView cel_CallExpr_Function(CEL_NONNULL(const cel_CallExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->function;
}

void cel_CallExpr_SetFunction(CEL_NONNULL(cel_CallExpr*) expr,
                              cel_StringView function) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->function = function;
}

void cel_CallExpr_AppendArg(CEL_NONNULL(cel_CallExpr*) expr,
                            CEL_NONNULL(cel_CallArgExpr*) arg) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_LinkChild(cel_Expr_UpCast(expr), &expr->args_head, &expr->args_tail,
                      &expr->args, cel_nullptr, (_cel_ExprLink*)arg);
}

void cel_CallExpr_PrependArg(CEL_NONNULL(cel_CallExpr*) expr,
                             CEL_NONNULL(cel_CallArgExpr*) arg) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_LinkChild(cel_Expr_UpCast(expr), &expr->args_head, &expr->args_tail,
                      &expr->args, expr->args_head, (_cel_ExprLink*)arg);
}

void cel_CallExpr_InsertArg(CEL_NONNULL(cel_CallExpr*) expr,
                            CEL_NULLABLE(cel_CallArgExpr*) before,
                            CEL_NONNULL(cel_CallArgExpr*) arg) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_LinkChild(cel_Expr_UpCast(expr), &expr->args_head, &expr->args_tail,
                      &expr->args, (_cel_ExprLink*)before, (_cel_ExprLink*)arg);
}

CEL_NONNULL(cel_CallArgExpr*)
cel_CallExpr_ReleaseArg(CEL_NONNULL(cel_CallExpr*) expr,
                        CEL_NONNULL(cel_CallArgExpr*) arg) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_UnlinkChild(cel_Expr_UpCast(expr), &expr->args_head,
                        &expr->args_tail, &expr->args, (_cel_ExprLink*)arg);
  return arg;
}

CEL_NONNULL(cel_CallArgExpr*)
cel_CallExpr_Arg(CEL_NONNULL(const cel_CallExpr*) expr, size_t index) {
  CEL_ASSERT_NOT_NULL(expr);
  CEL_ASSERT_LT(index, expr->args);

  CEL_NULLABLE(_cel_ExprLink*) arg = expr->args_head;
  for (; index > 0; --index) {
    CEL_ASSERT_NOT_NULL(arg);
    arg = arg->right;
  }
  CEL_ASSERT_NOT_NULL(arg);
  CEL_ASSERT(arg->parent == cel_Expr_UpCast(expr));
  return (cel_CallArgExpr*)arg;
}

size_t cel_CallExpr_Args(CEL_NONNULL(const cel_CallExpr*) expr,
                         CEL_NULLABLE(cel_CallArgExpr*) * cel_nullable head,
                         CEL_NULLABLE(cel_CallArgExpr*) * cel_nullable tail) {
  CEL_ASSERT_NOT_NULL(expr);

  if (head != cel_nullptr) {
    *head = (cel_CallArgExpr*)expr->args_head;
  }
  if (tail != cel_nullptr) {
    *tail = (cel_CallArgExpr*)expr->args_tail;
  }
  return expr->args;
}

CEL_NULLABLE(cel_CallArgExpr*)
cel_CallExpr_PrevArg(CEL_NULLABLE(const cel_CallArgExpr*) arg) {
  if (arg != cel_nullptr) {
    arg = (cel_CallArgExpr*)arg->left;
  }
  return (cel_CallArgExpr*)arg;
}

CEL_NULLABLE(cel_CallArgExpr*)
cel_CallExpr_NextArg(CEL_NULLABLE(const cel_CallArgExpr*) arg) {
  if (arg != cel_nullptr) {
    arg = (cel_CallArgExpr*)arg->right;
  }
  return (cel_CallArgExpr*)arg;
}

CEL_NULLABLE(cel_UnaryExpr*)
cel_UnaryExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_UnaryExpr*)
  expr = (CEL_NULLABLE(cel_UnaryExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_UnaryExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kUnary;
    expr->range = cel_SourceRange(-1, -1);
  }
  return expr;
}

cel_UnaryOp cel_UnaryExpr_Op(CEL_NONNULL(const cel_UnaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->op;
}

void cel_UnaryExpr_SetOp(CEL_NONNULL(cel_UnaryExpr*) expr, cel_UnaryOp op) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->op = op;
}

CEL_NULLABLE(cel_Expr*)
cel_UnaryExpr_Arg(CEL_NONNULL(const cel_UnaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->args[0]);
}

CEL_NULLABLE(cel_Expr*)
cel_UnaryExpr_SetArg(CEL_NONNULL(cel_UnaryExpr*) expr,
                     CEL_NULLABLE(cel_Expr*) arg) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->args[0], arg);
}

CEL_NULLABLE(cel_Expr*)
cel_UnaryExpr_ReleaseArg(CEL_NONNULL(cel_UnaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->args[0]);
}

CEL_NULLABLE(cel_BinaryExpr*)
cel_BinaryExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_BinaryExpr*)
  expr = (CEL_NULLABLE(cel_BinaryExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_BinaryExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kBinary;
    expr->range = cel_SourceRange(-1, -1);
  }
  return expr;
}

cel_BinaryOp cel_BinaryExpr_Op(CEL_NONNULL(const cel_BinaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->op;
}

void cel_BinaryExpr_SetOp(CEL_NONNULL(cel_BinaryExpr*) expr, cel_BinaryOp op) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->op = op;
}

CEL_NULLABLE(cel_Expr*)
cel_BinaryExpr_Left(CEL_NONNULL(const cel_BinaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->args[0]);
}

CEL_NULLABLE(cel_Expr*)
cel_BinaryExpr_SetLeft(CEL_NONNULL(cel_BinaryExpr*) expr,
                       CEL_NULLABLE(cel_Expr*) left) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->args[0], left);
}

CEL_NULLABLE(cel_Expr*)
cel_BinaryExpr_ReleaseLeft(CEL_NONNULL(cel_BinaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->args[0]);
}

CEL_NULLABLE(cel_Expr*)
cel_BinaryExpr_Right(CEL_NONNULL(const cel_BinaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->args[1]);
}

CEL_NULLABLE(cel_Expr*)
cel_BinaryExpr_SetRight(CEL_NONNULL(cel_BinaryExpr*) expr,
                        CEL_NULLABLE(cel_Expr*) right) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->args[1], right);
}

CEL_NULLABLE(cel_Expr*)
cel_BinaryExpr_ReleaseRight(CEL_NONNULL(cel_BinaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->args[1]);
}

CEL_NULLABLE(cel_TernaryExpr*)
cel_TernaryExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_TernaryExpr*)
  expr = (CEL_NULLABLE(cel_TernaryExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_TernaryExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kTernary;
    expr->range = cel_SourceRange(-1, -1);
  }
  return expr;
}

cel_TernaryOp cel_TernaryExpr_Op(CEL_NONNULL(const cel_TernaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->op;
}

void cel_TernaryExpr_SetOp(CEL_NONNULL(cel_TernaryExpr*) expr,
                           cel_TernaryOp op) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->op = op;
}

CEL_NULLABLE(cel_Expr*)
cel_TernaryExpr_Condition(CEL_NONNULL(const cel_TernaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->args[0]);
}

CEL_NULLABLE(cel_Expr*)
cel_TernaryExpr_SetCondition(CEL_NONNULL(cel_TernaryExpr*) expr,
                             CEL_NULLABLE(cel_Expr*) condition) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->args[0], condition);
}

CEL_NULLABLE(cel_Expr*)
cel_TernaryExpr_ReleaseCondition(CEL_NONNULL(cel_TernaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->args[0]);
}

CEL_NULLABLE(cel_Expr*)
cel_TernaryExpr_IfTrue(CEL_NONNULL(const cel_TernaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->args[1]);
}

CEL_NULLABLE(cel_Expr*)
cel_TernaryExpr_SetIfTrue(CEL_NONNULL(cel_TernaryExpr*) expr,
                          CEL_NULLABLE(cel_Expr*) if_true) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->args[1], if_true);
}

CEL_NULLABLE(cel_Expr*)
cel_TernaryExpr_ReleaseIfTrue(CEL_NONNULL(cel_TernaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->args[1]);
}

CEL_NULLABLE(cel_Expr*)
cel_TernaryExpr_IfFalse(CEL_NONNULL(const cel_TernaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->args[2]);
}

CEL_NULLABLE(cel_Expr*)
cel_TernaryExpr_SetIfFalse(CEL_NONNULL(cel_TernaryExpr*) expr,
                           CEL_NULLABLE(cel_Expr*) if_false) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->args[2], if_false);
}

CEL_NULLABLE(cel_Expr*)
cel_TernaryExpr_ReleaseIfFalse(CEL_NONNULL(cel_TernaryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->args[2]);
}

CEL_NULLABLE(cel_ListElementExpr*)
cel_ListElementExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_ListElementExpr*)
  expr = (CEL_NULLABLE(cel_ListElementExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_ListElementExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kListElement;
    expr->range = cel_SourceRange(-1, -1);
    expr->index = -1;
  }
  return expr;
}

CEL_NULLABLE(cel_ListExpr*)
cel_ListElementExpr_Parent(CEL_NONNULL(const cel_ListElementExpr*) expr) {
  CEL_NULLABLE(cel_Expr*) parent = cel_Expr_Parent(cel_Expr_UpCast(expr));
  return parent != cel_nullptr ? cel_ListExpr_DownCast(parent) : cel_nullptr;
}

ptrdiff_t cel_ListElementExpr_Index(CEL_NONNULL(const cel_ListElementExpr*)
                                        expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->index;
}

CEL_NULLABLE(cel_Expr*)
cel_ListElementExpr_Value(CEL_NONNULL(const cel_ListElementExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->value);
}

CEL_NULLABLE(cel_Expr*)
cel_ListElementExpr_SetValue(CEL_NONNULL(cel_ListElementExpr*) expr,
                             CEL_NULLABLE(cel_Expr*) value) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->value, value);
}

CEL_NULLABLE(cel_Expr*)
cel_ListElementExpr_ReleaseValue(CEL_NONNULL(cel_ListElementExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->value);
}

bool cel_ListElementExpr_Optional(CEL_NONNULL(const cel_ListElementExpr*)
                                      expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->optional;
}

void cel_ListElementExpr_SetOptional(CEL_NONNULL(cel_ListElementExpr*) expr,
                                     bool optional) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->optional = optional;
}

CEL_NULLABLE(cel_ListExpr*)
cel_ListExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_ListExpr*)
  expr = (CEL_NULLABLE(cel_ListExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_ListExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kList;
    expr->range = cel_SourceRange(-1, -1);
  }
  return expr;
}

void cel_ListExpr_AppendElement(CEL_NONNULL(cel_ListExpr*) expr,
                                CEL_NONNULL(cel_ListElementExpr*) element) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_LinkChild(cel_Expr_UpCast(expr), &expr->elems_head,
                      &expr->elems_tail, &expr->elems, cel_nullptr,
                      (_cel_ExprLink*)element);
}

void cel_ListExpr_PrependElement(CEL_NONNULL(cel_ListExpr*) expr,
                                 CEL_NONNULL(cel_ListElementExpr*) element) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_LinkChild(cel_Expr_UpCast(expr), &expr->elems_head,
                      &expr->elems_tail, &expr->elems, expr->elems_head,
                      (_cel_ExprLink*)element);
}

void cel_ListExpr_InsertElement(CEL_NONNULL(cel_ListExpr*) expr,
                                CEL_NULLABLE(cel_ListElementExpr*) before,
                                CEL_NONNULL(cel_ListElementExpr*) element) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_LinkChild(cel_Expr_UpCast(expr), &expr->elems_head,
                      &expr->elems_tail, &expr->elems, (_cel_ExprLink*)before,
                      (_cel_ExprLink*)element);
}

CEL_NONNULL(cel_ListElementExpr*)
cel_ListExpr_ReleaseElement(CEL_NONNULL(cel_ListExpr*) expr,
                            CEL_NONNULL(cel_ListElementExpr*) element) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_UnlinkChild(cel_Expr_UpCast(expr), &expr->elems_head,
                        &expr->elems_tail, &expr->elems,
                        (_cel_ExprLink*)element);
  return element;
}

CEL_NONNULL(cel_ListElementExpr*)
cel_ListExpr_Element(CEL_NONNULL(const cel_ListExpr*) expr, size_t index) {
  CEL_ASSERT_NOT_NULL(expr);
  CEL_ASSERT_LT(index, expr->elems);

  CEL_NULLABLE(_cel_ExprLink*) element = expr->elems_head;
  for (; index > 0; --index) {
    CEL_ASSERT_NOT_NULL(element);
    element = element->right;
  }
  CEL_ASSERT_NOT_NULL(element);
  CEL_ASSERT(element->parent == cel_Expr_UpCast(expr));
  return (cel_ListElementExpr*)element;
}

size_t cel_ListExpr_Elements(CEL_NONNULL(const cel_ListExpr*) expr,
                             CEL_NULLABLE(cel_ListElementExpr*) *
                                 cel_nullable head,
                             CEL_NULLABLE(cel_ListElementExpr*) *
                                 cel_nullable tail) {
  CEL_ASSERT_NOT_NULL(expr);

  if (head != cel_nullptr) {
    *head = (cel_ListElementExpr*)expr->elems_head;
  }
  if (tail != cel_nullptr) {
    *tail = (cel_ListElementExpr*)expr->elems_tail;
  }
  return expr->elems;
}

CEL_NULLABLE(cel_ListElementExpr*)
cel_ListExpr_PrevElement(CEL_NULLABLE(const cel_ListElementExpr*) element) {
  if (element != cel_nullptr) {
    element = (cel_ListElementExpr*)element->left;
  }
  return (CEL_NULLABLE(cel_ListElementExpr*))element;
}

CEL_NULLABLE(cel_ListElementExpr*)
cel_ListExpr_NextElement(CEL_NULLABLE(const cel_ListElementExpr*) element) {
  if (element != cel_nullptr) {
    element = (cel_ListElementExpr*)element->right;
  }
  return (CEL_NULLABLE(cel_ListElementExpr*))element;
}

CEL_NULLABLE(cel_MapEntryExpr*)
cel_MapEntryExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_MapEntryExpr*)
  expr = (CEL_NULLABLE(cel_MapEntryExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_MapEntryExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kMapEntry;
    expr->range = cel_SourceRange(-1, -1);
    expr->index = -1;
  }
  return expr;
}

CEL_NULLABLE(cel_MapExpr*)
cel_MapEntryExpr_Parent(CEL_NONNULL(const cel_MapEntryExpr*) expr) {
  CEL_NULLABLE(cel_Expr*) parent = cel_Expr_Parent(cel_Expr_UpCast(expr));
  return parent != cel_nullptr ? cel_MapExpr_DownCast(parent) : cel_nullptr;
}

ptrdiff_t cel_MapEntryExpr_Index(CEL_NONNULL(const cel_MapEntryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->index;
}

CEL_NULLABLE(cel_Expr*)
cel_MapEntryExpr_Key(CEL_NONNULL(const cel_MapEntryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->key);
}

CEL_NULLABLE(cel_Expr*)
cel_MapEntryExpr_SetKey(CEL_NONNULL(cel_MapEntryExpr*) expr,
                        CEL_NULLABLE(cel_Expr*) key) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->key, key);
}

CEL_NULLABLE(cel_Expr*)
cel_MapEntryExpr_ReleaseKey(CEL_NONNULL(cel_MapEntryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->key);
}

CEL_NULLABLE(cel_Expr*)
cel_MapEntryExpr_Value(CEL_NONNULL(const cel_MapEntryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->value);
}

CEL_NULLABLE(cel_Expr*)
cel_MapEntryExpr_SetValue(CEL_NONNULL(cel_MapEntryExpr*) expr,
                          CEL_NULLABLE(cel_Expr*) value) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->value, value);
}

CEL_NULLABLE(cel_Expr*)
cel_MapEntryExpr_ReleaseValue(CEL_NONNULL(cel_MapEntryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->value);
}

bool cel_MapEntryExpr_Optional(CEL_NONNULL(const cel_MapEntryExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->optional;
}

void cel_MapEntryExpr_SetOptional(CEL_NONNULL(cel_MapEntryExpr*) expr,
                                  bool optional) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->optional = optional;
}

CEL_NULLABLE(cel_MapExpr*)
cel_MapExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_MapExpr*)
  expr = (CEL_NULLABLE(cel_MapExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_MapExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kMap;
    expr->range = cel_SourceRange(-1, -1);
  }
  return expr;
}

void cel_MapExpr_AppendEntry(CEL_NONNULL(cel_MapExpr*) expr,
                             CEL_NONNULL(cel_MapEntryExpr*) entry) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_LinkChild(cel_Expr_UpCast(expr), &expr->ents_head, &expr->ents_tail,
                      &expr->ents, cel_nullptr, (_cel_ExprLink*)entry);
}

void cel_MapExpr_PrependEntry(CEL_NONNULL(cel_MapExpr*) expr,
                              CEL_NONNULL(cel_MapEntryExpr*) entry) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_LinkChild(cel_Expr_UpCast(expr), &expr->ents_head, &expr->ents_tail,
                      &expr->ents, expr->ents_head, (_cel_ExprLink*)entry);
}

void cel_MapExpr_InsertEntry(CEL_NONNULL(cel_MapExpr*) expr,
                             CEL_NULLABLE(cel_MapEntryExpr*) before,
                             CEL_NONNULL(cel_MapEntryExpr*) entry) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_LinkChild(cel_Expr_UpCast(expr), &expr->ents_head, &expr->ents_tail,
                      &expr->ents, (_cel_ExprLink*)before,
                      (_cel_ExprLink*)entry);
}

CEL_NONNULL(cel_MapEntryExpr*)
cel_MapExpr_ReleaseEntry(CEL_NONNULL(cel_MapExpr*) expr,
                         CEL_NONNULL(cel_MapEntryExpr*) entry) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_UnlinkChild(cel_Expr_UpCast(expr), &expr->ents_head,
                        &expr->ents_tail, &expr->ents, (_cel_ExprLink*)entry);
  return entry;
}

CEL_NONNULL(cel_MapEntryExpr*)
cel_MapExpr_Entry(CEL_NONNULL(const cel_MapExpr*) expr, size_t index) {
  CEL_ASSERT_NOT_NULL(expr);
  CEL_ASSERT_LT(index, expr->ents);

  CEL_NULLABLE(_cel_ExprLink*) element = expr->ents_head;
  for (; index > 0; --index) {
    CEL_ASSERT_NOT_NULL(element);
    element = element->right;
  }
  CEL_ASSERT_NOT_NULL(element);
  CEL_ASSERT(element->parent == cel_Expr_UpCast(expr));
  return (cel_MapEntryExpr*)element;
}

size_t cel_MapExpr_Entries(CEL_NONNULL(const cel_MapExpr*) expr,
                           CEL_NULLABLE(cel_MapEntryExpr*) * cel_nullable head,
                           CEL_NULLABLE(cel_MapEntryExpr*) *
                               cel_nullable tail) {
  CEL_ASSERT_NOT_NULL(expr);

  if (head != cel_nullptr) {
    *head = (cel_MapEntryExpr*)expr->ents_head;
  }
  if (tail != cel_nullptr) {
    *tail = (cel_MapEntryExpr*)expr->ents_tail;
  }
  return expr->ents;
}

CEL_NULLABLE(cel_MapEntryExpr*)
cel_MapExpr_PrevEntry(CEL_NULLABLE(const cel_MapEntryExpr*) entry) {
  if (entry != cel_nullptr) {
    entry = (cel_MapEntryExpr*)entry->left;
  }
  return (CEL_NULLABLE(cel_MapEntryExpr*))entry;
}

CEL_NULLABLE(cel_MapEntryExpr*)
cel_MapExpr_NextEntry(CEL_NULLABLE(const cel_MapEntryExpr*) entry) {
  if (entry != cel_nullptr) {
    entry = (cel_MapEntryExpr*)entry->right;
  }
  return (CEL_NULLABLE(cel_MapEntryExpr*))entry;
}

CEL_NULLABLE(cel_StructFieldExpr*)
cel_StructFieldExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_StructFieldExpr*)
  expr = (CEL_NULLABLE(cel_StructFieldExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_StructFieldExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kStructField;
    expr->range = cel_SourceRange(-1, -1);
    expr->index = -1;
  }
  return expr;
}

CEL_NULLABLE(cel_StructExpr*)
cel_StructFieldExpr_Parent(CEL_NONNULL(const cel_StructFieldExpr*) expr) {
  CEL_NULLABLE(cel_Expr*) parent = cel_Expr_Parent(cel_Expr_UpCast(expr));
  return parent != cel_nullptr ? cel_StructExpr_DownCast(parent) : cel_nullptr;
}

ptrdiff_t cel_StructFieldExpr_Index(CEL_NONNULL(const cel_StructFieldExpr*)
                                        expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->index;
}

cel_StringView cel_StructFieldExpr_Name(CEL_NONNULL(const cel_StructFieldExpr*)
                                            expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->name;
}

void cel_StructFieldExpr_SetName(CEL_NONNULL(cel_StructFieldExpr*) expr,
                                 cel_StringView name) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->name = name;
}

CEL_NULLABLE(cel_Expr*)
cel_StructFieldExpr_Value(CEL_NONNULL(const cel_StructFieldExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->value);
}

CEL_NULLABLE(cel_Expr*)
cel_StructFieldExpr_SetValue(CEL_NONNULL(cel_StructFieldExpr*) expr,
                             CEL_NULLABLE(cel_Expr*) value) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->value, value);
}

CEL_NULLABLE(cel_Expr*)
cel_StructFieldExpr_ReleaseValue(CEL_NONNULL(cel_StructFieldExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->value);
}

bool cel_StructFieldExpr_Optional(CEL_NONNULL(const cel_StructFieldExpr*)
                                      expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->optional;
}

void cel_StructFieldExpr_SetOptional(CEL_NONNULL(cel_StructFieldExpr*) expr,
                                     bool optional) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->optional = optional;
}

CEL_NULLABLE(cel_StructExpr*)
cel_StructExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_StructExpr*)
  expr = (CEL_NULLABLE(cel_StructExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_StructExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kStruct;
    expr->range = cel_SourceRange(-1, -1);
  }
  return expr;
}

cel_StringView cel_StructExpr_Name(CEL_NONNULL(const cel_StructExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->name;
}

void cel_StructExpr_SetName(CEL_NONNULL(cel_StructExpr*) expr,
                            cel_StringView name) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->name = name;
}

void cel_StructExpr_AppendField(CEL_NONNULL(cel_StructExpr*) expr,
                                CEL_NONNULL(cel_StructFieldExpr*) field) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_LinkChild(cel_Expr_UpCast(expr), &expr->flds_head, &expr->flds_tail,
                      &expr->flds, cel_nullptr, (_cel_ExprLink*)field);
}

void cel_StructExpr_PrependField(CEL_NONNULL(cel_StructExpr*) expr,
                                 CEL_NONNULL(cel_StructFieldExpr*) field) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_LinkChild(cel_Expr_UpCast(expr), &expr->flds_head, &expr->flds_tail,
                      &expr->flds, expr->flds_head, (_cel_ExprLink*)field);
}

void cel_StructExpr_InsertField(CEL_NONNULL(cel_StructExpr*) expr,
                                CEL_NULLABLE(cel_StructFieldExpr*) before,
                                CEL_NONNULL(cel_StructFieldExpr*) field) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_LinkChild(cel_Expr_UpCast(expr), &expr->flds_head, &expr->flds_tail,
                      &expr->flds, (_cel_ExprLink*)before,
                      (_cel_ExprLink*)field);
}

CEL_NONNULL(cel_StructFieldExpr*)
cel_StructExpr_ReleaseField(CEL_NONNULL(cel_StructExpr*) expr,
                            CEL_NONNULL(cel_StructFieldExpr*) field) {
  CEL_ASSERT_NOT_NULL(expr);

  _cel_Expr_UnlinkChild(cel_Expr_UpCast(expr), &expr->flds_head,
                        &expr->flds_tail, &expr->flds, (_cel_ExprLink*)field);
  return field;
}

CEL_NONNULL(cel_StructFieldExpr*)
cel_StructExpr_Field(CEL_NONNULL(const cel_StructExpr*) expr, size_t index) {
  CEL_ASSERT_NOT_NULL(expr);
  CEL_ASSERT_LT(index, expr->flds);

  CEL_NULLABLE(_cel_ExprLink*) element = expr->flds_head;
  for (; index > 0; --index) {
    CEL_ASSERT_NOT_NULL(element);
    element = element->right;
  }
  CEL_ASSERT_NOT_NULL(element);
  CEL_ASSERT(element->parent == cel_Expr_UpCast(expr));
  return (cel_StructFieldExpr*)element;
}

size_t cel_StructExpr_Fields(CEL_NONNULL(const cel_StructExpr*) expr,
                             CEL_NULLABLE(cel_StructFieldExpr*) *
                                 cel_nullable head,
                             CEL_NULLABLE(cel_StructFieldExpr*) *
                                 cel_nullable tail) {
  CEL_ASSERT_NOT_NULL(expr);

  if (head != cel_nullptr) {
    *head = (cel_StructFieldExpr*)expr->flds_head;
  }
  if (tail != cel_nullptr) {
    *tail = (cel_StructFieldExpr*)expr->flds_tail;
  }
  return expr->flds;
}

CEL_NULLABLE(cel_StructFieldExpr*)
cel_StructExpr_PrevField(CEL_NULLABLE(const cel_StructFieldExpr*) field) {
  if (field != cel_nullptr) {
    field = (cel_StructFieldExpr*)field->left;
  }
  return (CEL_NULLABLE(cel_StructFieldExpr*))field;
}

CEL_NULLABLE(cel_StructFieldExpr*)
cel_StructExpr_NextField(CEL_NULLABLE(const cel_StructFieldExpr*) field) {
  if (field != cel_nullptr) {
    field = (cel_StructFieldExpr*)field->right;
  }
  return (CEL_NULLABLE(cel_StructFieldExpr*))field;
}

CEL_NULLABLE(cel_ComprehensionExpr*)
cel_ComprehensionExpr_New(CEL_NONNULL(cel_Ast*) ast) {
  CEL_ASSERT_NOT_NULL(ast);

  CEL_NULLABLE(cel_ComprehensionExpr*)
  expr = (CEL_NULLABLE(cel_ComprehensionExpr*))cel_Arena_Malloc(
      ast->arena, sizeof(cel_ComprehensionExpr), cel_nullptr);
  if (expr != cel_nullptr) {
    memset(expr, 0, sizeof(*expr));
    expr->ast = ast;
    expr->kind = cel_ExprKind_kComprehension;
    expr->range = cel_SourceRange(-1, -1);
  }
  return expr;
}

cel_StringView cel_ComprehensionExpr_IterVar(
    CEL_NONNULL(const cel_ComprehensionExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->iter_var;
}

void cel_ComprehensionExpr_SetIterVar(CEL_NONNULL(cel_ComprehensionExpr*) expr,
                                      cel_StringView iter_var) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->iter_var = iter_var;
}

cel_StringView cel_ComprehensionExpr_IterVar2(
    CEL_NONNULL(const cel_ComprehensionExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->iter_var2;
}

void cel_ComprehensionExpr_SetIterVar2(CEL_NONNULL(cel_ComprehensionExpr*) expr,
                                       cel_StringView iter_var2) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->iter_var2 = iter_var2;
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_IterRange(CEL_NONNULL(const cel_ComprehensionExpr*)
                                    expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->iter_range);
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_SetIterRange(CEL_NONNULL(cel_ComprehensionExpr*) expr,
                                   CEL_NULLABLE(cel_Expr*) iter_range) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->iter_range,
                            iter_range);
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_ReleaseIterRange(CEL_NONNULL(cel_ComprehensionExpr*)
                                           expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->iter_range);
}

cel_StringView cel_ComprehensionExpr_AccuVar(
    CEL_NONNULL(const cel_ComprehensionExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return expr->accu_var;
}

void cel_ComprehensionExpr_SetAccuVar(CEL_NONNULL(cel_ComprehensionExpr*) expr,
                                      cel_StringView accu_var) {
  CEL_ASSERT_NOT_NULL(expr);

  expr->accu_var = accu_var;
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_AccuInit(CEL_NONNULL(const cel_ComprehensionExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->accu_init);
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_SetAccuInit(CEL_NONNULL(cel_ComprehensionExpr*) expr,
                                  CEL_NULLABLE(cel_Expr*) accu_init) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->accu_init, accu_init);
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_ReleaseAccuInit(CEL_NONNULL(cel_ComprehensionExpr*)
                                          expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->accu_init);
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_LoopCondition(CEL_NONNULL(const cel_ComprehensionExpr*)
                                        expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->loop_condition);
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_SetLoopCondition(CEL_NONNULL(cel_ComprehensionExpr*) expr,
                                       CEL_NULLABLE(cel_Expr*) loop_condition) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->loop_condition,
                            loop_condition);
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_ReleaseLoopCondition(CEL_NONNULL(cel_ComprehensionExpr*)
                                               expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->loop_condition);
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_LoopStep(CEL_NONNULL(const cel_ComprehensionExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->loop_step);
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_SetLoopStep(CEL_NONNULL(cel_ComprehensionExpr*) expr,
                                  CEL_NULLABLE(cel_Expr*) loop_step) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->loop_step, loop_step);
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_ReleaseLoopStep(CEL_NONNULL(cel_ComprehensionExpr*)
                                          expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->loop_step);
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_Result(CEL_NONNULL(const cel_ComprehensionExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_Child(cel_Expr_UpCast(expr), expr->result);
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_SetResult(CEL_NONNULL(cel_ComprehensionExpr*) expr,
                                CEL_NULLABLE(cel_Expr*) result) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_SetChild(cel_Expr_UpCast(expr), &expr->result, result);
}

CEL_NULLABLE(cel_Expr*)
cel_ComprehensionExpr_ReleaseResult(CEL_NONNULL(cel_ComprehensionExpr*) expr) {
  CEL_ASSERT_NOT_NULL(expr);

  return _cel_Expr_ReleaseChild(cel_Expr_UpCast(expr), &expr->result);
}
