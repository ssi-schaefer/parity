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

#include "TestSuite.h"

#include <MappedFile.h>
#include <Exception.h>
#include <CoffFile.h>
#include <CoffImage.h>
#include <CoffObject.h>
#include <CoffArchive.h>
#include <CoffDirectiveSection.h>

#include <Timing.h>

#include <iomanip>

namespace utils = parity::utils;

namespace parity
{
	namespace testing
	{
		static unsigned long countAllSyms = 0;
		static unsigned long countAllImps = 0;

		void testFileHeader(binary::FileHeader& hdr)
		{
			if(!hdr.isValid())
				throw utils::Exception("invalid header of binary!");

			binary::Section::IndexedSectionMap vec = hdr.getSections();
			binary::Symbol::IndexedSymbolMap syms = hdr.getAllSymbols();

			for(binary::Section::IndexedSectionMap::iterator it = vec.begin(); it != vec.end(); ++it)
			{
				utils::Log::verbose("Section: %-10s, addr: %p, size: %p\n", it->second.getName().c_str(), it->second.getVirtualAddress(), it->second.getVirtualSize());

				//
				// Check if its a directive sections, and handle it
				//

				if(it->second.getName() == ".drectve" && it->second.getCharacteristics() & binary::Section::CharLinkInfo)
				{
					binary::DirectiveSection drectve(it->second);
					binary::DirectiveSection::DirectiveVector directives = drectve.getDirectives();

					for(binary::DirectiveSection::DirectiveVector::iterator dir = directives.begin(); dir != directives.end(); ++dir)
					{
						utils::Log::verbose(" * DIRECTIVE: %s\n", dir->c_str());
					}
				}

				binary::Relocation::RelocationVector relocs = it->second.getRelocations();

				//
				// The Relocations are that many, that there are about 1.5 mio. iterator constructions
				// in a test run with a C++ library, so only do this if required!
				//
				
				for(binary::Relocation::RelocationVector::iterator rel = relocs.begin(); rel != relocs.end(); ++rel)
				{
					utils::Log::verbose(" * RELOC: Address: %p, Index: %d, Type: %d (Symbol: %s)\n", rel->getVirtualAddress(), rel->getSymbolTableIndex(), rel->getType(), syms[rel->getSymbolTableIndex()].getName().c_str());
				}

				for(binary::Symbol::IndexedSymbolMap::iterator sym = syms.begin(); sym != syms.end(); ++sym)
				{
					++countAllSyms;

					if(sym->second.getSectionNumber() == static_cast<short>(it->first))
						utils::Log::verbose(" * SYM: Index: %5d, Name:, %s\n", sym->first, sym->second.getName().c_str());
				}
			}
		}

		static bool checkStructureSizes()
		{
			utils::Log::verbose("checking structure sizes for correctness...\n");

			#define EXPECTED_SIZE_FILEHEADER 20
			#define EXPECTED_SIZE_OPTIONALHEADER 224
			#define EXPECTED_SIZE_RELOCATION 10
			#define EXPECTED_SIZE_SECTION 40
			#define EXPECTED_SIZE_SYMBOL 18

			unsigned short FileHeaderSize = binary::FileHeader::getSizeInFile();
			unsigned short OptionalHeaderSize = binary::OptionalHeader::getSizeInFile();
			unsigned short RelocationSize = binary::Relocation::getSizeInFile();
			unsigned short SectionSize = binary::Section::getSizeInFile();
			unsigned short SymbolSize = binary::Symbol::getSizeInFile();

			utils::Log::verbose(
				"FileHeaderSize:     %d (%s)\n"
				"OptionalHeaderSize: %d (%s)\n"
				"RelocationSize:     %d (%s)\n"
				"SectionSize:        %d (%s)\n"
				"SymbolSize:         %d (%s)\n"
				, FileHeaderSize, (FileHeaderSize == EXPECTED_SIZE_FILEHEADER ? "ok" : "fail")
				, OptionalHeaderSize, (OptionalHeaderSize == EXPECTED_SIZE_OPTIONALHEADER ? "ok" : "fail")
				, RelocationSize, (RelocationSize == EXPECTED_SIZE_RELOCATION ? "ok" : "fail")
				, SectionSize, (SectionSize == EXPECTED_SIZE_SECTION ? "ok" : "fail")
				, SymbolSize, (SymbolSize == EXPECTED_SIZE_SYMBOL ? "ok" : "fail"));

			if(FileHeaderSize != EXPECTED_SIZE_FILEHEADER
			 || OptionalHeaderSize != EXPECTED_SIZE_OPTIONALHEADER
			 || RelocationSize != EXPECTED_SIZE_RELOCATION
			 || SectionSize != EXPECTED_SIZE_SECTION
			 || SymbolSize != EXPECTED_SIZE_SYMBOL)
				return false;

			return true;
		}

		bool TestSuite::testParityBinaryObject()
		{
			try {
				utils::Context& ctx = utils::Context::getContext();

				if(!checkStructureSizes())
					return false;

				countAllSyms = 0;

				for(utils::PathVector::iterator it = ctx.getObjectsLibraries().begin(); it != ctx.getObjectsLibraries().end(); ++it)
				{
					utils::Path pth(*it);
					pth.toNative();
					utils::MappedFile map(pth, utils::ModeRead);
					switch(binary::File::getType(&map))
					{
					case binary::File::TypeImage:
						{
							binary::Image img(&map);
							binary::FileHeader hdr = img.getHeader();

							if(!hdr.isValid())
								throw utils::Exception("invalid header of image!");

							binary::OptionalHeader ohdr = hdr.getOptionalHeader();
							binary::Section::IndexedSectionMap vec = hdr.getSections();

							for(binary::Section::IndexedSectionMap::iterator it = vec.begin(); it != vec.end(); ++it)
							{
								utils::Log::verbose("Section: %-10s, addr: %p, size: %p\n", it->second.getName().c_str(), it->second.getVirtualAddress(), it->second.getVirtualSize());
							}
						}
						break;
					case binary::File::TypeLibrary:
						{
							binary::Archive arch(&map);

							binary::Archive::MemberMap members = arch.getMembers();
							binary::Archive::ImportMap imports = arch.getImports();

							utils::Log::verbose("found %d members, and %d imports.\n", members.size(), imports.size());

							for(binary::Archive::MemberMap::iterator it = members.begin(); it != members.end(); ++it)
							{
								utils::Log::verbose("PROCESSING: %s\n", it->first.c_str());
								testFileHeader(it->second);
							}

							for(binary::Archive::ImportMap::iterator it = imports.begin(); it != imports.end(); ++it)
							{
								++countAllImps;
								utils::Log::verbose("IMPORT: %s, Library: %s, Symbol: %s\n", it->first.c_str(), it->second.getLibraryName().c_str(), it->second.getSymbolName().c_str());
							}
						}
						break;
					case binary::File::TypeObject:
						{
							binary::Object obj(&map);
							binary::FileHeader hdr = obj.getHeader();

							testFileHeader(hdr);
						}
						break;
					default:
						utils::Log::verbose("ignoring unsupported file type for file: %s", it->get().c_str());
					}
				}
				
				return true;
			} catch(const utils::Exception& e)
			{
				utils::Log::warning("catched: %s\n", e.what());
			}

			return false;
		}
	}
}

