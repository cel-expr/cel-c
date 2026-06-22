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

#include "cel-c/internal/regexp.h"

#include <cstdarg>
#include <cstddef>
#include <new>
#include <type_traits>

#include "absl/strings/string_view.h"
#include "cel-c/assert.h"
#include "cel-c/internal/alloca.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/malloc.h"
#include "cel-c/status.h"
#include "cel-c/status_code.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"
#include "re2/re2.h"

CEL_BEGIN_DECLS

struct _cel_RegExp {
  alignas(RE2) std::byte re2[sizeof(RE2)];
};

CEL_END_DECLS

CEL_STATIC_ASSERT(alignof(_cel_RegExp) <= cel_kMaxAlign);

namespace {

struct RE2CArg final {
  RE2CArg() : string(), arg(&string) {}

  RE2CArg(const RE2CArg&) = delete;
  RE2CArg(RE2CArg&&) = delete;
  RE2CArg& operator=(const RE2CArg&) = delete;
  RE2CArg& operator=(RE2CArg&&) = delete;

  absl::string_view string;
  RE2::Arg arg;
};

static_assert(std::is_trivially_destructible_v<RE2CArg>);
static_assert(alignof(RE2CArg) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__);

using MatchFunction = bool (*)(absl::string_view, const RE2&,
                               const RE2::Arg* const*, int);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOINLINE
bool _cel_RegExp_Match(const RE2& re2, absl::string_view subject,
                       MatchFunction cel_nonnull match,
                       cel_Status* cel_nonnull status, size_t argc,
                       va_list argv) {
  CEL_ASSERT_NOT_NULL(match);
  CEL_ASSERT_NOT_NULL(status);
  CEL_ASSERT_LT(argc, 64);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  char* args_data;
  RE2CArg* re2_cargs;
  RE2::Arg** re2_args;
  if (argc > 0) {
    const size_t re2_args_offset = (sizeof(RE2CArg) * argc);
    args_data = reinterpret_cast<char*>(
        _cel_alloca(re2_args_offset + (sizeof(RE2::Arg*) * argc)));
    re2_cargs = std::launder(reinterpret_cast<RE2CArg*>(args_data));
    re2_args =
        std::launder(reinterpret_cast<RE2::Arg**>(args_data + re2_args_offset));
    for (size_t i = 0; i < argc; ++i) {
      re2_args[i] = &(::new (static_cast<void*>(re2_cargs + i)) RE2CArg())->arg;
    }
  } else {
    args_data = nullptr;
    re2_cargs = nullptr;
    re2_args = nullptr;
  }
  if (!(*match)(subject, re2, re2_args, argc)) {
    return false;
  }
  for (size_t i = 0; i < argc; ++i) {
    cel_StringView* arg = va_arg(argv, cel_StringView*);
    CEL_ASSERT_NOT_NULL(arg);
    *arg = cel_StringView_FromAbsl(re2_cargs[i].string);
  }
  return true;
}

RE2::Options _cel_RegExpOptions_ToRe2(
    const _cel_RegExpOptions* cel_nullable options) {
  RE2::Options re2_options;
  re2_options.set_encoding(RE2::Options::EncodingUTF8);
  if (options != nullptr) {
    re2_options.set_max_mem(options->max_mem);
    re2_options.set_posix_syntax(options->posix_syntax);
    re2_options.set_longest_match(options->longest_match);
    re2_options.set_log_errors(options->log_errors);
    re2_options.set_literal(options->literal);
    re2_options.set_never_nl(options->never_nl);
    re2_options.set_dot_nl(options->dot_nl);
    re2_options.set_never_capture(options->never_capture);
    re2_options.set_case_sensitive(options->case_sensitive);
    re2_options.set_perl_classes(options->perl_classes);
    re2_options.set_word_boundary(options->word_boundary);
    re2_options.set_one_line(options->one_line);
  }
  return re2_options;
}

}  // namespace

extern "C" CEL_ATTRIBUTE_NOTHROW void _cel_RegExpOptions_Construct(
    _cel_RegExpOptions* cel_nonnull options) {
  CEL_ASSERT_NOT_NULL(options);

  RE2::Options re2_options;
  options->max_mem = re2_options.max_mem();
  options->posix_syntax = re2_options.posix_syntax();
  options->longest_match = re2_options.longest_match();
  options->log_errors = re2_options.log_errors();
  options->literal = re2_options.literal();
  options->never_nl = re2_options.never_nl();
  options->dot_nl = re2_options.dot_nl();
  options->never_capture = re2_options.never_capture();
  options->case_sensitive = re2_options.case_sensitive();
  options->perl_classes = re2_options.perl_classes();
  options->word_boundary = re2_options.word_boundary();
  options->one_line = re2_options.one_line();
}

