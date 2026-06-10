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

#include "cel-c/src/any.h"

#include <stdbool.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/status.h"
#include "cel-c/status_code.h"
#include "cel-c/string_view.h"
#include "cel-c/well_known_types.h"
#include "upb/mem/arena.h"
#include "upb/message/message.h"
#include "upb/mini_table/message.h"
#include "upb/reflection/def.h"
#include "upb/reflection/message.h"
#include "upb/wire/decode.h"

extern "C" _cel_AnyUnpackResult _cel_AnyUnpack(
    const upb_Message* cel_nonnull in_message,
    const upb_DefPool* cel_nonnull def_pool, const cel_AnyWellKnownType* wkt,
    upb_Arena* cel_nonnull arena,
    upb_Message * cel_nullable * cel_nonnull out_message,
    const upb_MessageDef * cel_nullable * cel_nonnull out_message_def) {
  CEL_ASSERT_NOT_NULL(in_message);
  CEL_ASSERT_NOT_NULL(def_pool);
  CEL_ASSERT_NOT_NULL(wkt);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_NOT_NULL(out_message);
  CEL_ASSERT_NOT_NULL(out_message_def);

  bool unpacked = false;

  while (true) {
    // We expect the type URL to be in the format `<hostname>/<type_name>`.
    cel_StringView type_url =
        upb_Message_GetFieldByDef(in_message, wkt->type_url_def).str_val;
    const char* type_url_data = cel_StringView_Data(type_url);
    const char* slash = reinterpret_cast<const char*>(
        memchr(type_url_data, '/', cel_StringView_Size(type_url)));
    if (slash == type_url_data || slash == cel_nullptr) {
      if (!unpacked) {
        *out_message = cel_nullptr;
        *out_message_def = cel_nullptr;
      }
      return _cel_AnyUnpackResult_kBadTypeUrl;
    }
    cel_StringView type_name = type_url;
    cel_StringView_RemovePrefix(&type_name, (slash - type_url_data) + 1);
    const upb_MessageDef* message_def = upb_DefPool_FindMessageByNameWithSize(
        def_pool, cel_StringView_Data(type_name),
        cel_StringView_Size(type_name));
    if (message_def == cel_nullptr) {
      if (!unpacked) {
        *out_message = cel_nullptr;
        *out_message_def = cel_nullptr;
      }
      return _cel_AnyUnpackResult_kDefNotFound;
    }
    const upb_MiniTable* message_tab = upb_MessageDef_MiniTable(message_def);
    upb_Message* message = upb_Message_New(message_tab, arena);
    if (message == cel_nullptr) {
      if (!unpacked) {
        *out_message = cel_nullptr;
        *out_message_def = cel_nullptr;
      }
      return _cel_AnyUnpackResult_kOutOfMemory;
    }
    cel_StringView value =
        upb_Message_GetFieldByDef(in_message, wkt->value_def).str_val;
    upb_DecodeStatus status = upb_Decode(
        cel_StringView_Data(value), cel_StringView_Size(value), message,
        message_tab, upb_DefPool_ExtensionRegistry(def_pool),
        kUpb_DecodeOption_AliasString, arena);
    if (status == kUpb_DecodeStatus_Ok) {
      *out_message = message;
      *out_message_def = message_def;
      if (upb_MessageDef_WellKnownType(message_def) == kUpb_WellKnown_Any) {
        CEL_ASSERT_EQ(message_def, wkt->def);
        in_message = message;
        unpacked = true;
        continue;
      }
      return _cel_AnyUnpackResult_kOk;
    }
    if (!unpacked) {
      *out_message = cel_nullptr;
      *out_message_def = cel_nullptr;
    }
    switch (status) {
      case kUpb_DecodeStatus_Ok:
        CEL_UNREACHABLE();
      case kUpb_DecodeStatus_Malformed:
        return _cel_AnyUnpackResult_kMalformed;
      case kUpb_DecodeStatus_OutOfMemory:
        return _cel_AnyUnpackResult_kOutOfMemory;
      case kUpb_DecodeStatus_BadUtf8:
        return _cel_AnyUnpackResult_kBadUtf8;
      case kUpb_DecodeStatus_MaxDepthExceeded:
        return _cel_AnyUnpackResult_kMaxDepthExceeded;
      default:
        return _cel_AnyUnpackResult_kUnknown;
    }
  }
}

extern "C" void _cel_AnyUnpackResult_ToStatus(_cel_AnyUnpackResult result,
                                              cel_Status* cel_nonnull status) {
  switch (result) {
    case _cel_AnyUnpackResult_kOk:
      cel_Status_Clear(status);
      return;
    case _cel_AnyUnpackResult_kOutOfMemory:
      cel_OutOfMemoryStatus(status);
      return;
    default:
      cel_CanonicalStatus(
          status, _cel_AnyUnpackResult_ToStatusCode(result),
          cel_StringView_FromString(_cel_AnyUnpackResult_ToMessage(result)));
      return;
  }
}

extern "C" cel_StatusCode _cel_AnyUnpackResult_ToStatusCode(
    _cel_AnyUnpackResult result) {
  switch (result) {
    case _cel_AnyUnpackResult_kOk:
      return cel_StatusCode_kOk;
    case _cel_AnyUnpackResult_kOutOfMemory:
      return cel_StatusCode_kResourceExhausted;
    case _cel_AnyUnpackResult_kBadTypeUrl:
      return cel_StatusCode_kInvalidArgument;
    case _cel_AnyUnpackResult_kDefNotFound:
      return cel_StatusCode_kNotFound;
    case _cel_AnyUnpackResult_kMalformed:
      return cel_StatusCode_kInvalidArgument;
    case _cel_AnyUnpackResult_kBadUtf8:
      return cel_StatusCode_kInvalidArgument;
    case _cel_AnyUnpackResult_kMaxDepthExceeded:
      return cel_StatusCode_kInvalidArgument;
    case _cel_AnyUnpackResult_kUnknown:
      return cel_StatusCode_kUnknown;
    default:
      CEL_UNREACHABLE();
  }
}

extern "C" const char* cel_nonnull
_cel_AnyUnpackResult_ToMessage(_cel_AnyUnpackResult result) {
  switch (result) {
    case _cel_AnyUnpackResult_kOk:
      return "OK";
    case _cel_AnyUnpackResult_kOutOfMemory:
      return "out of memory";
    case _cel_AnyUnpackResult_kBadTypeUrl:
      return "bad type URL";
    case _cel_AnyUnpackResult_kDefNotFound:
      return "descriptor not found";
    case _cel_AnyUnpackResult_kMalformed:
      return "malformed wire format";
    case _cel_AnyUnpackResult_kBadUtf8:
      return "bad UTF-8";
    case _cel_AnyUnpackResult_kMaxDepthExceeded:
      return "max depth exceeded";
    case _cel_AnyUnpackResult_kUnknown:
      return "unknown error";
    default:
      CEL_UNREACHABLE();
  }
}
