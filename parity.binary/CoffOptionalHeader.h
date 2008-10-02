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

#ifndef __OPTIONALHEADER_H__
#define __OPTIONALHEADER_H__

#include <Exception.h>
#include <Context.h>

namespace parity
{
	namespace binary
	{
		class FileHeader;

		class OptionalHeader {
		public:
			//
			// Contructors and Destructors
			//
			OptionalHeader(void* ptr) : hdr_(*reinterpret_cast<OHStruct*>(ptr)) { if(hdr_.Magic != 0x10b) throw utils::Exception("unsupported or invalid optional header magic: %x\n", hdr_.Magic); }
			OptionalHeader() : hdr_() { /* should not be neccessary: ::memset(&hdr_, 0, sizeof(OHStruct)); */ }

			//
			// Public Types
			//
			typedef enum {
				//	RESERVED			= 0x0001,
				//	RESERVED			= 0x0002,
				//	RESERVED			= 0x0004,
				//	RESERVED			= 0x0008,
				DllDynamicBase			= 0x0040,
				DllForceIntegrity		= 0x0080,
				DllNXCompat				= 0x0100,
				DllNoIsolation			= 0x0200,
				DllNoSEH				= 0x0400,
				DllNoBind				= 0x0800,
				//	RESERVED			= 0x1000,
				DllWDMDriver			= 0x2000,
				DllTerminalServerAware	= 0x4000
			} DllCharacteristicsFlags;

			typedef utils::SubsystemType Subsystem;

			//
			// Complex Methods
			//

			bool hasExportTable() { return (getNumberOfRvaAndSizes() >= 1 && getExportTableSize() > 0); }
			bool hasImportTable() { return (getNumberOfRvaAndSizes() >= 2 && getImportTableSize() > 0); }
			bool hasResourceTable() { return (getNumberOfRvaAndSizes() >= 3 && getResourceTableSize() > 0); }
			bool hasExceptionTable() { return (getNumberOfRvaAndSizes() >= 4 && getExceptionTableSize() > 0); }
			bool hasCertificateTable() { return (getNumberOfRvaAndSizes() >= 5 && getCertificateTableSize() > 0); }
			bool hasBaseRelocTable() { return (getNumberOfRvaAndSizes() >= 6 && getBaseRelocTableSize() > 0); }
			bool hasDebugTable() { return (getNumberOfRvaAndSizes() >= 7 && getDebugTableSize() > 0); }
			bool hasGlobalPointer() { return (getNumberOfRvaAndSizes() >= 9 && getGlobalPointerRVA() != 0); }
			bool hasTLSTable() { return (getNumberOfRvaAndSizes() >= 10 && getTLSTableSize() > 0); }
			bool hasLoadConfigTable() { return (getNumberOfRvaAndSizes() >= 11 && getLoadConfigTableSize() > 0); }
			bool hasBoundImportTable() { return (getNumberOfRvaAndSizes() >= 12 && getBoundImportTableSize() > 0); }
			bool hasImportAddressTable() { return (getNumberOfRvaAndSizes() >= 13 && getImportAddressTableSize() > 0); }
			bool hasDelayImportTable() { return (getNumberOfRvaAndSizes() >= 14 && getDelayImportTableSize() > 0); }
			bool hasCLRRuntimeTable() { return (getNumberOfRvaAndSizes() >= 15 && getCLRRuntimeTableSize() > 0); }
			
			static unsigned short getSizeInFile() { return sizeof(OHStruct); }
			void update(utils::MemoryFile& file) { file.append(&hdr_, getSizeInFile()); }
		private:
			//
			// TODO: PE32+ support
			//

