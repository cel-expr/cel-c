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

#ifndef THIRD_PARTY_CEL_C_SRC_RUNTIME_PROGRAM_H_
#define THIRD_PARTY_CEL_C_SRC_RUNTIME_PROGRAM_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/config.h"
#include "cel-c/program.h"  // IWYU pragma: export
#include "cel-c/src/arc.h"
#include "cel-c/src/array.h"
#include "cel-c/src/runtime/instr.h"
#include "cel-c/src/string.h"

CEL_BEGIN_DECLS

struct cel_Runtime;

typedef struct {
  uint32_t* cel_nullability_unknown data;
  size_t size;
} _cel_CandidateNames;

static CEL_INLINE void _cel_CandidateNames_Construct(
    _cel_CandidateNames* cel_nonnull candidate_names) {
  memset(candidate_names, 0, sizeof(*candidate_names));
}

static CEL_INLINE void _cel_CandidateNames_Destruct(
    _cel_CandidateNames* cel_nonnull candidate_names,
    cel_Allocator* cel_nonnull alloc) {
  cel_Allocator_FreeSized(
      alloc, candidate_names->data,
      candidate_names->size * sizeof(*candidate_names->data));
}

struct cel_Program {
  // The runtime that created this program.
  const struct cel_Runtime* cel_nonnull rt;
  _cel_AtomicRefCount rc;

  // Array of instructions. The entry point of the program is at index 0 and the
  // exit point of the program is at the last index.
  _cel_Array(_cel_Instr) instrs;

  // Array of strings, bytes, and idents.
  _cel_Array(_cel_String) strings_table;

  // Used if the runtime has a container configured.
  _cel_Array(_cel_CandidateNames) candidate_names_table;

  uint32_t max_stack_size;

  uint32_t max_slot_size;

  bool deleted;
};

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
cel_Program* cel_nullable
_cel_Program_New(const struct cel_Runtime* cel_nonnull rt);

CEL_ATTRIBUTE_NOTHROW
void _cel_Program_Delete(cel_Program* cel_nonnull prog);

static CEL_INLINE const cel_Program* cel_nullability_unknown
_cel_Program_ConstRef(const cel_Program* cel_nullability_unknown prog) {
  if (prog != cel_nullptr) {
    _cel_AtomicRefCount_Increment((_cel_AtomicRefCount*)&prog->rc);
  }
  return prog;
}

static CEL_INLINE cel_Program* cel_nullability_unknown
_cel_Program_MutableRef(cel_Program* cel_nullability_unknown prog) {
  return (cel_Program*)_cel_Program_ConstRef(prog);
}

static CEL_INLINE void _cel_Program_Unref(
    const cel_Program* cel_nullable prog) {
  if (prog != cel_nullptr &&
      _cel_AtomicRefCount_Decrement((_cel_AtomicRefCount*)&prog->rc)) {
    _cel_Program_Delete((cel_Program*)prog);
  }
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_SRC_RUNTIME_PROGRAM_H_
