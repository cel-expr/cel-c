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

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "google/api/expr/v1alpha1/checked.upb.h"
#include "google/api/expr/v1alpha1/syntax.upb.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/ast.h"
#include "cel-c/config.h"
#include "cel-c/constant_proto_v1alpha1.h"
#include "cel-c/operators.h"
#include "cel-c/ref.h"
#include "cel-c/ref_proto_v1alpha1.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/type.h"
#include "cel-c/type_proto_v1alpha1.h"
#include "cel-c/src/malloc.h"
#include "cel-c/src/setjmp.h"

typedef struct {
  CEL_NONNULL(cel_Ast*) ast;
  CEL_NONNULL(cel_Arena*) arena;
  CEL_NONNULL(cel_Status*) status;
  CEL_NONNULL(const google_api_expr_v1alpha1_CheckedExpr*) checked_expr;
  CEL_NULLABLE(const google_api_expr_v1alpha1_SourceInfo*) source_info;
  _cel_jmp_buf jmp;
} _cel_AstFromProtoV1Alpha1State;

static int64_t _cel_AstFromProtoV1Alpha1_CheckId(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state, cel_ExprId id) {
  if (CEL_UNLIKELY(id < 0)) {
    cel_InvalidArgumentStatusF(state->status,
                               "cel: expected google.api.expr.v1alpha1.Expr.id "
                               "to be greater than or equal to 0: %" PRId64,
                               id);
    _cel_longjmp(state->jmp);
  }
  return id;
}

static cel_StringView _cel_AstFromProtoV1Alpha1_StrDup(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state, cel_StringView in) {
  cel_StringView out;
  if (CEL_UNLIKELY(!cel_Arena_StrDup(state->arena, &out, in))) {
    cel_OutOfMemoryStatus(state->status);
    _cel_longjmp(state->jmp);
  }
  return out;
}

static void _cel_AstFromProtoV1Alpha1_UpdatePosition(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state,
    CEL_NONNULL(cel_Expr*) expr) {
  if (state->source_info != cel_nullptr) {
    int64_t id = cel_Expr_Id(expr);
    if (id != 0) {
      int32_t position;
      if (google_api_expr_v1alpha1_SourceInfo_positions_get(state->source_info,
                                                            id, &position)) {
        if (CEL_UNLIKELY(position < -1)) {
          cel_InvalidArgumentStatusF(
              state->status,
              "cel: expected "
              "google.api.expr.v1alpha1.SourceInfo.positions[%" PRId64
              "] "
              "to be greater than or equal to -1: %" PRId32,
              id, position);
          _cel_longjmp(state->jmp);
        }
        cel_Expr_SetSourcePosition(expr, position);
      }
    }
  }
}

static void _cel_AstFromProtoV1Alpha1_SetType(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state,
    CEL_NONNULL(cel_Expr*) expr) {
  int64_t id = cel_Expr_Id(expr);
  if (id != 0) {
    const google_api_expr_v1alpha1_Type* in_type;
    if (google_api_expr_v1alpha1_CheckedExpr_type_map_get(
            state->checked_expr, id,
            (google_api_expr_v1alpha1_Type**)&in_type)) {
      const cel_Type* type =
          cel_Type_FromProtoV1Alpha1(in_type, state->arena, state->status);
      if (CEL_UNLIKELY(type == cel_nullptr)) {
        _cel_longjmp(state->jmp);
      }
      cel_Expr_SetType(expr, type);
    }
  }
}

static void _cel_AstFromProtoV1Alpha1_SetRef(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state,
    CEL_NONNULL(cel_Expr*) expr) {
  int64_t id = cel_Expr_Id(expr);
  if (id != 0) {
    const google_api_expr_v1alpha1_Reference* in_ref;
    if (google_api_expr_v1alpha1_CheckedExpr_reference_map_get(
            state->checked_expr, id,
            (google_api_expr_v1alpha1_Reference**)&in_ref)) {
      const cel_Ref* ref =
          cel_Ref_FromProtoV1Alpha1(in_ref, state->arena, state->status);
      if (CEL_UNLIKELY(ref == cel_nullptr)) {
        _cel_longjmp(state->jmp);
      }
      cel_Expr_SetRef(expr, ref);
    }
  }
}

static CEL_NONNULL(cel_Expr*) _cel_AstFromProtoV1Alpha1_Expr(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state,
    CEL_NONNULL(const google_api_expr_v1alpha1_Expr*) in);

