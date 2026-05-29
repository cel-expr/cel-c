// Copyright 2024 Google LLC
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

#ifndef THIRD_PARTY_CEL_C_ASSERT_H_
#define THIRD_PARTY_CEL_C_ASSERT_H_

#include <stdbool.h>  // IWYU pragma: keep

#include "cel-c/config.h"

#ifdef NDEBUG
#define _CEL_ASSERT_UNARY(condition, assertion, op) \
  (false ? CEL_USED(condition) : CEL_USED(0))
#define _CEL_ASSERT_BINARY(condition, lhs, op, rhs) \
  (false ? CEL_USED(condition) : CEL_USED(0))
#else
#define _CEL_ASSERT_UNARY(condition, assertion, op) \
  (CEL_LIKELY(condition)                            \
       ? CEL_USED(0)                                \
       : cel_AssertionFailed(__FILE__, __LINE__, op assertion))
#define _CEL_ASSERT_BINARY(condition, lhs, op, rhs) \
  (CEL_LIKELY(condition)                            \
       ? CEL_USED(0)                                \
       : cel_AssertionFailed(__FILE__, __LINE__, lhs " " op " " rhs))
#endif

// CEL_ASSERT
//
// Emits a debug assertion that `condition` is true. If `condition` evaluates to
// `false`, the program halts.
#define CEL_ASSERT(condition) _CEL_ASSERT_UNARY((condition), #condition, "")

// CEL_ASSERT_NOT
//
// Emits a debug assertion that `condition` is false. If `condition` evaluates
// to `true`, the program halts.
#define CEL_ASSERT_NOT(condition) \
  _CEL_ASSERT_UNARY(!(condition), #condition, "!")

// CEL_ASSERT_NOT_NULL
//
// Emits a debug assertion that `expression` is not `NULL`. If `condition`
// evaluates to `NULL`, the program halts.
#define CEL_ASSERT_NOT_NULL(expression) \
  _CEL_ASSERT_BINARY((expression) != cel_nullptr, #expression, "!=", "nullptr")

// CEL_ASSERT_NULL
//
// Emits a debug assertion that `expression` is `NULL`. If `condition` evaluates
// to not `NULL`, the program halts.
#define CEL_ASSERT_NULL(expression) \
  _CEL_ASSERT_BINARY((expression) == cel_nullptr, #expression, "==", "nullptr")

// CEL_ASSERT_EQ
//
// Emits a debug assertion that `lhs` is equal to `rhs`. If `condition`
// evaluates to false, the program halts.
#define CEL_ASSERT_EQ(lhs, rhs) \
  _CEL_ASSERT_BINARY((lhs) == (rhs), #lhs, "==", #rhs)

// CEL_ASSERT_NE
//
// Emits a debug assertion that `lhs` is not equal to `rhs`. If `condition`
// evaluates to false, the program halts.
#define CEL_ASSERT_NE(lhs, rhs) \
  _CEL_ASSERT_BINARY((lhs) != (rhs), #lhs, "!=", #rhs)

// CEL_ASSERT_LT
//
// Emits a debug assertion that `lhs` is less than `rhs`. If `condition`
// evaluates to false, the program halts.
#define CEL_ASSERT_LT(lhs, rhs) \
  _CEL_ASSERT_BINARY((lhs) < (rhs), #lhs, "<", #rhs)

// CEL_ASSERT_LE
//
// Emits a debug assertion that `lhs` is less than or equal to `rhs`. If
// `condition` evaluates to false, the program halts.
#define CEL_ASSERT_LE(lhs, rhs) \
  _CEL_ASSERT_BINARY((lhs) <= (rhs), #lhs, "<=", #rhs)

// CEL_ASSERT_GT
//
// Emits a debug assertion that `lhs` is greater than `rhs`. If `condition`
// evaluates to false, the program halts.
#define CEL_ASSERT_GT(lhs, rhs) \
  _CEL_ASSERT_BINARY((lhs) > (rhs), #lhs, ">", #rhs)

// CEL_ASSERT_GE
//
// Emits a debug assertion that `lhs` is greater than or equal to `rhs`. If
// `condition` evaluates to false, the program halts.
#define CEL_ASSERT_GE(lhs, rhs) \
  _CEL_ASSERT_BINARY((lhs) >= (rhs), #lhs, ">=", #rhs)

CEL_BEGIN_DECLS

CEL_ATTRIBUTE_NORETURN
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_AssertionFailed(CEL_NONNULL(const char*) file, int line,
                                    CEL_NONNULL(const char*) cond);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_ASSERT_H_
