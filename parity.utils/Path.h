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

#ifndef __PATH_H__
#define __PATH_H__

#include <string>
#include <sys/stat.h>

#ifdef _WIN32
#  define stat _stat
#endif

namespace parity
{
	namespace utils
	{
		class Path
		{
		public:
			Path();
			Path(const std::string& source);

			static Path getTemporary(std::string templ);
			static Path getTemporaryDirectory();

			const std::string& get() const;
			void set(const std::string& value);

			void toNative();
			void toForeign();

			bool isNative() const;
			bool isForeign() const;

			bool isFile() const;
			bool isDirectory() const;

			bool exists() const;
			bool remove() const;

			bool move(Path dest);
			bool moveCopy(Path dest);

			void mode(int mode) const;

			std::string file() const;
			std::string base() const;

			void append(const std::string& component);
			void appendDirect(std::string component);

			bool convertToWindows() { return convertGeneric(true); }
			bool convertToUnix() { return convertGeneric(false); }

			bool operator<(const Path& rhs) const { return path_ < rhs.path_; }
			bool operator==(const Path& rhs) const { return path_ == rhs.path_; }
			bool operator==(const char* rhs) const { return path_ == rhs; }
			bool operator!=(const Path& rhs) const { return path_ != rhs.path_; }
			bool operator!=(const char* rhs) const { return path_ != rhs; }

			Path& operator+=(const Path& rhs) { this->append(rhs.path_); return *this; }
			Path operator+(const Path& rhs) { Path pth(path_); pth += rhs; return pth; }

			Path& operator+=(const std::string& rhs) { this->append(rhs); return *this; }
			Path operator+(const std::string& rhs) { Path pth(path_); pth += rhs; return pth; }

		private:
			std::string path_;
			struct stat stat_;
			bool stated_;
			static bool convert_;

			bool convertGeneric(bool bWindows);
			bool naiveInterixConverter(std::string& target, std::string& source, bool bWindows);

			void expand();

			bool isWindows() const;
			bool isUnix() const;

			bool isBackendWindows() const;
			bool isBackendUnix() const;

			char getSeperator() const;
		};

	}
}

#endif

