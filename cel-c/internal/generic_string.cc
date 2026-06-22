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

#include "cel-c/internal/generic_string.h"

#include <limits.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/internal/asan.h"
#include "cel-c/internal/ckdint.h"
#include "cel-c/string_view.h"

static void _cel_GenericString_AnnotateNew(CEL_NONNULL(char*) str, size_t len,
                                           size_t cap) {
  _cel_sanitizer_annotate_contiguous_container(str, str + cap + 1,
                                               str + cap + 1, str + len + 1);
}

static void _cel_GenericString_Annotate(CEL_NONNULL(char*) str, size_t old_len,
                                        size_t new_len, size_t cap) {
  _cel_sanitizer_annotate_contiguous_container(
      str, str + cap + 1, str + old_len + 1, str + new_len + 1);
}

static void _cel_GenericString_AnnotateDelete(CEL_NONNULL(char*) str,
                                              size_t len, size_t cap) {
  _cel_sanitizer_annotate_contiguous_container(str, str + cap + 1,
                                               str + len + 1, str + cap + 1);
}

static void _cel_GenericString_AnnotateLargeNew(CEL_NONNULL(char*) str,
                                                size_t len, size_t cap) {
  _cel_GenericString_AnnotateNew(str, len, cap);
}

static void _cel_GenericString_AnnotateLarge(CEL_NONNULL(char*) str,
                                             size_t old_len, size_t new_len,
                                             size_t cap) {
  _cel_GenericString_Annotate(str, old_len, new_len, cap);
}

static void _cel_GenericString_AnnotateLargeDelete(CEL_NONNULL(char*) str,
                                                   size_t len, size_t cap) {
  _cel_GenericString_AnnotateDelete(str, len, cap);
}

extern "C" void _cel_GenericString_Construct(CEL_NONNULL(_cel_GenericString*)
                                                 str) {
  CEL_ASSERT_NOT_NULL(str);

  str->small.data[0] = '\0';
  str->small.len = 0;
  str->small.is_large = 0;
}

extern "C" void _cel_GenericString_DestructAllocator(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Allocator*) alloc) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(alloc);

  if (str->large.is_large) {
    _cel_GenericString_AnnotateLargeDelete(str->large.data, str->large.len,
                                           str->large.cap);
    cel_Allocator_FreeSized(alloc, str->large.data, str->large.cap + 1);
  }
}

extern "C" void _cel_GenericString_Clear(CEL_NONNULL(_cel_GenericString*) str) {
  CEL_ASSERT_NOT_NULL(str);

  if (str->large.is_large) {
    // Avoid the likely cache miss if we do not actually need to write '\0'.
    if (str->large.len > 0) {
      _cel_GenericString_AnnotateLarge(str->large.data, str->large.len, 0,
                                       str->large.cap);
      str->large.data[0] = '\0';
      str->large.len = 0;
    }
  } else {
    str->small.data[0] = '\0';
    str->small.len = 0;
  }
}

extern "C" void _cel_GenericString_ResetAllocator(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Allocator*) alloc) {
  _cel_GenericString_DestructAllocator(str, alloc);
  _cel_GenericString_Construct(str);
}

extern "C" bool _cel_GenericString_ShrinkToFitAllocator(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Allocator*) alloc) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(alloc);

  if (str->large.is_large && str->large.cap > str->large.len) {
    if (str->large.len <= _cel_GenericStringSmall_kCapacity) {
      CEL_NONNULL(char*) data = str->large.data;
      size_t len = str->large.len;
      size_t cap = str->large.cap;
      memcpy(str->small.data, data, len + 1);
      str->small.len = len;
      str->small.is_large = 0;
      _cel_GenericString_AnnotateLargeDelete(data, len, cap);
      cel_Allocator_FreeSized(alloc, data, cap + 1);
      return true;
    }
    size_t new_cap = str->large.cap;
    size_t actual_new_cap;
    CEL_NULLABLE(char*)
    data = reinterpret_cast<char*>(
        cel_Allocator_Malloc(alloc, new_cap + 1, &actual_new_cap));
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      return false;
    }
    --actual_new_cap;
    if (actual_new_cap >= str->large.cap) {
      cel_Allocator_FreeSized(alloc, data, actual_new_cap + 1);
      return true;
    }
    _cel_GenericString_AnnotateLargeNew(data, str->large.len, actual_new_cap);
    memcpy(data, str->large.data, str->large.len + 1);
    _cel_GenericString_AnnotateLargeDelete(str->large.data, str->large.len,
                                           str->large.cap);
    cel_Allocator_FreeSized(alloc, str->large.data, str->large.cap + 1);
    str->large.data = data;
    str->large.cap = actual_new_cap;
  }

  return true;
}

