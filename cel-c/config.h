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

// Defines various macros. All macros prefixed with `CEL_` are public. Macros
// prefixed with `_CEL_` are private.

// IWYU pragma: always_keep

#ifndef THIRD_PARTY_CEL_C_CONFIG_H_
#define THIRD_PARTY_CEL_C_CONFIG_H_

// Common includes needed regardless of whether this is C or C++.
#ifdef __cplusplus
#include <climits>      // IWYU pragma: keep
#include <cstddef>      // IWYU pragma: keep
#include <cstdint>      // IWYU pragma: keep
#include <cstdlib>      // IWYU pragma: keep
#ifdef __has_include
#if __has_include(<version>)
#include <version>  // IWYU pragma: keep
#endif
#endif
#else
#include <limits.h>    // IWYU pragma: keep
#include <stdalign.h>  // IWYU pragma: keep
#include <stdbool.h>   // IWYU pragma: keep
#include <stddef.h>    // IWYU pragma: keep
#include <stdint.h>    // IWYU pragma: keep
#include <stdlib.h>    // IWYU pragma: keep
#endif

#ifdef _CEL_IS_BIG_ENDIAN
#error _CEL_IS_BIG_ENDIAN cannot be directly set
#endif

#ifdef _CEL_IS_LITTLE_ENDIAN
#error _CEL_IS_LITTLE_ENDIAN cannot be directly set
#endif

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

#ifdef CEL_STATIC_ASSERT
#error CEL_STATIC_ASSERT cannot be directly set
#endif

// CEL_STATIC_ASSERT
//
// Expands to `static_assert` or `_Static_assert`.
#if (defined(__cplusplus) && defined(__cpp_static_assert) && \
     __cpp_static_assert >= 201411L) ||                      \
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L)
#define CEL_STATIC_ASSERT(condition) static_assert(condition)
#elif defined(__cplusplus)
#define CEL_STATIC_ASSERT(condition) static_assert(condition, #condition)
#else
#define CEL_STATIC_ASSERT(condition) _Static_assert(condition, #condition)
#endif

CEL_STATIC_ASSERT(CHAR_BIT == 8);
CEL_STATIC_ASSERT(sizeof(char) == 1);
CEL_STATIC_ASSERT(sizeof(signed char) == 1);
CEL_STATIC_ASSERT(sizeof(unsigned char) == 1);

#ifdef CEL_BEGIN_DECLS
#error CEL_BEGIN_DECLS cannot be directly set
#endif

#ifdef CEL_END_DECLS
#error CEL_END_DECLS cannot be directly set
#endif

// CEL_BEGIN_DECLS/CEL_END_DECLS
//
// Expands to tokens indicating the beginning and end of C declarations
// respectively.
#ifdef __cplusplus
#define CEL_BEGIN_DECLS extern "C" {
#define CEL_END_DECLS }
#else
#define CEL_BEGIN_DECLS
#define CEL_END_DECLS
#endif

#ifdef cel_nullptr
#error cel_nullptr cannot be directly set
#endif

// cel_nullptr
//
// Expands to either `nullptr` or `NULL`.
#if defined(__cplusplus) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L)
#define cel_nullptr nullptr
#else
#define cel_nullptr NULL
#endif

#ifdef CEL_HAVE_BUILTIN
#error CEL_HAVE_BUILTIN cannot be directly set
#endif

// CEL_HAVE_BUILTIN
//
// Wrapper around `__has_builtin` if it exists, otherwise expands to 0.
#ifdef __has_builtin
#define CEL_HAVE_BUILTIN(x) __has_builtin(x)
#else
#define CEL_HAVE_BUILTIN(x) 0
#endif

#ifdef CEL_HAVE_ATTRIBUTE
#error CEL_HAVE_ATTRIBUTE cannot be directly set
#endif

// CEL_HAVE_ATTRIBUTE
//
// Wrapper around `__has_attribute` if it exists, otherwise expands to 0.
#ifdef __has_attribute
#define CEL_HAVE_ATTRIBUTE(x) __has_attribute(x)
#else
#define CEL_HAVE_ATTRIBUTE(x) 0
#endif

