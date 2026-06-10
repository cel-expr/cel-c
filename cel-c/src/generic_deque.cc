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

#include "cel-c/src/generic_deque.h"

#include <stdalign.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>
#include <string.h>

#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/src/align.h"
#include "cel-c/src/asan.h"

typedef struct _cel_GenericDequeBlock _cel_GenericDequeBlock;

struct _cel_GenericDequeBlock {
  CEL_NULLABLE(struct _cel_GenericDequeBlock*) prev;
  CEL_NULLABLE(struct _cel_GenericDequeBlock*) next;
  size_t cap;
};

static void _cel_GenericDequeBlock_Size(size_t ele_size,
                                        CEL_NONNULL(size_t*) block_size) {
  *block_size =
      ((ele_size <
        ((4096 - _cel_align_up(sizeof(_cel_GenericDequeBlock), cel_kMaxAlign)) /
         16))
           ? (4096 -
              _cel_align_up(sizeof(_cel_GenericDequeBlock), cel_kMaxAlign)) /
                 ele_size * ele_size
           : (16 * ele_size)) +
      _cel_align_up(sizeof(_cel_GenericDequeBlock), cel_kMaxAlign);
}

static void _cel_GenericDequeBlock_Annotate(CEL_NONNULL(_cel_GenericDequeBlock*)
                                                block,
                                            size_t old_begin, size_t old_end,
                                            size_t new_begin, size_t new_end,
                                            size_t ele_size) {
  const size_t size = block->cap * ele_size;
  CEL_NONNULL(char*) addr = ((CEL_NONNULL(char*))block);
  addr += _cel_align_up(sizeof(_cel_GenericDequeBlock), cel_kMaxAlign);
  _cel_sanitizer_annotate_double_ended_contiguous_container(
      addr, addr + size, addr + (old_begin * ele_size),
      addr + (old_end * ele_size), addr + (new_begin * ele_size),
      addr + (new_end * ele_size));
}

static void _cel_GenericDequeBlock_AnnotateNew(
    CEL_NONNULL(_cel_GenericDequeBlock*) block, size_t ele_size) {
  _cel_GenericDequeBlock_Annotate(block, /*old_begin=*/0,
                                  /*old_end=*/block->cap, /*new_begin=*/0,
                                  /*new_end=*/0, ele_size);
}

static void _cel_GenericDequeBlock_AnnotateDelete(
    CEL_NONNULL(_cel_GenericDequeBlock*) block, size_t ele_size) {
  _cel_GenericDequeBlock_Annotate(block, /*old_begin=*/0,
                                  /*old_end=*/0, /*new_begin=*/0,
                                  /*new_end=*/block->cap, ele_size);
}

static CEL_NULLABLE(_cel_GenericDequeBlock*)
    _cel_GenericDequeBlock_NewAllocator(CEL_NONNULL(cel_Allocator*) alloc,
                                        size_t ele_size) {
  size_t block_size;
  _cel_GenericDequeBlock_Size(ele_size, &block_size);
  size_t actual_block_size;
  CEL_NULLABLE(void*)
  addr = cel_Allocator_Malloc(alloc, block_size, &actual_block_size);
  if (CEL_UNLIKELY(addr == cel_nullptr)) {
    return cel_nullptr;
  }
  const size_t cap =
      (actual_block_size -
       _cel_align_up(sizeof(_cel_GenericDequeBlock), cel_kMaxAlign)) /
      ele_size;
  CEL_NONNULL(_cel_GenericDequeBlock*) block;
  block = reinterpret_cast<_cel_GenericDequeBlock*>(addr);
  block->prev = block->next = cel_nullptr;
  block->cap = cap;
  return block;
}

static void _cel_GenericDequeBlock_DeleteAllocator(
    CEL_NONNULL(_cel_GenericDequeBlock*) block,
    CEL_NONNULL(cel_Allocator*) alloc, size_t ele_size) {
  const size_t size = block->cap * ele_size;
  CEL_NONNULL(void*) addr;
  addr = block;
  cel_Allocator_FreeSized(
      alloc, addr,
      size + _cel_align_up(sizeof(_cel_GenericDequeBlock), cel_kMaxAlign));
}

