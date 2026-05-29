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

#ifndef THIRD_PARTY_CEL_C_ARENA_H_
#define THIRD_PARTY_CEL_C_ARENA_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "upb/base/string_view.h"
#include "upb/mem/arena.h"

CEL_BEGIN_DECLS

#define cel_Arena_kMaxAlign cel_kMaxAlign

typedef upb_Arena cel_Arena;

// cel_Arena_New
//
// Creates a new arena which uses `alloc` for memory allocation.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(cel_Arena*)
    cel_Arena_New(CEL_NONNULL(cel_Allocator*) alloc) {
  CEL_ASSERT_NOT_NULL(alloc);

  return upb_Arena_Init(cel_nullptr, 0, alloc);
}

// cel_Arena_Delete
//
// Destroys the arena, releasing its resources.
static CEL_INLINE void cel_Arena_Delete(CEL_NULLABLE(cel_Arena*) arena) {
  if (arena != cel_nullptr) {
    upb_Arena_Free(arena);
  }
}

// cel_Arena_Allocator
//
// Returns the allocator.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Allocator* cel_nonnull
cel_Arena_Allocator(const cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(arena);

  return upb_Arena_GetUpbAlloc((upb_Arena*)arena);
}

// cel_Arena_Malloc
//
// Allocates `size` bytes of memory from the arena and returns a pointer to the
// allocated memory. If `size` is `0` or the arena is out of memory, a null
// pointer is returned.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NULLABLE(void*)
    cel_Arena_Malloc(CEL_NONNULL(cel_Arena*) arena, size_t size,
                     CEL_NULLABLE(size_t*) actual_size) {
  CEL_ASSERT_NOT_NULL(arena);

  void* addr;
  if (size > 0) {
    addr = upb_Arena_Malloc(arena, size);
  } else {
    addr = cel_nullptr;
  }
  if (actual_size != cel_nullptr) {
    *actual_size = addr != cel_nullptr ? size : 0;
  }
  return addr;
}

// cel_Arena_Calloc
//
// Allocates `num` elements which are each `size` bytes from the system and
// returns a pointer to the allocated memory, the memory is initialized to zero.
// If `num` is `0`, `size` is `0`, or the system is out of memory, a null
// pointer is returned.
CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
CEL_EXTERN CEL_NULLABLE(void*)
    cel_Arena_Calloc(CEL_NONNULL(cel_Arena*) arena, size_t num, size_t size,
                     CEL_NULLABLE(size_t*) actual_num);

// cel_Arena_Realloc
//
// Reallocates a previously allocated block of memory. If `old_size` is `0`,
// this is equivalent to calling `cel_Arena_Malloc`. If `new_size` is `0`,
// this is equivalent to call `cel_Arena_FreeSized`. Otherwise a new block
// of memory is allocated, the lesser of `old_size` and `new_size` bytes are
// copied from the old memory block to the new memory block, and the old memory
// block is freed. If the arena is out of memory, a null pointer is returned
// and no memory is allocated or freed.
CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_NODISCARD
CEL_EXTERN CEL_NULLABLE(void*)
    cel_Arena_Realloc(CEL_NONNULL(cel_Arena*) arena, CEL_NULLABLE(void*) addr,
                      size_t old_size, size_t new_size,
                      CEL_NULLABLE(size_t*) actual_size);

// cel_Arena_FreeSized
//
// Attempts to return a block of memory to `arena`.
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Arena_FreeSized(CEL_NONNULL(cel_Arena*) arena,
                                    CEL_NULLABLE(void*) addr, size_t size);

// cel_Arena_StrDup
//
// Convenience function which duplicates the given null-terminated string. This
// function is similar to `strdup`. If the allocation fails, `false` is
// returned.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Arena_StrDup(CEL_NONNULL(cel_Arena*) arena,
                                        CEL_NONNULL(upb_StringView*) out,
                                        upb_StringView in) {
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_NOT_NULL(out);

  char* data;
  if (in.size == 0) {
    data = (char*)"";
  } else {
    data = (char*)upb_Arena_Malloc(arena, in.size);
    if (CEL_UNLIKELY(data == cel_nullptr)) {
      return false;
    }
    memcpy(data, in.data, in.size);
  }
  *out = upb_StringView_FromDataAndSize(data, in.size);
  return true;
}

// cel_Arena_VPrintF
//
// See `cel_Arena_PrintF`.
CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_VFORMAT(3)
CEL_ATTRIBUTE_NODISCARD
CEL_EXTERN bool cel_Arena_VPrintF(CEL_NONNULL(cel_Arena*) arena,
                                  CEL_NONNULL(upb_StringView*) out,
                                  CEL_NONNULL(const char*) fmt, va_list args);

// cel_Arena_PrintF
//
// Convenience function which prints a formatted string into memory allocated
// using `arena`. If the allocation fails or the format string is invalid,
// `false` is returned. This function is similar to `asprintf`.
CEL_ATTRIBUTE_FORMAT(3, 4)
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool cel_Arena_PrintF(CEL_NONNULL(cel_Arena*) arena,
                                 CEL_NONNULL(upb_StringView*) out,
                                 CEL_NONNULL(const char*) fmt, ...) {
  va_list args;
  va_start(args, fmt);
  const bool ok = cel_Arena_VPrintF(arena, out, fmt, args);
  va_end(args);
  return ok;
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_ARENA_H_
