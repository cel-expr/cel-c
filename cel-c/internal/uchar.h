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

// Internal header providing compatible typedefs for `uchar.h`. Some platforms,
// namely older versions of macOS, do not have them.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_UCHAR_H_
#define THIRD_PARTY_CEL_C_INTERNAL_UCHAR_H_

#include <stdint.h>  // IWYU pragma: keep

#include "cel-c/config.h"  // IWYU pragma: keep

#if !defined(__cplusplus) || __cplusplus < 201103L
CEL_BEGIN_DECLS
#ifdef __CHAR16_TYPE__
typedef __CHAR16_TYPE__ char16_t;
#else
typedef uint_least16_t char16_t;
#endif
#ifdef __CHAR32_TYPE__
typedef __CHAR32_TYPE__ char32_t;
#else
typedef uint_least32_t char32_t;
#endif
CEL_END_DECLS
#endif

#endif  // THIRD_PARTY_CEL_C_INTERNAL_UCHAR_H_
