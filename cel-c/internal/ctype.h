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

#include <stdbool.h>

#include "cel-c/internal/config.h"

CEL_BEGIN_DECLS

CEL_EXTERN const unsigned char _cel_CType_kProperties[256];
CEL_EXTERN const char _cel_CType_kToLower[256];
CEL_EXTERN const char _cel_CType_kToUpper[256];

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_isalpha(unsigned char c) {
  return (_cel_CType_kProperties[c] & 0x01u) != 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_isalnum(unsigned char c) {
  return (_cel_CType_kProperties[c] & 0x04u) != 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_isspace(unsigned char c) {
  return (_cel_CType_kProperties[c] & 0x08u) != 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ispunct(unsigned char c) {
  return (_cel_CType_kProperties[c] & 0x10u) != 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_isblank(unsigned char c) {
  return (_cel_CType_kProperties[c] & 0x20u) != 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_iscntrl(unsigned char c) {
  return (_cel_CType_kProperties[c] & 0x40u) != 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_isxdigit(unsigned char c) {
  return (_cel_CType_kProperties[c] & 0x80u) != 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_isdigit(unsigned char c) {
  return c >= '0' && c <= '9';
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_isprint(unsigned char c) {
  return c >= 32u && c < 127u;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_isgraph(unsigned char c) {
  return c > 32u && c < 127u;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_islower(unsigned char c) {
  return c >= 'a' && c <= 'z';
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_isupper(unsigned char c) {
  return c >= 'A' && c <= 'Z';
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_isascii(unsigned char c) { return c < 128u; }

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE char _cel_tolower(unsigned char c) {
  return _cel_CType_kToLower[c];
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE char _cel_toupper(unsigned char c) {
  return _cel_CType_kToUpper[c];
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_CTYPE_H_
