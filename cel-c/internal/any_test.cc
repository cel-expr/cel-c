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

#include "cel-c/internal/any.h"

#include <string>

#include "google/protobuf/any.pb.h"
#include "google/protobuf/any.upb.h"
#include "google/protobuf/any.upbdefs.h"
#include "google/protobuf/wrappers.pb.h"
#include "google/protobuf/wrappers.upbdefs.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/absl_check.h"
#include "absl/log/die_if_null.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/config.h"
#include "cel-c/status.h"
#include "cel-c/status_code.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"
#include "cel-c/well_known_types.h"
#include "upb/base/upcast.h"
#include "upb/message/message.h"
#include "upb/reflection/def.h"
#include "upb/reflection/message.h"

namespace {

using ::testing::IsNull;
using ::testing::NotNull;

class AnyTest : public ::testing::Test {
 public:
  void SetUp() override {
    arena_ = ABSL_DIE_IF_NULL(cel_Arena_New(alloc()));
    def_pool_ = ABSL_DIE_IF_NULL(upb_DefPool_New());
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_StringValue_getmsgdef(def_pool_)));
    static_cast<void>(
        ABSL_DIE_IF_NULL(google_protobuf_Any_getmsgdef(def_pool_)));
    cel_Status_Construct(&status_);
    ABSL_CHECK(
        cel_AnyWellKnownType_Initialize(&any_wkt_, def_pool(), &status_));
    ABSL_CHECK(cel_StringValueWellKnownType_Initialize(&string_value_wkt_,
                                                       def_pool(), &status_));
  }

  void TearDown() override {
    cel_Status_Destruct(&status_);
    upb_DefPool_Free(def_pool_);
    def_pool_ = nullptr;
    cel_Arena_Delete(arena_);
    arena_ = nullptr;
  }

 protected:
  CEL_NONNULL(cel_Allocator*) alloc() { return cel_DefaultAllocator; }

  CEL_NONNULL(cel_Arena*) arena() { return ABSL_DIE_IF_NULL(arena_); }

  CEL_NONNULL(upb_DefPool*) def_pool() { return ABSL_DIE_IF_NULL(def_pool_); }

  CEL_NONNULL(cel_Status*) status() { return &status_; }

  CEL_NONNULL(const cel_AnyWellKnownType*) any_wkt() { return &any_wkt_; }

  CEL_NONNULL(const cel_StringValueWellKnownType*) string_value_wkt() {
    return &string_value_wkt_;
  }

 private:
  CEL_NULLABLE(cel_Arena*) arena_ = nullptr;
  CEL_NULLABLE(upb_DefPool*) def_pool_ = nullptr;
  cel_Status status_;
  cel_AnyWellKnownType any_wkt_;
  cel_StringValueWellKnownType string_value_wkt_;
};

TEST_F(AnyTest, Unpack) {
  std::string type_url;
  std::string value;
  {
    google::protobuf::Any packed;
    google::protobuf::StringValue unpacked;
    unpacked.set_value("foo");
    ASSERT_TRUE(packed.PackFrom(unpacked));
    type_url = std::string(packed.type_url());
    value = static_cast<std::string>(packed.value());
  }
  google_protobuf_Any* in_message =
      ABSL_DIE_IF_NULL(google_protobuf_Any_new(arena()));
  google_protobuf_Any_set_type_url(in_message,
                                   cel_StringView_FromAbsl(type_url));
  google_protobuf_Any_set_value(in_message, cel_StringView_FromAbsl(value));
  upb_Message* out_message;
  const upb_MessageDef* out_message_def;
  ASSERT_EQ(_cel_AnyUnpack(UPB_UPCAST(in_message), def_pool(), any_wkt(),
                           arena(), &out_message, &out_message_def),
            _cel_AnyUnpackResult_kOk);
  ASSERT_THAT(out_message, NotNull());
  ASSERT_EQ(out_message_def, string_value_wkt()->def);
  EXPECT_TRUE(cel_StringView_Equals(
      upb_Message_GetFieldByDef(out_message, string_value_wkt()->value_def)
          .str_val,
      cel_StringView_From("foo")));
}

