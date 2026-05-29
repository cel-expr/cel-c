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

#ifndef THIRD_PARTY_CEL_C_AST_TRAVERSE_H_
#define THIRD_PARTY_CEL_C_AST_TRAVERSE_H_

#include <stdbool.h>  // IWYU pragma: keep

#include "cel-c/ast.h"
#include "cel-c/ast_visitor.h"
#include "cel-c/config.h"
#include "cel-c/status.h"

CEL_BEGIN_DECLS

typedef struct cel_AstTraverser cel_AstTraverser;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NULLABLE(cel_AstTraverser*)
    cel_AstTraverser_New(CEL_NONNULL(const cel_Ast*) ast,
                         CEL_NONNULL(cel_AstVisitor*) visitor);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_AstTraverser_Delete(CEL_NULLABLE(cel_AstTraverser*)
                                            ast_traverser);

// Informs the traverser to continue traversing child nodes of the current node
// being visited. This is the default and does not need to be called unless to
// undo a call to `cel_AstTraverser_StepOut` over `cel_AstTraverser_StepOver`.
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_AstTraverser_StepIn(
    cel_AstTraverser* cel_nonnull ast_traverser);

// Informs the traverser to skip traversing child nodes of the current node
// being visited. If called during a PreVisit called, also skips calling the
// corresponding PostVisit.
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_AstTraverser_StepOut(
    cel_AstTraverser* cel_nonnull ast_traverser);

// Informs the traverser to skip traversing child nodes of the current node
// being visited.
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_AstTraverser_StepOver(
    cel_AstTraverser* cel_nonnull ast_traverser);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_AstTraverser_Traverse(CEL_NONNULL(cel_AstTraverser*)
                                              ast_traverser,
                                          CEL_NONNULL(cel_Status*) status);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_AST_TRAVERSE_H_
