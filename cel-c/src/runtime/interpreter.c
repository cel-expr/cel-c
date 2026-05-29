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

#include "cel-c/src/runtime/interpreter.h"

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/assert.h"
#include "cel-c/ast.h"
#include "cel-c/ast_traverse.h"
#include "cel-c/ast_visitor.h"
#include "cel-c/config.h"
#include "cel-c/constant.h"
#include "cel-c/hash.h"
#include "cel-c/operators.h"
#include "cel-c/ref.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/type.h"
#include "cel-c/well_known_types.h"
#include "cel-c/src/array.h"
#include "cel-c/src/container.h"
#include "cel-c/src/deque.h"
#include "cel-c/src/flat_hash_map.h"
#include "cel-c/src/runtime/instr.h"
#include "cel-c/src/runtime/program.h"
#include "cel-c/src/runtime/runtime.h"
#include "cel-c/src/setjmp.h"
#include "cel-c/src/string.h"
#include "upb/reflection/def.h"

typedef struct {
  const cel_IdentExpr* cel_nullability_unknown expr;
  // Starting PC.
  uint32_t start_pc;
} _cel_InterpreterNspaceVarIdent;

typedef struct {
  const cel_SelectExpr* cel_nonnull expr;
  // Starting PC.
  uint32_t ident_pc;
  uint32_t select_pc;
} _cel_InterpreterNspaceVarSelect;

typedef struct {
  _cel_InterpreterNspaceVarIdent ident;
  _cel_Array(_cel_InterpreterNspaceVarSelect) selects;
  _cel_String name;
} _cel_InterpreterNspaceVar;

static void _cel_InterpreterNspaceVar_Construct(
    _cel_InterpreterNspaceVar* cel_nonnull nspace_var) {
  memset(nspace_var, 0, sizeof(*nspace_var));
  _cel_Array_Construct(&nspace_var->selects);
  _cel_String_Construct(&nspace_var->name);
}

static void _cel_InterpreterNspaceVar_Destruct(
    _cel_InterpreterNspaceVar* cel_nonnull nspace_var,
    cel_Allocator* cel_nonnull alloc) {
  _cel_Array_Destruct(&nspace_var->selects, alloc);
  _cel_String_Destruct(&nspace_var->name, alloc);
}

typedef struct {
  const cel_BinaryExpr* cel_nonnull node;
  uint32_t pc;
  bool cond;
} _cel_InterpreterBinaryCond;

typedef struct {
  const cel_TernaryExpr* cel_nonnull node;
  uint32_t trilean_jump_pc;
  uint32_t jump_pc;
} _cel_InterpreterTernaryCond;

typedef struct {
  // cel_BinaryExpr or cel_TernaryExpr
  union {
    struct {
      const cel_Expr* cel_nonnull node;
    };
    _cel_InterpreterBinaryCond binary;
    _cel_InterpreterTernaryCond ternary;
  };
} _cel_InterpreterCond;

typedef struct {
  // One for each element of the list. Each is an _cel_ErrorJumpInstr.
  _cel_Array(uint32_t) pcs;
} _cel_InterpreterList;

typedef struct {
  // One for each key and value of the map. The key is an _cel_Foo and the value
  // is an _cel_ErrorJumpInstr.
  _cel_Array(uint32_t) pcs;
} _cel_InterpreterMap;

typedef struct {
  uint32_t current;
  uint32_t max;
} _cel_InterpreterValueStackBounds;

static CEL_INLINE void _cel_InterpreterValueStackBounds_Construct(
    _cel_InterpreterValueStackBounds* cel_nonnull bounds) {
  bounds->current = 0;
  bounds->max = 0;
}

static CEL_INLINE void _cel_InterpreterValueStackBounds_Add(
    _cel_InterpreterValueStackBounds* cel_nonnull bounds, uint32_t n) {
  CEL_ASSERT_LE(n, UINT32_MAX - bounds->current);

  bounds->current += n;

  if (bounds->current > bounds->max) {
    bounds->max = bounds->current;
  }
}

static CEL_INLINE void _cel_InterpreterValueStackBounds_Subtract(
    _cel_InterpreterValueStackBounds* cel_nonnull bounds, uint32_t n) {
  CEL_ASSERT_LE(n, bounds->current);

  bounds->current -= n;
}

static CEL_INLINE void _cel_InterpreterValueStackBounds_Increment(
    _cel_InterpreterValueStackBounds* cel_nonnull bounds) {
  _cel_InterpreterValueStackBounds_Add(bounds, 1);
}

static CEL_INLINE void _cel_InterpreterValueStackBounds_Decrement(
    _cel_InterpreterValueStackBounds* cel_nonnull bounds) {
  _cel_InterpreterValueStackBounds_Subtract(bounds, 1);
}

typedef struct {
  // Name of the bound variable.
  cel_StringView name;
  // When variable names shadow each other, we need to know the prev slot index
  // so we can restore it in _cel_Interpreter::slot_map.
  int32_t prev_slot;
  // The slot index, also the index into _cel_Interpreter::slots.
  uint32_t index;
  // PC for the jump we insert before assembling the subexpression. It is filled
  // in to jump over the subexpression.
  uint32_t guard_pc;
  // PC for the first instruction in the subexpression.
  uint32_t pc;
  // Current value stack size for the initExpr of this comprehension.
  _cel_InterpreterValueStackBounds value_stack_bounds;
} _cel_InterpreterSlot;

typedef struct {
  cel_AstVisitor visitor;

  cel_AstTraverser* cel_nonnull traverser;
  const cel_Runtime* cel_nonnull rt;
  cel_Allocator* cel_nonnull alloc;
  const upb_DefPool* cel_nonnull def_pool;
  const cel_WellKnownTypes* cel_nonnull wkts;
  cel_Status* cel_nonnull status;

  cel_Program* cel_nullable prog;

  // Mapping between strings, bytes, idents and their index in
  // `prog->string_pool`.
  _cel_FlatHashMap(cel_StringView, uint32_t) string_pool_indices;

  _cel_FlatHashMap(cel_StringView, uint32_t) candidate_names_indices;

  _cel_InterpreterNspaceVar nspace_var;

  _cel_Array(_cel_InterpreterCond) conds;

  _cel_Array(_cel_InterpreterList) lists;

  _cel_Array(_cel_InterpreterMap) maps;

  _cel_Deque(_cel_InterpreterSlot) slots;
  _cel_FlatHashMap(cel_StringView, uint32_t) slot_map;
  _cel_Array(uint32_t) slot_stack;

  const cel_ComprehensionExpr* cel_nullable root_bind;

  _cel_InterpreterValueStackBounds value_stack_bounds;
  _cel_Array(_cel_InterpreterValueStackBounds* cel_nonnull)
      value_stack_bounds_stack;

  _cel_jmp_buf jmp;
} _cel_Interpreter;

CEL_ATTRIBUTE_NODISCARD
static uint32_t _cel_Interpreter_PC(const _cel_Interpreter* cel_nonnull interp,
                                    const _cel_Instr* cel_nonnull instr) {
  return (uint32_t)(instr - _cel_Array_Data(&interp->prog->instrs));
}

CEL_ATTRIBUTE_NODISCARD
static uint32_t _cel_Interpreter_NextPC(
    const _cel_Interpreter* cel_nonnull interp) {
  return (uint32_t)_cel_Array_Size(&interp->prog->instrs);
}

