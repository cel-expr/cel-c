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

#include "cel-c/status_absl.h"

#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/strings/cord.h"
#include "absl/strings/string_view.h"
#include "cel-c/config.h"
#include "cel-c/error_space.h"
#include "cel-c/status.h"
#include "cel-c/status_code.h"
#include "cel-c/string_view.h"

namespace {

using ::absl_testing::IsOk;
using ::absl_testing::StatusIs;
using ::testing::Optional;
using ::testing::Test;

class StatusAbslTest : public Test {
 public:
  void SetUp() override { cel_Status_Construct(&status_); }

  void TearDown() override { cel_Status_Destruct(&status_); }

 protected:
  CEL_NONNULL(cel_Status*) status() { return &status_; }

 private:
  cel_Status status_;
};

TEST_F(StatusAbslTest, ToAbsl) {
  absl::Status out = cel_Status_ToAbsl(status());
  EXPECT_THAT(out, IsOk());
  size_t payload_count = 0;
  out.ForEachPayload([&payload_count](absl::string_view, const absl::Cord&) {
    ++payload_count;
  });
  EXPECT_EQ(payload_count, 0);

  ASSERT_TRUE(
      cel_InvalidArgumentStatus(status(), cel_StringView_From("Hello World!")));
  ASSERT_TRUE(cel_Status_SetPayload(status(), cel_StringView_From("type_url"),
                                    cel_StringView_From("value")));
  out = cel_Status_ToAbsl(status());
  EXPECT_THAT(out,
              StatusIs(absl::StatusCode::kInvalidArgument, "Hello World!"));
  payload_count = 0;
  out.ForEachPayload([&payload_count](absl::string_view, const absl::Cord&) {
    ++payload_count;
  });
  EXPECT_EQ(payload_count, 1);
  EXPECT_THAT(out.GetPayload(absl::string_view("type_url")),
              Optional(absl::string_view("value")));
}

TEST_F(StatusAbslTest, FromAbsl) {
  absl::Status in = absl::OkStatus();
  ASSERT_TRUE(cel_Status_FromAbsl(status(), in));
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);

  in = absl::InvalidArgumentError("Hello World!");
  in.SetPayload("type_url", absl::Cord("value"));
  ASSERT_TRUE(cel_Status_FromAbsl(status(), in));
  EXPECT_FALSE(cel_Status_Ok(status()));
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