extern "C" bool _cel_GenericString_AssignAllocator(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Allocator*) alloc,
    cel_StringView val) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(alloc);

  size_t new_len = cel_StringView_Size(val);

  if (new_len == 0) {
    _cel_GenericString_Clear(str);
    return true;
  }

  if (str->large.is_large) {
    if (new_len > str->large.cap) {
      size_t new_cap = new_len;
      size_t actual_new_cap;
      CEL_NULLABLE(char*)
      data = reinterpret_cast<char*>(
          cel_Allocator_Malloc(alloc, new_cap + 1, &actual_new_cap));
      if (CEL_UNLIKELY(data == cel_nullptr)) {
        _cel_GenericString_AnnotateLargeNew(str->large.data, str->large.len,
                                            str->large.cap);
        str->large.data[str->large.len] = '\0';
        return false;
      }
      --actual_new_cap;
      _cel_GenericString_AnnotateLargeNew(data, new_len, actual_new_cap);
      _cel_GenericString_AnnotateLargeDelete(str->large.data, str->large.len,
                                             str->large.cap);
      cel_Allocator_FreeSized(alloc, str->large.data, str->large.cap + 1);
      str->large.data = data;
      str->large.cap = actual_new_cap;
    }
  } else {
    if (new_len <= _cel_GenericStringSmall_kCapacity) {
      memcpy(str->small.data, cel_StringView_Data(val), new_len);
      str->small.data[new_len] = '\0';
      str->small.len = new_len;
      return true;
    }
    size_t new_cap = new_len;
    size_t actual_new_cap;
    CEL_NULLABLE(char*)
    data = reinterpret_cast<char*>(
        cel_Allocator_Malloc(alloc, new_cap + 1, &actual_new_cap));
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      str->small.data[str->small.len] = '\0';
      return false;
    }
    --actual_new_cap;
    _cel_GenericString_AnnotateLargeNew(data, new_len, actual_new_cap);
    str->large.data = data;
    str->large.cap = actual_new_cap;
    str->large.is_large = 1;
  }

  // If we got here, the string **must** be large.
  memcpy(str->large.data, cel_StringView_Data(val), new_len);
  str->large.data[new_len] = '\0';
  str->large.len = new_len;
  return true;
}

extern "C" bool _cel_GenericString_AssignArena(CEL_NONNULL(_cel_GenericString*)
                                                   str,
                                               CEL_NONNULL(cel_Arena*) arena,
                                               cel_StringView val) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(arena);

  size_t new_len = cel_StringView_Size(val);

  if (new_len == 0) {
    _cel_GenericString_Clear(str);
    return true;
  }

  if (str->large.is_large) {
    if (new_len > str->large.cap) {
      size_t new_cap = new_len;
      CEL_NULLABLE(char*)
      data = reinterpret_cast<char*>(
          cel_Arena_Malloc(arena, new_cap + 1, cel_nullptr));
      if (CEL_UNLIKELY(data == cel_nullptr)) {
        _cel_GenericString_AnnotateLargeNew(str->large.data, str->large.len,
                                            str->large.cap);
        str->large.data[str->large.len] = '\0';
        return false;
      }
      _cel_GenericString_AnnotateLargeNew(data, new_len, new_cap);
      _cel_GenericString_AnnotateLargeDelete(str->large.data, str->large.len,
                                             str->large.cap);
      cel_Arena_FreeSized(arena, str->large.data, str->large.cap + 1);
      str->large.data = data;
      str->large.cap = new_cap;
    }
  } else {
    if (new_len <= _cel_GenericStringSmall_kCapacity) {
      memcpy(str->small.data, cel_StringView_Data(val), new_len);
      str->small.data[new_len] = '\0';
      str->small.len = new_len;
      return true;
    }
    size_t new_cap = new_len;
    CEL_NULLABLE(char*)
    data = reinterpret_cast<char*>(
        cel_Arena_Malloc(arena, new_cap + 1, cel_nullptr));
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      str->small.data[str->small.len] = '\0';
      return false;
    }
    _cel_GenericString_AnnotateLargeNew(data, new_len, new_cap);
    str->large.data = data;
    str->large.cap = new_cap;
    str->large.is_large = 1;
  }

  // If we got here, the string **must** be large.
  memcpy(str->large.data, cel_StringView_Data(val), new_len);
  str->large.data[new_len] = '\0';
  str->large.len = new_len;
  return true;
}

