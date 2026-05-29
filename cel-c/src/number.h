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

#ifndef THIRD_PARTY_CEL_C_SRC_NUMERIC_H_
#define THIRD_PARTY_CEL_C_SRC_NUMERIC_H_

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/src/compare.h"

CEL_BEGIN_DECLS

#define _cel_kNumberDoubleToIntMax ((double)(int64_t)INT64_MAX)
#define _cel_kNumberDoubleToIntMin ((double)(int64_t)INT64_MIN)
#define _cel_kNumberDoubleToUintMax ((double)(uint64_t)UINT64_MAX)
#define _cel_kNumberDoubleToUintMin ((double)(uint64_t)0)

#define _cel_kNumberDoubleAsIntMax \
  ((double)(((int64_t)INT64_MAX) - \
            (1 << ((CHAR_BIT * sizeof(int64_t) - 1) - DBL_MANT_DIG - 1))))
#define _cel_kNumberDoubleAsUintMax  \
  ((double)(((uint64_t)UINT64_MAX) - \
            (1 << ((CHAR_BIT * sizeof(uint64_t)) - DBL_MANT_DIG - 1))))

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM {
  _cel_NumberKind_kInt = 1,
  _cel_NumberKind_kUint,
  _cel_NumberKind_kDouble,
} _cel_NumberKind;

