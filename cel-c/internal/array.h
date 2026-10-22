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

// Internal header providing a dynamic array implementation which uses
// allocator-based memory management.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_ARRAY_H_
#define THIRD_PARTY_CEL_C_INTERNAL_ARRAY_H_

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#include "cel-c/alloc.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/generic_array.h"
#include "cel-c/internal/trivial_relocation.hpp"

template <typename T>
class cel_Array : private _cel_GenericArray {
 public:
  // T must also be trivially relocatable
  static_assert(cel_kIsTriviallyRelocatable<T>,
                "T must be trivially relocatable");
  static_assert(std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, T>,
                "T must be unqualified");

  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using iterator = pointer;
  using const_iterator = const_pointer;

  CEL_INLINE cel_Array() { _cel_GenericArray_Construct(this); }

  cel_Array(const cel_Array&) = delete;

  CEL_INLINE cel_Array(cel_Array&& other) noexcept
      : _cel_GenericArray{std::exchange(other.ptr, nullptr),
                          std::exchange(other.len, 0),
                          std::exchange(other.cap, 0)} {}

  CEL_INLINE ~cel_Array() { DestroyAndDeallocate(); }

  cel_Array& operator=(const cel_Array&) = delete;

  CEL_INLINE cel_Array& operator=(cel_Array&& other) noexcept {
    if (CEL_LIKELY(this != &other)) {
      DestroyAndDeallocate();
      ptr = std::exchange(other.ptr, nullptr);
      len = std::exchange(other.len, 0);
      cap = std::exchange(other.cap, 0);
    }
    return *this;
  }

  [[nodiscard]]
  CEL_INLINE bool empty() const {
    return size() == 0;
  }

  [[nodiscard]]
  CEL_INLINE size_type size() const {
    return len;
  }

  [[nodiscard]]
  CEL_INLINE size_type capacity() const {
    return cap;
  }

  [[nodiscard]]
  CEL_INLINE pointer cel_nullability_unknown data()
      CEL_ATTRIBUTE_LIFETIME_BOUND {
    return reinterpret_cast<T*>(ptr);
  }