extern "C" ptrdiff_t _cel_GenericString_VAppendFAllocator(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Allocator*) alloc,
    CEL_NONNULL(const char*) fmt, va_list args) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_NOT_NULL(fmt);

  int n;
  {
    va_list args_copy;
    va_copy(args_copy, args);
    if (str->large.is_large) {
      // We need to use the remaining capacity. Unpoison it all.
      _cel_GenericString_AnnotateLargeDelete(str->large.data, str->large.len,
                                             str->large.cap);
      n = vsnprintf(str->large.data + str->large.len,
                    (str->large.cap - str->large.len) + 1, fmt, args_copy);
    } else {
      n = vsnprintf(str->small.data + str->small.len,
                    (_cel_GenericStringSmall_kCapacity - str->small.len) + 1,
                    fmt, args_copy);
    }
    va_end(args_copy);
  }

  if (n <= 0) {
    // Repoison and trust no one, re-terminate the string with NIL.
    if (str->large.is_large) {
      _cel_GenericString_AnnotateLargeNew(str->large.data, str->large.len,
                                          str->large.cap);
      str->large.data[str->large.len] = '\0';
    } else {
      str->small.data[str->small.len] = '\0';
    }
    return n;
  }

  if (str->large.is_large) {
    if ((size_t)n < (str->large.cap - str->large.len) + 1) {
      // Fit in the remaining capacity.
      size_t new_len = str->large.len + (size_t)n;
      _cel_GenericString_AnnotateLargeNew(str->large.data, new_len,
                                          str->large.cap);
      str->large.data[new_len] = '\0';
      str->large.len = new_len;
      return n;
    }
    size_t new_cap = str->large.len + (size_t)n;
    size_t actual_new_cap;
    CEL_NULLABLE(char*)
    data = reinterpret_cast<char*>(
        cel_Allocator_Malloc(alloc, new_cap + 1, &actual_new_cap));
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      _cel_GenericString_AnnotateLargeNew(str->large.data, str->large.len,
                                          str->large.cap);
      str->large.data[str->large.len] = '\0';
      return -1;
    }
    --actual_new_cap;
    _cel_GenericString_AnnotateLargeNew(data, new_cap, actual_new_cap);
    memcpy(data, str->large.data, str->large.len);
    _cel_GenericString_AnnotateLargeDelete(str->large.data, str->large.len,
                                           str->large.cap);
    cel_Allocator_FreeSized(alloc, str->large.data, str->large.cap + 1);
    str->large.data = data;
    str->large.cap = actual_new_cap;
  } else {
    if ((size_t)n < (_cel_GenericStringSmall_kCapacity - str->small.len) + 1) {
      size_t new_len = str->small.len + (size_t)n;
      // Fit in the remaining capacity.
      str->small.data[new_len] = '\0';
      str->small.len = new_len;
      return n;
    }
    size_t len = str->small.len;
    size_t new_cap = len + (size_t)n;
    size_t actual_new_cap;
    CEL_NULLABLE(char*)
    data = reinterpret_cast<char*>(
        cel_Allocator_Malloc(alloc, new_cap + 1, &actual_new_cap));
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      str->small.data[len] = '\0';
      return -1;
    }
    --actual_new_cap;
    _cel_GenericString_AnnotateLargeNew(data, new_cap, actual_new_cap);
    memcpy(data, str->small.data, len);
    str->large.data = data;
    str->large.len = len;
    str->large.cap = actual_new_cap;
    str->large.is_large = 1;
  }

  // If we got here, the string **must** be large.
  CEL_ASSERT(str->large.is_large);
  const int n2 = vsnprintf(str->large.data + str->large.len,
                           (str->large.cap - str->large.len) + 1, fmt, args);
  CEL_ASSERT_EQ(n2, n);
  n = n2;
  size_t new_len = str->large.len + (size_t)n;
  CEL_ASSERT((size_t)n < (str->large.cap - str->large.len) + 1);
  str->large.data[new_len] = '\0';
  str->large.len = new_len;
  return n;
}

