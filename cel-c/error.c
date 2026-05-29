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

#include "cel-c/error.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/error_code.h"
#include "cel-c/error_space.h"
#include "cel-c/status_code.h"
#include "cel-c/string_view.h"

typedef struct _cel_ErrorPayload _cel_ErrorPayload;

struct _cel_ErrorPayload {
  struct _cel_ErrorPayload* cel_nullable prev;
  struct _cel_ErrorPayload* cel_nullable next;
  cel_StringView type_url;
  cel_StringView value;
};

struct cel_Error {
  cel_StringView message;
  const cel_ErrorSpace* cel_nullable space;
  int raw_code;
  cel_ErrorCode code;
  size_t payloads_len;
  _cel_ErrorPayload* cel_nullable payloads_head;
  _cel_ErrorPayload* cel_nullable payloads_tail;
};

static const cel_Error _cel_DefaultError = {
    .message = CEL_STRINGVIEW_C(""),
    .space = cel_nullptr,
    .raw_code = cel_ErrorCode_kUnknown,
    .code = cel_ErrorCode_kUnknown,
    .payloads_len = 0,
    .payloads_head = cel_nullptr,
    .payloads_tail = cel_nullptr,
};

const cel_Error* const cel_nonnull cel_DefaultError = &_cel_DefaultError;

cel_Error* cel_nullable cel_Error_New(cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(arena);

  cel_Error* error =
      (cel_Error*)cel_Arena_Malloc(arena, sizeof(cel_Error), cel_nullptr);
  if (CEL_LIKELY(error != cel_nullptr)) {
    memset(error, 0, sizeof(*error));
    error->space = cel_CanonicalErrorSpace;
    error->raw_code = cel_ErrorCode_kUnknown;
    error->code = (cel_ErrorCode)error->raw_code;
  }
  return error;
}

void cel_Error_Clear(cel_Error* cel_nonnull error) {
  CEL_ASSERT_NOT_NULL(error);

  error->message = cel_StringView_FromString("");
  error->space = cel_CanonicalErrorSpace;
  error->raw_code = cel_ErrorCode_kUnknown;
  error->code = (cel_ErrorCode)error->raw_code;
  cel_Error_ClearPayloads(error);
}

cel_ErrorCode cel_Error_CanonicalCode(const cel_Error* cel_nonnull error) {
  CEL_ASSERT_NOT_NULL(error);

  return error->code;
}

int cel_Error_Code(const cel_Error* cel_nonnull error) {
  CEL_ASSERT_NOT_NULL(error);

  return error->raw_code;
}

void cel_Error_SetCode(cel_Error* cel_nonnull error,
                       const cel_ErrorSpace* cel_nonnull space, int code) {
  CEL_ASSERT_NOT_NULL(error);
  CEL_ASSERT_NOT_NULL(space);

  cel_StatusCode canonical_code = cel_ErrorSpace_Canonical(space, code);
  CEL_ASSERT_NE(canonical_code, cel_StatusCode_kOk);

  if (CEL_UNLIKELY(canonical_code == cel_StatusCode_kOk)) {
    space = cel_CanonicalErrorSpace;
    code = cel_StatusCode_kUnknown;
    canonical_code = cel_StatusCode_kUnknown;
  }

  error->space = space;
  error->raw_code = code;
  error->code = (cel_ErrorCode)canonical_code;
}

const cel_ErrorSpace* cel_nonnull
cel_Error_Space(const cel_Error* cel_nonnull error) {
  CEL_ASSERT_NOT_NULL(error);

  const cel_ErrorSpace* space = error->space;
  if (space == cel_nullptr) {
    space = cel_CanonicalErrorSpace;
  }
  return space;
}

cel_StringView cel_Error_Message(const cel_Error* cel_nonnull error) {
  CEL_ASSERT_NOT_NULL(error);

  return error->message;
}

void cel_Error_SetMessage(cel_Error* cel_nonnull error,
                          cel_StringView message) {
  CEL_ASSERT_NOT_NULL(error);

  error->message = message;
}

bool cel_Error_VFormatMessage(cel_Error* cel_nonnull error,
                              cel_Arena* cel_nonnull arena,
                              const char* cel_nonnull fmt, va_list args) {
  CEL_ASSERT_NOT_NULL(error);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_NOT_NULL(fmt);

  cel_StringView message;
  if (!cel_Arena_VPrintF(arena, &message, fmt, args)) {
    return false;
  }
  cel_Error_SetMessage(error, message);
  return true;
}

