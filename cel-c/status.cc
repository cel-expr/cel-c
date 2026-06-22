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

#include "cel-c/status.h"

#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/error_space.h"
#include "cel-c/internal/array.h"
#include "cel-c/internal/string.h"
#include "cel-c/status_code.h"
#include "cel-c/string_view.h"

#undef cel_OutOfMemoryStatus
#undef cel_Status_SetMessage
#undef cel_Status_VFormatMessage
#undef cel_Status_FormatMessage
#undef cel_Status_Set
#undef cel_Status_SetPayload

typedef struct {
  _cel_String type_url;
  _cel_String value;
} _cel_StatusRepPayload;

typedef union {
  CEL_ATTRIBUTE_MAYBE_UNUSED cel_Status pub;
  struct {
    // code **must** be first.
    unsigned int code;

    int raw_code;
    CEL_NONNULL(const cel_ErrorSpace*) space;
    _cel_String message;
    _cel_Array(_cel_StatusRepPayload) payloads;
    CEL_NULLABLE(const char*) file;
    int line;
  };
} _cel_StatusRep;

CEL_STATIC_ASSERT(sizeof(_cel_StatusRep) == sizeof(cel_Status));
CEL_STATIC_ASSERT(alignof(_cel_StatusRep) == alignof(cel_Status));
CEL_STATIC_ASSERT(offsetof(_cel_StatusRep, code) == offsetof(cel_Status, code));