extern "C" ptrdiff_t _cel_GenericString_VAppendFArena(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Arena*) arena,
    CEL_NONNULL(const char*) fmt, va_list args) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_NOT_NULL(fmt);

  int n;
  {
    va_list args_copy;
    va_copy(args_copy, args);
    if (str->large.is_large) {
      // We need to use the remaining capacity. Unpoison it all.
      _cel_GenericString_AnnotateLargeDelete(str->large.data, str->large.len,
                                             str->large.cap);
      n = vsnprintf(str->large.data + str->large.len,
                    (str->large.cap - str->large.len) + 1, fmt, args_copy);
    } else {
      n = vsnprintf(str->small.data + str->small.len,
                    (_cel_GenericStringSmall_kCapacity - str->small.len) + 1,
                    fmt, args_copy);
    }
    va_end(args_copy);
  }

  if (n <= 0) {
    // Repoison and trust no one, re-terminate the string with NIL.
    if (str->large.is_large) {
      _cel_GenericString_AnnotateLargeNew(str->large.data, str->large.len,
                                          str->large.cap);
      str->large.data[str->large.len] = '\0';
    } else {
      str->small.data[str->small.len] = '\0';
    }
    return n;
  }

  if (str->large.is_large) {
    if ((size_t)n < (str->large.cap - str->large.len) + 1) {
      // Fit in the remaining capacity.
      size_t new_len = str->large.len + (size_t)n;
      _cel_GenericString_AnnotateLargeNew(str->large.data, new_len,
                                          str->large.cap);
      str->large.data[new_len] = '\0';
      str->large.len = new_len;
      return n;
    }
    size_t new_cap = str->large.len + (size_t)n;
    CEL_NULLABLE(char*)
    data = reinterpret_cast<char*>(
        cel_Arena_Malloc(arena, new_cap + 1, cel_nullptr));
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      _cel_GenericString_AnnotateLargeNew(str->large.data, str->large.len,
                                          str->large.cap);
      str->large.data[str->large.len] = '\0';
      return -1;
    }
    _cel_GenericString_AnnotateLargeNew(data, new_cap, new_cap);
    memcpy(data, str->large.data, str->large.len);
    _cel_GenericString_AnnotateLargeDelete(str->large.data, str->large.len,
                                           str->large.cap);
    cel_Arena_FreeSized(arena, str->large.data, str->large.cap + 1);
    str->large.data = data;
    str->large.cap = new_cap;
  } else {
    if ((size_t)n < (_cel_GenericStringSmall_kCapacity - str->small.len) + 1) {
      size_t new_len = str->small.len + (size_t)n;
      // Fit in the remaining capacity.
      str->small.data[new_len] = '\0';
      str->small.len = new_len;
      return n;
    }
    size_t len = str->small.len;
    size_t new_cap = len + (size_t)n;
    CEL_NULLABLE(char*)
    data = reinterpret_cast<char*>(
        cel_Arena_Malloc(arena, new_cap + 1, cel_nullptr));
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      str->small.data[len] = '\0';
      return -1;
    }
    _cel_GenericString_AnnotateLargeNew(data, new_cap, new_cap);
    memcpy(data, str->small.data, len);
    str->large.data = data;
    str->large.len = len;
    str->large.cap = new_cap;
    str->large.is_large = 1;
  }

  // If we got here, the string **must** be large.
  CEL_ASSERT(str->large.is_large);
  const int n2 = vsnprintf(str->large.data + str->large.len,
                           (str->large.cap - str->large.len) + 1, fmt, args);
  CEL_ASSERT_EQ(n2, n);
  n = n2;
  size_t new_len = str->large.len + (size_t)n;
  CEL_ASSERT((size_t)n < (str->large.cap - str->large.len) + 1);
  str->large.data[new_len] = '\0';
  str->large.len = new_len;
  return n;
}

