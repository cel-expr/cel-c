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

#include <cstddef>

#include "google/protobuf/any.upb.h"
#include "google/rpc/code.upb.h"
#include "google/rpc/status.upb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "cel-c/config.h"
#include "cel-c/error_space.h"
#include "cel-c/status.h"
#include "cel-c/status_code.h"
#include "cel-c/string_view.h"
#include "upb/mem/arena.h"

namespace {

using ::testing::NotNull;
using ::testing::Test;

class StatusProtoTest : public Test {
 public:
  void SetUp() override {
    cel_Status_Construct(&status_);
    arena_ = ABSL_DIE_IF_NULL(upb_Arena_New());
  }

  void TearDown() override {
    cel_Status_Destruct(&status_);
    upb_Arena_Free(arena_);
    arena_ = nullptr;
  }

 protected:
  CEL_NONNULL(cel_Status*) status() { return &status_; }

  CEL_NONNULL(upb_Arena*) arena() { return ABSL_DIE_IF_NULL(arena_); }

 private:
  cel_Status status_;
  CEL_NULLABILITY_UNKNOWN(upb_Arena*) arena_ = nullptr;
};

TEST_F(StatusProtoTest, ToProto) {
  CEL_NONNULL(google_rpc_Status*)
  out = ABSL_DIE_IF_NULL(google_rpc_Status_new(arena()));
  ASSERT_TRUE(cel_Status_ToProto(status(), arena(), out));
  EXPECT_EQ(google_rpc_Status_code(out), google_rpc_OK);
  EXPECT_TRUE(cel_StringView_Empty(google_rpc_Status_message(out)));
  size_t details_size = 0;
  google_rpc_Status_details(out, &details_size);
  EXPECT_EQ(details_size, 0);

  ASSERT_TRUE(
      cel_InvalidArgumentStatus(status(), cel_StringView_From("Hello World!")));
  ASSERT_TRUE(cel_Status_SetPayload(status(), cel_StringView_From("type_url"),
                                    cel_StringView_From("value")));
  ASSERT_TRUE(cel_Status_ToProto(status(), arena(), out));
  EXPECT_EQ(google_rpc_Status_code(out), google_rpc_INVALID_ARGUMENT);
  EXPECT_TRUE(cel_StringView_Equals(google_rpc_Status_message(out),
                                    cel_StringView_From("Hello World!")));
  CEL_NULLABILITY_UNKNOWN(CEL_NULLABILITY_UNKNOWN(const google_protobuf_Any*)
                              const*)
  details = google_rpc_Status_details(out, &details_size);
  ASSERT_EQ(details_size, 1);
  ASSERT_THAT(details, NotNull());
  ASSERT_THAT(details[0], NotNull());
  EXPECT_TRUE(cel_StringView_Equals(google_protobuf_Any_type_url(details[0]),
                                    cel_StringView_From("type_url")));
  EXPECT_TRUE(cel_StringView_Equals(google_protobuf_Any_value(details[0]),
                                    cel_StringView_From("value")));
}

TEST_F(StatusProtoTest, FromProto) {
  CEL_NONNULL(google_rpc_Status*)
  in = ABSL_DIE_IF_NULL(google_rpc_Status_new(arena()));
  ASSERT_TRUE(cel_Status_FromProto(status(), in));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);

  google_rpc_Status_set_code(in, google_rpc_INVALID_ARGUMENT);
  google_rpc_Status_set_message(in, cel_StringView_From("Hello World!"));
  CEL_NONNULL(google_protobuf_Any*)
  detail = ABSL_DIE_IF_NULL(google_rpc_Status_add_details(in, arena()));
  google_protobuf_Any_set_type_url(detail, cel_StringView_From("type_url"));
  google_protobuf_Any_set_value(detail, cel_StringView_From("value"));
  ASSERT_TRUE(cel_Status_FromProto(status(), in));
  EXPECT_TRUE(cel_Status_IsInvalidArgument(status()));
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_CanonicalCode(status()),
            cel_StatusCode_kInvalidArgument);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kInvalidArgument);
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Payloads(status()), 1);
  cel_StringView value;
  ASSERT_TRUE(
      cel_Status_GetPayload(status(), cel_StringView_From("type_url"), &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_From("value")));
}

}  // namespace
