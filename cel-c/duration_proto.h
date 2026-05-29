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

#ifndef THIRD_PARTY_CEL_C_DURATION_PROTO_H_
#define THIRD_PARTY_CEL_C_DURATION_PROTO_H_

#include <stdbool.h>  // IWYU pragma: keep

#include "google/protobuf/duration.upb.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"

CEL_BEGIN_DECLS

// cel_Duration_ToProto
//
// Converts `cel_Duration` to `google_protobuf_Duration`.
CEL_ATTRIBUTE_NOTHROW
void cel_Duration_ToProto(cel_Duration in,
                          CEL_NONNULL(google_protobuf_Duration*) out);

// cel_Duration_FromProto
//
// Converts `google_protobuf_Duration` to `cel_Duration`. Returns `false` if an
// error occurred, `true` otherwise.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool cel_Duration_FromProto(CEL_NONNULL(cel_Duration*) out,
                            CEL_NONNULL(const google_protobuf_Duration*) in);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_DURATION_PROTO_H_
