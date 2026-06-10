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

#include "cel-c/alloc.h"

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/src/ckdint.h"
#include "cel-c/src/malloc.h"
#include "upb/mem/alloc.h"

extern "C" CEL_NULLABLE(void*)
    cel_Allocator_Calloc(CEL_NONNULL(cel_Allocator*) alloc, size_t num,
                         size_t size, CEL_NULLABLE(size_t*) actual_num) {
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_NOT_NULL(alloc->func);

  size_t total_size;
  if (CEL_UNLIKELY(_cel_ckd_mul(&total_size, num, size))) {
    errno = ENOMEM;
    return cel_nullptr;
  }
  void* addr = (*alloc->func)(alloc, cel_nullptr, 0, total_size, actual_num);
  if (actual_num != cel_nullptr && addr != cel_nullptr) {
    *actual_num /= size;
  }
  return addr;
}

static CEL_NULLABILITY_UNKNOWN(void*) _cel_DefaultAllocator_AllocFunc(
    CEL_NULLABILITY_UNKNOWN(upb_alloc*) alloc,
    CEL_NULLABILITY_UNKNOWN(void*) ptr, size_t oldsize, size_t size,
    size_t* cel_nullability_unknown actual_size) {
  CEL_ASSERT_NOT_NULL(alloc);

  if (ptr == cel_nullptr) {
    CEL_ASSERT_EQ(oldsize, 0);
    return _cel_Malloc(size, actual_size);
  }
  if (size == 0) {
    if (oldsize != 0) {
      _cel_FreeSized(ptr, oldsize);
    } else {
      _cel_Free(ptr);
    }
    if (actual_size != cel_nullptr) {
      *actual_size = 0;
    }
    return cel_nullptr;
  }
  return _cel_Realloc(ptr, oldsize, size, actual_size);
}

static cel_Allocator _cel_DefaultAllocator = {
    .func = &_cel_DefaultAllocator_AllocFunc,
};

extern "C" CEL_NONNULL(cel_Allocator*) const cel_DefaultAllocator =
    &_cel_DefaultAllocator;

extern "C" CEL_NULLABLE(char*)
    cel_Allocator_StrDup(CEL_NONNULL(cel_Allocator*) alloc,
                         CEL_NULLABLE(size_t*) actual_size,
                         CEL_NONNULL(const char*) str) {
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_NOT_NULL(str);
  size_t len = strlen(str) + 1;
  CEL_NULLABLE(char*)
  dup = (CEL_NULLABLE(char*))cel_Allocator_Malloc(alloc, len, actual_size);
  if (CEL_UNLIKELY(dup == cel_nullptr)) {
    return cel_nullptr;
  }
  memcpy(dup, str, len);
  return dup;
}

extern "C" CEL_NULLABLE(char*)
    cel_Allocator_PrintF(CEL_NONNULL(cel_Allocator*) alloc,
                         CEL_NULLABLE(size_t*) actual_size,
                         CEL_NONNULL(const char*) fmt, ...) {
  va_list args;
  va_start(args, fmt);
  CEL_NULLABLE(char*)
  str = cel_Allocator_VPrintF(alloc, actual_size, fmt, args);
  va_end(args);
  return str;
}

extern "C" CEL_NULLABLE(char*)
    cel_Allocator_VPrintF(CEL_NONNULL(cel_Allocator*) alloc,
                          CEL_NULLABLE(size_t*) actual_size,
                          CEL_NONNULL(const char*) fmt, va_list args) {
  CEL_ASSERT_NOT_NULL(alloc);
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
    errno = EINVAL;
    if (actual_size != cel_nullptr) {
      *actual_size = 0;
    }
    return cel_nullptr;
  }
  CEL_NULLABLE(char*)
  str = (CEL_NULLABLE(char*))cel_Allocator_Malloc(alloc, ((size_t)size) + 1,
                                                  actual_size);
  if (CEL_UNLIKELY(str == cel_nullptr)) {
    return cel_nullptr;
  }
  if (size < sizeof(buf)) {
    memcpy(str, buf, ((size_t)size) + 1);
  } else {
    const int size_copy = vsnprintf(str, ((size_t)size) + 1, fmt, args);
    CEL_ASSERT_EQ(size_copy, size);
  }
  return str;
}
