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

// Internal header exposing an interface to malloc.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_MALLOC_H_
#define THIRD_PARTY_CEL_C_INTERNAL_MALLOC_H_

#include <stddef.h>

#include "cel-c/internal/config.h"

CEL_BEGIN_DECLS

// _cel_Malloc
//
// Allocates `size` bytes of memory from the system and returns a pointer to the
// allocated memory. If `size` is `0` or the system is out of memory, a null
// pointer is returned. If `actual_size` is not null, the actual usable size in
// bytes of the underlying allocation is stored in `actual_size` which is
// guaranteed to be greater than or equal to `size` when the allocation is
// successful.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_MALLOC
CEL_NULLABLE(void*) _cel_Malloc(size_t size, CEL_NULLABLE(size_t*) actual_size);

// _cel_Calloc
//
// Allocates `num` elements which are each `size` bytes from the system and
// returns a pointer to the allocated memory, the memory is initialized to zero.
// If `num` is `0`, `size` is `0`, or the system is out of memory, a null
// pointer is returned. If `actual_num` is not null, the actual usable size in
// elements of the underlying allocation is stored in `actual_num` which is
// guaranteed to be greater than or equal to `num` when the allocation is
// successful.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_MALLOC
CEL_NULLABLE(void*)
_cel_Calloc(size_t num, size_t size, CEL_NULLABLE(size_t*) actual_num);

// _cel_Realloc
//
// Reallocates a previously allocated block of memory. If `old_size` is `0`,
// this is equivalent to calling `_cel_Malloc`. If `new_size` is `0`, this is
// equivalent to call `_cel_FreeSized`. Otherwise a new block of memory is
// allocated, the lesser of `old_size` and `new_size` bytes are copied from the
// old memory block to the new memory block, and the old memory block is freed.
// If the system is out of memory, a null pointer is returned and no memory is
// allocated or freed.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_NULLABLE(void*)
_cel_Realloc(CEL_NULLABLE(void*) addr, size_t old_size, size_t new_size,
             CEL_NULLABLE(size_t*) actual_size);

// _cel_Free
//
// Deallocates a previously allocated block of memory.
CEL_ATTRIBUTE_NOTHROW
void _cel_Free(CEL_NULLABLE(void*) addr);

// _cel_FreeSized
//
// Deallocates a previously allocated block of memory with a known size.
CEL_ATTRIBUTE_NOTHROW
void _cel_FreeSized(CEL_NULLABLE(void*) addr, size_t size);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_MALLOC_H_
