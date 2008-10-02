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

#ifndef __SECTIONAUXSYMBOL_H__
#define __SECTIONAUXSYMBOL_H__

#include <Pointer.h>
#include <Exception.h>

#include "CoffAuxSymbol.h"

namespace parity
{
	namespace binary
	{
		class SectionAuxSymbol : public AuxSymbol {
		private:
			SectionAuxSymbol(); /* private, only cast to this */

			#define AUXSEC_MEMBERS \
				AUXSEC_MEM  (unsigned int	,Length				) \
				AUXSEC_MEM  (unsigned short	,NumberOfRelocations) \
				AUXSEC_MEM  (unsigned short	,NumberOfLineNumbers) \
				AUXSEC_MEM  (int			,CheckSum			) \
				AUXSEC_MEM  (unsigned short	,Number				) \
				AUXSEC_MEM  (unsigned char	,Selection			) \

			#define AUXSEC_MEM(type, name) type name;
			#include "CoffHelperPackStart.h"
			typedef struct STRUCT_PACKING_ATTRIBUTE {
				AUXSEC_MEMBERS
				unsigned char Unused[3];
			} SectionAuxSymbolStruct;
			#include "CoffHelperPackStop.h"

			#undef AUXSEC_MEM
			#define AUXSEC_MEM(type, name) \
				public: \
					type get##name() const { return (reinterpret_cast<const SectionAuxSymbolStruct*>(reinterpret_cast<const char*>(data_)))->name; } \
					void set##name(type val) { (const_cast<SectionAuxSymbolStruct*>(reinterpret_cast<const SectionAuxSymbolStruct*>(reinterpret_cast<const char*>(data_))))->name = val; } \

			AUXSEC_MEMBERS

		};
	}
}

#endif