#ifdef CEL_HAVE_DECLSPEC_ATTRIBUTE
#error CEL_HAVE_DECLSPEC_ATTRIBUTE cannot be directly set
#endif

// CEL_HAVE_DECLSPEC_ATTRIBUTE
//
// Wrapper around `__has_declspec_attribute` if it exists, otherwise expands to
// 0.
#ifdef __has_declspec_attribute
#define CEL_HAVE_DECLSPEC_ATTRIBUTE(x) __has_declspec_attribute(x)
#else
#define CEL_HAVE_DECLSPEC_ATTRIBUTE(x) 0
#endif

#ifdef CEL_HAVE_C_ATTRIBUTE
#error CEL_HAVE_C_ATTRIBUTE cannot be directly set
#endif

// CEL_HAVE_C_ATTRIBUTE
//
// Wrapper around `__has_c_attribute` if it exists, otherwise expands to 0.
#ifdef __has_c_attribute
#define CEL_HAVE_C_ATTRIBUTE(x) __has_c_attribute(x)
#else
#define CEL_HAVE_C_ATTRIBUTE(x) 0
#endif

#ifdef CEL_HAVE_CPP_ATTRIBUTE
#error CEL_HAVE_CPP_ATTRIBUTE cannot be directly set
#endif

// CEL_HAVE_CPP_ATTRIBUTE
//
// Wrapper around `__has_cpp_attribute` if it exists, otherwise expands to 0.
#ifdef __has_cpp_attribute
#define CEL_HAVE_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define CEL_HAVE_CPP_ATTRIBUTE(x) 0
#endif

#ifdef CEL_HAVE_FEATURE
#error CEL_HAVE_FEATURE cannot be directly set
#endif

// CEL_HAVE_FEATURE
//
// Wrapper around `__has_feature` if it exists, otherwise expands to 0.
#ifdef __has_feature
#define CEL_HAVE_FEATURE(x) __has_feature(x)
#else
#define CEL_HAVE_FEATURE(x) 0
#endif

#ifdef CEL_HAVE_INCLUDE
#error CEL_HAVE_INCLUDE cannot be directly set
#endif

// CEL_HAVE_INCLUDE
//
// Wrapper around `__has_include` if it exists, otherwise expands to 0.
#ifdef __has_include
#define CEL_HAVE_INCLUDE(x) __has_include(x)
#else
#define CEL_HAVE_INCLUDE(x) 0
#endif

#ifdef CEL_EXTERN
#error CEL_EXTERN cannot be directly set
#endif

// CEL_EXTERN
//
// Expands to a keyword and/or attribute that ensures the symbol is exported for
// consumption by embedders. It should be used in place of `extern`.
#if (defined(__GNUC__) && !defined(__clang__)) || CEL_HAVE_ATTRIBUTE(visibility)
#define CEL_EXTERN __attribute__((visibility("default"))) extern
#elif defined(_MSC_VER) && defined(CEL_BUILD_DLL)
#define CEL_EXTERN __declspec(dllexport) extern
#elif defined(_MSC_VER) && defined(CEL_CONSUME_DLL)
#define CEL_EXTERN __declspec(dllimport) extern
#else
#define CEL_EXTERN extern
#endif

#ifdef CEL_INLINE
#error CEL_INLINE cannot be directly set
#endif

// CEL_INLINE
//
// Expands to a keyword and/or attribute that strongly suggests that the
// function should be inlined. It should be used in place of `inline` when we
// need to strongly suggest to the compiler to perform inlining. During debug
// builds, this expands to `inline`.
#if ((defined(__GNUC__) && !defined(__clang__)) || \
     CEL_HAVE_ATTRIBUTE(always_inline)) &&         \
    defined(NDEBUG)
#define CEL_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER) && defined(NDEBUG)
#define CEL_INLINE __forceinline
#else
#define CEL_INLINE inline
#endif

#ifdef CEL_LIKELY
#error CEL_LIKELY cannot be directly set
#endif

#ifdef CEL_UNLIKELY
#error CEL_UNLIKELY cannot be directly set
#endif

