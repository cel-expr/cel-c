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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_CHARCONV_H_
#define THIRD_PARTY_CEL_C_INTERNAL_CHARCONV_H_

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

#include "cel-c/internal/config.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

CEL_BEGIN_DECLS

#define _CEL_MAX_INT_CHARS 66
#define _CEL_MAX_UINT_CHARS 65
#define _CEL_MAX_FLOAT_CHARS                                             \
  (/*<mantissa>*/ 9 + /*-*/ 1 + /*<decimal_point>*/ 4 + /*[eE][+-]*/ 2 + \
   /*<exponent>*/ 3 + /*\0*/ 1)
#define _CEL_MAX_DOUBLE_CHARS                                             \
  (/*<mantissa>*/ 17 + /*-*/ 1 + /*<decimal_point>*/ 4 + /*[eE][+-]*/ 2 + \
   /*<exponent>*/ 3 + /*\0*/ 1)

typedef struct {
  const char* cel_nullability_unknown ptr;
  int ec;
} _cel_FromCharsResult;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
_cel_FromCharsResult _cel_FromChars_signed(const char* cel_nonnull first,
                                           const char* cel_nonnull last,
                                           long long* cel_nonnull val, int base,
                                           long long min, long long max);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
_cel_FromCharsResult _cel_FromChars_unsigned(
    const char* cel_nonnull first, const char* cel_nonnull last,
    unsigned long long* cel_nonnull val, int base, unsigned long long max);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_FromCharsResult
