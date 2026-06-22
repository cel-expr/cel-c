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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_MAP_VALUE_H_
#define THIRD_PARTY_CEL_C_INTERNAL_MAP_VALUE_H_

#include <stdint.h>

#include "cel-c/hash.h"
#include "cel-c/internal/config.h"
#include "cel-c/string_view.h"
#include "cel-c/value.h"
#include "upb/base/descriptor_constants.h"
#include "upb/message/array.h"

CEL_BEGIN_DECLS

CEL_ATTRIBUTE_NODISCARD
static inline int _cel_MapValueKey_Compare(
    const cel_MapValueKey* cel_nonnull lhs,
    const cel_MapValueKey* cel_nonnull rhs) {
  switch (cel_MapValueKey_Kind(lhs)) {
    case cel_MapValueKeyKind_kBool:
      switch (cel_MapValueKey_Kind(rhs)) {
        case cel_MapValueKeyKind_kBool: {
          const bool lhs_value = cel_MapValueKey_GetBool(lhs);
          const bool rhs_value = cel_MapValueKey_GetBool(rhs);
          return lhs_value < rhs_value ? -1 : lhs_value > rhs_value ? 1 : 0;
        }
        case cel_MapValueKeyKind_kInt:
          CEL_ATTRIBUTE_FALLTHROUGH;
        case cel_MapValueKeyKind_kUint:
          CEL_ATTRIBUTE_FALLTHROUGH;
        case cel_MapValueKeyKind_kString:
          return -1;
        default:
          CEL_UNREACHABLE();
      }
    case cel_MapValueKeyKind_kInt:
      switch (cel_MapValueKey_Kind(rhs)) {
        case cel_MapValueKeyKind_kBool:
          return 1;
        case cel_MapValueKeyKind_kInt: {
          const int64_t lhs_value = cel_MapValueKey_GetInt(lhs);
          const int64_t rhs_value = cel_MapValueKey_GetInt(rhs);
          return lhs_value < rhs_value ? -1 : lhs_value > rhs_value ? 1 : 0;
        }
        case cel_MapValueKeyKind_kUint: {
          const int64_t lhs_value = cel_MapValueKey_GetInt(lhs);
          const uint64_t rhs_value = cel_MapValueKey_GetUint(rhs);
          if (lhs_value < 0) {
            return -1;
          }
          return ((uint64_t)lhs_value < rhs_value)   ? -1
                 : ((uint64_t)lhs_value > rhs_value) ? 1
                                                     : 0;
        }
        case cel_MapValueKeyKind_kString:
          return -1;
        default:
          CEL_UNREACHABLE();
      }
    case cel_MapValueKeyKind_kUint:
      switch (cel_MapValueKey_Kind(rhs)) {
        case cel_MapValueKeyKind_kBool:
          return 1;
        case cel_MapValueKeyKind_kInt: {
          const uint64_t lhs_value = cel_MapValueKey_GetUint(lhs);
          const int64_t rhs_value = cel_MapValueKey_GetInt(rhs);
          if (rhs_value < 0) {
            return 1;
          }
          return (lhs_value < (uint64_t)rhs_value)   ? -1
                 : (lhs_value > (uint64_t)rhs_value) ? 1
                                                     : 0;
        }
        case cel_MapValueKeyKind_kUint: {
          const uint64_t lhs_value = cel_MapValueKey_GetUint(lhs);
          const uint64_t rhs_value = cel_MapValueKey_GetUint(rhs);
          return lhs_value < rhs_value ? -1 : lhs_value > rhs_value ? 1 : 0;
        }
        case cel_MapValueKeyKind_kString:
          return -1;
        default:
          CEL_UNREACHABLE();
      }
    case cel_MapValueKeyKind_kString:
      switch (cel_MapValueKey_Kind(rhs)) {
        case cel_MapValueKeyKind_kBool:
          CEL_ATTRIBUTE_FALLTHROUGH;
        case cel_MapValueKeyKind_kInt:
          CEL_ATTRIBUTE_FALLTHROUGH;
        case cel_MapValueKeyKind_kUint:
          return 1;
        case cel_MapValueKeyKind_kString:
          return cel_StringView_Compare(cel_MapValueKey_GetString(lhs),
                                        cel_MapValueKey_GetString(rhs));
        default:
          CEL_UNREACHABLE();
      }
    default:
      CEL_UNREACHABLE();
  }
}

