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

#ifndef __IMPORT_H__
#define __IMPORT_H__

#include <cstring>
#include <string>
#include <vector>

#include <GenericFile.h>
#include <Pointer.h>
#include <Exception.h>

namespace parity
{
	namespace binary
	{
		class FileHeader;

		//
		// This is READ ONLY since archives are read only too.
		//
		class Import {
		public:
			//
			// Contructors and Destructors
			//
			Import(const utils::GenericFile* file, void* ptr) : file_(file), struct_(*(ImpStruct*)ptr) { calcName(ptr); }
			
			//
			// Public Types
			//
			typedef enum  {
				ImportCode = 0,
				ImportData = 1,
				ImportConst = 2
			} ImportType;

			typedef enum {
				ImportByOrdinal = 0,
				ImportByName = 1,
				ImportByNameNoPrefix = 2,
				ImportByNameUndecorate = 3
			} ImportNameType;

			typedef std::vector<Import> ImportVector;

			//
			// Complex Methods
			//
			const utils::GenericFile* getFile() const { return file_; }

			std::string getSymbolName() const { return symbol_; }
			std::string getLibraryName() const { return library_; }

			//
			// required to be able to use symbol as key for a map!
			//
			bool operator<(const Import& rhs) const { return symbol_ < rhs.symbol_; }
			bool operator==(const Import& rhs) const { return (symbol_ == rhs.symbol_ && library_ == rhs.library_ && struct_.Type == rhs.struct_.Type); }
			bool operator!=(const Import& rhs) const { return (symbol_ != rhs.symbol_ || library_ != rhs.library_ ||struct_.Type != rhs.struct_.Type); }
		private:

			#define IMP_MEMBERS \
				IMP_MEM_P(unsigned short	,Sig1				) \
				IMP_MEM_P(unsigned short	,Sig2				) \
				IMP_MEM  (unsigned short	,Version			) \
				IMP_MEM  (unsigned short	,Machine			) \
				IMP_MEM  (unsigned int		,TimeDateStamp		) \
				IMP_MEM  (unsigned int		,SizeOfData			) \
				IMP_MEM  (unsigned short	,OrdinalOrHint		) \
				IMP_MAN  (unsigned short	,Type				) \
				IMP_MAN  (unsigned short	,NameType			) \
				IMP_MAN_P(unsigned short	,Reserved			) \

			#define IMP_MEM(type, name) type name;
			#define IMP_MEM_P(type, name) type name;
			#define IMP_MAN(type, name)
			#define IMP_MAN_P(type, name)
			#include "CoffHelperPackStart.h"
			typedef struct STRUCT_PACKING_ATTRIBUTE {
				IMP_MEMBERS
				//
				// Additional Manual Members
				//
				unsigned short Type : 2;
				unsigned short NameType : 3;
				unsigned short Reserved : 11;
			} ImpStruct;
			#include "CoffHelperPackStop.h"

			#undef IMP_MEM
			#undef IMP_MEM_P
			#undef IMP_MAN
			#undef IMP_MAN_P
			#define IMP_MAN(type, name) IMP_MEM(type, name)
			#define IMP_MAN_P(type, name)
			#define IMP_MEM_P(type, name)
			#define IMP_MEM(type, name) \
				public: \
					type get##name() const { return struct_.name; } \
					void set##name(type val) { struct_.name = val; } \

			//
			// Simple Getters and Setters
			//
			IMP_MEMBERS

		private:
			void calcName(const void* ptr);

			//
			// Attributes
			//
			const utils::GenericFile* file_;
			ImpStruct struct_;

			std::string symbol_;
			std::string library_;
		};
	}
}

#endif

