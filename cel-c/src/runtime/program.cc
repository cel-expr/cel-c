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

#include "cel-c/src/runtime/program.h"

#include <stdint.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/src/arc.h"
#include "cel-c/src/array.h"
#include "cel-c/src/runtime/activation.h"
#include "cel-c/src/runtime/runtime.h"
#include "cel-c/src/string.h"

extern "C" cel_ProgramOptions* cel_nonnull
cel_ProgramOptions_Default(cel_ProgramOptions* cel_nonnull opts) {
  CEL_ASSERT_NOT_NULL(opts);

  memset(opts, 0, sizeof(*opts));
  return opts;
}

extern "C" void cel_Program_Delete(cel_Program* cel_nullable prog) {
  CEL_ASSERT(prog == cel_nullptr || !prog->deleted);

  if (prog != cel_nullptr) {
    prog->deleted = true;
    _cel_Program_Unref(prog);
  }
}

extern "C" cel_Activation* cel_nullable
cel_Program_Activate(const cel_Program* cel_nonnull program,
                     const cel_VariableResolver* cel_nonnull var_resolver,
                     const cel_ActivationOptions* cel_nullable opts) {
  CEL_ASSERT_NOT_NULL(program);
  CEL_ASSERT_NOT_NULL(var_resolver);
  CEL_USED(opts);

  return _cel_Activation_New(program, var_resolver);
}

extern "C" cel_Program* cel_nullable
_cel_Program_New(const cel_Runtime* cel_nonnull rt) {
  CEL_ASSERT_NOT_NULL(rt);

  cel_Program* prog = (cel_Program*)cel_Allocator_Malloc(
      rt->alloc, sizeof(cel_Program), cel_nullptr);
  if (CEL_UNLIKELY(prog == cel_nullptr)) {
    return cel_nullptr;
  }
  memset(prog, 0, sizeof(*prog));
  prog->rt = _cel_Runtime_ConstRef(rt);
  _cel_AtomicRefCount_Initialize(&prog->rc);
  _cel_Array_Construct(&prog->instrs);
  _cel_Array_Construct(&prog->strings_table);
  _cel_Array_Construct(&prog->candidate_names_table);
  return prog;
}

extern "C" void _cel_Program_Delete(cel_Program* cel_nonnull prog) {
  CEL_ASSERT_NOT_NULL(prog);
  CEL_ASSERT(_cel_AtomicRefCount_Expired(&prog->rc));
  CEL_ASSERT(prog->deleted);

  const cel_Runtime* rt = prog->rt;
  cel_Allocator* alloc = rt->alloc;
  _cel_CandidateNames* candidate_names_ptr =
      _cel_Array_MutableData(&prog->candidate_names_table);
  size_t candidate_names_len = _cel_Array_Size(&prog->candidate_names_table);
  for (; candidate_names_len > 0;
       ++candidate_names_ptr, --candidate_names_len) {
    _cel_CandidateNames_Destruct(candidate_names_ptr, alloc);
  }
  _cel_Array_Destruct(&prog->candidate_names_table, alloc);
  _cel_String* string_ptr = _cel_Array_MutableData(&prog->strings_table);
  size_t string_len = _cel_Array_Size(&prog->strings_table);
  for (; string_len > 0; ++string_ptr, --string_len) {
    _cel_String_Destruct(string_ptr, alloc);
  }
  _cel_Array_Destruct(&prog->strings_table, alloc);
  _cel_Array_Destruct(&prog->instrs, alloc);
  cel_Allocator_FreeSized(alloc, prog, sizeof(*prog));
  _cel_Runtime_Unref(rt);
}
