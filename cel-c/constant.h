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

#ifndef THIRD_PARTY_CEL_C_CONSTANT_H_
#define THIRD_PARTY_CEL_C_CONSTANT_H_

#include <stdalign.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stdint.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"

CEL_BEGIN_DECLS

// cel_ConstantKind
//
// Enumeration indicating the kinds of values that `cel_Constant` can hold.
typedef enum CEL_ATTRIBUTE_OPEN_ENUM {
  cel_ConstantKind_kUnspecified = 0,
  cel_ConstantKind_kNull,
  cel_ConstantKind_kBool,
  cel_ConstantKind_kInt,
  cel_ConstantKind_kUint,
  cel_ConstantKind_kDouble,
  cel_ConstantKind_kBytes,
  cel_ConstantKind_kString,
  cel_ConstantKind_kDuration,
  cel_ConstantKind_kTimestamp,
} cel_ConstantKind;

// cel_Constant
//
// Represents a constant as specified by the Common Expression Language.
typedef struct {
  union {
#ifdef _MSC_VER
#pragma pack(push, 4)
#endif
    struct {
      union CEL_ATTRIBUTE_PACKED(4) {
        bool b;      // bool
        int64_t i;   // int
        uint64_t u;  // uint
        double f;    // double
        struct CEL_ATTRIBUTE_PACKED(4) {
          const char* cel_nullability_unknown data;
          uint32_t size;
        } s;              // bytes/string
        cel_Duration d;   // duration
        cel_Timestamp t;  // timestamp
      } data;
      cel_ConstantKind kind;
    };
#ifdef _MSC_VER
#pragma pack(pop)
#endif
    alignas(8) char raw[16];
  };
} cel_Constant;

CEL_STATIC_ASSERT(sizeof(cel_Constant) == 16);
CEL_STATIC_ASSERT(alignof(cel_Constant) == 8);

// cel_UnspecifiedConstant
//
// Returns an unspecified constant.
#define cel_UnspecifiedConstant() \
  ((cel_Constant){.kind = cel_ConstantKind_kUnspecified})

// cel_NullConstant
//
// Returns a null constant.
#define cel_NullConstant() ((cel_Constant){.kind = cel_ConstantKind_kNull})

// cel_BoolConstant
//
// Returns a bool constant.
#define cel_BoolConstant(val)         \
  ((cel_Constant){.data =             \
                      {               \
                          .b = (val), \
                      },              \
                  .kind = cel_ConstantKind_kBool})

// cel_IntConstant
//
// Returns an int constant.
#define cel_IntConstant(val)          \
  ((cel_Constant){.data =             \
                      {               \
                          .i = (val), \
                      },              \
                  .kind = cel_ConstantKind_kInt})

// cel_IntConstant
//
// Returns a uint constant.
#define cel_UintConstant(val)         \
  ((cel_Constant){.data =             \
                      {               \
                          .u = (val), \
                      },              \
                  .kind = cel_ConstantKind_kUint})

// cel_DoubleConstant
//
// Returns a double constant.
#define cel_DoubleConstant(val)       \
  ((cel_Constant){.data =             \
                      {               \
                          .f = (val), \
                      },              \
                  .kind = cel_ConstantKind_kDouble})

// cel_BytesConstant
//
// Returns a bytes constant.
#define cel_BytesConstant(val)                                                 \
  ((cel_Constant){                                                             \
      .data =                                                                  \
          {                                                                    \
              .s = {cel_StringView_Data((val)), cel_StringView_Size32((val))}, \
          },                                                                   \
      .kind = cel_ConstantKind_kBytes})

// cel_StringConstant
//
// Returns a string constant.
#define cel_StringConstant(val)                                                \
  ((cel_Constant){                                                             \
      .data =                                                                  \
          {                                                                    \
              .s = {cel_StringView_Data((val)), cel_StringView_Size32((val))}, \
          },                                                                   \
      .kind = cel_ConstantKind_kString})

// cel_DurationConstant
//
// Returns a duration constant.
#define cel_DurationConstant(val)     \
  ((cel_Constant){.data =             \
                      {               \
                          .d = (val), \
                      },              \
                  .kind = cel_ConstantKind_kDuration})

// cel_DurationConstant
//
// Returns a timestamp constant.
#define cel_TimestampConstant(val)    \
  ((cel_Constant){.data =             \
                      {               \
                          .t = (val), \
                      },              \
                  .kind = cel_ConstantKind_kTimestamp})

// cel_Constant_SetNull
//
// Sets the constant to be null.
static CEL_INLINE void cel_Constant_SetNull(CEL_NONNULL(cel_Constant*)
                                                constant) {
  CEL_ASSERT_NOT_NULL(constant);

  *constant = cel_NullConstant();
}

// cel_Constant_SetBool
//
// Sets the constant to be bool.
static CEL_INLINE void cel_Constant_SetBool(CEL_NONNULL(cel_Constant*) constant,
                                            bool val) {
  CEL_ASSERT_NOT_NULL(constant);

  *constant = cel_BoolConstant(val);
}

// cel_Constant_SetInt
//
// Sets the constant to be int.
static CEL_INLINE void cel_Constant_SetInt(CEL_NONNULL(cel_Constant*) constant,
                                           int64_t val) {
  CEL_ASSERT_NOT_NULL(constant);

  *constant = cel_IntConstant(val);
}

// cel_Constant_SetUint
//
// Sets the constant to be uint.
static CEL_INLINE void cel_Constant_SetUint(CEL_NONNULL(cel_Constant*) constant,
                                            uint64_t val) {
  CEL_ASSERT_NOT_NULL(constant);

  *constant = cel_UintConstant(val);
}

