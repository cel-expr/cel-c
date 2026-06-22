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

// Internal header providing miscellaneous memory functions.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_MEMORY_H_
#define THIRD_PARTY_CEL_C_INTERNAL_MEMORY_H_

#include <stdbool.h>
#include <stddef.h>

#include "cel-c/internal/config.h"

CEL_BEGIN_DECLS

// _cel_Memory_Equals
//
// Tests two memory blocks for equality. Returns `true` if they are equal,
// `false` otherwise.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_Memory_Equals(CEL_NULLABLE(const void*) lhs_data, size_t lhs_size,
                        CEL_NULLABLE(const void*) rhs_data, size_t rhs_size);

// _cel_Memory_Compare
//
// Compares two potentially different length memory blocks.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
int _cel_Memory_Compare(CEL_NULLABLE(const void*) lhs_data, size_t lhs_size,
                        CEL_NULLABLE(const void*) rhs_data, size_t rhs_size);

// _cel_Memory_StartsWith
//
// Tests whether the memory block at `hay_data` with a byte length of `hay_size`
// starts with the memory block at `ndl_data` with a byte length of `ndl_size`.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_Memory_StartsWith(CEL_NULLABLE(const void*) hay_data, size_t hay_size,
                            CEL_NULLABLE(const void*) ndl_data,
                            size_t ndl_size);

// _cel_Memory_EndsWith
//
// Tests whether the memory block at `hay_data` with a byte length of `hay_size`
// ends with the memory block at `ndl_data` with a byte length of `ndl_size`.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_Memory_EndsWith(CEL_NULLABLE(const void*) hay_data, size_t hay_size,
                          CEL_NULLABLE(const void*) ndl_data, size_t ndl_size);

// _cel_Memory_FindFirst
//
// Finds the first occurrence of the the memory block at `ndl_data` with a byte
// length of `ndl_size` in the memory block at `hay_data` with a byte length of
// `hay_size` and returns a pointer to it. Returns a null pointer if the memory
// block does not occur.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_NULLABLE(void*)
_cel_Memory_FindFirst(CEL_NULLABLE(const void*) hay_data, size_t hay_size,
                      CEL_NULLABLE(const void*) ndl_data, size_t ndl_size);

// _cel_Memory_FindLast
//
// Finds the last occurrence of the the memory block at `ndl_data` with a byte
// length of `ndl_size` in the memory block at `hay_data` with a byte length of
// `hay_size` and returns a pointer to it. Returns a null pointer if the memory
// block does not occur.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_NULLABLE(void*)
_cel_Memory_FindLast(CEL_NULLABLE(const void*) hay_data, size_t hay_size,
                     CEL_NULLABLE(const void*) ndl_data, size_t ndl_size);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_MEMORY_H_