static CEL_NULLABLE(_cel_GenericDequeBlock*)
    _cel_GenericDequeBlock_NewArena(CEL_NONNULL(cel_Arena*) arena,
                                    size_t ele_size) {
  size_t block_size;
  _cel_GenericDequeBlock_Size(ele_size, &block_size);
  CEL_NULLABLE(void*)
  addr = cel_Arena_Malloc(arena, block_size, cel_nullptr);
  if (CEL_UNLIKELY(addr == cel_nullptr)) {
    return cel_nullptr;
  }
  const size_t cap = (block_size - _cel_align_up(sizeof(_cel_GenericDequeBlock),
                                                 cel_kMaxAlign)) /
                     ele_size;
  CEL_NONNULL(_cel_GenericDequeBlock*) block;
  block = reinterpret_cast<_cel_GenericDequeBlock*>(addr);
  block->prev = block->next = cel_nullptr;
  block->cap = cap;
  return block;
}

static CEL_NONNULL(const char*)
    _cel_GenericDequeBlock_At(CEL_NONNULL(const _cel_GenericDequeBlock*) block,
                              size_t index, size_t ele_size) {
  return ((CEL_NONNULL(const char*))block) +
         _cel_align_up(sizeof(_cel_GenericDequeBlock), cel_kMaxAlign) +
         (index * ele_size);
}

static CEL_NONNULL(char*)
    _cel_GenericDequeBlock_MutableAt(CEL_NONNULL(_cel_GenericDequeBlock*) block,
                                     size_t index, size_t ele_size) {
  return (CEL_NONNULL(char*))_cel_GenericDequeBlock_At(block, index, ele_size);
}

static CEL_NONNULL(_cel_GenericDequeBlock*)
    _cel_GenericDequeBlock_LinkBefore(CEL_NONNULL(_cel_GenericDequeBlock*)
                                          new_block,
                                      CEL_NULLABLE(_cel_GenericDequeBlock*)
                                          before_block) {
  CEL_NULLABLE(_cel_GenericDequeBlock*) prev;
  if (before_block != cel_nullptr) {
    prev = before_block->prev;
  } else {
    prev = cel_nullptr;
  }
  if (prev != cel_nullptr) {
    prev->next = new_block;
  }
  new_block->prev = prev;
  new_block->next = before_block;
  if (before_block != cel_nullptr) {
    before_block->prev = new_block;
  }
  return new_block;
}

static CEL_NONNULL(_cel_GenericDequeBlock*)
    _cel_GenericDequeBlock_LinkAfter(CEL_NONNULL(_cel_GenericDequeBlock*)
                                         new_block,
                                     CEL_NONNULL(_cel_GenericDequeBlock*)
                                         after_block) {
  CEL_NULLABLE(_cel_GenericDequeBlock*) next;
  if (after_block != cel_nullptr) {
    next = after_block->next;
  } else {
    next = cel_nullptr;
  }
  if (next != cel_nullptr) {
    next->prev = new_block;
  }
  new_block->prev = after_block;
  new_block->next = next;
  if (after_block != cel_nullptr) {
    after_block->next = new_block;
  }
  return new_block;
}

static void _cel_GenericDequeBlock_Unlink(CEL_NONNULL(_cel_GenericDequeBlock*)
                                              block) {
  CEL_NULLABLE(_cel_GenericDequeBlock*) prev = block->prev;
  CEL_NULLABLE(_cel_GenericDequeBlock*) next = block->next;
  if (prev != cel_nullptr) {
    prev->next = next;
    block->prev = cel_nullptr;
  }
  if (next != cel_nullptr) {
    next->prev = prev;
    block->next = cel_nullptr;
  }
}

static void _cel_GenericDequeBlock_DeleteChainAllocator(
    CEL_NULLABLE(_cel_GenericDequeBlock*) block,
    CEL_NONNULL(cel_Allocator*) alloc, size_t ele_size) {
  while (block != cel_nullptr) {
    CEL_NULLABLE(_cel_GenericDequeBlock*) next = block->next;
    _cel_GenericDequeBlock_DeleteAllocator(block, alloc, ele_size);
    block = next;
  }
}

