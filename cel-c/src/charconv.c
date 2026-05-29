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

#include "cel-c/src/charconv.h"

#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/src/ckdint.h"
#include "cel-c/src/config.h"
#include "cel-c/src/ctype.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

CEL_ATTRIBUTE_NODISCARD
static bool _cel_FromChar(char* cel_nonnull out, char in, int base) {
  if (in >= '0' && in <= '9') {
    *out = in - '0';
    return *out < base;
  }
  if (base >= 10 && in >= 'a' && in <= 'z') {
    *out = (in - 'a') + (char)10;
    return *out < base;
  }
  if (base >= 10 && in >= 'A' && in <= 'Z') {
    *out = (in - 'A') + (char)10;
    return *out < base;
  }
  return false;
}

_cel_FromCharsResult _cel_FromChars_signed(const char* cel_nonnull first,
                                           const char* cel_nonnull last,
                                           long long* cel_nonnull val, int base,
                                           long long min, long long max) {
  bool negative = first != last && *first == '-';
  if (negative) {
    ++first;
  }
  _cel_FromCharsResult result;
  result.ec = 0;
  bool digits = false;
  *val = 0;
  while (first != last) {
    char digit;
    if (!_cel_FromChar(&digit, *first, base)) {
      break;
    }
    if (_cel_ckd_mul(val, *val, (long long)base)) {
      result.ec = ERANGE;
      break;
    }
    if (_cel_ckd_add(val, *val, (long long)digit)) {
      result.ec = ERANGE;
      break;
    }
    if (*val < min || *val > max) {
      result.ec = ERANGE;
      break;
    }
    digits = true;
    ++first;
  }
  result.ptr = first;
  if (!digits) {
    result.ec = EINVAL;
  } else if (negative && _cel_ckd_mul(val, *val, (long long)-1) &&
             (*val < min || *val > max)) {
    result.ec = ERANGE;
  }
  return result;
}

_cel_FromCharsResult _cel_FromChars_unsigned(
    const char* cel_nonnull first, const char* cel_nonnull last,
    unsigned long long* cel_nonnull val, int base, unsigned long long max) {
  _cel_FromCharsResult result;
  result.ec = 0;
  bool digits = false;
  *val = 0;
  while (first != last) {
    char digit;
    if (!_cel_FromChar(&digit, *first, base)) {
      break;
    }
    if (_cel_ckd_mul(val, *val, (unsigned long long)base)) {
      result.ec = ERANGE;
      break;
    }
    if (_cel_ckd_add(val, *val, (unsigned long long)digit)) {
      result.ec = ERANGE;
      break;
    }
    if (*val > max) {
      result.ec = ERANGE;
      break;
    }
    digits = true;
    ++first;
  }
  result.ptr = first;
  if (!digits) {
    result.ec = EINVAL;
  }
  return result;
}

typedef struct {
  char str[5];
  uint8_t len;
} _cel_FromChars_LocaleRadix;

CEL_ATTRIBUTE_NODISCARD
static _cel_FromChars_LocaleRadix _cel_FromChars_GetLocaleRadix() {
  _cel_FromChars_LocaleRadix result;
#ifdef LC_GLOBAL_LOCALE
  struct lconv* lc = localeconv();
  result.len = (uint8_t)strlen(lc->decimal_point);
  CEL_ASSERT_GT(result.len, 0);
  CEL_ASSERT_LE(result.len, 4);
  memcpy(result.str, lc->decimal_point, result.len);
  result.str[result.len] = '\0';
#else
  char buffer[/*1*/ 1 + /*<decimal_point>*/ 4 + /*5*/ 1 + /*\0*/ 1];
  snprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), "%.1f", 1.5f);
  size_t len = strlen(buffer);
  CEL_ASSERT_GT(len, 2);
  CEL_ASSERT_EQ(buffer[0], '1');
  CEL_ASSERT_EQ(buffer[len - 1], '5');
  size_t out_len = len - 2;
  CEL_ASSERT_GE(out_len, 1);
  CEL_ASSERT_LE(out_len, 4);
  result.len = (uint8_t)(len - 2);
  memcpy(result.str, buffer + 1, out_len);
  result.str[result.len] = '\0';
