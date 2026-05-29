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

#ifndef THIRD_PARTY_CEL_C_PROGRAM_H_
#define THIRD_PARTY_CEL_C_PROGRAM_H_

#include <stdbool.h>

#include "cel-c/activation.h"
#include "cel-c/config.h"

CEL_BEGIN_DECLS

typedef struct {
} cel_ProgramOptions;

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_ProgramOptions* cel_nonnull
cel_ProgramOptions_Default(cel_ProgramOptions* cel_nonnull opts);

// cel_Program
//
// Created via `cel_Runtime_Compile`.
typedef struct cel_Program cel_Program;

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Program_Delete(cel_Program* cel_nullable program);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_Activation* cel_nullable
cel_Program_Activate(const cel_Program* cel_nonnull program,
                     const cel_VariableResolver* cel_nonnull var_resolver,
                     const cel_ActivationOptions* cel_nullable opts);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_PROGRAM_H_
