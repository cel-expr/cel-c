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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_RUNTIME_INSTR_H_
#define THIRD_PARTY_CEL_C_INTERNAL_RUNTIME_INSTR_H_

#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>

#include "cel-c/config.h"
#include "cel-c/duration.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"
#include "upb/reflection/def.h"

CEL_BEGIN_DECLS

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM {
  _cel_InstrKind_kUnreachable = 0,
  _cel_InstrKind_kNullConst,
  _cel_InstrKind_kFalseConst,
  _cel_InstrKind_kTrueConst,
  _cel_InstrKind_kIntConst,
  _cel_InstrKind_kUintConst,
  _cel_InstrKind_kDoubleConst,
  _cel_InstrKind_kBytesConst,
  _cel_InstrKind_kStringConst,
  _cel_InstrKind_kDurationConst,
  _cel_InstrKind_kTimestampConst,
  _cel_InstrKind_kIdent,
  _cel_InstrKind_kIdentJump,
  _cel_InstrKind_kContIdent,
  _cel_InstrKind_kContIdentJump,
  _cel_InstrKind_kJump,
  _cel_InstrKind_kHas,
  _cel_InstrKind_kMessageHas,
  _cel_InstrKind_kSelect,
  _cel_InstrKind_kMessageSelect,
  _cel_InstrKind_kAdd,
  _cel_InstrKind_kSubtract,
  _cel_InstrKind_kMultiply,
  _cel_InstrKind_kDivide,
  _cel_InstrKind_kModulo,
  _cel_InstrKind_kLogicalNot,
  _cel_InstrKind_kNegate,
  _cel_InstrKind_kLogicalAnd,
  _cel_InstrKind_kLogicalOr,
  _cel_InstrKind_kEquals,
  _cel_InstrKind_kNotEquals,
  _cel_InstrKind_kLess,
  _cel_InstrKind_kLessEquals,
  _cel_InstrKind_kGreater,
  _cel_InstrKind_kGreaterEquals,
  _cel_InstrKind_kCondJump,
  _cel_InstrKind_kTrileanJump,
  _cel_InstrKind_kErrorJump,
  _cel_InstrKind_kList,
  _cel_InstrKind_kKeyJump,
  _cel_InstrKind_kMap,
  _cel_InstrKind_kCallBool,
  _cel_InstrKind_kCallInt,
  _cel_InstrKind_kCallUint,
  _cel_InstrKind_kCallDouble,
  _cel_InstrKind_kCallBytes,
  _cel_InstrKind_kCallString,
  _cel_InstrKind_kCallTimestamp,
  _cel_InstrKind_kCallDuration,
  _cel_InstrKind_kCallSize,
  _cel_InstrKind_kCallContainsString,
  _cel_InstrKind_kCallStartsWithString,
  _cel_InstrKind_kCallEndsWithString,
  _cel_InstrKind_kCallRegexExpMatch,
  _cel_InstrKind_kIn,
  _cel_InstrKind_kIndex,
  // Accesses a slot. If the slot has not been populated, initializes it by
  // evaluating the associated expression.
  _cel_InstrKind_kLazyCall,
  // Located at the end of subexpressions, assigns the value at the top of the
  // stack to the slot and returns to the caller.
  _cel_InstrKind_kLazyReturn,
  // Debug only instruction which verifies that a slot is not active. This is
  // really only a defense in depth measure to help catch bugs in our planner.
  _cel_InstrKind_kLazyEnter,
  // Clears slots which are no longer needed.
  _cel_InstrKind_kLazyLeave,

  // Keep this one last.
  _cel_InstrKind_kExit,
} _cel_InstrKind;

