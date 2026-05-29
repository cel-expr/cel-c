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

#include "cel-c/src/runtime/activation.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/trilean.h"
#include "cel-c/value.h"
#include "cel-c/src/runtime/instr.h"
#include "cel-c/src/runtime/interpretable.h"
#include "cel-c/src/runtime/program.h"
#include "cel-c/src/runtime/runtime.h"

cel_ActivationOptions* cel_nonnull
cel_ActivationOptions_Default(cel_ActivationOptions* cel_nonnull opts) {
  CEL_ASSERT_NOT_NULL(opts);

  memset(opts, 0, sizeof(*opts));
  return opts;
}

void cel_Activation_Delete(cel_Activation* cel_nullable activation) {
  if (activation == cel_nullptr) {
    return;
  }
  const cel_Program* prog = activation->prog;
  cel_Allocator_FreeSized(prog->rt->alloc, activation, activation->size);
  _cel_Program_Unref(prog);
}

cel_Activation* cel_nullable
_cel_Activation_New(const cel_Program* cel_nonnull prog,
                    const cel_VariableResolver* cel_nonnull var_resolver) {
  CEL_ASSERT_NOT_NULL(prog);
  CEL_ASSERT_NOT_NULL(var_resolver);

  // Allocate the activation, value stack, slots, and call stack in a single
  // allocation.
  size_t size = sizeof(cel_Activation) +
                (sizeof(cel_Value) * prog->max_stack_size) +
                (sizeof(_cel_InterpretableSlot) * prog->max_slot_size) +
                (sizeof(_cel_Instr*) * prog->max_slot_size);
  char* addr = (char*)cel_Allocator_Malloc(prog->rt->alloc, size, cel_nullptr);
  if (addr == cel_nullptr) {
    return cel_nullptr;
  }
  memset(addr, 0, sizeof(cel_Activation));
  cel_Activation* act = (cel_Activation*)addr;
  act->size = size;
  act->prog = _cel_Program_ConstRef(prog);
  act->var_resolver = var_resolver;
  act->value_stack_base = (cel_Value*)(addr + sizeof(cel_Activation));
  act->value_stack_end = act->value_stack_base + prog->max_stack_size;
  act->slots = (_cel_InterpretableSlot*)act->value_stack_end;
  act->lazy_stack_base = (const _cel_Instr**)(act->slots + prog->max_slot_size);
  act->lazy_stack_end = act->lazy_stack_base + prog->max_slot_size;
  memset(act->slots, 0, sizeof(_cel_InterpretableSlot) * prog->max_slot_size);
  _cel_Interpretable_Initialize(&act->interp, prog, act->value_stack_base,
                                act->value_stack_end - act->value_stack_base,
                                act->slots, act->prog->max_slot_size,
                                act->lazy_stack_base,
                                act->lazy_stack_end - act->lazy_stack_base);
  return act;
}

cel_Trilean cel_Activation_FindVariable(
    const cel_Activation* cel_nonnull activation, cel_StringView name,
    cel_Value* cel_nonnull result, cel_Arena* cel_nonnull arena,
    cel_Status* cel_nonnull status) {
  return _cel_Activation_FindVariable(activation, name, result, arena, status);
}

bool cel_Activation_Execute(cel_Activation* cel_nonnull activation,
                            cel_Value* cel_nonnull result,
                            cel_Arena* cel_nonnull arena,
                            cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(activation);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT(cel_Status_Ok(status));

  return _cel_Interpretable_Interpret(&activation->interp, result, arena,
                                      status);
}