// CEL_LIKELY/CEL_UNLIKELY
//
// Expands to an intrinsic indicating that the expression is either likely or
// unlikely. The compiler can use this as a hint to perform optimizations such
// as code layout. Should be used only when the compiler would be incapable of
// coming to that conclusion itself.
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_expect)
#define CEL_LIKELY(condition) __builtin_expect((condition) || false, true)
#define CEL_UNLIKELY(condition) __builtin_expect((condition) || false, false)
#else
#define CEL_LIKELY(condition) (condition)
#define CEL_UNLIKELY(condition) (condition)
#endif

#ifdef CEL_ASSUME
#error CEL_ASSUME cannot be directly set
#endif

// CEL_ASSUME
//
// Expands to an intrinsic indicating that the condition is true. The compiler
// is free to use this information to implement optimizations. If the condition
// is actually false at runtime, the behavior is undefined. Use sparingly.
#if CEL_HAVE_BUILTIN(__builtin_assume)
#define CEL_ASSUME(condition) __builtin_assume(condition)
#elif defined(_MSC_VER)
#define CEL_ASSUME(condition) __assume(condition)
#else
#define CEL_ASSUME(condition) \
  do {                        \
    if (false) {              \
      condition;              \
    }                         \
  } while (false)
#endif

#ifdef CEL_ATTRIBUTE_NORETURN
#error CEL_ATTRIBUTE_NORETURN cannot be directly set
#endif

// CEL_ATTRIBUTE_NORETURN
//
// Expands to an attribute indicating that the function never returns.
#if (defined(__GNUC__) && !defined(__clang__)) || CEL_HAVE_ATTRIBUTE(noreturn)
#define CEL_ATTRIBUTE_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define CEL_ATTRIBUTE_NORETURN __declspec(noreturn)
#else
#define CEL_ATTRIBUTE_NORETURN
#endif

#ifdef CEL_ATTRIBUTE_NOINLINE
#error CEL_ATTRIBUTE_NOINLINE cannot be directly set
#endif

// CEL_ATTRIBUTE_NOINLINE
//
// Expands to an attribute indicating that the function should not be inlined.
#if (defined(__GNUC__) && !defined(__clang__)) || CEL_HAVE_ATTRIBUTE(noinline)
#define CEL_ATTRIBUTE_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define CEL_ATTRIBUTE_NOINLINE __declspec(noinline)
#else
#define CEL_ATTRIBUTE_NOINLINE
#endif

#ifdef CEL_ATTRIBUTE_NOTHROW
#error CEL_ATTRIBUTE_NOTHROW cannot be directly set
#endif

// CEL_ATTRIBUTE_NOTHROW
//
// Expands to an attribute indicating that the function does not throw. Similar
// to `noexcept` in C++.
#if (defined(__GNUC__) && !defined(__clang__)) || CEL_HAVE_ATTRIBUTE(nothrow)
#define CEL_ATTRIBUTE_NOTHROW __attribute__((nothrow))
#elif defined(_MSC_VER)
#define CEL_ATTRIBUTE_NOTHROW __declspec(nothrow)
#else
#define CEL_ATTRIBUTE_NOTHROW
#endif

#ifdef CEL_ATTRIBUTE_FALLTHROUGH
#error CEL_ATTRIBUTE_FALLTHROUGH cannot be directly set
#endif

// CEL_ATTRIBUTE_FALLTHROUGH
//
// Expands to an attribute indicating that fallthrough of the case in a select
// block is intentional.
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_ATTRIBUTE(fallthrough)
#define CEL_ATTRIBUTE_FALLTHROUGH __attribute__((fallthrough))
#else
#define CEL_ATTRIBUTE_FALLTHROUGH
#endif

#ifdef CEL_ATTRIBUTE_FORMAT
#error CEL_ATTRIBUTE_FORMAT cannot be directly set
#endif

#ifdef CEL_ATTRIBUTE_VFORMAT
#error CEL_ATTRIBUTE_VFORMAT cannot be directly set
#endif

