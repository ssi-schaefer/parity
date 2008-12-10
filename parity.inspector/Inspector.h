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

#ifndef __INSPECTOR_H__
#define __INSPECTOR_H__

#include <Path.h>
#include <Context.h>
#include <vector>
#include <string>
#include <CoffFileHeader.h>

namespace parity 
{
	namespace inspector 
	{
		//
		// WARNING: when changing this, don't forget to update
		// LoaderInit.h from parity.loader too!
		//

		#include <CoffHelperPackStart.h>

		typedef struct STRUCT_PACKING_ATTRIBUTE {
			unsigned int name;
			unsigned int import;
			unsigned short ordinal;
			unsigned int library;
		} InspectorImportsPointerLayoutVA;

		typedef struct STRUCT_PACKING_ATTRIBUTE {
			const char* name;
			const void* import;
			const char* library;
			unsigned short ordinal;
		} InspectorImports;

		typedef std::vector<InspectorImports> InspectorImportVector;

		typedef struct STRUCT_PACKING_ATTRIBUTE {
			unsigned int name;
			unsigned int imports;
		} InspectorLibrariesPointerLayoutVA;

		typedef struct _InspectorLibrariesStruct {
			_InspectorLibrariesStruct() : name(), imports(), children(), file() {}

			std::string name;
			unsigned long base;
			bool native;
			InspectorImportVector imports;
			std::vector<struct _InspectorLibrariesStruct> children;
			utils::Path file;
		} InspectorLibraries;

		typedef std::vector<InspectorLibraries> InspectorLibraryVector;
		typedef std::map<utils::Path, InspectorLibraryVector> InspectorLibraryVectorMap;

		//
		// WARNING: keep this in sync with the strucure in LoaderGenerator.cpp from
		// parity.tasks.
		//
		typedef struct STRUCT_PACKING_ATTRIBUTE {
			unsigned int table;
			unsigned int rpaths;
			unsigned int name;
			utils::SubsystemType subsystem;
		} InspectorPointerLayoutVA;

		#include <CoffHelperPackStop.h>

		#define NAMEOF_SECTION_POINTERS	".p.ptrs"
		#define NAMEOF_SECTION_IMPORTS	".idata"

		bool ProcessFileList(const utils::PathVector& vec, InspectorLibraryVectorMap& map);
		bool ProcessFile(const utils::Path& pth, InspectorLibraryVector& vec);

		utils::PathVector ConvertRunPaths(const char* rpaths);
		InspectorLibraryVector InspectorLayoutToLibraries(const InspectorLibrariesPointerLayoutVA* ptr, binary::FileHeader& hdr);

		utils::Path LookupFile(const std::string& file, utils::PathVector& vec);

		void DisplayItem(const InspectorLibraryVector& vec);
	}
}

#endif

