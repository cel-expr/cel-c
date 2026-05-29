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

// Internal header providing byte swapping functions. Uses compiler intrinsics
// when available.

#ifndef THIRD_PARTY_CEL_C_SRC_ENDIAN_H_
#define THIRD_PARTY_CEL_C_SRC_ENDIAN_H_

#include <limits.h>
#include <stdint.h>

#ifdef _MSC_VER
#include <stdlib.h>
#endif

#include "cel-c/assert.h"
#include "cel-c/src/config.h"
#include "cel-c/src/unaligned.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

CEL_BEGIN_DECLS

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM _cel_ByteOrder {
#if defined(__ORDER_BIG_ENDIAN__) && defined(__ORDER_LITTLE_ENDIAN__)
  _cel_ByteOrder_kBigEndian = __ORDER_BIG_ENDIAN__,
  _cel_ByteOrder_kLittleEndian = __ORDER_LITTLE_ENDIAN__,
#else
  _cel_ByteOrder_kBigEndian = 4321,
  _cel_ByteOrder_kLittleEndian = 1234,
#endif
#if defined(_CEL_IS_BIG_ENDIAN)
  _cel_ByteOrder_kNative = _cel_ByteOrder_kBigEndian,
#elif defined(_CEL_IS_LITTLE_ENDIAN)
  _cel_ByteOrder_kNative = _cel_ByteOrder_kLittleEndian,
#else
#error unreachable
#endif
} _cel_ByteOrder;

CEL_STATIC_ASSERT(_cel_ByteOrder_kBigEndian != _cel_ByteOrder_kLittleEndian);
CEL_STATIC_ASSERT(_cel_ByteOrder_kNative == _cel_ByteOrder_kBigEndian ||
                  _cel_ByteOrder_kNative == _cel_ByteOrder_kLittleEndian);

// _cel_ReverseBytes8
//
// Just returns the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint8_t _cel_ReverseBytes8(uint8_t x) { return x; }

// _cel_ReverseBytes16
//
// Reverses the bytes of the value and returns the result.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint16_t _cel_ReverseBytes16(uint16_t x) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_bswap16)
  return __builtin_bswap16(x);
#elif defined(_MSC_VER)
  CEL_STATIC_ASSERT(sizeof(uint16_t) == sizeof(unsigned short));
  return (uint16_t)_byteswap_ushort((unsigned short)x);
#else
  return (((x & ((uint16_t)0xff)) << 8) | ((x & ((uint16_t)0xff00)) >> 8));
#endif
}

// _cel_ReverseBytes32
//
// Reverses the bytes of the value and returns the result.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint32_t _cel_ReverseBytes32(uint32_t x) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_bswap32)
  return __builtin_bswap32(x);
#elif defined(_MSC_VER)
  CEL_STATIC_ASSERT(sizeof(uint32_t) == sizeof(unsigned int));
  return (uint32_t)_byteswap_uint((unsigned int)x);
#else
  return (((x & UINT32_C(0xff)) << 24) | ((x & UINT32_C(0xff00)) << 8) |
          ((x & UINT32_C(0xff0000)) >> 8) | ((x & UINT32_C(0xff000000)) >> 24));
#endif
}

// _cel_ReverseBytes64
//
// Reverses the bytes of the value and returns the result.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint64_t _cel_ReverseBytes64(uint64_t x) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_bswap64)
  return __builtin_bswap64(x);
#elif defined(_MSC_VER)
  CEL_STATIC_ASSERT(sizeof(uint64_t) == sizeof(unsigned __int64));
  return (uint64_t)_byteswap_uint64((unsigned __int64)x);
#else
  return (((x & UINT64_C(0xff)) << 56) | ((x & UINT64_C(0xff00)) << 40) |
          ((x & UINT64_C(0xff0000)) << 24) | ((x & UINT64_C(0xff000000)) << 8) |
          ((x & UINT64_C(0xff00000000)) >> 8) |
          ((x & UINT64_C(0xff0000000000)) >> 24) |
          ((x & UINT64_C(0xff000000000000)) >> 40) |
          ((x & UINT64_C(0xff00000000000000)) >> 56));
#endif
}

// _cel_LittleEndianUnalignedLoad8
//
// Same as _cel_UnalignedLoad8.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint8_t
_cel_LittleEndianUnalignedLoad8(CEL_NONNULL(const void*) data) {
  CEL_ASSERT_NOT_NULL(data);
  return _cel_UnalignedLoad8(data);
}

// _cel_LittleEndianUnalignedLoad16
//
// Performs _cel_UnalignedLoad16. If the platform endianness if big endian,
// _cel_ReverseBytes16 is called before returning the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint16_t
_cel_LittleEndianUnalignedLoad16(CEL_NONNULL(const void*) data) {
  CEL_ASSERT_NOT_NULL(data);
#ifdef _CEL_IS_BIG_ENDIAN
  return _cel_ReverseBytes16(_cel_UnalignedLoad16(data));
#else
  return _cel_UnalignedLoad16(data);
#endif
}

// _cel_LittleEndianUnalignedLoad32
//
// Performs _cel_UnalignedLoad32. If the platform endianness if big endian,
// _cel_ReverseBytes32 is called before returning the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint32_t
_cel_LittleEndianUnalignedLoad32(CEL_NONNULL(const void*) data) {
  CEL_ASSERT_NOT_NULL(data);
#ifdef _CEL_IS_BIG_ENDIAN
  return _cel_ReverseBytes32(_cel_UnalignedLoad32(data));
#else
  return _cel_UnalignedLoad32(data);
#endif
}

// _cel_LittleEndianUnalignedLoad64
//
// Performs _cel_UnalignedLoad64. If the platform endianness if big endian,
// _cel_ReverseBytes64 is called before returning the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint64_t
_cel_LittleEndianUnalignedLoad64(CEL_NONNULL(const void*) data) {
  CEL_ASSERT_NOT_NULL(data);
#ifdef _CEL_IS_BIG_ENDIAN
  return _cel_ReverseBytes64(_cel_UnalignedLoad64(data));
#else
  return _cel_UnalignedLoad64(data);
#endif
}

CEL_END_DECLS

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)

#endif  // THIRD_PARTY_CEL_C_SRC_ENDIAN_H_
