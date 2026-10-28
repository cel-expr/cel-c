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

// Internal header providing a generic string implementation which uses
// allocator-based memory management.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_STRING_H_
#define THIRD_PARTY_CEL_C_INTERNAL_STRING_H_

#include <cstdarg>
#include <cstddef>
#include <type_traits>

#include "cel-c/alloc.h"
#include "cel-c/assert.h"
#include "cel-c/cstring_view.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/generic_string.h"
#include "cel-c/internal/trivial_relocation.hpp"
#include "cel-c/string_view.h"

class _cel_String : private _cel_GenericString {
 public:
  using value_type = char;
  using reference = char&;
  using const_reference = const char&;
  using pointer = char*;
  using const_pointer = const char*;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using iterator = pointer;
  using const_iterator = const_pointer;

  CEL_INLINE _cel_String() { _cel_GenericString_Construct(this); }

  _cel_String(const _cel_String&) = delete;

  CEL_INLINE _cel_String(_cel_String&& other) noexcept
      : _cel_GenericString(other) {
    _cel_GenericString_Construct(&other);
  }

  CEL_INLINE ~_cel_String() { Deallocate(); }

  _cel_String& operator=(const _cel_String&) = delete;

  CEL_INLINE _cel_String& operator=(_cel_String&& other) noexcept {
    if (CEL_LIKELY(this != &other)) {
      Deallocate();
      _cel_GenericString_Construct(&other);
    }
    return *this;
  }

  [[nodiscard]]
  CEL_INLINE bool empty() const {
    return _cel_GenericString_Empty(this);
  }

  [[nodiscard]]
  CEL_INLINE size_type size() const {
    return _cel_GenericString_Size(this);
  }

  [[nodiscard]]
  CEL_INLINE size_type int_size() const {
    return _cel_GenericString_SizeInt(this);
  }

  [[nodiscard]]
  CEL_INLINE size_type capacity() const {
    return _cel_GenericString_Capacity(this);
  }

  [[nodiscard]]
  CEL_INLINE pointer cel_nonnull data() CEL_ATTRIBUTE_LIFETIME_BOUND {
    return _cel_GenericString_MutableData(this);
  }

  [[nodiscard]]
  CEL_INLINE const_pointer cel_nonnull
  data() const CEL_ATTRIBUTE_LIFETIME_BOUND {
    return _cel_GenericString_Data(this);
  }

  [[nodiscard]]
  CEL_INLINE reference front() CEL_ATTRIBUTE_LIFETIME_BOUND {
    CEL_ASSERT_NOT(empty());
    return data()[0];
  }

  [[nodiscard]]
  CEL_INLINE const_reference front() const CEL_ATTRIBUTE_LIFETIME_BOUND {
    CEL_ASSERT_NOT(empty());
    return data()[0];
  }

  [[nodiscard]]
  CEL_INLINE reference back() CEL_ATTRIBUTE_LIFETIME_BOUND {
    CEL_ASSERT_NOT(empty());
    return data()[size() - 1];
  }

  [[nodiscard]]
  CEL_INLINE const_reference back() const CEL_ATTRIBUTE_LIFETIME_BOUND {
    CEL_ASSERT_NOT(empty());
    return data()[size() - 1];
  }

  CEL_INLINE void clear() { _cel_GenericString_Clear(this); }

  CEL_INLINE void reset() {
    _cel_GenericString_ResetAllocator(this, cel_DefaultAllocator);
  }

  CEL_INLINE bool shrink_to_fit() {
    return _cel_GenericString_ShrinkToFitAllocator(this, cel_DefaultAllocator);
  }

  [[nodiscard]]
  CEL_INLINE reference operator[](size_type idx) CEL_ATTRIBUTE_LIFETIME_BOUND {
    return at(idx);
  }

  [[nodiscard]]
  CEL_INLINE const_reference
  operator[](size_type idx) const CEL_ATTRIBUTE_LIFETIME_BOUND {
    return at(idx);
  }

  [[nodiscard]]
  CEL_INLINE reference at(size_type idx) CEL_ATTRIBUTE_LIFETIME_BOUND {
    CEL_ASSERT_LT(idx, size());
    return data()[idx];
  }

  [[nodiscard]]
  CEL_INLINE const_reference
  at(size_type idx) const CEL_ATTRIBUTE_LIFETIME_BOUND {
    CEL_ASSERT_LT(idx, size());
    return data()[idx];
  }

  [[nodiscard]]
  CEL_INLINE bool Assign(cel_StringView val) {
    return _cel_GenericString_AssignAllocator(this, cel_DefaultAllocator, val);
  }

  CEL_ATTRIBUTE_VFORMAT(2)
  [[nodiscard]]
  CEL_INLINE ptrdiff_t VAppendF(const char* cel_nonnull fmt, va_list args) {
    return _cel_GenericString_VAppendFAllocator(this, cel_DefaultAllocator, fmt,
                                                args);
  }

