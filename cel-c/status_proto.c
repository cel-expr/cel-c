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

#include "cel-c/status_proto.h"

#include <stdbool.h>
#include <string.h>

#include "google/protobuf/any.upb.h"
#include "google/rpc/code.upb.h"
#include "google/rpc/status.upb.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/status.h"
#include "cel-c/status_code_proto.h"
#include "cel-c/string_view.h"
#include "upb/base/string_view.h"
#include "upb/mem/arena.h"

static bool _cel_Status_StrDup(cel_StringView in, CEL_NONNULL(upb_Arena*) arena,
                               CEL_NONNULL(upb_StringView*) out) {
  if (cel_StringView_Empty(in)) {
    *out = upb_StringView_FromString("");
    return true;
  }
  char* copied = upb_Arena_Malloc(arena, cel_StringView_Size(in));
  if (copied == cel_nullptr) {
    return false;
  }
  memcpy(copied, cel_StringView_Data(in), cel_StringView_Size(in));
  *out = upb_StringView_FromDataAndSize(copied, cel_StringView_Size(in));
  return true;
}

bool cel_Status_ToProto(CEL_NONNULL(const cel_Status*) in,
                        CEL_NONNULL(upb_Arena*) arena,
                        CEL_NONNULL(google_rpc_Status*) out) {
  CEL_ASSERT_NOT_NULL(in);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_NOT_NULL(out);

  if (cel_Status_Ok(in)) {
    google_rpc_Status_clear_code(out);
    google_rpc_Status_clear_message(out);
    google_rpc_Status_clear_details(out);
    return true;
  }

  google_rpc_Status_set_code(out, cel_StatusCode_ToProto(cel_Status_CanonicalCode(in)));
  cel_StringView in_message = cel_Status_Message(in);
  upb_StringView out_message;
  if (!_cel_Status_StrDup(in_message, arena, &out_message)) {
    return false;
  }
  google_rpc_Status_set_message(out, out_message);

  size_t in_payloads_len = cel_Status_Payloads(in);
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
    cel_StatusPayloadIterator iter = cel_Status_BeginPayloads(in);
    bool oom = false;
    while (cel_Status_NextPayload(in, &in_type_url, &in_value, &iter)) {
      google_protobuf_Any* out_payload = google_protobuf_Any_new(arena);
      if (out_payload == cel_nullptr) {
        oom = true;
        break;
      }
      out_payloads[i++] = out_payload;
      upb_StringView out_type_url;
      if (!_cel_Status_StrDup(in_type_url, arena, &out_type_url)) {
        oom = true;
        break;
      }
      google_protobuf_Any_set_type_url(out_payload, out_type_url);
      upb_StringView out_value;
      if (!_cel_Status_StrDup(in_value, arena, &out_value)) {
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

bool cel_Status_FromProto(CEL_NONNULL(cel_Status*) out,
                          CEL_NONNULL(const google_rpc_Status*) in) {
  CEL_ASSERT_NOT_NULL(in);
  CEL_ASSERT_NOT_NULL(out);

  google_rpc_Code in_code = google_rpc_Status_code(in);
  if (in_code == google_rpc_OK) {
    cel_Status_Clear(out);
    return true;
  }
  cel_Status_SetCanonicalCode(out, cel_StatusCode_FromProto(in_code));
  if (!cel_Status_SetMessage(out, google_rpc_Status_message(in))) {
    return false;
  }

  cel_Status_ClearPayloads(out);

  size_t in_payloads_len;
  const google_protobuf_Any* const* in_payloads =
      google_rpc_Status_details(in, &in_payloads_len);
  for (size_t i = 0; i < in_payloads_len; ++i) {
    const google_protobuf_Any* in_payload = in_payloads[i];
    CEL_ASSERT_NOT_NULL(in_payload);
    if (in_payload == cel_nullptr) {
      continue;
    }
    if (!cel_Status_SetPayload(out, google_protobuf_Any_type_url(in_payload),
                               google_protobuf_Any_value(in_payload))) {
      return false;
    }
  }
  return true;
}
