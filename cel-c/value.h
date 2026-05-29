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

#ifndef THIRD_PARTY_CEL_C_VALUE_H_
#define THIRD_PARTY_CEL_C_VALUE_H_

#include <stdalign.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>
#include <stdint.h>

#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"
#include "cel-c/error.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"
#include "cel-c/value_kind.h"
#include "cel-c/well_known_types.h"
#include "upb/message/array.h"
#include "upb/message/message.h"
#include "upb/reflection/def.h"

CEL_BEGIN_DECLS

typedef struct cel_Value cel_Value;

// cel_ValueContent

typedef union {
  alignas(8) char raw[16];
  bool b[16 / sizeof(bool)];
  uint64_t u64[2];
  int64_t i64[2];
  uint32_t u32[4];
  int32_t i32[4];
  uint16_t u16[8];
  int16_t i16[8];
  uint8_t u8[16];
  int8_t i8[16];
  uintptr_t uptr[16 / sizeof(uintptr_t)];
  intptr_t iptr[16 / sizeof(intptr_t)];
  float f[16 / sizeof(float)];
  double d[16 / sizeof(double)];
  const void* cel_nullability_unknown ptr[16 / sizeof(const void*)];
  struct {
    const char* cel_nullability_unknown data;
    size_t size;
  } str;
} cel_ValueContent;

CEL_STATIC_ASSERT(sizeof(cel_ValueContent) == 16);
CEL_STATIC_ASSERT(alignof(cel_ValueContent) == 8);

// cel_ValueContext

typedef struct {
  cel_Allocator* cel_nonnull alloc;
  cel_Arena* cel_nonnull arena;
  const upb_DefPool* cel_nonnull def_pool;
  const cel_WellKnownTypes* cel_nonnull well_known_types;
} cel_ValueContext;

// cel_ValueIterator

typedef struct cel_ValueIterator cel_ValueIterator;
typedef struct cel_ValueIteratorVTable cel_ValueIteratorVTable;

typedef void cel_ValueIteratorVTable_Delete(
    cel_ValueIterator* cel_nonnull iterator);
typedef bool cel_ValueIteratorVTable_Next1(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_Value* cel_nonnull key_or_value, cel_Status* cel_nonnull status);
typedef bool cel_ValueIteratorVTable_Next2(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull key,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status);
typedef bool cel_ValueIteratorVTable_Remaining(
    const cel_ValueIterator* cel_nonnull iterator,
    size_t* cel_nonnull remaining);

struct cel_ValueIteratorVTable {
  // NOLINTBEGIN(google3-readability-class-member-naming)
  cel_ValueIteratorVTable_Delete* cel_nullable Delete;
  cel_ValueIteratorVTable_Next1* cel_nonnull Next1;
  cel_ValueIteratorVTable_Next2* cel_nonnull Next2;
  cel_ValueIteratorVTable_Remaining* cel_nullable Remaining;
  // NOLINTEND(google3-readability-class-member-naming)
};

struct cel_ValueIterator {
  const cel_ValueIteratorVTable* cel_nonnull vtable;
};