CEL_ATTRIBUTE_NODISCARD
static CEL_NONNULL(const _cel_StatusRep*)
    _cel_Status_ConstRep(CEL_NONNULL(const cel_Status*) status) {
  return (const _cel_StatusRep*)status;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_NONNULL(_cel_StatusRep*)
    _cel_Status_MutableRep(CEL_NONNULL(cel_Status*) status) {
  return (_cel_StatusRep*)status;
}

#define _cel_Status_Rep(status)                 \
  (_Generic((status),                           \
       const cel_Status*: _cel_Status_ConstRep, \
       cel_Status*: _cel_Status_MutableRep)((status)))

static void _cel_StatusRepPayload_Construct(CEL_NONNULL(_cel_StatusRepPayload*)
                                                payload) {
  _cel_String_Construct(&payload->type_url);
  _cel_String_Construct(&payload->value);
}

static void _cel_StatusRepPayload_Destruct(CEL_NONNULL(_cel_StatusRepPayload*)
                                               payload) {
  _cel_String_Destruct(&payload->value, cel_DefaultAllocator);
  _cel_String_Destruct(&payload->type_url, cel_DefaultAllocator);
}

static void _cel_StatusRep_ResetMessage(CEL_NONNULL(_cel_StatusRep*)
                                            status_rep) {
  _cel_String_Reset(&status_rep->message, cel_DefaultAllocator);
}

static void _cel_StatusRep_ResetPayloads(CEL_NONNULL(_cel_StatusRep*)
                                             status_rep) {
  const size_t payloads_size = _cel_Array_Size(&status_rep->payloads);
  for (size_t i = 0; i < payloads_size; ++i) {
    _cel_StatusRepPayload_Destruct(
        _cel_Array_MutableAt(&status_rep->payloads, i));
  }
  _cel_Array_Reset(&status_rep->payloads, cel_DefaultAllocator);
}

CEL_ATTRIBUTE_NODISCARD
static unsigned int _cel_StatusRep_EncodeCanonical(cel_StatusCode code,
                                                   bool oom) {
  return (((unsigned int)code) << 1) | (oom ? 1u : 0u);
}

extern "C" void cel_Status_Construct(CEL_NONNULL(cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(status);

  CEL_NONNULL(_cel_StatusRep*) status_rep = _cel_Status_Rep(status);
  memset(status_rep, '\0', sizeof(*status_rep));
  status_rep->space = cel_CanonicalErrorSpace;
  _cel_String_Construct(&status_rep->message);
  _cel_Array_Construct(&status_rep->payloads);
}

extern "C" void cel_Status_Destruct(CEL_NONNULL(cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(status);

  CEL_NONNULL(_cel_StatusRep*) status_rep = _cel_Status_Rep(status);
  const size_t payloads_size = _cel_Array_Size(&status_rep->payloads);
  for (size_t i = 0; i < payloads_size; ++i) {
    _cel_StatusRepPayload_Destruct(
        _cel_Array_MutableAt(&status_rep->payloads, i));
  }
  _cel_Array_Destruct(&status_rep->payloads, cel_DefaultAllocator);
  _cel_String_Destruct(&status_rep->message, cel_DefaultAllocator);
}

extern "C" void cel_Status_Clear(CEL_NONNULL(cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(status);

  CEL_NONNULL(_cel_StatusRep*) status_rep = _cel_Status_Rep(status);
  const size_t payloads_size = _cel_Array_Size(&status_rep->payloads);
  for (size_t i = 0; i < payloads_size; ++i) {
    _cel_StatusRepPayload_Destruct(
        _cel_Array_MutableAt(&status_rep->payloads, i));
  }
  _cel_Array_Clear(&status_rep->payloads);
  _cel_String_Clear(&status_rep->message);
  status_rep->code =
      _cel_StatusRep_EncodeCanonical(cel_StatusCode_kOk, /*oom=*/false);
  status_rep->space = cel_CanonicalErrorSpace;
  status_rep->raw_code = cel_StatusCode_kOk;
  status_rep->file = cel_nullptr;
  status_rep->line = 0;
}

extern "C" void cel_Status_Reset(CEL_NONNULL(cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(status);

  CEL_NONNULL(_cel_StatusRep*) status_rep = _cel_Status_Rep(status);
  _cel_StatusRep_ResetMessage(status_rep);
  _cel_StatusRep_ResetPayloads(status_rep);
  status_rep->code =
      _cel_StatusRep_EncodeCanonical(cel_StatusCode_kOk, /*oom=*/false);
  status_rep->space = cel_CanonicalErrorSpace;
  status_rep->raw_code = cel_StatusCode_kOk;
  status_rep->file = cel_nullptr;
  status_rep->line = 0;
}

extern "C" CEL_NONNULL(const cel_ErrorSpace*)
    cel_Status_Space(CEL_NONNULL(const cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(status);

  return _cel_Status_Rep(status)->space;
}

extern "C" int cel_Status_Code(CEL_NONNULL(const cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(status);

  return _cel_Status_Rep(status)->raw_code;
}

extern "C" cel_StringView cel_Status_Message(CEL_NONNULL(const cel_Status*)
                                                 status) {
  CEL_ASSERT_NOT_NULL(status);

  return _cel_String_ToStringView(&_cel_Status_Rep(status)->message);
}

extern "C" void cel_Status_SetCode(CEL_NONNULL(cel_Status*) status,
                                   CEL_NONNULL(const cel_ErrorSpace*) space,
                                   int code) {
  CEL_ASSERT_NOT_NULL(status);
  CEL_ASSERT_NOT_NULL(space);

  cel_StatusCode canonical_code = cel_ErrorSpace_Canonical(space, code);
  bool oom;
  if (canonical_code == cel_StatusCode_kOk) {
    cel_Status_Reset(status);
    oom = false;
  } else {
    oom = cel_ErrorSpace_OutOfMemory(space, code);
  }
  CEL_NONNULL(_cel_StatusRep*) status_rep = _cel_Status_Rep(status);
  status_rep->code = _cel_StatusRep_EncodeCanonical(canonical_code, oom);
  status_rep->space = space;
  status_rep->raw_code = code;
}

extern "C" bool cel_Status_SetMessage(CEL_NONNULL(cel_Status*) status,
                                      CEL_NONNULL(const char*) file, int line,
                                      cel_StringView message) {
  CEL_ASSERT_NOT_NULL(status);
  CEL_ASSERT_NOT_NULL(file);
  CEL_ASSERT_GT(line, 0);

  if (cel_StringView_Empty(message) || status->code == cel_StatusCode_kOk) {
    return true;
  }

  if (!_cel_String_Assign(&_cel_Status_Rep(status)->message,
                          cel_DefaultAllocator, message)) {
    cel_OutOfMemoryStatus(status, file, line);
    return false;
  }

  return true;
}

extern "C" void cel_OutOfMemoryStatus(CEL_NONNULL(cel_Status*) status,
                                      CEL_NONNULL(const char*) file, int line) {
  CEL_ASSERT_NOT_NULL(status);
  CEL_ASSERT_NOT_NULL(file);
  CEL_ASSERT_GT(line, 0);

  CEL_NONNULL(_cel_StatusRep*) status_rep = _cel_Status_Rep(status);
  _cel_StatusRep_ResetMessage(status_rep);
  _cel_StatusRep_ResetPayloads(status_rep);
  status_rep->code = _cel_StatusRep_EncodeCanonical(
      cel_StatusCode_kResourceExhausted, /*oom=*/true);
  status_rep->space = cel_CanonicalErrorSpace;
  status_rep->file = file;
  status_rep->line = line;
  status_rep->raw_code = cel_StatusCode_kResourceExhausted;
}

extern "C" bool cel_Status_VFormatMessage(CEL_NONNULL(cel_Status*) status,
                                          CEL_NONNULL(const char*) file,
                                          int line, const char* cel_nonnull fmt,
                                          va_list args) {
  CEL_ASSERT_NOT_NULL(status);
  CEL_ASSERT_NOT_NULL(file);
  CEL_ASSERT_GT(line, 0);
  CEL_ASSERT_NOT_NULL(fmt);

  CEL_NONNULL(_cel_StatusRep*) status_rep = _cel_Status_Rep(status);
  _cel_String_Clear(&status_rep->message);
  if (!_cel_String_VAppendF(&status_rep->message, cel_DefaultAllocator, fmt,
                            args)) {
    cel_OutOfMemoryStatus(status, file, line);
    return false;
  }
  status_rep->file = file;
  status_rep->line = line;
  return true;
}

extern "C" bool cel_Status_GetPayload(CEL_NONNULL(const cel_Status*) status,
                                      cel_StringView type_url,
                                      CEL_NULLABLE(cel_StringView*) value) {
  CEL_ASSERT_NOT_NULL(status);

  if (cel_Status_Ok(status) || cel_StringView_Empty(type_url)) {
    return false;
  }

  CEL_NONNULL(const _cel_StatusRep*) status_rep = _cel_Status_Rep(status);
  const size_t payloads_len = _cel_Array_Size(&status_rep->payloads);
  for (size_t i = 0; i < payloads_len; ++i) {
    CEL_NONNULL(const _cel_StatusRepPayload*)
    payload_at = _cel_Array_At(&status_rep->payloads, i);
    if (cel_StringView_Equals(_cel_String_ToStringView(&payload_at->type_url),
                              type_url)) {
      if (value) {
        *value = _cel_String_ToStringView(&payload_at->value);
      }
      return true;
    }
  }

  return false;
}

extern "C" bool cel_Status_SetPayload(CEL_NONNULL(cel_Status*) status,
                                      cel_StringView type_url,
                                      cel_StringView value,
                                      CEL_NONNULL(const char*) file, int line) {
  CEL_ASSERT_NOT_NULL(status);
  CEL_ASSERT_NOT_NULL(file);
  CEL_ASSERT_GT(line, 0);

  if (cel_Status_Ok(status) || cel_StringView_Empty(type_url)) {
    return true;
  }

  CEL_NONNULL(_cel_StatusRep*) status_rep = _cel_Status_Rep(status);
  const size_t payloads_len = _cel_Array_Size(&status_rep->payloads);
  for (size_t i = 0; i < payloads_len; ++i) {
    CEL_NONNULL(_cel_StatusRepPayload*)
    payload_at = _cel_Array_MutableAt(&status_rep->payloads, i);
    if (cel_StringView_Equals(_cel_String_ToStringView(&payload_at->type_url),
                              type_url)) {
      if (!_cel_String_Assign(&payload_at->value, cel_DefaultAllocator,
                              value)) {
        cel_OutOfMemoryStatus(status, file, line);
        return false;
      }
      return true;
    }
  }

  CEL_NULLABLE(_cel_StatusRepPayload*)
  payload = _cel_Array_Push(&status_rep->payloads, cel_DefaultAllocator);
  if (payload == cel_nullptr) {
    cel_OutOfMemoryStatus(status, file, line);
    return false;
  }

  _cel_StatusRepPayload_Construct(payload);

  if (!_cel_String_Assign(&payload->type_url, cel_DefaultAllocator, type_url)) {
    cel_OutOfMemoryStatus(status, file, line);
    return false;
  }

  if (!_cel_String_Assign(&payload->value, cel_DefaultAllocator, value)) {
    cel_OutOfMemoryStatus(status, file, line);
    return false;
  }

  return true;
}

extern "C" bool cel_Status_DeletePayload(CEL_NONNULL(cel_Status*) status,
                                         cel_StringView type_url) {
  CEL_ASSERT_NOT_NULL(status);

  if (cel_Status_Ok(status) || cel_StringView_Empty(type_url)) {
    return false;
  }

  CEL_NONNULL(_cel_StatusRep*) status_rep = _cel_Status_Rep(status);
  const size_t payloads_len = _cel_Array_Size(&status_rep->payloads);
  for (size_t i = 0; i < payloads_len; ++i) {
    CEL_NONNULL(_cel_StatusRepPayload*)
    payload_at = _cel_Array_MutableAt(&status_rep->payloads, i);
    if (cel_StringView_Equals(_cel_String_ToStringView(&payload_at->type_url),
                              type_url)) {
      _cel_StatusRepPayload_Destruct(payload_at);
      _cel_Array_Erase(&status_rep->payloads, i);
      return true;
    }
  }

  return false;
}

extern "C" bool cel_Status_NextPayload(CEL_NONNULL(const cel_Status*) status,
                                       CEL_NULLABLE(cel_StringView*) type_url,
                                       CEL_NULLABLE(cel_StringView*) value,
                                       CEL_NONNULL(cel_StatusPayloadIterator*)
                                           iter) {
  CEL_ASSERT_NOT_NULL(status);
  CEL_ASSERT_NE(type_url, value);
  CEL_ASSERT_NOT_NULL(iter);
  CEL_ASSERT(iter->rep < cel_Status_Payloads(status) || iter->rep == SIZE_MAX);

  if (cel_Status_Ok(status)) {
    return false;
  }

  CEL_NONNULL(const _cel_StatusRep*) status_rep = _cel_Status_Rep(status);
  size_t idx = ++iter->rep;
  if (idx >= _cel_Array_Size(&status_rep->payloads)) {
    return false;
  }
  CEL_NONNULL(const _cel_StatusRepPayload*)
  payload = _cel_Array_At(&status_rep->payloads, idx);
  if (type_url != cel_nullptr) {
    *type_url = _cel_String_ToStringView(&payload->type_url);
  }
  if (value != cel_nullptr) {
    *value = _cel_String_ToStringView(&payload->value);
  }
  iter->rep = idx;
  return true;
}

extern "C" size_t cel_Status_Payloads(CEL_NONNULL(const cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(status);

  return cel_Status_Ok(status)
             ? 0
             : _cel_Array_Size(&_cel_Status_Rep(status)->payloads);
}

extern "C" void cel_Status_ClearPayloads(CEL_NONNULL(cel_Status*) status) {
  CEL_ASSERT_NOT_NULL(status);

  _cel_Array_Clear(&_cel_Status_Rep(status)->payloads);
}
