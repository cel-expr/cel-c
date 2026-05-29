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

#ifndef THIRD_PARTY_CEL_C_ERROR_CODE_H_
#define THIRD_PARTY_CEL_C_ERROR_CODE_H_

#include "cel-c/config.h"

CEL_BEGIN_DECLS

// Canonical error codes for `cel_Error`.
typedef enum CEL_ATTRIBUTE_OPEN_ENUM {
  cel_ErrorCode_kCancelled = 1,
  cel_ErrorCode_kUnknown = 2,
  cel_ErrorCode_kInvalidArgument = 3,
  cel_ErrorCode_kDeadlineExceeded = 4,
  cel_ErrorCode_kNotFound = 5,
  cel_ErrorCode_kAlreadyExists = 6,
  cel_ErrorCode_kPermissionDenied = 7,
  cel_ErrorCode_kResourceExhausted = 8,
  cel_ErrorCode_kFailedPrecondition = 9,
  cel_ErrorCode_kAborted = 10,
  cel_ErrorCode_kOutOfRange = 11,
  cel_ErrorCode_kUnimplemented = 12,
  cel_ErrorCode_kInternal = 13,
  cel_ErrorCode_kUnavailable = 14,
  cel_ErrorCode_kDataLoss = 15,
  cel_ErrorCode_kUnauthenticated = 16,
} cel_ErrorCode;

CEL_ATTRIBUTE_PURE
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(const char*) cel_ErrorCode_Name(cel_ErrorCode code);

CEL_ATTRIBUTE_PURE
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(const char*) cel_ErrorCode_Message(cel_ErrorCode code);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_ERROR_CODE_H_
