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

#include "cel-c/timestamp_proto.h"

#include <stdbool.h>
#include <stdint.h>

#include "google/protobuf/timestamp.upb.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/timestamp.h"

extern "C" void cel_Timestamp_ToProto(cel_Timestamp in,
                                      CEL_NONNULL(google_protobuf_Timestamp*)
                                          out) {
  CEL_ASSERT_NOT_NULL(out);

  int64_t sec;
  int32_t nsec;
  cel_Timestamp_ToUnix(in, &sec, &nsec);
  google_protobuf_Timestamp_set_seconds(out, sec);
  google_protobuf_Timestamp_set_nanos(out, nsec);
}

extern "C" bool cel_Timestamp_FromProto(
    CEL_NONNULL(cel_Timestamp*) out,
    CEL_NONNULL(const google_protobuf_Timestamp*) in) {
  CEL_ASSERT_NOT_NULL(out);
  CEL_ASSERT_NOT_NULL(in);

  int64_t sec;
  int32_t nsec;
  sec = google_protobuf_Timestamp_seconds(in);
  nsec = google_protobuf_Timestamp_nanos(in);
  if (!cel_Timestamp_Normalize(&sec, &nsec)) {
    return false;
  }
  *out = cel_Timestamp_FromUnix(sec, nsec);
  return true;
}
