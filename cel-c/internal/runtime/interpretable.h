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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_RUNTIME_INTERPRETABLE_H_
#define THIRD_PARTY_CEL_C_INTERNAL_RUNTIME_INTERPRETABLE_H_

#include <stdbool.h>
#include <stddef.h>

#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/runtime/instr.h"
#include "cel-c/internal/runtime/program.h"
#include "cel-c/internal/setjmp.h"
#include "cel-c/internal/string.h"
#include "cel-c/status.h"
#include "cel-c/value.h"
#include "cel-c/well_known_types.h"
#include "upb/reflection/def.h"

CEL_BEGIN_DECLS

typedef struct {
  cel_Value value;
  bool active;
} _cel_InterpretableSlot;

typedef struct {
  cel_Allocator* cel_nonnull alloc;
  cel_Arena* cel_nonnull arena;
  const upb_DefPool* cel_nonnull def_pool;
  const cel_WellKnownTypes* cel_nonnull wkts;
  cel_Status* cel_nonnull status;

  const _cel_Instr* cel_nonnull instr_ptr;
  size_t instr_len;

  const _cel_String* cel_nullable strings_table_ptr;
  size_t strings_table_len;

  const _cel_CandidateNames* cel_nullable candidate_names_ptr;
  size_t candidate_names_len;

  cel_Value* cel_nonnull value_stack_base;
  cel_Value* cel_nonnull value_stack_top;
  cel_Value* cel_nonnull value_stack_end;

  _cel_InterpretableSlot* cel_nonnull slots;
  size_t slots_len;

  const _cel_Instr * cel_nonnull * cel_nonnull lazy_stack_base;
  const _cel_Instr * cel_nonnull * cel_nonnull lazy_stack_top;
  const _cel_Instr * cel_nonnull * cel_nonnull lazy_stack_end;

  cel_ValueContext context;

  _cel_jmp_buf jmp;
} _cel_Interpretable;

CEL_ATTRIBUTE_NOTHROW
void _cel_Interpretable_Initialize(
    _cel_Interpretable* cel_nonnull interp, const cel_Program* cel_nonnull prog,
    cel_Value* cel_nonnull value_stack_base, size_t value_stack_size,
    _cel_InterpretableSlot* cel_nonnull slots, size_t num_slots,
    const _cel_Instr * cel_nonnull * cel_nonnull lazy_stack_base,
    size_t lazy_stack_size);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_Interpretable_Execute(_cel_Interpretable* cel_nonnull interp,
                                cel_Value* cel_nonnull result);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_Interpretable_Interpret(
    _cel_Interpretable* cel_nonnull interp, cel_Value* cel_nonnull result,
    cel_Arena* cel_nonnull arena, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT(cel_Status_Ok(status));
  CEL_ASSERT(interp->value_stack_top == interp->value_stack_base);

  interp->arena = interp->context.arena = arena;
  interp->status = status;
  return _cel_Interpretable_Execute(interp, result);
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_RUNTIME_INTERPRETABLE_H_