  [[nodiscard]]
  CEL_INLINE const_pointer cel_nullability_unknown
  data() const CEL_ATTRIBUTE_LIFETIME_BOUND {
    return reinterpret_cast<const T*>(ptr);
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
  CEL_INLINE pointer cel_nonnull Front() CEL_ATTRIBUTE_LIFETIME_BOUND {
    CEL_ASSERT_NOT(empty());
    return data();
  }

  [[nodiscard]]
  CEL_INLINE const_pointer cel_nonnull
  Front() const CEL_ATTRIBUTE_LIFETIME_BOUND {
    CEL_ASSERT_NOT(empty());
    return data();
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

  [[nodiscard]]
  CEL_INLINE pointer cel_nonnull Back() CEL_ATTRIBUTE_LIFETIME_BOUND {
    CEL_ASSERT_NOT(empty());
    return data() + size() - 1;
  }

  [[nodiscard]]
  CEL_INLINE const_pointer cel_nonnull
  Back() const CEL_ATTRIBUTE_LIFETIME_BOUND {
    CEL_ASSERT_NOT(empty());
    return data() + size() - 1;
  }

  [[nodiscard]]
  CEL_INLINE pointer PushBack() CEL_ATTRIBUTE_LIFETIME_BOUND {
    return Append(1);
  }

  [[nodiscard]]
  CEL_INLINE pointer Append(size_type n) CEL_ATTRIBUTE_LIFETIME_BOUND {
    pointer ele = reinterpret_cast<pointer>(_cel_GenericArray_AppendAllocator(
        this, cel_DefaultAllocator, n, sizeof(T)));
    if (ele != nullptr) {
      std::uninitialized_default_construct_n(ele, n);
    }
    return ele;
  }

  CEL_INLINE void PopBack() {
    std::destroy_at(&back());
    _cel_GenericArray_PopAllocator(this, sizeof(T));
  }

  CEL_INLINE bool Reserve(size_type new_capacity) {
    return _cel_GenericArray_ReserveAllocator(this, cel_DefaultAllocator,
                                              new_capacity, sizeof(T));
  }

  [[nodiscard]]
  CEL_INLINE pointer Resize(size_type new_size) CEL_ATTRIBUTE_LIFETIME_BOUND {
    const size_t old_size = size();
    if (new_size < old_size) {
      // Shrinking always succeeds.
      std::destroy_n(data() + new_size, old_size - new_size);
      return reinterpret_cast<pointer>(_cel_GenericArray_ResizeAllocator(
          this, cel_DefaultAllocator, new_size, sizeof(T)));
    }
    pointer ele = reinterpret_cast<pointer>(_cel_GenericArray_ResizeAllocator(
        this, cel_DefaultAllocator, new_size, sizeof(T)));
    if (ele != nullptr) {
      std::uninitialized_default_construct_n(data() + old_size,
                                             new_size - old_size);
    }
    return ele;
  }

  CEL_INLINE iterator erase(const_iterator pos) CEL_ATTRIBUTE_LIFETIME_BOUND {
    return erase(static_cast<size_type>(pos - data()));
  }

  CEL_INLINE iterator erase(size_type idx) CEL_ATTRIBUTE_LIFETIME_BOUND {
    std::destroy_at(data() + idx);
    _cel_GenericArray_EraseAllocator(this, idx, sizeof(T));
    return begin() + idx;
  }

  CEL_INLINE void shrink_to_fit() {
    _cel_GenericArray_ShrinkToFitAllocator(this, cel_DefaultAllocator,
                                           sizeof(T));
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
  CEL_INLINE pointer cel_nonnull At(size_type idx)
      CEL_ATTRIBUTE_LIFETIME_BOUND {
    CEL_ASSERT_LT(idx, size());
    return data() + idx;
  }

  [[nodiscard]]
  CEL_INLINE const_pointer cel_nonnull
  At(size_type idx) const CEL_ATTRIBUTE_LIFETIME_BOUND {
    CEL_ASSERT_LT(idx, size());
    return data() + idx;
  }

  CEL_INLINE void clear() {
    Destroy();
    len = 0;
  }

  CEL_INLINE void reset() {
    DestroyAndDeallocate();
    ptr = nullptr;
    len = 0;
    cap = 0;
  }

  [[nodiscard]]
  CEL_INLINE iterator begin() CEL_ATTRIBUTE_LIFETIME_BOUND {
    return data();
  }

  [[nodiscard]]
  CEL_INLINE const_iterator begin() const CEL_ATTRIBUTE_LIFETIME_BOUND {
    return data();
  }

  [[nodiscard]]
  CEL_INLINE iterator end() CEL_ATTRIBUTE_LIFETIME_BOUND {
    return data() + size();
  }

  [[nodiscard]]
  CEL_INLINE const_iterator end() const CEL_ATTRIBUTE_LIFETIME_BOUND {
    return data() + size();
  }

  friend CEL_INLINE void swap(cel_Array& lhs, cel_Array& rhs) noexcept {
    using std::swap;
    swap(lhs.ptr, rhs.ptr);
    swap(lhs.len, rhs.len);
    swap(lhs.cap, rhs.cap);
  }

 private:
  CEL_INLINE void Destroy() { std::destroy_n(data(), size()); }

  CEL_INLINE void Deallocate() {
    _cel_GenericArray_DestructAllocator(this, cel_DefaultAllocator, sizeof(T));
  }

  CEL_INLINE void DestroyAndDeallocate() {
    Destroy();
    Deallocate();
  }
};

template <typename T>
struct cel_IsTriviallyRelocatable<cel_Array<T>> : std::true_type {};

#define _cel_Array(type) cel_Array<type>

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE size_t
_cel_Array_Size(const cel_Array<T>* cel_nonnull arr) {
  return arr->size();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_Array_Empty(
    const cel_Array<T>* cel_nonnull arr) {
  return arr->empty();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE size_t
_cel_Array_Capacity(const cel_Array<T>* cel_nonnull arr) {
  return arr->capacity();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE const T* cel_nullable
_cel_Array_Data(const cel_Array<T>* cel_nonnull arr) {
  return arr->data();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE T* cel_nullable
_cel_Array_MutableData(cel_Array<T>* cel_nonnull arr) {
  return arr->data();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE const T* cel_nonnull
_cel_Array_At(const cel_Array<T>* cel_nonnull arr, size_t idx) {
  return arr->At(idx);
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE T* cel_nonnull
_cel_Array_MutableAt(cel_Array<T>* cel_nonnull arr, size_t idx) {
  return arr->At(idx);
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE const T* cel_nonnull
_cel_Array_Front(const cel_Array<T>* cel_nonnull arr) {
  return arr->Front();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE T* cel_nonnull
_cel_Array_MutableFront(cel_Array<T>* cel_nonnull arr) {
  return arr->Front();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE const T* cel_nonnull
_cel_Array_Back(const cel_Array<T>* cel_nonnull arr) {
  return arr->Back();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE T* cel_nonnull
_cel_Array_MutableBack(cel_Array<T>* cel_nonnull arr) {
  return arr->Back();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE T* cel_nullable
_cel_Array_Push(cel_Array<T>* cel_nonnull arr) {
  return arr->PushBack();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE void _cel_Array_Pop(
    cel_Array<T>* cel_nonnull arr) {
  return arr->PopBack();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE T* cel_nullable
_cel_Array_Append(cel_Array<T>* cel_nonnull arr, size_t n) {
  return arr->Append(n);
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE void _cel_Array_Erase(
    cel_Array<T>* cel_nonnull arr, size_t idx) {
  arr->erase(idx);
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_Array_Reserve(
    cel_Array<T>* cel_nonnull arr, size_t new_cap) {
  return arr->Reserve(new_cap);
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE T* cel_nullable
_cel_Array_Resize(cel_Array<T>* cel_nonnull arr, size_t new_len) {
  return arr->Resize(new_len);
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE void _cel_Array_ShrinkToFit(
    cel_Array<T>* cel_nonnull arr) {
  arr->shrink_to_fit();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE void _cel_Array_Reset(
    cel_Array<T>* cel_nonnull arr) {
  arr->reset();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE void _cel_Array_Clear(
    cel_Array<T>* cel_nonnull arr) {
  arr->clear();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE const T* cel_nullable
_cel_Array_Begin(const cel_Array<T>* cel_nonnull arr) {
  return arr->begin();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE T* cel_nullable
_cel_Array_MutableBegin(cel_Array<T>* cel_nonnull arr) {
  return arr->begin();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE const T* cel_nullable
_cel_Array_End(const cel_Array<T>* cel_nonnull arr) {
  return arr->end();
}

template <typename T>
CEL_DEPRECATE_AND_INLINE CEL_INLINE T* cel_nullable
_cel_Array_MutableEnd(cel_Array<T>* cel_nonnull arr) {
  return arr->end();
}

#endif  // THIRD_PARTY_CEL_C_INTERNAL_ARRAY_H_