  [[nodiscard]]
  CEL_INLINE ptrdiff_t AppendF(const char* cel_nonnull fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const ptrdiff_t ret = VAppendF(fmt, args);
    va_end(args);
    return ret;
  }

  [[nodiscard]]
  CEL_INLINE bool Append(cel_StringView val) {
    return _cel_GenericString_AppendAllocator(this, cel_DefaultAllocator, val);
  }

  [[nodiscard]]
  CEL_INLINE bool PushBack(char val) {
    return _cel_GenericString_PushBackAllocator(this, cel_DefaultAllocator,
                                                val);
  }

  CEL_INLINE bool Reserve(size_t new_capacity) {
    return _cel_GenericString_ReserveAllocator(this, cel_DefaultAllocator,
                                               new_capacity);
  }

  [[nodiscard]]
  CEL_INLINE bool Stabilize() {
    return _cel_GenericString_StabilizeAllocator(this, cel_DefaultAllocator);
  }

  CEL_INLINE void Destabilize() {
    _cel_GenericString_DestabilizeAllocator(this, cel_DefaultAllocator);
  }

  [[nodiscard]]
  CEL_INLINE cel_StringView ToStringView() const CEL_ATTRIBUTE_LIFETIME_BOUND {
    return _cel_GenericString_ToStringView(this);
  }

  [[nodiscard]]
  CEL_INLINE cel_CStringView
  ToCStringView() const CEL_ATTRIBUTE_LIFETIME_BOUND {
    return _cel_GenericString_ToCStringView(this);
  }

 private:
  CEL_INLINE void Deallocate() {
    _cel_GenericString_DestructAllocator(this, cel_DefaultAllocator);
  }
};

template <>
struct cel_IsTriviallyRelocatable<_cel_String> : std::true_type {};

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE size_t
_cel_String_Size(CEL_NONNULL(const _cel_String*) str) {
  return str->size();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE int _cel_String_SizeInt(
    CEL_NONNULL(const _cel_String*) str) {
  return str->int_size();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_String_Empty(
    CEL_NONNULL(const _cel_String*) str) {
  return str->empty();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE size_t
_cel_String_Capacity(CEL_NONNULL(const _cel_String*) str) {
  return str->capacity();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE CEL_NONNULL(const char*)
    _cel_String_Data(CEL_NONNULL(const _cel_String*) str) {
  return str->data();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE CEL_NONNULL(char*)
    _cel_String_MutableData(CEL_NONNULL(_cel_String*) str) {
  return str->data();
}

CEL_DEPRECATE_AND_INLINE CEL_INLINE void _cel_String_Clear(
    CEL_NONNULL(_cel_String*) str) {
  str->clear();
}

CEL_DEPRECATE_AND_INLINE CEL_INLINE void _cel_String_Reset(
    CEL_NONNULL(_cel_String*) str) {
  str->reset();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_String_ShrinkToFit(
    CEL_NONNULL(_cel_String*) str) {
  return str->shrink_to_fit();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_String_Assign(
    CEL_NONNULL(_cel_String*) str, cel_StringView val) {
  return str->Assign(val);
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_VFORMAT(2)
CEL_DEPRECATE_AND_INLINE CEL_INLINE ptrdiff_t _cel_String_VAppendF(
    CEL_NONNULL(_cel_String*) str, CEL_NONNULL(const char*) fmt, va_list args) {
  return str->VAppendF(fmt, args);
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_FORMAT(2, 3)
[[deprecated]] CEL_INLINE ptrdiff_t _cel_String_AppendF(
    CEL_NONNULL(_cel_String*) str, CEL_NONNULL(const char*) fmt, ...) {
  va_list args;
  va_start(args, fmt);
  const ptrdiff_t ret = _cel_String_VAppendF(str, fmt, args);
  va_end(args);
  return ret;
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_String_Append(
    CEL_NONNULL(_cel_String*) str, cel_StringView val) {
  return str->Append(val);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_String_PushBack(
    CEL_NONNULL(_cel_String*) str, char val) {
  return str->PushBack(val);
}

CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_String_Reserve(
    CEL_NONNULL(_cel_String*) str, size_t capacity) {
  return str->Reserve(capacity);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_String_Stabilize(
    CEL_NONNULL(_cel_String*) str) {
  return str->Stabilize();
}

CEL_DEPRECATE_AND_INLINE CEL_INLINE void _cel_String_Destabilize(
    CEL_NONNULL(_cel_String*) str) {
  str->Destabilize();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE cel_StringView
_cel_String_ToStringView(CEL_NONNULL(const _cel_String*) str) {
  return str->ToStringView();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE cel_CStringView
_cel_String_ToCStringView(CEL_NONNULL(const _cel_String*) str) {
  return str->ToCStringView();
}

#endif  // THIRD_PARTY_CEL_C_INTERNAL_STRING_H_
