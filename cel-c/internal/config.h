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

// IWYU pragma: always_keep

#ifndef THIRD_PARTY_CEL_C_INTERNAL_CONFIG_H_
#define THIRD_PARTY_CEL_C_INTERNAL_CONFIG_H_

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "cel-c/config.h"  // IWYU pragma: export

#if defined(__BIG_ENDIAN__) || defined(__ARMEB__) || defined(__THUMBEB__) || \
    defined(__AARCH64EB__) || defined(_MIPSEB) || defined(__MIPSEB) ||       \
    defined(__MIPSEB__) ||                                                   \
    (defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) &&             \
     __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define _CEL_IS_BIG_ENDIAN 1
#elif defined(__LITTLE_ENDIAN__) || defined(__ARMEL__) ||                 \
    defined(__THUMBEL__) || defined(__AARCH64EL__) || defined(_MIPSEL) || \
    defined(__MIPSEL) || defined(__MIPSEL__) ||                           \
    (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) &&       \
     __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) ||                        \
    defined(_WIN32)
#define _CEL_IS_LITTLE_ENDIAN 1
#else
#error Endian detection needs to be set up for your environment.
#endif

#if (defined(__clang__) && !defined(_WIN32)) ||                           \
    (defined(__CUDACC__) && __CUDACC_VER_MAJOR__ >= 9) ||                 \
    (defined(__GNUC__) && !defined(__clang__) && !defined(__CUDACC__)) || \
    (defined(__CUDACC__) && __CUDACC_VER__ >= 70000)
#define _CEL_HAVE_INTRINSIC_INT128 1
#endif

#if defined(__cplusplus) &&                                 \
    (CEL_HAVE_FEATURE(cxx_exceptions) ||                    \
     (!(defined(__GNUC__) && !defined(__cpp_exceptions)) && \
      !(defined(_MSC_VER) && !defined(_CPPUNWIND))))
#define _CEL_HAVE_EXCEPTIONS 1
#define _CEL_TRY try
#define _CEL_CATCH_ANY catch (...)
#define _CEL_RETHROW \
  do {               \
    throw;           \
  } while (false)
#else
#define _CEL_TRY if (true)
#define _CEL_CATCH_ANY else if (false)
#define _CEL_RETHROW \
  do {               \
  } while (false)
#endif

#if defined(__cplusplus) && (CEL_HAVE_FEATURE(cxx_rtti) ||                 \
                             (defined(__GNUC__) && defined(__GXX_RTTI)) || \
                             (defined(_MSC_VER) && defined(_CPPRTTI)) ||   \
                             (!defined(__GNUC__) && !defined(_MSC_VER)))
#define _CEL_HAVE_RTTI 1
#endif

#if ((defined(__GNUC__) && !defined(__clang__)) || \
     CEL_HAVE_ATTRIBUTE(weak)) &&                  \
    !defined(_WIN32) && !defined(__MINGW32__)
#define _CEL_ATTRIBUTE_WEAK __attribute__((weak))
#define _CEL_HAVE_ATTRIBUTE_WEAK 1
#else
#define _CEL_ATTRIBUTE_WEAK
#define _CEL_HAVE_ATTRIBUTE_WEAK 0
#endif

#if ((defined(__GNUC__) && !defined(__clang__)) || CEL_HAVE_ATTRIBUTE(unused))
#define _CEL_ATTRIBUTE_UNUSED __attribute__((unused))
#else
#define _CEL_ATTRIBUTE_UNUSED
#endif

#ifdef _CEL_HAVE_ASAN
#error _CEL_HAVE_ASAN cannot be directly set
#endif

#if defined(__SANITIZE_ADDRESS__) || CEL_HAVE_FEATURE(address_sanitizer)
#define _CEL_HAVE_ASAN 1
#endif

#ifdef _CEL_HAVE_HWASAN
#error _CEL_HAVE_HWASAN cannot be directly set
#endif

#if defined(__SANITIZE_HWADDRESS__) || CEL_HAVE_FEATURE(hwaddress_sanitizer)
#define _CEL_HAVE_HWASAN 1
#endif

#ifdef _CEL_HAVE_TSAN
#error _CEL_HAVE_TSAN cannot be directly set
#endif

#if defined(__SANITIZE_THREAD__) || CEL_HAVE_FEATURE(thread_sanitizer)
#define _CEL_HAVE_TSAN 1
#endif

#ifdef _CEL_HAVE_MSAN
#error _CEL_HAVE_MSAN cannot be directly set
#endif

#if CEL_HAVE_FEATURE(memory_sanitizer)
#define _CEL_HAVE_MSAN 1
#endif

#ifdef _CEL_HAVE_LSAN
#error _CEL_HAVE_LSAN cannot be directly set
#endif

#if CEL_HAVE_FEATURE(leak_sanitizer)
#define _CEL_HAVE_LSAN 1
#endif

#ifdef _CEL_HAVE_SANITIZER
#error _CEL_HAVE_SANITIZER cannot be directly set
#endif

#if defined(_CEL_HAVE_ASAN) || defined(_CEL_HAVE_HWASAN) || \
    defined(_CEL_HAVE_TSAN) || defined(_CEL_HAVE_MSAN) ||   \
    defined(_CEL_HAVE_LSAN)
#define _CEL_HAVE_SANITIZER 1
#endif

