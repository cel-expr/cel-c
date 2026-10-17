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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_CTYPE_H_
#define THIRD_PARTY_CEL_C_INTERNAL_CTYPE_H_

#include "absl/strings/ascii.h"
#include "cel-c/internal/config.h"

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_isalpha(unsigned char c) {
  return absl::ascii_isalpha(c);
}

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_isalnum(unsigned char c) {
  return absl::ascii_isalnum(c);
}

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_isspace(unsigned char c) {
  return absl::ascii_isspace(c);
}

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_ispunct(unsigned char c) {
  return absl::ascii_ispunct(c);
}

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_isblank(unsigned char c) {
  return absl::ascii_isblank(c);
}

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_iscntrl(unsigned char c) {
  return absl::ascii_iscntrl(c);
}

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_isxdigit(unsigned char c) {
  return absl::ascii_isxdigit(c);
}

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_isdigit(unsigned char c) {
  return absl::ascii_isdigit(c);
}

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_isprint(unsigned char c) {
  return absl::ascii_isprint(c);
}

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_isgraph(unsigned char c) {
  return absl::ascii_isgraph(c);
}

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_islower(unsigned char c) {
  return absl::ascii_islower(c);
}

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_isupper(unsigned char c) {
  return absl::ascii_isupper(c);
}

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_isascii(unsigned char c) {
  return absl::ascii_isascii(c);
}

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE char _cel_tolower(unsigned char c) {
  return absl::ascii_tolower(c);
}

[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE char _cel_toupper(unsigned char c) {
  return absl::ascii_toupper(c);
}

#endif  // THIRD_PARTY_CEL_C_INTERNAL_CTYPE_H_
