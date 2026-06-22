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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_ANY_H_
#define THIRD_PARTY_CEL_C_INTERNAL_ANY_H_

#include <stdbool.h>

#include "cel-c/internal/config.h"
#include "cel-c/status.h"
#include "cel-c/status_code.h"
#include "cel-c/well_known_types.h"
#include "upb/mem/arena.h"
#include "upb/message/message.h"
#include "upb/reflection/def.h"

CEL_BEGIN_DECLS

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM {
  _cel_AnyUnpackResult_kOk = 1,
  _cel_AnyUnpackResult_kOutOfMemory,
  // Type URL does not have a known prefix.
  _cel_AnyUnpackResult_kBadTypeUrl,
  // Type name not found in upb_DefPool.
  _cel_AnyUnpackResult_kDefNotFound,
  // See kUpb_DecodeStatus_Malformed.
  _cel_AnyUnpackResult_kMalformed,
  // See kUpb_DecodeStatus_BadUtf8.
  _cel_AnyUnpackResult_kBadUtf8,
  // See kUpb_DecodeStatus_MaxDepthExceeded.
  _cel_AnyUnpackResult_kMaxDepthExceeded,
  // Unknown error.
  _cel_AnyUnpackResult_kUnknown,
} _cel_AnyUnpackResult;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
_cel_AnyUnpackResult _cel_AnyUnpack(
    const upb_Message* cel_nonnull in_message,
    const upb_DefPool* cel_nonnull def_pool, const cel_AnyWellKnownType* wkt,
    upb_Arena* cel_nonnull arena,
    upb_Message* cel_nullable* cel_nonnull out_message,
    const upb_MessageDef* cel_nullable* cel_nonnull out_message_def);

CEL_ATTRIBUTE_NOTHROW
void _cel_AnyUnpackResult_ToStatus(_cel_AnyUnpackResult result,
                                   cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
cel_StatusCode _cel_AnyUnpackResult_ToStatusCode(_cel_AnyUnpackResult result);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
const char* cel_nonnull
_cel_AnyUnpackResult_ToMessage(_cel_AnyUnpackResult result);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_ANY_H_
