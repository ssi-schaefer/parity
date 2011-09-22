/****************************************************************\
*                                                                *
* Copyright (C) 2011 by Markus Duft <markus.duft@salomon.at>     *
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

#if (_MSC_VER-0) < 1400
// necessary with _POSIX_ and Visual Studio .NET 2003, to avoid:
// error LNK2019: Nicht aufgelöstes externes Symbol '"long std::_Fpz" (?_Fpz@std@@3JA)', verwiesen in Funktion '"public: __thiscall std::fpos<int>::fpos<int>(long)" (??0?$fpos@H@std@@QAE@J@Z)'
// References:
//   <stdio.h>:       typedef {long,__int64,struct} fpos_t;
//   <iosfwd>:        extern fpos_t _Fpz;
//   crt/src/ios.cpp: fpos_t _Fpz;
extern "C++" {
namespace std {
long _Fpz = {0};
}
}
#endif
