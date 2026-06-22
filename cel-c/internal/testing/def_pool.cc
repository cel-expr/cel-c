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

#include "cel-c/internal/testing/def_pool.h"

#include "google/protobuf/any.upbdefs.h"
#include "google/protobuf/duration.upbdefs.h"
#include "google/protobuf/field_mask.upbdefs.h"
#include "google/protobuf/struct.upbdefs.h"
#include "google/protobuf/timestamp.upbdefs.h"
#include "google/protobuf/wrappers.upbdefs.h"
#include "absl/log/die_if_null.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel/expr/conformance/proto3/test_all_types.upbdefs.h"
#include "upb/reflection/def.h"

extern "C" CEL_ATTRIBUTE_NOTHROW const upb_DefPool* cel_nonnull
_cel_TestingDefPool() {
  static const upb_DefPool* def_pool = []() -> const upb_DefPool* {
    upb_DefPool* def_pool = ABSL_DIE_IF_NULL(upb_DefPool_New());
    _cel_TestingDefs(def_pool);
    return def_pool;
  }();
  return def_pool;
}

extern "C" CEL_ATTRIBUTE_NOTHROW void _cel_TestingDefs(
    upb_DefPool* cel_nonnull def_pool) {
  CEL_ASSERT_NOT_NULL(def_pool);

  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_BoolValue_getmsgdef(def_pool)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_Int32Value_getmsgdef(def_pool)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_Int64Value_getmsgdef(def_pool)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_UInt32Value_getmsgdef(def_pool)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_UInt64Value_getmsgdef(def_pool)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_FloatValue_getmsgdef(def_pool)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_DoubleValue_getmsgdef(def_pool)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_StringValue_getmsgdef(def_pool)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_BytesValue_getmsgdef(def_pool)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_Duration_getmsgdef(def_pool)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_Timestamp_getmsgdef(def_pool)));
  static_cast<void>(ABSL_DIE_IF_NULL(google_protobuf_Any_getmsgdef(def_pool)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_Value_getmsgdef(def_pool)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_ListValue_getmsgdef(def_pool)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_Struct_getmsgdef(def_pool)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_FieldMask_getmsgdef(def_pool)));
  static_cast<void>(ABSL_DIE_IF_NULL(
      cel_expr_conformance_proto3_TestAllTypes_getmsgdef(def_pool)));
}