// CEL_ATTRIBUTE_FORMAT/CEL_ATTRIBUTE_VFORMAT
//
// Expands to an attribute indicating the function is a printf-like function.
#if (defined(__GNUC__) && !defined(__clang__)) || CEL_HAVE_ATTRIBUTE(format)
#define CEL_ATTRIBUTE_FORMAT(fmt, arg) __attribute__((format(printf, fmt, arg)))
#define CEL_ATTRIBUTE_VFORMAT(fmt) __attribute__((format(printf, fmt, 0)))
#else
#define CEL_ATTRIBUTE_FORMAT(fmt, arg)
#define CEL_ATTRIBUTE_VFORMAT(fmt)
#endif

#ifdef CEL_ATTRIBUTE_NODISCARD
#error CEL_ATTRIBUTE_NODISCARD cannot be directly set
#endif

// CEL_ATTRIBUTE_MAYBE_UNUSED
//
// Expands to an attribute indicating the entity must not be discarded.
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_ATTRIBUTE(warn_unused_result)
#define CEL_ATTRIBUTE_NODISCARD __attribute__((warn_unused_result))
#else
#define CEL_ATTRIBUTE_NODISCARD
#endif

#ifdef CEL_ATTRIBUTE_MAYBE_UNUSED
#error CEL_ATTRIBUTE_MAYBE_UNUSED cannot be directly set
#endif

// CEL_ATTRIBUTE_MAYBE_UNUSED
//
// Expands to an attribute indicating the entity may be unused.
#if (defined(__GNUC__) && !defined(__clang__)) || CEL_HAVE_ATTRIBUTE(unused)
#define CEL_ATTRIBUTE_MAYBE_UNUSED __attribute__((unused))
#else
#define CEL_ATTRIBUTE_MAYBE_UNUSED
#endif

#ifdef CEL_ATTRIBUTE_MALLOC
#error CEL_ATTRIBUTE_MALLOC cannot be directly set
#endif

// CEL_ATTRIBUTE_MALLOC
//
// Expands to an attribute indicating the function acts like a system memory
// allocation function. See
// https://clang.llvm.org/docs/AttributeReference.html#malloc.
#if (defined(__GNUC__) && !defined(__clang__)) || CEL_HAVE_ATTRIBUTE(malloc)
#define CEL_ATTRIBUTE_MALLOC __attribute__((malloc))
#else
#define CEL_ATTRIBUTE_MALLOC
#endif

#ifdef CEL_ATTRIBUTE_CONST
#error CEL_ATTRIBUTE_CONST cannot be directly set
#endif

// CEL_ATTRIBUTE_PURE
//
// Expands to an attribute indicating the function is constant.
#if (defined(__GNUC__) && !defined(__clang__)) || CEL_HAVE_ATTRIBUTE(const)
#define CEL_ATTRIBUTE_CONST __attribute__((const))
#else
#define CEL_ATTRIBUTE_CONST
#endif

#ifdef CEL_ATTRIBUTE_PURE
#error CEL_ATTRIBUTE_PURE cannot be directly set
#endif

// CEL_ATTRIBUTE_PURE
//
// Expands to an attribute indicating the function is pure.
#if (defined(__GNUC__) && !defined(__clang__)) || CEL_HAVE_ATTRIBUTE(pure)
#define CEL_ATTRIBUTE_PURE __attribute__((pure))
#else
#define CEL_ATTRIBUTE_PURE
#endif

#ifdef CEL_ATTRIBUTE_CLOSED_ENUM
#error CEL_ATTRIBUTE_CLOSED_ENUM cannot be directly set
#endif

#ifdef CEL_ATTRIBUTE_OPEN_ENUM
#error CEL_ATTRIBUTE_OPEN_ENUM cannot be directly set
#endif

// CEL_ATTRIBUTE_CLOSED_ENUM/CEL_ATTRIBUTE_OPEN_ENUM
//
// Expands to an attribute indicating the enum is open or closed.
#if CEL_HAVE_ATTRIBUTE(enum_extensibility)
#define CEL_ATTRIBUTE_CLOSED_ENUM __attribute__((enum_extensibility(closed)))
#define CEL_ATTRIBUTE_OPEN_ENUM __attribute__((enum_extensibility(open)))
#else
#define CEL_ATTRIBUTE_CLOSED_ENUM
#define CEL_ATTRIBUTE_OPEN_ENUM
#endif