extern "C" CEL_ATTRIBUTE_NOTHROW _cel_RegExp* cel_nullable _cel_RegExp_New(
    cel_StringView pattern, const _cel_RegExpOptions* cel_nullable options,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return nullptr;
  }

  _cel_RegExp* regexp =
      reinterpret_cast<_cel_RegExp*>(_cel_Malloc(sizeof(_cel_RegExp), nullptr));
  if (CEL_UNLIKELY(regexp == nullptr)) {
    return nullptr;
  }
  RE2::Options re2_options = _cel_RegExpOptions_ToRe2(options);
  RE2* re2 = ::new (static_cast<void*>(&regexp->re2[0]))
      RE2(cel_StringView_ToAbsl(pattern), re2_options);
  if (CEL_UNLIKELY(!re2->ok())) {
    cel_StatusCode code = re2->error_code() != RE2::ErrorPatternTooLarge
                              ? cel_StatusCode_kInvalidArgument
                              : cel_StatusCode_kResourceExhausted;
    cel_CanonicalStatus(status, code,
                        cel_StringView_FromString(re2->error().c_str()));
    re2->~RE2();
    _cel_FreeSized(regexp, sizeof(*regexp));
    return nullptr;
  }
  return regexp;
}

extern "C" CEL_ATTRIBUTE_NOTHROW void _cel_RegExp_Delete(
    _cel_RegExp* cel_nullable regexp) {
  if (regexp == nullptr) {
    return;
  }
  std::launder(reinterpret_cast<RE2*>(&regexp->re2[0]))->~RE2();
  _cel_FreeSized(regexp, sizeof(*regexp));
}

#undef _cel_RegExp_FullMatch

extern "C" CEL_ATTRIBUTE_NOTHROW bool _cel_RegExp_FullMatch(
    const _cel_RegExp* cel_nonnull regexp, cel_StringView subject,
    cel_Status* cel_nonnull status, size_t argc, ...) {
  CEL_ASSERT_NOT_NULL(regexp);

  va_list argv;
  va_start(argv, argc);
  const bool match = _cel_RegExp_Match(
      *std::launder(reinterpret_cast<const RE2*>(&regexp->re2[0])),
      cel_StringView_ToAbsl(subject), &RE2::FullMatchN, status, argc, argv);
  va_end(argv);
  return match;
}

#undef _cel_RegExp_PartialMatch

extern "C" CEL_ATTRIBUTE_NOTHROW bool _cel_RegExp_PartialMatch(
    const _cel_RegExp* cel_nonnull regexp, cel_StringView subject,
    cel_Status* cel_nonnull status, size_t argc, ...) {
  CEL_ASSERT_NOT_NULL(regexp);

  va_list argv;
  va_start(argv, argc);
  const bool match = _cel_RegExp_Match(
      *std::launder(reinterpret_cast<const RE2*>(&regexp->re2[0])),
      cel_StringView_ToAbsl(subject), &RE2::PartialMatchN, status, argc, argv);
  va_end(argv);
  return match;
}

extern "C" CEL_ATTRIBUTE_NOTHROW bool _cel_RegExp_Matches(
    cel_StringView pattern, const _cel_RegExpOptions* cel_nullable options,
    cel_StringView subject, cel_Status* cel_nonnull status) {
  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  RE2::Options re2_options = _cel_RegExpOptions_ToRe2(options);
  RE2 re2(cel_StringView_ToAbsl(pattern), re2_options);
  if (CEL_UNLIKELY(!re2.ok())) {
    cel_StatusCode code = re2.error_code() != RE2::ErrorPatternTooLarge
                              ? cel_StatusCode_kInvalidArgument
                              : cel_StatusCode_kResourceExhausted;
    cel_CanonicalStatus(status, code,
                        cel_StringView_FromString(re2.error().c_str()));
    return false;
  }
  return RE2::PartialMatch(cel_StringView_ToAbsl(subject), re2);
}
