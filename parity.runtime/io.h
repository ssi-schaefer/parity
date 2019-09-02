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

#ifndef __PCRT_IO_H__
#define __PCRT_IO_H__

#include "internal/pcrt.h"

#define __PCRT_INTERNAL_IO_H_NEED_PREAMBLE
#include "internal/pcrt-io.h"


#pragma push_macro("__STDC__")
#  undef __STDC__
#  include UCRT_INC(Io.h)
#pragma pop_macro("__STDC__")

#include RUNTIME_INC(Stdarg.h)


#define __PCRT_INTERNAL_IO_H_NEED_POSTAMBLE
#include "internal/pcrt-io.h"

#endif

