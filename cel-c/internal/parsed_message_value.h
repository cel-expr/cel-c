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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_PARSED_MESSAGE_VALUE_H_
#define THIRD_PARTY_CEL_C_INTERNAL_PARSED_MESSAGE_VALUE_H_

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/type.h"
#include "cel-c/value.h"
#include "upb/message/message.h"
#include "upb/reflection/def.h"

CEL_BEGIN_DECLS

extern const cel_StructValueVTable _cel_ParsedMessageValueVTable;

static CEL_INLINE void _cel_ParsedMessageValue_Set(
    cel_StructValue* cel_nonnull struct_value,
    const upb_Message* cel_nonnull message,
    const upb_MessageDef* cel_nonnull message_def) {
  CEL_ASSERT_NOT_NULL(struct_value);
  CEL_ASSERT_NOT_NULL(message);
  CEL_ASSERT_NOT_NULL(message_def);
  CEL_ASSERT_NOT(cel_IsWellKnownMessageType(message_def));

  struct_value->vtable = &_cel_ParsedMessageValueVTable;
  struct_value->content.ptr[0] = message;
  struct_value->content.ptr[1] = message_def;
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_PARSED_MESSAGE_VALUE_H_
