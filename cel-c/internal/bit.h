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

// Internal header providing bit manipulation functions. Uses <stdbit.h> if
// available, otherwise falls back to compiler intrinsics or a pure
// implementation.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_BIT_H_
#define THIRD_PARTY_CEL_C_INTERNAL_BIT_H_

#include <type_traits>

#include "absl/numeric/bits.h"
#include "cel-c/internal/config.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

template <typename T>
[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, int>
    _cel_leading_zeros(T val) {
  return absl::countl_zero(val);
}

template <typename T>
[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, int>
    _cel_trailing_zeros(T val) {
  return absl::countr_zero(val);
}

template <typename T>
[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, bool>
    _cel_has_single_bit(T val) {
  return absl::has_single_bit(val);
}

template <typename T>
[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, int>
    _cel_bit_width(T val) {
  return absl::bit_width(val);
}

template <typename T>
[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>
    _cel_bit_ceil(T val) {
  return absl::bit_ceil(val);
}

template <typename T>
[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>
    _cel_bit_floor(T val) {
  return absl::bit_floor(val);
}

template <typename T>
[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>
    _cel_rotr(T val, int s) {
  return absl::rotr(val, s);
}

template <typename T>
[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>
    _cel_rotl(T val, int s) {
  return absl::rotl(val, s);
}

template <typename T>
[[nodiscard]]
CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, int>
    _cel_ffs(T val) {
  return val != 0 ? absl::countr_zero(val) + 1 : 0;
}

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)

#endif  // THIRD_PARTY_CEL_C_INTERNAL_BIT_H_