static bool _cel_GenericDeque_NewBlockAllocator(
    CEL_NONNULL(_cel_GenericDeque*) deq, int which,
    CEL_NONNULL(cel_Allocator*) alloc, size_t ele_size) {
  CEL_ASSERT(which == -1 || which == 0 || which == 1);

  // First insertion. There may be a single block we can use at deq->cache,
  // otherwise we need to allocate one.

  CEL_NULLABLE(_cel_GenericDequeBlock*)
  new_block = deq->cache;
  if (new_block != cel_nullptr) {
    // allocator-based deque should only cache a single block.
    CEL_ASSERT_NULL(new_block->prev);
    CEL_ASSERT_NULL(new_block->next);
    deq->cache = cel_nullptr;
    // No need to update ASan annotation. Blocks in the cache are already
    // poisoned fully.
  } else {
    new_block = _cel_GenericDequeBlock_NewAllocator(alloc, ele_size);
    if (CEL_UNLIKELY(new_block == cel_nullptr)) {
      return false;
    }
    _cel_GenericDequeBlock_AnnotateNew(new_block, ele_size);
  }
  CEL_ASSUME(which == -1 || which == 0 || which == 1);
  switch (which) {
    case -1:
      deq->head = _cel_GenericDequeBlock_LinkBefore(new_block, deq->head);
      deq->head_pos = new_block->cap;
      break;
    case 0:
      deq->head = deq->tail = new_block;
      deq->tail_pos = deq->head_pos = new_block->cap / 2;
      break;
    case 1:
      deq->tail = _cel_GenericDequeBlock_LinkAfter(new_block, deq->tail);
      deq->tail_pos = 0;
      break;
    default:
      CEL_UNREACHABLE();
  }
  return true;
}

static bool _cel_GenericDeque_NewHeadBlockAllocator(
    CEL_NONNULL(_cel_GenericDeque*) deq, CEL_NONNULL(cel_Allocator*) alloc,
    size_t ele_size) {
  CEL_ASSERT_EQ(deq->head_pos, 0);

  return _cel_GenericDeque_NewBlockAllocator(
      deq, deq->head != cel_nullptr ? -1 : 0, alloc, ele_size);
}

static bool _cel_GenericDeque_NewTailBlockAllocator(
    CEL_NONNULL(_cel_GenericDeque*) deq, CEL_NONNULL(cel_Allocator*) alloc,
    size_t ele_size) {
  CEL_ASSERT(deq->tail == cel_nullptr ||
             deq->tail_pos ==
                 ((CEL_NONNULL(_cel_GenericDequeBlock*))deq->tail)->cap);

  return _cel_GenericDeque_NewBlockAllocator(
      deq, deq->tail != cel_nullptr ? 1 : 0, alloc, ele_size);
}

static bool _cel_GenericDeque_NewBlockArena(CEL_NONNULL(_cel_GenericDeque*) deq,
                                            int which,
                                            CEL_NONNULL(cel_Arena*) arena,
                                            size_t ele_size) {
  CEL_ASSERT(which == -1 || which == 0 || which == 1);

  // First insertion. There may be a single block we can use at deq->cache,
  // otherwise we need to allocate one.

  CEL_NULLABLE(_cel_GenericDequeBlock*)
  new_block = deq->cache;
  if (new_block != cel_nullptr) {
    // allocator-based deque should only cache a single block.
    deq->cache = new_block->next;
    _cel_GenericDequeBlock_Unlink(new_block);
  } else {
    new_block = _cel_GenericDequeBlock_NewArena(arena, ele_size);
    if (CEL_UNLIKELY(new_block == cel_nullptr)) {
      return false;
    }
  }
  CEL_ASSUME(which == -1 || which == 0 || which == 1);
  switch (which) {
    case -1:
      deq->head = _cel_GenericDequeBlock_LinkBefore(new_block, deq->head);
      deq->head_pos = new_block->cap;
      break;
    case 0:
      deq->head = deq->tail = new_block;
      deq->tail_pos = deq->head_pos = new_block->cap / 2;
      break;
    case 1:
      deq->tail = _cel_GenericDequeBlock_LinkAfter(new_block, deq->tail);
      deq->tail_pos = 0;
      break;
    default:
      CEL_UNREACHABLE();
  }
  return true;
}

