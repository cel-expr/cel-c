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

// Internal header providing macros to the fastest implementation of
// setjmp/longjmp, that is the implementation that avoids saving and restoring
// signal masks which requires syscalls.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_SETJMP_H_
#define THIRD_PARTY_CEL_C_INTERNAL_SETJMP_H_

#include <setjmp.h>  // IWYU pragma: keep

#include "cel-c/config.h"

#if CEL_HAVE_INCLUDE(<unistd.h>)
// Include <unistd.h> for _POSIX_VERSION.
#include <unistd.h>
#endif

CEL_BEGIN_DECLS

// setjmp/longjmp are not explicitly documented to save and restore the current
// signal mask, but in many implementations they do. This requires making
// syscalls. We do not need this feature, so we prefer using
// sigsetjmp/siglongjmp on platforms that have it and explicitly opt-out of from
// preserving the signal mask.
#if defined(_POSIX_VERSION) && _POSIX_VERSION >= 200112L
typedef sigjmp_buf _cel_jmp_buf;
#define _cel_setjmp(env) sigsetjmp((env), 0)
#define _cel_longjmp(env) siglongjmp((env), 1)
#else
typedef jmp_buf _cel_jmp_buf;
#define _cel_setjmp(env) setjmp(env)
#define _cel_longjmp(env) longjmp((env), 1)
#endif

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_SETJMP_H_