TEST_F(AnyTest, UnpackRecursive) {
  std::string type_url;
  std::string value;
  {
    google::protobuf::Any packed1;
    google::protobuf::Any packed2;
    google::protobuf::StringValue unpacked;
    unpacked.set_value("foo");
    ASSERT_TRUE(packed2.PackFrom(unpacked));
    ASSERT_TRUE(packed1.PackFrom(packed2));
    type_url = std::string(packed1.type_url());
    value = static_cast<std::string>(packed1.value());
  }
  google_protobuf_Any* in_message =
      ABSL_DIE_IF_NULL(google_protobuf_Any_new(arena()));
  google_protobuf_Any_set_type_url(in_message,
                                   cel_StringView_FromAbsl(type_url));
  google_protobuf_Any_set_value(in_message, cel_StringView_FromAbsl(value));
  upb_Message* out_message;
  const upb_MessageDef* out_message_def;
  ASSERT_EQ(_cel_AnyUnpack(UPB_UPCAST(in_message), def_pool(), any_wkt(),
                           arena(), &out_message, &out_message_def),
            _cel_AnyUnpackResult_kOk);
  ASSERT_THAT(out_message, NotNull());
  ASSERT_EQ(out_message_def, string_value_wkt()->def);
  EXPECT_TRUE(cel_StringView_Equals(
      upb_Message_GetFieldByDef(out_message, string_value_wkt()->value_def)
          .str_val,
      cel_StringView_From("foo")));
}

TEST_F(AnyTest, UnpackBadTypeUrl) {
  std::string type_url = "google.protobuf.BoolValue";
  std::string value;
  google_protobuf_Any* in_message =
      ABSL_DIE_IF_NULL(google_protobuf_Any_new(arena()));
  google_protobuf_Any_set_type_url(in_message,
                                   cel_StringView_FromAbsl(type_url));
  google_protobuf_Any_set_value(in_message, cel_StringView_FromAbsl(value));
  upb_Message* out_message;
  const upb_MessageDef* out_message_def;
  ASSERT_EQ(_cel_AnyUnpack(UPB_UPCAST(in_message), def_pool(), any_wkt(),
                           arena(), &out_message, &out_message_def),
            _cel_AnyUnpackResult_kBadTypeUrl);
  EXPECT_THAT(out_message, IsNull());
  EXPECT_THAT(out_message_def, IsNull());
}

TEST_F(AnyTest, UnpackDefNotFound) {
  std::string type_url = "type.googleapis.com/message.that.does.not.Exist";
  std::string value;
  google_protobuf_Any* in_message =
      ABSL_DIE_IF_NULL(google_protobuf_Any_new(arena()));
  google_protobuf_Any_set_type_url(in_message,
                                   cel_StringView_FromAbsl(type_url));
  google_protobuf_Any_set_value(in_message, cel_StringView_FromAbsl(value));
  upb_Message* out_message;
  const upb_MessageDef* out_message_def;
  ASSERT_EQ(_cel_AnyUnpack(UPB_UPCAST(in_message), def_pool(), any_wkt(),
                           arena(), &out_message, &out_message_def),
            _cel_AnyUnpackResult_kDefNotFound);
  EXPECT_THAT(out_message, IsNull());
  EXPECT_THAT(out_message_def, IsNull());
}

TEST_F(AnyTest, ToStatus) {
  _cel_AnyUnpackResult_ToStatus(_cel_AnyUnpackResult_kOk, status());
  EXPECT_EQ(cel_Status_CanonicalCode(status()), cel_StatusCode_kOk);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("")));

  _cel_AnyUnpackResult_ToStatus(_cel_AnyUnpackResult_kOutOfMemory, status());
  EXPECT_TRUE(cel_Status_IsOutOfMemory(status()));

  _cel_AnyUnpackResult_ToStatus(_cel_AnyUnpackResult_kBadTypeUrl, status());
  EXPECT_EQ(cel_Status_CanonicalCode(status()),
            cel_StatusCode_kInvalidArgument);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("bad type URL")));

  _cel_AnyUnpackResult_ToStatus(_cel_AnyUnpackResult_kDefNotFound, status());
  EXPECT_EQ(cel_Status_CanonicalCode(status()), cel_StatusCode_kNotFound);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_Status_Message(status()),
                            cel_StringView_From("descriptor not found")));

  _cel_AnyUnpackResult_ToStatus(_cel_AnyUnpackResult_kMalformed, status());
  EXPECT_EQ(cel_Status_CanonicalCode(status()),
            cel_StatusCode_kInvalidArgument);
  EXPECT_TRUE(
      cel_StringView_Equals(cel_Status_Message(status()),
                            cel_StringView_From("malformed wire format")));

  _cel_AnyUnpackResult_ToStatus(_cel_AnyUnpackResult_kBadUtf8, status());
  EXPECT_EQ(cel_Status_CanonicalCode(status()),
            cel_StatusCode_kInvalidArgument);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("bad UTF-8")));

  _cel_AnyUnpackResult_ToStatus(_cel_AnyUnpackResult_kMaxDepthExceeded,
                                status());
  EXPECT_EQ(cel_Status_CanonicalCode(status()),
            cel_StatusCode_kInvalidArgument);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("max depth exceeded")));

  _cel_AnyUnpackResult_ToStatus(_cel_AnyUnpackResult_kUnknown, status());
  EXPECT_EQ(cel_Status_CanonicalCode(status()), cel_StatusCode_kUnknown);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("unknown error")));
}

}  // namespace
