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

#ifndef THIRD_PARTY_CEL_C_SRC_CONFIG_H_
#define THIRD_PARTY_CEL_C_SRC_CONFIG_H_

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

#endif  // THIRD_PARTY_CEL_C_SRC_CONFIG_H_
