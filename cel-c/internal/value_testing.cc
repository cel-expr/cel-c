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

#include "cel-c/internal/value_testing.h"

#include <cstring>
#include <memory>
#include <string>

#include "google/protobuf/any.pb.h"
#include "google/protobuf/any.upbdefs.h"
#include "google/protobuf/duration.pb.h"
#include "google/protobuf/duration.upbdefs.h"
#include "google/protobuf/field_mask.pb.h"
#include "google/protobuf/field_mask.upbdefs.h"
#include "google/protobuf/struct.pb.h"
#include "google/protobuf/struct.upbdefs.h"
#include "google/protobuf/timestamp.pb.h"
#include "google/protobuf/timestamp.upbdefs.h"
#include "google/protobuf/wrappers.pb.h"
#include "google/protobuf/wrappers.upbdefs.h"
#include "absl/log/absl_check.h"
#include "absl/log/die_if_null.h"
#include "absl/strings/string_view.h"
#include "cel-c/arena.h"
#include "cel-c/internal/config.h"
#include "cel-c/status.h"
#include "cel-c/value.h"
#include "cel-c/well_known_types.h"
#include "cel/expr/conformance/proto3/test_all_types.pb.h"
#include "cel/expr/conformance/proto3/test_all_types.upbdefs.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/unknown_field_set.h"
#include "upb/message/array.h"
#include "upb/message/message.h"
#include "upb/reflection/def.h"
#include "upb/reflection/message.h"
#include "upb/wire/decode.h"

void ValueTest::SetUp() {
  arena_ = ABSL_DIE_IF_NULL(cel_Arena_New(alloc()));
  def_pool_ = ABSL_DIE_IF_NULL(upb_DefPool_New());
  cel_Status_Construct(&status_);
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_BoolValue_getmsgdef(def_pool_)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_Int32Value_getmsgdef(def_pool_)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_Int64Value_getmsgdef(def_pool_)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_UInt32Value_getmsgdef(def_pool_)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_UInt64Value_getmsgdef(def_pool_)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_FloatValue_getmsgdef(def_pool_)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_DoubleValue_getmsgdef(def_pool_)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_BytesValue_getmsgdef(def_pool_)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_StringValue_getmsgdef(def_pool_)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_Duration_getmsgdef(def_pool_)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_Timestamp_getmsgdef(def_pool_)));
  static_cast<void>(ABSL_DIE_IF_NULL(google_protobuf_Any_getmsgdef(def_pool_)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_Value_getmsgdef(def_pool_)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_Struct_getmsgdef(def_pool_)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_ListValue_getmsgdef(def_pool_)));
  static_cast<void>(
      ABSL_DIE_IF_NULL(google_protobuf_FieldMask_getmsgdef(def_pool_)));
  test_all_types_def_ = ABSL_DIE_IF_NULL(
      cel_expr_conformance_proto3_TestAllTypes_getmsgdef(def_pool_));
  ABSL_CHECK(cel_WellKnownTypes_Initialize(&wkts_, def_pool_, &status_));
  google::protobuf::LinkMessageReflection<google::protobuf::BoolValue>();
  google::protobuf::LinkMessageReflection<google::protobuf::Int32Value>();
  google::protobuf::LinkMessageReflection<google::protobuf::Int64Value>();
  google::protobuf::LinkMessageReflection<google::protobuf::UInt32Value>();
  google::protobuf::LinkMessageReflection<google::protobuf::UInt64Value>();
  google::protobuf::LinkMessageReflection<google::protobuf::FloatValue>();
  google::protobuf::LinkMessageReflection<google::protobuf::DoubleValue>();
  google::protobuf::LinkMessageReflection<google::protobuf::BytesValue>();
  google::protobuf::LinkMessageReflection<google::protobuf::StringValue>();
  google::protobuf::LinkMessageReflection<google::protobuf::Duration>();
  google::protobuf::LinkMessageReflection<google::protobuf::Timestamp>();
  google::protobuf::LinkMessageReflection<google::protobuf::Any>();
  google::protobuf::LinkMessageReflection<google::protobuf::Value>();
  google::protobuf::LinkMessageReflection<google::protobuf::Struct>();
  google::protobuf::LinkMessageReflection<google::protobuf::ListValue>();
  google::protobuf::LinkMessageReflection<google::protobuf::FieldMask>();
  google::protobuf::LinkMessageReflection<cel::expr::conformance::proto3::TestAllTypes>();
  std::memset(&ctx_, 0, sizeof(ctx_));
  ctx_.alloc = alloc();
  ctx_.arena = arena();
  ctx_.def_pool = def_pool();
  ctx_.well_known_types = wkts();
}