static CEL_NONNULL(cel_Expr*) _cel_AstFromProtoV1Alpha1_UnspecifiedExpr(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state, int64_t id) {
  _cel_AstFromProtoV1Alpha1_CheckId(state, id);
  CEL_NULLABLE(cel_UnspecifiedExpr*) expr = cel_UnspecifiedExpr_New(state->ast);
  if (CEL_UNLIKELY(expr == cel_nullptr)) {
    cel_OutOfMemoryStatus(state->status);
    _cel_longjmp(state->jmp);
  }
  cel_Expr_SetId(cel_Expr_UpCast(expr), id);
  _cel_AstFromProtoV1Alpha1_UpdatePosition(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetType(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetRef(state, cel_Expr_UpCast(expr));
  return cel_Expr_UpCast(expr);
}

static CEL_NONNULL(cel_Expr*) _cel_AstFromProtoV1Alpha1_IdentExpr(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state, int64_t id,
    CEL_NONNULL(const google_api_expr_v1alpha1_Expr_Ident*) in) {
  _cel_AstFromProtoV1Alpha1_CheckId(state, id);
  CEL_NULLABLE(cel_IdentExpr*) expr = cel_IdentExpr_New(state->ast);
  if (CEL_UNLIKELY(expr == cel_nullptr)) {
    cel_OutOfMemoryStatus(state->status);
    _cel_longjmp(state->jmp);
  }
  cel_Expr_SetId(cel_Expr_UpCast(expr), id);
  _cel_AstFromProtoV1Alpha1_UpdatePosition(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetType(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetRef(state, cel_Expr_UpCast(expr));
  cel_IdentExpr_SetName(
      expr, _cel_AstFromProtoV1Alpha1_StrDup(
                state, google_api_expr_v1alpha1_Expr_Ident_name(in)));
  return cel_Expr_UpCast(expr);
}

static CEL_NONNULL(cel_Expr*) _cel_AstFromProtoV1Alpha1_ConstExpr(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state, int64_t id,
    CEL_NONNULL(const google_api_expr_v1alpha1_Constant*) in) {
  _cel_AstFromProtoV1Alpha1_CheckId(state, id);
  CEL_NULLABLE(cel_ConstExpr*) expr = cel_ConstExpr_New(state->ast);
  if (CEL_UNLIKELY(expr == cel_nullptr)) {
    cel_OutOfMemoryStatus(state->status);
    _cel_longjmp(state->jmp);
  }
  cel_Expr_SetId(cel_Expr_UpCast(expr), id);
  _cel_AstFromProtoV1Alpha1_UpdatePosition(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetType(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetRef(state, cel_Expr_UpCast(expr));
  if (CEL_UNLIKELY(!cel_Constant_FromProtoV1Alpha1(
          cel_ConstExpr_MutableValue(expr), in, state->arena, state->status))) {
    _cel_longjmp(state->jmp);
  }
  return cel_Expr_UpCast(expr);
}

static CEL_NONNULL(cel_Expr*) _cel_AstFromProtoV1Alpha1_SelectExpr(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state, int64_t id,
    CEL_NONNULL(const google_api_expr_v1alpha1_Expr_Select*) in) {
  _cel_AstFromProtoV1Alpha1_CheckId(state, id);
  CEL_NULLABLE(cel_SelectExpr*) expr = cel_SelectExpr_New(state->ast);
  if (CEL_UNLIKELY(expr == cel_nullptr)) {
    cel_OutOfMemoryStatus(state->status);
    _cel_longjmp(state->jmp);
  }
  cel_Expr_SetId(cel_Expr_UpCast(expr), id);
  _cel_AstFromProtoV1Alpha1_UpdatePosition(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetType(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetRef(state, cel_Expr_UpCast(expr));
  if (google_api_expr_v1alpha1_Expr_Select_has_operand(in)) {
    cel_SelectExpr_SetOperand(
        expr, _cel_AstFromProtoV1Alpha1_Expr(
                  state, google_api_expr_v1alpha1_Expr_Select_operand(in)));
  }
  cel_SelectExpr_SetField(
      expr, _cel_AstFromProtoV1Alpha1_StrDup(
                state, google_api_expr_v1alpha1_Expr_Select_field(in)));
  cel_SelectExpr_SetTestOnly(
      expr, google_api_expr_v1alpha1_Expr_Select_test_only(in));
  return cel_Expr_UpCast(expr);
}

static CEL_NONNULL(cel_Expr*) _cel_AstFromProtoV1Alpha1_UnaryExpr(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state, int64_t id,
    cel_UnaryOp op, CEL_NONNULL(const google_api_expr_v1alpha1_Expr*) arg) {
  _cel_AstFromProtoV1Alpha1_CheckId(state, id);
  CEL_NULLABLE(cel_UnaryExpr*) expr = cel_UnaryExpr_New(state->ast);
  if (CEL_UNLIKELY(expr == cel_nullptr)) {
    cel_OutOfMemoryStatus(state->status);
    _cel_longjmp(state->jmp);
  }
  cel_Expr_SetId(cel_Expr_UpCast(expr), id);
  _cel_AstFromProtoV1Alpha1_UpdatePosition(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetType(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetRef(state, cel_Expr_UpCast(expr));
  cel_UnaryExpr_SetOp(expr, op);
  cel_UnaryExpr_SetArg(expr, _cel_AstFromProtoV1Alpha1_Expr(state, arg));
  return cel_Expr_UpCast(expr);
}

static CEL_NONNULL(cel_Expr*) _cel_AstFromProtoV1Alpha1_BinaryExpr(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state, int64_t id,
    cel_BinaryOp op, CEL_NONNULL(const google_api_expr_v1alpha1_Expr*) left,
    CEL_NONNULL(const google_api_expr_v1alpha1_Expr*) right) {
  _cel_AstFromProtoV1Alpha1_CheckId(state, id);
  CEL_NULLABLE(cel_BinaryExpr*) expr = cel_BinaryExpr_New(state->ast);
  if (CEL_UNLIKELY(expr == cel_nullptr)) {
    cel_OutOfMemoryStatus(state->status);
    _cel_longjmp(state->jmp);
  }
  cel_Expr_SetId(cel_Expr_UpCast(expr), id);
  _cel_AstFromProtoV1Alpha1_UpdatePosition(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetType(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetRef(state, cel_Expr_UpCast(expr));
  cel_BinaryExpr_SetOp(expr, op);
  cel_BinaryExpr_SetLeft(expr, _cel_AstFromProtoV1Alpha1_Expr(state, left));
  cel_BinaryExpr_SetRight(expr, _cel_AstFromProtoV1Alpha1_Expr(state, right));
  return cel_Expr_UpCast(expr);
}

static CEL_NONNULL(cel_Expr*) _cel_AstFromProtoV1Alpha1_TernaryExpr(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state, int64_t id,
    cel_TernaryOp op,
    CEL_NONNULL(const google_api_expr_v1alpha1_Expr*) condition,
    CEL_NONNULL(const google_api_expr_v1alpha1_Expr*) if_true,
    CEL_NONNULL(const google_api_expr_v1alpha1_Expr*) if_false) {
  _cel_AstFromProtoV1Alpha1_CheckId(state, id);
  CEL_NULLABLE(cel_TernaryExpr*) expr = cel_TernaryExpr_New(state->ast);
  if (CEL_UNLIKELY(expr == cel_nullptr)) {
    cel_OutOfMemoryStatus(state->status);
    _cel_longjmp(state->jmp);
  }
  cel_Expr_SetId(cel_Expr_UpCast(expr), id);
  _cel_AstFromProtoV1Alpha1_UpdatePosition(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetType(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetRef(state, cel_Expr_UpCast(expr));
  cel_TernaryExpr_SetOp(expr, op);
  cel_TernaryExpr_SetCondition(
      expr, _cel_AstFromProtoV1Alpha1_Expr(state, condition));
  cel_TernaryExpr_SetIfTrue(expr,
                            _cel_AstFromProtoV1Alpha1_Expr(state, if_true));
  cel_TernaryExpr_SetIfFalse(expr,
                             _cel_AstFromProtoV1Alpha1_Expr(state, if_false));
  return cel_Expr_UpCast(expr);
}

static CEL_NONNULL(cel_Expr*) _cel_AstFromProtoV1Alpha1_CallExpr(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state, int64_t id,
    CEL_NONNULL(const google_api_expr_v1alpha1_Expr_Call*) in) {
  size_t in_args_len = 0;
  const google_api_expr_v1alpha1_Expr* const* in_args =
      google_api_expr_v1alpha1_Expr_Call_args(in, &in_args_len);
  cel_StringView function = google_api_expr_v1alpha1_Expr_Call_function(in);
  if (!cel_StringView_Empty(function) &&
      !google_api_expr_v1alpha1_Expr_Call_has_target(in)) {
    switch (in_args_len) {
      case 1: {
        cel_UnaryOp op;
        if (cel_UnaryOp_FromString(function, &op)) {
          return _cel_AstFromProtoV1Alpha1_UnaryExpr(state, id, op, in_args[0]);
        }
      } break;
      case 2: {
        cel_BinaryOp op;
        if (cel_BinaryOp_FromString(function, &op)) {
          return _cel_AstFromProtoV1Alpha1_BinaryExpr(state, id, op, in_args[0],
                                                      in_args[1]);
        }
      } break;
      case 3: {
        cel_TernaryOp op;
        if (cel_TernaryOp_FromString(function, &op)) {
          return _cel_AstFromProtoV1Alpha1_TernaryExpr(
              state, id, op, in_args[0], in_args[1], in_args[2]);
        }
      } break;
      default:
        break;
    }
  }
  _cel_AstFromProtoV1Alpha1_CheckId(state, id);
  CEL_NULLABLE(cel_CallExpr*) expr = cel_CallExpr_New(state->ast);
  if (CEL_UNLIKELY(expr == cel_nullptr)) {
    cel_OutOfMemoryStatus(state->status);
    _cel_longjmp(state->jmp);
  }
  cel_Expr_SetId(cel_Expr_UpCast(expr), id);
  _cel_AstFromProtoV1Alpha1_UpdatePosition(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetType(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetRef(state, cel_Expr_UpCast(expr));
  if (google_api_expr_v1alpha1_Expr_Call_has_target(in)) {
    cel_CallExpr_SetTarget(
        expr, _cel_AstFromProtoV1Alpha1_Expr(
                  state, google_api_expr_v1alpha1_Expr_Call_target(in)));
  }
  cel_CallExpr_SetFunction(expr,
                           _cel_AstFromProtoV1Alpha1_StrDup(state, function));
  for (size_t i = 0; i < in_args_len; ++i) {
    CEL_NULLABLE(cel_CallArgExpr*)
    arg = cel_CallArgExpr_New(state->ast);
    if (CEL_UNLIKELY(arg == cel_nullptr)) {
      cel_OutOfMemoryStatus(state->status);
      _cel_longjmp(state->jmp);
    }
    CEL_NONNULL(cel_Expr*)
    arg_value = _cel_AstFromProtoV1Alpha1_Expr(state, in_args[i]);
    cel_Expr_SetSourceRange(cel_Expr_UpCast(arg),
                            cel_Expr_SourceRange(arg_value));
    cel_CallArgExpr_SetValue(arg, arg_value);
    cel_CallExpr_AppendArg(expr, arg);
  }
  return cel_Expr_UpCast(expr);
}

static CEL_NONNULL(cel_Expr*) _cel_AstFromProtoV1Alpha1_ListExpr(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state, int64_t id,
    CEL_NONNULL(const google_api_expr_v1alpha1_Expr_CreateList*) in) {
  _cel_AstFromProtoV1Alpha1_CheckId(state, id);
  CEL_NULLABLE(cel_ListExpr*) expr = cel_ListExpr_New(state->ast);
  if (CEL_UNLIKELY(expr == cel_nullptr)) {
    cel_OutOfMemoryStatus(state->status);
    _cel_longjmp(state->jmp);
  }
  cel_Expr_SetId(cel_Expr_UpCast(expr), id);
  _cel_AstFromProtoV1Alpha1_UpdatePosition(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetType(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetRef(state, cel_Expr_UpCast(expr));
  size_t in_opt_indices_len = 0;
  const int32_t* in_opt_indices =
      google_api_expr_v1alpha1_Expr_CreateList_optional_indices(
          in, &in_opt_indices_len);
  size_t in_elems_len = 0;
  const google_api_expr_v1alpha1_Expr* const* in_elems =
      google_api_expr_v1alpha1_Expr_CreateList_elements(in, &in_elems_len);
  for (size_t i = 0; i < in_elems_len; ++i) {
    CEL_NULLABLE(cel_ListElementExpr*)
    elem = cel_ListElementExpr_New(state->ast);
    if (CEL_UNLIKELY(elem == cel_nullptr)) {
      cel_OutOfMemoryStatus(state->status);
      _cel_longjmp(state->jmp);
    }
    CEL_NONNULL(cel_Expr*)
    elem_value = _cel_AstFromProtoV1Alpha1_Expr(state, in_elems[i]);
    cel_Expr_SetSourceRange(cel_Expr_UpCast(elem),
                            cel_Expr_SourceRange(elem_value));
    cel_ListElementExpr_SetValue(elem, elem_value);
    bool optional = false;
    for (size_t j = 0; j < in_opt_indices_len; ++j) {
      int32_t in_opt_index = in_opt_indices[j];
      if (in_opt_index < 0) {
        // Just skip for now. In future we should error.
        continue;
      }
      if (((uint32_t)in_opt_index) == i) {
        optional = true;
        break;
      }
    }
    cel_ListElementExpr_SetOptional(elem, optional);
    cel_ListExpr_AppendElement(expr, elem);
  }
  return cel_Expr_UpCast(expr);
}

static CEL_NONNULL(cel_Expr*) _cel_AstFromProtoV1Alpha1_MapExpr(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state, int64_t id,
    CEL_NONNULL(const google_api_expr_v1alpha1_Expr_CreateStruct*) in) {
  _cel_AstFromProtoV1Alpha1_CheckId(state, id);
  CEL_NULLABLE(cel_MapExpr*) expr = cel_MapExpr_New(state->ast);
  if (CEL_UNLIKELY(expr == cel_nullptr)) {
    cel_OutOfMemoryStatus(state->status);
    _cel_longjmp(state->jmp);
  }
  cel_Expr_SetId(cel_Expr_UpCast(expr), id);
  _cel_AstFromProtoV1Alpha1_UpdatePosition(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetType(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetRef(state, cel_Expr_UpCast(expr));
  size_t in_ents_len = 0;
  const google_api_expr_v1alpha1_Expr_CreateStruct_Entry* const* in_ents =
      google_api_expr_v1alpha1_Expr_CreateStruct_entries(in, &in_ents_len);
  for (size_t i = 0; i < in_ents_len; ++i) {
    const google_api_expr_v1alpha1_Expr_CreateStruct_Entry* in_ent = in_ents[i];
    const google_api_expr_v1alpha1_Expr_CreateStruct_Entry_key_kind_oneofcases
        key_kind =
            google_api_expr_v1alpha1_Expr_CreateStruct_Entry_key_kind_case(
                in_ent);
    if (key_kind !=
        google_api_expr_v1alpha1_Expr_CreateStruct_Entry_key_kind_map_key) {
      cel_InvalidArgumentStatusF(
          state->status,
          "cel: expected "
          "google.api.expr.v1alpha1.Expr.CreateStruct.Entry.key_kind to "
          "be map_key: %d",
          key_kind);
      _cel_longjmp(state->jmp);
    }
    CEL_NONNULL(cel_MapEntryExpr*) ent = cel_MapEntryExpr_New(state->ast);
    if (CEL_UNLIKELY(ent == cel_nullptr)) {
      cel_OutOfMemoryStatus(state->status);
      _cel_longjmp(state->jmp);
    }
    cel_Expr_SetId(
        cel_Expr_UpCast(ent),
        _cel_AstFromProtoV1Alpha1_CheckId(
            state,
            google_api_expr_v1alpha1_Expr_CreateStruct_Entry_id(in_ent)));
    _cel_AstFromProtoV1Alpha1_UpdatePosition(state, cel_Expr_UpCast(ent));
    _cel_AstFromProtoV1Alpha1_SetType(state, cel_Expr_UpCast(ent));
    _cel_AstFromProtoV1Alpha1_SetRef(state, cel_Expr_UpCast(ent));
    cel_MapEntryExpr_SetKey(
        ent,
        _cel_AstFromProtoV1Alpha1_Expr(
            state,
            google_api_expr_v1alpha1_Expr_CreateStruct_Entry_map_key(in_ent)));
    if (google_api_expr_v1alpha1_Expr_CreateStruct_Entry_has_value(in_ent)) {
      cel_MapEntryExpr_SetValue(
          ent,
          _cel_AstFromProtoV1Alpha1_Expr(
              state,
              google_api_expr_v1alpha1_Expr_CreateStruct_Entry_value(in_ent)));
    }
    cel_MapEntryExpr_SetOptional(
        ent, google_api_expr_v1alpha1_Expr_CreateStruct_Entry_optional_entry(
                 in_ent));
    cel_MapExpr_AppendEntry(expr, ent);
  }
  return cel_Expr_UpCast(expr);
}

static CEL_NONNULL(cel_Expr*) _cel_AstFromProtoV1Alpha1_StructExpr(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state, int64_t id,
    CEL_NONNULL(const google_api_expr_v1alpha1_Expr_CreateStruct*) in) {
  _cel_AstFromProtoV1Alpha1_CheckId(state, id);
  CEL_NULLABLE(cel_StructExpr*) expr = cel_StructExpr_New(state->ast);
  if (CEL_UNLIKELY(expr == cel_nullptr)) {
    cel_OutOfMemoryStatus(state->status);
    _cel_longjmp(state->jmp);
  }
  cel_Expr_SetId(cel_Expr_UpCast(expr), id);
  _cel_AstFromProtoV1Alpha1_UpdatePosition(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetType(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetRef(state, cel_Expr_UpCast(expr));
  cel_StructExpr_SetName(
      expr,
      _cel_AstFromProtoV1Alpha1_StrDup(
          state, google_api_expr_v1alpha1_Expr_CreateStruct_message_name(in)));
  size_t in_flds_len = 0;
  const google_api_expr_v1alpha1_Expr_CreateStruct_Entry* const* in_flds =
      google_api_expr_v1alpha1_Expr_CreateStruct_entries(in, &in_flds_len);
  for (size_t i = 0; i < in_flds_len; ++i) {
    const google_api_expr_v1alpha1_Expr_CreateStruct_Entry* in_fld = in_flds[i];
    const google_api_expr_v1alpha1_Expr_CreateStruct_Entry_key_kind_oneofcases
        key_kind =
            google_api_expr_v1alpha1_Expr_CreateStruct_Entry_key_kind_case(
                in_fld);
    if (key_kind !=
        google_api_expr_v1alpha1_Expr_CreateStruct_Entry_key_kind_field_key) {
      cel_InvalidArgumentStatusF(
          state->status,
          "cel: expected "
          "google.api.expr.v1alpha1.Expr.CreateStruct.Entry.key_kind to "
          "be field_key: %d",
          key_kind);
      _cel_longjmp(state->jmp);
    }
    CEL_NONNULL(cel_StructFieldExpr*) fld = cel_StructFieldExpr_New(state->ast);
    if (CEL_UNLIKELY(fld == cel_nullptr)) {
      cel_OutOfMemoryStatus(state->status);
      _cel_longjmp(state->jmp);
    }
    cel_Expr_SetId(
        cel_Expr_UpCast(fld),
        _cel_AstFromProtoV1Alpha1_CheckId(
            state,
            google_api_expr_v1alpha1_Expr_CreateStruct_Entry_id(in_fld)));
    _cel_AstFromProtoV1Alpha1_UpdatePosition(state, cel_Expr_UpCast(fld));
    _cel_AstFromProtoV1Alpha1_SetType(state, cel_Expr_UpCast(fld));
    _cel_AstFromProtoV1Alpha1_SetRef(state, cel_Expr_UpCast(fld));
    cel_StructFieldExpr_SetName(
        fld,
        _cel_AstFromProtoV1Alpha1_StrDup(
            state, google_api_expr_v1alpha1_Expr_CreateStruct_Entry_field_key(
                       in_fld)));
    if (google_api_expr_v1alpha1_Expr_CreateStruct_Entry_has_value(in_fld)) {
      cel_StructFieldExpr_SetValue(
          fld,
          _cel_AstFromProtoV1Alpha1_Expr(
              state,
              google_api_expr_v1alpha1_Expr_CreateStruct_Entry_value(in_fld)));
    }
    cel_StructFieldExpr_SetOptional(
        fld, google_api_expr_v1alpha1_Expr_CreateStruct_Entry_optional_entry(
                 in_fld));
    cel_StructExpr_AppendField(expr, fld);
  }
  return cel_Expr_UpCast(expr);
}

static CEL_NONNULL(cel_Expr*) _cel_AstFromProtoV1Alpha1_ComprehensionExpr(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state, int64_t id,
    CEL_NONNULL(const google_api_expr_v1alpha1_Expr_Comprehension*) in) {
  _cel_AstFromProtoV1Alpha1_CheckId(state, id);
  CEL_NULLABLE(cel_ComprehensionExpr*)
  expr = cel_ComprehensionExpr_New(state->ast);
  if (CEL_UNLIKELY(expr == cel_nullptr)) {
    cel_OutOfMemoryStatus(state->status);
    _cel_longjmp(state->jmp);
  }
  cel_Expr_SetId(cel_Expr_UpCast(expr), id);
  _cel_AstFromProtoV1Alpha1_UpdatePosition(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetType(state, cel_Expr_UpCast(expr));
  _cel_AstFromProtoV1Alpha1_SetRef(state, cel_Expr_UpCast(expr));
  cel_ComprehensionExpr_SetIterVar(
      expr,
      _cel_AstFromProtoV1Alpha1_StrDup(
          state, google_api_expr_v1alpha1_Expr_Comprehension_iter_var(in)));
  cel_ComprehensionExpr_SetIterVar2(
      expr,
      _cel_AstFromProtoV1Alpha1_StrDup(
          state, google_api_expr_v1alpha1_Expr_Comprehension_iter_var2(in)));
  cel_ComprehensionExpr_SetAccuVar(
      expr,
      _cel_AstFromProtoV1Alpha1_StrDup(
          state, google_api_expr_v1alpha1_Expr_Comprehension_accu_var(in)));
  if (google_api_expr_v1alpha1_Expr_Comprehension_has_iter_range(in)) {
    cel_ComprehensionExpr_SetIterRange(
        expr,
        _cel_AstFromProtoV1Alpha1_Expr(
            state, google_api_expr_v1alpha1_Expr_Comprehension_iter_range(in)));
  }
  if (google_api_expr_v1alpha1_Expr_Comprehension_has_accu_init(in)) {
    cel_ComprehensionExpr_SetAccuInit(
        expr,
        _cel_AstFromProtoV1Alpha1_Expr(
            state, google_api_expr_v1alpha1_Expr_Comprehension_accu_init(in)));
  }
  if (google_api_expr_v1alpha1_Expr_Comprehension_has_loop_condition(in)) {
    cel_ComprehensionExpr_SetLoopCondition(
        expr,
        _cel_AstFromProtoV1Alpha1_Expr(
            state,
            google_api_expr_v1alpha1_Expr_Comprehension_loop_condition(in)));
  }
  if (google_api_expr_v1alpha1_Expr_Comprehension_has_loop_step(in)) {
    cel_ComprehensionExpr_SetLoopStep(
        expr,
        _cel_AstFromProtoV1Alpha1_Expr(
            state, google_api_expr_v1alpha1_Expr_Comprehension_loop_step(in)));
  }
  if (google_api_expr_v1alpha1_Expr_Comprehension_has_result(in)) {
    cel_ComprehensionExpr_SetResult(
        expr,
        _cel_AstFromProtoV1Alpha1_Expr(
            state, google_api_expr_v1alpha1_Expr_Comprehension_result(in)));
  }
  return cel_Expr_UpCast(expr);
}

static CEL_NONNULL(cel_Expr*) _cel_AstFromProtoV1Alpha1_Expr(
    CEL_NONNULL(_cel_AstFromProtoV1Alpha1State*) state,
    CEL_NONNULL(const google_api_expr_v1alpha1_Expr*) in) {
  google_api_expr_v1alpha1_Expr_expr_kind_oneofcases kind =
      google_api_expr_v1alpha1_Expr_expr_kind_case(in);
  switch (kind) {
    case google_api_expr_v1alpha1_Expr_expr_kind_NOT_SET:
      return _cel_AstFromProtoV1Alpha1_UnspecifiedExpr(
          state, google_api_expr_v1alpha1_Expr_id(in));
    case google_api_expr_v1alpha1_Expr_expr_kind_ident_expr:
      return _cel_AstFromProtoV1Alpha1_IdentExpr(
          state, google_api_expr_v1alpha1_Expr_id(in),
          google_api_expr_v1alpha1_Expr_ident_expr(in));
    case google_api_expr_v1alpha1_Expr_expr_kind_const_expr:
      return _cel_AstFromProtoV1Alpha1_ConstExpr(
          state, google_api_expr_v1alpha1_Expr_id(in),
          google_api_expr_v1alpha1_Expr_const_expr(in));
    case google_api_expr_v1alpha1_Expr_expr_kind_select_expr:
      return _cel_AstFromProtoV1Alpha1_SelectExpr(
          state, google_api_expr_v1alpha1_Expr_id(in),
          google_api_expr_v1alpha1_Expr_select_expr(in));
    case google_api_expr_v1alpha1_Expr_expr_kind_call_expr:
      return _cel_AstFromProtoV1Alpha1_CallExpr(
          state, google_api_expr_v1alpha1_Expr_id(in),
          google_api_expr_v1alpha1_Expr_call_expr(in));
    case google_api_expr_v1alpha1_Expr_expr_kind_list_expr:
      return _cel_AstFromProtoV1Alpha1_ListExpr(
          state, google_api_expr_v1alpha1_Expr_id(in),
          google_api_expr_v1alpha1_Expr_list_expr(in));
    case google_api_expr_v1alpha1_Expr_expr_kind_struct_expr: {
      CEL_NONNULL(const google_api_expr_v1alpha1_Expr_CreateStruct*)
      struct_expr = google_api_expr_v1alpha1_Expr_struct_expr(in);
      if (cel_StringView_Empty(
              google_api_expr_v1alpha1_Expr_CreateStruct_message_name(
                  struct_expr))) {
        return _cel_AstFromProtoV1Alpha1_MapExpr(
            state, google_api_expr_v1alpha1_Expr_id(in), struct_expr);
      }
      return _cel_AstFromProtoV1Alpha1_StructExpr(
          state, google_api_expr_v1alpha1_Expr_id(in), struct_expr);
    }
    case google_api_expr_v1alpha1_Expr_expr_kind_comprehension_expr:
      return _cel_AstFromProtoV1Alpha1_ComprehensionExpr(
          state, google_api_expr_v1alpha1_Expr_id(in),
          google_api_expr_v1alpha1_Expr_comprehension_expr(in));
    default:
      cel_InvalidArgumentStatusF(
          state->status,
          "cel: unexpected google.api.expr.v1alpha1.Expr kind: %d", kind);
      _cel_longjmp(state->jmp);
  }
}

CEL_NULLABLE(cel_Ast*)
cel_Ast_FromProtoV1Alpha1(
    CEL_NONNULL(const google_api_expr_v1alpha1_CheckedExpr*) in,
    CEL_NONNULL(cel_Arena*) arena, CEL_NONNULL(cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(in);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return cel_nullptr;
  }
  CEL_NULLABLE(_cel_AstFromProtoV1Alpha1State*)
  volatile state = (CEL_NULLABLE(_cel_AstFromProtoV1Alpha1State*))_cel_Malloc(
      sizeof(_cel_AstFromProtoV1Alpha1State), cel_nullptr);
  if (CEL_UNLIKELY(state == cel_nullptr)) {
    cel_OutOfMemoryStatus(status);
    return cel_nullptr;
  }
  CEL_NULLABLE(cel_Ast*) ast = cel_Ast_New(arena);
  if (CEL_UNLIKELY(ast == cel_nullptr)) {
    _cel_FreeSized(state, sizeof(_cel_AstFromProtoV1Alpha1State));
    cel_OutOfMemoryStatus(status);
    return cel_nullptr;
  }
  state->ast = ast;
  state->arena = arena;
  state->status = status;
  state->checked_expr = in;
  state->source_info = cel_nullptr;

  if (_cel_setjmp(state->jmp)) {
    // ERROR
    CEL_ASSERT(!cel_Status_Ok(state->status));
    cel_Ast_Delete(state->ast);
    _cel_FreeSized(state, sizeof(_cel_AstFromProtoV1Alpha1State));
    return cel_nullptr;
  } else {
    if (google_api_expr_v1alpha1_CheckedExpr_has_expr(in)) {
      if (google_api_expr_v1alpha1_CheckedExpr_has_source_info(in)) {
        state->source_info =
            google_api_expr_v1alpha1_CheckedExpr_source_info(in);
      }
      cel_Ast_SetExpr(
          state->ast,
          _cel_AstFromProtoV1Alpha1_Expr(
              state, google_api_expr_v1alpha1_CheckedExpr_expr(in)));
    }
    // OK
    CEL_ASSERT(cel_Status_Ok(state->status));
    _cel_FreeSized(state, sizeof(_cel_AstFromProtoV1Alpha1State));
    return ast;
  }
}
