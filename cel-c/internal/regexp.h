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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_REGEXP_H_
#define THIRD_PARTY_CEL_C_INTERNAL_REGEXP_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cel-c/config.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"

CEL_BEGIN_DECLS

typedef struct _cel_RegExpOptions {
  int64_t max_mem;
  bool posix_syntax;
  bool longest_match;
  bool log_errors;
  bool literal;
  bool never_nl;
  bool dot_nl;
  bool never_capture;
  bool case_sensitive;
  bool perl_classes;
  bool word_boundary;
  bool one_line;
} _cel_RegExpOptions;

CEL_ATTRIBUTE_NOTHROW
void _cel_RegExpOptions_Construct(_cel_RegExpOptions* cel_nonnull options);

typedef struct _cel_RegExp _cel_RegExp;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
_cel_RegExp* cel_nullable _cel_RegExp_New(
    cel_StringView pattern, const _cel_RegExpOptions* cel_nullable options,
    cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NOTHROW
void _cel_RegExp_Delete(_cel_RegExp* cel_nullable regexp);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_RegExp_FullMatch(const _cel_RegExp* cel_nonnull regexp,
                           cel_StringView subject,
                           cel_Status* cel_nonnull status, size_t argc, ...);

#define _cel_RegExp_FullMatch(regexp, subject, status, ...) \
  _cel_RegExp_FullMatch((regexp), (subject), (status),      \
                        _CEL_NARGS(__VA_ARGS__), ##__VA_ARGS__)

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_RegExp_PartialMatch(const _cel_RegExp* cel_nonnull regexp,
                              cel_StringView subject,
                              cel_Status* cel_nonnull status, size_t argc, ...);

#define _cel_RegExp_PartialMatch(regexp, subject, status, ...) \
  _cel_RegExp_PartialMatch((regexp), (subject), (status),      \
                           _CEL_NARGS(__VA_ARGS__), ##__VA_ARGS__)

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_RegExp_Matches(cel_StringView pattern,
                         const _cel_RegExpOptions* cel_nullable options,
                         cel_StringView subject,
                         cel_Status* cel_nonnull status);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_REGEXP_H_
