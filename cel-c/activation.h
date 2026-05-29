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

#ifndef THIRD_PARTY_CEL_C_ACTIVATION_H_
#define THIRD_PARTY_CEL_C_ACTIVATION_H_

#include "cel-c/arena.h"
#include "cel-c/config.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/trilean.h"
#include "cel-c/value.h"

CEL_BEGIN_DECLS

typedef struct cel_VariableResolverVTable cel_VariableResolverVTable;
typedef struct cel_VariableResolver cel_VariableResolver;

typedef cel_Trilean cel_VariableResolverVTable_Find(
    const cel_VariableResolver* cel_nonnull resolver, cel_StringView name,
    cel_Value* cel_nonnull value, cel_Arena* cel_nonnull arena,
    cel_Status* cel_nonnull status);

struct cel_VariableResolverVTable {
  // NOLINTBEGIN(google3-readability-class-member-naming)
  cel_VariableResolverVTable_Find* cel_nonnull Find;
  // NOLINTEND(google3-readability-class-member-naming)
};

struct cel_VariableResolver {
  const cel_VariableResolverVTable* cel_nonnull vtable;
};

typedef struct {
} cel_ActivationOptions;

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_ActivationOptions* cel_nonnull
cel_ActivationOptions_Default(cel_ActivationOptions* cel_nonnull opts);

// cel_Activation
//
// A single active instance of a given program, which can be executed.
//
// Created via `cel_Program_Activate`.
typedef struct cel_Activation cel_Activation;

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Activation_Delete(cel_Activation* cel_nullable activation);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_Trilean cel_Activation_FindVariable(
    const cel_Activation* cel_nonnull activation, cel_StringView name,
    cel_Value* cel_nonnull result, cel_Arena* cel_nonnull arena,
    cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Activation_Execute(cel_Activation* cel_nonnull activation,
                                       cel_Value* cel_nonnull result,
                                       cel_Arena* cel_nonnull arena,
                                       cel_Status* cel_nonnull status);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_ACTIVATION_H_