CEL_ATTRIBUTE_NODISCARD
static inline bool _cel_MapValueKey_Equals(
    const cel_MapValueKey* cel_nonnull lhs,
    const cel_MapValueKey* cel_nonnull rhs) {
  switch (cel_MapValueKey_Kind(lhs)) {
    case cel_MapValueKeyKind_kBool:
      switch (cel_MapValueKey_Kind(rhs)) {
        case cel_MapValueKeyKind_kBool: {
          const bool lhs_value = cel_MapValueKey_GetBool(lhs);
          const bool rhs_value = cel_MapValueKey_GetBool(rhs);
          return lhs_value == rhs_value;
        }
        case cel_MapValueKeyKind_kInt:
          CEL_ATTRIBUTE_FALLTHROUGH;
        case cel_MapValueKeyKind_kUint:
          CEL_ATTRIBUTE_FALLTHROUGH;
        case cel_MapValueKeyKind_kString:
          return false;
        default:
          CEL_UNREACHABLE();
      }
    case cel_MapValueKeyKind_kInt:
      switch (cel_MapValueKey_Kind(rhs)) {
        case cel_MapValueKeyKind_kBool:
          return false;
        case cel_MapValueKeyKind_kInt: {
          const int64_t lhs_value = cel_MapValueKey_GetInt(lhs);
          const int64_t rhs_value = cel_MapValueKey_GetInt(rhs);
          return lhs_value == rhs_value;
        }
        case cel_MapValueKeyKind_kUint: {
          const int64_t lhs_value = cel_MapValueKey_GetInt(lhs);
          const uint64_t rhs_value = cel_MapValueKey_GetUint(rhs);
          return lhs_value >= 0 && (uint64_t)lhs_value == rhs_value;
        }
        case cel_MapValueKeyKind_kString:
          return false;
        default:
          CEL_UNREACHABLE();
      }
    case cel_MapValueKeyKind_kUint:
      switch (cel_MapValueKey_Kind(rhs)) {
        case cel_MapValueKeyKind_kBool:
          return false;
        case cel_MapValueKeyKind_kInt: {
          const uint64_t lhs_value = cel_MapValueKey_GetUint(lhs);
          const int64_t rhs_value = cel_MapValueKey_GetInt(rhs);
          return rhs_value >= 0 && (uint64_t)rhs_value == lhs_value;
        }
        case cel_MapValueKeyKind_kUint: {
          const uint64_t lhs_value = cel_MapValueKey_GetUint(lhs);
          const uint64_t rhs_value = cel_MapValueKey_GetUint(rhs);
          return lhs_value == rhs_value;
        }
        case cel_MapValueKeyKind_kString:
          return false;
        default:
          CEL_UNREACHABLE();
      }
    case cel_MapValueKeyKind_kString:
      switch (cel_MapValueKey_Kind(rhs)) {
        case cel_MapValueKeyKind_kBool:
          CEL_ATTRIBUTE_FALLTHROUGH;
        case cel_MapValueKeyKind_kInt:
          CEL_ATTRIBUTE_FALLTHROUGH;
        case cel_MapValueKeyKind_kUint:
          return false;
        case cel_MapValueKeyKind_kString:
          return cel_StringView_Equals(cel_MapValueKey_GetString(lhs),
                                       cel_MapValueKey_GetString(rhs));
        default:
          CEL_UNREACHABLE();
      }
    default:
      CEL_UNREACHABLE();
  }
}

CEL_ATTRIBUTE_NODISCARD
static inline cel_HashState _cel_MapValueKey_Hash(
    const cel_MapValueKey* cel_nonnull map_value_key, cel_HashState state) {
  switch (cel_MapValueKey_Kind(map_value_key)) {
    case cel_MapValueKeyKind_kBool:
      state = cel_HashState_Combine(state, cel_MapValueKeyKind_kBool);
      state =
          cel_HashState_Combine(state, cel_MapValueKey_GetBool(map_value_key));
      break;
    case cel_MapValueKeyKind_kInt: {
      const int64_t value = cel_MapValueKey_GetInt(map_value_key);
      if (value < 0) {
        state = cel_HashState_Combine(state, cel_MapValueKeyKind_kInt);
        state = cel_HashState_Combine(state, value);
      } else {
        state = cel_HashState_Combine(state, cel_MapValueKeyKind_kUint);
        state = cel_HashState_Combine(state, (uint64_t)value);
      }
    } break;
    case cel_MapValueKeyKind_kUint:
      state = cel_HashState_Combine(state, cel_MapValueKeyKind_kUint);
      state =
          cel_HashState_Combine(state, cel_MapValueKey_GetUint(map_value_key));
      break;
    case cel_MapValueKeyKind_kString:
      state = cel_HashState_Combine(state, cel_MapValueKeyKind_kString);
      state =
          cel_StringView_Hash(cel_MapValueKey_GetString(map_value_key), state);
      break;
    default:
      CEL_UNREACHABLE();
  }
  return state;
}

