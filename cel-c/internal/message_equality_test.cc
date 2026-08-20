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

#include "cel-c/internal/message_equality.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <variant>

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
#include "gtest/gtest.h"
#include "absl/base/casts.h"
#include "absl/functional/overload.h"
#include "absl/log/absl_check.h"
#include "absl/log/die_if_null.h"
#include "absl/strings/string_view.h"
#include "absl/types/variant.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/internal/config.h"
#include "cel-c/status.h"
#include "cel-c/well_known_types.h"
#include "cel/expr/conformance/proto2/test_all_types.upbdefs.h"
#include "cel/expr/conformance/proto2/test_all_types_extensions.upb_minitable.h"
#include "cel/expr/conformance/proto3/test_all_types.pb.h"
#include "cel/expr/conformance/proto3/test_all_types.upbdefs.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/unknown_field_set.h"
#include "upb/message/array.h"
#include "upb/message/message.h"
#include "upb/message/unknown_fields_testonly.h"
#include "upb/mini_table/message.h"
#include "upb/reflection/def.h"
#include "upb/reflection/message.h"
#include "upb/wire/decode.h"

namespace {

class UnknownFields;

class UnknownField final {
 public:
  static UnknownField Varint(int number, int64_t value) {
    UnknownField field;
    field.number_ = number;
    field.value_.emplace<int64_t>(value);
    return field;
  }

  static UnknownField Fixed32(int number, uint32_t value) {
    UnknownField field;
    field.number_ = number;
    field.value_.emplace<uint32_t>(value);
    return field;
  }

  static UnknownField Fixed64(int number, uint64_t value) {
    UnknownField field;
    field.number_ = number;
    field.value_.emplace<uint64_t>(value);
    return field;
  }

  static UnknownField LengthDelimited(int number, absl::string_view value) {
    UnknownField field;
    field.number_ = number;
    field.value_.emplace<std::string>(std::string(value));
    return field;
  }

  static UnknownField Group(int number, const UnknownFields& value);

 private:
  friend class UnknownFields;

  int number_;
  std::variant<int64_t, uint32_t, uint64_t, std::string,
               std::unique_ptr<UnknownFields>>
      value_;
};

class UnknownFields final {
 public:
  template <typename... Fields>
  // NOLINTNEXTLINE(google-explicit-constructor)
  UnknownFields(Fields&&... fields) {
    (Add(std::forward<Fields>(fields)), ...);
  }

  // NOLINTNEXTLINE(google-explicit-constructor)
  operator std::string() const {
    std::string serialized;
    ABSL_CHECK(unknowns_.SerializeToString(&serialized));
    return serialized;
  }

 private:
  friend class UnknownField;

  void Add(const UnknownField& field) {
    absl::visit(
        absl::Overload(
            [this, &field](int64_t value) -> void {
              unknowns_.AddVarint(field.number_,
                                  absl::bit_cast<uint64_t>(value));
            },
            [this, &field](uint32_t value) -> void {
              unknowns_.AddFixed32(field.number_, value);
            },
            [this, &field](uint64_t value) -> void {
              unknowns_.AddFixed64(field.number_, value);
            },
            [this, &field](const std::string& value) -> void {
              unknowns_.AddLengthDelimited(field.number_, value);
            },
            [this,
             &field](const std::unique_ptr<UnknownFields>& value) -> void {
              unknowns_.AddGroup(field.number_)->MergeFrom(value->unknowns_);
            }),
        field.value_);
  }

  google::protobuf::UnknownFieldSet unknowns_;
};

UnknownField UnknownField::Group(int number, const UnknownFields& value) {
  UnknownField field;
  field.number_ = number;
  field.value_
      .emplace<std::unique_ptr<UnknownFields>>(
          std::make_unique<UnknownFields>())
      ->unknowns_.MergeFrom(value.unknowns_);
  return field;
}

struct MessageEqualityTestParam {
  absl::string_view message_name;
  absl::string_view lhs_message_text;
  std::string lhs_message_binary;
  absl::string_view rhs_message_text;
  std::string rhs_message_binary;
  _cel_MessageEquality result = _cel_MessageEquality_kEqual;
};

class MessageEqualityTest
    : public ::testing::TestWithParam<MessageEqualityTestParam> {
 public:
  void SetUp() override {
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
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Any_getmsgdef(def_pool_)));
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
    google::protobuf::LinkMessageReflection<
        cel::expr::conformance::proto3::TestAllTypes>();
  }

  void TearDown() override {
    cel_Status_Destruct(&status_);
    upb_DefPool_Free(def_pool_);
    def_pool_ = nullptr;
    cel_Arena_Delete(arena_);
    arena_ = nullptr;
  }

 protected:
  upb_Message* ParseProto(absl::string_view name, absl::string_view text,
                          absl::string_view binary) {
    const google::protobuf::Descriptor* proto_def = ABSL_DIE_IF_NULL(
        google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(name));
    std::unique_ptr<google::protobuf::Message> proto_message(
        ABSL_DIE_IF_NULL(
            google::protobuf::MessageFactory::generated_factory()->GetPrototype(
                proto_def))
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
        upb_Decode(proto_serialized.data(), proto_serialized.size(),
                   upb_message, upb_MessageDef_MiniTable(upb_def),
                   upb_DefPool_ExtensionRegistry(def_pool()), 0, arena()),
        kUpb_DecodeStatus_Ok);
    ABSL_CHECK_EQ(
        upb_Decode(binary.data(), binary.size(), upb_message,
                   upb_MessageDef_MiniTable(upb_def),
                   upb_DefPool_ExtensionRegistry(def_pool()), 0, arena()),
        kUpb_DecodeStatus_Ok);
    return upb_message;
  }

  cel_Allocator* cel_nonnull alloc() { return cel_DefaultAllocator; }

  cel_Arena* cel_nonnull arena() { return ABSL_DIE_IF_NULL(arena_); }

  upb_DefPool* cel_nonnull def_pool() { return ABSL_DIE_IF_NULL(def_pool_); }

  cel_WellKnownTypes* cel_nonnull wkts() { return &wkts_; }

  const upb_MessageDef* cel_nonnull MessageDef(absl::string_view name) {
    return ABSL_DIE_IF_NULL(upb_DefPool_FindMessageByNameWithSize(
        def_pool(), name.data(), name.size()));
  }

  const upb_MessageDef* cel_nonnull TestAllTypesDef() {
    return ABSL_DIE_IF_NULL(test_all_types_def_);
  }

  const upb_FieldDef* cel_nonnull TestAllTypesFieldDef(absl::string_view name) {
    return ABSL_DIE_IF_NULL(upb_MessageDef_FindFieldByNameWithSize(
        TestAllTypesDef(), name.data(), name.size()));
  }

  upb_MessageValue TestAllTypesField(const upb_Message* cel_nonnull msg,
                                     const upb_FieldDef* cel_nonnull field) {
    return upb_Message_GetFieldByDef(msg, field);
  }

  upb_MessageValue TestAllTypesField(const upb_Message* cel_nonnull msg,
                                     absl::string_view field) {
    return TestAllTypesField(msg, TestAllTypesFieldDef(field));
  }

 private:
  cel_Arena* cel_nullable arena_ = nullptr;
  upb_DefPool* cel_nullable def_pool_ = nullptr;
  const upb_MessageDef* cel_nullable test_all_types_def_ = nullptr;
  cel_Status status_;
  cel_WellKnownTypes wkts_;
};

