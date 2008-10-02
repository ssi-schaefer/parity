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

#ifndef __FILEHEADER_H__
#define __FILEHEADER_H__

#include <cstring>
#include <vector>
#include <string>
#include <map>

#include <Exception.h>
#include <GenericFile.h>
#include <MemoryFile.h>

#include "CoffOptionalHeader.h"
#include "CoffSection.h"
#include "CoffSymbol.h"

namespace parity
{
	namespace binary
	{
		class FileHeader {
		public:
			//
			// Contructors and Destructors
			//
			FileHeader(const utils::GenericFile* file, void* ptr, bool isArch);
			FileHeader();

			FileHeader(FileHeader const& rhs)
				: start_(rhs.start_)
				, file_(rhs.file_)
				, hdr_(rhs.hdr_)
				, stringtable_(rhs.stringtable_)
				, map_(rhs.map_)
				, sections_(rhs.sections_)
				, nextStringOffset_(rhs.nextStringOffset_)
				, nextSectionIndex_(rhs.nextSectionIndex_)
				, nextSymbolIndex_(rhs.nextSymbolIndex_)
				, isArchive_(rhs.isArchive_) {}

			FileHeader& operator=(FileHeader const& rhs) { 
				start_				= rhs.start_;
				file_				= rhs.file_;
				hdr_				= rhs.hdr_;
				stringtable_		= rhs.stringtable_;
				map_				= rhs.map_;
				sections_			= rhs.sections_;
				nextStringOffset_	= rhs.nextStringOffset_;
				nextSectionIndex_	= rhs.nextSectionIndex_;
				nextSymbolIndex_	= rhs.nextSymbolIndex_;
				isArchive_			= rhs.isArchive_;
				return *this;
			}

			//
			// Public Types
			//
			typedef enum {
				//
				// Note: Not all types implemented here!
				//
				MachineUnknown	= 0x0, // applicable to all machines if encountered (c runtime, etc.)
				MachineI386		= 0x14c,
				MachineAMD64	= 0x8664,
				MachineARM		= 0x1c0,
				MachineIA64		= 0x200
			} MachineType;

			typedef enum {
				CharRelocsStripped			= 0x0001,
				CharExecutableImage			= 0x0002,
				CharLineNumbersStripped		= 0x0004,
				CharLocalSymbolsStripped	= 0x0008,
				CharAgressiveWSTrim			= 0x0010,
				CharLargeAddressAware		= 0x0020,
				// RESERVED					= 0x0040,
				CharLittleEndian			= 0x0080,
				Char32BitMachine			= 0x0100,
				CharDebugStripped			= 0x0200,
				CharRemovableRunFromSwap	= 0x0400,
				CharNetRunFromSwap			= 0x0800,
				CharSystemFile				= 0x1000,
				CharDllFile					= 0x2000,
				CharUniprocessorOnly		= 0x4000,
				CharBigEndian				= 0x8000
			} CharacteristicsFlags;

			typedef std::vector<FileHeader> FileHeaderVector;
			typedef std::map<unsigned int, std::string> OffsetStringMap;

			//
			// Complex Methods
			//
			bool isValid() const;
			static bool isValidMachine(unsigned short mach);
			OptionalHeader getOptionalHeader() const;

			Section::IndexedSectionMap& getSections() { return sections_; }
			const std::string& getStringFromOffset(unsigned int offset) const;
			const utils::GenericFile* getFile() const { return file_; }
			Section& getSection(const std::string& name);
			Section& getSection(unsigned int idx);
			Section& getSectionForRVA(unsigned int rva);
			void* getPointerFromRVA(unsigned int rva);
			void* getPointerFromVA(unsigned int va);

			Symbol::IndexedSymbolMap& getAllSymbols() { return map_; }
			const Symbol::IndexedSymbolMap& getAllSymbols() const { return map_; }

			void* getBasePointer() const { if(isArchive_) { return start_; } else { if(file_) return file_->getBase(); else throw utils::Exception("not file set!"); } }

			//
			// methods for creating new files.
			//
			unsigned int addString(const std::string& str);
			Section& addSection(const std::string& name);
			Symbol& addSymbol(const std::string& name);
			void addAuxSymbol(Symbol& sym, AuxSymbol& aux);

			static unsigned short getSizeInFile() { return sizeof(FHStruct); }
		private:
			void lookupStringTab();
			void lookupSymbols();
			void lookupSections();

			void update(utils::MemoryFile& file, OptionalHeader* opt);
			//
			// object and image instances are allowed to update to file.
			//
			friend class Object;
			friend class Image;

			#define BIN_MEMBERS \
				BIN_MEM  (unsigned short	,Machine				) \
				BIN_MEM  (unsigned short	,NumberOfSections		) \
				BIN_MEM  (unsigned int		,TimeDateStamp			) \
				BIN_MEM_P(unsigned int		,PointerToSymbolTable	) \
				BIN_MEM  (unsigned int		,NumberOfSymbols		) \
				BIN_MEM_P(unsigned short	,SizeOfOptionalHeader	) \
				BIN_MEM  (unsigned short	,Characteristics		) \

			#define BIN_MEM(type, name) type name;
			#define BIN_MEM_P(type, name) type name;
			#include "CoffHelperPackStart.h"
			typedef struct STRUCT_PACKING_ATTRIBUTE {
				BIN_MEMBERS
			} FHStruct;
			#include "CoffHelperPackStop.h"

			#undef BIN_MEM
			#undef BIN_MEM_P
			#define BIN_MEM_P(type, name)
			#define BIN_MEM(type, name) \
				public: \
					type get##name() const { return hdr_.name; } \
					void set##name(type val) { hdr_.name = val; } \

			//
			// Simple Getters and Setters
			//
			BIN_MEMBERS

		private:
			//
			// Attributes
			//
			void * start_;
			const utils::GenericFile* file_;

			FHStruct hdr_;
			OffsetStringMap stringtable_;
			Symbol::IndexedSymbolMap map_;
			Section::IndexedSectionMap sections_;

			unsigned int nextStringOffset_;
			unsigned int nextSectionIndex_;
			unsigned int nextSymbolIndex_;

			bool isArchive_;
		};
	}
}

#endif

