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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_TESTING_DEF_POOL_H_
#define THIRD_PARTY_CEL_C_INTERNAL_TESTING_DEF_POOL_H_

#include "cel-c/config.h"
#include "upb/reflection/def.h"

CEL_BEGIN_DECLS

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_PURE
const upb_DefPool* cel_nonnull _cel_TestingDefPool();

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
void _cel_TestingDefs(upb_DefPool* cel_nonnull def_pool);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_TESTING_DEF_POOL_H_