using HomogeneousMessageEqualityTest = MessageEqualityTest;

TEST_P(HomogeneousMessageEqualityTest, Equals) {
  const auto& param = GetParam();

  upb_Message* lhs_message = ParseProto(
      param.message_name, param.lhs_message_text, param.lhs_message_binary);
  upb_Message* rhs_message = ParseProto(
      param.message_name, param.rhs_message_text, param.rhs_message_binary);
  const upb_MessageDef* message_def = MessageDef(param.message_name);

  EXPECT_EQ(_cel_Message_Equals(lhs_message, rhs_message, message_def,
                                def_pool(), wkts(), alloc()),
            param.result);
  EXPECT_EQ(_cel_Message_Equals(rhs_message, lhs_message, message_def,
                                def_pool(), wkts(), alloc()),
            param.result);
}

TEST_P(HomogeneousMessageEqualityTest, FieldEquals) {
  const auto& param = GetParam();

  upb_Message* lhs_message = ParseProto(
      param.message_name, param.lhs_message_text, param.lhs_message_binary);
  upb_Message* rhs_message = ParseProto(
      param.message_name, param.rhs_message_text, param.rhs_message_binary);
  const upb_MessageDef* message_def = MessageDef(param.message_name);
  const upb_FieldDef* lhs_message_field_def;
  upb_MessageValue lhs_message_field_val;
  size_t lhs_message_iter = kUpb_Message_Begin;
  if (!upb_Message_Next(lhs_message, message_def, def_pool(),
                        &lhs_message_field_def, &lhs_message_field_val,
                        &lhs_message_iter)) {
    return;
  }
  const upb_FieldDef* rhs_message_field_def;
  upb_MessageValue rhs_message_field_val;
  size_t rhs_message_iter = kUpb_Message_Begin;
  if (!upb_Message_Next(rhs_message, message_def, def_pool(),
                        &rhs_message_field_def, &rhs_message_field_val,
                        &rhs_message_iter)) {
    return;
  }
  if (lhs_message_field_def != rhs_message_field_def) {
    return;
  }
  EXPECT_EQ(
      _cel_MessageField_Equals(lhs_message_field_val, lhs_message_field_def,
                               rhs_message_field_val, rhs_message_field_def,
                               def_pool(), wkts(), alloc()),
      param.result);
  EXPECT_EQ(
      _cel_MessageField_Equals(rhs_message_field_val, rhs_message_field_def,
                               lhs_message_field_val, lhs_message_field_def,
                               def_pool(), wkts(), alloc()),
      param.result);
  ASSERT_FALSE(upb_Message_Next(lhs_message, message_def, def_pool(),
                                &lhs_message_field_def, &lhs_message_field_val,
                                &lhs_message_iter));
  ASSERT_FALSE(upb_Message_Next(rhs_message, message_def, def_pool(),
                                &rhs_message_field_def, &rhs_message_field_val,
                                &rhs_message_iter));
}

