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

#include "cel-c/error_status.h"

#include <stdbool.h>
#include <stddef.h>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/error.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"

extern "C" bool cel_Error_FromStatus(cel_Error* cel_nonnull out,
                          const cel_Status* cel_nonnull in,
                          cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(out);
  CEL_ASSERT_NOT_NULL(in);
  CEL_ASSERT_NOT(cel_Status_Ok(in));
  CEL_ASSERT_NOT_NULL(arena);

  if (CEL_UNLIKELY(cel_Status_Ok(in))) {
    cel_Error_Clear(out);
    return true;
  }

  cel_Error_SetCode(out, cel_Status_Space(in), cel_Status_Code(in));

  cel_StringView in_message = cel_Status_Message(in);
  cel_StringView out_message;
  if (cel_StringView_Empty(in_message)) {
    out_message = cel_StringView_FromString("");
  } else {
    if (!cel_Arena_StrDup(arena, &out_message, in_message)) {
      return false;
    }
  }
  cel_Error_SetMessage(out, out_message);

  cel_StringView in_type_url;
  cel_StringView in_value;
  cel_StatusPayloadIterator in_iter = cel_Status_BeginPayloads(in);
  while (cel_Status_NextPayload(in, &in_type_url, &in_value, &in_iter)) {
    cel_StringView out_type_url;
    cel_StringView out_value;
    if (!cel_Arena_StrDup(arena, &out_type_url, in_type_url)) {
      return false;
    }
    if (!cel_Arena_StrDup(arena, &out_value, in_value)) {
      return false;
    }
    if (!cel_Error_SetPayload(out, out_type_url, out_value, arena)) {
      return false;
    }
  }
  return true;
}