static bool _cel_GenericDeque_NewHeadBlockArena(CEL_NONNULL(_cel_GenericDeque*)
                                                    deq,
                                                CEL_NONNULL(cel_Arena*) arena,
                                                size_t ele_size) {
  CEL_ASSERT_EQ(deq->head_pos, 0);

  return _cel_GenericDeque_NewBlockArena(deq, deq->head != cel_nullptr ? -1 : 0,
                                         arena, ele_size);
}

static bool _cel_GenericDeque_NewTailBlockArena(CEL_NONNULL(_cel_GenericDeque*)
                                                    deq,
                                                CEL_NONNULL(cel_Arena*) arena,
                                                size_t ele_size) {
  CEL_ASSERT(deq->tail == cel_nullptr ||
             deq->tail_pos ==
                 ((CEL_NONNULL(_cel_GenericDequeBlock*))deq->tail)->cap);

  return _cel_GenericDeque_NewBlockArena(deq, deq->tail != cel_nullptr ? 1 : 0,
                                         arena, ele_size);
}

extern "C" void _cel_GenericDeque_Construct(CEL_NONNULL(_cel_GenericDeque*)
                                                deq) {
  CEL_ASSERT_NOT_NULL(deq);

  memset(deq, '\0', sizeof(*deq));
}

extern "C" void _cel_GenericDeque_DestructAllocator(
    CEL_NONNULL(_cel_GenericDeque*) deq, CEL_NONNULL(cel_Allocator*) alloc,
    size_t ele_size) {
  CEL_ASSERT_NOT_NULL(deq);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_GT(ele_size, 0);

  _cel_GenericDequeBlock_DeleteChainAllocator(deq->head, alloc, ele_size);
  _cel_GenericDequeBlock_DeleteChainAllocator(deq->cache, alloc, ele_size);
}

extern "C" CEL_NULLABLE(const void*)
    _cel_GenericDeque_At(CEL_NONNULL(const _cel_GenericDeque*) deq, size_t idx,
                         size_t ele_size) {
  CEL_ASSERT_NOT_NULL(deq);
  CEL_ASSERT_LT(idx, _cel_GenericDeque_Size(deq));
  CEL_ASSERT_GT(ele_size, 0);

  // We scan forward is idx < half the size, backwards otherwise.
  if (idx >= deq->len / 2) {
    // Backward.
    _cel_GenericDequeBlock* block = deq->tail;
    if (block == deq->head) {
      // Only one block, the element should be relative to head_pos.
      return _cel_GenericDequeBlock_At(block, deq->head_pos + idx, ele_size);
    }
    size_t npos = deq->len - deq->tail_pos;
    if (npos <= idx) {
      return _cel_GenericDequeBlock_At(block, idx - npos, ele_size);
    }
    block = block->prev;
    while (block != deq->head && npos - block->cap > idx) {
      npos -= block->cap;
      block = block->prev;
    }
    if (block == deq->head) {
      // Only one block, the element should be relative to head_pos.
      return _cel_GenericDequeBlock_At(block, deq->head_pos + idx, ele_size);
    }
    return _cel_GenericDequeBlock_At(block, idx - npos, ele_size);
  }

  // Forward.
  _cel_GenericDequeBlock* block = deq->head;
  size_t block_size;
  if (block == deq->tail || (block_size = (block->cap - deq->head_pos)) > idx) {
    // Only one block or the element is in the head block, the element should be
    // here relative to head_pos.
    return _cel_GenericDequeBlock_At(block, deq->head_pos + idx, ele_size);
  }
  idx -= block_size;
  block = block->next;
  while (block != deq->tail && (block_size = block->cap) >= idx) {
    idx -= block_size;
    block = block->next;
  }
  // Must be in this block which is not head (could be in the middle or tail).
  // So we always look relative to 0. deq->tail_pos is the upper bound,
  // everything before it is occupied.
  return _cel_GenericDequeBlock_At(block, idx, ele_size);
}

