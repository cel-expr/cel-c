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

#include "cel-c/src/memory.h"

#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/src/config.h"

// It is hard to exhaustively detect the presence of `memrchr` as it's a GNU
// extension which has been implemented on more than just GNU/Linux, such as
// FreeBSD. Instead we define a implementation of memrchr ourselves. On
// platforms that have weak symbols, we mark our implementation as weak so if
// the platform libc has `memrchr` the linker will select the one from libc. On
// other platforms, we mark are implementation static inline so it is private.
#if _CEL_HAVE_ATTRIBUTE_WEAK
_CEL_ATTRIBUTE_WEAK CEL_EXTERN
#else
static inline
#endif
    void*
    memrchr(const void* b, int c, size_t len) {
  if (len > 0) {
    const unsigned char* p = ((const unsigned char*)b) + len;
    do {
      --p;
      if (*p == (unsigned char)c) {
        return (void*)p;
      }
      --len;
    } while (len > 0);
  }
  return cel_nullptr;
}

bool _cel_Memory_Equals(CEL_NULLABLE(const void*) lhs_data, size_t lhs_size,
                        CEL_NULLABLE(const void*) rhs_data, size_t rhs_size) {
  CEL_ASSERT(lhs_size == 0 || lhs_data != cel_nullptr);
  CEL_ASSERT(rhs_size == 0 || rhs_data != cel_nullptr);
  return lhs_size == rhs_size && (lhs_size == 0 || lhs_data == rhs_data ||
                                  memcmp(lhs_data, rhs_data, lhs_size) == 0);
}

int _cel_Memory_Compare(CEL_NULLABLE(const void*) lhs_data, size_t lhs_size,
                        CEL_NULLABLE(const void*) rhs_data, size_t rhs_size) {
  CEL_ASSERT(lhs_size == 0 || lhs_data != cel_nullptr);
  CEL_ASSERT(rhs_size == 0 || rhs_data != cel_nullptr);
  int cmp = lhs_data != rhs_data
                ? memcmp(lhs_data, rhs_data,
                         lhs_size < rhs_size ? lhs_size : rhs_size)
                : 0;
  if (cmp == 0) {
    cmp = lhs_size < rhs_size ? -1 : lhs_size > rhs_size ? 1 : 0;
  }
  return cmp;
}

bool _cel_Memory_StartsWith(CEL_NULLABLE(const void*) hay_data, size_t hay_size,
                            CEL_NULLABLE(const void*) ndl_data,
                            size_t ndl_size) {
  CEL_ASSERT(hay_size == 0 || hay_data != cel_nullptr);
  CEL_ASSERT(ndl_size == 0 || ndl_data != cel_nullptr);
  return hay_size >= ndl_size && memcmp(hay_data, ndl_data, ndl_size) == 0;
}

bool _cel_Memory_EndsWith(CEL_NULLABLE(const void*) hay_data, size_t hay_size,
                          CEL_NULLABLE(const void*) ndl_data, size_t ndl_size) {
  CEL_ASSERT(hay_size == 0 || hay_data != cel_nullptr);
  CEL_ASSERT(ndl_size == 0 || ndl_data != cel_nullptr);
  return hay_size >= ndl_size &&
         memcmp(((CEL_NULLABLE(const char*))hay_data) + (hay_size - ndl_size),
                ndl_data, ndl_size) == 0;
}

CEL_NULLABLE(void*)
_cel_Memory_FindFirst(CEL_NULLABLE(const void*) hay_data, size_t hay_size,
                      CEL_NULLABLE(const void*) ndl_data, size_t ndl_size) {
  CEL_ASSERT(hay_size == 0 || hay_data != cel_nullptr);
  CEL_ASSERT(ndl_size == 0 || ndl_data != cel_nullptr);
  if (CEL_UNLIKELY(ndl_size == 0)) {
    return (CEL_NULLABLE(void*))hay_data;
  }
  if (ndl_size > hay_size) {
    return cel_nullptr;
  }
  if (CEL_UNLIKELY(hay_data == ndl_data)) {
    CEL_ASSERT_GE(hay_size, ndl_size);
    return (CEL_NULLABLE(void*))hay_data;
  }
  if (hay_size > ndl_size) {
    const unsigned char ndl_front =
        *((CEL_NONNULL(const unsigned char*))ndl_data);
    do {
      CEL_NULLABLE(char*)
      hay_next = (CEL_NULLABLE(char*))memchr(hay_data, ndl_front,
                                             (hay_size - ndl_size) + 1);
      if (hay_next == cel_nullptr) {
        return cel_nullptr;
      }
      if (memcmp(hay_next + 1, ((CEL_NONNULL(char*))ndl_data) + 1,
                 ndl_size - 1) == 0) {
        return hay_next;
      }
      size_t skip = (hay_next - ((CEL_NONNULL(const char*))hay_data)) + 1;
      hay_data = ((CEL_NONNULL(const char*))hay_data) + skip;
      hay_size -= skip;
    } while (hay_size > ndl_size);
  }
  CEL_ASSERT_LE(hay_size, ndl_size);
  if (hay_size == ndl_size) {
    return memcmp(hay_data, ndl_data, ndl_size) == 0
               ? (CEL_NULLABLE(void*))hay_data
               : (CEL_NULLABLE(void*))cel_nullptr;
  }
  return cel_nullptr;
}

CEL_NULLABLE(void*)
_cel_Memory_FindLast(CEL_NULLABLE(const void*) hay_data, size_t hay_size,
                     CEL_NULLABLE(const void*) ndl_data, size_t ndl_size) {
  CEL_ASSERT(hay_size == 0 || hay_data != cel_nullptr);
  CEL_ASSERT(ndl_size == 0 || ndl_data != cel_nullptr);
  if (CEL_UNLIKELY(ndl_size == 0)) {
    return (CEL_NULLABLE(void*))(((CEL_NULLABLE(const char*))hay_data));
  }
  if (ndl_size > hay_size) {
    return cel_nullptr;
  }
  if (hay_size > ndl_size) {
    const unsigned char ndl_front =
        *((CEL_NONNULL(const unsigned char*))ndl_data);
    do {
      const size_t hay_lim = (hay_size - ndl_size) + 1;
      CEL_NULLABLE(char*)
      hay_prev = (CEL_NULLABLE(char*))memrchr(hay_data, ndl_front, hay_lim);
      if (hay_prev == cel_nullptr) {
        return cel_nullptr;
      }
      if (memcmp(hay_prev + 1, ((CEL_NONNULL(char*))ndl_data) + 1,
                 ndl_size - 1) == 0) {
        return hay_prev;
      }
      size_t skip =
          ((((CEL_NULLABLE(const char*))hay_data) + hay_lim) - hay_prev) + 1;
      hay_size -= skip;
    } while (hay_size > ndl_size);
  }
  CEL_ASSERT_LE(hay_size, ndl_size);
  if (hay_size == ndl_size) {
    return memcmp(hay_data, ndl_data, ndl_size) == 0
               ? (CEL_NULLABLE(void*))hay_data
               : (CEL_NULLABLE(void*))cel_nullptr;
  }
  return cel_nullptr;
}
