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

#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include <cstring>
#include <string>
#include <vector>
#include <map>

#include <GenericFile.h>
#include <MemoryFile.h>
#include <Pointer.h>
#include <Exception.h>

#include "CoffAuxSymbol.h"

namespace parity
{
	namespace binary
	{
		class FileHeader;
		class Section;

		class Symbol {
		public:
			//
			// Contructors and Destructors
			//
			Symbol(const FileHeader* fh, long idx, void* ptr) : idx_(idx) { if(ptr) { struct_ = *(SymStruct*)ptr; calcName(fh); } }
			Symbol(FileHeader* fh, long idx, const std::string& name);
			
			//
			// Public Types
			//
			typedef enum {
				SymbolUndefined	= 0,
				SymbolAbsolute	= -1,
				SymbolDebug		= -2,
			} SectionNumberFlags;

			//
			// The following are *not* used by Microsoft tools!
			//
			typedef enum {
				TypeNull	= 0,
				TypeVoid	= 1,
				TypeChar	= 2,
				TypeShort	= 3,
				TypeInt		= 4,
				TypeLong	= 5,
				TypeFloat	= 6,
				TypeDouble	= 7,
				TypeStruct	= 8,
				TypeUnion	= 9,
				TypeEnum	= 10,
				TypeMoe		= 11,
				TypeByte	= 12,
				TypeWord	= 13,
				TypeUint	= 14,
				TypeDWord	= 15,
			} BaseTypes;

			//
			// Microsoft tools *only* use ComplexFunction to tell wether a symbol
			// is a function. This means that the Type field may only be 0x00 or
			// 0x20 (since the above BaseTypes are not used either).
			//
			typedef enum {
				ComplexNull		= 0x00,
				ComplexPointer	= 0x10,
				ComplexFunction	= 0x20,
				ComplexArray	= 0x30,
			} ComplexTypes;

			typedef enum {
				ClassEndOfFunction	= 0xff,
				ClassNull			= 0,
				ClassAutomatic		= 1,
				ClassExternal		= 2,
				ClassStatic			= 3,
				ClassRegister		= 4,
				ClassExternalDef	= 5,
				ClassLabel			= 6,
				ClassUndefinedLabel	= 7,
				ClassMemberOfStruct	= 8,
				ClassArgument		= 9,
				ClassStructTag		= 10,
				ClassMemberOfUnion	= 11,
				ClassUnionTag		= 12,
				ClassTypeDefinition	= 13,
				ClassUndefinedStatic= 14,
				ClassEnumTag		= 15,
				ClassMemberOfEnum	= 16,
				ClassRegisterArgument= 17,
				ClassBitField		= 18,
				ClassBlock			= 100,
				ClassFunction		= 101,
				ClassEndOfStruct	= 102,
				ClassFile			= 103,
				ClassSection		= 104,	// MS Tools use ClassStatic instead.
				ClassWeakExternal	= 105,
				ClassCLRToken		= 107,
			} StorageClassFlags;

			typedef std::vector<Symbol> SymbolVector;
			typedef std::map<int, Symbol> IndexedSymbolMap;

			//
			// Complex Methods
			//
			int getIndex() { return idx_; }
			Section getSection() const;

			static unsigned short getSizeInFile() { return sizeof(SymStruct); }
			void update(utils::MemoryFile& file);

			//
			// required to be able to use symbol as key for a map!
			//
			bool operator<(const Symbol& rhs) const { return name_ < rhs.name_; }
			bool operator==(const Symbol& rhs) const { return (name_ == rhs.name_ && struct_.Type == rhs.struct_.Type && struct_.SectionNumber == rhs.struct_.SectionNumber); }
			bool operator!=(const Symbol& rhs) const { return (name_ != rhs.name_ || struct_.Type != rhs.struct_.Type || struct_.SectionNumber != rhs.struct_.SectionNumber); }
		private:

			//
			// To support the correctly indexed map, we need a default constructor
			// that is accessable from the map's operator[]
			//
			Symbol() : idx_(-11) { throw utils::Exception("construction of invalid symbol!"); }
			friend IndexedSymbolMap::mapped_type& IndexedSymbolMap::operator [](const IndexedSymbolMap::key_type&);

			#define SYM_MEMBERS \
				SYM_MEM(unsigned int	,Value				) \
				SYM_MEM(short			,SectionNumber		) \
				SYM_MEM(unsigned short	,Type				) /* either function (0x20) or not (0x0) */ \
				SYM_MEM(unsigned char	,StorageClass		) \
				SYM_MEM(unsigned char	,NumberOfAuxSymbols	) \

			#define SYM_MEM(type, name) type name;
			#include "CoffHelperPackStart.h"
			typedef struct STRUCT_PACKING_ATTRIBUTE {
				union {
					char ShortName[8];
					struct {
						unsigned int Zeros;
						unsigned int Offset;
					} N;
				} N;
				SYM_MEMBERS
			} SymStruct;
			#include "CoffHelperPackStop.h"

			#undef SYM_MEM
			#define SYM_MEM(type, name) \
				public: \
					type get##name() const { return struct_.name; } \
					void set##name(type val) { struct_.name = val; } \

			//
			// Simple Getters and Setters
			//
		public:
			const std::string& getName() const { return name_; }

			AuxSymbol::AuxSymbolVector& getAuxSymbols() { return aux_; }
			void addAuxSymbol(void* ptr) { aux_.push_back(AuxSymbol(ptr)); }
			void addAuxSymbol(AuxSymbol& aux) { aux_.push_back(aux); }

			SYM_MEMBERS

		private:
			void calcName(const FileHeader* fh);

			//
			// Attributes
			//
			long idx_;
			SymStruct struct_;

			std::string name_;
			AuxSymbol::AuxSymbolVector aux_;
		};
	}
}

#endif