extern "C" CEL_NULLABLE(void*)
    _cel_GenericDeque_PushFrontAllocator(CEL_NONNULL(_cel_GenericDeque*) deq,
                                         CEL_NONNULL(cel_Allocator*) alloc,
                                         size_t ele_size) {
  CEL_ASSERT_NOT_NULL(deq);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_GT(ele_size, 0);

  CEL_NULLABLE(_cel_GenericDequeBlock*) head = deq->head;
  if (CEL_UNLIKELY(
          head == cel_nullptr ||
          (deq->head_pos == 0 && (head != deq->tail || deq->tail_pos != 0)))) {
    if (!_cel_GenericDeque_NewHeadBlockAllocator(deq, alloc, ele_size)) {
      return cel_nullptr;
    }
    head = deq->head;
  } else if (head == deq->tail && deq->head_pos == deq->tail_pos) {
    deq->head_pos = deq->tail_pos = head->cap / 2;
  }

  CEL_ASSERT_NOT_NULL(head);
  CEL_ASSERT_GT(deq->head_pos, 0);

  CEL_NONNULL(void*)
  ele = _cel_GenericDequeBlock_MutableAt(head, deq->head_pos - 1, ele_size);
  // Before updating fields and returning to the caller, update the ASan
  // annotation to unpoison the storage for the newly appended element. Keep in
  // mind that deq->tail may be the same as deq->head.
  _cel_GenericDequeBlock_Annotate(
      head, /*old_begin=*/deq->head_pos,
      /*old_end=*/head != deq->tail
          ? ((CEL_NONNULL(_cel_GenericDequeBlock*))deq->tail)->cap
          : deq->tail_pos,
      /*new_begin=*/deq->head_pos - 1,
      /*new_end=*/head != deq->tail
          ? ((CEL_NONNULL(_cel_GenericDequeBlock*))deq->tail)->cap
          : deq->tail_pos,
      ele_size);
  --(deq->head_pos);
  ++(deq->len);
  return ele;
}

extern "C" CEL_NULLABLE(void*)
    _cel_GenericDeque_PushFrontArena(CEL_NONNULL(_cel_GenericDeque*) deq,
                                     CEL_NONNULL(cel_Arena*) arena,
                                     size_t ele_size) {
  CEL_ASSERT_NOT_NULL(deq);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_GT(ele_size, 0);

  CEL_NULLABLE(_cel_GenericDequeBlock*) head = deq->head;
  if (CEL_UNLIKELY(
          head == cel_nullptr ||
          (deq->head_pos == 0 && (head != deq->tail || deq->tail_pos != 0)))) {
    if (!_cel_GenericDeque_NewHeadBlockArena(deq, arena, ele_size)) {
      return cel_nullptr;
    }
    head = deq->head;
  } else if (head == deq->tail && deq->head_pos == deq->tail_pos) {
    deq->head_pos = deq->tail_pos = head->cap / 2;
  }

  CEL_ASSERT_NOT_NULL(head);
  CEL_ASSERT_GT(deq->head_pos, 0);

  CEL_NONNULL(void*)
  ele = _cel_GenericDequeBlock_MutableAt(head, deq->head_pos - 1, ele_size);
  --(deq->head_pos);
  ++(deq->len);
  return ele;
}

extern "C" void _cel_GenericDeque_PopFrontAllocator(
    CEL_NONNULL(_cel_GenericDeque*) deq, CEL_NONNULL(cel_Allocator*) alloc,
    size_t ele_size) {
  CEL_ASSERT_NOT_NULL(deq);
  CEL_ASSERT_NOT(_cel_GenericDeque_Empty(deq));
  CEL_ASSERT_GT(ele_size, 0);
  CEL_ASSERT_NOT_NULL(deq->tail);

  // Before updating fields, update the ASan annotation. Keep in
  // mind that deq->tail may be the same as deq->head.
  _cel_GenericDequeBlock_Annotate(
      deq->head, /*old_begin=*/deq->head_pos,
      /*old_end=*/deq->head != deq->tail ? deq->head->cap : deq->tail_pos,
      /*new_begin=*/deq->head_pos + 1,
      /*new_end=*/deq->head != deq->tail ? deq->head->cap : deq->tail_pos,
      ele_size);

  ++(deq->head_pos);
  --(deq->len);
  if (deq->head != deq->tail && deq->head_pos == deq->head->cap) {
    CEL_NONNULL(_cel_GenericDequeBlock*) old_head = deq->head;
    CEL_NONNULL(_cel_GenericDequeBlock*) new_head = old_head->next;
    deq->head = new_head;
    deq->head_pos = 0;
    _cel_GenericDequeBlock_Unlink(old_head);
    if (deq->cache == cel_nullptr) {
      deq->cache = old_head;
    } else {
      // Block already cached. Delete this one.
      _cel_GenericDequeBlock_DeleteAllocator(old_head, alloc, ele_size);
    }
  }
  if (deq->head == deq->tail && deq->head_pos == deq->tail_pos) {
    if (deq->cache != cel_nullptr) {
      _cel_GenericDequeBlock_AnnotateDelete(deq->cache, ele_size);
      _cel_GenericDequeBlock_DeleteAllocator(deq->cache, alloc, ele_size);
      deq->cache = cel_nullptr;
    }
    deq->head_pos = deq->tail_pos = deq->head->cap / 2;
  }
}