_cel_FromChars_s(const char* cel_nonnull first, const char* cel_nonnull last,
                 short* cel_nonnull val, int base) {
  long long ret;
  _cel_FromCharsResult res =
      _cel_FromChars_signed(first, last, &ret, base, SHRT_MIN, SHRT_MAX);
  *val = (short)ret;
  return res;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_FromCharsResult
_cel_FromChars_us(const char* cel_nonnull first, const char* cel_nonnull last,
                  unsigned short* cel_nonnull val, int base) {
  unsigned long long ret;
  _cel_FromCharsResult res =
      _cel_FromChars_unsigned(first, last, &ret, base, USHRT_MAX);
  *val = (unsigned short)ret;
  return res;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_FromCharsResult
_cel_FromChars_i(const char* cel_nonnull first, const char* cel_nonnull last,
                 int* cel_nonnull val, int base) {
  long long ret;
  _cel_FromCharsResult res =
      _cel_FromChars_signed(first, last, &ret, base, INT_MIN, INT_MAX);
  *val = (int)ret;
  return res;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_FromCharsResult
_cel_FromChars_ui(const char* cel_nonnull first, const char* cel_nonnull last,
                  unsigned int* cel_nonnull val, int base) {
  unsigned long long ret;
  _cel_FromCharsResult res =
      _cel_FromChars_unsigned(first, last, &ret, base, UINT_MAX);
  *val = (unsigned int)ret;
  return res;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_FromCharsResult
_cel_FromChars_l(const char* cel_nonnull first, const char* cel_nonnull last,
                 long* cel_nonnull val, int base) {
  long long ret;
  _cel_FromCharsResult res =
      _cel_FromChars_signed(first, last, &ret, base, LONG_MIN, LONG_MAX);
  *val = (long)ret;
  return res;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_FromCharsResult
_cel_FromChars_ul(const char* cel_nonnull first, const char* cel_nonnull last,
                  unsigned long* cel_nonnull val, int base) {
  unsigned long long ret;
  _cel_FromCharsResult res =
      _cel_FromChars_unsigned(first, last, &ret, base, ULONG_MAX);
  *val = (unsigned long)ret;
  return res;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_FromCharsResult
_cel_FromChars_ll(const char* cel_nonnull first, const char* cel_nonnull last,
                  long long* cel_nonnull val, int base) {
  return _cel_FromChars_signed(first, last, val, base, LLONG_MIN, LLONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_FromCharsResult
_cel_FromChars_ull(const char* cel_nonnull first, const char* cel_nonnull last,
                   unsigned long long* cel_nonnull val, int base) {
  return _cel_FromChars_unsigned(first, last, val, base, ULLONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
_cel_FromCharsResult _cel_FromChars_f(const char* cel_nonnull first,
                                      const char* cel_nonnull last,
                                      float* cel_nonnull val);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
_cel_FromCharsResult _cel_FromChars_d(const char* cel_nonnull first,
                                      const char* cel_nonnull last,
                                      double* cel_nonnull val);

CEL_END_DECLS

// _cel_FromChars
//
// Parses an integral or floating point from its textual representation.
#ifndef __cplusplus
#define _cel_FromChars(first, last, val, ...)  \
  (_Generic(*(val),                            \
       short: _cel_FromChars_s,                \
       unsigned short: _cel_FromChars_us,      \
       int: _cel_FromChars_i,                  \
       unsigned int: _cel_FromChars_ui,        \
       long: _cel_FromChars_l,                 \
       unsigned long: _cel_FromChars_ul,       \
       long long: _cel_FromChars_ll,           \
       unsigned long long: _cel_FromChars_ull, \
       float: _cel_FromChars_f,                \
       double: _cel_FromChars_d)((first), (last), (val), ##__VA_ARGS__))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_FromCharsResult _cel_FromChars(const char* cel_nonnull first,
                                               const char* cel_nonnull last,
                                               short* cel_nonnull val,
                                               int base) {
  return _cel_FromChars_s(first, last, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_FromCharsResult _cel_FromChars(const char* cel_nonnull first,
                                               const char* cel_nonnull last,
                                               unsigned short* cel_nonnull val,
                                               int base) {
  return _cel_FromChars_us(first, last, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_FromCharsResult _cel_FromChars(const char* cel_nonnull first,
                                               const char* cel_nonnull last,
                                               int* cel_nonnull val, int base) {
  return _cel_FromChars_i(first, last, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_FromCharsResult _cel_FromChars(const char* cel_nonnull first,
                                               const char* cel_nonnull last,
                                               unsigned int* cel_nonnull val,
                                               int base) {
  return _cel_FromChars_ui(first, last, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_FromCharsResult _cel_FromChars(const char* cel_nonnull first,
                                               const char* cel_nonnull last,
                                               long* cel_nonnull val,
                                               int base) {
  return _cel_FromChars_l(first, last, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_FromCharsResult _cel_FromChars(const char* cel_nonnull first,
                                               const char* cel_nonnull last,
                                               unsigned long* cel_nonnull val,
                                               int base) {
  return _cel_FromChars_ul(first, last, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_FromCharsResult _cel_FromChars(const char* cel_nonnull first,
                                               const char* cel_nonnull last,
                                               long long* cel_nonnull val,
                                               int base) {
  return _cel_FromChars_ll(first, last, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_FromCharsResult
_cel_FromChars(const char* cel_nonnull first, const char* cel_nonnull last,
               unsigned long long* cel_nonnull val, int base) {
  return _cel_FromChars_ull(first, last, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_FromCharsResult _cel_FromChars(const char* cel_nonnull first,
                                               const char* cel_nonnull last,
                                               float* cel_nonnull val) {
  return _cel_FromChars_f(first, last, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_FromCharsResult _cel_FromChars(const char* cel_nonnull first,
                                               const char* cel_nonnull last,
                                               double* cel_nonnull val) {
  return _cel_FromChars_d(first, last, val);
}
#endif

CEL_BEGIN_DECLS

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
size_t _cel_ToChars_ll(char* cel_nonnull out, long long val, int base);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
size_t _cel_ToChars_ull(char* cel_nonnull out, unsigned long long val,
                        int base);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
size_t _cel_ToChars_f(char* cel_nonnull first, float val);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
size_t _cel_ToChars_d(char* cel_nonnull first, double val);

CEL_END_DECLS

// _cel_ToChars
//
// Formats an integral or floating point to its textual representation.
// For floating point we currently use `%.17g` for double and `%.9g` for float.
// The size increase from using `<charconv>`, implementing Ryu, or implementing
// Dragonbox is untenable.
#ifndef __cplusplus
#define _cel_ToChars(first, val, ...)        \
  (_Generic((val),                           \
       short: _cel_ToChars_ll,               \
       unsigned short: _cel_ToChars_ull,     \
       int: _cel_ToChars_ll,                 \
       unsigned int: _cel_ToChars_ull,       \
       long: _cel_ToChars_ll,                \
       unsigned long: _cel_ToChars_ull,      \
       long long: _cel_ToChars_ll,           \
       unsigned long long: _cel_ToChars_ull, \
       float: _cel_ToChars_f,                \
       double: _cel_ToChars_d)((first), (val), ##__VA_ARGS__))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE size_t _cel_ToChars(char* cel_nonnull out, short val, int base) {
  return _cel_ToChars_ll(out, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE size_t _cel_ToChars(char* cel_nonnull out, unsigned short val,
                               int base) {
  return _cel_ToChars_ull(out, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE size_t _cel_ToChars(char* cel_nonnull out, int val, int base) {
  return _cel_ToChars_ll(out, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE size_t _cel_ToChars(char* cel_nonnull out, unsigned int val,
                               int base) {
  return _cel_ToChars_ull(out, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE size_t _cel_ToChars(char* cel_nonnull out, long val, int base) {
  return _cel_ToChars_ll(out, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE size_t _cel_ToChars(char* cel_nonnull out, unsigned long val,
                               int base) {
  return _cel_ToChars_ull(out, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE size_t _cel_ToChars(char* cel_nonnull out, long long val, int base) {
  return _cel_ToChars_ll(out, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE size_t _cel_ToChars(char* cel_nonnull out, unsigned long long val,
                               int base) {
  return _cel_ToChars_ull(out, val, base);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE size_t _cel_ToChars(char* cel_nonnull first, float val) {
  return _cel_ToChars_f(first, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE size_t _cel_ToChars(char* cel_nonnull first, double val) {
  return _cel_ToChars_d(first, val);
}
#endif

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)

#endif  // THIRD_PARTY_CEL_C_INTERNAL_CHARCONV_H_
