// Copyright 2024 Google LLC
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

// Internal header providing checked arithmetic functions. Uses <stdckdint.h> if
// available, otherwise falls back to compiler intrinsics or a pure
// implementation.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_CKDINT_H_
#define THIRD_PARTY_CEL_C_INTERNAL_CKDINT_H_

#include <limits>
#include <type_traits>

#include "cel-c/internal/config.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>, bool>
_cel_ckd_add(T* cel_nonnull out, T x, T y) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_add_overflow)
  return __builtin_add_overflow(x, y, out);
#else
  if (y >= 0 ? x > (std::numeric_limits<T>::max)() - y
             : x < (std::numeric_limits<T>::min)() - y) {
    return true;
  }
  *out = x + y;
  return false;
#endif
}

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,
                            bool> _cel_ckd_add(T* cel_nonnull out, T x, T y) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_add_overflow)
  return __builtin_add_overflow(x, y, out);
#else
  if (x > (std::numeric_limits<T>::max)() - y) {
    return true;
  }
  *out = x + y;
  return false;
#endif
}

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>, bool>
_cel_ckd_sub(T* cel_nonnull out, T x, T y) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_sub_overflow)
  return __builtin_sub_overflow(x, y, out);
#else
  if (y >= 0 ? x < (std::numeric_limits<T>::min)() + y
             : x > (std::numeric_limits<T>::max)() + y) {
    return true;
  }
  *out = x - y;
  return false;
#endif
}

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,
                            bool> _cel_ckd_sub(T* cel_nonnull out, T x, T y) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_sub_overflow)
  return __builtin_sub_overflow(x, y, out);
#else
  if (x < y) {
    return true;
  }
  *out = x - y;
  return false;
#endif
}

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>, bool>
_cel_ckd_mul(T* cel_nonnull out, T x, T y) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_mul_overflow)
  return __builtin_mul_overflow(x, y, out);
#else
  if ((x == -1 && y == (std::numeric_limits<T>::min)()) ||
      (y == -1 && x == (std::numeric_limits<T>::min)()) ||
      (x > 0 && y > 0 && x > (std::numeric_limits<T>::max)() / y) ||
      (x < 0 && y < 0 && x < (std::numeric_limits<T>::max)() / y) ||
      (x > 0 && y < 0 && y < (std::numeric_limits<T>::min)() / x) ||
      (x < 0 && y > 0 && x < (std::numeric_limits<T>::min)() / y)) {
    return true;
  }
  *out = x * y;
  return false;
#endif
}

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,
                            bool> _cel_ckd_mul(T* cel_nonnull out, T x, T y) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_mul_overflow)
  return __builtin_mul_overflow(x, y, out);
#else
  if (x > 0 && y > 0 && x > (std::numeric_limits<T>::max)() / y) {
    return true;
  }
  *out = x * y;
  return false;
#endif
}

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>, bool>
_cel_ckd_div(T* cel_nonnull out, T x, T y) {
  if (y == 0 || (x == (std::numeric_limits<T>::min)() && y == -1)) {
    return true;
  }
  *out = x / y;
  return false;
}

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,
                            bool> _cel_ckd_div(T* cel_nonnull out, T x, T y) {
  if (y == 0) {
    return true;
  }
  *out = x / y;
  return false;
}

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>, bool>
_cel_ckd_mod(T* cel_nonnull out, T x, T y) {
  if (y == 0 || (x == (std::numeric_limits<T>::min)() && y == -1)) {
    return true;
  }
  *out = x % y;
  return false;
}

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,
                            bool> _cel_ckd_mod(T* cel_nonnull out, T x, T y) {
  if (y == 0) {
    return true;
  }
  *out = x % y;
  return false;
}

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)

#endif  // THIRD_PARTY_CEL_C_INTERNAL_CKDINT_H_
