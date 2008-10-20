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


//
// There are a few tested configurations for the following:
//
//   *) cl.exe from visual studio project (_WIN32)
//   *) cl.exe through parity itself (_WIN32)
//      *) configure: CXX=parity.gnu.gcc LD=parity.gnu.ld
//   *) cl.exe through vista cc89 (patched) (__INTERIX && !__GNUC__)
//      *) configure: CXX=cc89 CXXFLAGS="-D_ALL_SOURCE -X /Ox"
//   *) gcc 4.2.1 (__GNUC__ (4))
//      *) configure: CXXFLAGS="-D_ALL_SOURCE -O3 -fmessage-length=0" LDFLAGS="-static"
//   *) gcc 3.3 (interix) (__GNUC__ && __GNUC__ == 3)
//      *) configure: CXXFLAGS="-D_ALL_SOURCE -O3 -fmessage-length=0"
//
// The patch for vistas cc89 is:
/*
### begin
--- /bin/cc89.orig  Fri Sep  7 17:33:41 2007
+++ /bin/cc89   Fri Sep  7 17:32:44 2007
@@ -822,7 +822,7 @@
 # now gather up the operands
 #
 while true ; do
-    if [ "$progname" = "cc" ]; then
+    if [ "$progname" = "cc" -o "$progname" = "cc89" ]; then
         #
         # special case for 'cc':
         #   it can handle certain options anywhere in the command line
### end */
//

#if defined(_WIN32) || (defined(__INTERIX) && (!defined(__GNUC__) || __GNUC__ == 3 ))
#  ifndef __GNUC__
#    pragma warning(disable:4103)
#  endif
#  pragma pack(push, 2)
#  define STRUCT_PACKING_ATTRIBUTE
#elif defined(__GNUC__)
#  define STRUCT_PACKING_ATTRIBUTE __attribute__((gcc_struct, packed, aligned(2)))
#else
#  error "don't know how to handle structure alignment!"
#endif