extern "C" bool _cel_GenericString_StabilizeAllocator(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Allocator*) alloc) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(alloc);

  if (str->large.is_large) {
    return true;
  }
  size_t len = str->small.len;
  size_t actual_new_cap;
  CEL_NULLABLE(char*)
  data = reinterpret_cast<char*>(
      cel_Allocator_Malloc(alloc, len + 1, &actual_new_cap));
  if (CEL_UNLIKELY(data == cel_nullptr)) {
    return false;
  }
  --actual_new_cap;
  _cel_GenericString_AnnotateLargeNew(data, len, actual_new_cap);
  memcpy(data, str->small.data, str->small.len + 1);
  str->large.is_large = 1;
  str->large.len = len;
  str->large.cap = actual_new_cap;
  str->large.data = data;
  return true;
}

extern "C" bool _cel_GenericString_StabilizeArena(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Arena*) arena) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(arena);

  if (str->large.is_large) {
    return true;
  }
  size_t len = str->small.len;
  size_t actual_new_cap;
  CEL_NULLABLE(char*)
  data = reinterpret_cast<char*>(
      cel_Arena_Malloc(arena, len + 1, &actual_new_cap));
  if (CEL_UNLIKELY(data == cel_nullptr)) {
    return false;
  }
  --actual_new_cap;
  _cel_GenericString_AnnotateLargeNew(data, len, actual_new_cap);
  memcpy(data, str->small.data, str->small.len + 1);
  str->large.is_large = 1;
  str->large.len = len;
  str->large.cap = actual_new_cap;
  str->large.data = data;
  return true;
}

extern "C" void _cel_GenericString_DestabilizeAllocator(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Allocator*) alloc) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(alloc);

  if (str->large.is_large &&
      str->large.len <= _cel_GenericStringSmall_kCapacity) {
    char* data = str->large.data;
    size_t len = str->large.len;
    size_t cap = str->large.cap;
    str->small.is_large = 0;
    str->small.len = (uint8_t)len;
    memcpy(str->small.data, data, len + 1);
    _cel_GenericString_AnnotateLargeDelete(data, len, cap);
    cel_Allocator_FreeSized(alloc, data, cap + 1);
  }
}

