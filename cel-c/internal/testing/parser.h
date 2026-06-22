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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_TESTING_PARSER_H_
#define THIRD_PARTY_CEL_C_INTERNAL_TESTING_PARSER_H_

#include "cel-c/arena.h"
#include "cel-c/ast.h"
#include "cel-c/internal/config.h"
#include "cel-c/string_view.h"

#ifdef __cplusplus
#include <memory>
#endif

CEL_BEGIN_DECLS

typedef struct _cel_TestingParser _cel_TestingParser;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
_cel_TestingParser* cel_nonnull _cel_TestingParser_New();

CEL_ATTRIBUTE_NOTHROW
void _cel_TestingParser_Delete(_cel_TestingParser* cel_nonnull parser);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
cel_Ast* cel_nonnull
_cel_TestingParser_Parse(_cel_TestingParser* cel_nonnull parser,
                         cel_StringView content, cel_Arena* cel_nonnull arena);

CEL_END_DECLS

#ifdef __cplusplus
struct _cel_TestingParserDeleter {
  void operator()(
      const _cel_TestingParser* cel_nullable parser) const noexcept {
    if (parser != nullptr) {
      _cel_TestingParser_Delete(const_cast<_cel_TestingParser*>(parser));
    }
  }
};

using _cel_TestingParserPtr =
    std::unique_ptr<_cel_TestingParser, _cel_TestingParserDeleter>;
#endif

#endif  // THIRD_PARTY_CEL_C_INTERNAL_TESTING_PARSER_H_
