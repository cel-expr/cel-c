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

#include "cel-c/error_space.h"

#include <errno.h>

#include "cel-c/status_code.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>
#include <winsock2.h>
#endif

#include "gtest/gtest.h"
#include "cel-c/cstring_view.h"

namespace {

TEST(ErrorCategory, Equality) {
  EXPECT_NE(cel_CanonicalErrorSpace, cel_GenericErrorSpace);
  EXPECT_NE(cel_CanonicalErrorSpace, cel_SystemErrorSpace);
  EXPECT_NE(cel_GenericErrorSpace, cel_SystemErrorSpace);
}

TEST(ErrorCategory, Names) {
  EXPECT_TRUE(
      cel_CStringView_Equals(cel_ErrorSpace_Name(cel_CanonicalErrorSpace),
                             cel_CStringView_FromString("canonical")));
  EXPECT_TRUE(cel_CStringView_Equals(cel_ErrorSpace_Name(cel_GenericErrorSpace),
                                     cel_CStringView_FromString("generic")));
  EXPECT_TRUE(cel_CStringView_Equals(cel_ErrorSpace_Name(cel_SystemErrorSpace),
                                     cel_CStringView_FromString("system")));
}

TEST(GenericErrorCategory, CurrentCode) {
  int prev_errno = errno;

  errno = 0;
  EXPECT_EQ(cel_GenericErrorSpace_CurrentCode(), 0);
  errno = EINVAL;
  EXPECT_EQ(cel_GenericErrorSpace_CurrentCode(), EINVAL);

  errno = prev_errno;
}

TEST(SystemErrorCategory, CurrentCode) {
#ifndef _WIN32
  int prev_errno = errno;

  errno = 0;
  EXPECT_EQ(cel_SystemErrorSpace_CurrentCode(), 0);
  errno = EINVAL;
  EXPECT_EQ(cel_SystemErrorSpace_CurrentCode(), EINVAL);

  errno = prev_errno;
#else
  DWORD last_error = GetLastError();

  SetLastError(0);
  EXPECT_EQ(cel_SystemErrorSpace_CurrentCode(), 0);
  SetLastError(ERROR_INVALID_PARAMETER);
  EXPECT_EQ(cel_SystemErrorSpace_CurrentCode(), ERROR_INVALID_PARAMETER);

  SetLastError(last_error);
#endif
}

TEST(CanonicalErrorSpace, OutOfMemory) {
  EXPECT_FALSE(
      cel_ErrorSpace_OutOfMemory(cel_CanonicalErrorSpace, cel_StatusCode_kOk));
  EXPECT_FALSE(cel_ErrorSpace_OutOfMemory(cel_CanonicalErrorSpace,
                                          cel_StatusCode_kResourceExhausted));
}

TEST(GenericErrorSpace, OutOfMemory) {
  EXPECT_FALSE(cel_ErrorSpace_OutOfMemory(cel_GenericErrorSpace, 0));
  EXPECT_TRUE(cel_ErrorSpace_OutOfMemory(cel_GenericErrorSpace, ENOMEM));
  EXPECT_FALSE(cel_ErrorSpace_OutOfMemory(cel_GenericErrorSpace, EINVAL));
}

TEST(SystemErrorSpace, OutOfMemory) {
#ifndef _WIN32
  EXPECT_FALSE(cel_ErrorSpace_OutOfMemory(cel_SystemErrorSpace, 0));
  EXPECT_TRUE(cel_ErrorSpace_OutOfMemory(cel_SystemErrorSpace, ENOMEM));
  EXPECT_FALSE(cel_ErrorSpace_OutOfMemory(cel_SystemErrorSpace, EINVAL));
#else
  EXPECT_FALSE(cel_ErrorSpace_OutOfMemory(cel_SystemErrorSpace, 0));
  EXPECT_TRUE(cel_ErrorSpace_OutOfMemory(cel_SystemErrorSpace,
                                         ERROR_NOT_ENOUGH_MEMORY));
  EXPECT_TRUE(
      cel_ErrorSpace_OutOfMemory(cel_SystemErrorSpace, ERROR_OUTOFMEMORY));
  EXPECT_FALSE(cel_ErrorSpace_OutOfMemory(cel_SystemErrorSpace,
                                          ERROR_INVALID_PARAMETER));
#endif
}

TEST(CanonicalErrorSpace, Canonical) {
  EXPECT_EQ(
      cel_ErrorSpace_Canonical(cel_CanonicalErrorSpace, cel_StatusCode_kOk),
      cel_StatusCode_kOk);
  EXPECT_EQ(cel_ErrorSpace_Canonical(cel_CanonicalErrorSpace,
                                     cel_StatusCode_kInvalidArgument),
            cel_StatusCode_kInvalidArgument);
}

TEST(GenericErrorSpace, Canonical) {
  EXPECT_EQ(cel_ErrorSpace_Canonical(cel_GenericErrorSpace, 0),
            cel_StatusCode_kOk);
  EXPECT_EQ(cel_ErrorSpace_Canonical(cel_GenericErrorSpace, EINVAL),
            cel_StatusCode_kInvalidArgument);
}

TEST(SystemErrorSpace, Canonical) {
  EXPECT_EQ(cel_ErrorSpace_Canonical(cel_SystemErrorSpace, 0),
            cel_StatusCode_kOk);

#ifndef _WIN32
  EXPECT_EQ(cel_ErrorSpace_Canonical(cel_SystemErrorSpace, EINVAL),
            cel_StatusCode_kInvalidArgument);
#else
  EXPECT_EQ(
      cel_ErrorSpace_Canonical(cel_SystemErrorSpace, ERROR_INVALID_PARAMETER),
      cel_StatusCode_kInvalidArgument);
#endif
}

TEST(CanonicalErrorSpace, Message) {
  char buf[1024];
  EXPECT_EQ(cel_ErrorSpace_Message(cel_CanonicalErrorSpace, cel_StatusCode_kOk,
                                   buf, sizeof(buf)),
            0);
  EXPECT_EQ(
      cel_ErrorSpace_Message(cel_CanonicalErrorSpace,
                             cel_StatusCode_kInvalidArgument, buf, sizeof(buf)),
      0);
}

TEST(GenericErrorSpace, Message) {
  char buf[1024];
  EXPECT_EQ(cel_ErrorSpace_Message(cel_GenericErrorSpace, 0, buf, sizeof(buf)),
            0);
  EXPECT_EQ(
      cel_ErrorSpace_Message(cel_GenericErrorSpace, EINVAL, buf, sizeof(buf)),
      0);
}

TEST(SystemErrorSpace, Message) {
  char buf[1024];
  EXPECT_EQ(cel_ErrorSpace_Message(cel_SystemErrorSpace, 0, buf, sizeof(buf)),
            0);
#ifndef _WIN32
  EXPECT_EQ(
      cel_ErrorSpace_Message(cel_SystemErrorSpace, EINVAL, buf, sizeof(buf)),
      0);
#else
  EXPECT_EQ(cel_ErrorSpace_Message(cel_SystemErrorSpace,
                                   ERROR_INVALID_PARAMETER, buf, sizeof(buf)),
            0);
#endif
}

}  // namespace