// cel_typeof/cel_typeof_unqual
//
// Expands to `typeof` and `typeof_unqual` equivalents, respectively. In C++
// mode we remove references which do not exist in C. We also map `char8_t`,
// `char16_t`, `char32_t`, and `wchar_t` to their respective C types which are
// aliases of other builtin types instead of distinct types. Additionally we map
// enums to their underlying type.
#ifdef __cplusplus
template <typename T, typename = void>
struct _cel_typeof {
  using type = T;
};
template <typename T>
struct _cel_typeof<T, typename std::enable_if<(
                          std::is_enum<T>::value && !std::is_const<T>::value &&
                          !std::is_volatile<T>::value)>::type> {
  using type =
      typename _cel_typeof<typename std::underlying_type<T>::type>::type;
};
template <typename T>
struct _cel_typeof<T&> {
  using type = typename _cel_typeof<T>::type;
};
template <typename T>
struct _cel_typeof<T&&> {
  using type = typename _cel_typeof<T>::type;
};
template <typename T>
struct _cel_typeof<T*> {
  using type = typename _cel_typeof<T>::type*;
};
template <typename T>
struct _cel_typeof<T[]> {
  using type = typename _cel_typeof<T>::type[];
};
template <typename T>
struct _cel_typeof<const T[]> {
  using type = const typename _cel_typeof<T>::type[];
};
template <typename T>
struct _cel_typeof<volatile T[]> {
  using type = volatile typename _cel_typeof<T>::type[];
};
template <typename T>
struct _cel_typeof<const volatile T[]> {
  using type = const volatile typename _cel_typeof<T>::type[];
};
template <typename T, size_t N>
struct _cel_typeof<T[N]> {
  using type = typename _cel_typeof<T>::type[N];
};
template <typename T, size_t N>
struct _cel_typeof<const T[N]> {
  using type = const typename _cel_typeof<T>::type[N];
};
template <typename T, size_t N>
struct _cel_typeof<volatile T[N]> {
  using type = volatile typename _cel_typeof<T>::type[N];
};
template <typename T, size_t N>
struct _cel_typeof<const volatile T[N]> {
  using type = const volatile typename _cel_typeof<T>::type[N];
};
template <typename T>
struct _cel_typeof<const T> {
  using type = const typename _cel_typeof<T>::type;
};
template <typename T>
struct _cel_typeof<volatile T> {
  using type = volatile typename _cel_typeof<T>::type;
};
template <typename T>
struct _cel_typeof<const volatile T> {
  using type = const volatile typename _cel_typeof<T>::type;
};
template <>
struct _cel_typeof<wchar_t> {
#if defined(__GNUC__) || defined(__clang__)
  using type = __WCHAR_TYPE__;
#elif defined(_MSC_VER)
  using type = unsigned short;  // NOLINT(runtime/int)
#else
#error Unexpected compiler.
#endif
};
template <>
struct _cel_typeof<char16_t> {
  using type = uint_least16_t;
};
template <>
struct _cel_typeof<char32_t> {
  using type = uint_least32_t;
};
#if defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
template <>
struct _cel_typeof<char8_t> {
  using type = unsigned char;
};
#endif
template <typename T>
struct _cel_typeof_unqual {
  using type = typename std::remove_cv<typename _cel_typeof<T>::type>::type;
};
#define cel_typeof(x) typename ::_cel_typeof<decltype(x)>::type
#define cel_typeof_unqual(x) typename ::_cel_typeof_unqual<decltype(x)>::type
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#define cel_typeof(x) typeof(x)
#define cel_typeof_unqual(x) typeof_unqual(x)
#else
#define cel_typeof(x) __typeof__(x)
#define cel_typeof_unqual(x) __typeof_unqual__(x)
#endif

#ifdef cel_containerof
#error cel_containerof cannot be directly set
#endif

// cel_containerof
//
// Behaves the same as `container_of` in the Linux kernel.
#define cel_containerof(ptr, type, member)                                    \
  ((CEL_NULLABILITY_UNKNOWN(type*))(((CEL_NULLABILITY_UNKNOWN(char*))(ptr)) - \
                                    offsetof(type, member)))

#ifdef _CEL_HAVE_TYPES_COMPATIBLE
#error _CEL_HAVE_TYPES_COMPATIBLE cannot be directly set
#endif

#ifdef _CEL_TYPES_COMPATIBLE
#error _CEL_TYPES_COMPATIBLE cannot be directly set
#endif

#ifdef __cplusplus
#define _CEL_HAVE_TYPES_COMPATIBLE 1
template <typename, typename>
struct _cel_types_compatible : std::false_type {};
template <typename T>
struct _cel_types_compatible<T, T> : std::true_type {};
template <typename T>
struct _cel_types_compatible<T[], T[]> : std::true_type {};
template <typename T, size_t N>
struct _cel_types_compatible<T[N], T[]> : _cel_types_compatible<T[], T[]> {};
template <typename T, size_t N>
struct _cel_types_compatible<T[], T[N]> : _cel_types_compatible<T[], T[]> {};
#define _CEL_TYPES_COMPATIBLE(type1, type2)                           \
  ::_cel_types_compatible<typename ::_cel_typeof_unqual<type1>::type, \
                          typename ::_cel_typeof_unqual<type2>::type>::value
#elif (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_types_compatible_p)
#define _CEL_HAVE_TYPES_COMPATIBLE 1
#define _CEL_TYPES_COMPATIBLE(type1, type2) \
  __builtin_types_compatible_p(type1, type2)
#else
#define _CEL_TYPES_COMPATIBLE(type1, type2) true
#endif

#endif  // THIRD_PARTY_CEL_C_INTERNAL_CONFIG_H_
