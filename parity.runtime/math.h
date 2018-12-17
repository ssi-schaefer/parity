/****************************************************************\
*                                                                *
* Copyright (C) 2007 by Markus Duft <markus.duft@salomon.at>     *
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

#ifndef __PCRT_MATH_H__
#define __PCRT_MATH_H__

#include "internal/pcrt.h"
#include "float.h"

#pragma push_macro("__STDC__")
#pragma push_macro("_USE_MATH_DEFINES")
#  ifdef __STDC__
#    undef __STDC__
#  endif
#  ifndef _USE_MATH_DEFINES
#    define _USE_MATH_DEFINES 1
#  endif
#  include UCRT_INC(Math.h)
#pragma pop_macro("_USE_MATH_DEFINES")
#pragma pop_macro("__STDC__")

typedef float		float_t;
typedef double		double_t;

PCRT_BEGIN_C

#pragma push_macro("copysign")
#pragma push_macro("copysignl")
#pragma push_macro("finite")
#pragma push_macro("nextafter")
#pragma push_macro("remainder")

#undef copysign
#undef copysignl
#undef finite
#undef nextafter
#undef remainder

static PCRT_INLINE double copysign(double x, double y) { return _copysign(x, y); }
#if (_MSC_VER-0) >= 1400
/* available since Visual Studio 2005 */
static PCRT_INLINE long double copysignl(long double x, long double y) { return _copysignl(x, y); }
#endif

static PCRT_INLINE int finite(double x) { return _finite(x); }

static PCRT_INLINE double nextafter(double x, double y) { return _nextafter(x, y); }
static PCRT_INLINE double remainder(double x, double y) { return fmod(x, y); }

#pragma pop_macro("copysign")
#pragma pop_macro("copysignl")
#pragma pop_macro("finite")
#pragma pop_macro("nextafter")
#pragma pop_macro("remainder")

PCRT_END_C

#endif