			#define OH_MEMBERS \
				OH_MEM  (unsigned short		,Magic					) \
				OH_MEM  (unsigned char		,MajorLinkerVersion		) \
				OH_MEM  (unsigned char		,MinorLinkerVersion		) \
				OH_MEM  (unsigned int		,SizeOfCode				) \
				OH_MEM  (unsigned int		,SizeOfInitializedData	) \
				OH_MEM  (unsigned int		,SizeOfUninitializedData) \
				OH_MEM  (unsigned int		,AddressOfEntryPoint	) \
				OH_MEM  (unsigned int		,BaseOfCode				) \
				OH_MEM  (unsigned int		,BaseOfData				) /* Not in PE32+ */ \
				OH_MEM  (unsigned int		,ImageBase				) /* 8 bytes in PE32+, must be multiple of 64K */ \
				OH_MEM  (unsigned int		,SectionAlignment		) /* greater or equal to FileAlignment */ \
				OH_MEM  (unsigned int		,FileAlignment			) /* must be power of 2 */ \
				OH_MEM  (unsigned short		,MajorOSVersion			) \
				OH_MEM  (unsigned short		,MinorOSVersion			) \
				OH_MEM  (unsigned short		,MajorImageVersion		) \
				OH_MEM  (unsigned short		,MinorImageVersion		) \
				OH_MEM  (unsigned short		,MajorSubsystemVersion	) \
				OH_MEM  (unsigned short		,MinorSubsystemVersion	) \
				OH_MEM_P(unsigned int		,Reserved_Win32Version	) /* RESERVED must be zero */ \
				OH_MEM  (unsigned int		,SizeOfImage			) /* must be multiple of SectionAlignment */ \
				OH_MEM  (unsigned int		,SizeOfHeaders			) \
				OH_MEM  (unsigned int		,CheckSum				) \
				OH_MEM  (unsigned short		,Subsystem				) \
				OH_MEM  (unsigned short		,DllCharacteristics		) \
				OH_MEM  (unsigned int		,SizeOfStackReserve		) /* 8 bytes in PE32+ */ \
				OH_MEM  (unsigned int		,SizeOfStackCommit		) /* 8 bytes in PE32+ */ \
				OH_MEM  (unsigned int		,SizeOfHeapReserve		) /* 8 bytes in PE32+ */ \
				OH_MEM  (unsigned int		,SizeOfHeapCommit		) /* 8 bytes in PE32+ */ \
				OH_MEM_P(unsigned int		,Reserved_LoaderFlags	) /* RESERVED must be zero */ \
				OH_MEM  (unsigned int		,NumberOfRvaAndSizes	) /* Description of following Data Directories */ \
				OH_MEM  (unsigned int		,ExportTableRVA			) /* DATA DIR (01) [ Export Table ] */ \
				OH_MEM  (unsigned int		,ExportTableSize		) /* DATA DIR (01) [ Export Table ] */ \
				OH_MEM  (unsigned int		,ImportTableRVA			) /* DATA DIR (02) [ Import Table ] */ \
				OH_MEM  (unsigned int		,ImportTableSize		) /* DATA DIR (02) [ Import Table ] */ \
				OH_MEM  (unsigned int		,ResourceTableRVA		) /* DATA DIR (03) [ Resource Table ] */ \
				OH_MEM  (unsigned int		,ResourceTableSize		) /* DATA DIR (03) [ Resource Table ] */ \
				OH_MEM  (unsigned int		,ExceptionTableRVA		) /* DATA DIR (04) [ Exception Table ] */ \
				OH_MEM  (unsigned int		,ExceptionTableSize		) /* DATA DIR (04) [ Exception Table ] */ \
				OH_MEM  (unsigned int		,CertificateTableRVA	) /* DATA DIR (05) [ Certificate Table ] (FILE POINTER!) */ \
				OH_MEM  (unsigned int		,CertificateTableSize	) /* DATA DIR (05) [ Certificate Table ] */ \
				OH_MEM  (unsigned int		,BaseRelocTableRVA		) /* DATA DIR (06) [ BaseReloc Table ] */ \
				OH_MEM  (unsigned int		,BaseRelocTableSize		) /* DATA DIR (06) [ BaseReloc Table ] */ \
				OH_MEM  (unsigned int		,DebugTableRVA			) /* DATA DIR (07) [ Debug Table ] */ \
				OH_MEM  (unsigned int		,DebugTableSize			) /* DATA DIR (07) [ Debug Table ] */ \
				OH_MEM_P(unsigned int		,Reserved_ArchitectureA	) /* DATA DIR (08) [ Architecture Table ] */ \
				OH_MEM_P(unsigned int		,Reserved_ArchitectureS	) /* DATA DIR (08) [ Architecture Table ] */ \
				OH_MEM  (unsigned int		,GlobalPointerRVA		) /* DATA DIR (09) [ Global Pointer ] */ \
				OH_MEM_P(unsigned int		,Reserved_GPSize		) /* DATA DIR (09) [ Global Pointer (must be zero)] */ \
				OH_MEM  (unsigned int		,TLSTableRVA			) /* DATA DIR (10) [ TLS Table ] */ \
				OH_MEM  (unsigned int		,TLSTableSize			) /* DATA DIR (10) [ TLS Table ] */ \
				OH_MEM  (unsigned int		,LoadConfigTableRVA		) /* DATA DIR (11) [ LoadConfig Table ] */ \
				OH_MEM  (unsigned int		,LoadConfigTableSize	) /* DATA DIR (11) [ LoadConfig Table ] */ \
				OH_MEM  (unsigned int		,BoundImportTableRVA	) /* DATA DIR (12) [ Bound Import Table ] */ \
				OH_MEM  (unsigned int		,BoundImportTableSize	) /* DATA DIR (12) [ Bound Import Table ] */ \
				OH_MEM  (unsigned int		,ImportAddressTableRVA	) /* DATA DIR (13) [ Import Address Table ] */ \
				OH_MEM  (unsigned int		,ImportAddressTableSize	) /* DATA DIR (13) [ Import Address Table ] */ \
				OH_MEM  (unsigned int		,DelayImportTableRVA	) /* DATA DIR (14) [ Delay Import Table ] */ \
				OH_MEM  (unsigned int		,DelayImportTableSize	) /* DATA DIR (14) [ Delay Import Table ] */ \
				OH_MEM  (unsigned int		,CLRRuntimeTableRVA		) /* DATA DIR (15) [ CLR Runtime Table ] */ \
				OH_MEM  (unsigned int		,CLRRuntimeTableSize	) /* DATA DIR (15) [ CLR Runtime Table ] */ \
				OH_MEM_P(unsigned int		,Reserved_LastRVA		) /* DATA DIR (16) [ Reserved ] */ \
				OH_MEM_P(unsigned int		,Reserved_LastSize		) /* DATA DIR (16) [ Reserved ] */ \


			#define OH_MEM(type, name) type name;
			#define OH_MEM_P(type, name) type name;
			#include "CoffHelperPackStart.h"
			typedef struct STRUCT_PACKING_ATTRIBUTE {
				OH_MEMBERS
			} OHStruct;
			#include "CoffHelperPackStop.h"

			#undef OH_MEM
			#undef OH_MEM_P
			#define OH_MEM_P(type, name)
			#define OH_MEM(type, name) \
				public: \
					type get##name() { return hdr_.name; } \
					void set##name(type val) { hdr_.name = val; } \

			//
			// Simple Getters and Setters
			//
			OH_MEMBERS

		private:		
			//
			// Attributes
			//
			OHStruct hdr_;
		};
	}
}

#endif

