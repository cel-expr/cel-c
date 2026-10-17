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

// Internal header providing address alignment functions. Uses compiler
// intrinsics if available, otherwise falls back to a pure implementation.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_ALIGN_H_
#define THIRD_PARTY_CEL_C_INTERNAL_ALIGN_H_

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "cel-c/assert.h"
#include "cel-c/internal/bit.h"
#include "cel-c/internal/config.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>
_cel_align_up(T val, size_t align) {
  CEL_ASSERT(_cel_has_single_bit(align));
#if CEL_HAVE_BUILTIN(__builtin_align_up)
  return (val != 0) ? __builtin_align_up(val, align) : 0;
#else
  using U = std::common_type_t<T, size_t>;
  return static_cast<T>((val != 0)
                            ? static_cast<U>((static_cast<U>(val) +
                                              (static_cast<U>(align) - U{1})) &
                                             ~(static_cast<U>(align) - U{1}))
                            : U{0});
#endif
}

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_pointer_v<T>, T> _cel_align_up(
    T val, size_t align) {
  return reinterpret_cast<T>(
      (_cel_align_up)(reinterpret_cast<uintptr_t>(val), align));
}

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>
_cel_align_down(T val, size_t align) {
  CEL_ASSERT(_cel_has_single_bit(align));
#if CEL_HAVE_BUILTIN(__builtin_align_down)
  return (val != 0) ? __builtin_align_down(val, align) : 0;
#else
  using U = std::common_type_t<T, size_t>;
  return static_cast<T>(
      (val != 0) ? (static_cast<U>(val) & ~(static_cast<U>(align) - U{1}))
                 : U{0});
#endif
}

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_pointer_v<T>, T> _cel_align_down(
    T val, size_t align) {
  return reinterpret_cast<T>(
      (_cel_align_down)(reinterpret_cast<uintptr_t>(val), align));
}

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,
                            bool> _cel_is_aligned(T val, size_t align) {
  CEL_ASSERT(_cel_has_single_bit(align));
#if CEL_HAVE_BUILTIN(__builtin_is_aligned)
  return __builtin_is_aligned(val, align);
#else
  using U = std::common_type_t<T, size_t>;
  return (static_cast<U>(val) & (static_cast<U>(align) - U{1})) == U{0};
#endif
}

template <typename T>
[[nodiscard]]
CEL_INLINE std::enable_if_t<std::is_pointer_v<T>, bool> _cel_is_aligned(
    T val, size_t align) {
  return (_cel_is_aligned)(reinterpret_cast<uintptr_t>(val), align);
}

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)

#endif  // THIRD_PARTY_CEL_C_INTERNAL_ALIGN_H_