extern "C" void _cel_GenericString_DestabilizeArena(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Arena*) arena) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(arena);

  if (str->large.is_large &&
      str->large.len <= _cel_GenericStringSmall_kCapacity) {
    char* data = str->large.data;
    size_t len = str->large.len;
    size_t cap = str->large.cap;
    str->small.is_large = 0;
    str->small.len = (uint8_t)len;
    memcpy(str->small.data, data, len + 1);
    _cel_GenericString_AnnotateLargeDelete(data, len, cap);
    cel_Arena_FreeSized(arena, data, cap + 1);
  }
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_GenericString_PrepareToAppendAllocator(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Allocator*) alloc,
    size_t size) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(alloc);

  if (str->large.is_large) {
    size_t capacity;
    if (CEL_UNLIKELY(_cel_ckd_add(&capacity, size, (size_t)str->large.len))) {
      return false;
    }
    if (CEL_LIKELY(str->large.cap >= capacity)) {
      return true;
    }
    size_t min_cap = capacity;
    size_t max_cap = _cel_GenericString_kMaxSize;
    if (min_cap > max_cap) {
      return false;
    }
    size_t new_cap = str->large.cap;
    while (new_cap < min_cap) {
      if (_cel_ckd_mul(&new_cap, new_cap, (size_t)2) || new_cap > max_cap) {
        new_cap = max_cap;
        break;
      }
    }
    size_t actual_cap;
    char* data = (char*)cel_Allocator_Malloc(alloc, new_cap + 1, &actual_cap);
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      return false;
    }
    --actual_cap;
    _cel_GenericString_AnnotateNew(data, str->large.len, actual_cap);
    memcpy(data, str->large.data, str->large.len + 1);
    _cel_GenericString_AnnotateDelete(str->large.data, str->large.len,
                                      str->large.cap);
    cel_Allocator_FreeSized(alloc, str->large.data, str->large.cap + 1);
    str->large.data = data;
    str->large.cap = actual_cap;
  } else {
    size_t capacity;
    if (CEL_UNLIKELY(_cel_ckd_add(&capacity, size, (size_t)str->small.len))) {
      return false;
    }
    if (_cel_GenericStringSmall_kCapacity >= capacity) {
      return true;
    }
    size_t min_cap = capacity;
    size_t max_cap = _cel_GenericString_kMaxSize;
    if (min_cap > max_cap) {
      return false;
    }
    size_t new_cap = _cel_GenericStringSmall_kCapacity;
    while (new_cap < min_cap) {
      if (_cel_ckd_mul(&new_cap, new_cap, (size_t)2) || new_cap > max_cap) {
        new_cap = max_cap;
        break;
      }
    }
    size_t actual_cap;
    char* data = (char*)cel_Allocator_Malloc(alloc, new_cap + 1, &actual_cap);
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      return false;
    }
    --actual_cap;
    _cel_GenericString_AnnotateNew(data, str->small.len, actual_cap);
    memcpy(data, str->small.data, str->small.len + 1);
    size_t len = str->small.len;
    str->large.is_large = 1;
    str->large.len = len;
    str->large.data = data;
    str->large.cap = actual_cap;
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_GenericString_PrepareToAppendArena(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Arena*) arena,
    size_t size) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(arena);

  if (str->large.is_large) {
    size_t capacity;
    if (CEL_UNLIKELY(_cel_ckd_add(&capacity, size, (size_t)str->large.len))) {
      return false;
    }
    if (CEL_LIKELY(str->large.cap >= capacity)) {
      return true;
    }
    size_t min_cap = capacity;
    size_t max_cap = _cel_GenericString_kMaxSize;
    if (min_cap > max_cap) {
      return false;
    }
    size_t new_cap = str->large.cap;
    while (new_cap < min_cap) {
      if (_cel_ckd_mul(&new_cap, new_cap, (size_t)2) || new_cap > max_cap) {
        new_cap = max_cap;
        break;
      }
    }
    size_t actual_cap;
    char* data = (char*)cel_Arena_Malloc(arena, new_cap + 1, &actual_cap);
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      return false;
    }
    --actual_cap;
    _cel_GenericString_AnnotateNew(data, str->large.len, actual_cap);
    memcpy(data, str->large.data, str->large.len + 1);
    _cel_GenericString_AnnotateDelete(str->large.data, str->large.len,
                                      str->large.cap);
    cel_Arena_FreeSized(arena, str->large.data, str->large.cap + 1);
    str->large.data = data;
    str->large.cap = actual_cap;
  } else {
    size_t capacity;
    if (CEL_UNLIKELY(_cel_ckd_add(&capacity, size, (size_t)str->small.len))) {
      return false;
    }
    if (_cel_GenericStringSmall_kCapacity >= capacity) {
      return true;
    }
    size_t min_cap = capacity;
    size_t max_cap = _cel_GenericString_kMaxSize;
    if (min_cap > max_cap) {
      return false;
    }
    size_t new_cap = _cel_GenericStringSmall_kCapacity;
    while (new_cap < min_cap) {
      if (_cel_ckd_mul(&new_cap, new_cap, (size_t)2) || new_cap > max_cap) {
        new_cap = max_cap;
        break;
      }
    }
    size_t actual_cap;
    char* data = (char*)cel_Arena_Malloc(arena, new_cap + 1, &actual_cap);
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      return false;
    }
    --actual_cap;
    _cel_GenericString_AnnotateNew(data, str->small.len, actual_cap);
    memcpy(data, str->small.data, str->small.len + 1);
    size_t len = str->small.len;
    str->large.is_large = 1;
    str->large.len = len;
    str->large.data = data;
    str->large.cap = actual_cap;
  }
  return true;
}

