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

#include "cel-c/arena.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/internal/align.h"
#include "cel-c/internal/ckdint.h"
#include "upb/base/string_view.h"
#include "upb/mem/arena.h"

extern "C" CEL_NULLABLE(void*)
    cel_Arena_Calloc(CEL_NONNULL(cel_Arena*) arena, size_t num, size_t size,
                     CEL_NULLABLE(size_t*) actual_num) {
  CEL_ASSERT_NOT_NULL(arena);

  if (CEL_UNLIKELY(num == 0 || size == 0)) {
    if (actual_num != cel_nullptr) {
      *actual_num = 0;
    }
    return cel_nullptr;
  }
  size_t total_size;
  if (CEL_UNLIKELY(_cel_ckd_mul(&total_size, num, size))) {
    if (actual_num != cel_nullptr) {
      *actual_num = 0;
    }
    return cel_nullptr;
  }
  CEL_NULLABLE(void*) addr = upb_Arena_Malloc(arena, total_size);
  CEL_ASSERT(addr == cel_nullptr || _cel_is_aligned(addr, cel_Arena_kMaxAlign));
  if (CEL_LIKELY(addr != cel_nullptr)) {
    memset(addr, '\0', total_size);
  }
  if (actual_num != cel_nullptr) {
    *actual_num = addr != cel_nullptr ? num : 0;
  }
  return addr;
}

extern "C" CEL_NULLABLE(void*)
    cel_Arena_Realloc(CEL_NONNULL(cel_Arena*) arena, CEL_NULLABLE(void*) addr,
                      size_t old_size, size_t new_size,
                      CEL_NULLABLE(size_t*) actual_size) {
  CEL_ASSERT_NOT_NULL(arena);

  if (old_size == 0) {
    CEL_ASSERT_NULL(addr);
    return cel_Arena_Malloc(arena, new_size, actual_size);
  }
  CEL_NULLABLE(void*)
  new_addr = upb_Arena_Realloc(arena, addr, old_size, new_size);
  CEL_ASSERT(addr == cel_nullptr || _cel_is_aligned(addr, cel_Arena_kMaxAlign));
  if (actual_size != cel_nullptr) {
    *actual_size = new_addr != cel_nullptr ? new_size : 0;
  }
  return new_size != 0 ? new_addr : cel_nullptr;
}

extern "C" void cel_Arena_FreeSized(CEL_NONNULL(cel_Arena*) arena,
                                    CEL_NULLABLE(void*) addr, size_t size) {
  CEL_ASSERT_NOT_NULL(arena);

  if (CEL_UNLIKELY(size == 0)) {
    CEL_ASSERT_NULL(addr);
    return;
  }
  CEL_ASSERT_NOT_NULL(addr);
  CEL_USED(upb_Arena_Realloc(arena, addr, size, 0));
}

extern "C" bool cel_Arena_VPrintF(CEL_NONNULL(cel_Arena*) arena,
                                  CEL_NONNULL(upb_StringView*) out,
                                  CEL_NONNULL(const char*) fmt, va_list args) {
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_NOT_NULL(out);
  CEL_ASSERT_NOT_NULL(fmt);

  char buf[128];
  int size;
  {
    va_list args_copy;
    va_copy(args_copy, args);
    size = vsnprintf(buf, sizeof(buf), fmt, args_copy);
    va_end(args_copy);
  }
  CEL_ASSERT_GE(size, 0);
  if (CEL_UNLIKELY(size < 0)) {
    return false;
  }
  if (CEL_UNLIKELY(size == 0)) {
    *out = upb_StringView_FromDataAndSize("", 0);
    return true;
  }
  const bool fit = size < sizeof(buf);
  CEL_NULLABLE(char*)
  str = (CEL_NULLABLE(char*))cel_Arena_Malloc(
      arena, ((size_t)size) + (fit ? 0 : 1), cel_nullptr);
  if (CEL_UNLIKELY(str == cel_nullptr)) {
    return false;
  }
  if (fit) {
    memcpy(str, buf, (size_t)size);
  } else {
    const int size_copy = vsnprintf(str, ((size_t)size) + 1, fmt, args);
    CEL_ASSERT_EQ(size_copy, size);
  }
  *out = upb_StringView_FromDataAndSize(str, (size_t)size);
  return true;
}
