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

#ifndef THIRD_PARTY_CEL_C_RUNTIME_H_
#define THIRD_PARTY_CEL_C_RUNTIME_H_

#include <stdbool.h>

#include "cel-c/alloc.h"
#include "cel-c/ast.h"
#include "cel-c/config.h"
#include "cel-c/program.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/well_known_types.h"
#include "upb/reflection/def.h"

CEL_BEGIN_DECLS

typedef struct {
  cel_StringView container;
} cel_RuntimeOptions;

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_RuntimeOptions* cel_nonnull
cel_RuntimeOptions_Default(cel_RuntimeOptions* cel_nonnull opts);

typedef struct cel_Runtime cel_Runtime;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_Runtime* cel_nullable cel_Runtime_New(
    cel_Allocator* cel_nonnull alloc, const upb_DefPool* cel_nonnull def_pool,
    const cel_RuntimeOptions* cel_nullable opts,
    cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Runtime_Delete(cel_Runtime* cel_nullable rt);

CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
CEL_EXTERN cel_Allocator* cel_nonnull
cel_Runtime_Allocator(const cel_Runtime* cel_nonnull rt);

CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
CEL_EXTERN const cel_RuntimeOptions* cel_nonnull
cel_Runtime_Options(const cel_Runtime* cel_nonnull rt);

CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
CEL_EXTERN const upb_DefPool* cel_nonnull
cel_Runtime_DefPool(const cel_Runtime* cel_nonnull rt);

CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
CEL_EXTERN const cel_WellKnownTypes* cel_nonnull
cel_Runtime_WellKnownTypes(const cel_Runtime* cel_nonnull rt);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_Program* cel_nullable cel_Runtime_Compile(
    const cel_Runtime* cel_nonnull runtime, const cel_Ast* cel_nonnull ast,
    const cel_ProgramOptions* cel_nullable opts,
    cel_Status* cel_nonnull status);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_RUNTIME_H_
