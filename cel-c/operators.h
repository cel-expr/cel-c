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

#ifndef THIRD_PARTY_CEL_C_OPERATORS_H_
#define THIRD_PARTY_CEL_C_OPERATORS_H_

#include <stdbool.h>  // IWYU pragma: keep

#include "cel-c/config.h"
#include "cel-c/string_view.h"

CEL_BEGIN_DECLS

typedef enum CEL_ATTRIBUTE_OPEN_ENUM {
  cel_UnaryOp_kUnspecified = 0,
  cel_UnaryOp_kLogicalNot,
  cel_UnaryOp_kNegate,
  cel_UnaryOp_kNotStrictlyFalse,
} cel_UnaryOp;

// cel_UnaryOp_FromString
//
// Maps unary operator name to unary operator. Returns true on success and
// places the operator in op, otherwise returns false.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_UnaryOp_FromString(cel_StringView str,
                                       CEL_NONNULL(cel_UnaryOp*) op);

// cel_UnaryOp_ToString
//
// Returns the unary operator name.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_UnaryOp_ToString(cel_UnaryOp op);

// cel_UnaryOp_Precedence
//
// Returns the precedence for the unary operator. If precedence is not
// applicable to the operator, 0 is returned.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN int cel_UnaryOp_Precedence(cel_UnaryOp op);

typedef enum CEL_ATTRIBUTE_OPEN_ENUM {
  cel_BinaryOp_kUnspecified = 0,
  cel_BinaryOp_kLogicalAnd,
  cel_BinaryOp_kLogicalOr,
  cel_BinaryOp_kEquals,
  cel_BinaryOp_kNotEquals,
  cel_BinaryOp_kLess,
  cel_BinaryOp_kLessEquals,
  cel_BinaryOp_kGreater,
  cel_BinaryOp_kGreaterEquals,
  cel_BinaryOp_kAdd,
  cel_BinaryOp_kSubtract,
  cel_BinaryOp_kMultiply,
  cel_BinaryOp_kDivide,
  cel_BinaryOp_kModulo,
  cel_BinaryOp_kIndex,
  cel_BinaryOp_kIn,
  cel_BinaryOp_kOptIndex,
  cel_BinaryOp_kOptSelect,
} cel_BinaryOp;

// cel_BinaryOp_FromString
//
// Maps binary operator name to binary operator. Returns true on success and
// places the operator in op, otherwise returns false.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_BinaryOp_FromString(cel_StringView str,
                                        CEL_NONNULL(cel_BinaryOp*) op);

// cel_BinaryOp_ToString
//
// Returns the binary operator name.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_BinaryOp_ToString(cel_BinaryOp op);

// cel_BinaryOp_Precedence
//
// Returns the precedence for the binary operator. If precedence is not
// applicable to the operator, 0 is returned.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN int cel_BinaryOp_Precedence(cel_BinaryOp op);

typedef enum CEL_ATTRIBUTE_OPEN_ENUM {
  cel_TernaryOp_kUnspecified = 0,
  cel_TernaryOp_kConditional,
} cel_TernaryOp;

// cel_TernaryOp_FromString
//
// Maps ternary operator name to ternary operator. Returns true on success and
// places the operator in op, otherwise returns false.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_TernaryOp_FromString(cel_StringView str,
                                         CEL_NONNULL(cel_TernaryOp*) op);

// cel_TernaryOp_ToString
//
// Returns the ternary operator name.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringView cel_TernaryOp_ToString(cel_TernaryOp op);

// cel_TernaryOp_Precedence
//
// Returns the precedence for the ternary operator. If precedence is not
// applicable to the operator, 0 is returned.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN int cel_TernaryOp_Precedence(cel_TernaryOp op);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_OPERATORS_H_
