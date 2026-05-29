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

#include "cel-c/constant.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"

namespace {

using ::testing::_;

TEST(Constant, Unspecified) {
  cel_Constant constant = cel_UnspecifiedConstant();
  EXPECT_EQ(cel_Constant_Kind(&constant), cel_ConstantKind_kUnspecified);

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBool(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetInt(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetUint(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDouble(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBytes(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetString(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDuration(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetTimestamp(&constant)), _);
#endif
}

TEST(Constant, Null) {
  cel_Constant constant;
  cel_Constant_SetNull(&constant);
  EXPECT_EQ(cel_Constant_Kind(&constant), cel_ConstantKind_kNull);

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBool(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetInt(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetUint(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDouble(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBytes(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetString(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDuration(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetTimestamp(&constant)), _);
#endif
}

TEST(Constant, Bool) {
  cel_Constant constant;
  cel_Constant_SetBool(&constant, true);
  EXPECT_EQ(cel_Constant_Kind(&constant), cel_ConstantKind_kBool);
  EXPECT_TRUE(cel_Constant_GetBool(&constant));

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetInt(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetUint(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDouble(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBytes(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetString(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDuration(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetTimestamp(&constant)), _);
#endif
}

TEST(Constant, Int) {
  cel_Constant constant;
  cel_Constant_SetInt(&constant, 1);
  EXPECT_EQ(cel_Constant_Kind(&constant), cel_ConstantKind_kInt);
  EXPECT_EQ(cel_Constant_GetInt(&constant), 1);

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBool(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetUint(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDouble(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBytes(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetString(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDuration(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetTimestamp(&constant)), _);
#endif
}

TEST(Constant, Uint) {
  cel_Constant constant;
  cel_Constant_SetUint(&constant, 1);
  EXPECT_EQ(cel_Constant_Kind(&constant), cel_ConstantKind_kUint);
  EXPECT_EQ(cel_Constant_GetUint(&constant), 1);

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBool(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetInt(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDouble(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBytes(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetString(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDuration(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetTimestamp(&constant)), _);
#endif
}

TEST(Constant, Double) {
  cel_Constant constant;
  cel_Constant_SetDouble(&constant, 1);
  EXPECT_EQ(cel_Constant_Kind(&constant), cel_ConstantKind_kDouble);
  EXPECT_EQ(cel_Constant_GetDouble(&constant), 1);

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBool(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetInt(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetUint(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBytes(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetString(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDuration(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetTimestamp(&constant)), _);
#endif
}

TEST(Constant, Bytes) {
  cel_Constant constant;
  cel_Constant_SetBytes(&constant, cel_StringView_FromString("foo"));
  EXPECT_EQ(cel_Constant_Kind(&constant), cel_ConstantKind_kBytes);
  EXPECT_TRUE(cel_StringView_Equals(cel_Constant_GetBytes(&constant),
                                    cel_StringView_FromString("foo")));

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBool(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetInt(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetUint(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDouble(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetString(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDuration(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetTimestamp(&constant)), _);
#endif
}

TEST(Constant, String) {
  cel_Constant constant;
  cel_Constant_SetString(&constant, cel_StringView_FromString("foo"));
  EXPECT_EQ(cel_Constant_Kind(&constant), cel_ConstantKind_kString);
  EXPECT_TRUE(cel_StringView_Equals(cel_Constant_GetString(&constant),
                                    cel_StringView_FromString("foo")));

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBool(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetInt(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetUint(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDouble(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBytes(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDuration(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetTimestamp(&constant)), _);
#endif
}

TEST(Constant, Duration) {
  cel_Constant constant;
  cel_Constant_SetDuration(&constant, cel_Duration_FromUnix(1, 2));
  EXPECT_EQ(cel_Constant_Kind(&constant), cel_ConstantKind_kDuration);
  EXPECT_TRUE(cel_Duration_Equals(cel_Constant_GetDuration(&constant),
                                  cel_Duration_FromUnix(1, 2)));

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBool(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetInt(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetUint(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDouble(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBytes(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetString(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetTimestamp(&constant)), _);
#endif
}

TEST(Constant, Timestamp) {
  cel_Constant constant;
  cel_Constant_SetTimestamp(&constant, cel_Timestamp_FromUnix(1, 2));
  EXPECT_EQ(cel_Constant_Kind(&constant), cel_ConstantKind_kTimestamp);
  EXPECT_TRUE(cel_Timestamp_Equals(cel_Constant_GetTimestamp(&constant),
                                   cel_Timestamp_FromUnix(1, 2)));

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBool(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetInt(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetUint(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDouble(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetBytes(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetString(&constant)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(cel_Constant_GetDuration(&constant)), _);
#endif
}

}  // namespace
