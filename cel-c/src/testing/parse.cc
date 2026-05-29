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

#include "cel-c/src/testing/parse.h"

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/ast.h"
#include "cel-c/config.h"
#include "cel-c/src/testing/parser.h"
#include "cel-c/string_view.h"

extern "C" CEL_ATTRIBUTE_NOTHROW cel_Ast* cel_nonnull
_cel_TestingParse(cel_StringView content, cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(arena);

  _cel_TestingParserPtr parser(_cel_TestingParser_New());
  return _cel_TestingParser_Parse(parser.get(), content, arena);
}
