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

#ifndef THIRD_PARTY_CEL_C_AST_PROTO_H_
#define THIRD_PARTY_CEL_C_AST_PROTO_H_

#include "cel/expr/checked.upb.h"
#include "cel-c/arena.h"
#include "cel-c/ast.h"
#include "cel-c/config.h"
#include "cel-c/status.h"

CEL_BEGIN_DECLS

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_Ast*)
    cel_Ast_FromProto(CEL_NONNULL(const cel_expr_CheckedExpr*) in,
                      CEL_NONNULL(cel_Arena*) arena,
                      CEL_NONNULL(cel_Status*) status);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_AST_PROTO_H_
