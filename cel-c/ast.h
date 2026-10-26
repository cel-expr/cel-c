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

#ifndef THIRD_PARTY_CEL_C_AST_H_
#define THIRD_PARTY_CEL_C_AST_H_

#include "cel-c/config.h"

CEL_BEGIN_DECLS

typedef struct cel_Ast cel_Ast;

// cel_Ast

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Ast_Delete(CEL_NULLABLE(cel_Ast*) ast);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_AST_H_
