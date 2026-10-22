// Copyright 2026 Google LLC
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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_TRIVIAL_RELOCATION_HPP_
#define THIRD_PARTY_CEL_C_INTERNAL_TRIVIAL_RELOCATION_HPP_

#include <cstddef>
#include <cstring>
#include <type_traits>

#include "cel-c/internal/config.h"  // IWYU pragma: keep

#if CEL_HAVE_BUILTIN(__builtin_is_cpp_trivially_relocatable)
// https://github.com/llvm/llvm-project/pull/127636#pullrequestreview-2637005293
// In the current implementation, __builtin_is_cpp_trivially_relocatable will
// only return true for types that are trivially relocatable according to the
// standard. Notably, this means that marking a type [[clang::trivial_abi]] aka
// ABSL_HAVE_ATTRIBUTE_TRIVIAL_ABI will have no effect on this trait.
template <typename T, typename = void>
struct cel_IsTriviallyRelocatable
    : std::bool_constant<__builtin_is_cpp_trivially_relocatable(T)> {};
#else
template <typename T, typename = void>
struct cel_IsTriviallyRelocatable : std::is_trivially_copyable<T> {};
#endif

template <typename T>
constexpr bool cel_kIsTriviallyRelocatable =
    cel_IsTriviallyRelocatable<T>::value;

// Performs trivial relocation via memcpy.
CEL_INLINE void cel_TriviallyCopy(void* cel_nullability_unknown dst,
                                  const void* cel_nullability_unknown src,
                                  size_t count) {
  memcpy(dst, src, count);
}

// Performs trivial relocation via memcpy, silencing undefined behavior warnings
// if cel_kIsTriviallyRelocatable<T> is true.
template <typename T>
CEL_INLINE void cel_TriviallyCopy(T* cel_nullability_unknown dst,
                                  const T* cel_nullability_unknown src,
                                  size_t count) {
  static_assert(!std::is_void_v<T>, "T must not be void");
  static_assert(cel_kIsTriviallyRelocatable<T>,
                "T must be trivially relocatable");
  (cel_TriviallyCopy)(reinterpret_cast<void*>(dst),
                      reinterpret_cast<const void*>(src), count * sizeof(T));
}

// Performs trivial relocation via memmove.
CEL_INLINE void cel_TriviallyMove(void* cel_nullability_unknown dst,
                                  const void* cel_nullability_unknown src,
                                  size_t count) {
  memmove(dst, src, count);
}

// Performs trivial relocation via memmove, silencing undefined behavior
// warnings if cel_kIsTriviallyRelocatable<T> is true.
template <typename T>
CEL_INLINE void cel_TriviallyMove(T* cel_nullability_unknown dst,
                                  const T* cel_nullability_unknown src,
                                  size_t count) {
  static_assert(!std::is_void_v<T>, "T must not be void");
  static_assert(cel_kIsTriviallyRelocatable<T>,
                "T must be trivially relocatable");
  (cel_TriviallyMove)(reinterpret_cast<void*>(dst),
                      reinterpret_cast<const void*>(src), count * sizeof(T));
}

#endif  // THIRD_PARTY_CEL_C_INTERNAL_TRIVIAL_RELOCATION_HPP_