extern "C" void _cel_GenericDeque_PopFrontArena(CEL_NONNULL(_cel_GenericDeque*)
                                                    deq) {
  CEL_ASSERT_NOT_NULL(deq);
  CEL_ASSERT_NOT(_cel_GenericDeque_Empty(deq));
  CEL_ASSERT_NOT_NULL(deq->head);

  ++(deq->head_pos);
  --(deq->len);
  if (deq->head != deq->tail && deq->head_pos == deq->head->cap) {
    CEL_NONNULL(_cel_GenericDequeBlock*) old_head = deq->head;
    CEL_NONNULL(_cel_GenericDequeBlock*) new_head = old_head->next;
    deq->head = new_head;
    deq->head_pos = 0;
    _cel_GenericDequeBlock_Unlink(old_head);
    _cel_GenericDequeBlock_LinkBefore(old_head, deq->cache);
    deq->cache = old_head;
  }
  if (deq->head == deq->tail && deq->head_pos == deq->tail_pos) {
    deq->head_pos = deq->tail_pos = deq->head->cap / 2;
  }
}

extern "C" CEL_NONNULL(const void*)
    _cel_GenericDeque_PeekFront(CEL_NONNULL(const _cel_GenericDeque*) deq,
                                size_t ele_size) {
  CEL_ASSERT_NOT_NULL(deq);
  CEL_ASSERT_NOT(_cel_GenericDeque_Empty(deq));
  CEL_ASSERT_GT(ele_size, 0);

  CEL_ASSERT_NOT_NULL(deq->head);
  CEL_ASSERT_GE(deq->head_pos, 0);
  CEL_ASSERT_LT(deq->head_pos, deq->head->cap);

  return _cel_GenericDequeBlock_At(deq->head, deq->head_pos, ele_size);
}

extern "C" CEL_NULLABLE(void*)
    _cel_GenericDeque_PushBackAllocator(CEL_NONNULL(_cel_GenericDeque*) deq,
                                        CEL_NONNULL(cel_Allocator*) alloc,
                                        size_t ele_size) {
  CEL_ASSERT_NOT_NULL(deq);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_GT(ele_size, 0);

  CEL_NULLABLE(_cel_GenericDequeBlock*) tail = deq->tail;
  if (CEL_UNLIKELY(tail == cel_nullptr ||
                   (deq->tail_pos == tail->cap &&
                    (tail != deq->head || deq->head_pos != deq->tail_pos)))) {
    if (!_cel_GenericDeque_NewTailBlockAllocator(deq, alloc, ele_size)) {
      return cel_nullptr;
    }
    tail = deq->tail;
  } else if (tail == deq->head && deq->tail_pos == deq->head_pos) {
    deq->head_pos = deq->tail_pos = tail->cap / 2;
  }

  CEL_ASSERT_NOT_NULL(tail);
  CEL_ASSERT_LT(deq->tail_pos, tail->cap);

  CEL_NONNULL(void*)
  ele = _cel_GenericDequeBlock_MutableAt(tail, deq->tail_pos, ele_size);
  // Before updating fields and returning to the caller, update the ASan
  // annotation to unpoison the storage for the newly appended element. Keep in
  // mind that deq->tail may be the same as deq->head.
  _cel_GenericDequeBlock_Annotate(
      tail, /*old_begin=*/tail != deq->head ? 0 : deq->head_pos,
      /*old_end=*/deq->tail_pos,
      /*new_begin=*/tail != deq->head ? 0 : deq->head_pos,
      /*new_end=*/deq->tail_pos + 1, ele_size);
  ++(deq->tail_pos);
  ++(deq->len);
  return ele;
}