INSTANTIATE_TEST_SUITE_P(
    Unknown, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary = UnknownFields(
                UnknownField::Varint(10000, 1), UnknownField::Fixed32(10001, 2),
                UnknownField::Fixed64(10002, 3),
                UnknownField::LengthDelimited(10003, "4"),
                UnknownField::Group(
                    10004, UnknownFields(UnknownField::Varint(10005, 5)))),
            .rhs_message_binary = UnknownFields(
                UnknownField::Group(
                    10004, UnknownFields(UnknownField::Varint(10005, 5))),
                UnknownField::LengthDelimited(10003, "4"),
                UnknownField::Fixed64(10002, 3),
                UnknownField::Fixed32(10001, 2),
                UnknownField::Varint(10000, 1)),
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary =
                UnknownFields(UnknownField::LengthDelimited(10003, "foo"),
                              UnknownField::Fixed32(10001, 2),
                              UnknownField::Fixed32(10001, 3),
                              UnknownField::Fixed32(10001, 4),
                              UnknownField::Fixed32(10001, 5),
                              UnknownField::Fixed64(10002, 6),
                              UnknownField::Varint(10000, 1)),
            .rhs_message_binary =
                UnknownFields(UnknownField::Varint(10000, 1),
                              UnknownField::Fixed32(10001, 2),
                              UnknownField::Fixed32(10001, 3),
                              UnknownField::Fixed32(10001, 4),
                              UnknownField::Fixed32(10001, 5),
                              UnknownField::Fixed64(10002, 6),
                              UnknownField::LengthDelimited(10003, "foo")),
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary =
                UnknownFields(UnknownField::Varint(10000, 1),
                              UnknownField::LengthDelimited(10000, "4")),
            .rhs_message_binary =
                UnknownFields(UnknownField::LengthDelimited(10000, "4"),
                              UnknownField::Varint(10000, 1)),
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary = UnknownFields(UnknownField::Varint(10000, 1)),
            .rhs_message_binary = UnknownFields(UnknownField::Varint(10000, 1)),
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary = UnknownFields(UnknownField::Varint(10000, 1)),
            .rhs_message_binary = UnknownFields(UnknownField::Varint(10000, 2)),
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary = UnknownFields(UnknownField::Varint(10000, 2)),
            .rhs_message_binary = UnknownFields(UnknownField::Varint(10000, 1)),
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary = UnknownFields(UnknownField::Varint(10000, 2)),
            .rhs_message_binary = UnknownFields(UnknownField::Varint(10000, 2)),
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary = UnknownFields(UnknownField::Fixed32(10000,
                                                                      1)),
            .rhs_message_binary = UnknownFields(UnknownField::Fixed32(10000,
                                                                      1)),
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary = UnknownFields(UnknownField::Fixed32(10000,
                                                                      1)),
            .rhs_message_binary = UnknownFields(UnknownField::Fixed32(10000,
                                                                      2)),
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary = UnknownFields(UnknownField::Fixed32(10000,
                                                                      2)),
            .rhs_message_binary = UnknownFields(UnknownField::Fixed32(10000,
                                                                      1)),
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary = UnknownFields(UnknownField::Fixed32(10000,
                                                                      2)),
            .rhs_message_binary = UnknownFields(UnknownField::Fixed32(10000,
                                                                      2)),
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary = UnknownFields(UnknownField::Fixed64(10000,
                                                                      1)),
            .rhs_message_binary = UnknownFields(UnknownField::Fixed64(10000,
                                                                      1)),
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary = UnknownFields(UnknownField::Fixed64(10000,
                                                                      1)),
            .rhs_message_binary = UnknownFields(UnknownField::Fixed64(10000,
                                                                      2)),
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary = UnknownFields(UnknownField::Fixed64(10000,
                                                                      2)),
            .rhs_message_binary = UnknownFields(UnknownField::Fixed64(10000,
                                                                      1)),
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary = UnknownFields(UnknownField::Fixed64(10000,
                                                                      2)),
            .rhs_message_binary = UnknownFields(UnknownField::Fixed64(10000,
                                                                      2)),
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary =
                UnknownFields(UnknownField::LengthDelimited(10000, "foo")),
            .rhs_message_binary =
                UnknownFields(UnknownField::LengthDelimited(10000, "foo")),
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary =
                UnknownFields(UnknownField::LengthDelimited(10000, "foo")),
            .rhs_message_binary =
                UnknownFields(UnknownField::LengthDelimited(10000, "bar")),
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary =
                UnknownFields(UnknownField::LengthDelimited(10000, "bar")),
            .rhs_message_binary =
                UnknownFields(UnknownField::LengthDelimited(10000, "foo")),
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary =
                UnknownFields(UnknownField::LengthDelimited(10000, "bar")),
            .rhs_message_binary =
                UnknownFields(UnknownField::LengthDelimited(10000, "bar")),
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_binary = UnknownFields(UnknownField::Group(
                10000, UnknownFields(UnknownField::Varint(10000, 1),
                                     UnknownField::Varint(10001, 1)))),
            .rhs_message_binary = UnknownFields(UnknownField::Group(
                10000, UnknownFields(UnknownField::Varint(10001, 1),
                                     UnknownField::Varint(10000, 1)))),
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Bool, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bool: false)pb",
            .rhs_message_text = R"pb(single_bool: false)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bool: false)pb",
            .rhs_message_text = R"pb(single_bool: true)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bool: true)pb",
            .rhs_message_text = R"pb(single_bool: false)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bool: true)pb",
            .rhs_message_text = R"pb(single_bool: true)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Int32, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int32: 0)pb",
            .rhs_message_text = R"pb(single_int32: 0)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int32: 0)pb",
            .rhs_message_text = R"pb(single_int32: 1)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int32: 1)pb",
            .rhs_message_text = R"pb(single_int32: 0)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int32: 1)pb",
            .rhs_message_text = R"pb(single_int32: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    SInt32, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sint32: 0)pb",
            .rhs_message_text = R"pb(single_sint32: 0)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sint32: 0)pb",
            .rhs_message_text = R"pb(single_sint32: 1)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sint32: 1)pb",
            .rhs_message_text = R"pb(single_sint32: 0)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sint32: 1)pb",
            .rhs_message_text = R"pb(single_sint32: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    SFixed32, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sfixed32: 0)pb",
            .rhs_message_text = R"pb(single_sfixed32: 0)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sfixed32: 0)pb",
            .rhs_message_text = R"pb(single_sfixed32: 1)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sfixed32: 1)pb",
            .rhs_message_text = R"pb(single_sfixed32: 0)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sfixed32: 1)pb",
            .rhs_message_text = R"pb(single_sfixed32: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Int64, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int64: 0)pb",
            .rhs_message_text = R"pb(single_int64: 0)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int64: 0)pb",
            .rhs_message_text = R"pb(single_int64: 1)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int64: 1)pb",
            .rhs_message_text = R"pb(single_int64: 0)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int64: 1)pb",
            .rhs_message_text = R"pb(single_int64: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    SInt64, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sint64: 0)pb",
            .rhs_message_text = R"pb(single_sint64: 0)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sint64: 0)pb",
            .rhs_message_text = R"pb(single_sint64: 1)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sint64: 1)pb",
            .rhs_message_text = R"pb(single_sint64: 0)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sint64: 1)pb",
            .rhs_message_text = R"pb(single_sint64: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    SFixed64, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sfixed64: 0)pb",
            .rhs_message_text = R"pb(single_sfixed64: 0)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sfixed64: 0)pb",
            .rhs_message_text = R"pb(single_sfixed64: 1)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sfixed64: 1)pb",
            .rhs_message_text = R"pb(single_sfixed64: 0)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_sfixed64: 1)pb",
            .rhs_message_text = R"pb(single_sfixed64: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    UInt32, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint32: 0)pb",
            .rhs_message_text = R"pb(single_uint32: 0)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint32: 0)pb",
            .rhs_message_text = R"pb(single_uint32: 1)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint32: 1)pb",
            .rhs_message_text = R"pb(single_uint32: 0)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint32: 1)pb",
            .rhs_message_text = R"pb(single_uint32: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Fixed32, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_fixed32: 0)pb",
            .rhs_message_text = R"pb(single_fixed32: 0)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_fixed32: 0)pb",
            .rhs_message_text = R"pb(single_fixed32: 1)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_fixed32: 1)pb",
            .rhs_message_text = R"pb(single_fixed32: 0)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_fixed32: 1)pb",
            .rhs_message_text = R"pb(single_fixed32: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    UInt64, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint64: 0)pb",
            .rhs_message_text = R"pb(single_uint64: 0)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint64: 0)pb",
            .rhs_message_text = R"pb(single_uint64: 1)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint64: 1)pb",
            .rhs_message_text = R"pb(single_uint64: 0)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint64: 1)pb",
            .rhs_message_text = R"pb(single_uint64: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Fixed64, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_fixed64: 0)pb",
            .rhs_message_text = R"pb(single_fixed64: 0)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_fixed64: 0)pb",
            .rhs_message_text = R"pb(single_fixed64: 1)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_fixed64: 1)pb",
            .rhs_message_text = R"pb(single_fixed64: 0)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_fixed64: 1)pb",
            .rhs_message_text = R"pb(single_fixed64: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Float, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_float: 0.0)pb",
            .rhs_message_text = R"pb(single_float: 0.0)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_float: 0.0)pb",
            .rhs_message_text = R"pb(single_float: 1.0)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_float: 1.0)pb",
            .rhs_message_text = R"pb(single_float: 0.0)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_float: 1.0)pb",
            .rhs_message_text = R"pb(single_float: 1.0)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_float: nan)pb",
            .rhs_message_text = R"pb(single_float: nan)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Double, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_double: 0.0)pb",
            .rhs_message_text = R"pb(single_double: 0.0)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_double: 0.0)pb",
            .rhs_message_text = R"pb(single_double: 1.0)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_double: 1.0)pb",
            .rhs_message_text = R"pb(single_double: 0.0)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_double: 1.0)pb",
            .rhs_message_text = R"pb(single_double: 1.0)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_double: nan)pb",
            .rhs_message_text = R"pb(single_double: nan)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Bytes, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bytes: "")pb",
            .rhs_message_text = R"pb(single_bytes: "")pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bytes: "")pb",
            .rhs_message_text = R"pb(single_bytes: "foo")pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bytes: "foo")pb",
            .rhs_message_text = R"pb(single_bytes: "")pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bytes: "foo")pb",
            .rhs_message_text = R"pb(single_bytes: "foo")pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    String, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_string: "")pb",
            .rhs_message_text = R"pb(single_string: "")pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_string: "")pb",
            .rhs_message_text = R"pb(single_string: "foo")pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_string: "foo")pb",
            .rhs_message_text = R"pb(single_string: "")pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_string: "foo")pb",
            .rhs_message_text = R"pb(single_string: "foo")pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    NullValue, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(null_value: NULL_VALUE)pb",
            .rhs_message_text = R"pb(null_value: NULL_VALUE)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    BoolValue, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bool_wrapper: {})pb",
            .rhs_message_text = R"pb(single_bool_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bool_wrapper: { value: false })pb",
            .rhs_message_text = R"pb(single_bool_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bool_wrapper: {})pb",
            .rhs_message_text = R"pb(single_bool_wrapper: { value: false })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bool_wrapper: { value: false })pb",
            .rhs_message_text = R"pb(single_bool_wrapper: { value: false })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bool_wrapper: { value: false })pb",
            .rhs_message_text = R"pb(single_bool_wrapper: { value: true })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bool_wrapper: { value: true })pb",
            .rhs_message_text = R"pb(single_bool_wrapper: { value: false })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bool_wrapper: { value: true })pb",
            .rhs_message_text = R"pb(single_bool_wrapper: { value: true })pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Int32Value, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int32_wrapper: {})pb",
            .rhs_message_text = R"pb(single_int32_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int32_wrapper: { value: 0 })pb",
            .rhs_message_text = R"pb(single_int32_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int32_wrapper: {})pb",
            .rhs_message_text = R"pb(single_int32_wrapper: { value: 0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int32_wrapper: { value: 0 })pb",
            .rhs_message_text = R"pb(single_int32_wrapper: { value: 0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int32_wrapper: { value: 0 })pb",
            .rhs_message_text = R"pb(single_int32_wrapper: { value: 1 })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int32_wrapper: { value: 1 })pb",
            .rhs_message_text = R"pb(single_int32_wrapper: { value: 0 })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int32_wrapper: { value: 1 })pb",
            .rhs_message_text = R"pb(single_int32_wrapper: { value: 1 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Int64Value, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int64_wrapper: {})pb",
            .rhs_message_text = R"pb(single_int64_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int64_wrapper: { value: 0 })pb",
            .rhs_message_text = R"pb(single_int64_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int64_wrapper: {})pb",
            .rhs_message_text = R"pb(single_int64_wrapper: { value: 0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int64_wrapper: { value: 0 })pb",
            .rhs_message_text = R"pb(single_int64_wrapper: { value: 0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int64_wrapper: { value: 0 })pb",
            .rhs_message_text = R"pb(single_int64_wrapper: { value: 1 })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int64_wrapper: { value: 1 })pb",
            .rhs_message_text = R"pb(single_int64_wrapper: { value: 0 })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int64_wrapper: { value: 1 })pb",
            .rhs_message_text = R"pb(single_int64_wrapper: { value: 1 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    UInt32Value, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint32_wrapper: {})pb",
            .rhs_message_text = R"pb(single_uint32_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint32_wrapper: { value: 0 })pb",
            .rhs_message_text = R"pb(single_uint32_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint32_wrapper: {})pb",
            .rhs_message_text = R"pb(single_uint32_wrapper: { value: 0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint32_wrapper: { value: 0 })pb",
            .rhs_message_text = R"pb(single_uint32_wrapper: { value: 0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint32_wrapper: { value: 0 })pb",
            .rhs_message_text = R"pb(single_uint32_wrapper: { value: 1 })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint32_wrapper: { value: 1 })pb",
            .rhs_message_text = R"pb(single_uint32_wrapper: { value: 0 })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint32_wrapper: { value: 1 })pb",
            .rhs_message_text = R"pb(single_uint32_wrapper: { value: 1 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    UInt64Value, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint64_wrapper: {})pb",
            .rhs_message_text = R"pb(single_uint64_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint64_wrapper: { value: 0 })pb",
            .rhs_message_text = R"pb(single_uint64_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint64_wrapper: {})pb",
            .rhs_message_text = R"pb(single_uint64_wrapper: { value: 0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint64_wrapper: { value: 0 })pb",
            .rhs_message_text = R"pb(single_uint64_wrapper: { value: 0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint64_wrapper: { value: 0 })pb",
            .rhs_message_text = R"pb(single_uint64_wrapper: { value: 1 })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint64_wrapper: { value: 1 })pb",
            .rhs_message_text = R"pb(single_uint64_wrapper: { value: 0 })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint64_wrapper: { value: 1 })pb",
            .rhs_message_text = R"pb(single_uint64_wrapper: { value: 1 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    FloatValue, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_float_wrapper: {})pb",
            .rhs_message_text = R"pb(single_float_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_float_wrapper: { value: 0.0 })pb",
            .rhs_message_text = R"pb(single_float_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_float_wrapper: {})pb",
            .rhs_message_text = R"pb(single_float_wrapper: { value: 0.0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_float_wrapper: { value: 0.0 })pb",
            .rhs_message_text = R"pb(single_float_wrapper: { value: 0.0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_float_wrapper: { value: 0.0 })pb",
            .rhs_message_text = R"pb(single_float_wrapper: { value: 1.0 })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_float_wrapper: { value: 1.0 })pb",
            .rhs_message_text = R"pb(single_float_wrapper: { value: 0.0 })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_float_wrapper: { value: 1.0 })pb",
            .rhs_message_text = R"pb(single_float_wrapper: { value: 1.0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_float_wrapper: { value: nan })pb",
            .rhs_message_text = R"pb(single_float_wrapper: { value: nan })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    DoubleValue, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_double_wrapper: {})pb",
            .rhs_message_text = R"pb(single_double_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_double_wrapper: { value: 0.0 })pb",
            .rhs_message_text = R"pb(single_double_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_double_wrapper: {})pb",
            .rhs_message_text = R"pb(single_double_wrapper: { value: 0.0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_double_wrapper: { value: 0.0 })pb",
            .rhs_message_text = R"pb(single_double_wrapper: { value: 0.0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_double_wrapper: { value: 0.0 })pb",
            .rhs_message_text = R"pb(single_double_wrapper: { value: 1.0 })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_double_wrapper: { value: 1.0 })pb",
            .rhs_message_text = R"pb(single_double_wrapper: { value: 0.0 })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_double_wrapper: { value: 1.0 })pb",
            .rhs_message_text = R"pb(single_double_wrapper: { value: 1.0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_double_wrapper: { value: nan })pb",
            .rhs_message_text = R"pb(single_double_wrapper: { value: nan })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    BytesValue, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bytes_wrapper: {})pb",
            .rhs_message_text = R"pb(single_bytes_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bytes_wrapper: { value: "" })pb",
            .rhs_message_text = R"pb(single_bytes_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bytes_wrapper: {})pb",
            .rhs_message_text = R"pb(single_bytes_wrapper: { value: "" })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bytes_wrapper: { value: "" })pb",
            .rhs_message_text = R"pb(single_bytes_wrapper: { value: "" })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bytes_wrapper: { value: "" })pb",
            .rhs_message_text = R"pb(single_bytes_wrapper: { value: "foo" })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bytes_wrapper: { value: "foo" })pb",
            .rhs_message_text = R"pb(single_bytes_wrapper: { value: "" })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_bytes_wrapper: { value: "foo" })pb",
            .rhs_message_text = R"pb(single_bytes_wrapper: { value: "foo" })pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    StringValue, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_string_wrapper: {})pb",
            .rhs_message_text = R"pb(single_string_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_string_wrapper: { value: "" })pb",
            .rhs_message_text = R"pb(single_string_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_string_wrapper: {})pb",
            .rhs_message_text = R"pb(single_string_wrapper: { value: "" })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_string_wrapper: { value: "" })pb",
            .rhs_message_text = R"pb(single_string_wrapper: { value: "" })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_string_wrapper: { value: "" })pb",
            .rhs_message_text =
                R"pb(single_string_wrapper: { value: "foo" })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text =
                R"pb(single_string_wrapper: { value: "foo" })pb",
            .rhs_message_text = R"pb(single_string_wrapper: { value: "" })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text =
                R"pb(single_string_wrapper: { value: "foo" })pb",
            .rhs_message_text =
                R"pb(single_string_wrapper: { value: "foo" })pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Duration, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>(
        {
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(single_duration: {})pb",
                .rhs_message_text = R"pb(single_duration: {})pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(single_duration: {})pb",
                .rhs_message_text = R"pb(single_duration: {
                                           seconds: 0
                                           nanos: 0
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(single_duration: {
                                           seconds: 0
                                           nanos: 0
                                         })pb",
                .rhs_message_text = R"pb(single_duration: {})pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(single_duration: {
                                           seconds: 0
                                           nanos: 0
                                         })pb",
                .rhs_message_text = R"pb(single_duration: {
                                           seconds: 0
                                           nanos: 0
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(single_duration: {
                                           seconds: 0
                                           nanos: 0
                                         })pb",
                .rhs_message_text = R"pb(single_duration: {
                                           seconds: 1
                                           nanos: 1
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(single_duration: {
                                           seconds: 1
                                           nanos: 1
                                         })pb",
                .rhs_message_text = R"pb(single_duration: {
                                           seconds: 0
                                           nanos: 0
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(single_duration: {
                                           seconds: 1
                                           nanos: 1
                                         })pb",
                .rhs_message_text = R"pb(single_duration: {
                                           seconds: 1
                                           nanos: 1
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
        }));

