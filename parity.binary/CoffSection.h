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

#ifndef __SECTION_H__
#define __SECTION_H__

#include <cstring>
#include <string>
#include <vector>

#include <GenericFile.h>
#include <MemoryFile.h>
#include <Pointer.h>
#include <Exception.h>

#include "CoffRelocation.h"
#include "CoffSymbol.h"

namespace parity
{
	namespace binary
	{
		class FileHeader;

		class Section {

			//
			// Friend classes for special Sections. Those Classes
			// should only provide a different view on the data of this section.
			//
			friend class DirectiveSection;

		public:
			//
			// Contructors and Destructors
			//
			Section(const FileHeader* fh, int idx, void* ptr);
			~Section();
			Section(Section const& rhs);
			Section& operator=(Section const& rhs);
			Section() : struct_(), idx_(-11), data_(0), allocated_(0), allocated_size_(0), relocs_(), name_("INVALID") { throw utils::Exception("invalid construction of Section object!"); }
		private:
			friend class FileHeader;
			Section(FileHeader* fh, int idx, const std::string& name);
		public:

			//
			// Public Types
			//
			typedef enum {
				//	RESERVED			= 0x00000000,
				//	RESERVED			= 0x00000001,
				//	RESERVED			= 0x00000002,
				//	RESERVED			= 0x00000004,
				CharTypeNoPadding		= 0x00000008,
				//	RESERVED			= 0x00000010,
				CharContentCode			= 0x00000020,
				CharContentInitData		= 0x00000040,
				CharContentUninitData	= 0x00000080,
				CharLinkOther			= 0x00000100,	// RESERVED
				CharLinkInfo			= 0x00000200,
				//	RESERVED			= 0x00000400,
				CharLinkRemove			= 0x00000800,
				CharLinkComdat			= 0x00001000,
				CharGPRelative			= 0x00008000,
				CharMemoryPurgable		= 0x00020000,	// RESERVED
				CharMemory16Bit			= 0x00020000,	// RESERVED (also 0x0002000 by specs... ?)
				CharMemoryLocked		= 0x00040000,	// RESERVED
				CharMemoryPreload		= 0x00080000,	// RESERVED
				CharAlign1Bytes			= 0x00100000,
				CharAlign2Bytes			= 0x00200000,
				CharAlign4Bytes			= 0x00300000,
				CharAlign8Bytes			= 0x00400000,
				CharAlign16Bytes		= 0x00500000,
				CharAlign32Bytes		= 0x00600000,
				CharAlign64Bytes		= 0x00700000,
				CharAlign128Bytes		= 0x00800000,
				CharAlign256Bytes		= 0x00900000,
				CharAlign512Bytes		= 0x00a00000,
				CharAlign1024Bytes		= 0x00b00000,
				CharAlign2048Bytes		= 0x00c00000,
				CharAlign4096Bytes		= 0x00d00000,
				CharAlign8192Bytes		= 0x00e00000,
				CharExtendedRelocations	= 0x01000000,
				CharMemoryDiscardable	= 0x02000000,
				CharMemoryNotCached		= 0x04000000,
				CharMemoryNotPaged		= 0x08000000,
				CharMemoryShared		= 0x10000000,
				CharMemoryExecute		= 0x20000000,
				CharMemoryRead			= 0x40000000,
				CharMemoryWrite			= 0x80000000
			} CharacteristicsFlags;

			typedef std::vector<Section> SectionVector;
			typedef std::map<unsigned int, Section> IndexedSectionMap;

		private:
			friend IndexedSectionMap::mapped_type& IndexedSectionMap::operator [](const IndexedSectionMap::key_type&);
		public:
			
			//
			// Complex Methods
			//
			const Relocation::RelocationVector& getRelocations() const { return relocs_; }

			void markRelocation(Symbol& sym, Relocation::i386TypeFlags flags);		// only add relocation
			void markRelocation(Symbol& sym, Relocation::Amd64TypeFlags type);
			void markSymbol(Symbol& sym);											// modifies sym to point at the current end of the data, and adds a Relocation
			void addData(const void* data, size_t len);									// only add data without setting a marker.
			void insert(FileHeader& hdr, const void* data, size_t len, size_t pos);	// insert data at given position.
			void padSection();
			void padSection(unsigned int alignment);
			void takeBufferControl();
			unsigned int getAlignment();

			static unsigned short getSizeInFile() { return sizeof(SecStruct); }
			
			void updateHeader(utils::MemoryFile& file) { file.append(&struct_, getSizeInFile()); }
			void updateDataAndRelocs(utils::MemoryFile& file);
		private:

			#define SEC_MEMBERS \
				SEC_MEM(unsigned int	,VirtualSize			) \
				SEC_MEM(unsigned int	,VirtualAddress			) \
				SEC_MEM(unsigned int	,SizeOfRawData			) \
				SEC_MEM(unsigned int	,PointerToRawData		) \
				SEC_MEM(unsigned int	,PointerToRelocations	) \
				SEC_MEM(unsigned int	,PointerToLinenumbers	) \
				SEC_MEM(unsigned short	,NumberOfRelocations	) \
				SEC_MEM(unsigned short	,NumberOfLinenumbers	) \
				SEC_MEM(unsigned int	,Characteristics		) \

			#define SEC_MEM(type, name) type name;
			#include "CoffHelperPackStart.h"
			typedef struct STRUCT_PACKING_ATTRIBUTE {
				unsigned char Name[8];
				SEC_MEMBERS
			} SecStruct;
			#include "CoffHelperPackStop.h"

			#undef SEC_MEM
			#define SEC_MEM(type, name) \
				public: \
					type get##name() const { return struct_.name; } \
					void set##name(type val) { struct_.name = val; } \

			//
			// Simple Getters and Setters
			//
		public:
			const std::string& getName() const { return name_; }
			int getIndex() const { return idx_; }
			
			void* getDataPtr() const { return data_; }

			SEC_MEMBERS

		private:
			void calcName(const FileHeader* fh);
			void lookupRelocations(const FileHeader* fh);
			void insertData(FileHeader& hdr, const void* data, size_t len, size_t pos);
			int updateOffsets(FileHeader& hdr, size_t* pos, size_t len);

			//
			// Attributes
			//
			SecStruct struct_;
			int idx_;

			//
			// data_ is set to point to allocated_ if the section data is allocated
			// by parity. allocated_ then points to the real byte array
			//
			// If an existing file is modified, copy data_ to allocated_ on first
			// modification.
			//
			void * data_;
			void * allocated_;
			size_t allocated_size_;

			Relocation::RelocationVector relocs_;
			std::string name_;
		};
	}
}

#endif