// cel_Constant_SetDouble
//
// Sets the constant to be double.
static CEL_INLINE void cel_Constant_SetDouble(CEL_NONNULL(cel_Constant*)
                                                  constant,
                                              double val) {
  CEL_ASSERT_NOT_NULL(constant);

  *constant = cel_DoubleConstant(val);
}

// cel_Constant_SetBytes
//
// Sets the constant to be bytes.
static CEL_INLINE void cel_Constant_SetBytes(CEL_NONNULL(cel_Constant*)
                                                 constant,
                                             cel_StringView val) {
  CEL_ASSERT_NOT_NULL(constant);

  *constant = cel_BytesConstant(val);
}

// cel_Constant_SetString
//
// Sets the constant to be string.
static CEL_INLINE void cel_Constant_SetString(CEL_NONNULL(cel_Constant*)
                                                  constant,
                                              cel_StringView val) {
  CEL_ASSERT_NOT_NULL(constant);

  *constant = cel_StringConstant(val);
}

// cel_Constant_SetDuration
//
// Sets the constant to be duration.
static CEL_INLINE void cel_Constant_SetDuration(CEL_NONNULL(cel_Constant*)
                                                    constant,
                                                cel_Duration val) {
  CEL_ASSERT_NOT_NULL(constant);

  *constant = cel_DurationConstant(val);
}

// cel_Constant_SetTimestamp
//
// Sets the constant to be timestamp.
static CEL_INLINE void cel_Constant_SetTimestamp(CEL_NONNULL(cel_Constant*)
                                                     constant,
                                                 cel_Timestamp val) {
  CEL_ASSERT_NOT_NULL(constant);

  *constant = cel_TimestampConstant(val);
}

// cel_Constant_Kind
//
// Returns the kind of the constant.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_ConstantKind
cel_Constant_Kind(CEL_NONNULL(const cel_Constant*) constant) {
  CEL_ASSERT_NOT_NULL(constant);

  return constant->kind;
}

// cel_Constant_GetBool
//
// Asserts that the constant is a bool and returns the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Constant_GetBool(CEL_NONNULL(const cel_Constant*)
                                                constant) {
  CEL_ASSERT_EQ(cel_Constant_Kind(constant), cel_ConstantKind_kBool);

  return constant->data.b;
}

// cel_Constant_GetInt
//
// Asserts that the constant is an int and returns the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int64_t cel_Constant_GetInt(CEL_NONNULL(const cel_Constant*)
                                                  constant) {
  CEL_ASSERT_EQ(cel_Constant_Kind(constant), cel_ConstantKind_kInt);

  return constant->data.i;
}

// cel_Constant_GetUint
//
// Asserts that the constant is a uint and returns the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint64_t cel_Constant_GetUint(CEL_NONNULL(const cel_Constant*)
                                                    constant) {
  CEL_ASSERT_EQ(cel_Constant_Kind(constant), cel_ConstantKind_kUint);

  return constant->data.u;
}

// cel_Constant_GetDouble
//
// Asserts that the constant is a double and returns the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE double cel_Constant_GetDouble(CEL_NONNULL(const cel_Constant*)
                                                    constant) {
  CEL_ASSERT_EQ(cel_Constant_Kind(constant), cel_ConstantKind_kDouble);

  return constant->data.f;
}

// cel_Constant_GetBytes
//
// Asserts that the constant is bytes and returns the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
cel_Constant_GetBytes(CEL_NONNULL(const cel_Constant*) constant) {
  CEL_ASSERT_EQ(cel_Constant_Kind(constant), cel_ConstantKind_kBytes);

  return cel_StringView_FromArray(constant->data.s.data, constant->data.s.size);
}

// cel_Constant_GetString
//
// Asserts that the constant is a string and returns the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
cel_Constant_GetString(CEL_NONNULL(const cel_Constant*) constant) {
  CEL_ASSERT_EQ(cel_Constant_Kind(constant), cel_ConstantKind_kString);

  return cel_StringView_FromArray(constant->data.s.data, constant->data.s.size);
}

// cel_Constant_GetDuration
//
// Asserts that the constant is a duration and returns the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Duration
cel_Constant_GetDuration(CEL_NONNULL(const cel_Constant*) constant) {
  CEL_ASSERT_EQ(cel_Constant_Kind(constant), cel_ConstantKind_kDuration);

  return constant->data.d;
}

// cel_Constant_GetTimestamp
//
// Asserts that the constant is a timestamp and returns the value.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Timestamp
cel_Constant_GetTimestamp(CEL_NONNULL(const cel_Constant*) constant) {
  CEL_ASSERT_EQ(cel_Constant_Kind(constant), cel_ConstantKind_kTimestamp);

  return constant->data.t;
}

// cel_Constant_Equals
//
// Tests two constants for equality. To be considered equal, the constants must
// have the same kind and value.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Constant_Equals(CEL_NONNULL(const cel_Constant*) lhs,
                                    CEL_NONNULL(const cel_Constant*) rhs);

CEL_END_DECLS

#ifdef __cplusplus

inline bool operator==(const cel_Constant& lhs, const cel_Constant& rhs) {
  return cel_Constant_Equals(&lhs, &rhs);
}

inline bool operator!=(const cel_Constant& lhs, const cel_Constant& rhs) {
  return !operator==(lhs, rhs);
}

#endif

#endif  // THIRD_PARTY_CEL_C_CONSTANT_H_
