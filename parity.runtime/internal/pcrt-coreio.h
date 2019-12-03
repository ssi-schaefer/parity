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


#ifdef    __PCRT_INTERNAL_COREIO_H_NEED_PREWRAP
# ifndef  __PCRT_INTERNAL_COREIO_H_NEED_REAL

#  pragma push_macro( "remove")
#  pragma push_macro( "rename")
#  pragma push_macro( "unlink")
#  pragma push_macro("_unlink")

#  define  remove pcrt_no_remove
#  define  rename pcrt_no_rename
#  define  unlink pcrt_no_unlink
#  define _unlink pcrt_no__unlink

# endif // !__PCRT_INTERNAL_COREIO_H_NEED_REAL
# undef     __PCRT_INTERNAL_COREIO_H_NEED_PREWRAP
#endif //   __PCRT_INTERNAL_COREIO_H_NEED_PREWRAP


#ifdef __PCRT_INTERNAL_COREIO_H_NEED_POSTWRAP

PCRT_BEGIN_C

extern int pcrt_remove(const char* f);
extern int pcrt_rename(const char* o, const char *n);
extern int pcrt_unlink(const char* f);

PCRT_END_C

# ifndef __PCRT_INTERNAL_COREIO_H_NEED_REAL

#  undef  remove
#  undef  rename
#  undef  unlink
#  undef _unlink

#  ifndef  __PCRT_INTERNAL_COREIO_H
#   define __PCRT_INTERNAL_COREIO_H

//
// redefine io.h functions which work with files to be able
// of parsing non-native paths.
//
PCRT_BEGIN_C

static PCRT_INLINE int remove(const char* f)
{
  return pcrt_remove(f);
}

static PCRT_INLINE int rename(const char* o, const char *n)
{
  return pcrt_rename(o, n);
}

static PCRT_INLINE int unlink(const char* f)
{
  return pcrt_unlink(f);
}

static PCRT_INLINE int _unlink(const char* f)
{
  return pcrt_unlink(f);
}

PCRT_END_C

#  endif // __PCRT_INTERNAL_COREIO_H

#  pragma pop_macro( "remove")
#  pragma pop_macro( "rename")
#  pragma pop_macro( "unlink")
#  pragma pop_macro("_unlink")

# endif // !__PCRT_INTERNAL_COREIO_H_NEED_REAL
# undef     __PCRT_INTERNAL_COREIO_H_NEED_POSTWRAP
#endif //   __PCRT_INTERNAL_COREIO_H_NEED_POSTWRAP
