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

// Internal header providing unaligned load and store functions.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_UNALIGNED_H_
#define THIRD_PARTY_CEL_C_INTERNAL_UNALIGNED_H_

#include <stdint.h>
#include <string.h>  // IWYU pragma: keep

#include "cel-c/assert.h"
#include "cel-c/internal/config.h"

#if defined(_CEL_HAVE_ASAN) || defined(_CEL_HAVE_HWASAN) || \
    defined(_CEL_HAVE_TSAN) || defined(_CEL_HAVE_MSAN)
#include <sanitizer/common_interface_defs.h>
#endif

CEL_BEGIN_DECLS

// _cel_UnalignedLoad/_cel_UnalignedStore
//
// Performs an unaligned load or store. The first argument is a pointer to the
// unaligned storage. The second argument is a pointer to the value to be loaded
// to or stored from.
#define _cel_UnalignedLoad(in, out) ((void)memcpy((out), (in), sizeof(*(out))))
#define _cel_UnalignedStore(out, in) ((void)memcpy((out), (in), sizeof(*(in))))

// _cel_UnalignedLoad8
//
// Performs a load of a byte. Since bytes are always aligned, this does nothing
// special.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint8_t _cel_UnalignedLoad8(CEL_NONNULL(const void*) data) {
  CEL_ASSERT_NOT_NULL(data);
  return *((CEL_NONNULL(const uint8_t*))data);
}

// _cel_UnalignedLoad16
//
// Performs an unaligned load of 2 bytes.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint16_t _cel_UnalignedLoad16(CEL_NONNULL(const void*) data) {
  CEL_ASSERT_NOT_NULL(data);
#if defined(_CEL_HAVE_ASAN) || defined(_CEL_HAVE_HWASAN) || \
    defined(_CEL_HAVE_TSAN) || defined(_CEL_HAVE_MSAN)
  return __sanitizer_unaligned_load16(data);
#else
  uint16_t val;
  _cel_UnalignedLoad(data, &val);
  return val;
#endif
}

// _cel_UnalignedLoad32
//
// Performs an unaligned load of 4 bytes.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint32_t _cel_UnalignedLoad32(CEL_NONNULL(const void*) data) {
  CEL_ASSERT_NOT_NULL(data);
#if defined(_CEL_HAVE_ASAN) || defined(_CEL_HAVE_HWASAN) || \
    defined(_CEL_HAVE_TSAN) || defined(_CEL_HAVE_MSAN)
  return __sanitizer_unaligned_load32(data);
#else
  uint32_t val;
  _cel_UnalignedLoad(data, &val);
  return val;
#endif
}

// _cel_UnalignedLoad64
//
// Performs an unaligned load of 8 bytes.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint64_t _cel_UnalignedLoad64(CEL_NONNULL(const void*) data) {
  CEL_ASSERT_NOT_NULL(data);
#if defined(_CEL_HAVE_ASAN) || defined(_CEL_HAVE_HWASAN) || \
    defined(_CEL_HAVE_TSAN) || defined(_CEL_HAVE_MSAN)
  return __sanitizer_unaligned_load64(data);
#else
  uint64_t val;
  _cel_UnalignedLoad(data, &val);
  return val;
#endif
}

// _cel_UnalignedStore8
//
// Performs a store of a byte. Since bytes are always aligned, this does nothing
// special.
static CEL_INLINE void _cel_UnalignedStore8(CEL_NONNULL(void*) data,
                                            uint8_t val) {
  CEL_ASSERT_NOT_NULL(data);
  *((CEL_NONNULL(uint8_t*))data) = val;
}

// _cel_UnalignedStore16
//
// Performs an unaligned store of 2 bytes.
static CEL_INLINE void _cel_UnalignedStore16(CEL_NONNULL(void*) data,
                                             uint16_t val) {
  CEL_ASSERT_NOT_NULL(data);
#if defined(_CEL_HAVE_ASAN) || defined(_CEL_HAVE_HWASAN) || \
    defined(_CEL_HAVE_TSAN) || defined(_CEL_HAVE_MSAN)
  __sanitizer_unaligned_store16(data, val);
#else
  _cel_UnalignedStore(data, &val);
#endif
}

// _cel_UnalignedStore32
//
// Performs an unaligned store of 4 bytes.
static CEL_INLINE void _cel_UnalignedStore32(CEL_NONNULL(void*) data,
                                             uint32_t val) {
  CEL_ASSERT_NOT_NULL(data);
#if defined(_CEL_HAVE_ASAN) || defined(_CEL_HAVE_HWASAN) || \
    defined(_CEL_HAVE_TSAN) || defined(_CEL_HAVE_MSAN)
  __sanitizer_unaligned_store32(data, val);
#else
  _cel_UnalignedStore(data, &val);
#endif
}

// _cel_UnalignedStore64
//
// Performs an unaligned store of 8 bytes.
static CEL_INLINE void _cel_UnalignedStore64(CEL_NONNULL(void*) data,
                                             uint64_t val) {
  CEL_ASSERT_NOT_NULL(data);
#if defined(_CEL_HAVE_ASAN) || defined(_CEL_HAVE_HWASAN) || \
    defined(_CEL_HAVE_TSAN) || defined(_CEL_HAVE_MSAN)
  __sanitizer_unaligned_store64(data, val);
#else
  _cel_UnalignedStore(data, &val);
#endif
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_UNALIGNED_H_
