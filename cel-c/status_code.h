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

#ifndef THIRD_PARTY_CEL_C_STATUS_CODE_H_
#define THIRD_PARTY_CEL_C_STATUS_CODE_H_

#include "cel-c/config.h"
#include "cel-c/error_code.h"

CEL_BEGIN_DECLS

// Canonical error codes for `cel_Status`. All values have a 1:1 mapping with
// `cel_ErrorCode` except `cel_StatusCode_kOk`.
typedef enum CEL_ATTRIBUTE_OPEN_ENUM {
  cel_StatusCode_kOk = 0,
  cel_StatusCode_kCancelled = cel_ErrorCode_kCancelled,
  cel_StatusCode_kUnknown = cel_ErrorCode_kUnknown,
  cel_StatusCode_kInvalidArgument = cel_ErrorCode_kInvalidArgument,
  cel_StatusCode_kDeadlineExceeded = cel_ErrorCode_kDeadlineExceeded,
  cel_StatusCode_kNotFound = cel_ErrorCode_kNotFound,
  cel_StatusCode_kAlreadyExists = cel_ErrorCode_kAlreadyExists,
  cel_StatusCode_kPermissionDenied = cel_ErrorCode_kPermissionDenied,
  cel_StatusCode_kResourceExhausted = cel_ErrorCode_kResourceExhausted,
  cel_StatusCode_kFailedPrecondition = cel_ErrorCode_kFailedPrecondition,
  cel_StatusCode_kAborted = cel_ErrorCode_kAborted,
  cel_StatusCode_kOutOfRange = cel_ErrorCode_kOutOfRange,
  cel_StatusCode_kUnimplemented = cel_ErrorCode_kUnimplemented,
  cel_StatusCode_kInternal = cel_ErrorCode_kInternal,
  cel_StatusCode_kUnavailable = cel_ErrorCode_kUnavailable,
  cel_StatusCode_kDataLoss = cel_ErrorCode_kDataLoss,
  cel_StatusCode_kUnauthenticated = cel_ErrorCode_kUnauthenticated,
} cel_StatusCode;

CEL_ATTRIBUTE_PURE
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(const char*) cel_StatusCode_Name(cel_StatusCode code);

CEL_ATTRIBUTE_PURE
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(const char*) cel_StatusCode_Message(cel_StatusCode code);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_STATUS_CODE_H_
