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

#ifndef THIRD_PARTY_CEL_C_ERROR_SPACE_H_
#define THIRD_PARTY_CEL_C_ERROR_SPACE_H_

#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/cstring_view.h"
#include "cel-c/status_code.h"

CEL_BEGIN_DECLS

typedef struct cel_ErrorSpaceVTable cel_ErrorSpaceVTable;
typedef struct cel_ErrorSpace cel_ErrorSpace;

typedef cel_StatusCode (*cel_ErrorSpaceVTable_Canonical)(
    CEL_NONNULL(const cel_ErrorSpace*) space, int code);
typedef int (*cel_ErrorSpaceVTable_Message)(CEL_NONNULL(const cel_ErrorSpace*)
                                                space,
                                            int code, CEL_NONNULL(char*) buf,
                                            size_t buflen);
typedef bool (*cel_ErrorSpaceVTable_OutOfMemory)(
    CEL_NONNULL(const cel_ErrorSpace*) space, int code);

struct cel_ErrorSpaceVTable {
  cel_CStringView name;

  // NOLINTBEGIN(google3-readability-class-member-naming)
  CEL_NONNULL(cel_ErrorSpaceVTable_OutOfMemory) OutOfMemory;
  CEL_NONNULL(cel_ErrorSpaceVTable_Canonical) Canonical;
  CEL_NONNULL(cel_ErrorSpaceVTable_Message) Message;
  // NOLINTEND(google3-readability-class-member-naming)
};

struct cel_ErrorSpace {
  CEL_NONNULL(const cel_ErrorSpaceVTable*) vtable;
};

// The canonical error space.
CEL_EXTERN CEL_NONNULL(const cel_ErrorSpace*) const cel_CanonicalErrorSpace;

// The generic error space. Similar to C++11 `std::generic_category()`.
CEL_EXTERN CEL_NONNULL(const cel_ErrorSpace*) const cel_GenericErrorSpace;

// The system error space. Similar to C++11 `std::system_category()`.
CEL_EXTERN CEL_NONNULL(const cel_ErrorSpace*) const cel_SystemErrorSpace;

// cel_ErrorSpace_Name
//
// Returns the name of the error space `space`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_CStringView
cel_ErrorSpace_Name(CEL_NONNULL(const cel_ErrorSpace*) space) {
  CEL_ASSERT_NOT_NULL(space);
  CEL_ASSERT_NOT_NULL(space->vtable);

  return space->vtable->name;
}

// cel_ErrorSpace_OutOfMemory
//
// Tests whether the error code `code` represents an out of memory condition.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_ErrorSpace_OutOfMemory(
    CEL_NONNULL(const cel_ErrorSpace*) space, int code) {
  CEL_ASSERT_NOT_NULL(space);
  CEL_ASSERT_NOT_NULL(space->vtable);
  CEL_ASSERT_NOT_NULL(space->vtable->OutOfMemory);

  return (*space->vtable->OutOfMemory)(space, code);
}

// cel_ErrorSpace_Canonical
//
// Maps the error code `code` belonging to the error space `space` to a
// canonical code belonging to the error space `cel_CanonicalErrorSpace`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StatusCode
cel_ErrorSpace_Canonical(CEL_NONNULL(const cel_ErrorSpace*) space, int code) {
  CEL_ASSERT_NOT_NULL(space);
  CEL_ASSERT_NOT_NULL(space->vtable);
  CEL_ASSERT_NOT_NULL(space->vtable->Canonical);

  return (*space->vtable->Canonical)(space, code);
}

// cel_ErrorSpace_Message
//
// Retrieves an error message for the given error code `code` in the error space
// `space` and places it in the buffer starting at `buf` and extending `buflen`
// bytes. The buffer must not be null and must have a size greater than 0. On
// success `0` is returned and the string placed in the buffer is guaranteed to
// be null terminated. Otherwise an error code belonging to this error space is
// returned.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int cel_ErrorSpace_Message(CEL_NONNULL(const cel_ErrorSpace*)
                                                 space,
                                             int code, CEL_NONNULL(char*) buf,
                                             size_t buflen) {
  CEL_ASSERT_NOT_NULL(space);
  CEL_ASSERT_NOT_NULL(space->vtable);
  CEL_ASSERT_NOT_NULL(space->vtable->Message);
  CEL_ASSERT_NOT_NULL(buf);

  return (*space->vtable->Message)(space, code, buf, buflen);
}

// cel_GenericErrorSpace_CurrentCode
//
// Returns the current error code for the generic error space. This is `errno`
// on all platforms.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN int cel_GenericErrorSpace_CurrentCode();

// cel_SystemErrorSpace_CurrentCode
//
// Returns the current error code for the system error space. On Windows this is
// the result of `GetLastError()`, on all other platforms it is `errno`.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN int cel_SystemErrorSpace_CurrentCode();

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_ERROR_SPACE_H_