extern "C" CEL_NULLABLE(void*)
    _cel_GenericDeque_PushBackArena(CEL_NONNULL(_cel_GenericDeque*) deq,
                                    CEL_NONNULL(cel_Arena*) arena,
                                    size_t ele_size) {
  CEL_ASSERT_NOT_NULL(deq);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT_GT(ele_size, 0);

  CEL_NULLABLE(_cel_GenericDequeBlock*) tail = deq->tail;
  if (CEL_UNLIKELY(tail == cel_nullptr ||
                   (deq->tail_pos == tail->cap &&
                    (tail != deq->head || deq->head_pos != deq->tail_pos)))) {
    if (!_cel_GenericDeque_NewTailBlockArena(deq, arena, ele_size)) {
      return cel_nullptr;
    }
    tail = deq->tail;
  } else if (tail == deq->head && deq->tail_pos == deq->head_pos) {
    deq->head_pos = deq->tail_pos = tail->cap / 2;
  }

  CEL_ASSERT_NOT_NULL(tail);
  CEL_ASSERT_LT(deq->tail_pos, tail->cap);

  CEL_NONNULL(void*)
  ele = _cel_GenericDequeBlock_MutableAt(tail, deq->tail_pos, ele_size);
  ++(deq->tail_pos);
  ++(deq->len);
  return ele;
}

extern "C" void _cel_GenericDeque_PopBackAllocator(
    CEL_NONNULL(_cel_GenericDeque*) deq, CEL_NONNULL(cel_Allocator*) alloc,
    size_t ele_size) {
  CEL_ASSERT_NOT_NULL(deq);
  CEL_ASSERT_NOT(_cel_GenericDeque_Empty(deq));
  CEL_ASSERT_GT(ele_size, 0);
  CEL_ASSERT_NOT_NULL(deq->tail);

  // Before updating fields, update the ASan annotation. Keep in
  // mind that deq->tail may be the same as deq->head.
  _cel_GenericDequeBlock_Annotate(
      deq->tail, /*old_begin=*/deq->tail != deq->head ? 0 : deq->head_pos,
      /*old_end=*/deq->tail_pos,
      /*new_begin=*/deq->tail != deq->head ? 0 : deq->head_pos,
      /*new_end=*/deq->tail_pos - 1, ele_size);

  --(deq->tail_pos);
  --(deq->len);
  if (deq->tail_pos == 0 && deq->head != deq->tail) {
    CEL_NONNULL(_cel_GenericDequeBlock*) old_tail = deq->tail;
    CEL_NONNULL(_cel_GenericDequeBlock*) new_tail = old_tail->prev;
    deq->tail = new_tail;
    deq->tail_pos = new_tail->cap;
    _cel_GenericDequeBlock_Unlink(old_tail);
    if (deq->cache == cel_nullptr) {
      deq->cache = old_tail;
    } else {
      // Block already cached. Delete this one.
      _cel_GenericDequeBlock_DeleteAllocator(old_tail, alloc, ele_size);
    }
  }
  if (deq->head == deq->tail && deq->head_pos == deq->tail_pos) {
    if (deq->cache != cel_nullptr) {
      _cel_GenericDequeBlock_AnnotateDelete(deq->cache, ele_size);
      _cel_GenericDequeBlock_DeleteAllocator(deq->cache, alloc, ele_size);
      deq->cache = cel_nullptr;
    }
    deq->head_pos = deq->tail_pos = deq->head->cap / 2;
  }
}

extern "C" void _cel_GenericDeque_PopBackArena(CEL_NONNULL(_cel_GenericDeque*)
                                                   deq) {
  CEL_ASSERT_NOT_NULL(deq);
  CEL_ASSERT_NOT(_cel_GenericDeque_Empty(deq));
  CEL_ASSERT_NOT_NULL(deq->tail);

  --(deq->tail_pos);
  --(deq->len);
  if (deq->tail_pos == 0 && deq->head != deq->tail) {
    CEL_NONNULL(_cel_GenericDequeBlock*) old_tail = deq->tail;
    CEL_NONNULL(_cel_GenericDequeBlock*) new_tail = old_tail->prev;
    deq->tail = new_tail;
    deq->tail_pos = new_tail->cap;
    _cel_GenericDequeBlock_Unlink(old_tail);
    _cel_GenericDequeBlock_LinkBefore(old_tail, deq->cache);
    deq->cache = old_tail;
  }
  if (deq->head == deq->tail && deq->head_pos == deq->tail_pos) {
    deq->head_pos = deq->tail_pos = deq->head->cap / 2;
  }
}

