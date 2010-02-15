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

#include "CoffArchive.h"
#include "CoffFile.h"

#include <Log.h>

namespace parity
{
	namespace binary
	{
		//
		// private structures used during parsing of the archive.
		// This is somehow very specialized for my usecase, since
		// i don't want to get myself into the stupidness of the
		// archive file format (oh man, this is sooo stupid!)
		//

		typedef struct {
			char Name[16];
			char Date[12];
			char UID[6];
			char GID[6];
			char Mode[8];
			char Size[10];
			char EOH[2];
		} ArchiveHeaderStruct;

		typedef struct {
			unsigned short	Sig1;
			unsigned short	Sig2;
		} PartialImportDescriptor;

		void Archive::gatherMembersAndImports()
		{
			//
			// the first linker member is at offset 8 (directly after the header "!<arch>\n"
			//
			ArchiveHeaderStruct* hdr = MAKEPTR(ArchiveHeaderStruct*, file_->getBase(), 8);
			const char* longnames = 0;

			while(hdr)
			{
				//
				// calculate member name
				//
				std::string name;

				if(hdr->Name[0] == '\0') {
					// oups - member without a name? we'll just skip that...
					utils::Log::verbose("Skipping unnamed member from archive '%s'\n", file_->getPath().get().c_str());
				} else if(hdr->Name[0] == '/') {
					if(hdr->Name[1] == ' ') {}
					else if(hdr->Name[1] == '/') {
						longnames = reinterpret_cast<const char*>((hdr+1));
					} else if(!isdigit(hdr->Name[1])) {
						throw utils::Exception("unexpected charackter \"%c\" in archive member name!", hdr->Name[1]);
					} else {
						//
						// name is in "/offset" format where offset is a decimal offset into longnames member
						//
						if(!longnames)
							throw utils::Exception("missing longnames member, but found offset into it!");

						name = MAKEPTR(char*, longnames, atoi(&hdr->Name[1]));
					}
				} else {
					//
					// name is in "name/" format
					//
					name = std::string(hdr->Name, reinterpret_cast<long>(::strchr(hdr->Name, '/')) - reinterpret_cast<long>(hdr->Name));
				}

				if(!name.empty())
				{
					//
					// create either a object member entry or an import record entry.
					//
					PartialImportDescriptor* imp = reinterpret_cast<PartialImportDescriptor*>(hdr+1);

					if(imp->Sig1 == 0x0 && imp->Sig2 == 0xFFFF)
					{
						//
						// Import record found.
						//
						imports_.insert(ImportMap::value_type(name, Import(file_, reinterpret_cast<void*>(imp))));
					} else {
						//
						// Object found?
						//
						if(FileHeader::isValidMachine(*reinterpret_cast<unsigned short*>(imp)))
						{
							members_.insert(MemberMap::value_type(name, FileHeader(file_, reinterpret_cast<void*>(imp), true)));
						} else {
							utils::Log::warning("invalid object signature: %p in library: %s, member: %s\n", reinterpret_cast<void*>(*reinterpret_cast<unsigned short*>(imp)), file_->getPath().get().c_str(), name.c_str());
						}
					}
				}

				//
				// skip to the next member
				//

				unsigned int size = atoi(hdr->Size) + sizeof(ArchiveHeaderStruct);
				//
				// ensure 2-byte align!
				//
				size = (size+1) & ~1;
				hdr = MAKEPTR(ArchiveHeaderStruct*, hdr, size);

				if(hdr >= file_->getTop())
					break;

				if(hdr->EOH[0] != 0x60 || hdr->EOH[1] != 0x0A)
					break;
			}
		}
	}
}