INSTANTIATE_TEST_SUITE_P(
    Timestamp, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>(
        {
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(single_timestamp: {})pb",
                .rhs_message_text = R"pb(single_timestamp: {})pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(single_timestamp: {})pb",
                .rhs_message_text = R"pb(single_timestamp: {
                                           seconds: 0
                                           nanos: 0
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(single_timestamp: {
                                           seconds: 0
                                           nanos: 0
                                         })pb",
                .rhs_message_text = R"pb(single_timestamp: {})pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(single_timestamp: {
                                           seconds: 0
                                           nanos: 0
                                         })pb",
                .rhs_message_text = R"pb(single_timestamp: {
                                           seconds: 0
                                           nanos: 0
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(single_timestamp: {
                                           seconds: 0
                                           nanos: 0
                                         })pb",
                .rhs_message_text = R"pb(single_timestamp: {
                                           seconds: 1
                                           nanos: 1
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(single_timestamp: {
                                           seconds: 1
                                           nanos: 1
                                         })pb",
                .rhs_message_text = R"pb(single_timestamp: {
                                           seconds: 0
                                           nanos: 0
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(single_timestamp: {
                                           seconds: 1
                                           nanos: 1
                                         })pb",
                .rhs_message_text = R"pb(single_timestamp: {
                                           seconds: 1
                                           nanos: 1
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
        }));