CEL_ATTRIBUTE_NODISCARD
static _cel_Instr* _cel_Interpreter_InstrAt(
    const _cel_Interpreter* cel_nonnull interp, uint32_t pc) {
  return _cel_Array_MutableData(&interp->prog->instrs) + pc;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_Interpreter* cel_nonnull
_cel_Interpreter_FromAstVisitor(cel_AstVisitor* cel_nonnull visitor) {
  return cel_containerof(visitor, _cel_Interpreter, visitor);
}

CEL_ATTRIBUTE_NODISCARD
static _cel_Instr* cel_nonnull _cel_Interpreter_AppendInstrN(
    _cel_Interpreter* cel_nonnull interp, size_t count) {
  _cel_Instr* instr =
      _cel_Array_Append(&interp->prog->instrs, interp->alloc, count);
  if (CEL_UNLIKELY(instr == cel_nullptr)) {
    cel_OutOfMemoryStatus(interp->status);
    _cel_longjmp(interp->jmp);
  }
  return instr;
}

CEL_ATTRIBUTE_NODISCARD
static _cel_Instr* cel_nonnull
_cel_Interpreter_AppendInstr(_cel_Interpreter* cel_nonnull interp) {
  return _cel_Interpreter_AppendInstrN(interp, 1);
}

CEL_ATTRIBUTE_NODISCARD
static _cel_InterpreterCond* cel_nonnull
_cel_Interpreter_PushCond(_cel_Interpreter* cel_nonnull interp) {
  _cel_InterpreterCond* cond = _cel_Array_Push(&interp->conds, interp->alloc);
  if (CEL_UNLIKELY(cond == cel_nullptr)) {
    cel_OutOfMemoryStatus(interp->status);
    _cel_longjmp(interp->jmp);
  }
  return cond;
}

CEL_ATTRIBUTE_NODISCARD
static _cel_InterpreterCond* cel_nonnull
_cel_Interpreter_TopCond(_cel_Interpreter* cel_nonnull interp) {
  if (_cel_Array_Empty(&interp->conds)) {
    cel_InternalStatus(interp->status,
                       cel_StringView_From("cel: cond stack empty"));
    _cel_longjmp(interp->jmp);
  }
  return _cel_Array_MutableBack(&interp->conds);
}

CEL_ATTRIBUTE_NODISCARD
static _cel_InterpreterCond* cel_nullable
_cel_Interpreter_PeekCond(_cel_Interpreter* cel_nonnull interp) {
  if (_cel_Array_Empty(&interp->conds)) {
    return cel_nullptr;
  }
  return _cel_Array_MutableBack(&interp->conds);
}

static void _cel_Interpreter_PopCond(_cel_Interpreter* cel_nonnull interp) {
  if (_cel_Array_Empty(&interp->conds)) {
    cel_InternalStatus(interp->status,
                       cel_StringView_From("cel: cond stack empty"));
    _cel_longjmp(interp->jmp);
  }
  _cel_Array_Pop(&interp->conds);
}

CEL_ATTRIBUTE_NODISCARD
static _cel_InterpreterList* cel_nonnull
_cel_Interpreter_PushList(_cel_Interpreter* cel_nonnull interp) {
  _cel_InterpreterList* list = _cel_Array_Push(&interp->lists, interp->alloc);
  if (CEL_UNLIKELY(list == cel_nullptr)) {
    cel_OutOfMemoryStatus(interp->status);
    _cel_longjmp(interp->jmp);
  }
  return list;
}

CEL_ATTRIBUTE_NODISCARD
static _cel_InterpreterList* cel_nonnull
_cel_Interpreter_TopList(_cel_Interpreter* cel_nonnull interp) {
  if (_cel_Array_Empty(&interp->lists)) {
    cel_InternalStatus(interp->status,
                       cel_StringView_From("cel: list stack empty"));
    _cel_longjmp(interp->jmp);
  }
  return _cel_Array_MutableBack(&interp->lists);
}

static void _cel_Interpreter_PopList(_cel_Interpreter* cel_nonnull interp) {
  if (_cel_Array_Empty(&interp->lists)) {
    cel_InternalStatus(interp->status,
                       cel_StringView_From("cel: list stack empty"));
    _cel_longjmp(interp->jmp);
  }
  _cel_Array_Pop(&interp->lists);
}

CEL_ATTRIBUTE_NODISCARD
static _cel_InterpreterMap* cel_nonnull
_cel_Interpreter_PushMap(_cel_Interpreter* cel_nonnull interp) {
  _cel_InterpreterMap* list = _cel_Array_Push(&interp->maps, interp->alloc);
  if (CEL_UNLIKELY(list == cel_nullptr)) {
    cel_OutOfMemoryStatus(interp->status);
    _cel_longjmp(interp->jmp);
  }
  return list;
}

CEL_ATTRIBUTE_NODISCARD
static _cel_InterpreterMap* cel_nonnull
_cel_Interpreter_TopMap(_cel_Interpreter* cel_nonnull interp) {
  if (_cel_Array_Empty(&interp->maps)) {
    cel_InternalStatus(interp->status,
                       cel_StringView_From("cel: map stack empty"));
    _cel_longjmp(interp->jmp);
  }
  return _cel_Array_MutableBack(&interp->maps);
}

static void _cel_Interpreter_PopMap(_cel_Interpreter* cel_nonnull interp) {
  if (_cel_Array_Empty(&interp->maps)) {
    cel_InternalStatus(interp->status,
                       cel_StringView_From("cel: map stack empty"));
    _cel_longjmp(interp->jmp);
  }
  _cel_Array_Pop(&interp->maps);
}

CEL_ATTRIBUTE_NODISCARD
static _cel_InterpreterSlot* cel_nonnull _cel_Interpreter_NewSlot(
    _cel_Interpreter* cel_nonnull interp, cel_StringView name) {
  uint32_t slot_index = (uint32_t)_cel_Deque_Size(&interp->slots);
  _cel_InterpreterSlot* slot =
      _cel_Deque_PushBack(&interp->slots, interp->alloc);
  if (CEL_UNLIKELY(slot == cel_nullptr)) {
    cel_OutOfMemoryStatus(interp->status);
    _cel_longjmp(interp->jmp);
  }
  slot->index = slot_index;
  if (slot_index + 1 > interp->prog->max_slot_size) {
    interp->prog->max_slot_size = slot_index + 1;
  }
  slot->name = name;
  slot->pc = 0;
  slot->guard_pc = 0;
  slot->prev_slot = -1;
  _cel_InterpreterValueStackBounds_Construct(&slot->value_stack_bounds);
  return slot;
}

static void _cel_Interpreter_PushSlot(_cel_Interpreter* cel_nonnull interp,
                                      _cel_InterpreterSlot* cel_nonnull slot) {
  uint32_t* index = _cel_Array_Push(&interp->slot_stack, interp->alloc);
  if (CEL_UNLIKELY(slot == cel_nullptr)) {
    cel_OutOfMemoryStatus(interp->status);
    _cel_longjmp(interp->jmp);
  }
  *index = slot->index;
}

CEL_ATTRIBUTE_NODISCARD
static _cel_InterpreterSlot* cel_nonnull
_cel_Interpreter_TopSlot(_cel_Interpreter* cel_nonnull interp) {
  if (_cel_Array_Empty(&interp->slot_stack)) {
    cel_InternalStatus(interp->status,
                       cel_StringView_From("cel: slot stack empty"));
    _cel_longjmp(interp->jmp);
  }
  const uint32_t* index = _cel_Array_Back(&interp->slot_stack);
  return _cel_Deque_MutableAt(&interp->slots, *index);
}

static void _cel_Interpreter_PopSlot(_cel_Interpreter* cel_nonnull interp) {
  if (_cel_Array_Empty(&interp->slot_stack)) {
    cel_InternalStatus(interp->status,
                       cel_StringView_From("cel: slot stack empty"));
    _cel_longjmp(interp->jmp);
  }
  _cel_Array_Pop(&interp->slot_stack);
}

static void _cel_Interpreter_PushValueStackBounds(
    _cel_Interpreter* cel_nonnull interp,
    _cel_InterpreterValueStackBounds* cel_nonnull bounds) {
  _cel_InterpreterValueStackBounds** bounds_ptr =
      _cel_Array_Push(&interp->value_stack_bounds_stack, interp->alloc);
  if (CEL_UNLIKELY(bounds_ptr == cel_nullptr)) {
    cel_OutOfMemoryStatus(interp->status);
    _cel_longjmp(interp->jmp);
  }
  *bounds_ptr = bounds;
}

CEL_ATTRIBUTE_NODISCARD
static _cel_InterpreterValueStackBounds* cel_nonnull
_cel_Interpreter_TopValueStackBounds(_cel_Interpreter* cel_nonnull interp) {
  if (_cel_Array_Empty(&interp->value_stack_bounds_stack)) {
    cel_InternalStatus(
        interp->status,
        cel_StringView_From("cel: value stack bounds stack empty"));
    _cel_longjmp(interp->jmp);
  }
  return *_cel_Array_Back(&interp->value_stack_bounds_stack);
}

static void _cel_Interpreter_PopValueStackBounds(
    _cel_Interpreter* cel_nonnull interp) {
  if (_cel_Array_Empty(&interp->value_stack_bounds_stack)) {
    cel_InternalStatus(
        interp->status,
        cel_StringView_From("cel: value stack bounds stack empty"));
    _cel_longjmp(interp->jmp);
  }
  _cel_Array_Pop(&interp->value_stack_bounds_stack);
}

CEL_ATTRIBUTE_NODISCARD
static uint32_t _cel_Interpreter_DeleteSlots(
    _cel_Interpreter* cel_nonnull interp) {
  uint32_t size = (uint32_t)_cel_Deque_Size(&interp->slots);
  _cel_Deque_Clear(&interp->slots, interp->alloc);
  return size;
}

static void _cel_Interpreter_IncrementValueStackSizeN(
    _cel_Interpreter* cel_nonnull interp, uint32_t n) {
  _cel_InterpreterValueStackBounds_Add(
      _cel_Interpreter_TopValueStackBounds(interp), n);
}

static void _cel_Interpreter_IncrementValueStackSize(
    _cel_Interpreter* cel_nonnull interp) {
  _cel_InterpreterValueStackBounds_Increment(
      _cel_Interpreter_TopValueStackBounds(interp));
}

static void _cel_Interpreter_DecrementValueStackSizeN(
    _cel_Interpreter* cel_nonnull interp, uint32_t n) {
  _cel_InterpreterValueStackBounds_Subtract(
      _cel_Interpreter_TopValueStackBounds(interp), n);
}

static void _cel_Interpreter_DecrementValueStackSize(
    _cel_Interpreter* cel_nonnull interp) {
  _cel_InterpreterValueStackBounds_Decrement(
      _cel_Interpreter_TopValueStackBounds(interp));
}

static void _cel_Interpreter_PreVisitExpr(cel_AstVisitor* cel_nonnull visitor,
                                          const cel_Expr* cel_nonnull expr) {
  cel_ExprKind kind = cel_Expr_Kind(expr);
  switch (kind) {
    case cel_ExprKind_kUnspecified:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ExprKind_kIdent:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ExprKind_kConst:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ExprKind_kSelect:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ExprKind_kBinary:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ExprKind_kUnary:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ExprKind_kTernary:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ExprKind_kListElement:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ExprKind_kList:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ExprKind_kMapEntry:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ExprKind_kMap:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ExprKind_kCallArg:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ExprKind_kCall:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ExprKind_kComprehension:
      break;
    default:
      // For now, reject other AST node kinds.
      cel_InvalidArgumentStatusF(
          _cel_Interpreter_FromAstVisitor(visitor)->status,
          "cel: unsupported AST node kind: %d", kind);
      _cel_longjmp(_cel_Interpreter_FromAstVisitor(visitor)->jmp);
  }
}

static void _cel_Interpreter_VisitUnspecifiedExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_UnspecifiedExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  // Reject unspecified AST nodes, they are only created when errors occur.
  cel_InvalidArgumentStatus(
      interp->status,
      cel_StringView_From("cel: unexpected unspecified AST node kind"));
  _cel_longjmp(interp->jmp);
}

CEL_ATTRIBUTE_NODISCARD
static cel_StringView _cel_Interpreter_InternedString(
    const _cel_Interpreter* cel_nonnull interp, uint32_t index) {
  return _cel_String_ToStringView(
      _cel_Array_At(&interp->prog->strings_table, index));
}

CEL_ATTRIBUTE_NODISCARD
static uint32_t _cel_Interpreter_InternString(
    _cel_Interpreter* cel_nonnull interp, cel_StringView string) {
  _cel_FlatHashMapNode(interp->string_pool_indices) node =
      _cel_FlatHashMap_Find(&interp->string_pool_indices, &string);
  if (node == cel_nullptr) {
    uint32_t index = (uint32_t)_cel_Array_Size(&interp->prog->strings_table);
    _cel_String* string_ptr =
        _cel_Array_Push(&interp->prog->strings_table, interp->alloc);
    if (CEL_UNLIKELY(string_ptr == cel_nullptr)) {
      cel_OutOfMemoryStatus(interp->status);
      _cel_longjmp(interp->jmp);
    }
    _cel_String_Construct(string_ptr);
    if (!_cel_String_Assign(string_ptr, interp->alloc, string)) {
      cel_OutOfMemoryStatus(interp->status);
      _cel_longjmp(interp->jmp);
    }
    if (!_cel_String_Stabilize(string_ptr, interp->alloc)) {
      cel_OutOfMemoryStatus(interp->status);
      _cel_longjmp(interp->jmp);
    }
    string = _cel_String_ToStringView(string_ptr);
    _cel_FlatHashMapMutableNode(interp->string_pool_indices) mutable_node;
    if (!_cel_FlatHashMap_Insert(&interp->string_pool_indices, interp->alloc,
                                 &string, &mutable_node)) {
      cel_OutOfMemoryStatus(interp->status);
      _cel_longjmp(interp->jmp);
    }
    mutable_node->val = index;
    return index;
  }
  return node->val;
}

