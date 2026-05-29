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

#include <stdalign.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"
#include "cel-c/value.h"
#include "cel-c/value_kind.h"

static const cel_OptionalValueVTable _cel_OptionalValueVTables[11];

#define _cel_EmptyOptionalValueVTable _cel_OptionalValueVTables[0]
#define _cel_NullOptionalValueVTable _cel_OptionalValueVTables[1]
#define _cel_BoolOptionalValueVTable _cel_OptionalValueVTables[2]
#define _cel_IntOptionalValueVTable _cel_OptionalValueVTables[3]
#define _cel_UintOptionalValueVTable _cel_OptionalValueVTables[4]
#define _cel_DoubleOptionalValueVTable _cel_OptionalValueVTables[5]
#define _cel_BytesOptionalValueVTable _cel_OptionalValueVTables[6]
#define _cel_StringOptionalValueVTable _cel_OptionalValueVTables[7]
#define _cel_DurationOptionalValueVTable _cel_OptionalValueVTables[8]
#define _cel_TimestampOptionalValueVTable _cel_OptionalValueVTables[9]
#define _cel_OptionalValueVTable _cel_OptionalValueVTables[10]

static void _cel_EmptyOptionalValue_Set(
    cel_OptionalValue* cel_nonnull optional_value) {
  CEL_ASSERT_NOT_NULL(optional_value);

  optional_value->vtable = &_cel_EmptyOptionalValueVTable;
}

static void _cel_NullOptionalValue_Set(
    cel_OptionalValue* cel_nonnull optional_value) {
  CEL_ASSERT_NOT_NULL(optional_value);

  optional_value->vtable = &_cel_NullOptionalValueVTable;
}

static void _cel_BoolOptionalValue_Set(
    cel_OptionalValue* cel_nonnull optional_value, bool value) {
  CEL_ASSERT_NOT_NULL(optional_value);

  optional_value->vtable = &_cel_BoolOptionalValueVTable;
  optional_value->content.b[0] = value;
}

static void _cel_IntOptionalValue_Set(
    cel_OptionalValue* cel_nonnull optional_value, int64_t value) {
  CEL_ASSERT_NOT_NULL(optional_value);

  optional_value->vtable = &_cel_IntOptionalValueVTable;
  optional_value->content.i64[0] = value;
}

static void _cel_UintOptionalValue_Set(
    cel_OptionalValue* cel_nonnull optional_value, uint64_t value) {
  CEL_ASSERT_NOT_NULL(optional_value);

  optional_value->vtable = &_cel_UintOptionalValueVTable;
  optional_value->content.u64[0] = value;
}

static void _cel_DoubleOptionalValue_Set(
    cel_OptionalValue* cel_nonnull optional_value, double value) {
  CEL_ASSERT_NOT_NULL(optional_value);

  optional_value->vtable = &_cel_DoubleOptionalValueVTable;
  optional_value->content.d[0] = value;
}

static void _cel_BytesOptionalValue_Set(
    cel_OptionalValue* cel_nonnull optional_value, cel_StringView value) {
  CEL_ASSERT_NOT_NULL(optional_value);

  optional_value->vtable = &_cel_BytesOptionalValueVTable;
  optional_value->content.str.data = value.data;
  optional_value->content.str.size = value.size;
}

static void _cel_StringOptionalValue_Set(
    cel_OptionalValue* cel_nonnull optional_value, cel_StringView value) {
  CEL_ASSERT_NOT_NULL(optional_value);

  optional_value->vtable = &_cel_StringOptionalValueVTable;
  optional_value->content.str.data = value.data;
  optional_value->content.str.size = value.size;
}

static void _cel_DurationOptionalValue_Set(
    cel_OptionalValue* cel_nonnull optional_value, cel_Duration value) {
  CEL_ASSERT_NOT_NULL(optional_value);

  optional_value->vtable = &_cel_DurationOptionalValueVTable;
  CEL_STATIC_ASSERT(sizeof(cel_Duration) <= sizeof(cel_ValueContent));
  CEL_STATIC_ASSERT(alignof(cel_Duration) <= alignof(cel_ValueContent));
  memcpy(&optional_value->content.raw, &value, sizeof(value));
}