static inline void _cel_Value_SetMapValueKey(cel_Value* cel_nonnull value,
                                             const cel_MapValueKey* cel_nonnull
                                                 map_value_key) {
  switch (cel_MapValueKey_Kind(map_value_key)) {
    case cel_MapValueKeyKind_kBool:
      cel_Value_SetBool(value, cel_MapValueKey_GetBool(map_value_key));
      break;
    case cel_MapValueKeyKind_kInt:
      cel_Value_SetInt(value, cel_MapValueKey_GetInt(map_value_key));
      break;
    case cel_MapValueKeyKind_kUint:
      cel_Value_SetUint(value, cel_MapValueKey_GetUint(map_value_key));
      break;
    case cel_MapValueKeyKind_kString:
      cel_Value_SetString(value, cel_MapValueKey_GetString(map_value_key));
      break;
    default:
      CEL_UNREACHABLE();
  }
}

CEL_ATTRIBUTE_NODISCARD
static inline bool _cel_MapValueKey_ToMessageValue(
    const cel_MapValueKey* cel_nonnull key, upb_CType field_key_type,
    upb_MessageValue* cel_nonnull message_key) {
  switch (field_key_type) {
    case kUpb_CType_Int32:
      switch (cel_MapValueKey_Kind(key)) {
        case cel_MapValueKeyKind_kInt: {
          const int64_t val = cel_MapValueKey_GetInt(key);
          if (val < INT32_MIN || val > INT32_MAX) {
            return false;
          }
          message_key->int32_val = (int32_t)val;
        } break;
        case cel_MapValueKeyKind_kUint: {
          const uint64_t val = cel_MapValueKey_GetUint(key);
          if (val > (uint32_t)INT32_MAX) {
            return false;
          }
          message_key->int32_val = (int32_t)(uint32_t)val;
        } break;
        default:
          return false;
      }
      break;
    case kUpb_CType_UInt32:
      switch (cel_MapValueKey_Kind(key)) {
        case cel_MapValueKeyKind_kInt: {
          const int64_t val = cel_MapValueKey_GetInt(key);
          if (val < 0 || val > (int64_t)(uint64_t)UINT32_MAX) {
            return false;
          }
          message_key->uint32_val = (uint32_t)(int32_t)val;
        } break;
        case cel_MapValueKeyKind_kUint: {
          const uint64_t val = cel_MapValueKey_GetUint(key);
          if (val > UINT32_MAX) {
            return false;
          }
          message_key->uint32_val = (uint32_t)val;
        } break;
        default:
          return false;
      }
      break;
    case kUpb_CType_Int64:
      switch (cel_MapValueKey_Kind(key)) {
        case cel_MapValueKeyKind_kInt:
          message_key->int64_val = cel_MapValueKey_GetInt(key);
          break;
        case cel_MapValueKeyKind_kUint: {
          const uint64_t val = cel_MapValueKey_GetUint(key);
          if (val > (uint64_t)INT64_MAX) {
            return false;
          }
          message_key->int64_val = (int64_t)val;
        } break;
        default:
          return false;
      }
      break;
    case kUpb_CType_UInt64:
      switch (cel_MapValueKey_Kind(key)) {
        case cel_MapValueKeyKind_kInt: {
          const int64_t val = cel_MapValueKey_GetInt(key);
          if (val < 0) {
            return false;
          }
          message_key->uint64_val = (uint64_t)val;
        } break;
        case cel_MapValueKeyKind_kUint:
          message_key->uint64_val = cel_MapValueKey_GetUint(key);
          break;
        default:
          return false;
      }
      break;
    case kUpb_CType_Bool:
      switch (cel_MapValueKey_Kind(key)) {
        case cel_MapValueKeyKind_kBool: {
          message_key->bool_val = cel_MapValueKey_GetBool(key);
        } break;
        default:
          return false;
      }
      break;
    case kUpb_CType_String:
      switch (cel_MapValueKey_Kind(key)) {
        case cel_MapValueKeyKind_kString: {
          message_key->str_val = cel_MapValueKey_GetString(key);
        } break;
        default:
          return false;
      }
      break;
    default:
      return false;
  }
  return true;
}

static inline void _cel_MapValueKey_FromMessageValue(
    cel_MapValueKey* cel_nonnull key, upb_CType field_key_type,
    upb_MessageValue message_key) {
  switch (field_key_type) {
    case kUpb_CType_Int32:
      cel_MapValueKey_SetInt(key, message_key.int32_val);
      break;
    case kUpb_CType_UInt32:
      cel_MapValueKey_SetUint(key, message_key.uint32_val);
      break;
    case kUpb_CType_Int64:
      cel_MapValueKey_SetInt(key, message_key.int64_val);
      break;
    case kUpb_CType_UInt64:
      cel_MapValueKey_SetUint(key, message_key.uint64_val);
      break;
    case kUpb_CType_Bool:
      cel_MapValueKey_SetBool(key, message_key.bool_val);
      break;
    case kUpb_CType_String:
      cel_MapValueKey_SetString(key, message_key.str_val);
      break;
    default:
      CEL_UNREACHABLE();
  }
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_MAP_VALUE_H_
