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

#ifndef THIRD_PARTY_CEL_C_ERROR_H_
#define THIRD_PARTY_CEL_C_ERROR_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/error_code.h"
#include "cel-c/error_space.h"
#include "cel-c/string_view.h"

CEL_BEGIN_DECLS

typedef struct cel_Error cel_Error;

CEL_EXTERN const cel_Error* const cel_nonnull cel_DefaultError;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_Error* cel_nullable cel_Error_New(cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Error_Clear(cel_Error* cel_nonnull error);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_ErrorCode
cel_Error_CanonicalCode(const cel_Error* cel_nonnull error);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN int cel_Error_Code(const cel_Error* cel_nonnull error);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Error_SetCode(cel_Error* cel_nonnull error,
                                  const cel_ErrorSpace* cel_nonnull space,
                                  int code);

static CEL_INLINE void cel_Error_SetCanonicalCode(cel_Error* cel_nonnull error,
                                                  cel_ErrorCode code) {
  cel_Error_SetCode(error, cel_CanonicalErrorSpace, code);
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const cel_ErrorSpace* cel_nonnull
cel_Error_Space(const cel_Error* cel_nonnull error);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_Is(const cel_Error* cel_nonnull error,
                                    const cel_ErrorSpace* cel_nonnull space,
                                    int code) {
  return cel_Error_Space(error) == space && cel_Error_Code(error) == code;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsCancelled(
    const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kCancelled;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsUnknown(const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kUnknown;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsInvalidArgument(
    const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kInvalidArgument;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsDeadlineExceeded(
    const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kDeadlineExceeded;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsNotFound(
    const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kNotFound;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsAlreadyExists(
    const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kAlreadyExists;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsPermissionDenied(
    const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kPermissionDenied;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsResourceExhausted(
    const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kResourceExhausted;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsFailedPrecondition(
    const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kFailedPrecondition;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsAborted(const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kAborted;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsOutOfRange(
    const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kOutOfRange;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsUnimplemented(
    const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kUnimplemented;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsInternal(
    const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kInternal;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsUnavailable(
    const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kUnavailable;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsDataLoss(
    const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kDataLoss;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_IsUnauthenticated(
    const cel_Error* cel_nonnull error) {
  return cel_Error_CanonicalCode(error) == cel_ErrorCode_kUnauthenticated;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_Error_Message(const cel_Error* cel_nonnull error);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Error_SetMessage(cel_Error* cel_nonnull error,
                                     cel_StringView message);

CEL_ATTRIBUTE_VFORMAT(3)
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Error_VFormatMessage(cel_Error* cel_nonnull error,
                                         cel_Arena* cel_nonnull arena,
                                         const char* cel_nonnull fmt,
                                         va_list args);

CEL_ATTRIBUTE_FORMAT(3, 4)
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_FormatMessage(cel_Error* cel_nonnull error,
                                               cel_Arena* cel_nonnull arena,
                                               const char* cel_nonnull fmt,
                                               ...) {
  va_list args;
  va_start(args, fmt);
  const bool ok = cel_Error_VFormatMessage(error, arena, fmt, args);
  va_end(args);
  return ok;
}

static CEL_INLINE void cel_Error_Set(cel_Error* cel_nonnull error,
                                     const cel_ErrorSpace* cel_nonnull space,
                                     int code, cel_StringView message) {
  cel_Error_SetCode(error, space, code);
  cel_Error_SetMessage(error, message);
}

CEL_ATTRIBUTE_VFORMAT(5)
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_VFormat(
    cel_Error* cel_nonnull error, const cel_ErrorSpace* cel_nonnull space,
    int code, cel_Arena* cel_nonnull arena, const char* cel_nonnull fmt,
    va_list args) {
  cel_Error_SetCode(error, space, code);
  return cel_Error_VFormatMessage(error, arena, fmt, args);
}

CEL_ATTRIBUTE_FORMAT(5, 6)
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Error_Format(cel_Error* cel_nonnull error,
                                        const cel_ErrorSpace* cel_nonnull space,
                                        int code, cel_Arena* cel_nonnull arena,
                                        const char* cel_nonnull fmt, ...) {
  va_list args;
  va_start(args, fmt);
  const bool ok = cel_Error_VFormat(error, space, code, arena, fmt, args);
  va_end(args);
  return ok;
}

#define cel_CanonicalError(error, code, message) \
  cel_Error_Set((error), cel_CanonicalErrorSpace, (code), (message))

#define cel_CanonicalErrorF(error, code, arena, fmt, ...)                    \
  cel_Error_Format((error), cel_CanonicalErrorSpace, (code), (arena), (fmt), \
                   ##__VA_ARGS__)

#define cel_VCanonicalErrorF(error, code, arena, fmt, args)                   \
  cel_Error_VFormat((error), cel_CanonicalErrorSpace, (code), (arena), (fmt), \
                    (args))

#define cel_AbortedError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kAborted, (message))

#define cel_AbortedErrorF(error, arena, fmt, ...)                      \
  cel_CanonicalErrorF((error), cel_ErrorCode_kAborted, (arena), (fmt), \
                      ##__VA_ARGS__)

#define cel_VAbortedErrorF(error, arena, fmt, args) \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kAborted, (arena), (fmt), (args))

#define cel_AlreadyExistsError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kAlreadyExists, (message))

#define cel_AlreadyExistsErrorF(error, arena, fmt, ...)                      \
  cel_CanonicalErrorF((error), cel_ErrorCode_kAlreadyExists, (arena), (fmt), \
                      ##__VA_ARGS__)

#define cel_VAlreadyExistsErrorF(error, arena, fmt, args)                     \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kAlreadyExists, (arena), (fmt), \
                       (args))

#define cel_CancelledError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kCancelled, (message))

#define cel_CancelledErrorF(error, arena, fmt, ...)                      \
  cel_CanonicalErrorF((error), cel_ErrorCode_kCancelled, (arena), (fmt), \
                      ##__VA_ARGS__)

#define cel_VCancelledErrorF(error, arena, fmt, args)                     \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kCancelled, (arena), (fmt), \
                       (args))

#define cel_DataLossError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kDataLoss, (message))

#define cel_DataLossErrorF(error, arena, fmt, ...)                      \
  cel_CanonicalErrorF((error), cel_ErrorCode_kDataLoss, (arena), (fmt), \
                      ##__VA_ARGS__)

#define cel_VDataLossErrorF(error, arena, fmt, args) \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kDataLoss, (arena), (fmt), (args))

#define cel_DeadlineExceededError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kDeadlineExceeded, (message))

#define cel_DeadlineExceededErrorF(error, arena, fmt, ...)               \
  cel_CanonicalErrorF((error), cel_ErrorCode_kDeadlineExceeded, (arena), \
                      (fmt), ##__VA_ARGS__)

#define cel_VDeadlineExceededErrorF(error, arena, fmt, args)              \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kDeadlineExceeded, (arena), \
                       (fmt), (args))

#define cel_FailedPreconditionError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kFailedPrecondition, (message))

#define cel_FailedPreconditionErrorF(error, arena, fmt, ...)               \
  cel_CanonicalErrorF((error), cel_ErrorCode_kFailedPrecondition, (arena), \
                      (fmt), ##__VA_ARGS__)

#define cel_VFailedPreconditionErrorF(error, arena, fmt, args)              \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kFailedPrecondition, (arena), \
                       (fmt), (args))

#define cel_InternalError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kInternal, (message))

#define cel_InternalErrorF(error, arena, fmt, ...)                      \
  cel_CanonicalErrorF((error), cel_ErrorCode_kInternal, (arena), (fmt), \
                      ##__VA_ARGS__)

#define cel_VInternalErrorF(error, arena, fmt, args) \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kInternal, (arena), (fmt), (args))

#define cel_InvalidArgumentError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kInvalidArgument, (message))

#define cel_InvalidArgumentErrorF(error, arena, fmt, ...)                      \
  cel_CanonicalErrorF((error), cel_ErrorCode_kInvalidArgument, (arena), (fmt), \
                      ##__VA_ARGS__)

#define cel_VInvalidArgumentErrorF(error, arena, fmt, args)              \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kInvalidArgument, (arena), \
                       (fmt), (args))

#define cel_NotFoundError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kNotFound, (message))

#define cel_NotFoundErrorF(error, arena, fmt, ...)                      \
  cel_CanonicalErrorF((error), cel_ErrorCode_kNotFound, (arena), (fmt), \
                      ##__VA_ARGS__)

#define cel_VNotFoundErrorF(error, arena, fmt, args) \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kNotFound, (arena), (fmt), (args))

#define cel_OutOfRangeError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kOutOfRange, (message))

#define cel_OutOfRangeErrorF(error, arena, fmt, ...)                      \
  cel_CanonicalErrorF((error), cel_ErrorCode_kOutOfRange, (arena), (fmt), \
                      ##__VA_ARGS__)

#define cel_VOutOfRangeErrorF(error, arena, fmt, args)                     \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kOutOfRange, (arena), (fmt), \
                       (args))

#define cel_PermissionDeniedError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kPermissionDenied, (message))

#define cel_PermissionDeniedErrorF(error, arena, fmt, ...)               \
  cel_CanonicalErrorF((error), cel_ErrorCode_kPermissionDenied, (arena), \
                      (fmt), ##__VA_ARGS__)

#define cel_VPermissionDeniedErrorF(error, arena, fmt, args)              \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kPermissionDenied, (arena), \
                       (fmt), (args))

#define cel_ResourceExhaustedError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kResourceExhausted, (message))

#define cel_ResourceExhaustedErrorF(error, arena, fmt, ...)               \
  cel_CanonicalErrorF((error), cel_ErrorCode_kResourceExhausted, (arena), \
                      (fmt), ##__VA_ARGS__)

#define cel_VResourceExhaustedErrorF(error, arena, fmt, args)              \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kResourceExhausted, (arena), \
                       (fmt), (args))

#define cel_UnauthenticatedError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kUnauthenticated, (message))

#define cel_UnauthenticatedErrorF(error, arena, fmt, ...)                      \
  cel_CanonicalErrorF((error), cel_ErrorCode_kUnauthenticated, (arena), (fmt), \
                      ##__VA_ARGS__)

#define cel_VUnauthenticatedErrorF(error, arena, fmt, args)              \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kUnauthenticated, (arena), \
                       (fmt), (args))

#define cel_UnavailableError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kUnavailable, (message))

#define cel_UnavailableErrorF(error, arena, fmt, ...)                      \
  cel_CanonicalErrorF((error), cel_ErrorCode_kUnavailable, (arena), (fmt), \
                      ##__VA_ARGS__)

#define cel_VUnavailableErrorF(error, arena, fmt, args)                     \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kUnavailable, (arena), (fmt), \
                       (args))

#define cel_UnimplementedError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kUnimplemented, (message))

#define cel_UnimplementedErrorF(error, arena, fmt, ...)                      \
  cel_CanonicalErrorF((error), cel_ErrorCode_kUnimplemented, (arena), (fmt), \
                      ##__VA_ARGS__)

#define cel_VUnimplementedErrorF(error, arena, fmt, args)                     \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kUnimplemented, (arena), (fmt), \
                       (args))

#define cel_UnknownError(error, message) \
  cel_CanonicalError((error), cel_ErrorCode_kUnknown, (message))

#define cel_UnknownErrorF(error, arena, fmt, ...)                      \
  cel_CanonicalErrorF((error), cel_ErrorCode_kUnknown, (arena), (fmt), \
                      ##__VA_ARGS__)

#define cel_VUnknownErrorF(error, arena, fmt, args) \
  cel_VCanonicalErrorF((error), cel_ErrorCode_kUnknown, (arena), (fmt), (args))

#define cel_GenericError(error, code, message) \
  cel_Error((error), cel_GenericErrorSpace, (code), (message))

#define cel_GenericErrorF(error, code, arena, fmt, ...)              \
  cel_ErrorF((error), cel_GenericErrorSpace, (code), (arena), (fmt), \
             ##__VA_ARGS__)

#define cel_VGenericErrorF(error, code, arena, fmt, args) \
  cel_VErrorF((error), cel_GenericErrorSpace, (code), (arena), (fmt), (args))

#define cel_SystemError(error, code, message) \
  cel_Error((error), cel_SystemErrorSpace, (code), (message))

#define cel_SystemErrorF(error, code, arena, fmt, ...)              \
  cel_ErrorF((error), cel_SystemErrorSpace, (code), (arena), (fmt), \
             ##__VA_ARGS__)

#define cel_VSystemErrorF(error, code, arena, fmt, args) \
  cel_VErrorF((error), cel_SystemErrorSpace, (code), (arena), (fmt), (args))

#define cel_CurrentGenericError(error, message) \
  cel_GenericError((error), cel_GenericErrorSpace_CurrentCode(), (message))

#define cel_CurrentGenericErrorF(error, arena, fmt, ...)                   \
  cel_GenericErrorF((error), cel_GenericErrorSpace_CurrentCode(), (arena), \
                    (fmt), ##__VA_ARGS__)

#define cel_VCurrentGenericErrorF(error, arena, fmt, args)                  \
  cel_VGenericErrorF((error), cel_GenericErrorSpace_CurrentCode(), (arena), \
                     (fmt), (args))

#define cel_CurrentSystemError(error, message) \
  cel_SystemError((error), cel_SystemErrorSpace_CurrentCode(), (message))

#define cel_CurrentSystemErrorF(error, arena, fmt, ...)                  \
  cel_SystemErrorF((error), cel_SystemErrorSpace_CurrentCode(), (arena), \
                   (fmt), ##__VA_ARGS__)

#define cel_VCurrentSystemErrorF(error, arena, fmt, args)                 \
  cel_VSystemErrorF((error), cel_SystemErrorSpace_CurrentCode(), (arena), \
                    (fmt), (args))

typedef struct {
  uintptr_t rep;
} cel_ErrorPayloadIterator;

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_ErrorPayloadIterator
cel_Error_BeginPayloads(const cel_Error* cel_nonnull error) {
  CEL_ASSERT_NOT_NULL(error);

  cel_ErrorPayloadIterator iter;
  iter.rep = (uintptr_t)-1;
  return iter;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Error_NextPayload(
    const cel_Error* cel_nonnull error, cel_StringView* cel_nullable type_url,
    cel_StringView* cel_nullable value,
    cel_ErrorPayloadIterator* cel_nonnull iter);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN size_t cel_Error_Payloads(const cel_Error* cel_nonnull error);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Error_GetPayload(const cel_Error* cel_nonnull error,
                                     cel_StringView type_url,
                                     cel_StringView* cel_nullable value);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Error_SetPayload(cel_Error* cel_nonnull error,
                                     cel_StringView type_url,
                                     cel_StringView value,
                                     cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Error_DeletePayload(cel_Error* cel_nonnull error,
                                        cel_StringView type_url,
                                        cel_StringView* cel_nullable value);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_Error_ClearPayloads(cel_Error* cel_nonnull error);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_ERROR_H_
