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

#ifndef __BINARYGATHERER_H__
#define __BINARYGATHERER_H__

#include <CoffArchive.h>
#include <CoffFileHeader.h>
#include <CoffImport.h>
#include <CoffSymbol.h>

#include <map>
#include <vector>
#include <string>

namespace parity
{
	namespace tasks
	{
		class BinaryGatherer {
		public:
			BinaryGatherer();
			void doWork();

			//
			// Exported Symbols are those for which we need to generate
			// a line in the .def file.
			//
			const binary::Symbol::SymbolVector& getExportedSymbols() const { return exports_; }

			//
			// Static imports are symbols that are imported, but they are available as local
			// symbol instead of imports from a DLL.
			//
			const binary::Symbol::SymbolVector& getStaticImports() const { return staticImports_; }

			//
			// Local Symbols explain themselves :)
			//
			const binary::Symbol::SymbolVector& getLocalSymbols() const { return localSymbols_; }

			//
			// Loaded import are the symbols that will be handled by the loader generator.
			//
			typedef std::pair<utils::Path, binary::Import> PathImportPair;
			typedef std::map<PathImportPair, bool> ImportHybridityMap;
			const ImportHybridityMap& getLoadedImports() const { return loadedImports_; }

		private:
			void processArchive(binary::Archive& file, const utils::Path& path);
			void processHeader(binary::FileHeader& hdr, const utils::Path& path);
			void processImport(binary::Import& import, const utils::Path& path);

			void resolveSymbols();
			bool isSystemToSkip(std::string const& str) const;

			typedef std::pair<binary::Symbol, utils::Path> SymbolUsagePair;
			typedef std::pair<binary::Import, utils::Path> ImportUsagePair;
			typedef std::pair<utils::Path, bool> LibraryPathHybridityPair;
			
			typedef std::map<std::string, SymbolUsagePair> SymbolUsageMap;
			typedef std::map<std::string, ImportUsagePair> ImportUsageMap;
			typedef std::map<utils::Path, bool> PathUsageMap;
			typedef std::map<std::string, LibraryPathHybridityPair> LibraryHybridityMap;
			typedef std::map<std::string, bool> StringBoolMap;

			SymbolUsageMap unresolved_;
			SymbolUsageMap local_;
			ImportUsageMap imports_;

			binary::Symbol::SymbolVector exports_;
			binary::Symbol::SymbolVector maybeExports_;
			binary::Symbol::SymbolVector staticImports_;
			binary::Symbol::SymbolVector localSymbols_;
			ImportHybridityMap loadedImports_;

			StringBoolMap directivesDone_;
			PathUsageMap implicits_;
			PathUsageMap processed_;
			PathUsageMap nodefaultlibs_;

			LibraryHybridityMap hybrids_;
		};
	}
}

#endif