INSTANTIATE_TEST_SUITE_P(
    Value, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text =
                R"pb(single_value: { null_value: NULL_VALUE })pb",
            .rhs_message_text =
                R"pb(single_value: { null_value: NULL_VALUE })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text =
                R"pb(single_value: { null_value: NULL_VALUE })pb",
            .rhs_message_text = R"pb(single_value: { bool_value: true })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: { bool_value: true })pb",
            .rhs_message_text =
                R"pb(single_value: { null_value: NULL_VALUE })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: { bool_value: true })pb",
            .rhs_message_text = R"pb(single_value: { bool_value: true })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: { number_value: 1.0 })pb",
            .rhs_message_text = R"pb(single_value: { number_value: 1.0 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: { string_value: "foo" })pb",
            .rhs_message_text = R"pb(single_value: { string_value: "foo" })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: { list_value: {} })pb",
            .rhs_message_text = R"pb(single_value: { list_value: {} })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: {
                                       list_value: {
                                         values: { null_value: NULL_VALUE }
                                       }
                                     })pb",
            .rhs_message_text = R"pb(single_value: {
                                       list_value: {
                                         values: { null_value: NULL_VALUE }
                                       }
                                     })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: {
                                       list_value: {
                                         values: { null_value: NULL_VALUE }
                                       }
                                     })pb",
            .rhs_message_text = R"pb(single_value: {
                                       list_value: {
                                         values: { bool_value: true }
                                       }
                                     })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: {
                                       list_value: {
                                         values: { bool_value: true }
                                       }
                                     })pb",
            .rhs_message_text = R"pb(single_value: {
                                       list_value: {
                                         values: { null_value: NULL_VALUE }
                                       }
                                     })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: {
                                       list_value: {
                                         values: { bool_value: true }
                                       }
                                     })pb",
            .rhs_message_text = R"pb(single_value: {
                                       list_value: {
                                         values: { bool_value: true }
                                       }
                                     })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: { struct_value: {} })pb",
            .rhs_message_text = R"pb(single_value: { struct_value: {} })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: {
                                       struct_value: {
                                         fields: {
                                           key: "foo"
                                           value: { null_value: NULL_VALUE }
                                         }
                                       }
                                     })pb",
            .rhs_message_text = R"pb(single_value: {
                                       struct_value: {
                                         fields: {
                                           key: "foo"
                                           value: { null_value: NULL_VALUE }
                                         }
                                       }
                                     })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: {
                                       struct_value: {
                                         fields: {
                                           key: "foo"
                                           value: { null_value: NULL_VALUE }
                                         }
                                       }
                                     })pb",
            .rhs_message_text = R"pb(single_value: {
                                       struct_value: {
                                         fields: {
                                           key: "foo"
                                           value: { bool_value: true }
                                         }
                                       }
                                     })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: {
                                       struct_value: {
                                         fields: {
                                           key: "foo"
                                           value: { bool_value: true }
                                         }
                                       }
                                     })pb",
            .rhs_message_text = R"pb(single_value: {
                                       struct_value: {
                                         fields: {
                                           key: "foo"
                                           value: { null_value: NULL_VALUE }
                                         }
                                       }
                                     })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: {
                                       struct_value: {
                                         fields: {
                                           key: "foo"
                                           value: { null_value: NULL_VALUE }
                                         }
                                       }
                                     })pb",
            .rhs_message_text = R"pb(single_value: {
                                       struct_value: {
                                         fields: {
                                           key: "bar"
                                           value: { null_value: NULL_VALUE }
                                         }
                                       }
                                     })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_value: {
                                       struct_value: {
                                         fields: {
                                           key: "bar"
                                           value: { null_value: NULL_VALUE }
                                         }
                                       }
                                     })pb",
            .rhs_message_text = R"pb(single_value: {
                                       struct_value: {
                                         fields: {
                                           key: "foo"
                                           value: { null_value: NULL_VALUE }
                                         }
                                       }
                                     })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    ListValue, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(list_value: {})pb",
            .rhs_message_text = R"pb(list_value: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(list_value: {
                                       values: { null_value: NULL_VALUE }
                                     })pb",
            .rhs_message_text = R"pb(list_value: {
                                       values: { null_value: NULL_VALUE }
                                     })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(list_value: {
                                       values: { null_value: NULL_VALUE }
                                     })pb",
            .rhs_message_text = R"pb(list_value: {
                                       values: { bool_value: true }
                                     })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(list_value: {
                                       values: { bool_value: true }
                                     })pb",
            .rhs_message_text = R"pb(list_value: {
                                       values: { null_value: NULL_VALUE }
                                     })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(list_value: {
                                       values: { bool_value: true }
                                     })pb",
            .rhs_message_text = R"pb(list_value: {
                                       values: { bool_value: true }
                                     })pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Struct, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_struct: {})pb",
            .rhs_message_text = R"pb(single_struct: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_struct: {
                                       fields: {
                                         key: "foo"
                                         value: { null_value: NULL_VALUE }
                                       }
                                     })pb",
            .rhs_message_text = R"pb(single_struct: {
                                       fields: {
                                         key: "foo"
                                         value: { null_value: NULL_VALUE }
                                       }
                                     })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_struct: {
                                       fields: {
                                         key: "foo"
                                         value: { null_value: NULL_VALUE }
                                       }
                                     })pb",
            .rhs_message_text = R"pb(single_struct: {
                                       fields: {
                                         key: "foo"
                                         value: { bool_value: true }
                                       }
                                     })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_struct: {
                                       fields: {
                                         key: "foo"
                                         value: { bool_value: true }
                                       }
                                     })pb",
            .rhs_message_text = R"pb(single_struct: {
                                       fields: {
                                         key: "foo"
                                         value: { null_value: NULL_VALUE }
                                       }
                                     })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_struct: {
                                       fields: {
                                         key: "foo"
                                         value: { null_value: NULL_VALUE }
                                       }
                                     })pb",
            .rhs_message_text = R"pb(single_struct: {
                                       fields: {
                                         key: "bar"
                                         value: { null_value: NULL_VALUE }
                                       }
                                     })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_struct: {
                                       fields: {
                                         key: "bar"
                                         value: { null_value: NULL_VALUE }
                                       }
                                     })pb",
            .rhs_message_text = R"pb(single_struct: {
                                       fields: {
                                         key: "foo"
                                         value: { null_value: NULL_VALUE }
                                       }
                                     })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Message, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(standalone_message: {})pb",
            .rhs_message_text = R"pb(standalone_message: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(standalone_message: {})pb",
            .rhs_message_text = R"pb(standalone_message: { bb: 1 })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(standalone_message: { bb: 1 })pb",
            .rhs_message_text = R"pb(standalone_message: {})pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(standalone_message: { bb: 1 })pb",
            .rhs_message_text = R"pb(standalone_message: { bb: 1 })pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Enum, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(standalone_enum: FOO)pb",
            .rhs_message_text = R"pb(standalone_enum: FOO)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(standalone_enum: FOO)pb",
            .rhs_message_text = R"pb(standalone_enum: BAR)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(standalone_enum: BAR)pb",
            .rhs_message_text = R"pb(standalone_enum: FOO)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(standalone_enum: BAR)pb",
            .rhs_message_text = R"pb(standalone_enum: BAR)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Map, HomogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>(
        {
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_bool_string: {
                                           key: true
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_bool_string: {
                                           key: true
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_int32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_int32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_int64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_int64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_uint32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_uint32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_uint64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_uint64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_string_string: {
                                           key: "bar"
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_string_string: {
                                           key: "bar"
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
        }));

using HeterogeneousMessageEqualityTest = HomogeneousMessageEqualityTest;

TEST_P(HeterogeneousMessageEqualityTest, Equals) {
  const auto& param = GetParam();

  upb_Message* lhs_message = ParseProto(
      param.message_name, param.lhs_message_text, param.lhs_message_binary);
  upb_Message* rhs_message = ParseProto(
      param.message_name, param.rhs_message_text, param.rhs_message_binary);
  const upb_MessageDef* message_def = MessageDef(param.message_name);
  const upb_FieldDef* lhs_message_field_def;
  upb_MessageValue lhs_message_field_val;
  size_t lhs_message_iter = kUpb_Message_Begin;
  ASSERT_TRUE(upb_Message_Next(lhs_message, message_def, def_pool(),
                               &lhs_message_field_def, &lhs_message_field_val,
                               &lhs_message_iter));
  const upb_FieldDef* rhs_message_field_def;
  upb_MessageValue rhs_message_field_val;
  size_t rhs_message_iter = kUpb_Message_Begin;
  ASSERT_TRUE(upb_Message_Next(rhs_message, message_def, def_pool(),
                               &rhs_message_field_def, &rhs_message_field_val,
                               &rhs_message_iter));
  EXPECT_EQ(
      _cel_MessageField_Equals(lhs_message_field_val, lhs_message_field_def,
                               rhs_message_field_val, rhs_message_field_def,
                               def_pool(), wkts(), alloc()),
      param.result);
  EXPECT_EQ(
      _cel_MessageField_Equals(rhs_message_field_val, rhs_message_field_def,
                               lhs_message_field_val, lhs_message_field_def,
                               def_pool(), wkts(), alloc()),
      param.result);
  ASSERT_FALSE(upb_Message_Next(lhs_message, message_def, def_pool(),
                                &lhs_message_field_def, &lhs_message_field_val,
                                &lhs_message_iter));
  ASSERT_FALSE(upb_Message_Next(rhs_message, message_def, def_pool(),
                                &rhs_message_field_def, &rhs_message_field_val,
                                &rhs_message_iter));
}

INSTANTIATE_TEST_SUITE_P(
    Number, HeterogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int32: 1)pb",
            .rhs_message_text = R"pb(single_uint32: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_int32: 1)pb",
            .rhs_message_text = R"pb(single_float: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_uint32: 1)pb",
            .rhs_message_text = R"pb(single_int32: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Any, HeterogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text =
                R"pb(single_any: {
                       type_url: "type.googleapis.com/google.protobuf.Int32Value"
                       value: ""
                     })pb",
            .rhs_message_text = R"pb(single_int32_wrapper: {})pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text =
                R"pb(single_any: {
                       type_url: "type.googleapis.com/message.does.not.Exist"
                       value: "foo"
                     })pb",
            .rhs_message_text =
                R"pb(single_any: {
                       type_url: "type.googleapis.com/message.does.not.Exist"
                       value: "foo"
                     })pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text =
                R"pb(single_any: {
                       type_url: "type.googleapis.com/message.does.not.Exist"
                       value: "foo"
                     })pb",
            .rhs_message_text =
                R"pb(single_any: {
                       type_url: "type.googleapis.com/message.does.not.Exist"
                       value: "bar"
                     })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text =
                R"pb(single_any: {
                       type_url: "type.googleapis.com/message.does.not.Exist"
                       value: "bar"
                     })pb",
            .rhs_message_text =
                R"pb(single_any: {
                       type_url: "type.googleapis.com/message.does.not.Exist"
                       value: "foo"
                     })pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Enum, HeterogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(single_nested_enum: FOO)pb",
            .rhs_message_text = R"pb(optional_null_value: NULL_VALUE)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(optional_null_value: NULL_VALUE)pb",
            .rhs_message_text = R"pb(single_nested_enum: FOO)pb",
            .result = _cel_MessageEquality_kNotEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Repeated, HeterogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>({
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(repeated_int32: 1)pb",
            .rhs_message_text = R"pb(repeated_int64: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(repeated_int32: 1)pb",
            .rhs_message_text = R"pb(repeated_uint32: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(repeated_int32: 1)pb",
            .rhs_message_text = R"pb(repeated_uint64: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(repeated_int32: 1)pb",
            .rhs_message_text = R"pb(repeated_float: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(repeated_int32: 1)pb",
            .rhs_message_text = R"pb(repeated_double: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(repeated_int64: 1)pb",
            .rhs_message_text = R"pb(repeated_int32: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(repeated_int64: 1)pb",
            .rhs_message_text = R"pb(repeated_uint32: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(repeated_int64: 1)pb",
            .rhs_message_text = R"pb(repeated_uint64: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(repeated_int64: 1)pb",
            .rhs_message_text = R"pb(repeated_float: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
        {
            .message_name = "cel.expr.conformance.proto3.TestAllTypes",
            .lhs_message_text = R"pb(repeated_int64: 1)pb",
            .rhs_message_text = R"pb(repeated_double: 1)pb",
            .result = _cel_MessageEquality_kEqual,
        },
    }));