extern "C" CEL_NONNULL(const void*)
    _cel_GenericDeque_PeekBack(CEL_NONNULL(const _cel_GenericDeque*) deq,
                               size_t ele_size) {
  CEL_ASSERT_NOT_NULL(deq);
  CEL_ASSERT_NOT(_cel_GenericDeque_Empty(deq));
  CEL_ASSERT_GT(ele_size, 0);

  CEL_ASSERT_NOT_NULL(deq->tail);
  CEL_ASSERT_GT(deq->tail_pos, 0);
  CEL_ASSERT_LE(deq->tail_pos, deq->tail->cap);

  return _cel_GenericDequeBlock_At(deq->tail, deq->tail_pos - 1, ele_size);
}

extern "C" void _cel_GenericDeque_ResetAllocator(CEL_NONNULL(_cel_GenericDeque*)
                                                     deq,
                                                 CEL_NONNULL(cel_Allocator*)
                                                     alloc,
                                                 size_t ele_size) {
  _cel_GenericDeque_DestructAllocator(deq, alloc, ele_size);
  _cel_GenericDeque_Construct(deq);
}

extern "C" void _cel_GenericDeque_ClearAllocator(CEL_NONNULL(_cel_GenericDeque*)
                                                     deq,
                                                 CEL_NONNULL(cel_Allocator*)
                                                     alloc,
                                                 size_t ele_size) {
  CEL_ASSERT_NOT_NULL(deq);
  CEL_ASSERT_NOT_NULL(alloc);
  CEL_ASSERT_GT(ele_size, 0);

  if (deq->len == 0) {
    return;
  }
  deq->len = 0;

  CEL_NULLABLE(_cel_GenericDequeBlock*) head = deq->head;
  if (head != cel_nullptr) {
    CEL_ASSERT_NOT_NULL(deq->tail);

    // Add a block to the cache, if necessary. For malloc-based deque, we only
    // cache a single block.
    if (deq->cache == cel_nullptr) {
      CEL_NULLABLE(_cel_GenericDequeBlock*) new_head = head->next;
      _cel_GenericDequeBlock_Unlink(head);
      if (head == deq->tail) {
        deq->tail = new_head;
      }
      deq->head = new_head;
      deq->cache = head;
      head = new_head;
    }

    // Delete the rest of the blocks.
    _cel_GenericDequeBlock_DeleteChainAllocator(head, alloc, ele_size);
    deq->head = deq->tail = cel_nullptr;
    deq->head_pos = deq->tail_pos = 0;
  } else {
    CEL_ASSERT_NULL(deq->tail);
    CEL_ASSERT_EQ(deq->head_pos, 0);
    CEL_ASSERT_EQ(deq->tail_pos, 0);
  }
}

extern "C" void _cel_GenericDeque_ClearArena(CEL_NONNULL(_cel_GenericDeque*)
                                                 deq) {
  // For arena-based dequeue, we do not return memory to the arena as there is
  // currently no way to do that. So we move all the allocated blocks to the
  // cache list for re-use.

  CEL_ASSERT_NOT_NULL(deq);

  if (deq->len == 0) {
    return;
  }
  deq->len = 0;

  if (deq->head != cel_nullptr) {
    CEL_ASSERT_NOT_NULL(deq->tail);
    deq->cache = _cel_GenericDequeBlock_LinkBefore(deq->head, deq->cache);
    deq->head = deq->tail = cel_nullptr;
    deq->head_pos = deq->tail_pos = 0;
  } else {
    CEL_ASSERT_NULL(deq->tail);
    CEL_ASSERT_EQ(deq->head_pos, 0);
    CEL_ASSERT_EQ(deq->tail_pos, 0);
  }
}
