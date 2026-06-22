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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_RUNTIME_RUNTIME_H_
#define THIRD_PARTY_CEL_C_INTERNAL_RUNTIME_RUNTIME_H_

#include "cel-c/alloc.h"
#include "cel-c/config.h"
#include "cel-c/internal/arc.h"
#include "cel-c/internal/container.h"
#include "cel-c/runtime.h"  // IWYU pragma: export
#include "cel-c/well_known_types.h"
#include "upb/reflection/def.h"

CEL_BEGIN_DECLS

struct cel_Runtime {
  cel_RuntimeOptions opts;
  _cel_AtomicRefCount rc;
  cel_Allocator* cel_nonnull alloc;
  const upb_DefPool* cel_nonnull def_pool;
  cel_WellKnownTypes wkts;
  _cel_Container container;
  bool deleted;
};

CEL_ATTRIBUTE_NOTHROW
void _cel_Runtime_Delete(cel_Runtime* cel_nonnull rt);

static CEL_INLINE const cel_Runtime* cel_nullability_unknown
_cel_Runtime_ConstRef(const cel_Runtime* cel_nullability_unknown rt) {
  if (rt != cel_nullptr) {
    _cel_AtomicRefCount_Increment((_cel_AtomicRefCount*)&rt->rc);
  }
  return rt;
}

static CEL_INLINE cel_Runtime* cel_nullability_unknown
_cel_Runtime_MutableRef(cel_Runtime* cel_nullability_unknown rt) {
  return (cel_Runtime*)_cel_Runtime_ConstRef(rt);
}

static CEL_INLINE void _cel_Runtime_Unref(const cel_Runtime* cel_nullable rt) {
  if (rt != cel_nullptr &&
      _cel_AtomicRefCount_Decrement((_cel_AtomicRefCount*)&rt->rc)) {
    _cel_Runtime_Delete((cel_Runtime*)rt);
  }
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_RUNTIME_RUNTIME_H_