typedef struct {
  union {
    int64_t i;
    uint64_t u;
    double f;
  } data;
  _cel_NumberKind kind;
} _cel_Number;

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_Number _cel_IntNumber(int64_t value) {
  _cel_Number number;
  number.data.i = value;
  number.kind = _cel_NumberKind_kInt;
  return number;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_Number _cel_UintNumber(uint64_t value) {
  _cel_Number number;
  number.data.u = value;
  number.kind = _cel_NumberKind_kUint;
  return number;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_Number _cel_DoubleNumber(double value) {
  _cel_Number number;
  number.data.f = value;
  number.kind = _cel_NumberKind_kDouble;
  return number;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_Number_IntCompare(int64_t lhs, int64_t rhs) {
  return lhs < rhs ? -1 : lhs > rhs ? 1 : 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_Number_UintCompare(uint64_t lhs, uint64_t rhs) {
  return lhs < rhs ? -1 : lhs > rhs ? 1 : 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_PartialOrdering _cel_Number_DoubleCompare(double lhs,
                                                                 double rhs) {
  if (isnan(lhs) || isnan(rhs)) {
    return _cel_PartialOrdering_kUnordered;
  }
  return lhs < rhs   ? _cel_PartialOrdering_kLess
         : lhs > rhs ? _cel_PartialOrdering_kGreater
                     : _cel_PartialOrdering_kEquivalent;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_PartialOrdering _cel_Number_Compare(_cel_Number lhs,
                                                           _cel_Number rhs) {
  switch (lhs.kind) {
    case _cel_NumberKind_kInt:
      switch (rhs.kind) {
        case _cel_NumberKind_kInt:
          return _cel_PartialOrdering_FromInt(
              _cel_Number_IntCompare(lhs.data.i, rhs.data.i));
        case _cel_NumberKind_kUint:
          return _cel_PartialOrdering_FromInt(
              lhs.data.i < 0
                  ? -1
                  : _cel_Number_UintCompare((uint64_t)lhs.data.i, rhs.data.u));
        case _cel_NumberKind_kDouble:
          if (isnan(rhs.data.f)) {
            return _cel_PartialOrdering_kUnordered;
          }
          if (rhs.data.f < _cel_kNumberDoubleToIntMin) {
            return _cel_PartialOrdering_kGreater;
          }
          if (rhs.data.f > _cel_kNumberDoubleToIntMax) {
            return _cel_PartialOrdering_kLess;
          }
          return _cel_Number_DoubleCompare((double)lhs.data.i, rhs.data.f);
        default:
          CEL_UNREACHABLE();
      }
    case _cel_NumberKind_kUint:
      switch (rhs.kind) {
        case _cel_NumberKind_kInt:
          return _cel_PartialOrdering_FromInt(
              rhs.data.i < 0
                  ? 1
                  : _cel_Number_UintCompare(lhs.data.u, (uint64_t)rhs.data.i));
        case _cel_NumberKind_kUint:
          return _cel_PartialOrdering_FromInt(
              _cel_Number_UintCompare(lhs.data.u, rhs.data.u));
        case _cel_NumberKind_kDouble:
          if (isnan(rhs.data.f)) {
            return _cel_PartialOrdering_kUnordered;
          }
          if (rhs.data.f < _cel_kNumberDoubleToUintMin) {
            return _cel_PartialOrdering_kGreater;
          }
          if (rhs.data.f > _cel_kNumberDoubleToUintMax) {
            return _cel_PartialOrdering_kLess;
          }
          return _cel_Number_DoubleCompare((double)lhs.data.u, rhs.data.f);
        default:
          CEL_UNREACHABLE();
      }
    case _cel_NumberKind_kDouble:
      if (isnan(lhs.data.f)) {
        return _cel_PartialOrdering_kUnordered;
      }
      switch (rhs.kind) {
        case _cel_NumberKind_kInt:
          if (lhs.data.f < _cel_kNumberDoubleToIntMin) {
            return _cel_PartialOrdering_kLess;
          }
          if (lhs.data.f > _cel_kNumberDoubleToIntMax) {
            return _cel_PartialOrdering_kGreater;
          }
          return _cel_Number_DoubleCompare(lhs.data.f, (double)rhs.data.i);
        case _cel_NumberKind_kUint:
          if (lhs.data.f < _cel_kNumberDoubleToUintMin) {
            return _cel_PartialOrdering_kLess;
          }
          if (lhs.data.f > _cel_kNumberDoubleToUintMax) {
            return _cel_PartialOrdering_kGreater;
          }
          return _cel_Number_DoubleCompare(lhs.data.f, (double)rhs.data.u);
        case _cel_NumberKind_kDouble:
          if (isnan(rhs.data.f)) {
            return _cel_PartialOrdering_kUnordered;
          }
          return _cel_Number_DoubleCompare(lhs.data.f, rhs.data.f);
        default:
          CEL_UNREACHABLE();
      }
    default:
      CEL_UNREACHABLE();
  }
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_Number_Equals(_cel_Number lhs, _cel_Number rhs) {
  switch (lhs.kind) {
    case _cel_NumberKind_kInt:
      switch (rhs.kind) {
        case _cel_NumberKind_kInt:
          return lhs.data.i == rhs.data.i;
        case _cel_NumberKind_kUint:
          return lhs.data.i >= 0 && (uint64_t)lhs.data.i == rhs.data.u;
        case _cel_NumberKind_kDouble:
          return !isnan(rhs.data.f) &&
                 rhs.data.f >= _cel_kNumberDoubleToIntMin &&
                 rhs.data.f <= _cel_kNumberDoubleToIntMax &&
                 (double)(int64_t)rhs.data.f == rhs.data.f &&
                 (int64_t)rhs.data.f == lhs.data.i;
        default:
          CEL_UNREACHABLE();
          return false;
      }
    case _cel_NumberKind_kUint:
      switch (rhs.kind) {
        case _cel_NumberKind_kInt:
          return rhs.data.i >= 0 && (uint64_t)rhs.data.i == lhs.data.u;
        case _cel_NumberKind_kUint:
          return lhs.data.u == rhs.data.u;
        case _cel_NumberKind_kDouble:
          return !isnan(rhs.data.f) &&
                 rhs.data.f >= _cel_kNumberDoubleToUintMin &&
                 rhs.data.f <= _cel_kNumberDoubleToUintMax &&
                 (double)(uint64_t)rhs.data.f == rhs.data.f &&
                 (uint64_t)rhs.data.f == lhs.data.u;
        default:
          CEL_UNREACHABLE();
          return false;
      }
      break;
    case _cel_NumberKind_kDouble:
      switch (rhs.kind) {
        case _cel_NumberKind_kInt:
          return !isnan(lhs.data.f) &&
                 lhs.data.f >= _cel_kNumberDoubleToIntMin &&
                 lhs.data.f <= _cel_kNumberDoubleToIntMax &&
                 (double)(int64_t)lhs.data.f == lhs.data.f &&
                 (int64_t)lhs.data.f == rhs.data.i;
        case _cel_NumberKind_kUint:
          return !isnan(lhs.data.f) &&
                 lhs.data.f >= _cel_kNumberDoubleToUintMin &&
                 lhs.data.f <= _cel_kNumberDoubleToUintMax &&
                 (double)(uint64_t)lhs.data.f == lhs.data.f &&
                 (uint64_t)lhs.data.f == rhs.data.u;
        case _cel_NumberKind_kDouble:
          return lhs.data.f == rhs.data.f;
        default:
          CEL_UNREACHABLE();
          return false;
      }
    default:
      CEL_UNREACHABLE();
      return false;
  }
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_Number_ToIntLossless(_cel_Number in,
                                                 int64_t* cel_nullable out) {
  switch (in.kind) {
    case _cel_NumberKind_kInt:
      if (out != cel_nullptr) {
        *out = in.data.i;
      }
      return true;
    case _cel_NumberKind_kUint:
      if (in.data.u > (uint64_t)(int64_t)INT64_MAX) {
        return false;
      }
      if (out != cel_nullptr) {
        *out = (int64_t)in.data.u;
      }
      return true;
    case _cel_NumberKind_kDouble:
      if (!isnan(in.data.f) && in.data.f >= _cel_kNumberDoubleToIntMin &&
          in.data.f <= _cel_kNumberDoubleAsIntMax &&
          in.data.f == (double)(int64_t)in.data.f) {
        if (out != cel_nullptr) {
          *out = (int64_t)in.data.f;
        }
        return true;
      }
      return false;
    default:
      CEL_UNREACHABLE();
      return false;
  }
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_Number_ToInt(_cel_Number in,
                                         int64_t* cel_nullable out) {
  switch (in.kind) {
    case _cel_NumberKind_kInt:
      if (out != cel_nullptr) {
        *out = in.data.i;
      }
      return true;
    case _cel_NumberKind_kUint:
      if (in.data.u > (uint64_t)(int64_t)INT64_MAX) {
        return false;
      }
      if (out != cel_nullptr) {
        *out = (int64_t)in.data.u;
      }
      return true;
    case _cel_NumberKind_kDouble:
      if (!isnan(in.data.f) && in.data.f >= _cel_kNumberDoubleToIntMin &&
          in.data.f <= _cel_kNumberDoubleAsIntMax) {
        if (out != cel_nullptr) {
          *out = (int64_t)in.data.f;
        }
        return true;
      }
      return false;
    default:
      CEL_UNREACHABLE();
      return false;
  }
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_Number_ToUint(_cel_Number in,
                                          uint64_t* cel_nullable out) {
  switch (in.kind) {
    case _cel_NumberKind_kInt:
      if (in.data.i < 0) {
        return false;
      }
      if (out != cel_nullptr) {
        *out = (uint64_t)in.data.i;
      }
      return true;
    case _cel_NumberKind_kUint:
      if (out != cel_nullptr) {
        *out = in.data.u;
      }
      return true;
    case _cel_NumberKind_kDouble:
      if (!isnan(in.data.f) && in.data.f >= _cel_kNumberDoubleToUintMin &&
          in.data.f <= _cel_kNumberDoubleAsUintMax) {
        if (out != cel_nullptr) {
          *out = (uint64_t)in.data.f;
        }
        return true;
      }
      return false;
    default:
      CEL_UNREACHABLE();
      return false;
  }
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE double _cel_Number_ToDouble(_cel_Number in) {
  switch (in.kind) {
    case _cel_NumberKind_kInt:
      return (double)in.data.i;
    case _cel_NumberKind_kUint:
      return (double)in.data.u;
    case _cel_NumberKind_kDouble:
      return in.data.f;
    default:
      CEL_UNREACHABLE();
      return NAN;
  }
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_SRC_NUMERIC_H_
