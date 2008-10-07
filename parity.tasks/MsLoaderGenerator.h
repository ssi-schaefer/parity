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

#ifndef __LOADERGENERATOR_H__
#define __LOADERGENERATOR_H__

#include <map>
#include <string>

#include <Path.h>

#include <CoffImport.h>
#include <CoffSymbol.h>
#include "BinaryGatherer.h"

namespace parity
{
	namespace tasks
	{
		//
		// following is the data for the ExeLoader EntryPoint
		//

			//
			// part1:
			//   55                 push        ebp
			//	 8B EC              mov         ebp,esp
			//   E8 00 00 00 00     call        _ParityLoaderInit
			//
			// last four bytes not included!
			//
			static const unsigned char dataExeEntryPart1[] = { 0x55, 0x8B, 0xEC, 0xE8 };
			//
			// part2:
			//   85 C0              test        eax,eax
			//   75 02              jne         (2 bytes forward, call to real entry point)
			//   EB 05              jmp         (5 bytes forward, to exit of function)
			//   E8 00 00 00 00     call        <real entry point>
			//
			// last four bytes not included!
			//
			static const unsigned char dataExeEntryPart2[] = { 0x85, 0xC0, 0x75, 0x02, 0xEB, 0x05, 0xE8 };
			//
			// part3:
			//   5D                 pop         ebp
			//   C3                 ret
			//
			static const unsigned char dataExeEntryPart3[] = { 0x5D, 0xC3 };

		//
		// following is the data for the DllLoader EntryPoint
		//

			//
			// part1:
			//   55                 push        ebp
			//   8B EC              mov         ebp,esp
			//   83 7D 0C 01        cmp         dword ptr [ebp+0Ch],1
			//   75 0B              jne         (11 bytes forward, call to real entry point)
			//   E8 00 00 00 00     call        _ParityLoaderInit
			//
			// last four bytes not included!
			//
			static const unsigned char dataDllEntryPart1[] = { 0x55, 0x8B, 0xEC, 0x83, 0x7D, 0x0C, 0x01, 0x75, 0x0B, 0xE8 };

			//
			// part2:
			//   85 C0              test        eax,eax
			//   75 02              jne         (2 bytes forward, call to real entry point)
			//   EB 11              jmp         (18 bytes forward, exit of function)
			//   8B 45 10           mov         eax,dword ptr [ebp+10h]
			//   50                 push        eax
			//   8B 4D 0C           mov         ecx,dword ptr [ebp+0Ch]
			//   51                 push        ecx
			//   8B 55 08           mov         edx,dword ptr [ebp+8]
			//   52                 push        edx
			//   E8 00 00 00 00     call        <real entry point>
			//
			// last four bytes not included!
			//
			static const unsigned char dataDllEntryPart2[] = { 0x85, 0xC0, 0x75, 0x02, 0xEB, 0x11, 0x8B, 0x45, 0x10, 0x50,
				0x8B, 0x4D, 0x0C, 0x51, 0x8B, 0x55, 0x08, 0x52, 0xE8 };

			//
			// part3:
			//   5D                 pop         ebp
			//   C2 0C 00           ret         0Ch
			//
			static const unsigned char dataDllEntryPart3[] = { 0x5D, 0xC2, 0x0C, 0x00 };

		//
		// following is the data for the LazyLoading stub
		//

            //
			// part1:
			//   81 3D 00 00 00 00  cmp         dword ptr [<import pointer>],offset <stub>
			//   00 00 00 00
			//
			static const unsigned char dataLazyStubPart1[] = { 0x81, 0x3D };

			//
			// part2:
			//   75 0F              jne         (16 bytes forward, jump through pointer)
			//   60                 pushad
			//   68 00 00 00 00     push        offset <import item>
			//
			static const unsigned char dataLazyStubPart2[] = { 0x75, 0x0F, 0x60, 0x68 };

			//
			// part3:
			//   E8 00 00 00 00     call        _ParityLoaderLazyLoadSymbol
			//
			static const unsigned char dataLazyStubPart3[] = { 0xE8 };

			//
			// part4:
			//   83 C4 04           add         esp,4
			//   61                 popad
			//   FF 25 00 00 00 00  jmp         dword ptr [<import pointer>]
			//
			static const unsigned char dataLazyStubPart4[] = { 0x83, 0xC4, 0x04, 0x61, 0xFF, 0x25 };

		class MsLoaderGenerator {
		public:
			MsLoaderGenerator(const tasks::BinaryGatherer::ImportHybridityMap& imports);
			void doWork();
		private:
			const tasks::BinaryGatherer::ImportHybridityMap& imports_;

			typedef struct {
				std::string name;
				bool hybrid;
				utils::Path path;

				binary::Import::ImportVector imports;

				unsigned int recordIndex;
				unsigned int nameIndex;
				unsigned int tableIndex;
			} LoaderWorkingItem;

			typedef std::map<std::string, LoaderWorkingItem> LoaderWorkingMap;
		};
	}
}

#endif

