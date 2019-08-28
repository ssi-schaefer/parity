/****************************************************************\
*                                                                *
* Copyright (C) 2007 by Markus Duft <markus.duft@ssi-schaefer.com>
* Copyright (C) 2019 by Michaek Haubenwallner <michael.haubenwallner@ssi-schaefer.com>
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

#if !defined(_OFF_T_DEFINED)

/*
 * Unfortunately, even with MSVC 2019/Win10, off_t is 32bit only,
 * but for POSIX fseeko/ftello, we do need 64bit off_t eventually.
 *
 * Fortunately, MSVC headers internally use _off_t only,
 * so we can leave _off_t as 32bit for struct stat.
 */
typedef long _off_t;

/*
 * OTOH, off_t needs to match fpos_t:
 * up to MSVC 2010 the _FPOSOFF() macro does 32bit only,
 * since MSVC 2011 the _FPOSOFF() macro does 64bit only.
 */
# if _MSC_VER - 0 < 1700
typedef long off_t;
# else
typedef long long off_t;
#endif

# define _OFF_T_DEFINED

#endif /* !_OFF_T_DEFINED */

