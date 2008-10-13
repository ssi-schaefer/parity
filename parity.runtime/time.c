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

#include "sys/time.h"
#include "sys/timeb.h"
#include "sys/socket.h"

//
// TODO: 64 bit time_t? hmm... i think we can't do anything about it.
//
int gettimeofday(struct timeval *tv, void* unused)
{
    struct timeb tb; 
    ftime(&tb);

    tv->tv_sec  = (long)tb.time;
    tv->tv_usec = (long)tb.millitm;
    return(0);
}
