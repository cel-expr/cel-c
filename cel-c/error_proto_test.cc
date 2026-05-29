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

#include "cel-c/error_proto.h"

#include <cstddef>

#include "google/protobuf/any.upb.h"
#include "google/rpc/code.upb.h"
#include "google/rpc/status.upb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "cel-c/config.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/string_view.h"
#include "upb/mem/arena.h"

namespace {

using ::testing::NotNull;
using ::testing::Test;

class ErrorProtoTest : public Test {
 public:
  void SetUp() override { arena_ = ABSL_DIE_IF_NULL(upb_Arena_New()); }

  void TearDown() override {
    upb_Arena_Free(arena_);
    arena_ = nullptr;
  }

 protected:
  CEL_NONNULL(upb_Arena*) arena() { return ABSL_DIE_IF_NULL(arena_); }

 private:
  CEL_NULLABILITY_UNKNOWN(upb_Arena*) arena_ = nullptr;
};

TEST_F(ErrorProtoTest, ToProto) {
  cel_Error* in = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  CEL_NONNULL(google_rpc_Status*)
  out = ABSL_DIE_IF_NULL(google_rpc_Status_new(arena()));
  ASSERT_TRUE(cel_Error_ToProto(in, arena(), out));
  EXPECT_EQ(google_rpc_Status_code(out), google_rpc_UNKNOWN);
  EXPECT_TRUE(cel_StringView_Empty(google_rpc_Status_message(out)));
  size_t details_size = 0;
  google_rpc_Status_details(out, &details_size);
  EXPECT_EQ(details_size, 0);

  cel_Error_SetCanonicalCode(in, cel_ErrorCode_kInvalidArgument);
  cel_Error_SetMessage(in, cel_StringView_From("Hello World!"));
  ASSERT_TRUE(cel_Error_SetPayload(in, cel_StringView_From("type_url"),
                                   cel_StringView_From("value"), arena()));
  ASSERT_TRUE(cel_Error_ToProto(in, arena(), out));
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

}  // namespace