static void _cel_Interpreter_ValidateIdentExpr(
    _cel_Interpreter* cel_nonnull interp,
    const cel_IdentExpr* cel_nonnull expr) {
  cel_StringView name = cel_IdentExpr_Name(expr);
  if (CEL_UNLIKELY(cel_StringView_Empty(name))) {
    cel_InvalidArgumentStatusF(
        interp->status, "cel: ident expr AST node is missing name: id=%" PRIi64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
}

static void _cel_Interpreter_ValidateSelectExpr(
    _cel_Interpreter* cel_nonnull interp,
    const cel_SelectExpr* cel_nonnull expr) {
  const cel_Expr* operand = cel_SelectExpr_Operand(expr);
  if (CEL_UNLIKELY(operand == cel_nullptr)) {
    cel_InvalidArgumentStatusF(
        interp->status,
        "cel: select expr AST node is missing operand: id=%" PRIi64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  cel_StringView field = cel_SelectExpr_Field(expr);
  if (CEL_UNLIKELY(cel_StringView_Empty(field))) {
    cel_InvalidArgumentStatusF(
        interp->status,
        "cel: select expr AST node is missing field: id=%" PRIi64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
}

static void _cel_Interpreter_PlanIdent(_cel_Interpreter* cel_nonnull interp,
                                       cel_StringView name,
                                       bool jump_if_found) {
  // TODO: fix namespaced identifier lookup

  if (_cel_Container_Empty(&interp->rt->container)) {
    int32_t index = _cel_Interpreter_InternString(interp, name);
    if (CEL_UNLIKELY(index == -1)) {
      cel_OutOfMemoryStatus(interp->status);
      _cel_longjmp(interp->jmp);
    }
    _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
    if (CEL_UNLIKELY(instr == cel_nullptr)) {
      cel_OutOfMemoryStatus(interp->status);
      _cel_longjmp(interp->jmp);
    }
    instr->data.ident.name.indirect = (uint32_t)index;
    instr->kind =
        jump_if_found ? _cel_InstrKind_kIdentJump : _cel_InstrKind_kIdent;
  } else {
    _cel_FlatHashMapNode(interp->candidate_names_indices) node =
        _cel_FlatHashMap_Find(&interp->candidate_names_indices, &name);
    uint32_t candidate_names_index;
    if (node == cel_nullptr) {
      candidate_names_index =
          (uint32_t)_cel_Array_Size(&interp->prog->candidate_names_table);
      _cel_CandidateNames* candidate_names =
          _cel_Array_Push(&interp->prog->candidate_names_table, interp->alloc);
      if (CEL_UNLIKELY(candidate_names == cel_nullptr)) {
        cel_OutOfMemoryStatus(interp->status);
        _cel_longjmp(interp->jmp);
      }
      memset(candidate_names, 0, sizeof(*candidate_names));
      candidate_names->size = _cel_Container_Count(&interp->rt->container) + 1;
      candidate_names->data = (uint32_t*)cel_Allocator_Malloc(
          interp->alloc, sizeof(uint32_t) * candidate_names->size, cel_nullptr);
      if (CEL_UNLIKELY(candidate_names->data == cel_nullptr)) {
        _cel_Array_Pop(&interp->prog->candidate_names_table);
        cel_OutOfMemoryStatus(interp->status);
        _cel_longjmp(interp->jmp);
      }
      _cel_String candidate_name;
      _cel_String_Construct(&candidate_name);
      _cel_ContainerIterator cont_iter =
          _cel_Container_Iterate(&interp->rt->container);
      for (size_t i = 0; i < candidate_names->size - 1; ++i) {
        _cel_String_Clear(&candidate_name);
        CEL_ASSERT(_cel_ContainerIterator_HasNext(&cont_iter));
        cel_StringView prefix = _cel_ContainerIterator_Next(&cont_iter);
        _cel_String_Reserve(
            &candidate_name, interp->alloc,
            cel_StringView_Size(prefix) + 1 + cel_StringView_Size(name));
        if (!_cel_String_Append(&candidate_name, interp->alloc, prefix) ||
            !_cel_String_PushBack(&candidate_name, interp->alloc, '.') ||
            !_cel_String_Append(&candidate_name, interp->alloc, name)) {
          _cel_String_Destruct(&candidate_name, interp->alloc);
          cel_Allocator_FreeSized(interp->alloc, candidate_names->data,
                                  candidate_names->size * sizeof(uint32_t));
          _cel_Array_Pop(&interp->prog->candidate_names_table);
          cel_OutOfMemoryStatus(interp->status);
          _cel_longjmp(interp->jmp);
        }
        uint32_t string_index = _cel_Interpreter_InternString(
            interp, _cel_String_ToStringView(&candidate_name));
        candidate_names->data[i] = string_index;
      }
      uint32_t string_index = _cel_Interpreter_InternString(interp, name);
      candidate_names->data[candidate_names->size - 1] = string_index;
      name = _cel_Interpreter_InternedString(interp, string_index);
      _cel_FlatHashMapMutableNode(interp->candidate_names_indices) mutable_node;
      if (!_cel_FlatHashMap_Insert(&interp->candidate_names_indices,
                                   interp->alloc, &name, &mutable_node)) {
        cel_OutOfMemoryStatus(interp->status);
        _cel_longjmp(interp->jmp);
      }
      mutable_node->val = candidate_names_index;
    } else {
      candidate_names_index = node->val;
    }
    _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
    instr->data.cont_ident.candidate_names = candidate_names_index;
    instr->kind = jump_if_found ? _cel_InstrKind_kContIdentJump
                                : _cel_InstrKind_kContIdent;
  }
}

static void _cel_Interpreter_VisitIdentExpr(cel_AstVisitor* cel_nonnull visitor,
                                            const cel_IdentExpr* cel_nonnull
                                                expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_Interpreter_ValidateIdentExpr(interp, expr);
  cel_StringView name = cel_IdentExpr_Name(expr);
  _cel_FlatHashMapNode(interp->slot_map) slot_node;
  slot_node = _cel_FlatHashMap_Find(&interp->slot_map, &name);
  if (slot_node != cel_nullptr) {
    // Identifier belongs to a comprehension.
    _cel_Instr* lazy_call = _cel_Interpreter_AppendInstr(interp);
    lazy_call->kind = _cel_InstrKind_kLazyCall;
    lazy_call->data.lazy_call.slot = slot_node->val;
    const _cel_InterpreterSlot* slot =
        _cel_Deque_At(&interp->slots, slot_node->val);
    lazy_call->data.lazy_call.jump =
        (int32_t)slot->pc - (int32_t)_cel_Interpreter_PC(interp, lazy_call);
    // Account for the max value stack size of the subexpression in the current
    // expression.
    _cel_Interpreter_IncrementValueStackSizeN(interp,
                                              slot->value_stack_bounds.max);
    _cel_Interpreter_DecrementValueStackSizeN(interp,
                                              slot->value_stack_bounds.max);
  } else {
    _cel_Interpreter_PlanIdent(interp, name,
                               /*jump_if_found=*/false);
  }
  _cel_Interpreter_IncrementValueStackSize(interp);
}

static void _cel_Interpreter_VisitConstExpr(cel_AstVisitor* cel_nonnull visitor,
                                            const cel_ConstExpr* cel_nonnull
                                                expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  const cel_Constant* value = cel_ConstExpr_Value(expr);
  const cel_ConstantKind kind = cel_Constant_Kind(value);
  switch (kind) {
    case cel_ConstantKind_kNull: {
      _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
      instr->kind = _cel_InstrKind_kNullConst;
    } break;
    case cel_ConstantKind_kBool: {
      _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
      instr->kind = cel_Constant_GetBool(value) ? _cel_InstrKind_kTrueConst
                                                : _cel_InstrKind_kFalseConst;
    } break;
    case cel_ConstantKind_kInt: {
      _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
      instr->data.int_const.value = cel_Constant_GetInt(value);
      instr->kind = _cel_InstrKind_kIntConst;
    } break;
    case cel_ConstantKind_kUint: {
      _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
      instr->data.uint_const.value = cel_Constant_GetUint(value);
      instr->kind = _cel_InstrKind_kUintConst;
    } break;
    case cel_ConstantKind_kDouble: {
      _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
      instr->data.double_const.value = cel_Constant_GetDouble(value);
      instr->kind = _cel_InstrKind_kDoubleConst;
    } break;
    case cel_ConstantKind_kBytes: {
      uint32_t index =
          _cel_Interpreter_InternString(interp, cel_Constant_GetBytes(value));
      _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
      instr->data.bytes_const.value.indirect = index;
      instr->kind = _cel_InstrKind_kBytesConst;
    } break;
    case cel_ConstantKind_kString: {
      uint32_t index =
          _cel_Interpreter_InternString(interp, cel_Constant_GetString(value));
      _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
      instr->data.string_const.value.indirect = index;
      instr->kind = _cel_InstrKind_kStringConst;
    } break;
    case cel_ConstantKind_kDuration: {
      _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
      instr->data.duration_const.value = cel_Constant_GetDuration(value);
      instr->kind = _cel_InstrKind_kDurationConst;
    } break;
    case cel_ConstantKind_kTimestamp: {
      _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
      instr->data.timestamp_const.value = cel_Constant_GetTimestamp(value);
      instr->kind = _cel_InstrKind_kTimestampConst;
    } break;
    default:
      cel_InvalidArgumentStatusF(
          _cel_Interpreter_FromAstVisitor(visitor)->status,
          "cel: const expr AST node has unspecified value: id=%" PRIi64,
          cel_Expr_Id(cel_Expr_UpCast(expr)));
      _cel_longjmp(interp->jmp);
  }
  _cel_Interpreter_IncrementValueStackSize(interp);
}

typedef struct {
  const cel_IdentExpr* cel_nullable ident;
  size_t select_depth;
} _cel_InterpreterSelectIdentExpr;

CEL_ATTRIBUTE_NODISCARD
static _cel_InterpreterSelectIdentExpr _cel_Intepreter_GetSelectIdentExpr(
    const cel_SelectExpr* cel_nonnull expr) {
  _cel_InterpreterSelectIdentExpr result;
  result.ident = cel_nullptr;
  result.select_depth = 0;
  while (true) {
    const cel_Expr* operand = cel_SelectExpr_Operand(expr);
    if (operand == cel_nullptr) {
      result.select_depth = 0;
      return result;
    }
    switch (cel_Expr_Kind(operand)) {
      case cel_ExprKind_kIdent:
        result.ident = cel_IdentExpr_DownCast(operand);
        return result;
      case cel_ExprKind_kSelect:
        expr = cel_SelectExpr_DownCast(operand);
        ++result.select_depth;
        break;
      default:
        result.select_depth = 0;
        return result;
    }
  }
}

static void _cel_Intepreter_CollectNspaceVarSelects(
    _cel_Interpreter* cel_nonnull interp,
    const cel_SelectExpr* cel_nonnull expr) {
  {
    _cel_InterpreterNspaceVarSelect* select =
        _cel_Array_Push(&interp->nspace_var.selects, interp->alloc);
    if (CEL_UNLIKELY(select == cel_nullptr)) {
      cel_OutOfMemoryStatus(interp->status);
      _cel_longjmp(interp->jmp);
    }
    select->expr = expr;
    select->ident_pc = 0;
    select->select_pc = 0;
  }
  while (true) {
    const cel_Expr* operand = cel_SelectExpr_Operand(expr);
    CEL_ASSERT_NOT_NULL(operand);
    switch (cel_Expr_Kind(operand)) {
      case cel_ExprKind_kIdent:
        return;
      case cel_ExprKind_kSelect: {
        _cel_InterpreterNspaceVarSelect* select =
            _cel_Array_Push(&interp->nspace_var.selects, interp->alloc);
        if (CEL_UNLIKELY(select == cel_nullptr)) {
          cel_OutOfMemoryStatus(interp->status);
          _cel_longjmp(interp->jmp);
        }
        expr = select->expr = cel_SelectExpr_DownCast(operand);
        select->ident_pc = 0;
        select->select_pc = 0;
        break;
      }
      default:
        CEL_UNREACHABLE();
    }
  }
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_Interpreter_GetNspaceVar(_cel_Interpreter* cel_nonnull interp,
                                          const cel_SelectExpr* cel_nonnull
                                              expr) {
  // Check if this is the first select expr in a chain of `foo.bar.baz`, such
  // that this is `baz`.
  const cel_Expr* parent = cel_Expr_Parent(cel_Expr_UpCast(expr));
  if (parent == cel_nullptr || cel_Expr_Kind(parent) != cel_ExprKind_kSelect) {
    _cel_InterpreterSelectIdentExpr select_ident_expr =
        _cel_Intepreter_GetSelectIdentExpr(expr);
    if (select_ident_expr.ident != cel_nullptr) {
      // First select in a chain of select(s) and a single ident.
      const cel_Ref* ident_operand_ref =
          cel_Expr_Ref(cel_Expr_UpCast(select_ident_expr.ident));
      if (ident_operand_ref == cel_nullptr) {
        _cel_Array_Clear(&interp->nspace_var.selects);
        interp->nspace_var.ident.expr = select_ident_expr.ident;
        interp->nspace_var.ident.start_pc = 0;
        _cel_Array_Reserve(&interp->nspace_var.selects, interp->alloc,
                           select_ident_expr.select_depth + 1);
        _cel_Intepreter_CollectNspaceVarSelects(interp, expr);
        return true;
      }
    }
  }
  return false;
}

static void _cel_Interpreter_PlanSelectExpr(
    _cel_Interpreter* cel_nonnull interp,
    const cel_SelectExpr* cel_nonnull expr) {
  cel_StringView field_name = cel_SelectExpr_Field(expr);
  const cel_Expr* operand = cel_SelectExpr_Operand(expr);
  const cel_Type* operand_type =
      operand != cel_nullptr ? cel_Expr_Type(operand) : cel_DynType;
  const upb_FieldDef* field = cel_nullptr;
  if (cel_Type_IsStruct(operand_type)) {
    cel_StringView operand_type_name =
        cel_StructType_Name(cel_StructType_DownCast(operand_type));
    const upb_MessageDef* operand_message =
        upb_DefPool_FindMessageByNameWithSize(
            interp->def_pool, cel_StringView_Data(operand_type_name),
            cel_StringView_Size(operand_type_name));
    if (operand_message != cel_nullptr) {
      field = upb_MessageDef_FindFieldByNameWithSize(
          operand_message, cel_StringView_Data(field_name),
          cel_StringView_Size(field_name));
    }
    if (field != cel_nullptr) {
      _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
      if (cel_SelectExpr_TestOnly(expr)) {
        instr->kind = _cel_InstrKind_kMessageHas;
        instr->data.message_has.field = field;
      } else {
        instr->kind = _cel_InstrKind_kMessageSelect;
        instr->data.message_select.field = field;
      }
      return;
    }
  }
  uint32_t string_index = _cel_Interpreter_InternString(interp, field_name);
  _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
  if (cel_SelectExpr_TestOnly(expr)) {
    instr->kind = _cel_InstrKind_kHas;
    instr->data.has.field.indirect = (uint32_t)string_index;
  } else {
    instr->kind = _cel_InstrKind_kSelect;
    instr->data.select.field.indirect = (uint32_t)string_index;
  }
}

static void _cel_Interpreter_SetIdentJump(_cel_Instr* ident_jump,
                                          bool missing_error,
                                          int32_t found_jump,
                                          int32_t missing_jump) {
  CEL_ASSERT_NE(found_jump, 0);
  CEL_ASSERT_NE(missing_jump, 0);

  switch (ident_jump->kind) {
    case _cel_InstrKind_kIdentJump:
      ident_jump->data.ident_jump.missing_error = missing_error ? 1 : 0;
      ident_jump->data.ident_jump.found_jump = found_jump;
      ident_jump->data.ident_jump.missing_jump = missing_jump;
      break;
    case _cel_InstrKind_kContIdentJump:
      ident_jump->data.cont_ident_jump.missing_error = missing_error ? 1 : 0;
      ident_jump->data.cont_ident_jump.found_jump = found_jump;
      ident_jump->data.cont_ident_jump.missing_jump = missing_jump;
      break;
    default:
      CEL_UNREACHABLE();
  }
}

static void _cel_Interpreter_BuildIdentName(_cel_Interpreter* interp,
                                            size_t selects_num,
                                            size_t selects_count) {
  CEL_ASSERT_GE(selects_count, selects_num);

  _cel_InterpreterNspaceVarSelect* const selects =
      _cel_Array_MutableData(&interp->nspace_var.selects);
  _cel_String_Clear(&interp->nspace_var.name);
  if (!_cel_String_Append(&interp->nspace_var.name, interp->alloc,
                          cel_IdentExpr_Name(interp->nspace_var.ident.expr))) {
    cel_OutOfMemoryStatus(interp->status);
    _cel_longjmp(interp->jmp);
  }
  for (size_t i = 0; i < selects_num; ++i) {
    if (!_cel_String_PushBack(&interp->nspace_var.name, interp->alloc, '.') ||
        !_cel_String_Append(
            &interp->nspace_var.name, interp->alloc,
            cel_SelectExpr_Field(selects[selects_count - i - 1].expr))) {
      cel_OutOfMemoryStatus(interp->status);
      _cel_longjmp(interp->jmp);
    }
  }
}

static void _cel_Interpreter_PreVisitSelectExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_SelectExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  if (_cel_Interpreter_GetNspaceVar(interp, expr)) {
    // If we are dealing with namespaced variables, only under parse-only
    // expressions, detect that here. If that is the case, we skip traversing
    // further down the subtree and handle it all here.
    _cel_InterpreterNspaceVarSelect* const selects =
        _cel_Array_MutableData(&interp->nspace_var.selects);
    const size_t selects_count = _cel_Array_Size(&interp->nspace_var.selects);
    CEL_ASSERT_GT(selects_count, 0);
    {
      _cel_Interpreter_ValidateIdentExpr(interp, interp->nspace_var.ident.expr);
      for (size_t i = selects_count; i > 0; --i) {
        _cel_Interpreter_ValidateSelectExpr(interp, selects[i - 1].expr);
      }
    }

    // Idents.
    for (size_t i = selects_count; i > 0; --i) {
      _cel_Interpreter_BuildIdentName(interp, i, selects_count);
      selects[i - 1].ident_pc = _cel_Interpreter_NextPC(interp);
      _cel_Interpreter_PlanIdent(
          interp, _cel_String_ToStringView(&interp->nspace_var.name),
          /*jump_if_found=*/true);
    }
    _cel_Interpreter_BuildIdentName(interp, 0, selects_count);
    interp->nspace_var.ident.start_pc = _cel_Interpreter_NextPC(interp);
    _cel_Interpreter_PlanIdent(
        interp, _cel_String_ToStringView(&interp->nspace_var.name),
        /*jump_if_found=*/true);

    // Selects.
    const uint32_t first_select_pc = _cel_Interpreter_NextPC(interp);
    for (size_t i = selects_count; i > 0; --i) {
      selects[i - 1].select_pc = _cel_Interpreter_NextPC(interp);
      _cel_Interpreter_PlanSelectExpr(interp, selects[i - 1].expr);
    }

    // Jumps.
    const uint32_t last_pc = _cel_Interpreter_NextPC(interp);
    uint32_t select_pc;
    uint32_t ident_pc = selects[selects_count - 1].ident_pc;
    _cel_Instr* ident_jump = _cel_Interpreter_InstrAt(interp, ident_pc);
    _cel_Interpreter_SetIdentJump(ident_jump, /*missing_error=*/false,
                                  last_pc - ident_pc, 1);
    for (size_t i = selects_count - 1; i > 0; --i) {
      ident_pc = selects[i - 1].ident_pc;
      ident_jump = _cel_Interpreter_InstrAt(interp, ident_pc);
      select_pc = selects[selects_count - 1 - i].select_pc;
      _cel_Interpreter_SetIdentJump(ident_jump, /*missing_error=*/false,
                                    select_pc - ident_pc, 1);
    }
    ident_pc = interp->nspace_var.ident.start_pc;
    ident_jump = _cel_Interpreter_InstrAt(interp, ident_pc);
    _cel_Interpreter_SetIdentJump(ident_jump, /*missing_error=*/true,
                                  first_select_pc - ident_pc,
                                  last_pc - ident_pc);

    _cel_Interpreter_IncrementValueStackSize(interp);
    cel_AstTraverser_StepOut(interp->traverser);
  } else {
    _cel_Interpreter_ValidateSelectExpr(interp, expr);
  }
}

static void _cel_Interpreter_PostVisitSelectExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_SelectExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_Interpreter_PlanSelectExpr(interp, expr);
}

static void _cel_Interpreter_PreVisitBinaryExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_BinaryExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  switch (cel_BinaryExpr_Op(expr)) {
    case cel_BinaryOp_kAdd:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_BinaryOp_kSubtract:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_BinaryOp_kMultiply:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_BinaryOp_kDivide:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_BinaryOp_kModulo:
      break;
    case cel_BinaryOp_kLogicalAnd: {
      _cel_InterpreterCond* cond = _cel_Interpreter_PushCond(interp);
      cond->binary.node = expr;
      // pc is filled in _cel_Interpreter_PostVisitBinaryExprLeft.
      cond->binary.pc = 0;
      cond->binary.cond = false;
    } break;
    case cel_BinaryOp_kLogicalOr: {
      _cel_InterpreterCond* cond = _cel_Interpreter_PushCond(interp);
      cond->binary.node = expr;
      // pc is filled in _cel_Interpreter_PostVisitBinaryExprLeft.
      cond->binary.pc = 0;
      cond->binary.cond = true;
    } break;
    case cel_BinaryOp_kEquals:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_BinaryOp_kNotEquals:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_BinaryOp_kLess:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_BinaryOp_kLessEquals:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_BinaryOp_kGreater:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_BinaryOp_kGreaterEquals:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_BinaryOp_kIndex:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_BinaryOp_kIn:
      break;
    default:
      // For now, reject other AST node kinds.
      cel_InvalidArgumentStatusF(interp->status,
                                 "cel: unsupported binary expr operator: %d",
                                 cel_BinaryExpr_Op(expr));
      _cel_longjmp(interp->jmp);
  }
  if (cel_BinaryExpr_Left(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(
        interp->status, "cel: binary expr missing left operand: %" PRId64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  if (cel_BinaryExpr_Right(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(
        interp->status, "cel: binary expr missing right operand: %" PRId64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
}

static void _cel_Interpreter_PostVisitBinaryExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_BinaryExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
  switch (cel_BinaryExpr_Op(expr)) {
    case cel_BinaryOp_kAdd:
      instr->kind = _cel_InstrKind_kAdd;
      break;
    case cel_BinaryOp_kSubtract:
      instr->kind = _cel_InstrKind_kSubtract;
      break;
    case cel_BinaryOp_kMultiply:
      instr->kind = _cel_InstrKind_kMultiply;
      break;
    case cel_BinaryOp_kDivide:
      instr->kind = _cel_InstrKind_kDivide;
      break;
    case cel_BinaryOp_kModulo:
      instr->kind = _cel_InstrKind_kModulo;
      break;
    case cel_BinaryOp_kLogicalAnd: {
      instr->kind = _cel_InstrKind_kLogicalAnd;
      _cel_InterpreterCond* cond = _cel_Interpreter_TopCond(interp);
      CEL_ASSERT_EQ(cel_Expr_Kind(cond->node), cel_ExprKind_kBinary);
      _cel_InterpreterBinaryCond* binary_cond = &cond->binary;
      CEL_ASSERT_GT(binary_cond->pc, 0);
      _cel_Instr* jump_instr =
          _cel_Interpreter_InstrAt(interp, binary_cond->pc);
      CEL_ASSERT_EQ(jump_instr->kind, _cel_InstrKind_kCondJump);
      jump_instr->data.cond_jump.jump = (int32_t)((instr - jump_instr) + 1);
      _cel_Interpreter_PopCond(interp);
    } break;
    case cel_BinaryOp_kLogicalOr: {
      instr->kind = _cel_InstrKind_kLogicalOr;
      _cel_InterpreterCond* cond = _cel_Interpreter_TopCond(interp);
      CEL_ASSERT_EQ(cel_Expr_Kind(cond->node), cel_ExprKind_kBinary);
      _cel_InterpreterBinaryCond* binary_cond = &cond->binary;
      CEL_ASSERT_GT(binary_cond->pc, 0);
      _cel_Instr* jump_instr =
          _cel_Interpreter_InstrAt(interp, binary_cond->pc);
      CEL_ASSERT_EQ(jump_instr->kind, _cel_InstrKind_kCondJump);
      jump_instr->data.cond_jump.jump = (int32_t)((instr - jump_instr) + 1);
      _cel_Interpreter_PopCond(interp);
    } break;
    case cel_BinaryOp_kEquals:
      instr->kind = _cel_InstrKind_kEquals;
      break;
    case cel_BinaryOp_kNotEquals:
      instr->kind = _cel_InstrKind_kNotEquals;
      break;
    case cel_BinaryOp_kLess:
      instr->kind = _cel_InstrKind_kLess;
      break;
    case cel_BinaryOp_kLessEquals:
      instr->kind = _cel_InstrKind_kLessEquals;
      break;
    case cel_BinaryOp_kGreater:
      instr->kind = _cel_InstrKind_kGreater;
      break;
    case cel_BinaryOp_kGreaterEquals:
      instr->kind = _cel_InstrKind_kGreaterEquals;
      break;
    case cel_BinaryOp_kIndex:
      instr->kind = _cel_InstrKind_kIndex;
      break;
    case cel_BinaryOp_kIn:
      instr->kind = _cel_InstrKind_kIn;
      break;
    default:
      CEL_UNREACHABLE();
  }
  _cel_Interpreter_DecrementValueStackSize(interp);
}

static void _cel_Interpreter_PostVisitBinaryExprLeft(
    cel_AstVisitor* cel_nonnull visitor, const cel_Expr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_InterpreterCond* cond = _cel_Interpreter_PeekCond(interp);
  if (cond == cel_nullptr ||
      cel_Expr_Kind(cond->node) != cel_ExprKind_kBinary) {
    return;
  }
  _cel_InterpreterBinaryCond* binary_cond = &cond->binary;
  if (cel_Expr_UpCast(binary_cond->node) != cel_Expr_Parent(expr)) {
    return;
  }
  _cel_Instr* cond_jump = _cel_Interpreter_AppendInstr(interp);
  cond_jump->kind = _cel_InstrKind_kCondJump;
  // jump is filled out in _cel_Interpreter_PostVisitBinaryExpr.
  cond_jump->data.cond_jump.jump = 0;
  cond_jump->data.cond_jump.cond = binary_cond->cond;
  switch (cel_BinaryExpr_Op(cel_BinaryExpr_DownCast(cel_Expr_Parent(expr)))) {
    case cel_BinaryOp_kLogicalAnd:
      cond_jump->data.cond_jump.cond = false;
      break;
    case cel_BinaryOp_kLogicalOr:
      cond_jump->data.cond_jump.cond = true;
      break;
    default:
      CEL_UNREACHABLE();
  }
  binary_cond->pc = _cel_Interpreter_PC(interp, cond_jump);
}

static void _cel_Interpreter_PostVisitUnaryExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_UnaryExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
  switch (cel_UnaryExpr_Op(expr)) {
    case cel_UnaryOp_kLogicalNot:
      instr->kind = _cel_InstrKind_kLogicalNot;
      break;
    case cel_UnaryOp_kNegate:
      instr->kind = _cel_InstrKind_kNegate;
      break;
    default:
      CEL_UNREACHABLE();
  }
}

static void _cel_Interpreter_PreVisitUnaryExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_UnaryExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  switch (cel_UnaryExpr_Op(expr)) {
    case cel_UnaryOp_kLogicalNot:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_UnaryOp_kNegate:
      break;
    default:
      // For now, reject other AST node kinds.
      cel_InvalidArgumentStatusF(interp->status,
                                 "cel: unsupported unary expr operator: %d",
                                 cel_UnaryExpr_Op(expr));
      _cel_longjmp(interp->jmp);
  }
  if (cel_UnaryExpr_Arg(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(interp->status,
                               "cel: unary expr missing operand: %" PRId64,
                               cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
}

static void _cel_Interpreter_PreVisitTernaryExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_TernaryExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  switch (cel_TernaryExpr_Op(expr)) {
    case cel_TernaryOp_kConditional:
      break;
    default:
      // For now, reject other AST node kinds.
      cel_InvalidArgumentStatusF(interp->status,
                                 "cel: unsupported ternary expr operator: %d",
                                 cel_TernaryExpr_Op(expr));
      _cel_longjmp(interp->jmp);
  }
  if (cel_TernaryExpr_Condition(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(interp->status,
                               "cel: ternary expr missing condition: %" PRId64,
                               cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  if (cel_TernaryExpr_IfTrue(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(interp->status,
                               "cel: ternary expr missing if-true: %" PRId64,
                               cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  if (cel_TernaryExpr_IfFalse(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(interp->status,
                               "cel: ternary expr missing if-false: %" PRId64,
                               cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  _cel_InterpreterCond* cond = _cel_Interpreter_PushCond(interp);
  cond->ternary.node = expr;
  // Set in _cel_Interpreter_PostVisitTernaryExprCondition.
  cond->ternary.trilean_jump_pc = 0;
  // Set in _cel_Interpreter_PostVisitTernaryExprIfTrue.
  cond->ternary.jump_pc = 0;
}

static void _cel_Interpreter_PostVisitTernaryExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_TernaryExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_InterpreterCond* cond = _cel_Interpreter_TopCond(interp);
  CEL_ASSERT_EQ(cel_Expr_Kind(cond->node), cel_ExprKind_kTernary);
  _cel_InterpreterTernaryCond* ternary_cond = &cond->ternary;
  _cel_Instr* trilean_jump =
      _cel_Interpreter_InstrAt(interp, ternary_cond->trilean_jump_pc);
  CEL_ASSERT_EQ(trilean_jump->kind, _cel_InstrKind_kTrileanJump);
  trilean_jump->data.trilean_jump.error_jump =
      _cel_Interpreter_NextPC(interp) -
      _cel_Interpreter_PC(interp, trilean_jump);
  _cel_Instr* jump = _cel_Interpreter_InstrAt(interp, ternary_cond->jump_pc);
  CEL_ASSERT_EQ(jump->kind, _cel_InstrKind_kJump);
  jump->data.jump.jump =
      _cel_Interpreter_NextPC(interp) - _cel_Interpreter_PC(interp, jump);
  _cel_Interpreter_PopCond(interp);
}

static void _cel_Interpreter_PostVisitTernaryExprCondition(
    cel_AstVisitor* cel_nonnull visitor, const cel_Expr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_InterpreterCond* cond = _cel_Interpreter_TopCond(interp);
  CEL_ASSERT_EQ(cel_Expr_Kind(cond->node), cel_ExprKind_kTernary);
  _cel_InterpreterTernaryCond* ternary_cond = &cond->ternary;
  _cel_Instr* trilean_jump = _cel_Interpreter_AppendInstr(interp);
  trilean_jump->kind = _cel_InstrKind_kTrileanJump;
  // false_jump is set in _cel_Interpreter_PostVisitTernaryExprIfTrue.
  trilean_jump->data.trilean_jump.false_jump = 0;
  // error_jump is set in _cel_Interpreter_PostVisitTernaryExpr.
  trilean_jump->data.trilean_jump.error_jump = 0;
  ternary_cond->trilean_jump_pc = _cel_Interpreter_PC(interp, trilean_jump);
  _cel_Interpreter_DecrementValueStackSize(interp);
}

static void _cel_Interpreter_PostVisitTernaryExprIfTrue(
    cel_AstVisitor* cel_nonnull visitor, const cel_Expr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_InterpreterCond* cond = _cel_Interpreter_TopCond(interp);
  CEL_ASSERT_EQ(cel_Expr_Kind(cond->node), cel_ExprKind_kTernary);
  _cel_InterpreterTernaryCond* ternary_cond = &cond->ternary;
  _cel_Instr* jump = _cel_Interpreter_AppendInstr(interp);
  _cel_Instr* trilean_jump =
      _cel_Interpreter_InstrAt(interp, ternary_cond->trilean_jump_pc);
  CEL_ASSERT_EQ(trilean_jump->kind, _cel_InstrKind_kTrileanJump);
  trilean_jump->data.trilean_jump.false_jump =
      _cel_Interpreter_NextPC(interp) -
      _cel_Interpreter_PC(interp, trilean_jump);
  jump->kind = _cel_InstrKind_kJump;
  // Set in _cel_Interpreter_PostVisitTernaryExpr.
  jump->data.jump.jump = 0;
  ternary_cond->jump_pc = _cel_Interpreter_PC(interp, jump);
  _cel_Interpreter_DecrementValueStackSize(interp);
}

static void _cel_Interpreter_PreVisitListExpr(
    cel_AstVisitor* cel_nonnull visitor, const cel_ListExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  size_t num_elements = cel_ListExpr_Elements(expr, cel_nullptr, cel_nullptr);
  if (num_elements > (size_t)INT32_MAX) {
    // TODO: allow this to be configurable
    cel_InvalidArgumentStatusF(interp->status,
                               "cel: list expr too large: %" PRId64,
                               cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  _cel_InterpreterList* list = _cel_Interpreter_PushList(interp);
  _cel_Array_Construct(&list->pcs);
  if (!_cel_Array_Reserve(&list->pcs, interp->alloc, num_elements)) {
    cel_OutOfMemoryStatus(interp->status);
    _cel_longjmp(interp->jmp);
  }
}

static void _cel_Interpreter_PostVisitListExpr(
    cel_AstVisitor* cel_nonnull visitor, const cel_ListExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
  instr->kind = _cel_InstrKind_kList;
  size_t element_count = cel_ListExpr_Elements(expr, cel_nullptr, cel_nullptr);
  instr->data.list.count = (uint32_t)element_count;
  _cel_InterpreterList* list = _cel_Interpreter_TopList(interp);
  CEL_ASSERT_EQ(_cel_Array_Size(&list->pcs), element_count);
  for (size_t i = 0; i < element_count; ++i) {
    uint32_t pc = *_cel_Array_At(&list->pcs, i);
    _cel_Instr* error_jump = _cel_Interpreter_InstrAt(interp, pc);
    CEL_ASSERT_EQ(error_jump->kind, _cel_InstrKind_kErrorJump);
    error_jump->data.error_jump.jump = _cel_Interpreter_NextPC(interp) -
                                       _cel_Interpreter_PC(interp, error_jump);
    error_jump->data.error_jump.pop = (uint32_t)i;
  }
  _cel_Array_Destruct(&list->pcs, interp->alloc);
  _cel_Interpreter_PopList(interp);
  if (element_count > 0) {
    _cel_Interpreter_DecrementValueStackSizeN(interp, element_count - 1);
  } else {
    _cel_Interpreter_IncrementValueStackSize(interp);
  }
}

static void _cel_Interpreter_PreVisitListElementExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_ListElementExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  if (cel_ListElementExpr_Value(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(interp->status,
                               "cel: list element expr missing value: %" PRId64,
                               cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  if (cel_ListElementExpr_Optional(expr)) {
    cel_InvalidArgumentStatusF(
        interp->status,
        "cel: list element expr optional is not yet implemented: %" PRId64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
}

static void _cel_Interpreter_PostVisitListElementExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_ListElementExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_InterpreterList* list = _cel_Interpreter_TopList(interp);
  _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
  instr->kind = _cel_InstrKind_kErrorJump;
  // Set in _cel_Interpreter_PostVisitListExpr.
  instr->data.error_jump.jump = 0;
  instr->data.error_jump.pop = 0;
  uint32_t* pc = _cel_Array_Push(&list->pcs, interp->alloc);
  if (pc == cel_nullptr) {
    cel_OutOfMemoryStatus(interp->status);
    _cel_longjmp(interp->jmp);
  }
  *pc = _cel_Interpreter_PC(interp, instr);
}

static void _cel_Interpreter_PreVisitMapExpr(
    cel_AstVisitor* cel_nonnull visitor, const cel_MapExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  size_t num_entries = cel_MapExpr_Entries(expr, cel_nullptr, cel_nullptr);
  if (num_entries > (size_t)INT32_MAX) {
    // TODO: allow this to be configurable
    cel_InvalidArgumentStatusF(interp->status,
                               "cel: map expr too large: %" PRId64,
                               cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  _cel_InterpreterMap* map = _cel_Interpreter_PushMap(interp);
  _cel_Array_Construct(&map->pcs);
  if (!_cel_Array_Reserve(&map->pcs, interp->alloc, num_entries * 2)) {
    cel_OutOfMemoryStatus(interp->status);
    _cel_longjmp(interp->jmp);
  }
}

static void _cel_Interpreter_PostVisitMapExpr(
    cel_AstVisitor* cel_nonnull visitor, const cel_MapExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
  instr->kind = _cel_InstrKind_kMap;
  size_t entries_count = cel_MapExpr_Entries(expr, cel_nullptr, cel_nullptr);
  instr->data.map.count = (uint32_t)entries_count;
  _cel_InterpreterMap* map = _cel_Interpreter_TopMap(interp);
  CEL_ASSERT_EQ(_cel_Array_Size(&map->pcs), entries_count * 2);
  for (size_t i = 0; i < entries_count; ++i) {
    uint32_t key_pc = *_cel_Array_At(&map->pcs, i * 2);
    uint32_t value_pc = *_cel_Array_At(&map->pcs, i * 2 + 1);
    _cel_Instr* key_jump = _cel_Interpreter_InstrAt(interp, key_pc);
    CEL_ASSERT_EQ(key_jump->kind, _cel_InstrKind_kKeyJump);
    key_jump->data.key_jump.jump =
        _cel_Interpreter_NextPC(interp) - _cel_Interpreter_PC(interp, key_jump);
    key_jump->data.key_jump.pop = (uint32_t)(i * 2);
    _cel_Instr* error_jump = _cel_Interpreter_InstrAt(interp, value_pc);
    CEL_ASSERT_EQ(error_jump->kind, _cel_InstrKind_kErrorJump);
    error_jump->data.error_jump.jump = _cel_Interpreter_NextPC(interp) -
                                       _cel_Interpreter_PC(interp, error_jump);
    error_jump->data.error_jump.pop = (uint32_t)(i * 2 + 1);
  }
  _cel_Array_Destruct(&map->pcs, interp->alloc);
  _cel_Interpreter_PopMap(interp);
  if (entries_count > 0) {
    _cel_Interpreter_DecrementValueStackSizeN(interp, entries_count * 2 - 1);
  } else {
    _cel_Interpreter_IncrementValueStackSize(interp);
  }
}

static void _cel_Interpreter_PreVisitMapEntryExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_MapEntryExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  if (cel_MapEntryExpr_Key(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(interp->status,
                               "cel: map entry expr missing key: %" PRId64,
                               cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  if (cel_MapEntryExpr_Value(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(interp->status,
                               "cel: map entry expr missing value: %" PRId64,
                               cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  if (cel_MapEntryExpr_Optional(expr)) {
    cel_InvalidArgumentStatusF(
        interp->status,
        "cel: map entry expr optional is not yet implemented: %" PRId64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
}

static void _cel_Interpreter_PostVisitMapEntryExprKey(
    cel_AstVisitor* cel_nonnull visitor, const cel_Expr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_InterpreterMap* map = _cel_Interpreter_TopMap(interp);
  _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
  instr->kind = _cel_InstrKind_kKeyJump;
  // Set in _cel_Interpreter_PostVisitMapExpr.
  instr->data.key_jump.jump = 0;
  instr->data.key_jump.pop = 0;
  uint32_t* pc = _cel_Array_Push(&map->pcs, interp->alloc);
  if (pc == cel_nullptr) {
    cel_OutOfMemoryStatus(interp->status);
    _cel_longjmp(interp->jmp);
  }
  *pc = _cel_Interpreter_PC(interp, instr);
}

static void _cel_Interpreter_PostVisitMapEntryExprValue(
    cel_AstVisitor* cel_nonnull visitor, const cel_Expr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_InterpreterMap* map = _cel_Interpreter_TopMap(interp);
  _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
  instr->kind = _cel_InstrKind_kErrorJump;
  // Set in _cel_Interpreter_PostVisitMapExpr.
  instr->data.key_jump.jump = 0;
  instr->data.key_jump.pop = 0;
  uint32_t* pc = _cel_Array_Push(&map->pcs, interp->alloc);
  if (pc == cel_nullptr) {
    cel_OutOfMemoryStatus(interp->status);
    _cel_longjmp(interp->jmp);
  }
  *pc = _cel_Interpreter_PC(interp, instr);
}

static void _cel_Interpreter_PreVisitCallArgExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_CallArgExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  if (cel_CallArgExpr_Value(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(interp->status,
                               "cel: call arg expr missing value: %" PRId64,
                               cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
}

static void _cel_Interpreter_PostVisitCallExpr(
    cel_AstVisitor* cel_nonnull visitor, const cel_CallExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  uint32_t index =
      _cel_Interpreter_InternString(interp, cel_CallExpr_Function(expr));
  _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
  instr->data.call.name = index;
  instr->data.call.args = cel_CallExpr_Args(expr, cel_nullptr, cel_nullptr) +
                          (cel_CallExpr_Target(expr) != 0);
  cel_StringView fn_name =
      _cel_Interpreter_InternedString(interp, instr->data.call.name);
  bool found = false;
  if (instr->data.call.args == 1) {
    if (cel_StringView_Equals(fn_name, cel_StringView_From("int"))) {
      instr->kind = _cel_InstrKind_kCallInt;
      found = true;
    } else if (cel_StringView_Equals(fn_name, cel_StringView_From("uint"))) {
      instr->kind = _cel_InstrKind_kCallUint;
      found = true;
    } else if (cel_StringView_Equals(fn_name, cel_StringView_From("bool"))) {
      instr->kind = _cel_InstrKind_kCallBool;
      found = true;
    } else if (cel_StringView_Equals(fn_name, cel_StringView_From("double"))) {
      instr->kind = _cel_InstrKind_kCallDouble;
      found = true;
    } else if (cel_StringView_Equals(fn_name, cel_StringView_From("bytes"))) {
      instr->kind = _cel_InstrKind_kCallBytes;
      found = true;
    } else if (cel_StringView_Equals(fn_name, cel_StringView_From("string"))) {
      instr->kind = _cel_InstrKind_kCallString;
      found = true;
    } else if (cel_StringView_Equals(fn_name,
                                     cel_StringView_From("timestamp"))) {
      instr->kind = _cel_InstrKind_kCallTimestamp;
      found = true;
    } else if (cel_StringView_Equals(fn_name,
                                     cel_StringView_From("duration"))) {
      instr->kind = _cel_InstrKind_kCallDuration;
      found = true;
    } else if (cel_StringView_Equals(fn_name, cel_StringView_From("size"))) {
      instr->kind = _cel_InstrKind_kCallSize;
      found = true;
    }
  } else if (instr->data.call.args == 2) {
    if (cel_StringView_Equals(fn_name, cel_StringView_From("contains"))) {
      instr->kind = _cel_InstrKind_kCallContainsString;
      found = true;
    } else if (cel_StringView_Equals(fn_name,
                                     cel_StringView_From("startsWith"))) {
      instr->kind = _cel_InstrKind_kCallStartsWithString;
      found = true;
    } else if (cel_StringView_Equals(fn_name,
                                     cel_StringView_From("endsWith"))) {
      instr->kind = _cel_InstrKind_kCallEndsWithString;
      found = true;
    } else if (cel_StringView_Equals(fn_name, cel_StringView_From("matches"))) {
      instr->kind = _cel_InstrKind_kCallRegexExpMatch;
      found = true;
    }
  }

  if (!found) {
    cel_InvalidArgumentStatusF(interp->status,
                               "cel: function with name " CEL_STRINGVIEW_FMT
                               " taking %d args not found",
                               CEL_STRINGVIEW_ARGS(fn_name),
                               instr->data.call.args);
    _cel_longjmp(interp->jmp);
  }

  if (instr->data.call.args == 0) {
    _cel_Interpreter_IncrementValueStackSize(interp);
  } else {
    for (size_t i = 1; i < instr->data.call.args; ++i) {
      _cel_Interpreter_DecrementValueStackSize(interp);
    }
  }
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_Interpreter_IsBind(
    const cel_ComprehensionExpr* cel_nonnull expr) {
  if (!cel_StringView_Equals(cel_ComprehensionExpr_IterVar(expr),
                             cel_StringView_From("#unused"))) {
    return false;
  }
  if (!cel_StringView_Empty(cel_ComprehensionExpr_IterVar2(expr))) {
    return false;
  }
  cel_StringView accu_var = cel_ComprehensionExpr_AccuVar(expr);
  if (cel_StringView_Empty(accu_var)) {
    return false;
  }
  const cel_Expr* iter_range = cel_ComprehensionExpr_IterRange(expr);
  if (iter_range == cel_nullptr ||
      cel_Expr_Kind(iter_range) != cel_ExprKind_kList) {
    return false;
  }
  const cel_Expr* loop_condition = cel_ComprehensionExpr_LoopCondition(expr);
  if (loop_condition == cel_nullptr ||
      cel_Expr_Kind(loop_condition) != cel_ExprKind_kConst) {
    return false;
  }
  const cel_Constant* loop_condition_constant =
      cel_ConstExpr_Value(cel_ConstExpr_DownCast(loop_condition));
  if (cel_Constant_Kind(loop_condition_constant) != cel_ConstantKind_kBool ||
      cel_Constant_GetBool(loop_condition_constant)) {
    return false;
  }
  const cel_Expr* loop_step = cel_ComprehensionExpr_LoopStep(expr);
  if (loop_step == cel_nullptr ||
      cel_Expr_Kind(loop_step) != cel_ExprKind_kIdent ||
      !cel_StringView_Equals(
          cel_IdentExpr_Name(cel_IdentExpr_DownCast(loop_step)), accu_var)) {
    return false;
  }
  return true;
}

static void _cel_Interpreter_PreVisitComprehensionExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_ComprehensionExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  if (cel_StringView_Empty(cel_ComprehensionExpr_IterVar(expr))) {
    cel_InvalidArgumentStatusF(
        interp->status, "cel: comprehension expr missing iter var: %" PRId64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  if (cel_ComprehensionExpr_IterRange(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(
        interp->status, "cel: comprehension expr missing iter range: %" PRId64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  if (cel_StringView_Empty(cel_ComprehensionExpr_AccuVar(expr))) {
    cel_InvalidArgumentStatusF(
        interp->status, "cel: comprehension expr missing accu var: %" PRId64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  if (cel_ComprehensionExpr_AccuInit(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(
        interp->status, "cel: comprehension expr missing iter range: %" PRId64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  if (cel_ComprehensionExpr_LoopCondition(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(
        interp->status,
        "cel: comprehension expr missing loop condition: %" PRId64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  if (cel_ComprehensionExpr_LoopStep(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(
        interp->status, "cel: comprehension expr missing loop step: %" PRId64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }
  if (cel_ComprehensionExpr_Result(expr) == cel_nullptr) {
    cel_InvalidArgumentStatusF(
        interp->status, "cel: comprehension expr missing result: %" PRId64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }

  // We only support cel.bind comprehensions currently.
  if (!_cel_Interpreter_IsBind(expr)) {
    cel_InvalidArgumentStatusF(
        interp->status,
        "cel: comprehension expr does not resemble cel.bind: %" PRId64,
        cel_Expr_Id(cel_Expr_UpCast(expr)));
    _cel_longjmp(interp->jmp);
  }

  // Keep track of the outer most cel.bind.
  if (interp->root_bind == cel_nullptr) {
    interp->root_bind = expr;
  }

  _cel_InterpreterSlot* slot =
      _cel_Interpreter_NewSlot(interp, cel_ComprehensionExpr_AccuVar(expr));
  _cel_Interpreter_PushSlot(interp, slot);
}

static void _cel_Interpreter_PostVisitComprehensionExpr(
    cel_AstVisitor* cel_nonnull visitor,
    const cel_ComprehensionExpr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_Interpreter_PopSlot(interp);
  if (interp->root_bind == expr) {
    _cel_Instr* lazy_enter = _cel_Interpreter_AppendInstr(interp);
    lazy_enter->kind = _cel_InstrKind_kLazyLeave;
    // At the moment only cel.bind comprehensions are implemented. So we always
    // start at slot 0 when clearing.
    lazy_enter->data.lazy_leave.slot = 0;
    lazy_enter->data.lazy_leave.num_slots =
        _cel_Interpreter_DeleteSlots(interp);
    interp->root_bind = cel_nullptr;
  }
}

static void _cel_Interpreter_PreVisitComprehensionExprIterRange(
    cel_AstVisitor* cel_nonnull visitor, const cel_Expr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  cel_AstTraverser_StepOut(interp->traverser);
}

static void _cel_Interpreter_PreVisitComprehensionExprAccuInit(
    cel_AstVisitor* cel_nonnull visitor, const cel_Expr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_InterpreterSlot* slot = _cel_Interpreter_TopSlot(interp);
  _cel_Instr* instr = _cel_Interpreter_AppendInstr(interp);
  instr->kind = _cel_InstrKind_kJump;
  instr->data.jump.jump = 0;
  slot->guard_pc = _cel_Interpreter_PC(interp, instr);
  slot->pc = _cel_Interpreter_NextPC(interp);
  _cel_Interpreter_PushValueStackBounds(interp, &slot->value_stack_bounds);
}

static void _cel_Interpreter_PostVisitComprehensionExprAccuInit(
    cel_AstVisitor* cel_nonnull visitor, const cel_Expr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_InterpreterSlot* slot = _cel_Interpreter_TopSlot(interp);
  _cel_Instr* lazy_return = _cel_Interpreter_AppendInstr(interp);
  lazy_return->kind = _cel_InstrKind_kLazyReturn;
  lazy_return->data.lazy_return.slot = slot->index;
  _cel_Instr* jump = _cel_Interpreter_InstrAt(interp, slot->guard_pc);
  CEL_ASSERT_EQ(jump->kind, _cel_InstrKind_kJump);
  jump->data.jump.jump = _cel_Interpreter_NextPC(interp) - slot->guard_pc;
  _cel_Interpreter_PopValueStackBounds(interp);
}

static void _cel_Interpreter_PreVisitComprehensionExprLoopCondition(
    cel_AstVisitor* cel_nonnull visitor, const cel_Expr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  cel_AstTraverser_StepOut(interp->traverser);
}

static void _cel_Interpreter_PreVisitComprehensionExprLoopStep(
    cel_AstVisitor* cel_nonnull visitor, const cel_Expr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  cel_AstTraverser_StepOut(interp->traverser);
}

static void _cel_Interpreter_PreVisitComprehensionExprResult(
    cel_AstVisitor* cel_nonnull visitor, const cel_Expr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_InterpreterSlot* slot = _cel_Interpreter_TopSlot(interp);
  _cel_FlatHashMapNode(interp->slot_map) prev_slot;
  prev_slot = _cel_FlatHashMap_Find(&interp->slot_map, &slot->name);
  if (prev_slot != cel_nullptr) {
    slot->prev_slot = (int32_t)prev_slot->val;
  }
  _cel_FlatHashMapMutableNode(interp->slot_map) slot_node;
  if (!_cel_FlatHashMap_Insert(&interp->slot_map, interp->alloc, &slot->name,
                               &slot_node)) {
    if (slot_node == cel_nullptr) {
      cel_OutOfMemoryStatus(interp->status);
      _cel_longjmp(interp->jmp);
    }
    CEL_ASSERT_NE(slot->prev_slot, -1);
  }
  slot_node->val = slot->index;
#ifndef NDEBUG
  _cel_Instr* lazy_enter = _cel_Interpreter_AppendInstr(interp);
  lazy_enter->kind = _cel_InstrKind_kLazyEnter;
  lazy_enter->data.lazy_enter.slot = slot->index;
#endif
}

static void _cel_Interpreter_PostVisitComprehensionExprResult(
    cel_AstVisitor* cel_nonnull visitor, const cel_Expr* cel_nonnull expr) {
  _cel_Interpreter* interp = _cel_Interpreter_FromAstVisitor(visitor);
  _cel_InterpreterSlot* slot = _cel_Interpreter_TopSlot(interp);
  _cel_FlatHashMapMutableNode(interp->slot_map) prev_slot;
  prev_slot = _cel_FlatHashMap_MutableFind(&interp->slot_map, &slot->name);
  CEL_ASSERT_NOT_NULL(prev_slot);
  if (slot->prev_slot != -1) {
    prev_slot->val = slot->prev_slot;
  } else {
    _cel_FlatHashMap_Erase(&interp->slot_map, prev_slot);
  }
}

static const cel_AstVisitorVTable _cel_InterpreterVTable = {
    .PreVisitExpr = &_cel_Interpreter_PreVisitExpr,
    .VisitUnspecifiedExpr = &_cel_Interpreter_VisitUnspecifiedExpr,
    .VisitIdentExpr = &_cel_Interpreter_VisitIdentExpr,
    .VisitConstExpr = &_cel_Interpreter_VisitConstExpr,
    .PreVisitSelectExpr = &_cel_Interpreter_PreVisitSelectExpr,
    .PostVisitSelectExpr = &_cel_Interpreter_PostVisitSelectExpr,
    .PreVisitBinaryExpr = &_cel_Interpreter_PreVisitBinaryExpr,
    .PostVisitBinaryExpr = &_cel_Interpreter_PostVisitBinaryExpr,
    .PostVisitBinaryExprLeft = &_cel_Interpreter_PostVisitBinaryExprLeft,
    .PreVisitUnaryExpr = &_cel_Interpreter_PreVisitUnaryExpr,
    .PostVisitUnaryExpr = &_cel_Interpreter_PostVisitUnaryExpr,
    .PreVisitTernaryExpr = &_cel_Interpreter_PreVisitTernaryExpr,
    .PostVisitTernaryExpr = &_cel_Interpreter_PostVisitTernaryExpr,
    .PostVisitCallExpr = &_cel_Interpreter_PostVisitCallExpr,
    .PreVisitCallArgExpr = &_cel_Interpreter_PreVisitCallArgExpr,
    .PostVisitTernaryExprCondition =
        &_cel_Interpreter_PostVisitTernaryExprCondition,
    .PostVisitTernaryExprIfTrue = &_cel_Interpreter_PostVisitTernaryExprIfTrue,
    .PreVisitListExpr = &_cel_Interpreter_PreVisitListExpr,
    .PostVisitListExpr = &_cel_Interpreter_PostVisitListExpr,
    .PreVisitListElementExpr = &_cel_Interpreter_PreVisitListElementExpr,
    .PostVisitListElementExpr = &_cel_Interpreter_PostVisitListElementExpr,
    .PreVisitMapExpr = &_cel_Interpreter_PreVisitMapExpr,
    .PostVisitMapExpr = &_cel_Interpreter_PostVisitMapExpr,
    .PreVisitMapEntryExpr = &_cel_Interpreter_PreVisitMapEntryExpr,
    .PostVisitMapEntryExprKey = &_cel_Interpreter_PostVisitMapEntryExprKey,
    .PostVisitMapEntryExprValue = &_cel_Interpreter_PostVisitMapEntryExprValue,
    .PreVisitComprehensionExpr = &_cel_Interpreter_PreVisitComprehensionExpr,
    .PostVisitComprehensionExpr = &_cel_Interpreter_PostVisitComprehensionExpr,
    .PreVisitComprehensionExprIterRange =
        &_cel_Interpreter_PreVisitComprehensionExprIterRange,
    .PreVisitComprehensionExprAccuInit =
        &_cel_Interpreter_PreVisitComprehensionExprAccuInit,
    .PostVisitComprehensionExprAccuInit =
        &_cel_Interpreter_PostVisitComprehensionExprAccuInit,
    .PreVisitComprehensionExprLoopCondition =
        &_cel_Interpreter_PreVisitComprehensionExprLoopCondition,
    .PreVisitComprehensionExprLoopStep =
        &_cel_Interpreter_PreVisitComprehensionExprLoopStep,
    .PreVisitComprehensionExprResult =
        &_cel_Interpreter_PreVisitComprehensionExprResult,
    .PostVisitComprehensionExprResult =
        &_cel_Interpreter_PostVisitComprehensionExprResult,
};

static size_t _cel_Interpreter_StringViewHasher(const void* cel_nonnull key) {
  return cel_HashState_Finalize(cel_HashState_Combine(
      cel_HashState_Initialize(), *(const cel_StringView*)key));
}

static bool _cel_Interpreter_StringViewEqualer(const void* cel_nonnull lhs,
                                               const void* cel_nonnull rhs) {
  return cel_StringView_Equals(*(const cel_StringView*)lhs,
                               *(const cel_StringView*)rhs);
}

static void _cel_Interpreter_Construct(_cel_Interpreter* cel_nonnull interp,
                                       const cel_Runtime* cel_nonnull rt,
                                       cel_Status* cel_nonnull status) {
  memset(interp, 0, sizeof(*interp));
  interp->visitor.vtable = &_cel_InterpreterVTable;
  interp->rt = _cel_Runtime_ConstRef(rt);
  interp->alloc = rt->alloc;
  interp->def_pool = rt->def_pool;
  interp->wkts = &rt->wkts;
  interp->status = status;
  interp->root_bind = cel_nullptr;
  _cel_FlatHashMap_Construct(&interp->string_pool_indices,
                             &_cel_Interpreter_StringViewHasher,
                             &_cel_Interpreter_StringViewEqualer);
  _cel_FlatHashMap_Construct(&interp->candidate_names_indices,
                             &_cel_Interpreter_StringViewHasher,
                             &_cel_Interpreter_StringViewEqualer);
  _cel_InterpreterNspaceVar_Construct(&interp->nspace_var);
  _cel_Array_Construct(&interp->conds);
  _cel_Array_Construct(&interp->lists);
  _cel_Array_Construct(&interp->maps);
  _cel_Deque_Construct(&interp->slots);
  _cel_FlatHashMap_Construct(&interp->slot_map,
                             &_cel_Interpreter_StringViewHasher,
                             &_cel_Interpreter_StringViewEqualer);
  _cel_Array_Construct(&interp->slot_stack);
  _cel_Array_Construct(&interp->value_stack_bounds_stack);
}

static void _cel_Interpreter_Destruct(_cel_Interpreter* cel_nonnull interp) {
  cel_AstTraverser_Delete(interp->traverser);
  _cel_Array_Destruct(&interp->value_stack_bounds_stack, interp->alloc);
  _cel_Array_Destruct(&interp->slot_stack, interp->alloc);
  _cel_FlatHashMap_Destruct(&interp->slot_map, interp->alloc);
  _cel_Deque_Destruct(&interp->slots, interp->alloc);
  _cel_Array_Destruct(&interp->conds, interp->alloc);
  for (size_t i = 0; i < _cel_Array_Size(&interp->lists); ++i) {
    _cel_Array_Destruct(&_cel_Array_MutableAt(&interp->lists, i)->pcs,
                        interp->alloc);
  }
  _cel_Array_Destruct(&interp->lists, interp->alloc);
  for (size_t i = 0; i < _cel_Array_Size(&interp->maps); ++i) {
    _cel_Array_Destruct(&_cel_Array_MutableAt(&interp->maps, i)->pcs,
                        interp->alloc);
  }
  _cel_Array_Destruct(&interp->maps, interp->alloc);
  _cel_InterpreterNspaceVar_Destruct(&interp->nspace_var, interp->alloc);
  _cel_FlatHashMap_Destruct(&interp->candidate_names_indices, interp->alloc);
  _cel_FlatHashMap_Destruct(&interp->string_pool_indices, interp->alloc);
  // interp->prog is only non-null when planning failed, call cel_Program_Delete
  // to avoid hitting a debug assert as we hold the only reference
  cel_Program_Delete(interp->prog);
  _cel_Runtime_Unref(interp->rt);
}

static void _cel_Interpreter_DirectInstrs(
    _cel_Interpreter* cel_nonnull interp) {
  const size_t instr_len = _cel_Array_Size(&interp->prog->instrs);
  _cel_Instr* instr_ptr = _cel_Array_MutableData(&interp->prog->instrs);
  for (size_t i = 0; i < instr_len; ++instr_ptr, ++i) {
    switch (instr_ptr->kind) {
      case _cel_InstrKind_kIdent:
        instr_ptr->data.ident.name.direct =
            _cel_PackedStringView_FromStringView(
                _cel_Interpreter_InternedString(
                    interp, instr_ptr->data.ident.name.indirect));
        break;
      case _cel_InstrKind_kBytesConst:
        instr_ptr->data.bytes_const.value.direct =
            _cel_PackedStringView_FromStringView(
                _cel_Interpreter_InternedString(
                    interp, instr_ptr->data.bytes_const.value.indirect));
        break;
      case _cel_InstrKind_kStringConst:
        instr_ptr->data.string_const.value.direct =
            _cel_PackedStringView_FromStringView(
                _cel_Interpreter_InternedString(
                    interp, instr_ptr->data.string_const.value.indirect));
        break;
      case _cel_InstrKind_kHas:
        instr_ptr->data.has.field.direct = _cel_PackedStringView_FromStringView(
            _cel_Interpreter_InternedString(
                interp, instr_ptr->data.has.field.indirect));
        break;
      case _cel_InstrKind_kSelect:
        instr_ptr->data.select.field.direct =
            _cel_PackedStringView_FromStringView(
                _cel_Interpreter_InternedString(
                    interp, instr_ptr->data.select.field.indirect));
        break;
      default:
        break;
    }
  }
}

cel_Program* cel_nullable _cel_Interpreter_Compile(
    const cel_Runtime* cel_nonnull rt, const cel_Ast* cel_nonnull ast,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(rt);
  CEL_ASSERT_NOT_NULL(ast);
  CEL_ASSERT(cel_Status_Ok(status));

  if (cel_Ast_Expr(ast) == cel_nullptr) {
    cel_InvalidArgumentStatus(status, cel_StringView_From("cel: AST is empty"));
    return cel_nullptr;
  }

  _cel_Interpreter interp;
  _cel_Interpreter* volatile interp_ptr = &interp;
  _cel_Interpreter_Construct(&interp, rt, status);

  cel_Program* prog = _cel_Program_New(rt);
  if (CEL_UNLIKELY(prog == cel_nullptr)) {
    cel_OutOfMemoryStatus(status);
    _cel_Interpreter_Destruct(&interp);
    return cel_nullptr;
  }
  interp.prog = prog;

  cel_AstTraverser* traverser = cel_AstTraverser_New(ast, &interp.visitor);
  if (CEL_UNLIKELY(traverser == cel_nullptr)) {
    cel_OutOfMemoryStatus(status);
    _cel_Interpreter_Destruct(&interp);
    return cel_nullptr;
  }
  interp.traverser = traverser;
  if (_cel_setjmp(interp_ptr->jmp)) {
    CEL_ASSERT_NOT(cel_Status_Ok(interp_ptr->status));
    _cel_Interpreter_Destruct(&interp);
    return cel_nullptr;
  }

  _cel_Interpreter_PushValueStackBounds(interp_ptr,
                                        &interp_ptr->value_stack_bounds);

  while (cel_AstTraverser_Traverse(traverser, status) &&
         cel_Status_Ok(status)) {
  }
  if (cel_Status_Ok(status)) {
    prog->max_stack_size = interp.value_stack_bounds.max;
    _cel_Instr* instr = _cel_Interpreter_AppendInstr(&interp);
    if (CEL_UNLIKELY(instr == cel_nullptr)) {
      _cel_Interpreter_Destruct(&interp);
      return cel_nullptr;
    }
    instr->kind = _cel_InstrKind_kExit;
    _cel_Array_ShrinkToFit(&prog->instrs, interp.alloc);
    _cel_Array_ShrinkToFit(&prog->strings_table, interp.alloc);
    _cel_Array_ShrinkToFit(&prog->candidate_names_table, interp.alloc);
    // We only needed pointer stability for strings in the string pool with
    // _cel_Interpreter. Now that planning is complete, we can forgo pointer
    // stability in the name of less memory usage.
    _cel_String* string_pool_ptr = _cel_Array_MutableData(&prog->strings_table);
    size_t string_pool_len = _cel_Array_Size(&prog->strings_table);
    for (; string_pool_len > 0; ++string_pool_ptr, --string_pool_len) {
      _cel_String_Destabilize(string_pool_ptr, interp.alloc);
    }
    // Now that we have destablized strings, we can convert instructions from
    // their indirect references to direct.
    _cel_Interpreter_DirectInstrs(&interp);

    interp.prog = cel_nullptr;
  } else {
    prog = cel_nullptr;
  }
  _cel_Interpreter_Destruct(&interp);
  return prog;
}