#ifdef CEL_ATTRIBUTE_FLAG_ENUM
#error CEL_ATTRIBUTE_FLAG_ENUM cannot be directly set
#endif

// CEL_ATTRIBUTE_FLAG_ENUM
//
// Expands to an attribute indicating the enum is a flag type.
#if CEL_HAVE_ATTRIBUTE(flag_enum)
#define CEL_ATTRIBUTE_FLAG_ENUM __attribute__((flag_enum))
#else
#define CEL_ATTRIBUTE_FLAG_ENUM
#endif

#ifdef CEL_ATTRIBUTE_PACKED
#error CEL_ATTRIBUTE_PACKED cannot be directly set
#endif

// CEL_ATTRIBUTE_PACKED
//
// Expands to an attribute indicating the fields of the structure should be
// packed tightly, rather than following natural alignment.
#if (defined(__GNUC__) && !defined(__clang__)) || \
    (CEL_HAVE_ATTRIBUTE(packed) && CEL_HAVE_ATTRIBUTE(aligned))
#define CEL_ATTRIBUTE_PACKED(alignment) \
  __attribute__((packed, aligned(alignment)))
#else
#define CEL_ATTRIBUTE_PACKED(alignment)
#endif

// CEL_ATTRIBUTE_PREFERRED_TYPE
#if CEL_HAVE_ATTRIBUTE(preferred_type)
#define CEL_ATTRIBUTE_PREFERRED_TYPE(type) __attribute__((preferred_type(type)))
#else
#define CEL_ATTRIBUTE_PREFERRED_TYPE(type)
#endif

#ifdef CEL_NONNULL
#error CEL_NONNULL cannot be directly set
#endif

#ifdef CEL_NULLABLE
#error CEL_NULLABLE cannot be directly set
#endif

#ifdef CEL_NULLABLE_RESULT
#error CEL_NULLABLE_RESULT cannot be directly set
#endif

#ifdef CEL_NULLABILITY_UNKNOWN
#error CEL_NULLABILITY_UNKNOWN cannot be directly set
#endif

// CEL_NONNULL/CEL_NULLABLE/CEL_NULLABLE_RESULT/CEL_NULLABILITY_UNKNOWN
//
// Nullability annotations. See
// https://clang.llvm.org/docs/AttributeReference.html#nullability-attributes.
#if CEL_HAVE_FEATURE(nullability)
#define cel_nonnull _Nonnull
#define cel_nullable _Nullable
#define cel_nullable_result _Nullable_result
#define cel_nullability_unknown _Null_unspecified
#else
#define cel_nonnull
#define cel_nullable
#define cel_nullable_result
#define cel_nullability_unknown
#endif

#define CEL_NONNULL(x) x cel_nonnull
#define CEL_NULLABLE(x) x cel_nullable
#define CEL_NULLABLE_RESULT(x) x cel_nullable_result
#define CEL_NULLABILITY_UNKNOWN(x) x cel_nullability_unknown

#ifdef cel_restrict
#error cel_restrict cannot be directly set
#endif

#ifdef __cplusplus
#define cel_restrict __restrict
#else
#define cel_restrict restrict
#endif

#ifdef CEL_UNREACHABLE
#error CEL_UNREACHABLE cannot be directly set
#endif

// CEL_UNREACHABLE
//
// Expands to a compiler intrinsic indicating that the line is unreachable under
// normal circumstances. The compiler is free to use this information to
// implement optimizations. If the code is actually reached at runtime, the
// behavior is undefined. Use sparingly.
#if ((defined(__GNUC__) && !defined(__clang__)) || \
     CEL_HAVE_BUILTIN(__builtin_unreachable)) &&   \
    defined(NDEBUG)
#define CEL_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER) && defined(NDEBUG)
#define CEL_UNREACHABLE() __assume(0)
#else
#ifdef __cplusplus
#define CEL_UNREACHABLE() ::std::abort()
#else
#define CEL_UNREACHABLE() abort()
#endif
#endif

#ifdef CEL_USED
#error CEL_USED cannot be directly set
#endif