#endif
  return result;
}

CEL_ATTRIBUTE_NODISCARD
static const char* cel_nonnull _cel_FromChars_MatchMinus(
    const char* cel_nonnull first, const char* cel_nonnull last) {
  if (first == last || *first != '-') {
    return first;
  }
  return first + 1;
}

CEL_ATTRIBUTE_NODISCARD
static const char* cel_nonnull _cel_FromChars_MatchDigits(
    const char* cel_nonnull first, const char* cel_nonnull last) {
  while (first != last) {
    if (!_cel_isdigit(*first)) {
      break;
    }
    ++first;
  }
  return first;
}

CEL_ATTRIBUTE_NODISCARD
static const char* cel_nonnull _cel_FromChars_MatchDot(
    const char* cel_nonnull first, const char* cel_nonnull last) {
  if (first == last || *first != '.') {
    return first;
  }
  return first + 1;
}

CEL_ATTRIBUTE_NODISCARD
static const char* cel_nonnull _cel_FromChars_MatchE(
    const char* cel_nonnull first, const char* cel_nonnull last) {
  if (first == last || (*first != 'e' && *first != 'E')) {
    return first;
  }
  return first + 1;
}

CEL_ATTRIBUTE_NODISCARD
static const char* cel_nonnull _cel_FromChars_MatchSign(
    const char* cel_nonnull first, const char* cel_nonnull last) {
  if (first == last || (*first != '-' && *first != '+')) {
    return first;
  }
  return first + 1;
}

CEL_ATTRIBUTE_NODISCARD
static int _cel_FromChars_Match(const char* cel_nonnull first,
                                const char* cel_nonnull last,
                                char* cel_nonnull out, size_t maxlen,
                                const char* cel_nullable* cel_nonnull end,
                                const char* cel_nullable* cel_nonnull radix,
                                uint8_t* cel_nonnull radixadj) {
  const char* cel_nonnull ptr = first;
  ptr = _cel_FromChars_MatchMinus(ptr, last);
  ptr = _cel_FromChars_MatchDigits(ptr, last);
  const char* cel_nullable sep = _cel_FromChars_MatchDot(ptr, last);
  if (sep == ptr) {
    sep = NULL;
  } else {
    const char* next = sep;
    sep = ptr;
    ptr = next;
  }
  ptr = _cel_FromChars_MatchDigits(ptr, last);
  ptr = _cel_FromChars_MatchE(ptr, last);
  ptr = _cel_FromChars_MatchSign(ptr, last);
  ptr = _cel_FromChars_MatchDigits(ptr, last);
  *end = ptr;
  size_t len = ptr - first;
  if (len >= maxlen) {
    return ERANGE;
  }
  if (sep != NULL) {
    _cel_FromChars_LocaleRadix r = _cel_FromChars_GetLocaleRadix();
    size_t before_radix = sep - first;
    size_t after_radix = sep < ptr ? (ptr - (sep + 1)) : 0;
    memcpy(out, first, before_radix);
    memcpy(out + before_radix, r.str, r.len);
    memcpy(out + before_radix + r.len, sep + 1, after_radix);
    out[before_radix + r.len + after_radix] = '\0';
    *radix = sep;
    *radixadj = r.len - 1;
  } else {
    memcpy(out, first, len);
    out[len] = '\0';
    *radix = NULL;
    *radixadj = 0;
  }
  return 0;
}

_cel_FromCharsResult _cel_FromChars_f(const char* cel_nonnull first,
                                      const char* cel_nonnull last,
                                      float* cel_nonnull val) {
  _cel_FromCharsResult result;
  char buffer[FLT_MANT_DIG + FLT_MAX_EXP + 1 + 4 + 1 + 1];
  const char* radix;
  uint8_t radixadj;
  result.ec = _cel_FromChars_Match(first, last, buffer, sizeof(buffer) - 3,
                                   &result.ptr, &radix, &radixadj);
  if (result.ec != 0) {
    return result;
  }
  size_t radixoff = SIZE_MAX;
  if (radix != NULL) {
    radixoff = radix - first;
  }
  int prev_errno = errno;
  errno = 0;
  char* endptr;
  *val = strtof(buffer, &endptr);
  result.ec = errno;
  result.ptr = first + (endptr - buffer) -
               (endptr - buffer > radixoff ? radixadj : ((uint8_t)0));
  errno = prev_errno;
  return result;
}

