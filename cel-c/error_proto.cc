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

#include "cel-c/error_proto.h"

#include <stdbool.h>
#include <string.h>

#include "google/protobuf/any.upb.h"
#include "google/rpc/status.upb.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/error.h"
#include "cel-c/error_code_proto.h"
#include "cel-c/string_view.h"
#include "upb/base/string_view.h"

static bool _cel_Error_StrDup(cel_StringView in, cel_Arena* cel_nonnull arena,
                              cel_StringView* cel_nonnull out) {
  if (cel_StringView_Empty(in)) {
    *out = cel_StringView_FromString("");
    return true;
  }
  char* copied =
      (char*)cel_Arena_Malloc(arena, cel_StringView_Size(in), cel_nullptr);
  if (copied == cel_nullptr) {
    return false;
  }
  memcpy(copied, cel_StringView_Data(in), cel_StringView_Size(in));
  *out = cel_StringView_FromArray(copied, cel_StringView_Size(in));
  return true;
}

extern "C" bool cel_Error_ToProto(const cel_Error* cel_nonnull in,
                                  cel_Arena* cel_nonnull arena,
                                  google_rpc_Status* cel_nonnull out) {
  CEL_ASSERT_NOT_NULL(in);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_NOT_NULL(out);

  google_rpc_Status_set_code(
      out, cel_ErrorCode_ToProto(cel_Error_CanonicalCode(in)));
  cel_StringView in_message = cel_Error_Message(in);
  upb_StringView out_message;
  if (!_cel_Error_StrDup(in_message, arena, &out_message)) {
    return false;
  }
  google_rpc_Status_set_message(out, out_message);

  size_t in_payloads_len = cel_Error_Payloads(in);
  if (in_payloads_len == 0) {
    google_rpc_Status_clear_details(out);
  } else {
    google_protobuf_Any** out_payloads =
        google_rpc_Status_resize_details(out, in_payloads_len, arena);
    if (out_payloads == cel_nullptr) {
      return false;
    }
    size_t i = 0;
    cel_StringView in_type_url;
    cel_StringView in_value;
    cel_ErrorPayloadIterator iter = cel_Error_BeginPayloads(in);
    bool oom = false;
    while (cel_Error_NextPayload(in, &in_type_url, &in_value, &iter)) {
      google_protobuf_Any* out_payload = google_protobuf_Any_new(arena);
      if (out_payload == cel_nullptr) {
        oom = true;
        break;
      }
      out_payloads[i++] = out_payload;
      upb_StringView out_type_url;
      if (!_cel_Error_StrDup(in_type_url, arena, &out_type_url)) {
        oom = true;
        break;
      }
      google_protobuf_Any_set_type_url(out_payload, out_type_url);
      upb_StringView out_value;
      if (!_cel_Error_StrDup(in_value, arena, &out_value)) {
        oom = true;
        break;
      }
      google_protobuf_Any_set_value(out_payload, out_value);
    }
    if (i < in_payloads_len) {
      google_rpc_Status_resize_details(out, i, arena);
    }
    if (oom) {
      return false;
    }
  }

  return true;
}
