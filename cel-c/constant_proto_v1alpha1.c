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

#include "cel-c/constant_proto_v1alpha1.h"

#include <inttypes.h>
#include <stdbool.h>

#include "google/api/expr/v1alpha1/syntax.upb.h"
#include "google/protobuf/duration.upb.h"
#include "google/protobuf/struct.upb.h"
#include "google/protobuf/timestamp.upb.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/constant.h"
#include "cel-c/duration.h"
#include "cel-c/duration_proto.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"
#include "cel-c/timestamp_proto.h"
#include "upb/mem/arena.h"

bool cel_Constant_ToProtoV1Alpha1(
    CEL_NONNULL(const cel_Constant*) in, CEL_NONNULL(upb_Arena*) arena,
    CEL_NONNULL(google_api_expr_v1alpha1_Constant*) out,
    CEL_NONNULL(cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(in);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_NOT_NULL(out);
  CEL_ASSERT_NOT_NULL(status);

  if (!cel_Status_Ok(status)) {
    return false;
  }
  switch (cel_Constant_Kind(in)) {
    case cel_ConstantKind_kUnspecified:
      google_api_expr_v1alpha1_Constant_clear_constant_kind(out);
      break;
    case cel_ConstantKind_kNull:
      google_api_expr_v1alpha1_Constant_set_null_value(
          out, google_protobuf_NULL_VALUE);
      break;
    case cel_ConstantKind_kBool:
      google_api_expr_v1alpha1_Constant_set_bool_value(
          out, cel_Constant_GetBool(in));
      break;
    case cel_ConstantKind_kInt:
      google_api_expr_v1alpha1_Constant_set_int64_value(
          out, cel_Constant_GetInt(in));
      break;
    case cel_ConstantKind_kUint:
      google_api_expr_v1alpha1_Constant_set_uint64_value(
          out, cel_Constant_GetUint(in));
      break;
    case cel_ConstantKind_kDouble:
      google_api_expr_v1alpha1_Constant_set_double_value(
          out, cel_Constant_GetDouble(in));
      break;
    case cel_ConstantKind_kBytes:
      google_api_expr_v1alpha1_Constant_set_bytes_value(
          out, cel_Constant_GetBytes(in));
      break;
    case cel_ConstantKind_kString:
      google_api_expr_v1alpha1_Constant_set_string_value(
          out, cel_Constant_GetString(in));
      break;
    case cel_ConstantKind_kDuration: {
      CEL_NULLABLE(google_protobuf_Duration*)
      out_duration = google_protobuf_Duration_new(arena);
      if (out_duration == cel_nullptr) {
        cel_OutOfMemoryStatus(status);
        return false;
      }
      cel_Duration_ToProto(cel_Constant_GetDuration(in), out_duration);
      google_api_expr_v1alpha1_Constant_set_duration_value(out, out_duration);
    } break;
    case cel_ConstantKind_kTimestamp: {
      CEL_NULLABLE(google_protobuf_Timestamp*)
      out_timestamp = google_protobuf_Timestamp_new(arena);
      if (out_timestamp == cel_nullptr) {
        cel_OutOfMemoryStatus(status);
        return false;
      }
      cel_Timestamp_ToProto(cel_Constant_GetTimestamp(in), out_timestamp);
      google_api_expr_v1alpha1_Constant_set_timestamp_value(out, out_timestamp);
    } break;
    default:
      cel_InvalidArgumentStatusF(status, "cel: unexpected constant kind: %d",
                                 cel_Constant_Kind(in));
      return false;
  }
  return true;
}

bool cel_Constant_FromProtoV1Alpha1(
    CEL_NONNULL(cel_Constant*) out,
    CEL_NONNULL(const google_api_expr_v1alpha1_Constant*) in,
    CEL_NONNULL(cel_Arena*) arena, CEL_NONNULL(cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(out);
  CEL_ASSERT_NOT_NULL(in);
  CEL_ASSERT_NOT_NULL(status);

  if (!cel_Status_Ok(status)) {
    return false;
  }
  switch (google_api_expr_v1alpha1_Constant_constant_kind_case(in)) {
    case google_api_expr_v1alpha1_Constant_constant_kind_NOT_SET:
      *out = cel_UnspecifiedConstant();
      break;
    case google_api_expr_v1alpha1_Constant_constant_kind_null_value:
      cel_Constant_SetNull(out);
      break;
    case google_api_expr_v1alpha1_Constant_constant_kind_bool_value:
      cel_Constant_SetBool(out,
                           google_api_expr_v1alpha1_Constant_bool_value(in));
      break;
    case google_api_expr_v1alpha1_Constant_constant_kind_int64_value:
      cel_Constant_SetInt(out,
                          google_api_expr_v1alpha1_Constant_int64_value(in));
      break;
    case google_api_expr_v1alpha1_Constant_constant_kind_uint64_value:
      cel_Constant_SetUint(out,
                           google_api_expr_v1alpha1_Constant_uint64_value(in));
      break;
    case google_api_expr_v1alpha1_Constant_constant_kind_double_value:
      cel_Constant_SetDouble(
          out, google_api_expr_v1alpha1_Constant_double_value(in));
      break;
    case google_api_expr_v1alpha1_Constant_constant_kind_bytes_value: {
      cel_StringView val = google_api_expr_v1alpha1_Constant_bytes_value(in);
      if (!cel_Arena_StrDup(arena, &val, val)) {
        cel_OutOfMemoryStatus(status);
        return false;
      }
      cel_Constant_SetBytes(out, val);
    } break;
    case google_api_expr_v1alpha1_Constant_constant_kind_string_value: {
      cel_StringView val = google_api_expr_v1alpha1_Constant_string_value(in);
      if (!cel_Arena_StrDup(arena, &val, val)) {
        cel_OutOfMemoryStatus(status);
        return false;
      }
      cel_Constant_SetString(out, val);
    } break;
    case google_api_expr_v1alpha1_Constant_constant_kind_duration_value: {
      CEL_NULLABILITY_UNKNOWN(const google_protobuf_Duration*)
      in_duration = google_api_expr_v1alpha1_Constant_duration_value(in);
      cel_Duration out_duration;
      if (in_duration != cel_nullptr) {
        if (!cel_Duration_FromProto(&out_duration, in_duration)) {
          cel_InvalidArgumentStatusF(
              status,
              "cel: invalid google.protobuf.Duration: { seconds: %" PRId64
              " nanos: %" PRId32 " }",
              google_protobuf_Duration_seconds(in_duration),
              google_protobuf_Duration_nanos(in_duration));
          return false;
        }
      } else {
        out_duration = cel_Duration_kZero;
      }
      cel_Constant_SetDuration(out, out_duration);
    } break;
    case google_api_expr_v1alpha1_Constant_constant_kind_timestamp_value: {
      CEL_NULLABILITY_UNKNOWN(const google_protobuf_Timestamp*)
      in_timestamp = google_api_expr_v1alpha1_Constant_timestamp_value(in);
      cel_Timestamp out_timestamp;
      if (in_timestamp != cel_nullptr) {
        if (!cel_Timestamp_FromProto(&out_timestamp, in_timestamp)) {
          cel_InvalidArgumentStatusF(
              status,
              "cel: invalid google.protobuf.Timestamp: { seconds: %" PRId64
              " nanos: %" PRId32 " }",
              google_protobuf_Timestamp_seconds(in_timestamp),
              google_protobuf_Timestamp_nanos(in_timestamp));
          return false;
        }
      } else {
        out_timestamp = cel_Timestamp_kUnixEpoch;
      }
      cel_Constant_SetTimestamp(out, out_timestamp);
    } break;
    default:
      cel_InvalidArgumentStatusF(
          status, "cel: unexpected constant kind: %d",
          google_api_expr_v1alpha1_Constant_constant_kind_case(in));
      return false;
  }
  return true;
}