_cel_FromCharsResult _cel_FromChars_d(const char* cel_nonnull first,
                                      const char* cel_nonnull last,
                                      double* cel_nonnull val) {
  _cel_FromCharsResult result;
  const char* radix;
  uint8_t radixadj;
  char buffer[DBL_MANT_DIG + DBL_MAX_EXP + 1 + 4 + 1 + 1];
  result.ec = _cel_FromChars_Match(first, last, buffer, sizeof(buffer) - 3,
                                   &result.ptr, &radix, &radixadj);
  if (result.ec != 0) {
    return result;
  }
  size_t radixoff = SIZE_MAX;
  if (radix != NULL) {
    radixoff = radix - first;
  }
  int prev_errno = errno;
  errno = 0;
  char* endptr;
  *val = strtod(buffer, &endptr);
  result.ec = errno;
  result.ptr = first + (endptr - buffer) -
               (endptr - buffer > radixoff ? radixadj : ((uint8_t)0));
  errno = prev_errno;
  return result;
}

static const char* _cel_ToChars_kDigits =
    "0123456789abcdefghijklmnopqrstuvwxyz";

static void _cel_ToChars_Reverse(char* cel_nonnull first,
                                 char* cel_nonnull last) {
  if (first < last) {
    --last;
    while (first < last) {
      char c = *first;
      *first = *last;
      *last = c;
      ++first;
      --last;
    }
  }
}

size_t _cel_ToChars_ll(char* cel_nonnull out, long long val, int base) {
  CEL_ASSERT_NOT_NULL(out);
  CEL_ASSERT_GE(base, 2);
  CEL_ASSERT_LE(base, 36);

  const bool negative = val < 0;
  if (negative) {
    *out++ = '-';
  }
  char* begin = out;
  do {
    int i = (int)(val % base);
    if (negative) {
      i = -i;
    }
    *out++ = _cel_ToChars_kDigits[i];
    val /= base;
  } while (val != 0);
  _cel_ToChars_Reverse(begin, out);
  return (size_t)(out - begin) + negative;
}

size_t _cel_ToChars_ull(char* cel_nonnull out, unsigned long long val,
                        int base) {
  CEL_ASSERT_NOT_NULL(out);
  CEL_ASSERT_GE(base, 2);
  CEL_ASSERT_LE(base, 36);

  char* begin = out;
  do {
    int i = (int)(val % base);
    *out++ = _cel_ToChars_kDigits[i];
    val /= base;
  } while (val != 0);
  _cel_ToChars_Reverse(begin, out);
  return (size_t)(out - begin);
}

CEL_ATTRIBUTE_NODISCARD
static size_t _cel_ToChars_FixLocale(char* cel_nonnull p) {
  _cel_FromChars_LocaleRadix radix = _cel_FromChars_GetLocaleRadix();
  size_t n = strlen(p);
  char* r = strstr(p, radix.str);
  if (r == NULL) {
    return n;
  }
  if (radix.len != 1) {
    memmove(r + 1, r + radix.len, (p + n) - (r + radix.len));
    n -= radix.len - 1;
    p[n] = '\0';
  }
  *r = '.';
  return n;
}

size_t _cel_ToChars_f(char* cel_nonnull first, float val) {
  first[0] = '\0';
  snprintf(first, _CEL_MAX_FLOAT_CHARS, "%.9g", val);
  return _cel_ToChars_FixLocale(first);
}

size_t _cel_ToChars_d(char* cel_nonnull first, double val) {
  first[0] = '\0';
  snprintf(first, _CEL_MAX_DOUBLE_CHARS, "%.17g", val);
  return _cel_ToChars_FixLocale(first);
}

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)
