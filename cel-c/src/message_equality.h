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

#ifndef THIRD_PARTY_CEL_C_SRC_MESSAGE_EQUALITY_H_
#define THIRD_PARTY_CEL_C_SRC_MESSAGE_EQUALITY_H_

#include <stdbool.h>

#include "cel-c/alloc.h"
#include "cel-c/config.h"
#include "cel-c/well_known_types.h"
#include "upb/message/array.h"
#include "upb/message/message.h"
#include "upb/reflection/def.h"

CEL_BEGIN_DECLS

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM {
  _cel_MessageEquality_kEqual = 1,
  _cel_MessageEquality_kNotEqual,
  _cel_MessageEquality_kOutOfMemory,
  _cel_MessageEquality_kMaxDepthExceeded,
} _cel_MessageEquality;

// _cel_Message_Equals
//
// Tests message equality according to CEL semantics.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
_cel_MessageEquality _cel_Message_Equals(
    const upb_Message* cel_nonnull lhs_val,
    const upb_Message* cel_nonnull rhs_val,
    const upb_MessageDef* cel_nonnull val_def,
    const upb_DefPool* cel_nonnull def_pool,
    const cel_WellKnownTypes* cel_nonnull wkts,
    cel_Allocator* cel_nonnull alloc);

// _cel_MessageField_Equals
//
// Tests message field equality according to CEL semantics. This is done as if
// the following was evaluated directly in CEL: `lhs_val.lhs_def ==
// rhs_val.rhs_def`.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
_cel_MessageEquality _cel_MessageField_Equals(
    upb_MessageValue lhs_val, const upb_FieldDef* cel_nonnull lhs_def,
    upb_MessageValue rhs_val, const upb_FieldDef* cel_nonnull rhs_def,
    const upb_DefPool* cel_nonnull def_pool,
    const cel_WellKnownTypes* cel_nonnull wkts,
    cel_Allocator* cel_nonnull alloc);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_SRC_MESSAGE_EQUALITY_H_