static void _cel_TimestampOptionalValue_Set(
    cel_OptionalValue* cel_nonnull optional_value, cel_Timestamp value) {
  CEL_ASSERT_NOT_NULL(optional_value);

  optional_value->vtable = &_cel_TimestampOptionalValueVTable;
  CEL_STATIC_ASSERT(sizeof(cel_Timestamp) <= sizeof(cel_ValueContent));
  CEL_STATIC_ASSERT(alignof(cel_Timestamp) <= alignof(cel_ValueContent));
  memcpy(&optional_value->content.raw, &value, sizeof(value));
}

static void _cel_OptionalValue_Set(cel_OptionalValue* cel_nonnull
                                       optional_value,
                                   const cel_Value* cel_nonnull value) {
  CEL_ASSERT_NOT_NULL(optional_value);
  CEL_ASSERT_NOT(cel_Value_IsNull(value));
  CEL_ASSERT_NOT(cel_Value_IsBool(value));
  CEL_ASSERT_NOT(cel_Value_IsInt(value));
  CEL_ASSERT_NOT(cel_Value_IsUint(value));
  CEL_ASSERT_NOT(cel_Value_IsDouble(value));
  CEL_ASSERT_NOT(cel_Value_IsBytes(value));
  CEL_ASSERT_NOT(cel_Value_IsString(value));
  CEL_ASSERT_NOT(cel_Value_IsDuration(value));
  CEL_ASSERT_NOT(cel_Value_IsTimestamp(value));
  CEL_ASSERT_NOT(cel_Value_IsError(value));

  optional_value->vtable = &_cel_OptionalValueVTable;
  optional_value->content.ptr[0] = (cel_Value*)value;
}

CEL_ATTRIBUTE_NODISCARD
static bool _cel_OpaqueValue_IsOptional(const cel_OpaqueValue* opaque_value) {
  return opaque_value->vtable >=
             ((const cel_OpaqueValueVTable*)_cel_OptionalValueVTables) &&
         opaque_value->vtable <=
             ((const cel_OpaqueValueVTable*)(_cel_OptionalValueVTables +
                                             cel_arraysize(
                                                 _cel_OptionalValueVTables)));
}

