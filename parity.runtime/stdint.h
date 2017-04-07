/****************************************************************\
*                                                                *
* Copyright (C) 2017 by                                          *
* Michael Haubenwallner <michael.haubenwallner@ssi-schaefer.com> *
*                                                                *
* This file is part of parity.                                   *
*                                                                *
* parity is free software: you can redistribute it and/or modify *
* it under the terms of the GNU Lesser General Public License as *
* published by the Free Software Foundation, either version 3 of *
* the License, or (at your option) any later version.            *
*                                                                *
* parity is distributed in the hope that it will be useful,      *
* but WITHOUT ANY WARRANTY; without even the implied warranty of *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  *
* GNU Lesser General Public License for more details.            *
*                                                                *
* You should have received a copy of the GNU Lesser General      *
* Public License along with parity. If not,                      *
* see <http://www.gnu.org/licenses/>.                            *
*                                                                *
\****************************************************************/

#ifndef __PCRT_STDINT_H_
#define __PCRT_STDINT_H_

#include "internal/pcrt.h"

#include "limits.h"
#include "wchar.h"

#include RUNTIME_INC(Intsafe.h)

PCRT_BEGIN_C

#define __PCRT_CONCAT(value1, value2) value1 ## value2

#define INT_LEAST8_MAX  INT8_MAX
#define INT_LEAST16_MAX INT16_MAX
#define INT_LEAST32_MAX INT32_MAX
#define INT_LEAST64_MAX INT64_MAX

#define INT_FAST8_MAX   INT8_MAX
#define INT_FAST16_MAX  INT16_MAX
#define INT_FAST32_MAX  INT32_MAX
#define INT_FAST64_MAX  INT64_MAX

#define INT_LEAST8_MIN  INT8_MIN
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST64_MIN INT64_MIN

#define INT_FAST8_MIN   INT8_MIN
#define INT_FAST16_MIN  INT16_MIN
#define INT_FAST32_MIN  INT32_MIN
#define INT_FAST64_MIN  INT64_MIN

#define  INT8_C(constant)  __PCRT_CONCAT(constant,  i8)
#define  INT16_C(constant) __PCRT_CONCAT(constant,  i16)
#define  INT32_C(constant) __PCRT_CONCAT(constant,  i32)
#define  INT64_C(constant) __PCRT_CONCAT(constant,  i64)

typedef  INT8   int8_t,  int_least8_t,  int_fast8_t;
typedef  INT16  int16_t, int_least16_t, int_fast16_t;
typedef  INT32  int32_t, int_least32_t, int_fast32_t;
typedef  INT64  int64_t, int_least64_t, int_fast64_t;

#define UINT_LEAST8_MAX  UINT8_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define UINT_LEAST32_MAX UINT32_MAX
#define UINT_LEAST64_MAX UINT64_MAX

#define UINT_FAST8_MAX   UINT8_MAX
#define UINT_FAST16_MAX  UINT16_MAX
#define UINT_FAST32_MAX  UINT32_MAX
#define UINT_FAST64_MAX  UINT64_MAX

#define UINT8_C(constant)  __PCRT_CONCAT(constant, ui8)
#define UINT16_C(constant) __PCRT_CONCAT(constant, ui16)
#define UINT32_C(constant) __PCRT_CONCAT(constant, ui32)
#define UINT64_C(constant) __PCRT_CONCAT(constant, ui64)

typedef UINT8  uint8_t,  uint_least8_t,  uint_fast8_t;
typedef UINT16 uint16_t, uint_least16_t, uint_fast16_t;
typedef UINT32 uint32_t, uint_least32_t, uint_fast32_t;
typedef UINT64 uint64_t, uint_least64_t, uint_fast64_t;

#define INTMAX_MAX          INT64_MAX
#define INTMAX_MIN          INT64_MIN
#define INTMAX_C(constant)  INT64_C(constant)
typedef                     INT64 intmax_t;

#define UINTMAX_MAX         UINT64_MAX
#define UINTMAX_C(constant) UINT64_C(constant)
typedef                     UINT64 uintmax_t;

#define PTRDIFF_MIN PTRDIFF_T_MIN
#define PTRDIFF_MAX PTRDIFF_T_MAX

#define SIG_ATOMIC_MIN INT_MIN
#define SIG_ATOMIC_MAX INT_MAX

#define WINT_MIN WCHAR_MIN
#define WINT_MAX WCHAR_MAX

PCRT_END_C

#endif
