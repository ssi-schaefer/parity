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

#ifndef __RELOCATION_H__
#define __RELOCATION_H__

#include <cstring>
#include <string>
#include <vector>

#include <GenericFile.h>
#include <MemoryFile.h>
#include <Pointer.h>
#include <Exception.h>

namespace parity
{
	namespace binary
	{
		class Section;

		class Relocation {
		public:
			//
			// Contructors and Destructors
			//
			Relocation(void* ptr) : struct_(*(RelStruct*)ptr) {}
		private:
			friend class Section;
			Relocation() : struct_() {}
		public:

			//
			// Public Types
			//
			typedef enum {
				TypeAmd64Absolute		= 0x0000,
				TypeAmd64Addr64			= 0x0001,
				TypeAmd64Addr32			= 0x0002,
				TypeAmd64Addr32NoBase	= 0x0003,
				TypeAmd64Relative32		= 0x0004,
				TypeAmd64Relative32Byte1= 0x0005,
				TypeAmd64Relative32Byte2= 0x0006,
				TypeAmd64Relative32Byte3= 0x0007,
				TypeAmd64Relative32Byte4= 0x0008,
				TypeAmd64Relative32Byte5= 0x0009,
				TypeAmd64Section		= 0x000a,
				TypeAmd64SectionRelative= 0x000b,
				TypeAmd64SectionRelative7= 0x000c,
				TypeAmd64Token			= 0x000d,
				TypeAmd64SpanRelative32	= 0x000e,
				TypeAmd64Pair			= 0x000f,
				TypeAmd64SignedSpan32	= 0x0010
			} Amd64TypeFlags;

			typedef enum {
				TypeArmAbsolute			= 0x0000,
				TypeArmAddr32			= 0x0001,
				TypeArmAddr32NoBase		= 0x0002,
				TypeArmBranch24			= 0x0003,
				TypeArmBranch11			= 0x0004,
				TypeArmSection			= 0x000e,
				TypeArmSectionRelative	= 0x000f
			} ArmTypeFlags;

			typedef enum {
				i386Absolute			= 0x0000,
				i386Direct16			= 0x0001,
				i386Relative16			= 0x0002,
				i386Direct32			= 0x0006,
				i386Direct32NoBase		= 0x0007,
				i386Segment12			= 0x0009,
				i386Section				= 0x000a,
				i386SectionRelative		= 0x000b,
				i386Token				= 0x000c,
				i386SectionRelative7	= 0x000d,
				i386Relative32			= 0x0014
			} i386TypeFlags;

			typedef enum {
				IA64Absolute			= 0x0000,
				IA64Immediate14			= 0x0001,
				IA64Immediate22			= 0x0002,
				IA64Immediate64			= 0x0003,
				IA64Direct32			= 0x0004,
				IA64Direct64			= 0x0005,
				IA64PCRelative21B		= 0x0006,
				IA64PCRelative21M		= 0x0007,
				IA64PCRelative21F		= 0x0008,
				IA64GPRelative22		= 0x0009,
				IA64LiteralTableOffset22= 0x000a,
				IA64Section				= 0x000b,
				IA64SectionRelative22	= 0x000c,
				IA64SectionRelative64I	= 0x000d,
				IA64SectionRelative32	= 0x000e,
				IA64Direct32NoBase		= 0x0010,
				IA64SignedRelative14	= 0x0011,
				IA64SignedRelative22	= 0x0012,
				IA64SignedRelative32	= 0x0013,
				IA64UnsignedRelative32	= 0x0014,
				IA64PCRelative60X		= 0x0015,
				IA64PCRelative60B		= 0x0016,
				IA64PCRelative60F		= 0x0017,
				IA64PCRelative60I		= 0x0018,
				IA64PCRelative60M		= 0x0019,
				IA64ImmediateGPRelative64= 0x001a,
				IA64Token				= 0x001b,
				IA64GPRelative32		= 0x001c,
				IA64Append				= 0x001f
			} IA64TypeFlags;

			//
			// More Type Flags for other CPUs go here (maybe?)
			//

			typedef std::vector<Relocation> RelocationVector;

			//
			// Complex Methods
			//
			static unsigned short getSizeInFile() { return sizeof(RelStruct); }
			void update(utils::MemoryFile& file) { file.append(&struct_, getSizeInFile()); }
		private:

			#define REL_MEMBERS \
				REL_MEM(unsigned int	,VirtualAddress			) \
				REL_MEM(unsigned int	,SymbolTableIndex		) \
				REL_MEM(unsigned short	,Type					) \

			#define REL_MEM(type, name) type name;
			#include "CoffHelperPackStart.h"
			typedef struct STRUCT_PACKING_ATTRIBUTE {
				REL_MEMBERS
			} RelStruct;
			#include "CoffHelperPackStop.h"

			#undef REL_MEM
			#define REL_MEM(type, name) \
				public: \
					type get##name() const { return struct_.name; } \
					void set##name(type val) { struct_.name = val; } \

			//
			// Simple Getters and Setters
			//
			REL_MEMBERS

		private:
			//
			// Attributes
			//
			RelStruct struct_;
		};
	}
}

#endif