#ifdef _MSC_VER
#pragma pack(push, 4)
#endif

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  const char* cel_nonnull data;
  uint32_t size;
} _cel_PackedStringView;

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_PackedStringView
_cel_PackedStringView_FromStringView(cel_StringView other) {
  _cel_PackedStringView result;
  result.data = cel_StringView_Data(other);
  result.size = cel_StringView_Size32(other);
  return result;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
_cel_PackedStringView_ToStringView(_cel_PackedStringView other) {
  return cel_StringView_FromArray(other.data, other.size);
}

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  union {
    // Used during planning.
    uint32_t indirect;
    // Dereferenced at the tail end of planning to avoid the indirection during
    // evaluation.
    _cel_PackedStringView direct;
  };
} _cel_InstrString;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  int64_t value;
} _cel_IntConstInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  uint64_t value;
} _cel_UintConstInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  double value;
} _cel_DoubleConstInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  _cel_InstrString value;
} _cel_BytesConstInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  _cel_InstrString value;
} _cel_StringConstInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  cel_Duration value;
} _cel_DurationConstInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  cel_Timestamp value;
} _cel_TimestampConstInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  _cel_InstrString name;
} _cel_IdentInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  // Index into string_table.
  uint32_t name : 31;
  uint32_t missing_error : 1;
  int32_t found_jump;
  int32_t missing_jump;
} _cel_IdentJumpInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  // Index into candidate_names_table.
  uint32_t candidate_names;
} _cel_ContIdentInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  // Index into candidate_names_table.
  uint32_t candidate_names : 31;
  uint32_t missing_error : 1;
  int32_t found_jump;
  int32_t missing_jump;
} _cel_ContIdentJumpInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  int32_t jump;
} _cel_JumpInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  _cel_InstrString field;
} _cel_HasInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  const upb_FieldDef* cel_nonnull field;
} _cel_MessageHasInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  _cel_InstrString field;
} _cel_SelectInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  const upb_FieldDef* cel_nonnull field;
} _cel_MessageSelectInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  int32_t jump;
  bool cond;
} _cel_CondJumpInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  int32_t false_jump;
  int32_t error_jump;
} _cel_TrileanJumpInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  int32_t jump;
  uint32_t pop;
} _cel_ErrorJumpInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  uint32_t count;
} _cel_ListInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  int32_t jump;
  uint32_t pop;
} _cel_KeyJumpInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  uint32_t count;
} _cel_MapInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  uint32_t name;
  uint32_t args;
} _cel_CallInstr;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  uint32_t slot;
  int32_t jump;
} _cel_LazyCall;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  uint32_t slot;
} _cel_LazyReturn;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  uint32_t slot;
} _cel_LazyEnter;

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  uint32_t slot;
  uint32_t num_slots;
} _cel_LazyLeave;

typedef union CEL_ATTRIBUTE_PACKED(4) {
  _cel_IntConstInstr int_const;
  _cel_UintConstInstr uint_const;
  _cel_DoubleConstInstr double_const;
  _cel_BytesConstInstr bytes_const;
  _cel_StringConstInstr string_const;
  _cel_DurationConstInstr duration_const;
  _cel_TimestampConstInstr timestamp_const;
  _cel_IdentInstr ident;
  _cel_IdentJumpInstr ident_jump;
  _cel_ContIdentInstr cont_ident;
  _cel_ContIdentJumpInstr cont_ident_jump;
  _cel_JumpInstr jump;
  _cel_HasInstr has;
  _cel_MessageHasInstr message_has;
  _cel_SelectInstr select;
  _cel_MessageSelectInstr message_select;
  _cel_CondJumpInstr cond_jump;
  _cel_TrileanJumpInstr trilean_jump;
  _cel_ErrorJumpInstr error_jump;
  _cel_ListInstr list;
  _cel_KeyJumpInstr key_jump;
  _cel_MapInstr map;
  _cel_CallInstr call;
  _cel_LazyCall lazy_call;
  _cel_LazyReturn lazy_return;
  _cel_LazyEnter lazy_enter;
  _cel_LazyLeave lazy_leave;
} _cel_InstrData;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

CEL_STATIC_ASSERT(sizeof(_cel_InstrData) <= 12);
CEL_STATIC_ASSERT(alignof(_cel_InstrData) <= 4);

#ifdef _MSC_VER
#pragma pack(push, 8)
#endif

typedef struct CEL_ATTRIBUTE_PACKED(8) {
  _cel_InstrData data;
  CEL_ATTRIBUTE_PREFERRED_TYPE(_cel_InstrKind) uint32_t kind : 16;
} _cel_Instr;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

CEL_STATIC_ASSERT(sizeof(_cel_Instr) <= 16);
CEL_STATIC_ASSERT(alignof(_cel_Instr) <= 8);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_RUNTIME_INSTR_H_
