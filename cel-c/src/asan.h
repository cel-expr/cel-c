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

// Internal header providing wrappers around the ASan interface.

#ifndef THIRD_PARTY_CEL_C_SRC_ASAN_H_
#define THIRD_PARTY_CEL_C_SRC_ASAN_H_

#include "cel-c/src/config.h"

#ifdef _CEL_HAVE_ASAN
#include <sanitizer/common_interface_defs.h>
#endif

CEL_BEGIN_DECLS

// _cel_sanitizer_annotate_contiguous_container
//
// When ASan is enabled calls `__sanitizer_annotate_contiguous_container`,
// otherwise does nothing.
#ifdef _CEL_HAVE_ASAN
#define _cel_sanitizer_annotate_contiguous_container(begin, end, old_mid, \
                                                     new_mid)             \
  __sanitizer_annotate_contiguous_container((begin), (end), (old_mid),    \
                                            (new_mid))
#else
static CEL_INLINE void _cel_sanitizer_annotate_contiguous_container(
    CEL_ATTRIBUTE_MAYBE_UNUSED CEL_NULLABILITY_UNKNOWN(const void*) begin,
    CEL_ATTRIBUTE_MAYBE_UNUSED CEL_NULLABILITY_UNKNOWN(const void*) end,
    CEL_ATTRIBUTE_MAYBE_UNUSED CEL_NULLABILITY_UNKNOWN(const void*) old_mid,
    CEL_ATTRIBUTE_MAYBE_UNUSED CEL_NULLABILITY_UNKNOWN(const void*) new_mid) {
  CEL_USED(begin);
  CEL_USED(end);
  CEL_USED(old_mid);
  CEL_USED(new_mid);
}
#define _cel_sanitizer_annotate_contiguous_container(begin, end, old_mid, \
                                                     new_mid)             \
  _cel_sanitizer_annotate_contiguous_container((begin), (end), (old_mid), \
                                               (new_mid))
#endif

// _cel_sanitizer_annotate_double_ended_contiguous_container
//
// When ASan is enabled calls
// `__sanitizer_annotate_double_ended_contiguous_container`, otherwise does
// nothing.
#ifdef _CEL_HAVE_ASAN
#define _cel_sanitizer_annotate_double_ended_contiguous_container(      \
    storage_begin, storage_end, old_container_begin, old_container_end, \
    new_container_begin, new_container_end)                             \
  __sanitizer_annotate_double_ended_contiguous_container(               \
      (storage_begin), (storage_end), (old_container_begin),            \
      (old_container_end), (new_container_begin), (new_container_end))
#else
static CEL_INLINE void
_cel_sanitizer_annotate_double_ended_contiguous_container(
    CEL_ATTRIBUTE_MAYBE_UNUSED CEL_NULLABILITY_UNKNOWN(const void*)
        storage_begin,
    CEL_ATTRIBUTE_MAYBE_UNUSED CEL_NULLABILITY_UNKNOWN(const void*) storage_end,
    CEL_ATTRIBUTE_MAYBE_UNUSED CEL_NULLABILITY_UNKNOWN(const void*)
        old_container_begin,
    CEL_ATTRIBUTE_MAYBE_UNUSED CEL_NULLABILITY_UNKNOWN(const void*)
        old_container_end,
    CEL_ATTRIBUTE_MAYBE_UNUSED CEL_NULLABILITY_UNKNOWN(const void*)
        new_container_begin,
    CEL_ATTRIBUTE_MAYBE_UNUSED CEL_NULLABILITY_UNKNOWN(const void*)
        new_container_end) {
  CEL_USED(storage_begin);
  CEL_USED(storage_end);
  CEL_USED(old_container_begin);
  CEL_USED(old_container_end);
  CEL_USED(new_container_begin);
  CEL_USED(new_container_end);
}
#define _cel_sanitizer_annotate_double_ended_contiguous_container(      \
    storage_begin, storage_end, old_container_begin, old_container_end, \
    new_container_begin, new_container_end)                             \
  _cel_sanitizer_annotate_double_ended_contiguous_container(            \
      (storage_begin), (storage_end), (old_container_begin),            \
      (old_container_end), (new_container_begin), (new_container_end))
#endif

CEL_END_DECLS

// _CEL_ATTRIBUTE_NO_SANITIZE_ADDRESS
//
// When ASan is enabled expands to an attribute which disables ASan
// instrumentation for the associated function otherwise expands to nothing.
#ifdef _CEL_HAVE_ASAN
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_ATTRIBUTE(no_sanitize)
#define _CEL_ATTRIBUTE_NO_SANITIZE_ADDRESS \
  __attribute__((no_sanitize("address")))
#elif defined(_MSC_VER)
#define _CEL_ATTRIBUTE_NO_SANITIZE_ADDRESS __declspec(no_sanitize_address)
#else
#error Unexpected compiler.
#endif
#else
#define _CEL_ATTRIBUTE_NO_SANITIZE_ADDRESS
#endif

#endif  // THIRD_PARTY_CEL_C_SRC_ASAN_H_