#ifdef __cplusplus
#define CEL_USED(arg) static_cast<void>(arg)
#else
#define CEL_USED(arg) ((void)(arg))
#endif

#ifdef cel_kMaxAlign
#error cel_kMaxAlign cannot be directly set
#endif

#define cel_kMaxAlign ((size_t)8)

#ifdef cel_arraysize
#error cel_arraysize cannot be directly set
#endif

// cel_arraysize
//
// Expands to the number of elements in the array literal.
#ifdef __cplusplus
template <typename T, std::size_t N>
auto _cel_arraysize(const T (&array)[N]) -> char (&)[N];
#define cel_arraysize(x) (sizeof(::_cel_arraysize((x))))
#else
#define cel_arraysize(x) (sizeof((x)) / sizeof((x)[0]))
#endif

// We require the standards conforming Microsoft C++ preprocessor.
#if (defined(_MSC_VER) && !defined(__clang__)) && \
    (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)
#error Microsoft C++ standards conforming preprocessor is required, pass \
  /Zc:preprocessor to the compiler
#endif

// _CEL_NARGS
//
// Macro which abuses a standards compliant preprocessor to count the number of
// arguments.

#define _CEL_CONCAT_HELPER(v1, v2) v1##v2
#define _CEL_CONCAT(v1, v2) _CEL_CONCAT_HELPER(v1, v2)

#define _CEL_CONCAT5(_0, _1, _2, _3, _4) _0##_1##_2##_3##_4

#define _CEL_IDENTITY_(x) x
#define _CEL_IDENTITY(x) _CEL_IDENTITY_(x)

#define _CEL_VA_ARGS_(...) __VA_ARGS__
#define _CEL_VA_ARGS(...) _CEL_VA_ARGS_(__VA_ARGS__)

#define _CEL_IDENTITY_VA_ARGS_(x, ...) x, __VA_ARGS__
#define _CEL_IDENTITY_VA_ARGS(x, ...) _CEL_IDENTITY_VA_ARGS_(x, __VA_ARGS__)

#define _CEL_IIF_0(x, ...) __VA_ARGS__
#define _CEL_IIF_1(x, ...) x
#define _CEL_IIF(c) _CEL_CONCAT_HELPER(_CEL_IIF_, c)

#define _CEL_HAS_COMMA(...)                                                  \
  _CEL_IDENTITY(_CEL_VA_ARGS_TAIL(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
                                  1, 1, 1, 1, 0))
#define _CEL_IS_EMPTY_TRIGGER_PARENTHESIS_(...) ,

#define _CEL_IS_EMPTY(...)                                            \
  _CEL_IS_EMPTY_(                                                     \
      _CEL_HAS_COMMA(__VA_ARGS__),                                    \
      _CEL_HAS_COMMA(_CEL_IS_EMPTY_TRIGGER_PARENTHESIS_ __VA_ARGS__), \
      _CEL_HAS_COMMA(__VA_ARGS__()),                                  \
      _CEL_HAS_COMMA(_CEL_IS_EMPTY_TRIGGER_PARENTHESIS_ __VA_ARGS__()))

#define _CEL_IS_EMPTY_(_0, _1, _2, _3) \
  _CEL_HAS_COMMA(_CEL_CONCAT5(_CEL_IS_EMPTY_IS_EMPTY_CASE_, _0, _1, _2, _3))
#define _CEL_IS_EMPTY_IS_EMPTY_CASE_0001 ,

#define _CEL_NARGS(...)                 \
  _CEL_IIF(_CEL_IS_EMPTY(__VA_ARGS__))( \
      0, _CEL_VA_ARGS_SIZE_(__VA_ARGS__, _CEL_VA_ARGS_SEQ64()))
#define _CEL_VA_ARGS_SIZE_(...) _CEL_IDENTITY(_CEL_VA_ARGS_TAIL(__VA_ARGS__))

#define _CEL_VA_ARGS_TAIL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, \
                          _12, _13, _14, x, ...)                            \
  x
#define _CEL_VA_ARGS_SEQ64() \
  15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#endif  // THIRD_PARTY_CEL_C_CONFIG_H_