extern "C" bool _cel_GenericString_AppendAllocator(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Allocator*) alloc,
    cel_StringView val) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(alloc);

  size_t val_len = cel_StringView_Size(val);
  if (val_len > 0) {
    if (CEL_UNLIKELY(!_cel_GenericString_PrepareToAppendAllocator(str, alloc,
                                                                  val_len))) {
      return false;
    }
    if (str->large.is_large) {
      size_t new_len = str->large.len + val_len;
      _cel_GenericString_AnnotateLarge(str->large.data, str->large.len, new_len,
                                       str->large.cap);
      memcpy(str->large.data + str->large.len, cel_StringView_Data(val),
             val_len);
      str->large.data[new_len] = '\0';
      str->large.len = new_len;
    } else {
      uint8_t new_len = str->small.len + (uint8_t)val_len;
      memcpy(str->small.data + str->small.len, cel_StringView_Data(val),
             val_len);
      str->small.data[new_len] = '\0';
      str->small.len = new_len;
    }
  }
  return true;
}

extern "C" bool _cel_GenericString_AppendArena(CEL_NONNULL(_cel_GenericString*)
                                                   str,
                                               CEL_NONNULL(cel_Arena*) arena,
                                               cel_StringView val) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(arena);

  size_t val_len = cel_StringView_Size(val);
  if (val_len > 0) {
    if (CEL_UNLIKELY(
            !_cel_GenericString_PrepareToAppendArena(str, arena, val_len))) {
      return false;
    }
    if (str->large.is_large) {
      size_t new_len = str->large.len + val_len;
      _cel_GenericString_AnnotateLarge(str->large.data, str->large.len, new_len,
                                       str->large.cap);
      memcpy(str->large.data + str->large.len, cel_StringView_Data(val),
             val_len);
      str->large.data[new_len] = '\0';
      str->large.len = new_len;
    } else {
      uint8_t new_len = str->small.len + (uint8_t)val_len;
      memcpy(str->small.data + str->small.len, cel_StringView_Data(val),
             val_len);
      str->small.data[new_len] = '\0';
      str->small.len = new_len;
    }
  }
  return true;
}

extern "C" bool _cel_GenericString_PushBackAllocator(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Allocator*) alloc,
    char val) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(alloc);

  if (CEL_UNLIKELY(
          !_cel_GenericString_PrepareToAppendAllocator(str, alloc, 1))) {
    return false;
  }
  if (str->large.is_large) {
    size_t new_len = str->large.len + 1;
    _cel_GenericString_AnnotateLarge(str->large.data, str->large.len, new_len,
                                     str->large.cap);
    str->large.data[new_len - 1] = val;
    str->large.data[new_len] = '\0';
    str->large.len = new_len;
  } else {
    uint8_t new_len = str->small.len + (uint8_t)1;
    str->small.data[new_len - 1] = val;
    str->small.data[new_len] = '\0';
    str->small.len = new_len;
  }
  return true;
}

