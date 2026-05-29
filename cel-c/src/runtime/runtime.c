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

#include "cel-c/src/runtime/runtime.h"

#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/assert.h"
#include "cel-c/ast.h"
#include "cel-c/config.h"
#include "cel-c/program.h"
#include "cel-c/status.h"
#include "cel-c/well_known_types.h"
#include "cel-c/src/arc.h"
#include "cel-c/src/container.h"
#include "cel-c/src/runtime/interpreter.h"
#include "cel-c/src/string.h"
#include "upb/reflection/def.h"

cel_RuntimeOptions* cel_nonnull
cel_RuntimeOptions_Default(cel_RuntimeOptions* cel_nonnull opts) {
  CEL_ASSERT_NOT_NULL(opts);

  memset(opts, 0, sizeof(*opts));
  return opts;
}

cel_Runtime* cel_nullable cel_Runtime_New(
    cel_Allocator* cel_nonnull alloc, const upb_DefPool* cel_nonnull def_pool,
    const cel_RuntimeOptions* cel_nullable opts,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_NOT_NULL(def_pool);
  CEL_ASSERT_NOT_NULL(status);
  CEL_ASSERT(cel_Status_Ok(status));

  cel_Runtime* rt = (cel_Runtime*)cel_Allocator_Malloc(
      alloc, sizeof(cel_Runtime), cel_nullptr);
  if (CEL_UNLIKELY(rt == cel_nullptr)) {
    cel_OutOfMemoryStatus(status);
    return cel_nullptr;
  }
  memset(rt, 0, sizeof(*rt));
  if (opts != cel_nullptr) {
    rt->opts = *opts;
  } else {
    opts = cel_RuntimeOptions_Default(&rt->opts);
  }
  rt->alloc = alloc;
  rt->def_pool = def_pool;
  if (!cel_WellKnownTypes_Initialize(&rt->wkts, def_pool, status)) {
    cel_Allocator_FreeSized(alloc, rt, sizeof(*rt));
    return cel_nullptr;
  }
  _cel_Container_Construct(&rt->container);
  if (!_cel_Container_Update(&rt->container, rt->opts.container, rt->alloc)) {
    _cel_Container_Destruct(&rt->container, rt->alloc);
    cel_Allocator_FreeSized(alloc, rt, sizeof(*rt));
    return cel_nullptr;
  }
  rt->opts.container = _cel_String_ToStringView(&rt->container.name);
  _cel_AtomicRefCount_Initialize(&rt->rc);
  return rt;
}

void cel_Runtime_Delete(cel_Runtime* cel_nullable rt) {
  CEL_ASSERT(rt == cel_nullptr || !rt->deleted);

  if (rt != cel_nullptr) {
    rt->deleted = true;
    _cel_Runtime_Unref(rt);
  }
}

cel_Allocator* cel_nonnull
cel_Runtime_Allocator(const cel_Runtime* cel_nonnull rt) {
  CEL_ASSERT_NOT_NULL(rt);

  return rt->alloc;
}

const cel_RuntimeOptions* cel_nonnull
cel_Runtime_Options(const cel_Runtime* cel_nonnull rt) {
  CEL_ASSERT_NOT_NULL(rt);

  return &rt->opts;
}

const upb_DefPool* cel_nonnull
cel_Runtime_DefPool(const cel_Runtime* cel_nonnull rt) {
  CEL_ASSERT_NOT_NULL(rt);

  return rt->def_pool;
}

const cel_WellKnownTypes* cel_nonnull
cel_Runtime_WellKnownTypes(const cel_Runtime* cel_nonnull rt) {
  CEL_ASSERT_NOT_NULL(rt);

  return &rt->wkts;
}

cel_Program* cel_nullable cel_Runtime_Compile(
    const cel_Runtime* cel_nonnull runtime, const cel_Ast* cel_nonnull ast,
    const cel_ProgramOptions* cel_nullable opts,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(runtime);
  CEL_ASSERT_NOT_NULL(ast);
  CEL_ASSERT(cel_Status_Ok(status));

  return _cel_Interpreter_Compile(runtime, ast, status);
}

void _cel_Runtime_Delete(cel_Runtime* cel_nonnull rt) {
  CEL_ASSERT_NOT_NULL(rt);
  CEL_ASSERT(_cel_AtomicRefCount_Expired(&rt->rc));
  CEL_ASSERT(rt->deleted);

  _cel_Container_Destruct(&rt->container, rt->alloc);
  cel_Allocator_FreeSized(rt->alloc, rt, sizeof(*rt));
}