static bool _cel_OptionalValue_Equals(
    const cel_OpaqueValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_OpaqueValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(vtable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(other);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  if (!_cel_OpaqueValue_IsOptional(other)) {
    // The only way to get here would be if somebody tried to mimic the optional
    // value implementation, as the type name would have to be `optional_type`.
    cel_Value_SetFalse(result);
    return true;
  }

  const bool has_value =
      vtable != (const cel_OpaqueValueVTable*)&_cel_EmptyOptionalValueVTable;
  const bool other_has_value =
      other->vtable !=
      (const cel_OpaqueValueVTable*)&_cel_EmptyOptionalValueVTable;
  if (has_value && other_has_value) {
    cel_Value value;
    cel_Value other_value;
    if (((const cel_OptionalValueVTable*)vtable)
            ->Value((const cel_OptionalValueVTable*)vtable, content, context,
                    &value, status) &&
        ((const cel_OptionalValueVTable*)other->vtable)
            ->Value((const cel_OptionalValueVTable*)other->vtable,
                    other->content, context, &other_value, status)) {
      return cel_Value_Equals(&value, context, &other_value, result, status);
    }
    CEL_ASSERT(!cel_Status_Ok(status));
    return false;
  }
  if (has_value != other_has_value) {
    // One empty.
    cel_Value_SetFalse(result);
    return true;
  }
  // Both empty.
  cel_Value_SetTrue(result);
  return true;
}

static cel_StringView _cel_OptionalValue_TypeName(
    const cel_OpaqueValueVTable* cel_nonnull vtable, cel_ValueContent content) {
  CEL_ASSERT_NOT_NULL(vtable);

  return cel_StringView_From("optional_type");
}

static bool _cel_EmptyOptionalValue_HasValue(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(vtable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Value_SetFalse(result);
  return true;
}

static bool _cel_EmptyOptionalValue_Value(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(vtable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Error* error = cel_Error_New(context->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    cel_OutOfMemoryStatus(status);
    return false;
  }
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kFailedPrecondition);
  cel_Error_SetMessage(error,
                       cel_StringView_From("optional.none() dereference"));
  cel_Value_SetError(value, error);
  return true;
}

static bool _cel_NullOptionalValue_Value(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(vtable);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Value_SetNull(value);
  return true;
}

static bool _cel_BoolOptionalValue_Value(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(vtable);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Value_SetBool(value, content.b[0]);
  return true;
}

static bool _cel_IntOptionalValue_Value(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(vtable);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Value_SetInt(value, content.i64[0]);
  return true;
}

static bool _cel_UintOptionalValue_Value(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(vtable);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Value_SetUint(value, content.u64[0]);
  return true;
}

static bool _cel_DoubleOptionalValue_Value(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(vtable);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Value_SetDouble(value, content.d[0]);
  return true;
}

static bool _cel_BytesOptionalValue_Value(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(vtable);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Value_SetBytes(
      value, cel_StringView_FromArray(content.str.data, content.str.size));
  return true;
}

static bool _cel_StringOptionalValue_Value(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(vtable);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Value_SetString(
      value, cel_StringView_FromArray(content.str.data, content.str.size));
  return true;
}

static bool _cel_DurationOptionalValue_Value(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(vtable);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  CEL_STATIC_ASSERT(sizeof(cel_Duration) <= sizeof(cel_ValueContent));
  CEL_STATIC_ASSERT(alignof(cel_Duration) <= alignof(cel_ValueContent));

  cel_Duration d;
  memcpy(&d, content.raw, sizeof(d));
  cel_Value_SetDuration(value, d);
  return true;
}

static bool _cel_TimestampOptionalValue_Value(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(vtable);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  CEL_STATIC_ASSERT(sizeof(cel_Timestamp) <= sizeof(cel_ValueContent));
  CEL_STATIC_ASSERT(alignof(cel_Timestamp) <= alignof(cel_ValueContent));
  cel_Timestamp t;
  memcpy(&t, content.raw, sizeof(t));
  cel_Value_SetTimestamp(value, t);
  return true;
}

static bool _cel_OptionalValue_HasValue(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(vtable);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  cel_Value_SetTrue(result);
  return true;
}

static bool _cel_OptionalValue_Value(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(vtable);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  if (CEL_UNLIKELY(!cel_Status_Ok(status))) {
    return false;
  }

  *value = *(const cel_Value*)content.ptr[0];
  return true;
}

static const cel_OptionalValueVTable _cel_OptionalValueVTables[11] = {
    // _cel_EmptyOptionalValueVTable
    {
        .super =
            {
                .Equals = &_cel_OptionalValue_Equals,
                .TypeName = &_cel_OptionalValue_TypeName,
            },
        .HasValue = &_cel_EmptyOptionalValue_HasValue,
        .Value = &_cel_EmptyOptionalValue_Value,
    },
    // _cel_NullOptionalValueVTable
    {
        .super =
            {
                .Equals = &_cel_OptionalValue_Equals,
                .TypeName = &_cel_OptionalValue_TypeName,
            },
        .HasValue = &_cel_OptionalValue_HasValue,
        .Value = &_cel_NullOptionalValue_Value,
    },
    // _cel_BoolOptionalValueVTable
    {
        .super =
            {
                .Equals = &_cel_OptionalValue_Equals,
                .TypeName = &_cel_OptionalValue_TypeName,
            },
        .HasValue = &_cel_OptionalValue_HasValue,
        .Value = &_cel_BoolOptionalValue_Value,
    },
    // _cel_IntOptionalValueVTable
    {
        .super =
            {
                .Equals = &_cel_OptionalValue_Equals,
                .TypeName = &_cel_OptionalValue_TypeName,
            },
        .HasValue = &_cel_OptionalValue_HasValue,
        .Value = &_cel_IntOptionalValue_Value,
    },
    // _cel_UintOptionalValueVTable
    {
        .super =
            {
                .Equals = &_cel_OptionalValue_Equals,
                .TypeName = &_cel_OptionalValue_TypeName,
            },
        .HasValue = &_cel_OptionalValue_HasValue,
        .Value = &_cel_UintOptionalValue_Value,
    },
    // _cel_DoubleOptionalValueVTable
    {
        .super =
            {
                .Equals = &_cel_OptionalValue_Equals,
                .TypeName = &_cel_OptionalValue_TypeName,
            },
        .HasValue = &_cel_OptionalValue_HasValue,
        .Value = &_cel_DoubleOptionalValue_Value,
    },
    // _cel_BytesOptionalValueVTable
    {
        .super =
            {
                .Equals = &_cel_OptionalValue_Equals,
                .TypeName = &_cel_OptionalValue_TypeName,
            },
        .HasValue = &_cel_OptionalValue_HasValue,
        .Value = &_cel_BytesOptionalValue_Value,
    },
    // _cel_StringOptionalValueVTable
    {
        .super =
            {
                .Equals = &_cel_OptionalValue_Equals,
                .TypeName = &_cel_OptionalValue_TypeName,
            },
        .HasValue = &_cel_OptionalValue_HasValue,
        .Value = &_cel_StringOptionalValue_Value,
    },
    // _cel_DurationOptionalValueVTable
    {
        .super =
            {
                .Equals = &_cel_OptionalValue_Equals,
                .TypeName = &_cel_OptionalValue_TypeName,
            },
        .HasValue = &_cel_OptionalValue_HasValue,
        .Value = &_cel_DurationOptionalValue_Value,
    },
    // _cel_TimestampOptionalValueVTable
    {
        .super =
            {
                .Equals = &_cel_OptionalValue_Equals,
                .TypeName = &_cel_OptionalValue_TypeName,
            },
        .HasValue = &_cel_OptionalValue_HasValue,
        .Value = &_cel_TimestampOptionalValue_Value,
    },
    // _cel_OptionalValueVTable
    {
        .super =
            {
                .Equals = &_cel_OptionalValue_Equals,
                .TypeName = &_cel_OptionalValue_TypeName,
            },
        .HasValue = &_cel_OptionalValue_HasValue,
        .Value = &_cel_OptionalValue_Value,
    },
};

cel_OptionalValue* cel_nonnull
cel_OptionalValue_Empty(cel_OptionalValue* cel_nonnull optional_value) {
  CEL_ASSERT_NOT_NULL(optional_value);

  _cel_EmptyOptionalValue_Set(optional_value);
  return optional_value;
}

bool cel_OptionalValue_Of(cel_OptionalValue* cel_nonnull optional_value,
                          const cel_Value* cel_nonnull value,
                          cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(optional_value);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(arena);

  switch (cel_Value_Kind(value)) {
    case cel_ValueKind_kNull:
      _cel_NullOptionalValue_Set(optional_value);
      return true;
    case cel_ValueKind_kBool:
      _cel_BoolOptionalValue_Set(optional_value, cel_Value_GetBool(value));
      return true;
    case cel_ValueKind_kInt:
      _cel_IntOptionalValue_Set(optional_value, cel_Value_GetInt(value));
      return true;
    case cel_ValueKind_kUint:
      _cel_UintOptionalValue_Set(optional_value, cel_Value_GetUint(value));
      return true;
    case cel_ValueKind_kDouble:
      _cel_DoubleOptionalValue_Set(optional_value, cel_Value_GetDouble(value));
      return true;
    case cel_ValueKind_kBytes:
      _cel_BytesOptionalValue_Set(optional_value, cel_Value_GetBytes(value));
      return true;
    case cel_ValueKind_kString:
      _cel_StringOptionalValue_Set(optional_value, cel_Value_GetString(value));
      return true;
    case cel_ValueKind_kDuration:
      _cel_DurationOptionalValue_Set(optional_value,
                                     cel_Value_GetDuration(value));
      return true;
    case cel_ValueKind_kTimestamp:
      _cel_TimestampOptionalValue_Set(optional_value,
                                      cel_Value_GetTimestamp(value));
      return true;
    default: {
      cel_Value* value_ptr =
          (cel_Value*)cel_Arena_Malloc(arena, sizeof(cel_Value), cel_nullptr);
      if (CEL_UNLIKELY(value_ptr == cel_nullptr)) {
        return false;
      }
      *value_ptr = *value;
      _cel_OptionalValue_Set(optional_value, value_ptr);
      return true;
    }
  }
}

bool cel_Value_IsOptional(const cel_Value* cel_nonnull value) {
  return cel_Value_IsOpaque(value) &&
         _cel_OpaqueValue_IsOptional(cel_Value_GetOpaque(value));
}

bool cel_OpaqueValue_IsOptional(
    const cel_OpaqueValue* cel_nonnull opaque_value) {
  return _cel_OpaqueValue_IsOptional(opaque_value);
}