bool cel_Error_NextPayload(const cel_Error* cel_nonnull error,
                           cel_StringView* cel_nullable type_url,
                           cel_StringView* cel_nullable value,
                           cel_ErrorPayloadIterator* cel_nonnull iter) {
  CEL_ASSERT_NOT_NULL(error);
  CEL_ASSERT_NOT_NULL(iter);
  const _cel_ErrorPayload* payload;
  if (iter->rep == (uintptr_t)-1) {
    payload = error->payloads_head;
  } else {
    payload = (const _cel_ErrorPayload*)iter->rep;
    payload = payload->next;
  }

  if (payload == cel_nullptr) {
    return false;
  }

  if (type_url != cel_nullptr) {
    *type_url = payload->type_url;
  }
  if (value != cel_nullptr) {
    *value = payload->value;
  }
  iter->rep = (uintptr_t)payload;
  return true;
}

size_t cel_Error_Payloads(const cel_Error* cel_nonnull error) {
  CEL_ASSERT_NOT_NULL(error);

  return error->payloads_len;
}

bool cel_Error_GetPayload(const cel_Error* cel_nonnull error,
                          cel_StringView type_url,
                          cel_StringView* cel_nullable value) {
  CEL_ASSERT_NOT_NULL(error);

  if (cel_StringView_Empty(type_url)) {
    return false;
  }

  const _cel_ErrorPayload* payload = error->payloads_head;
  while (payload != cel_nullptr) {
    if (cel_StringView_Equals(type_url, payload->type_url)) {
      if (value != cel_nullptr) {
        *value = payload->value;
      }
      return true;
    }
    payload = payload->next;
  }
  return false;
}

bool cel_Error_SetPayload(cel_Error* cel_nonnull error, cel_StringView type_url,
                          cel_StringView value, cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(error);
  CEL_ASSERT_NOT(cel_StringView_Empty(type_url));
  CEL_ASSERT_NOT_NULL(arena);

  _cel_ErrorPayload* payload = error->payloads_head;
  while (payload != cel_nullptr) {
    if (cel_StringView_Equals(type_url, payload->type_url)) {
      payload->value = value;
      return true;
    }
    payload = payload->next;
  }

  payload = (_cel_ErrorPayload*)cel_Arena_Malloc(
      arena, sizeof(_cel_ErrorPayload), cel_nullptr);
  if (CEL_UNLIKELY(payload == cel_nullptr)) {
    return false;
  }
  memset(payload, 0, sizeof(*payload));
  payload->type_url = type_url;
  payload->value = value;
  payload->prev = error->payloads_tail;

  if (error->payloads_tail != cel_nullptr) {
    error->payloads_tail->next = payload;
  } else {
    CEL_ASSERT_NULL(error->payloads_head);
    error->payloads_head = payload;
  }
  error->payloads_tail = payload;
  ++error->payloads_len;
  return true;
}

bool cel_Error_DeletePayload(cel_Error* cel_nonnull error,
                             cel_StringView type_url,
                             cel_StringView* cel_nullable value) {
  CEL_ASSERT_NOT_NULL(error);
  CEL_ASSERT_NOT(cel_StringView_Empty(type_url));

  if (cel_StringView_Empty(type_url)) {
    return false;
  }

  _cel_ErrorPayload* payload = error->payloads_head;
  while (payload != cel_nullptr) {
    if (cel_StringView_Equals(type_url, payload->type_url)) {
      if (value != cel_nullptr) {
        *value = payload->value;
      }
      if (payload->prev != cel_nullptr) {
        payload->prev->next = payload->next;
      } else {
        CEL_ASSERT_EQ(payload, error->payloads_head);
        error->payloads_head = payload->next;
      }
      if (payload->next != cel_nullptr) {
        payload->next->prev = payload->prev;
      } else {
        CEL_ASSERT_EQ(payload, error->payloads_tail);
        error->payloads_tail = payload->prev;
      }
      --error->payloads_len;
      return true;
    }
    payload = payload->next;
  }
  return false;
}

void cel_Error_ClearPayloads(cel_Error* cel_nonnull error) {
  CEL_ASSERT_NOT_NULL(error);

  error->payloads_len = 0;
  error->payloads_head = cel_nullptr;
  error->payloads_tail = cel_nullptr;
}
