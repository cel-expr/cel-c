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

#include "cel-c/ref_proto.h"

#include <stddef.h>

#include "cel/expr/checked.upb.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/constant_proto.h"
#include "cel-c/ref.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "upb/base/string_view.h"

cel_Ref* cel_nullable cel_Ref_FromProto(
    const cel_expr_Reference* cel_nonnull in,
    cel_Arena* cel_nonnull arena, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(in);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_NOT_NULL(status);
  CEL_ASSERT(cel_Status_Ok(status));

  cel_StringView out_name;
  if (!cel_Arena_StrDup(arena, &out_name, cel_expr_Reference_name(in))) {
    cel_OutOfMemoryStatus(status);
    return cel_nullptr;
  }
  size_t in_overloads_len;
  const upb_StringView* in_overloads_ptr =
      cel_expr_Reference_overload_id(in, &in_overloads_len);
  if (in_overloads_len > 0 && cel_expr_Reference_has_value(in)) {
    cel_InvalidArgumentStatus(
        status, cel_StringView_From("cel: resolved reference has both function "
                                    "overloads and a constant"));
    return cel_nullptr;
  }
  if (in_overloads_len > 0) {
    cel_FunctionRef* out = cel_FunctionRef_New(out_name, arena);
    if (out == cel_nullptr) {
      cel_OutOfMemoryStatus(status);
      return cel_nullptr;
    }
    for (size_t i = 0; i < in_overloads_len; ++i) {
      cel_StringView out_id;
      if (!cel_Arena_StrDup(arena, &out_id, in_overloads_ptr[i])) {
        cel_OutOfMemoryStatus(status);
        return cel_nullptr;
      }
      cel_FunctionOverloadRef* overload_ref =
          cel_FunctionOverloadRef_New(out_id, arena);
      if (overload_ref == cel_nullptr) {
        cel_OutOfMemoryStatus(status);
        return cel_nullptr;
      }
      CEL_USED(cel_FunctionRef_AppendOverload(out, overload_ref));
    }
    return cel_Ref_UpCast(out);
  }
  cel_IdentRef* out = cel_IdentRef_New(out_name, arena);
  if (out == cel_nullptr) {
    cel_OutOfMemoryStatus(status);
    return cel_nullptr;
  }
  if (cel_expr_Reference_has_value(in)) {
    if (!cel_Constant_FromProto(cel_IdentRef_MutableValue(out),
                                cel_expr_Reference_value(in), arena,
                                status)) {
      return cel_nullptr;
    }
  }
  return cel_Ref_UpCast(out);
}
