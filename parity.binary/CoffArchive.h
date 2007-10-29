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

#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

#include <cstring>
#include <map>
#include <string>

#include <MappedFile.h>
#include <Pointer.h>

#include "CoffFileHeader.h"
#include "CoffImport.h"

namespace parity
{
	namespace binary
	{
		//
		// This class is READ ONLY! (stupid archive format... ;o))
		//
		class Archive {
		public:
			Archive(const utils::MappedFile* ptr)
				: file_(ptr)
			{
				if(::strncmp((const char*)ptr->getBase(), "!<arch>\n", 8) != 0)
					throw utils::Exception("not an archive!");
				gatherMembersAndImports();
			}

			typedef std::multimap<std::string, FileHeader> MemberMap;
			typedef std::multimap<std::string, Import> ImportMap;

			MemberMap& getMembers() { return members_; }
			ImportMap& getImports() { return imports_; }

			const utils::MappedFile* getFile() const { return file_; }

		private:
			void gatherMembersAndImports();

			const utils::MappedFile* file_;

			MemberMap members_;
			ImportMap imports_;
		};
	}
}

#endif

