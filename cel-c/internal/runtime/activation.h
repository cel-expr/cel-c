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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_RUNTIME_ACTIVATION_H_
#define THIRD_PARTY_CEL_C_INTERNAL_RUNTIME_ACTIVATION_H_

#include <stddef.h>
#include <stdint.h>

#include "cel-c/activation.h"  // IWYU pragma: export
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/runtime/instr.h"
#include "cel-c/internal/runtime/interpretable.h"
#include "cel-c/program.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/trilean.h"
#include "cel-c/value.h"

CEL_BEGIN_DECLS

struct cel_Activation {
  _cel_Interpretable interp;
  size_t size;
  const cel_Program* cel_nonnull prog;
  const cel_VariableResolver* cel_nonnull var_resolver;
  cel_Value* cel_nonnull value_stack_base;
  cel_Value* cel_nonnull value_stack_end;
  _cel_InterpretableSlot* cel_nonnull slots;
  const _cel_Instr * cel_nonnull * cel_nonnull lazy_stack_base;
  const _cel_Instr * cel_nonnull * cel_nonnull lazy_stack_end;
};

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
cel_Activation* cel_nullable
_cel_Activation_New(const cel_Program* cel_nonnull prog,
                    const cel_VariableResolver* cel_nonnull var_resolver);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Trilean _cel_Activation_FindVariable(
    const cel_Activation* cel_nonnull activation, cel_StringView name,
    cel_Value* cel_nonnull result, cel_Arena* cel_nonnull arena,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(activation);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT(cel_Status_Ok(status));

  return (*activation->var_resolver->vtable->Find)(activation->var_resolver,
                                                   name, result, arena, status);
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_RUNTIME_ACTIVATION_H_