void ValueTest::TearDown() {
  cel_Status_Destruct(&status_);
  upb_DefPool_Free(def_pool_);
  def_pool_ = nullptr;
  cel_Arena_Delete(arena_);
  arena_ = nullptr;
}

upb_Message* ValueTest::ParseProto(absl::string_view name,
                                   absl::string_view text,
                                   absl::string_view binary) {
  const google::protobuf::Descriptor* proto_def = ABSL_DIE_IF_NULL(
      google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(name));
  std::unique_ptr<google::protobuf::Message> proto_message(
      ABSL_DIE_IF_NULL(
          google::protobuf::MessageFactory::generated_factory()->GetPrototype(proto_def))
          ->New());
  ABSL_CHECK(google::protobuf::TextFormat::ParseFromString(text, proto_message.get()));
  std::string proto_serialized;
  ABSL_CHECK(proto_message->SerializePartialToString(&proto_serialized));
  const upb_MessageDef* upb_def =
      ABSL_DIE_IF_NULL(upb_DefPool_FindMessageByNameWithSize(
          def_pool(), name.data(), name.size()));
  upb_Message* upb_message = ABSL_DIE_IF_NULL(
      upb_Message_New(upb_MessageDef_MiniTable(upb_def), arena()));
  ABSL_CHECK_EQ(
      upb_Decode(proto_serialized.data(), proto_serialized.size(), upb_message,
                 upb_MessageDef_MiniTable(upb_def),
                 upb_DefPool_ExtensionRegistry(def_pool()), 0, arena()),
      kUpb_DecodeStatus_Ok);
  ABSL_CHECK_EQ(
      upb_Decode(binary.data(), binary.size(), upb_message,
                 upb_MessageDef_MiniTable(upb_def),
                 upb_DefPool_ExtensionRegistry(def_pool()), 0, arena()),
      kUpb_DecodeStatus_Ok);
  return upb_message;
}

const upb_MessageDef* cel_nonnull
ValueTest::MessageDef(absl::string_view name) {
  return ABSL_DIE_IF_NULL(upb_DefPool_FindMessageByNameWithSize(
      def_pool(), name.data(), name.size()));
}

const upb_FieldDef* cel_nonnull
ValueTest::TestAllTypesFieldDef(absl::string_view name) {
  return ABSL_DIE_IF_NULL(upb_MessageDef_FindFieldByNameWithSize(
      TestAllTypesDef(), name.data(), name.size()));
}

upb_MessageValue ValueTest::TestAllTypesField(
    const upb_Message* cel_nonnull msg, const upb_FieldDef* cel_nonnull field) {
  return upb_Message_GetFieldByDef(msg, field);
}

upb_MessageValue ValueTest::TestAllTypesField(
    const upb_Message* cel_nonnull msg, absl::string_view field) {
  return TestAllTypesField(msg, TestAllTypesFieldDef(field));
}

const upb_EnumDef* cel_nonnull ValueTest::EnumDef(absl::string_view name) {
  return ABSL_DIE_IF_NULL(
      upb_DefPool_FindEnumByName(def_pool(), std::string(name).c_str()));
}

const upb_EnumValueDef* cel_nonnull
ValueTest::EnumValueDef(const upb_EnumDef* enm, absl::string_view value) {
  return ABSL_DIE_IF_NULL(
      upb_EnumDef_FindValueByNameWithSize(enm, value.data(), value.size()));
}

const upb_EnumValueDef* cel_nonnull
ValueTest::EnumValueDef(absl::string_view name, absl::string_view value) {
  return ABSL_DIE_IF_NULL(upb_EnumDef_FindValueByNameWithSize(
      EnumDef(name), value.data(), value.size()));
}
