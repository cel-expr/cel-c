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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_CONTAINER_H_
#define THIRD_PARTY_CEL_C_INTERNAL_CONTAINER_H_

#include <cstddef>
#include <utility>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/string.h"
#include "cel-c/string_view.h"

struct _cel_ContainerIterator {
  cel_StringView name = cel_StringView{.data = "", .size = 0};
  size_t size = 0;

  [[nodiscard]]
  bool HasNext() const {
    return size > 0;
  }

  [[nodiscard]]
  cel_StringView Next() {
    CEL_ASSERT_GT(size, 0);

    cel_StringView name = this->name;
    --size;
    const char* pos =
        cel_StringView_FindLast(this->name, cel_StringView_FromString("."));
    if (pos == cel_nullptr) {
      CEL_ASSERT_EQ(size, 0);
      pos = cel_StringView_Data(this->name);
    }
    cel_StringView_RemoveSuffix(&this->name, (cel_StringView_Data(this->name) +
                                              cel_StringView_Size(this->name)) -
                                                 pos);
    return name;
  }
};

class _cel_Container {
 public:
  _cel_Container() = default;
  _cel_Container(const _cel_Container&) = delete;
  _cel_Container& operator=(const _cel_Container&) = delete;

  CEL_INLINE _cel_Container(_cel_Container&& other) noexcept
      : name_(std::move(other.name_)), count_(std::exchange(other.count_, 0)) {}

  CEL_INLINE _cel_Container& operator=(_cel_Container&& other) noexcept {
    if (CEL_LIKELY(this != &other)) {
      name_ = std::move(other.name_);
      count_ = std::exchange(other.count_, 0);
    }
    return *this;
  }

  [[nodiscard]]
  CEL_INLINE bool empty() const {
    return count() == 0;
  }

  [[nodiscard]]
  CEL_INLINE size_t count() const {
    return count_;
  }

  [[nodiscard]]
  CEL_INLINE cel_StringView name() const CEL_ATTRIBUTE_LIFETIME_BOUND {
    return _cel_String_ToStringView(&name_);
  }

  [[nodiscard]]
  CEL_INLINE bool Update(cel_StringView name) {
    _cel_String_Clear(&name_);
    count_ = 0;
    _cel_String_Reserve(&name_, cel_StringView_Size(name));

    cel_StringViewTokenizer tokenizer =
        cel_StringView_Tokenize(name, cel_StringView_FromString("."));
    cel_StringView token;
    while (cel_StringViewTokenizer_Next(&tokenizer, &token)) {
      if (cel_StringView_Empty(token)) {
        continue;
      }
      ++count_;
      if (!_cel_String_Empty(&name_)) {
        if (!_cel_String_PushBack(&name_, '.')) {
          return false;
        }
      }
      if (!_cel_String_Append(&name_, token)) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]]
  CEL_INLINE _cel_ContainerIterator Iterate() const {
    _cel_ContainerIterator iter;
    iter.name = _cel_String_ToStringView(&name_);
    iter.size = count_;
    return iter;
  }

 private:
  _cel_String name_;
  size_t count_ = 0;
};

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_Container_Empty(
    const _cel_Container* cel_nonnull container) {
  return container->empty();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE size_t
_cel_Container_Count(const _cel_Container* cel_nonnull container) {
  return container->count();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_Container_Update(
    _cel_Container* cel_nonnull container, cel_StringView name) {
  return container->Update(name);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE _cel_ContainerIterator
_cel_Container_Iterate(const _cel_Container* cel_nonnull container) {
  return container->Iterate();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE bool _cel_ContainerIterator_HasNext(
    const _cel_ContainerIterator* cel_nonnull iter) {
  return iter->HasNext();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE CEL_INLINE cel_StringView
_cel_ContainerIterator_Next(_cel_ContainerIterator* cel_nonnull iter) {
  return iter->Next();
}

#endif  // THIRD_PARTY_CEL_C_INTERNAL_CONTAINER_H_
