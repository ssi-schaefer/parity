/****************************************************************\
*                                                                *
* Copyright (C) 2011-2019 by Markus Duft <markus.duft@ssi-schaefer.com>
* Copyright (C) 2019 by Michael Haubenwallner <michael.haubenwallner@ssi-schaefer.com>
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

#if defined(WRAP_REALBIN) && defined(WRAP_OPTIONS)
/*
 * While installed as part of runtime sources,
 * this is used to create the .exe wrappers only.
 */

#include <unistd.h>
#include <string.h>

#define _wrapstr(x) #x
#define wrapstr(x) _wrapstr(x)
#define wrapper main /* have Confix ignore this main */
int wrapper(int argc, char **argv)
{
  char opts[] = wrapstr(WRAP_OPTIONS);
  int i, nopts;
  i = nopts = 0;
  while(opts[i]) {
    if (opts[i] == ' ') {
      ++i;
      continue;
    }
    ++nopts;
    ++i;
    while(opts[i] && opts[i] != ' ') {
      ++i;
    }
  }
  char *newargv[argc+nopts+1];
  newargv[0] = argv[0];
  i = nopts = 0;
  while(opts[i]) {
    if (opts[i] == ' ') {
      opts[i] = '\0';
      ++i;
      continue;
    }
    ++nopts;
    newargv[nopts] = &opts[i];
    ++i;
    while(opts[i] && opts[i] != ' ') {
      ++i;
    }
  }
  for(i = 1; i <= argc; ++i) {
    ++nopts;
    newargv[nopts] = argv[i];
  }
  execv(wrapstr(WRAP_REALBIN), newargv);
}

#endif /* WRAP_REALBIN */