static CEL_INLINE void cel_ValueIterator_Delete(
    cel_ValueIterator* cel_nullable iterator) {
  if (iterator != cel_nullptr) {
    CEL_ASSERT_NOT_NULL(iterator->vtable);
    if (iterator->vtable->Delete != cel_nullptr) {
      (*iterator->vtable->Delete)(iterator);
    }
  }
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_ValueIterator_Next1(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_Value* cel_nonnull key_or_value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_NOT_NULL(iterator->vtable);
  CEL_ASSERT_NOT_NULL(iterator->vtable->Next1);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key_or_value);
  CEL_ASSERT_NOT_NULL(status);

  return (*iterator->vtable->Next1)(iterator, context, key_or_value, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_ValueIterator_Next2(
    cel_ValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull key,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_NOT_NULL(iterator->vtable);
  CEL_ASSERT_NOT_NULL(iterator->vtable->Next2);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  return (*iterator->vtable->Next2)(iterator, context, key, value, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_ValueIterator_Remaining(
    const cel_ValueIterator* cel_nonnull iterator,
    size_t* cel_nonnull remaining) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_NOT_NULL(iterator->vtable);
  CEL_ASSERT_NOT_NULL(remaining);

  if (iterator->vtable->Remaining != cel_nullptr) {
    return (*iterator->vtable->Remaining)(iterator, remaining);
  }
  return false;
}

// cel_ListValueIterator

typedef struct cel_ListValueIterator cel_ListValueIterator;
typedef struct cel_ListValueIteratorVTable cel_ListValueIteratorVTable;

typedef bool cel_ListValueIteratorVTable_Next(
    cel_ListValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, size_t* cel_nullable index,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status);

struct cel_ListValueIteratorVTable {
  cel_ValueIteratorVTable super;
  // NOLINTBEGIN(google3-readability-class-member-naming)
  cel_ListValueIteratorVTable_Next* cel_nonnull Next;
  // NOLINTEND(google3-readability-class-member-naming)
};

CEL_STATIC_ASSERT(sizeof(cel_ListValueIteratorVTable) >
                  sizeof(cel_ValueIteratorVTable));
CEL_STATIC_ASSERT(alignof(cel_ListValueIteratorVTable) ==
                  alignof(cel_ValueIteratorVTable));

struct cel_ListValueIterator {
  const cel_ListValueIteratorVTable* cel_nonnull vtable;
};

CEL_STATIC_ASSERT(sizeof(cel_ListValueIterator) == sizeof(cel_ValueIterator));
CEL_STATIC_ASSERT(alignof(cel_ListValueIterator) == alignof(cel_ValueIterator));

static CEL_INLINE void cel_ListValueIterator_Delete(
    cel_ListValueIterator* cel_nullable iterator) {
  cel_ValueIterator_Delete((cel_ValueIterator*)iterator);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_ListValueIterator_Next1(
    cel_ListValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_Value* cel_nonnull key_or_value, cel_Status* cel_nonnull status) {
  return cel_ValueIterator_Next1((cel_ValueIterator*)iterator, context,
                                 key_or_value, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_ListValueIterator_Next2(
    cel_ListValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull key,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status) {
  return cel_ValueIterator_Next2((cel_ValueIterator*)iterator, context, key,
                                 value, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_ListValueIterator_Remaining(
    const cel_ListValueIterator* cel_nonnull iterator,
    size_t* cel_nonnull remaining) {
  return cel_ValueIterator_Remaining((cel_ValueIterator*)iterator, remaining);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_ListValueIterator_Next(
    cel_ListValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, size_t* cel_nullable index,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_NOT_NULL(iterator->vtable);
  CEL_ASSERT_NOT_NULL(iterator->vtable->Next);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  return (*iterator->vtable->Next)(iterator, context, index, value, status);
}

// cel_ListValue

typedef struct cel_ListValueVTable cel_ListValueVTable;
typedef struct cel_ListValue cel_ListValue;

typedef bool cel_ListValueVTable_Equals(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_ListValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status);
typedef bool cel_ListValueVTable_FastSize(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    size_t* cel_nonnull size);
typedef bool cel_ListValueVTable_SlowSize(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull size,
    cel_Status* cel_nonnull status);
typedef bool cel_ListValueVTable_Get(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, size_t index,
    cel_Value* cel_nonnull element, cel_Status* cel_nonnull status);
typedef cel_ListValueIterator* cel_nullable cel_ListValueVTable_NewIterator(
    const cel_ListValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    cel_Status* cel_nonnull status);

struct cel_ListValueVTable {
  // NOLINTBEGIN(google3-readability-class-member-naming)
  cel_ListValueVTable_Equals* cel_nullable Equals;
  cel_ListValueVTable_FastSize* cel_nonnull FastSize;
  cel_ListValueVTable_SlowSize* cel_nonnull SlowSize;
  cel_ListValueVTable_Get* cel_nonnull Get;
  cel_ListValueVTable_NewIterator* cel_nonnull NewIterator;
  // NOLINTEND(google3-readability-class-member-naming)
};

struct cel_ListValue {
  const cel_ListValueVTable* cel_nonnull vtable;
  cel_ValueContent content;
};

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_ListValue_Equals(
    const cel_ListValue* cel_nonnull list_value,
    const cel_ValueContext* cel_nonnull context,
    const cel_ListValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_ListValue_Size(
    const cel_ListValue* cel_nonnull list_value,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull size,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(list_value);
  CEL_ASSERT_NOT_NULL(list_value->vtable);
  CEL_ASSERT_NOT_NULL(list_value->vtable->SlowSize);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(size);
  CEL_ASSERT_NOT_NULL(status);

  return (*list_value->vtable->SlowSize)(
      list_value->vtable, list_value->content, context, size, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_ListValue_Get(
    const cel_ListValue* cel_nonnull list_value,
    const cel_ValueContext* cel_nonnull context, size_t index,
    cel_Value* cel_nonnull element, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(list_value);
  CEL_ASSERT_NOT_NULL(list_value->vtable);
  CEL_ASSERT_NOT_NULL(list_value->vtable->Get);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(element);
  CEL_ASSERT_NOT_NULL(status);

  return (*list_value->vtable->Get)(list_value->vtable, list_value->content,
                                    context, index, element, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_ListValueIterator* cel_nullable
cel_ListValue_NewIterator(const cel_ListValue* cel_nonnull list_value,
                          const cel_ValueContext* cel_nonnull context,
                          cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(list_value);
  CEL_ASSERT_NOT_NULL(list_value->vtable);
  CEL_ASSERT_NOT_NULL(list_value->vtable->NewIterator);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(status);

  return (*list_value->vtable->NewIterator)(
      list_value->vtable, list_value->content, context, status);
}

// cel_MapValueKey

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM {
  cel_MapValueKeyKind_kBool = 1,
  cel_MapValueKeyKind_kInt,
  cel_MapValueKeyKind_kUint,
  cel_MapValueKeyKind_kString,
} cel_MapValueKeyKind;

typedef struct {
  union {
#ifdef _MSC_VER
#pragma pack(push, 4)
#endif
    struct {
      union CEL_ATTRIBUTE_PACKED(4) {
        bool b;
        int64_t i;
        uint64_t u;
        // Limited to strings up to 4 GiB.
        struct CEL_ATTRIBUTE_PACKED(4) {
          const char* cel_nonnull data;
          uint32_t size;
        } s;
      } data;
      cel_MapValueKeyKind kind;
    };
#ifdef _MSC_VER
#pragma pack(pop)
#endif
    alignas(8) char raw[16];
  };
} cel_MapValueKey;

CEL_STATIC_ASSERT(sizeof(cel_MapValueKey) <= 16);
CEL_STATIC_ASSERT(alignof(cel_MapValueKey) == 8);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_MapValueKeyKind
cel_MapValueKey_Kind(const cel_MapValueKey* cel_nonnull map_value_key) {
  CEL_ASSERT_NOT_NULL(map_value_key);
  CEL_ASSERT_NE(map_value_key->kind, 0);

  return map_value_key->kind;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_MapValueKey_IsBool(
    const cel_MapValueKey* cel_nonnull map_value_key) {
  return cel_MapValueKey_Kind(map_value_key) == cel_MapValueKeyKind_kBool;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_MapValueKey_IsInt(
    const cel_MapValueKey* cel_nonnull map_value_key) {
  return cel_MapValueKey_Kind(map_value_key) == cel_MapValueKeyKind_kInt;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_MapValueKey_IsUint(
    const cel_MapValueKey* cel_nonnull map_value_key) {
  return cel_MapValueKey_Kind(map_value_key) == cel_MapValueKeyKind_kUint;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_MapValueKey_IsString(
    const cel_MapValueKey* cel_nonnull map_value_key) {
  return cel_MapValueKey_Kind(map_value_key) == cel_MapValueKeyKind_kString;
}

static CEL_INLINE void cel_MapValueKey_SetBool(
    cel_MapValueKey* cel_nonnull map_value_key, bool b) {
  CEL_ASSERT_NOT_NULL(map_value_key);

  map_value_key->data.b = b;
  map_value_key->kind = cel_MapValueKeyKind_kBool;
}

static CEL_INLINE void cel_MapValueKey_SetInt(
    cel_MapValueKey* cel_nonnull map_value_key, int64_t i) {
  CEL_ASSERT_NOT_NULL(map_value_key);

  map_value_key->data.i = i;
  map_value_key->kind = cel_MapValueKeyKind_kInt;
}

static CEL_INLINE void cel_MapValueKey_SetUint(
    cel_MapValueKey* cel_nonnull map_value_key, uint64_t u) {
  CEL_ASSERT_NOT_NULL(map_value_key);

  map_value_key->data.u = u;
  map_value_key->kind = cel_MapValueKeyKind_kUint;
}

static CEL_INLINE void cel_MapValueKey_SetString(
    cel_MapValueKey* cel_nonnull map_value_key, cel_StringView s) {
  CEL_ASSERT_NOT_NULL(map_value_key);

  map_value_key->data.s.data = cel_StringView_Data(s);
  map_value_key->data.s.size = cel_StringView_Size32(s);
  map_value_key->kind = cel_MapValueKeyKind_kString;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_MapValueKey_GetBool(
    const cel_MapValueKey* cel_nonnull map_value_key) {
  CEL_ASSERT_EQ(cel_MapValueKey_Kind(map_value_key), cel_MapValueKeyKind_kBool);

  return map_value_key->data.b;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int64_t
cel_MapValueKey_GetInt(const cel_MapValueKey* cel_nonnull map_value_key) {
  CEL_ASSERT_EQ(cel_MapValueKey_Kind(map_value_key), cel_MapValueKeyKind_kInt);

  return map_value_key->data.i;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint64_t
cel_MapValueKey_GetUint(const cel_MapValueKey* cel_nonnull map_value_key) {
  CEL_ASSERT_EQ(cel_MapValueKey_Kind(map_value_key), cel_MapValueKeyKind_kUint);

  return map_value_key->data.u;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
cel_MapValueKey_GetString(const cel_MapValueKey* cel_nonnull map_value_key) {
  CEL_ASSERT_EQ(cel_MapValueKey_Kind(map_value_key),
                cel_MapValueKeyKind_kString);

  CEL_ASSUME(map_value_key->data.s.data != cel_nullptr);
  return cel_StringView_FromArray(map_value_key->data.s.data,
                                  map_value_key->data.s.size);
}

// cel_MapValueIterator

typedef struct cel_MapValueIterator cel_MapValueIterator;
typedef struct cel_MapValueIteratorVTable cel_MapValueIteratorVTable;

typedef bool cel_MapValueIteratorVTable_Next(
    cel_MapValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_MapValueKey* cel_nonnull key, cel_Value* cel_nullable value,
    cel_Status* cel_nonnull status);

struct cel_MapValueIteratorVTable {
  cel_ValueIteratorVTable super;
  // NOLINTBEGIN(google3-readability-class-member-naming)
  cel_MapValueIteratorVTable_Next* cel_nonnull Next;
  // NOLINTEND(google3-readability-class-member-naming)
};

CEL_STATIC_ASSERT(sizeof(cel_MapValueIteratorVTable) >
                  sizeof(cel_ValueIteratorVTable));
CEL_STATIC_ASSERT(alignof(cel_MapValueIteratorVTable) ==
                  alignof(cel_ValueIteratorVTable));

struct cel_MapValueIterator {
  const cel_MapValueIteratorVTable* cel_nonnull vtable;
};

CEL_STATIC_ASSERT(sizeof(cel_MapValueIterator) == sizeof(cel_ValueIterator));
CEL_STATIC_ASSERT(alignof(cel_MapValueIterator) == alignof(cel_ValueIterator));

static CEL_INLINE void cel_MapValueIterator_Delete(
    cel_MapValueIterator* cel_nullable iterator) {
  cel_ValueIterator_Delete((cel_ValueIterator*)iterator);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_MapValueIterator_Next1(
    cel_MapValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_Value* cel_nonnull key_or_value, cel_Status* cel_nonnull status) {
  return cel_ValueIterator_Next1((cel_ValueIterator*)iterator, context,
                                 key_or_value, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_MapValueIterator_Next2(
    cel_MapValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull key,
    cel_Value* cel_nonnull value, cel_Status* cel_nonnull status) {
  return cel_ValueIterator_Next2((cel_ValueIterator*)iterator, context, key,
                                 value, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_MapValueIterator_Remaining(
    const cel_MapValueIterator* cel_nonnull iterator,
    size_t* cel_nonnull remaining) {
  return cel_ValueIterator_Remaining((cel_ValueIterator*)iterator, remaining);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_MapValueIterator_Next(
    cel_MapValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_MapValueKey* cel_nonnull key, cel_Value* cel_nullable value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_NOT_NULL(iterator->vtable);
  CEL_ASSERT_NOT_NULL(iterator->vtable->Next);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(status);

  return (*iterator->vtable->Next)(iterator, context, key, value, status);
}

// cel_MapValue

typedef struct cel_MapValueVTable cel_MapValueVTable;
typedef struct cel_MapValue cel_MapValue;

typedef bool cel_MapValueVTable_Equals(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status);
typedef bool cel_MapValueVTable_FastSize(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    size_t* cel_nonnull size);
typedef bool cel_MapValueVTable_SlowSize(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull size,
    cel_Status* cel_nonnull status);
typedef bool cel_MapValueVTable_Get(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValueKey* cel_nonnull key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status);
typedef bool cel_MapValueVTable_Find(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValueKey* cel_nonnull key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status);
typedef bool cel_MapValueVTable_Has(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValueKey* cel_nonnull key, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status);
typedef cel_MapValueIterator* cel_nullable cel_MapValueVTable_NewIterator(
    const cel_MapValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    cel_Status* cel_nonnull status);

struct cel_MapValueVTable {
  // NOLINTBEGIN(google3-readability-class-member-naming)
  cel_MapValueVTable_Equals* cel_nullable Equals;
  cel_MapValueVTable_FastSize* cel_nonnull FastSize;
  cel_MapValueVTable_SlowSize* cel_nonnull SlowSize;
  cel_MapValueVTable_Get* cel_nonnull Get;
  cel_MapValueVTable_Find* cel_nonnull Find;
  cel_MapValueVTable_Has* cel_nonnull Has;
  cel_MapValueVTable_NewIterator* cel_nonnull NewIterator;
  // NOLINTEND(google3-readability-class-member-naming)
};

struct cel_MapValue {
  const cel_MapValueVTable* cel_nonnull vtable;
  cel_ValueContent content;
};

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_MapValue_Equals(const cel_MapValue* cel_nonnull map_value,
                                    const cel_ValueContext* cel_nonnull context,
                                    const cel_MapValue* cel_nonnull other,
                                    cel_Value* cel_nonnull result,
                                    cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_MapValue_Size(
    const cel_MapValue* cel_nonnull map_value,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull size,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(map_value);
  CEL_ASSERT_NOT_NULL(map_value->vtable);
  CEL_ASSERT_NOT_NULL(map_value->vtable->SlowSize);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(size);
  CEL_ASSERT_NOT_NULL(status);

  return (*map_value->vtable->SlowSize)(map_value->vtable, map_value->content,
                                        context, size, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_MapValue_Get(
    const cel_MapValue* cel_nonnull map_value,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValueKey* cel_nonnull key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(map_value);
  CEL_ASSERT_NOT_NULL(map_value->vtable);
  CEL_ASSERT_NOT_NULL(map_value->vtable->Get);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  return (*map_value->vtable->Get)(map_value->vtable, map_value->content,
                                   context, key, value, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_MapValue_Find(
    const cel_MapValue* cel_nonnull map_value,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValueKey* cel_nonnull key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(map_value);
  CEL_ASSERT_NOT_NULL(map_value->vtable);
  CEL_ASSERT_NOT_NULL(map_value->vtable->Find);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  return (*map_value->vtable->Find)(map_value->vtable, map_value->content,
                                    context, key, value, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_MapValue_Has(
    const cel_MapValue* cel_nonnull map_value,
    const cel_ValueContext* cel_nonnull context,
    const cel_MapValueKey* cel_nonnull key, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(map_value);
  CEL_ASSERT_NOT_NULL(map_value->vtable);
  CEL_ASSERT_NOT_NULL(map_value->vtable->Has);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  return (*map_value->vtable->Has)(map_value->vtable, map_value->content,
                                   context, key, result, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_MapValueIterator* cel_nullable
cel_MapValue_NewIterator(const cel_MapValue* cel_nonnull map_value,
                         const cel_ValueContext* cel_nonnull context,
                         cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(map_value);
  CEL_ASSERT_NOT_NULL(map_value->vtable);
  CEL_ASSERT_NOT_NULL(map_value->vtable->NewIterator);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(status);

  return (*map_value->vtable->NewIterator)(map_value->vtable,
                                           map_value->content, context, status);
}

// cel_StructValueKey

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM {
  cel_StructValueKeyKind_kName = 1,
  cel_StructValueKeyKind_kDef,
} cel_StructValueKeyKind;

typedef struct {
  union {
#ifdef _MSC_VER
#pragma pack(push, 4)
#endif
    struct {
      union CEL_ATTRIBUTE_PACKED(4) {
        // Limited to strings up to 4 GiB.
        struct CEL_ATTRIBUTE_PACKED(4) {
          const char* cel_nonnull data;
          uint32_t size;
        } name;
        const upb_FieldDef* cel_nonnull def;
      } data;
      cel_StructValueKeyKind kind;
    };
#ifdef _MSC_VER
#pragma pack(pop)
#endif
    alignas(8) char raw[16];
  };
} cel_StructValueKey;

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StructValueKeyKind cel_StructValueKey_Kind(
    const cel_StructValueKey* cel_nonnull struct_value_key) {
  CEL_ASSERT_NOT_NULL(struct_value_key);
  CEL_ASSERT_NE(struct_value_key->kind, 0);

  return struct_value_key->kind;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_StructValueKey_IsName(
    const cel_StructValueKey* cel_nonnull struct_value_key) {
  return cel_StructValueKey_Kind(struct_value_key) ==
         cel_StructValueKeyKind_kName;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_StructValueKey_IsDef(
    const cel_StructValueKey* cel_nonnull struct_value_key) {
  return cel_StructValueKey_Kind(struct_value_key) ==
         cel_StructValueKeyKind_kDef;
}

static CEL_INLINE void cel_StructValueKey_SetName(
    cel_StructValueKey* cel_nonnull struct_value_key, cel_StringView name) {
  CEL_ASSERT_NOT_NULL(struct_value_key);
  CEL_ASSERT_NOT(cel_StringView_Empty(name));

  struct_value_key->data.name.data = cel_StringView_Data(name);
  struct_value_key->data.name.size = cel_StringView_Size32(name);
  struct_value_key->kind = cel_StructValueKeyKind_kName;
}

static CEL_INLINE void cel_StructValueKey_SetDef(
    cel_StructValueKey* cel_nonnull struct_value_key,
    const upb_FieldDef* cel_nonnull def) {
  CEL_ASSERT_NOT_NULL(struct_value_key);
  CEL_ASSERT_NOT_NULL(def);

  struct_value_key->data.def = def;
  struct_value_key->kind = cel_StructValueKeyKind_kDef;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView cel_StructValueKey_GetName(
    const cel_StructValueKey* cel_nonnull struct_value_key) {
  CEL_ASSERT_NOT_NULL(struct_value_key);

  switch (cel_StructValueKey_Kind(struct_value_key)) {
    case cel_StructValueKeyKind_kName:
      CEL_ASSUME(struct_value_key->data.name.data != cel_nullptr);
      return cel_StringView_FromArray(struct_value_key->data.name.data,
                                      struct_value_key->data.name.size);
    case cel_StructValueKeyKind_kDef:
      return cel_StringView_FromString(
          upb_FieldDef_Name(struct_value_key->data.def));
    default:
      CEL_UNREACHABLE();
  }
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const upb_FieldDef* cel_nonnull cel_StructValueKey_GetDef(
    const cel_StructValueKey* cel_nonnull struct_value_key) {
  CEL_ASSERT_NOT_NULL(struct_value_key);
  CEL_ASSERT_EQ(cel_StructValueKey_Kind(struct_value_key),
                cel_StructValueKeyKind_kDef);

  return struct_value_key->data.def;
}

// cel_StructValueIterator

typedef struct cel_StructValueIterator cel_StructValueIterator;
typedef struct cel_StructValueIteratorVTable cel_StructValueIteratorVTable;

typedef void cel_StructValueIteratorVTable_Delete(
    cel_StructValueIterator* cel_nonnull iterator);
typedef bool cel_StructValueIteratorVTable_Next(
    cel_StructValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_StructValueKey* cel_nonnull key, cel_Value* cel_nullable value,
    cel_Status* cel_nonnull status);
typedef bool cel_StructValueIteratorVTable_Remaining(
    const cel_StructValueIterator* cel_nonnull iterator,
    size_t* cel_nonnull remaining);

struct cel_StructValueIteratorVTable {
  // NOLINTBEGIN(google3-readability-class-member-naming)
  cel_StructValueIteratorVTable_Delete* cel_nullable Delete;
  cel_StructValueIteratorVTable_Next* cel_nonnull Next;
  cel_StructValueIteratorVTable_Remaining* cel_nullable Remaining;
  // NOLINTEND(google3-readability-class-member-naming)
};

struct cel_StructValueIterator {
  const cel_StructValueIteratorVTable* cel_nonnull vtable;
};

static CEL_INLINE void cel_StructValueIterator_Delete(
    cel_StructValueIterator* cel_nullable iterator) {
  if (iterator != cel_nullptr) {
    CEL_ASSERT_NOT_NULL(iterator->vtable);
    if (iterator->vtable->Delete != cel_nullptr) {
      (*iterator->vtable->Delete)(iterator);
    }
  }
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_StructValueIterator_Next(
    cel_StructValueIterator* cel_nonnull iterator,
    const cel_ValueContext* cel_nonnull context,
    cel_StructValueKey* cel_nonnull key, cel_Value* cel_nullable value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_NOT_NULL(iterator->vtable);
  CEL_ASSERT_NOT_NULL(iterator->vtable->Next);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(status);

  return (*iterator->vtable->Next)(iterator, context, key, value, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_StructValueIterator_Remaining(
    const cel_StructValueIterator* cel_nonnull iterator,
    size_t* cel_nonnull remaining) {
  CEL_ASSERT_NOT_NULL(iterator);
  CEL_ASSERT_NOT_NULL(iterator->vtable);
  CEL_ASSERT_NOT_NULL(remaining);

  if (iterator->vtable->Remaining != cel_nullptr) {
    return (*iterator->vtable->Remaining)(iterator, remaining);
  }
  return false;
}

// cel_StructValue

typedef struct cel_StructValue cel_StructValue;
typedef struct cel_StructValueVTable cel_StructValueVTable;

typedef bool cel_StructValueVTable_Equals(
    const cel_StructValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_StructValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status);
typedef cel_StringView cel_StructValueVTable_TypeName(
    const cel_StructValueVTable* cel_nonnull vtable, cel_ValueContent content);
typedef bool cel_StructValueVTable_Get(
    const cel_StructValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_StructValueKey* cel_nonnull key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status);
typedef bool cel_StructValueVTable_Has(
    const cel_StructValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_StructValueKey* cel_nonnull key, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status);
typedef cel_StructValueIterator* cel_nullable cel_StructValueVTable_NewIterator(
    const cel_StructValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    cel_Status* cel_nonnull status);

struct cel_StructValueVTable {
  // NOLINTBEGIN(google3-readability-class-member-naming)
  cel_StructValueVTable_Equals* cel_nullable Equals;
  cel_StructValueVTable_TypeName* cel_nonnull TypeName;
  cel_StructValueVTable_Get* cel_nonnull Get;
  cel_StructValueVTable_Has* cel_nonnull Has;
  cel_StructValueVTable_NewIterator* cel_nonnull NewIterator;
  // NOLINTEND(google3-readability-class-member-naming)
};

struct cel_StructValue {
  const cel_StructValueVTable* cel_nonnull vtable;
  cel_ValueContent content;
};

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_StructValue_Equals(
    const cel_StructValue* cel_nonnull struct_value,
    const cel_ValueContext* cel_nonnull context,
    const cel_StructValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
cel_StructValue_TypeName(const cel_StructValue* cel_nonnull struct_value) {
  CEL_ASSERT_NOT_NULL(struct_value);
  CEL_ASSERT_NOT_NULL(struct_value->vtable);
  CEL_ASSERT_NOT_NULL(struct_value->vtable->TypeName);

  return (*struct_value->vtable->TypeName)(struct_value->vtable,
                                           struct_value->content);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_StructValue_Get(
    const cel_StructValue* cel_nonnull struct_value,
    const cel_ValueContext* cel_nonnull context,
    const cel_StructValueKey* cel_nonnull key, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(struct_value);
  CEL_ASSERT_NOT_NULL(struct_value->vtable);
  CEL_ASSERT_NOT_NULL(struct_value->vtable->Get);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(status);

  return (*struct_value->vtable->Get)(
      struct_value->vtable, struct_value->content, context, key, value, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_StructValue_Has(
    const cel_StructValue* cel_nonnull struct_value,
    const cel_ValueContext* cel_nonnull context,
    const cel_StructValueKey* cel_nonnull key, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(struct_value);
  CEL_ASSERT_NOT_NULL(struct_value->vtable);
  CEL_ASSERT_NOT_NULL(struct_value->vtable->Has);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(key);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  return (*struct_value->vtable->Has)(struct_value->vtable,
                                      struct_value->content, context, key,
                                      result, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StructValueIterator* cel_nullable
cel_StructValue_NewIterator(const cel_StructValue* cel_nonnull struct_value,
                            const cel_ValueContext* cel_nonnull context,
                            cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(struct_value);
  CEL_ASSERT_NOT_NULL(struct_value->vtable);
  CEL_ASSERT_NOT_NULL(struct_value->vtable->NewIterator);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(status);

  return (*struct_value->vtable->NewIterator)(
      struct_value->vtable, struct_value->content, context, status);
}

// cel_OpaqueValue

typedef struct cel_OpaqueValueVTable cel_OpaqueValueVTable;
typedef struct cel_OpaqueValue cel_OpaqueValue;

typedef bool cel_OpaqueValueVTable_Equals(
    const cel_OpaqueValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context,
    const cel_OpaqueValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status);
typedef cel_StringView cel_OpaqueValueVTable_TypeName(
    const cel_OpaqueValueVTable* cel_nonnull vtable, cel_ValueContent content);

struct cel_OpaqueValueVTable {
  // NOLINTBEGIN(google3-readability-class-member-naming)
  cel_OpaqueValueVTable_Equals* cel_nullable Equals;
  cel_OpaqueValueVTable_TypeName* cel_nonnull TypeName;
  // NOLINTEND(google3-readability-class-member-naming)
};

struct cel_OpaqueValue {
  const cel_OpaqueValueVTable* cel_nonnull vtable;
  cel_ValueContent content;
};

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_OpaqueValue_Equals(
    const cel_OpaqueValue* cel_nonnull opaque_value,
    const cel_ValueContext* cel_nonnull context,
    const cel_OpaqueValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
cel_OpaqueValue_TypeName(const cel_OpaqueValue* cel_nonnull opaque_value) {
  CEL_ASSERT_NOT_NULL(opaque_value);
  CEL_ASSERT_NOT_NULL(opaque_value->vtable);
  CEL_ASSERT_NOT_NULL(opaque_value->vtable->TypeName);

  return (*opaque_value->vtable->TypeName)(opaque_value->vtable,
                                           opaque_value->content);
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_OpaqueValue_IsOptional(
    const cel_OpaqueValue* cel_nonnull opaque_value);

// cel_OptionalValue

typedef struct cel_OptionalValueVTable cel_OptionalValueVTable;

typedef bool cel_OptionalValueVTable_HasValue(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status);
typedef bool cel_OptionalValueVTable_Value(
    const cel_OptionalValueVTable* cel_nonnull vtable, cel_ValueContent content,
    const cel_ValueContext* cel_nonnull context, cel_Value* cel_nonnull value,
    cel_Status* cel_nonnull status);

struct cel_OptionalValueVTable {
  cel_OpaqueValueVTable super;
  // NOLINTBEGIN(google3-readability-class-member-naming)
  cel_OptionalValueVTable_HasValue* cel_nonnull HasValue;
  cel_OptionalValueVTable_Value* cel_nonnull Value;
  // NOLINTEND(google3-readability-class-member-naming)
};

CEL_STATIC_ASSERT(sizeof(cel_OptionalValueVTable) >=
                  sizeof(cel_OpaqueValueVTable));
CEL_STATIC_ASSERT(alignof(cel_OptionalValueVTable) ==
                  alignof(cel_OpaqueValueVTable));

typedef struct {
  const cel_OptionalValueVTable* cel_nonnull vtable;
  cel_ValueContent content;
} cel_OptionalValue;

CEL_STATIC_ASSERT(sizeof(cel_OptionalValue) == sizeof(cel_OpaqueValue));
CEL_STATIC_ASSERT(alignof(cel_OptionalValue) == alignof(cel_OpaqueValue));
CEL_STATIC_ASSERT(offsetof(cel_OptionalValue, vtable) ==
                  offsetof(cel_OpaqueValue, vtable));
CEL_STATIC_ASSERT(offsetof(cel_OptionalValue, content) ==
                  offsetof(cel_OpaqueValue, content));

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_OptionalValue_Equals(
    const cel_OptionalValue* cel_nonnull optional_value,
    const cel_ValueContext* cel_nonnull context,
    const cel_OptionalValue* cel_nonnull other, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  return cel_OpaqueValue_Equals((const cel_OpaqueValue*)optional_value, context,
                                (const cel_OpaqueValue*)other, result, status);
}

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_OptionalValue* cel_nonnull
cel_OptionalValue_Empty(cel_OptionalValue* cel_nonnull optional_value);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_OptionalValue_Of(
    cel_OptionalValue* cel_nonnull optional_value,
    const cel_Value* cel_nonnull value, cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_OptionalValue_HasValue(
    const cel_OptionalValue* cel_nonnull optional_value,
    const cel_ValueContext* context, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(optional_value);
  CEL_ASSERT_NOT_NULL(optional_value->vtable);
  CEL_ASSERT_NOT_NULL(optional_value->vtable->HasValue);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  return (*optional_value->vtable->HasValue)(
      optional_value->vtable, optional_value->content, context, result, status);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_OptionalValue_Value(
    const cel_OptionalValue* cel_nonnull optional_value,
    const cel_ValueContext* context, cel_Value* cel_nonnull result,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(optional_value);
  CEL_ASSERT_NOT_NULL(optional_value->vtable);
  CEL_ASSERT_NOT_NULL(optional_value->vtable->Value);
  CEL_ASSERT_NOT_NULL(context);
  CEL_ASSERT_NOT_NULL(result);
  CEL_ASSERT_NOT_NULL(status);

  return (*optional_value->vtable->Value)(
      optional_value->vtable, optional_value->content, context, result, status);
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
static CEL_INLINE const cel_OptionalValue* cel_nonnull
cel_OpaqueValue_GetOptional(const cel_OpaqueValue* cel_nonnull opaque_value) {
  CEL_ASSERT(cel_OpaqueValue_IsOptional(opaque_value));

  return (const cel_OptionalValue*)opaque_value;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
static CEL_INLINE const cel_OptionalValue* cel_nullable
cel_OpaqueValue_AsOptional(const cel_OpaqueValue* cel_nonnull opaque_value) {
  if (cel_OpaqueValue_IsOptional(opaque_value)) {
    return (const cel_OptionalValue*)opaque_value;
  }
  return cel_nullptr;
}

// cel_Value

struct cel_Value {
  union {
    alignas(8) char v[24];
    bool bl;
    int64_t i;
    uint64_t u;
    double dbl;
    cel_StringView byts;
    cel_StringView strng;
    cel_Duration drtn;
    cel_Timestamp tmstmp;
    cel_ListValue lst;
    cel_MapValue mp;
    cel_StructValue strct;
    cel_OpaqueValue opq;
    // opt is not directly used, but is here to enforce layout constraints
    // between cel_OpaqueValue and cel_OptionalValue as required by the
    // standard.
    cel_OptionalValue opt;
    cel_StringView typ;
    const cel_Error* cel_nonnull err;
  } data;
  CEL_ATTRIBUTE_PREFERRED_TYPE(cel_ValueKind) unsigned char kind : 8;
  char padding[7];
};

CEL_STATIC_ASSERT(sizeof(cel_Value) == 32);
CEL_STATIC_ASSERT(alignof(cel_Value) == 8);

CEL_EXTERN const cel_Value cel_NullValue;

CEL_EXTERN const cel_Value cel_FalseValue;

CEL_EXTERN const cel_Value cel_TrueValue;

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_ValueKind
cel_Value_Kind(const cel_Value* cel_nonnull value) {
  CEL_ASSERT_NOT_NULL(value);

  return (cel_ValueKind)value->kind;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView
cel_Value_TypeName(const cel_Value* cel_nonnull value);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Value_Equals(const cel_Value* cel_nonnull value,
                                 const cel_ValueContext* cel_nonnull context,
                                 const cel_Value* cel_nonnull other,
                                 cel_Value* cel_nonnull result,
                                 cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Value_FromMessage(
    cel_Value* cel_nonnull value, const cel_ValueContext* cel_nonnull context,
    const upb_Message* message_val,
    const upb_MessageDef* cel_nonnull message_def,
    cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Value_FromEnum(cel_Value* cel_nonnull value,
                                   const cel_ValueContext* cel_nonnull context,
                                   int32_t enum_val,
                                   const upb_EnumDef* cel_nonnull enum_def,
                                   cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Value_FromEnumValue(
    cel_Value* cel_nonnull value, const cel_ValueContext* cel_nonnull context,
    const upb_EnumValueDef* enum_val, cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Value_FromField(cel_Value* cel_nonnull value,
                                    const cel_ValueContext* cel_nonnull context,
                                    upb_MessageValue field_val,
                                    const upb_FieldDef* cel_nonnull field_def,
                                    cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Value_FromRepeatedFieldElement(
    cel_Value* cel_nonnull value, const cel_ValueContext* cel_nonnull context,
    upb_MessageValue field_val, const upb_FieldDef* cel_nonnull field_def,
    cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Value_FromMapFieldKey(
    cel_Value* cel_nonnull value, const cel_ValueContext* cel_nonnull context,
    upb_MessageValue field_val, const upb_FieldDef* cel_nonnull field_def,
    cel_Status* cel_nonnull status);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Value_FromMapFieldValue(
    cel_Value* cel_nonnull value, const cel_ValueContext* cel_nonnull context,
    upb_MessageValue field_val, const upb_FieldDef* cel_nonnull field_def,
    cel_Status* cel_nonnull status);

// cel_Value_SetX

static CEL_INLINE void cel_Value_SetNull(cel_Value* cel_nonnull value) {
  CEL_ASSERT_NOT_NULL(value);

  value->kind = cel_ValueKind_kNull;
}

static CEL_INLINE void cel_Value_SetBool(cel_Value* cel_nonnull value, bool b) {
  CEL_ASSERT_NOT_NULL(value);

  value->data.bl = b;
  value->kind = cel_ValueKind_kBool;
}

static CEL_INLINE void cel_Value_SetTrue(cel_Value* cel_nonnull value) {
  cel_Value_SetBool(value, true);
}

static CEL_INLINE void cel_Value_SetFalse(cel_Value* cel_nonnull value) {
  cel_Value_SetBool(value, false);
}

static CEL_INLINE void cel_Value_SetInt(cel_Value* cel_nonnull value,
                                        int64_t i) {
  CEL_ASSERT_NOT_NULL(value);

  value->data.i = i;
  value->kind = cel_ValueKind_kInt;
}

static CEL_INLINE void cel_Value_SetUint(cel_Value* cel_nonnull value,
                                         uint64_t u) {
  CEL_ASSERT_NOT_NULL(value);

  value->data.u = u;
  value->kind = cel_ValueKind_kUint;
}

static CEL_INLINE void cel_Value_SetDouble(cel_Value* cel_nonnull value,
                                           double dbl) {
  CEL_ASSERT_NOT_NULL(value);

  value->data.dbl = dbl;
  value->kind = cel_ValueKind_kDouble;
}

static CEL_INLINE void cel_Value_SetBytes(cel_Value* cel_nonnull value,
                                          cel_StringView byts) {
  CEL_ASSERT_NOT_NULL(value);

  value->data.byts = byts;
  value->kind = cel_ValueKind_kBytes;
}

static CEL_INLINE void cel_Value_SetString(cel_Value* cel_nonnull value,
                                           cel_StringView strng) {
  CEL_ASSERT_NOT_NULL(value);

  value->data.strng = strng;
  value->kind = cel_ValueKind_kString;
}

static CEL_INLINE void cel_Value_SetDuration(cel_Value* cel_nonnull value,
                                             cel_Duration drtn) {
  CEL_ASSERT_NOT_NULL(value);

  value->data.drtn = drtn;
  value->kind = cel_ValueKind_kDuration;
}

static CEL_INLINE void cel_Value_SetTimestamp(cel_Value* cel_nonnull value,
                                              cel_Timestamp tmstmp) {
  CEL_ASSERT_NOT_NULL(value);

  value->data.tmstmp = tmstmp;
  value->kind = cel_ValueKind_kTimestamp;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_ListValue* cel_nonnull
cel_Value_SetList(cel_Value* cel_nonnull value) {
  CEL_ASSERT_NOT_NULL(value);

  value->kind = cel_ValueKind_kList;
  return &value->data.lst;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_MapValue* cel_nonnull
cel_Value_SetMap(cel_Value* cel_nonnull value) {
  CEL_ASSERT_NOT_NULL(value);

  value->kind = cel_ValueKind_kMap;
  return &value->data.mp;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StructValue* cel_nonnull
cel_Value_SetStruct(cel_Value* cel_nonnull value) {
  CEL_ASSERT_NOT_NULL(value);

  value->kind = cel_ValueKind_kStruct;
  return &value->data.strct;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_OpaqueValue* cel_nonnull
cel_Value_SetOpaque(cel_Value* cel_nonnull value) {
  CEL_ASSERT_NOT_NULL(value);

  value->kind = cel_ValueKind_kOpaque;
  return &value->data.opq;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_OptionalValue* cel_nonnull
cel_Value_SetOptional(cel_Value* cel_nonnull value) {
  return (cel_OptionalValue*)cel_Value_SetOpaque(value);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE void cel_Value_SetType(cel_Value* cel_nonnull value,
                                         cel_StringView type) {
  CEL_ASSERT_NOT_NULL(value);

  value->kind = cel_ValueKind_kType;
  value->data.typ = type;
}

static CEL_INLINE void cel_Value_SetError(cel_Value* cel_nonnull value,
                                          const cel_Error* cel_nonnull err) {
  CEL_ASSERT_NOT_NULL(value);
  CEL_ASSERT_NOT_NULL(err);

  value->data.err = err;
  value->kind = cel_ValueKind_kError;
}

// cel_Value_IsX

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsMapKey(const cel_Value* cel_nonnull value) {
  switch (cel_Value_Kind(value)) {
    case cel_ValueKind_kBool:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ValueKind_kInt:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ValueKind_kUint:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case cel_ValueKind_kString:
      return true;
    default:
      return false;
  }
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsNull(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kNull;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsBool(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kBool;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsFalse(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kBool && !value->data.bl;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsTrue(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kBool && value->data.bl;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsInt(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kInt;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsUint(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kUint;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsDouble(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kDouble;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsBytes(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kBytes;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsString(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kString;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsDuration(
    const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kDuration;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsTimestamp(
    const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kTimestamp;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsList(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kList;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsMap(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kMap;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsStruct(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kStruct;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsOpaque(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kOpaque;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Value_IsOptional(const cel_Value* cel_nonnull value);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsType(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kType;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsError(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kError;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_IsUnknown(const cel_Value* cel_nonnull value) {
  return cel_Value_Kind(value) == cel_ValueKind_kUnknown;
}

// cel_Value_GetX

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Value_GetBool(const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsBool(value));

  return value->data.bl;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int64_t cel_Value_GetInt(const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsInt(value));

  return value->data.i;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint64_t
cel_Value_GetUint(const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsUint(value));

  return value->data.u;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE double cel_Value_GetDouble(
    const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsDouble(value));

  return value->data.dbl;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
cel_Value_GetBytes(const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsBytes(value));

  return value->data.byts;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
cel_Value_GetString(const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsString(value));

  return value->data.strng;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Duration
cel_Value_GetDuration(const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsDuration(value));

  return value->data.drtn;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Timestamp
cel_Value_GetTimestamp(const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsTimestamp(value));

  return value->data.tmstmp;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_ListValue* cel_nonnull
cel_Value_GetList(const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsList(value));

  return &value->data.lst;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_MapValue* cel_nonnull
cel_Value_GetMap(const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsMap(value));

  return &value->data.mp;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_StructValue* cel_nonnull
cel_Value_GetStruct(const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsStruct(value));

  return &value->data.strct;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_OpaqueValue* cel_nonnull
cel_Value_GetOpaque(const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsOpaque(value));

  return &value->data.opq;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_OptionalValue* cel_nonnull
cel_Value_GetOptional(const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsOptional(value));

  return (const cel_OptionalValue*)cel_Value_GetOpaque(value);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
cel_Value_GetType(const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsType(value));

  return value->data.typ;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Error* cel_nonnull
cel_Value_GetError(const cel_Value* cel_nonnull value) {
  CEL_ASSERT(cel_Value_IsError(value));

  return value->data.err;
}

// cel_Value_AsX

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const bool* cel_nullable
cel_Value_AsBool(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsBool(value)) {
    return &value->data.bl;
  }
  return cel_nullptr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const int64_t* cel_nullable
cel_Value_AsInt(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsInt(value)) {
    return &value->data.i;
  }
  return cel_nullptr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const uint64_t* cel_nullable
cel_Value_AsUint(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsUint(value)) {
    return &value->data.u;
  }
  return cel_nullptr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const double* cel_nullable
cel_Value_AsDouble(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsDouble(value)) {
    return &value->data.dbl;
  }
  return cel_nullptr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_StringView* cel_nullable
cel_Value_AsBytes(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsBytes(value)) {
    return &value->data.byts;
  }
  return cel_nullptr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_StringView* cel_nullable
cel_Value_AsString(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsString(value)) {
    return &value->data.strng;
  }
  return cel_nullptr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Duration* cel_nullable
cel_Value_AsDuration(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsDuration(value)) {
    return &value->data.drtn;
  }
  return cel_nullptr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Timestamp* cel_nullable
cel_Value_AsTimestamp(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsTimestamp(value)) {
    return &value->data.tmstmp;
  }
  return cel_nullptr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_ListValue* cel_nullable
cel_Value_AsList(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsList(value)) {
    return &value->data.lst;
  }
  return cel_nullptr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_MapValue* cel_nullable
cel_Value_AsMap(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsMap(value)) {
    return &value->data.mp;
  }
  return cel_nullptr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_StructValue* cel_nullable
cel_Value_AsStruct(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsStruct(value)) {
    return &value->data.strct;
  }
  return cel_nullptr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_OpaqueValue* cel_nullable
cel_Value_AsOpaque(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsOpaque(value)) {
    return &value->data.opq;
  }
  return cel_nullptr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_OptionalValue* cel_nullable
cel_Value_AsOptional(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsOptional(value)) {
    return (const cel_OptionalValue*)&value->data.opq;
  }
  return cel_nullptr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_StringView* cel_nullable
cel_Value_AsType(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsType(value)) {
    return &value->data.typ;
  }
  return cel_nullptr;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const cel_Error* cel_nullable
cel_Value_AsError(const cel_Value* cel_nonnull value) {
  if (cel_Value_IsError(value)) {
    return value->data.err;
  }
  return cel_nullptr;
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_VALUE_H_