extern "C" bool _cel_GenericString_PushBackArena(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Arena*) arena,
    char val) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(arena);

  if (CEL_UNLIKELY(!_cel_GenericString_PrepareToAppendArena(str, arena, 1))) {
    return false;
  }
  if (str->large.is_large) {
    size_t new_len = str->large.len + 1;
    _cel_GenericString_AnnotateLarge(str->large.data, str->large.len, new_len,
                                     str->large.cap);
    str->large.data[new_len - 1] = val;
    str->large.data[new_len] = '\0';
    str->large.len = new_len;
  } else {
    uint8_t new_len = str->small.len + (uint8_t)1;
    str->small.data[new_len - 1] = val;
    str->small.data[new_len] = '\0';
    str->small.len = new_len;
  }
  return true;
}

extern "C" bool _cel_GenericString_ReserveAllocator(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Allocator*) alloc,
    size_t capacity) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(alloc);

  if (str->large.is_large) {
    if (str->large.cap >= capacity) {
      return true;
    }
    size_t min_cap = capacity;
    size_t max_cap = _cel_GenericString_kMaxSize;
    if (min_cap > max_cap) {
      return false;
    }
    size_t actual_cap;
    char* data = (char*)cel_Allocator_Malloc(alloc, min_cap + 1, &actual_cap);
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      return false;
    }
    --actual_cap;
    _cel_GenericString_AnnotateNew(data, str->large.len, actual_cap);
    memcpy(data, str->large.data, str->large.len + 1);
    _cel_GenericString_AnnotateDelete(str->large.data, str->large.len,
                                      str->large.cap);
    cel_Allocator_FreeSized(alloc, str->large.data, str->large.cap + 1);
    str->large.data = data;
    str->large.cap = actual_cap;
  } else {
    if (_cel_GenericStringSmall_kCapacity >= capacity) {
      return true;
    }
    size_t min_cap = capacity;
    size_t max_cap = _cel_GenericString_kMaxSize;
    if (min_cap > max_cap) {
      return false;
    }
    size_t actual_cap;
    char* data = (char*)cel_Allocator_Malloc(alloc, min_cap + 1, &actual_cap);
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      return false;
    }
    --actual_cap;
    _cel_GenericString_AnnotateNew(data, str->small.len, actual_cap);
    memcpy(data, str->small.data, str->small.len + 1);
    size_t len = str->small.len;
    str->large.is_large = 1;
    str->large.len = len;
    str->large.data = data;
    str->large.cap = actual_cap;
  }
  return true;
}

extern "C" bool _cel_GenericString_ReserveArena(CEL_NONNULL(_cel_GenericString*)
                                                    str,
                                                CEL_NONNULL(cel_Arena*) arena,
                                                size_t capacity) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_NOT_NULL(arena);

  if (str->large.is_large) {
    if (str->large.cap >= capacity) {
      return true;
    }
    size_t min_cap = capacity;
    size_t max_cap = _cel_GenericString_kMaxSize;
    if (min_cap > max_cap) {
      return false;
    }
    size_t actual_cap;
    char* data = (char*)cel_Arena_Malloc(arena, min_cap + 1, &actual_cap);
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      return false;
    }
    --actual_cap;
    _cel_GenericString_AnnotateNew(data, str->large.len, actual_cap);
    memcpy(data, str->large.data, str->large.len + 1);
    _cel_GenericString_AnnotateDelete(str->large.data, str->large.len,
                                      str->large.cap);
    cel_Arena_FreeSized(arena, str->large.data, str->large.cap + 1);
    str->large.data = data;
    str->large.cap = actual_cap;
  } else {
    if (_cel_GenericStringSmall_kCapacity >= capacity) {
      return true;
    }
    size_t min_cap = capacity;
    size_t max_cap = _cel_GenericString_kMaxSize;
    if (min_cap > max_cap) {
      return false;
    }
    size_t actual_cap;
    char* data = (char*)cel_Arena_Malloc(arena, min_cap + 1, &actual_cap);
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      return false;
    }
    --actual_cap;
    _cel_GenericString_AnnotateNew(data, str->small.len, actual_cap);
    memcpy(data, str->small.data, str->small.len + 1);
    size_t len = str->small.len;
    str->large.is_large = 1;
    str->large.len = len;
    str->large.data = data;
    str->large.cap = actual_cap;
  }
  return true;
}