INSTANTIATE_TEST_SUITE_P(
    Map, HeterogeneousMessageEqualityTest,
    ::testing::ValuesIn<MessageEqualityTestParam>(
        {
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_int32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_int64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_int32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_uint32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_int32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_uint64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_int64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_uint32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_int64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_uint64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_uint32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_uint64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_bool_string: {
                                           key: true
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_int32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_bool_string: {
                                           key: true
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_int64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_bool_string: {
                                           key: true
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_uint32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_bool_string: {
                                           key: true
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_uint64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_bool_string: {
                                           key: true
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_string_string: {
                                           key: "bar"
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_int32_string: {
                                           key: -1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_uint32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_int64_string: {
                                           key: -1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_uint32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_int32_string: {
                                           key: -1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_uint64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_int32_string: {
                                           key: -1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_int64_string: {
                                           key: 9223372036854775807
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_int64_string: {
                                           key: -1
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_uint64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_uint64_string: {
                                           key: 18446744073709551615
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_int32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_uint64_string: {
                                           key: 18446744073709551615
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_uint32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_uint64_string: {
                                           key: 18446744073709551615
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_int64_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
            {
                .message_name = "cel.expr.conformance.proto3.TestAllTypes",
                .lhs_message_text = R"pb(map_uint32_string: {
                                           key: 4294967295
                                           value: "foo"
                                         })pb",
                .rhs_message_text = R"pb(map_int32_string: {
                                           key: 1
                                           value: "foo"
                                         })pb",
                .result = _cel_MessageEquality_kNotEqual,
            },
        }));

class MessageEqualityTest_NonCanonical : public ::testing::Test {
 protected:
  void SetUp() override {
    arena_ = cel_Arena_New(cel_DefaultAllocator);
    def_pool_ = upb_DefPool_New();
    cel_Status_Construct(&status_);
    msg_def_ = cel_expr_conformance_proto2_TestAllTypes_getmsgdef(def_pool_);
    ASSERT_NE(msg_def_, nullptr);
    ASSERT_TRUE(cel_WellKnownTypes_Initialize(&wkts_, def_pool_, &status_));
    mt_ = upb_MessageDef_MiniTable(msg_def_);
  }

  void TearDown() override {
    cel_Status_Destruct(&status_);
    upb_DefPool_Free(def_pool_);
    cel_Arena_Delete(arena_);
  }

  upb_Message* NewMessage() { return upb_Message_New(mt_, arena_); }

  _cel_MessageEquality CheckEquals(const upb_Message* lhs,
                                   const upb_Message* rhs) {
    return _cel_Message_Equals(lhs, rhs, msg_def_, def_pool_, &wkts_,
                               cel_DefaultAllocator);
  }

  cel_Arena* arena_;
  upb_DefPool* def_pool_;
  cel_Status status_;
  cel_WellKnownTypes wkts_;
  const upb_MessageDef* msg_def_;
  const upb_MiniTable* mt_;
};

TEST_F(MessageEqualityTest_NonCanonical, EqualSameExtensionAndValue) {
  upb_Message* msg1 = NewMessage();
  int32_t val1 = 42;
  ASSERT_TRUE(upb_Message_SetNonCanonicalExtension(
      msg1, cel_expr_conformance_proto2_int32_ext_ext, &val1, arena_));

  upb_Message* msg2 = NewMessage();
  int32_t val2 = 42;
  ASSERT_TRUE(upb_Message_SetNonCanonicalExtension(
      msg2, cel_expr_conformance_proto2_int32_ext_ext, &val2, arena_));

  EXPECT_EQ(CheckEquals(msg1, msg2), _cel_MessageEquality_kEqual);
}

TEST_F(MessageEqualityTest_NonCanonical,
       EqualUnknownStringViewAndNonCanonicalExtension) {
  upb_Message* msg1 = NewMessage();
  std::string bytes = UnknownFields{UnknownField::Varint(1000, 42)};
  ASSERT_EQ(
      upb_Decode(bytes.data(), bytes.size(), msg1, mt_, nullptr, 0, arena_),
      kUpb_DecodeStatus_Ok);

  upb_Message* msg2 = NewMessage();
  int32_t val2 = 42;
  ASSERT_TRUE(upb_Message_SetNonCanonicalExtension(
      msg2, cel_expr_conformance_proto2_int32_ext_ext, &val2, arena_));

  EXPECT_EQ(CheckEquals(msg1, msg2), _cel_MessageEquality_kEqual);
}

TEST_F(MessageEqualityTest_NonCanonical, NotEqualDifferentValue) {
  upb_Message* msg1 = NewMessage();
  int32_t val1 = 42;
  ASSERT_TRUE(upb_Message_SetNonCanonicalExtension(
      msg1, cel_expr_conformance_proto2_int32_ext_ext, &val1, arena_));

  upb_Message* msg2 = NewMessage();
  int32_t val2 = 43;
  ASSERT_TRUE(upb_Message_SetNonCanonicalExtension(
      msg2, cel_expr_conformance_proto2_int32_ext_ext, &val2, arena_));

  EXPECT_EQ(CheckEquals(msg1, msg2), _cel_MessageEquality_kNotEqual);
}

TEST_F(MessageEqualityTest_NonCanonical, NotEqualDifferentExtension) {
  upb_Message* msg1 = NewMessage();
  int32_t val1 = 42;
  ASSERT_TRUE(upb_Message_SetNonCanonicalExtension(
      msg1, cel_expr_conformance_proto2_int32_ext_ext, &val1, arena_));

  upb_Message* msg2 = NewMessage();
  const upb_Message* nested_msg = NewMessage();
  ASSERT_TRUE(upb_Message_SetNonCanonicalExtension(
      msg2, cel_expr_conformance_proto2_nested_ext_ext, &nested_msg, arena_));

  EXPECT_EQ(CheckEquals(msg1, msg2), _cel_MessageEquality_kNotEqual);
}

TEST_F(MessageEqualityTest_NonCanonical, EncodeFailureMaxDepth) {
  upb_Message* msg1 = NewMessage();
  upb_Message* current = msg1;
  for (int i = 0; i < 105; ++i) {
    upb_Message* next = NewMessage();
    ASSERT_TRUE(upb_Message_SetNonCanonicalExtension(
        current, cel_expr_conformance_proto2_nested_ext_ext, &next, arena_));
    current = next;
  }

  upb_Message* msg2 = NewMessage();
  int32_t val2 = 42;
  ASSERT_TRUE(upb_Message_SetNonCanonicalExtension(
      msg2, cel_expr_conformance_proto2_int32_ext_ext, &val2, arena_));

  EXPECT_EQ(CheckEquals(msg1, msg2), _cel_MessageEquality_kMaxDepthExceeded);
}

}  // namespace
