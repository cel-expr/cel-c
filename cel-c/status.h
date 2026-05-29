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

#ifndef THIRD_PARTY_CEL_C_STATUS_H_
#define THIRD_PARTY_CEL_C_STATUS_H_

#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/error_space.h"
#include "cel-c/status_code.h"
#include "cel-c/string_view.h"

CEL_BEGIN_DECLS

typedef struct {
  union {
    alignas(8) char rep[96];
    // Encoded canonical status code. This is here for speed of checking and is
    // not public. Use the accessor methods to get the information you need. The
    // encoding is currently as follows: `(cel_StatusCode << 1) | oom`.
    unsigned int code;
  };
} cel_Status;

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Status_Construct(CEL_NONNULL(cel_Status*) status);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Status_Destruct(CEL_NONNULL(cel_Status*) status);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Status_Clear(CEL_NONNULL(cel_Status*) status);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Status_Reset(CEL_NONNULL(cel_Status*) status);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StatusCode
cel_Status_CanonicalCode(CEL_NONNULL(const cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(status);

  return (cel_StatusCode)(status->code >> 1);
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN CEL_NONNULL(const cel_ErrorSpace*)
    cel_Status_Space(CEL_NONNULL(const cel_Status*) status);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN int cel_Status_Code(CEL_NONNULL(const cel_Status*) status);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_Ok(CEL_NONNULL(const cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(status);

  return status->code == 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_Is(const cel_Status* cel_nonnull status,
                                     const cel_ErrorSpace* cel_nonnull space,
                                     int code) {
  return cel_Status_Space(status) == space && cel_Status_Code(status) == code;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsCancelled(CEL_NONNULL(const cel_Status*)
                                                  status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kCancelled;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsUnknown(CEL_NONNULL(const cel_Status*)
                                                status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kUnknown;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsInvalidArgument(
    CEL_NONNULL(const cel_Status*) status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kInvalidArgument;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsDeadlineExceeded(
    CEL_NONNULL(const cel_Status*) status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kDeadlineExceeded;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsNotFound(CEL_NONNULL(const cel_Status*)
                                                 status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kNotFound;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsAlreadyExists(CEL_NONNULL(const cel_Status*)
                                                      status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kAlreadyExists;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsPermissionDenied(
    CEL_NONNULL(const cel_Status*) status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kPermissionDenied;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsResourceExhausted(
    CEL_NONNULL(const cel_Status*) status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kResourceExhausted;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsFailedPrecondition(
    CEL_NONNULL(const cel_Status*) status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kFailedPrecondition;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsAborted(CEL_NONNULL(const cel_Status*)
                                                status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kAborted;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsOutOfRange(CEL_NONNULL(const cel_Status*)
                                                   status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kOutOfRange;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsUnimplemented(CEL_NONNULL(const cel_Status*)
                                                      status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kUnimplemented;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsInternal(CEL_NONNULL(const cel_Status*)
                                                 status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kInternal;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsUnavailable(CEL_NONNULL(const cel_Status*)
                                                    status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kUnavailable;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsDataLoss(CEL_NONNULL(const cel_Status*)
                                                 status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kDataLoss;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsUnauthenticated(
    CEL_NONNULL(const cel_Status*) status) {
  return cel_Status_CanonicalCode(status) == cel_StatusCode_kUnauthenticated;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Status_IsOutOfMemory(CEL_NONNULL(const cel_Status*)
                                                    status) {
  CEL_ASSERT_NOT_NULL(status);

  return (status->code & 1u) != 0;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_Status_Message(CEL_NONNULL(const cel_Status*)
                                                 status);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Status_SetCode(CEL_NONNULL(cel_Status*) status,
                                   CEL_NONNULL(const cel_ErrorSpace*) space,
                                   int code);

static CEL_INLINE void cel_Status_SetCanonicalCode(CEL_NONNULL(cel_Status*)
                                                       status,
                                                   cel_StatusCode code) {
  cel_Status_SetCode(status, cel_CanonicalErrorSpace, code);
}

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Status_SetMessage(CEL_NONNULL(cel_Status*) status,
                                      CEL_NONNULL(const char*) file, int line,
                                      cel_StringView message);

#define cel_Status_SetMessage(status, message) \
  cel_Status_SetMessage((status), __FILE__, __LINE__, (message))

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Status_VFormatMessage(CEL_NONNULL(cel_Status*) status,
                                          CEL_NONNULL(const char*) file,
                                          int line, const char* cel_nonnull fmt,
                                          va_list args);

#define cel_Status_VFormatMessage(status, fmt, args) \
  cel_Status_VFormatMessage((status), __FILE__, __LINE__, (fmt), (args))

static CEL_INLINE bool cel_Status_FormatMessage(CEL_NONNULL(cel_Status*) status,
                                                CEL_NONNULL(const char*) file,
                                                int line,
                                                const char* cel_nonnull fmt,
                                                ...) {
  va_list args;
  va_start(args, fmt);
  const bool ok = (cel_Status_VFormatMessage)(status, file, line, fmt, args);
  va_end(args);
  return ok;
}

#define cel_Status_FormatMessage(status, fmt, ...) \
  cel_Status_FormatMessage((status), __FILE__, __LINE__, (fmt), ##__VA_ARGS__)

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_OutOfMemoryStatus(CEL_NONNULL(cel_Status*) status,
                                      CEL_NONNULL(const char*) file, int line);

#define cel_OutOfMemoryStatus(status) \
  cel_OutOfMemoryStatus((status), __FILE__, __LINE__)

static CEL_INLINE bool cel_Status_Set(CEL_NONNULL(cel_Status*) status,
                                      CEL_NONNULL(const cel_ErrorSpace*) space,
                                      int code, CEL_NONNULL(const char*) file,
                                      int line, cel_StringView message) {
  cel_Status_SetCode(status, space, code);
  return (cel_Status_SetMessage)(status, file, line, message);
}

#define cel_Status_Set(status, space, code, message) \
  cel_Status_Set((status), (space), (code), __FILE__, __LINE__, (message))

CEL_ATTRIBUTE_VFORMAT(6)
static CEL_INLINE bool cel_Status_VFormat(
    CEL_NONNULL(cel_Status*) status, CEL_NONNULL(const cel_ErrorSpace*) space,
    int code, CEL_NONNULL(const char*) file, int line,
    CEL_NONNULL(const char*) fmt, va_list args) {
  cel_Status_SetCode(status, space, code);
  return (cel_Status_VFormatMessage)(status, file, line, fmt, args);
}

#define cel_Status_VFormat(status, space, code, fmt, args)                 \
  cel_Status_VFormat((status), (space), (code), __FILE__, __LINE__, (fmt), \
                     (args))

CEL_ATTRIBUTE_FORMAT(6, 7)
static CEL_INLINE bool cel_Status_Format(
    CEL_NONNULL(cel_Status*) status, CEL_NONNULL(const cel_ErrorSpace*) space,
    int code, CEL_NONNULL(const char*) file, int line,
    CEL_NONNULL(const char*) fmt, ...) {
  va_list args;
  va_start(args, fmt);
  const bool ok =
      (cel_Status_VFormat)(status, space, code, file, line, fmt, args);
  va_end(args);
  return ok;
}

#define cel_Status_Format(status, space, code, fmt, ...)                  \
  cel_Status_Format((status), (space), (code), __FILE__, __LINE__, (fmt), \
                    ##__VA_ARGS__)

#define cel_CanonicalStatus(status, code, message) \
  cel_Status_Set((status), cel_CanonicalErrorSpace, (code), (message))

#define cel_CanonicalStatusF(status, code, fmt, ...)                  \
  cel_Status_Format((status), cel_CanonicalErrorSpace, (code), (fmt), \
                    ##__VA_ARGS__)

#define cel_VCanonicalStatusF(status, code, fmt, args) \
  cel_Status_VFormat((status), cel_CanonicalErrorSpace, (code), (fmt), (args))

#define cel_AbortedStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kAborted, (message))

#define cel_AbortedStatusF(status, fmt, ...) \
  cel_CanonicalStatusF((status), cel_StatusCode_kAborted, (fmt), ##__VA_ARGS__)

#define cel_VAbortedStatusF(status, fmt, args) \
  cel_VCanonicalStatusF((status), cel_StatusCode_kAborted, (fmt), (args))

#define cel_AlreadyExistsStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kAlreadyExists, (message))

#define cel_AlreadyExistsStatusF(status, fmt, ...)                     \
  cel_CanonicalStatusF((status), cel_StatusCode_kAlreadyExists, (fmt), \
                       ##__VA_ARGS__)

#define cel_VAlreadyExistsStatusF(status, fmt, args) \
  cel_VCanonicalStatusF((status), cel_StatusCode_kAlreadyExists, (fmt), (args))

#define cel_CancelledStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kCancelled, (message))

#define cel_CancelledStatusF(status, fmt, ...)                     \
  cel_CanonicalStatusF((status), cel_StatusCode_kCancelled, (fmt), \
                       ##__VA_ARGS__)

#define cel_VCancelledStatusF(status, fmt, args) \
  cel_VCanonicalStatusF((status), cel_StatusCode_kCancelled, (fmt), (args))

#define cel_DataLossStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kDataLoss, (message))

#define cel_DataLossStatusF(status, fmt, ...) \
  cel_CanonicalStatusF((status), cel_StatusCode_kDataLoss, (fmt), ##__VA_ARGS__)

#define cel_VDataLossStatusF(status, fmt, args) \
  cel_VCanonicalStatusF((status), cel_StatusCode_kDataLoss, (fmt), (args))

#define cel_DeadlineExceededStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kDeadlineExceeded, (message))

#define cel_DeadlineExceededStatusF(status, fmt, ...)                     \
  cel_CanonicalStatusF((status), cel_StatusCode_kDeadlineExceeded, (fmt), \
                       ##__VA_ARGS__)

#define cel_VDeadlineExceededStatusF(status, fmt, args)                    \
  cel_VCanonicalStatusF((status), cel_StatusCode_kDeadlineExceeded, (fmt), \
                        (args))

#define cel_FailedPreconditionStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kFailedPrecondition, (message))

#define cel_FailedPreconditionStatusF(status, fmt, ...)                     \
  cel_CanonicalStatusF((status), cel_StatusCode_kFailedPrecondition, (fmt), \
                       ##__VA_ARGS__)

#define cel_VFailedPreconditionStatusF(status, fmt, args)                    \
  cel_VCanonicalStatusF((status), cel_StatusCode_kFailedPrecondition, (fmt), \
                        (args))

#define cel_InternalStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kInternal, (message))

#define cel_InternalStatusF(status, fmt, ...) \
  cel_CanonicalStatusF((status), cel_StatusCode_kInternal, (fmt), ##__VA_ARGS__)

#define cel_VInternalStatusF(status, fmt, args) \
  cel_VCanonicalStatusF((status), cel_StatusCode_kInternal, (fmt), (args))

#define cel_InvalidArgumentStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kInvalidArgument, (message))

#define cel_InvalidArgumentStatusF(status, fmt, ...)                     \
  cel_CanonicalStatusF((status), cel_StatusCode_kInvalidArgument, (fmt), \
                       ##__VA_ARGS__)

#define cel_VInvalidArgumentStatusF(status, fmt, args)                    \
  cel_VCanonicalStatusF((status), cel_StatusCode_kInvalidArgument, (fmt), \
                        (args))

#define cel_NotFoundStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kNotFound, (message))

#define cel_NotFoundStatusF(status, fmt, ...) \
  cel_CanonicalStatusF((status), cel_StatusCode_kNotFound, (fmt), ##__VA_ARGS__)

#define cel_VNotFoundStatusF(status, fmt, args) \
  cel_VCanonicalStatusF((status), cel_StatusCode_kNotFound, (fmt), (args))

#define cel_OutOfRangeStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kOutOfRange, (message))

#define cel_OutOfRangeStatusF(status, fmt, ...)                     \
  cel_CanonicalStatusF((status), cel_StatusCode_kOutOfRange, (fmt), \
                       ##__VA_ARGS__)

#define cel_VOutOfRangeStatusF(status, fmt, args) \
  cel_VCanonicalStatusF((status), cel_StatusCode_kOutOfRange, (fmt), (args))

#define cel_PermissionDeniedStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kPermissionDenied, (message))

#define cel_PermissionDeniedStatusF(status, fmt, ...)                     \
  cel_CanonicalStatusF((status), cel_StatusCode_kPermissionDenied, (fmt), \
                       ##__VA_ARGS__)

#define cel_VPermissionDeniedStatusF(status, fmt, args)                    \
  cel_VCanonicalStatusF((status), cel_StatusCode_kPermissionDenied, (fmt), \
                        (args))

#define cel_ResourceExhaustedStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kResourceExhausted, (message))

#define cel_ResourceExhaustedStatusF(status, fmt, ...)                     \
  cel_CanonicalStatusF((status), cel_StatusCode_kResourceExhausted, (fmt), \
                       ##__VA_ARGS__)

#define cel_VResourceExhaustedStatusF(status, fmt, args)                    \
  cel_VCanonicalStatusF((status), cel_StatusCode_kResourceExhausted, (fmt), \
                        (args))

#define cel_UnauthenticatedStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kUnauthenticated, (message))

#define cel_UnauthenticatedStatusF(status, fmt, ...)                     \
  cel_CanonicalStatusF((status), cel_StatusCode_kUnauthenticated, (fmt), \
                       ##__VA_ARGS__)

#define cel_VUnauthenticatedStatusF(status, fmt, args)                    \
  cel_VCanonicalStatusF((status), cel_StatusCode_kUnauthenticated, (fmt), \
                        (args))

#define cel_UnavailableStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kUnavailable, (message))

#define cel_UnavailableStatusF(status, fmt, ...)                     \
  cel_CanonicalStatusF((status), cel_StatusCode_kUnavailable, (fmt), \
                       ##__VA_ARGS__)

#define cel_VUnavailableStatusF(status, fmt, args) \
  cel_VCanonicalStatusF((status), cel_StatusCode_kUnavailable, (fmt), (args))

#define cel_UnimplementedStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kUnimplemented, (message))

#define cel_UnimplementedStatusF(status, fmt, ...)                     \
  cel_CanonicalStatusF((status), cel_StatusCode_kUnimplemented, (fmt), \
                       ##__VA_ARGS__)

#define cel_VUnimplementedStatusF(status, fmt, args) \
  cel_VCanonicalStatusF((status), cel_StatusCode_kUnimplemented, (fmt), (args))

#define cel_UnknownStatus(status, message) \
  cel_CanonicalStatus((status), cel_StatusCode_kUnknown, (message))

#define cel_UnknownStatusF(status, fmt, ...) \
  cel_CanonicalStatusF((status), cel_StatusCode_kUnknown, (fmt), ##__VA_ARGS__)

#define cel_VUnknownStatusF(status, fmt, args) \
  cel_VCanonicalStatusF((status), cel_StatusCode_kUnknown, (fmt), (args))

#define cel_GenericStatus(status, code, message)                              \
  cel_Status_Set((status), cel_GenericErrorSpace, (code), __FILE__, __LINE__, \
                 (message))

#define cel_GenericStatusF(status, code, fmt, ...)                  \
  cel_Status_Format((status), cel_GenericErrorSpace, (code), (fmt), \
                    ##__VA_ARGS__)

#define cel_VGenericStatusF(status, code, fmt, args) \
  cel_Status_VFormat((status), cel_GenericErrorSpace, (code), (fmt), (args))

#define cel_SystemStatus(status, code, message) \
  cel_Status_Set((status), cel_SystemErrorSpace, (code), (message))

#define cel_SystemStatusF(status, code, fmt, ...)                  \
  cel_Status_Format((status), cel_SystemErrorSpace, (code), (fmt), \
                    ##__VA_ARGS__)

#define cel_VSystemStatusF(status, code, fmt, args) \
  cel_Status_VFormat((status), cel_SystemErrorSpace, (code), (fmt), (args))

#define cel_CurrentGenericStatus(status, message) \
  cel_GenericStatus((status), cel_GenericErrorSpace_CurrentCode(), (message))

#define cel_CurrentGenericStatusF(status, fmt, ...)                        \
  cel_GenericStatusF((status), cel_GenericErrorSpace_CurrentCode(), (fmt), \
                     ##__VA_ARGS__)

#define cel_VCurrentGenericStatusF(status, fmt, args)                       \
  cel_VGenericStatusF((status), cel_GenericErrorSpace_CurrentCode(), (fmt), \
                      (args))

#define cel_CurrentSystemStatus(status, message) \
  cel_SystemStatus((status), cel_SystemErrorSpace_CurrentCode(), (message))

#define cel_CurrentSystemStatusF(status, fmt, ...)                       \
  cel_SystemStatusF((status), cel_SystemErrorSpace_CurrentCode(), (fmt), \
                    ##__VA_ARGS__)

#define cel_VCurrentSystemStatusF(status, fmt, args)                      \
  cel_VSystemStatusF((status), cel_SystemErrorSpace_CurrentCode(), (fmt), \
                     (args))

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Status_GetPayload(CEL_NONNULL(const cel_Status*) status,
                                      cel_StringView type_url,
                                      CEL_NULLABLE(cel_StringView*) value);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Status_SetPayload(CEL_NONNULL(cel_Status*) status,
                                      cel_StringView type_url,
                                      cel_StringView value,
                                      CEL_NONNULL(const char*) file, int line);

#define cel_Status_SetPayload(status, type_url, value) \
  cel_Status_SetPayload((status), (type_url), (value), __FILE__, __LINE__)

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Status_DeletePayload(CEL_NONNULL(cel_Status*) status,
                                         cel_StringView type_url);

typedef struct {
  size_t rep;
} cel_StatusPayloadIterator;

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StatusPayloadIterator
cel_Status_BeginPayloads(CEL_NONNULL(const cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(status);

  cel_StatusPayloadIterator iter;
  iter.rep = (size_t)-1;
  return iter;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Status_NextPayload(CEL_NONNULL(const cel_Status*) status,
                                       CEL_NULLABLE(cel_StringView*) type_url,
                                       CEL_NULLABLE(cel_StringView*) value,
                                       CEL_NONNULL(cel_StatusPayloadIterator*)
                                           iter);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN size_t cel_Status_Payloads(CEL_NONNULL(const cel_Status*) status);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Status_ClearPayloads(CEL_NONNULL(cel_Status*) status);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_STATUS_H_
