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

#include "cel-c/error_code.h"

#include "cel-c/assert.h"
#include "cel-c/config.h"

extern "C" CEL_NONNULL(const char*) cel_ErrorCode_Name(cel_ErrorCode code) {
  CEL_ASSERT_NE(code, 0);

  switch (code) {
    case cel_ErrorCode_kCancelled:
      return "CANCELLED";
    case cel_ErrorCode_kInvalidArgument:
      return "INVALID_ARGUMENT";
    case cel_ErrorCode_kDeadlineExceeded:
      return "DEADLINE_EXCEEDED";
    case cel_ErrorCode_kNotFound:
      return "NOT_FOUND";
    case cel_ErrorCode_kAlreadyExists:
      return "ALREADY_EXISTS";
    case cel_ErrorCode_kPermissionDenied:
      return "PERMISSION_DENIED";
    case cel_ErrorCode_kResourceExhausted:
      return "RESOURCE_EXHAUSTED";
    case cel_ErrorCode_kFailedPrecondition:
      return "FAILED_PRECONDITION";
    case cel_ErrorCode_kAborted:
      return "ABORTED";
    case cel_ErrorCode_kOutOfRange:
      return "OUT_OF_RANGE";
    case cel_ErrorCode_kUnimplemented:
      return "UNIMPLEMENTED";
    case cel_ErrorCode_kInternal:
      return "INTERNAL";
    case cel_ErrorCode_kUnavailable:
      return "UNAVAILABLE";
    case cel_ErrorCode_kDataLoss:
      return "DATA_LOSS";
    case cel_ErrorCode_kUnauthenticated:
      return "UNAUTHENTICATED";
    default:
      return "UNKNOWN";
  }
}

extern "C" CEL_NONNULL(const char*) cel_ErrorCode_Message(cel_ErrorCode code) {
  switch (code) {
    case cel_ErrorCode_kCancelled:
      return "the operation was cancelled";
    case cel_ErrorCode_kUnknown:
      return "unknown error";
    case cel_ErrorCode_kInvalidArgument:
      return "request contains an invalid argument";
    case cel_ErrorCode_kDeadlineExceeded:
      return "deadline expired before operation could complete";
    case cel_ErrorCode_kNotFound:
      return "requested entity was not found";
    case cel_ErrorCode_kAlreadyExists:
      return "requested entity already exists";
    case cel_ErrorCode_kPermissionDenied:
      return "caller does not have permission";
    case cel_ErrorCode_kResourceExhausted:
      return "resource has been exhausted";
    case cel_ErrorCode_kFailedPrecondition:
      return "precondition check failed";
    case cel_ErrorCode_kAborted:
      return "operation was aborted";
    case cel_ErrorCode_kOutOfRange:
      return "operation was attempted past the valid range";
    case cel_ErrorCode_kUnimplemented:
      return "operation is not implemented, supported, or enabled";
    case cel_ErrorCode_kInternal:
      return "internal error encountered";
    case cel_ErrorCode_kUnavailable:
      return "service is currently unavailable";
    case cel_ErrorCode_kDataLoss:
      return "unrecoverable data loss or corruption";
    case cel_ErrorCode_kUnauthenticated:
      return "request is missing required authentication credential";
    default:
      return "there was an error, that's all we know";
  }
}
