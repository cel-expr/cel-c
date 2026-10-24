// Copyright 2026 Google LLC
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

#ifndef THIRD_PARTY_CEL_C_ATTRIBUTE_H_
#define THIRD_PARTY_CEL_C_ATTRIBUTE_H_

#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/string_view.h"

CEL_BEGIN_DECLS

typedef enum {
  cel_AttributeQualifierKind_kUnspecified = 0,
  cel_AttributeQualifierKind_kBool,
  cel_AttributeQualifierKind_kInt,
  cel_AttributeQualifierKind_kUint,
  cel_AttributeQualifierKind_kString,
} cel_AttributeQualifierKind;

// cel_AttributeQualifier
//
// Represents an attribute qualifier.
typedef struct {
#ifdef _MSC_VER
#pragma pack(push, 4)
#endif
  // `alignas(8)` forces `cel_AttributeQualifier` to be 8-byte aligned,
  // regardless of `CEL_ATTRIBUTE_PACKED(4)` and platforms where `int64_t` is
  // only 4-byte aligned (e.g. 32-bit x86).
  alignas(8) union CEL_ATTRIBUTE_PACKED(4) {
    bool b;      // bool
    int64_t i;   // int
    uint64_t u;  // uint
    double f;    // double
    struct CEL_ATTRIBUTE_PACKED(4) {
      const char* cel_nullability_unknown data;
      uint32_t size;
    } s;  // string
  } data;
  cel_AttributeQualifierKind kind;
#ifdef _MSC_VER
#pragma pack(pop)
#endif
} cel_AttributeQualifier;

CEL_STATIC_ASSERT(sizeof(cel_AttributeQualifier) == 16);
CEL_STATIC_ASSERT(alignof(cel_AttributeQualifier) == 8);

// cel_UnspecifiedAttributeQualifier
//
// Returns an unspecified attribute qualifier.
#define cel_UnspecifiedAttributeQualifier() \
  ((cel_AttributeQualifier){.kind = cel_AttributeQualifierKind_kUnspecified})

// cel_BoolAttributeQualifier
//
// Returns a bool attribute qualifier.
#define cel_BoolAttributeQualifier(val)         \
  ((cel_AttributeQualifier){.data =             \
                                {               \
                                    .b = (val), \
                                },              \
                            .kind = cel_AttributeQualifierKind_kBool})

// cel_IntAttributeQualifier
//
// Returns an int attribute qualifier.
#define cel_IntAttributeQualifier(val)          \
  ((cel_AttributeQualifier){.data =             \
                                {               \
                                    .i = (val), \
                                },              \
                            .kind = cel_AttributeQualifierKind_kInt})

// cel_IntAttributeQualifier
//
// Returns a uint attribute qualifier.
#define cel_UintAttributeQualifier(val)         \
  ((cel_AttributeQualifier){.data =             \
                                {               \
                                    .u = (val), \
                                },              \
                            .kind = cel_AttributeQualifierKind_kUint})

// cel_StringAttributeQualifier
//
// Returns a string attribute qualifier.
#define cel_StringAttributeQualifier(val)                                      \
  ((cel_AttributeQualifier){                                                   \
      .data =                                                                  \
          {                                                                    \
              .s = {cel_StringView_Data((val)), cel_StringView_Size32((val))}, \
          },                                                                   \
      .kind = cel_AttributeQualifierKind_kString})

// cel_AttributeQualifier_SetBool
//
// Sets the attribute qualifier to be bool.
static CEL_INLINE void cel_AttributeQualifier_SetBool(
    CEL_NONNULL(cel_AttributeQualifier*) attribute_qualifier, bool val) {
  CEL_ASSERT_NOT_NULL(attribute_qualifier);

  *attribute_qualifier = cel_BoolAttributeQualifier(val);
}

// cel_AttributeQualifier_SetInt
//
// Sets the attribute qualifier to be int.
static CEL_INLINE void cel_AttributeQualifier_SetInt(
    CEL_NONNULL(cel_AttributeQualifier*) attribute_qualifier, int64_t val) {
  CEL_ASSERT_NOT_NULL(attribute_qualifier);

  *attribute_qualifier = cel_IntAttributeQualifier(val);
}

// cel_AttributeQualifier_SetUint
//
// Sets the attribute qualifier to be uint.
static CEL_INLINE void cel_AttributeQualifier_SetUint(
    CEL_NONNULL(cel_AttributeQualifier*) attribute_qualifier, uint64_t val) {
  CEL_ASSERT_NOT_NULL(attribute_qualifier);

  *attribute_qualifier = cel_UintAttributeQualifier(val);
}

// cel_AttributeQualifier_SetString
//
// Sets the attribute qualifier to be string.
static CEL_INLINE void cel_AttributeQualifier_SetString(
    CEL_NONNULL(cel_AttributeQualifier*) attribute_qualifier,
    cel_StringView val) {
  CEL_ASSERT_NOT_NULL(attribute_qualifier);

  *attribute_qualifier = cel_StringAttributeQualifier(val);
}

// cel_AttributeQualifier_Kind
//
// Returns the kind of the attribute qualifier.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_AttributeQualifierKind cel_AttributeQualifier_Kind(
    CEL_NONNULL(const cel_AttributeQualifier*) attribute_qualifier) {
  CEL_ASSERT_NOT_NULL(attribute_qualifier);

  return attribute_qualifier->kind;
}

// cel_AttributeQualifier_GetBool
//
// Asserts that the attribute qualifier is a bool and returns the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_AttributeQualifier_GetBool(
    CEL_NONNULL(const cel_AttributeQualifier*) attribute_qualifier) {
  CEL_ASSERT_EQ(cel_AttributeQualifier_Kind(attribute_qualifier),
                cel_AttributeQualifierKind_kBool);

  return attribute_qualifier->data.b;
}

// cel_AttributeQualifier_GetInt
//
// Asserts that the attribute qualifier is an int and returns the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int64_t cel_AttributeQualifier_GetInt(
    CEL_NONNULL(const cel_AttributeQualifier*) attribute_qualifier) {
  CEL_ASSERT_EQ(cel_AttributeQualifier_Kind(attribute_qualifier),
                cel_AttributeQualifierKind_kInt);

  return attribute_qualifier->data.i;
}

// cel_AttributeQualifier_GetUint
//
// Asserts that the attribute qualifier is a uint and returns the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint64_t cel_AttributeQualifier_GetUint(
    CEL_NONNULL(const cel_AttributeQualifier*) attribute_qualifier) {
  CEL_ASSERT_EQ(cel_AttributeQualifier_Kind(attribute_qualifier),
                cel_AttributeQualifierKind_kUint);

  return attribute_qualifier->data.u;
}

// cel_AttributeQualifier_GetString
//
// Asserts that the attribute qualifier is a string and returns the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView cel_AttributeQualifier_GetString(
    CEL_NONNULL(const cel_AttributeQualifier*) attribute_qualifier) {
  CEL_ASSERT_EQ(cel_AttributeQualifier_Kind(attribute_qualifier),
                cel_AttributeQualifierKind_kString);

  return cel_StringView_FromArray(attribute_qualifier->data.s.data,
                                  attribute_qualifier->data.s.size);
}

// cel_AttributeQualifier_Equals
//
// Tests two attribute qualifiers for equality. To be considered equal, the
// attribute qualifiers must have the same kind and value.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_AttributeQualifier_Equals(
    CEL_NONNULL(const cel_AttributeQualifier*) lhs,
    CEL_NONNULL(const cel_AttributeQualifier*) rhs);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_ATTRIBUTE_H_
