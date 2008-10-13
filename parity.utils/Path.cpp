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

#include "Path.h"
#include "Log.h"
#include "Exception.h"
#include "Context.h"

#include <sys/timeb.h>
#include <cstdlib>
#include <sstream>

#ifdef _WIN32
#  include <windows.h>
#  include <io.h>
#  include <process.h>
#  include <direct.h>
#endif

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#ifdef __INTERIX
#  include <interix/interix.h>
#endif

#ifdef __CYGWIN__
#  include <sys/cygwin.h>
#endif

#ifndef PATH_MAX
#  ifdef MAX_PATH
#    define PATH_MAX MAX_PATH
#  else
#    define PATH_MAX 260
#  endif
#endif

namespace parity
{
	namespace utils
	{

		Path::Path()
			: path_(), stat_(), stated_(false)
		{
			// hopefully no more required...
			//::memset(&stat_, 0, sizeof(stat_));
		}

		Path::Path(const std::string &source)
			: path_(source), stat_(), stated_(false)
		{
			// hopefully no more required...
			//::memset(&stat_, 0, sizeof(stat_));

			if(!path_.empty())
				expand();

			resolveLink();
		}

		#ifdef _WIN32
		#  define strdup _strdup
		#  define ftime _ftime
		#  define timeb _timeb
		#  define getpid _getpid
		#endif

		Path Path::getTemporary(std::string templ)
		{
			std::string::size_type pos = templ.find("XXXXXX");

			if(pos == std::string::npos)
				throw utils::Exception("cannot find template specifier!");

			struct timeb cur;
			ftime(&cur);

			std::ostringstream oss;
			oss << static_cast<unsigned short>(cur.time) << "." << cur.millitm << "." << getpid();
			templ.replace(pos, 6, oss.str());

			return Path(templ);
		}

		#ifdef _WIN32
		#  undef strdup
		#  undef ftime
		#  undef timeb
		#  undef getpid
		#endif

		/*
		 * not required ATM. keep for the future.
		 *
		Path Path::getTemporaryDirectory()
		{
#if defined(_WIN32)
			char pth[PATH_MAX];
			GetTempPath(PATH_MAX, pth);

			return Path(pth);
#else
			const char* env = getenv("TMPDIR");

			if(env)
				return Path(env);
			else
				return Path("/tmp");
#endif
		}
		 */

		const std::string& Path::get() const
		{
			return path_;
		}

		void Path::set(const std::string& value)
		{
			path_ = value;
			stated_ = false;

			if(!path_.empty())
				expand();
		}

		void Path::toNative()
		{
			if(isNative())
				return;

			#ifdef __INTERIX
				/* native:  unix style paths,
				 * foreign: depends on backend */

				char buf[PATH_MAX];

				if(winpath2unix(path_.c_str(), 0, buf, PATH_MAX) != -1)
					path_ = buf;
			#elif defined(__CYGWIN__)
				/* same as interix */
				char buf[PATH_MAX];

				cygwin_conv_to_full_win32_path(path_.c_str(), buf);
				path_ = buf;
			#else
				/* native:  windows style paths,
				 * foreign: depends on backend */
				convertToWindows();
			#endif

			expand();
		}

		void Path::toForeign()
		{
			if(isForeign())
				return;

			#ifdef __INTERIX
				/* native:  unix style paths,
				 * foreign: depends on backend */

				char buf[PATH_MAX];

				if(isBackendWindows()) {
					if(unixpath2win(path_.c_str(), 0, buf, PATH_MAX) != -1)
						path_ = buf;
				} else {
					if(winpath2unix(path_.c_str(), 0, buf, PATH_MAX) != -1)
						path_ = buf;
				}
			#elif defined(__CYGWIN__)
				/* same as interix */
				char buf[PATH_MAX];

				if(isBackendWindows()) {
					cygwin_conv_to_full_win32_path(path_.c_str(), buf);
				} else {
					cygwin_conv_to_full_posix_path(path_.c_str(), buf);
				}
				path_ = buf;
			#else
				/* native:  windows style paths,
				 * foreign: depends on backend */
				if(isBackendWindows())
					convertToWindows();
				else
					convertToUnix();
			#endif
		}

		bool Path::isNative() const 
		{
			#ifdef __INTERIX
				/* native:  unix style paths,
				 * foreign: windows style paths */
				return isUnix();
			#elif defined(__CYGWIN__)
				/* Theiretically cygwin understands both, so we could
				 * use isWindows() here to save conversion time. */
				return isUnix();
			#else
				/* native:  windows style paths,
				 * foreign: windows style paths */
				return isWindows();
			#endif
		}

		bool Path::isForeign() const
		{
			if(isBackendWindows())
				return isWindows();
			else
				return isUnix();
		}

		bool Path::isFile() const
		{
			/* What is considered the Path is not changed by this
			 * method, so it's const. Still it may need to gather
			 * information by calling stat, and buffering this in
			 * class members, so const_cast's are harmless! */

			if(path_.empty())
				return false;

			if(!stated_)
			{
				if(stat(path_.c_str(), const_cast<struct stat*>(&stat_)) == -1)
					return false;

				*(const_cast<bool*>(&stated_)) = true;
			}

#ifdef _WIN32
			return ((stat_.st_mode & S_IFREG) != 0);
#else
			return (S_ISREG(stat_.st_mode));
#endif
		}

		bool Path::isDirectory() const
		{
			/* What is considered the Path is not changed by this
			 * method, so it's const. Still it may need to gather
			 * information by calling stat, and buffering this in
			 * class members, so const_cast's are harmless! */

			if(path_.empty())
				return false;

			if(!stated_)
			{
				if(stat(path_.c_str(), const_cast<struct stat*>(&stat_)) == -1)
					return false;

				*(const_cast<bool*>(&stated_)) = true;
			}

#ifdef _WIN32
			return ((stat_.st_mode & S_IFDIR) != 0);
#else
			return (S_ISDIR(stat_.st_mode));
#endif
		}

		bool Path::exists() const
		{
			if(path_.empty())
				return false;

			#ifdef _WIN32
			#  define access _access
			#endif

			return (access(path_.c_str(), 0) == 0);

			#ifdef access
			#  undef access
			#endif
		}

		bool Path::remove() const
		{
			if(path_.empty())
				return false;

			if(isFile()) {
				return removeFile(path_);
			} else {
				//
				// recursive delete...
				//
				return removeRecursive(path_);
			}
		}

		bool Path::removeRecursive(std::string path) const
		{
			//
			// TODO: do a recursive delete, not only flat rmdir.
			//
#ifdef _WIN32
# define rmdir _rmdir
#endif

			if(rmdir(path.c_str()) != 0)
				return false;

#ifdef _WIN32
# undef rmdir
#endif
			return true;
		}

		bool Path::removeFile(std::string const& file) const
		{
			changeMode(file, 0777);

			*(const_cast<bool*>(&stated_)) = false;

			#ifdef _WIN32
			#  define unlink _unlink
			#endif

			if(unlink(file.c_str()) == 0)
				return true;

			#ifdef unlink
			#  undef _unlink
			#endif

			return false;
		}

		bool Path::move(Path dest)
		{
			if(!exists())
				return false;

			toNative();
			dest.toNative();

			mode(0777);

			stated_ = false;

			if(rename(path_.c_str(), dest.path_.c_str()) != 0) {
				utils::Log::verbose("Perfomance warning: doing slow full copy instead of move, crossing device boundary?\n"
					" * source: %s\n * dest  : %s\n", path_.c_str(), dest.path_.c_str());

				return moveCopy(dest);
			}

			path_ = dest.path_;
			return true;
		}

		bool Path::moveCopy(Path dest)
		{
			if(!exists())
				return false;

			toNative();
			dest.toNative();

			mode(0777);

			stated_ = false;

			//
			// do a full copy, and remove the old file.
			//
			FILE* old_file = fopen(path_.c_str(), "rb");
			FILE* new_file = fopen(dest.path_.c_str(), "wb");

			if(!old_file)
				throw Exception("cannot open source file (%s) for move", path_.c_str());

			if(!new_file)
				throw Exception("cannot open target file (%s) for move", dest.path_.c_str());

			#define READ_BUFFER_SIZE 4096
			char buffer[READ_BUFFER_SIZE];

			while(true)
			{
				size_t count = fread(buffer, 1, READ_BUFFER_SIZE, old_file);

				if(count > 0) {
					fwrite(buffer, 1, READ_BUFFER_SIZE, new_file);	
				}

				if(feof(old_file))
					break;

				if(ferror(old_file))
					throw Exception("error reading %s: %s!", path_.c_str(), strerror(ferror(old_file)));
				if(ferror(new_file))
					throw Exception("error writing %s: %s!", dest.path_.c_str(), strerror(ferror(new_file)));
			}

			fclose(old_file);
			fclose(new_file);

			remove();

			path_ = dest.path_;
			return true;
		}

		void Path::mode(int mode) const
		{
			changeMode(path_, mode);
		}

		void Path::changeMode(std::string const& file, int mode) const
		{
			#ifdef _WIN32
			#  define chmod _chmod
			#  define umask _umask
			#endif

			int final_mode = mode;

			#ifdef _WIN32
			//
			// Need to somehow calculate a windows mode out of
			// the given UNIX style mode (e.g. 0777)
			//
			final_mode = 0;

			if(mode >= 0200)
				final_mode |= _S_IWRITE;
			if(mode >= 0400)
				final_mode |= _S_IREAD;

			#endif

			//
			// Need to take umask into account on all systems
			//
			int mask = umask(0);
			umask(mask);

			final_mode = final_mode & ~mask;

			chmod(file.c_str(), final_mode);

			#ifdef chmod
			#  undef chmod
			#  undef umask
			#endif
		}

		std::string Path::file() const
		{
			if(path_.empty())
				return path_;

			return path_.substr(path_.rfind(getSeperator()) + 1);
		}

		std::string Path::base() const
		{
			if(path_.empty())
				return path_;

			return path_.substr(0, path_.rfind(getSeperator()));
		}

		void Path::append(const std::string& component)
		{
			//
			// An empty path expands to the current directory.
			// Since appending to an empty path results in a relative
			// path from the current directory, we expand here.
			// Expanding needs to be done before calling getSeperator.
			//
			if(path_.empty())
				expand();

			char ch = getSeperator();
			char str[2] = { ch, '\0' };

			if(path_[path_.length()] != ch)
				path_.append(str);

			appendDirect(component);
		}

		void Path::appendDirect(std::string component)
		{
			#ifdef _WIN32
			//
			// case insensitive, so convert to all lower case to prevent case clashes.
			// TODO: FIXXME: this shouldn't be done. better is to do a stricmp in 
			// operator== & co if on _WIN32.
			//
			for(size_t i = 0; i < component.length(); ++i)
				if(isalpha(component[i]) && isupper(component[i]))
					component[i] = static_cast<char>(::tolower(component[i]));
			#endif

			path_.append(component);
			stated_ = false;

			resolveLink();
		}

		bool Path::createPath() const {
			if(exists()) {
				return true;
			}

			//
			// break appart into components.
			//
			char* tmp = new char[path_.length() + 1];
			strcpy(tmp, path_.c_str());
			char* walk = tmp;
			std::vector<std::string> to_delete;

			while(walk) {
				char backup = '\0';
				if(*walk == '\\' || *walk == '/' || *walk == '\0') {
					backup = *walk;

					*walk = '\0';

					#ifdef _WIN32
					# define access _access
					# define rmdir _rmdir
					#endif

					if(access(tmp, 0) != 0) {
						#ifdef _WIN32
						if(_mkdir(tmp) != 0) {
						#else
						if(mkdir(tmp, 0777) != 0) {
						#endif
							//
							// clean any created directory.
							//
							for(std::vector<std::string>::reverse_iterator it = to_delete.rbegin(); it != to_delete.rend(); ++it) {
								rmdir(it->c_str());
							}
							return false;
						} else {
							to_delete.push_back(tmp);
						}
					}

					*walk = backup;

					if(backup == '\0')
						break;

					#ifdef _WIN32
					# undef access
					# undef rmdir
					#endif

				}
				++walk;
			}

			return true;
		}

		void Path::expand()
		{
			char path[PATH_MAX];

			if(!isNative())
				return;

			//
			// WARNING: This only expands *existing* paths. this has the
			// reason, that the realpath function has a little strange
			// behaviour:
			//  if you have a path like /this/is/my/path which exists only
			//  up to /this/is (my and my/path don't exist!) and you give
			//  it to realpath() you'll get /this/is/my. This for sure is
			//  not what we want!!
			//

			#ifndef _WIN32
				//
				// maybe case sensitive, so don't convert case!
				//
				if(exists())
					::realpath(path_.c_str(), path);
				else
					return;
			#else
				//
				// expand first, since fullpath changes case back to real case.
				//
				if(exists())
					_fullpath(path, path_.c_str(), PATH_MAX);
				else
					return;

				//
				// case insensitive, so convert to all lower case to prevent case clashes.
				// TODO: FIXME: this shouldn't be done. better is to do a stricmp in 
				// operator== & co if on _WIN32.
				//
				for(size_t i = 0; i < path_.length(); ++i)
					if(isalpha(path_[i]) && isupper(path_[i]))
						path_[i] = static_cast<char>(::tolower(path_[i]));
			#endif

			path_ = path;
			resolveLink();
		}

		//
		// WARNING: the following only reads links of the last
		// path component, not of pieces in the middle! so the whole
		// path set at call time must exist as file on disk!
		//
		void Path::resolveLink()
		{
			#ifdef _WIN32
				#define LINK_MAX_PATH 260
				//
				// Take a quick look at the file and check wether it can be a path.
				// If stat'ing failes, we simply return and do nothing, since the file
				// doesn't exist...
				//
				if(!stated_)
				{
					if(stat(path_.c_str(), &stat_) == -1)
						return;

					stated_ = true;
				}

				//
				// We make a simple assumption here: if the file is bigger than
				// 8 bytes plus 260 (MAX_PATH) * sizeof(wchar_t) then it can't 
				// be a valid link for us...
				//
				if(stat_.st_size > (8 + (LINK_MAX_PATH * sizeof(wchar_t))))
					return;

				if(isDirectory())
					return;

				//
				// Read the contents of the link.
				//
				MappedFile mapped(*this, ModeRead);

				const char signature[] = { 0x49, 0x6E, 0x74, 0x78, 0x4C, 0x4E, 0x4B, 0x01 };
				if(memcmp(signature, mapped.getBase(), sizeof(signature)) != 0) {
					return;
				}

				utils::Log::verbose("found softlink signature in %s\n", path_.c_str());

				wchar_t* link = (wchar_t*)((char*)mapped.getBase() + sizeof(signature));
				
				if(wcslen(link) > LINK_MAX_PATH) {
					return;
				}

				char buffer[LINK_MAX_PATH];
				if(wcstombs(buffer, link, LINK_MAX_PATH) == (size_t)-1) {
					return;
				}

				mapped.close();

				utils::Log::verbose("resolved link to %s\n", buffer);

				//
				// absolute link contents replaces the whole path. Relative
				// link contents replaces only the last part of the current value.
				//
				if(buffer[0] == '/')
					path_ = buffer;
				else
					path_ = base() + "/" + buffer;

				stated_ = false;
				expand();
			#endif
		}

		typedef char* (__stdcall * psx3_unixpath2win_func_t)(const char*, int, char*, size_t);
		typedef int   (* psx5_unixpath2win_func_t)(const char*, int, char*, size_t);
		typedef void  (*cygwin_init_func_t)();
		typedef void  (*cygwin_conv_func_t)(const char*, char*);

		bool Path::convert_ = true;

		#ifndef _WIN32
		# define PTH_MAYBE_UNUSED(x)
		#else
		# define PTH_MAYBE_UNUSED(x) x
		#endif

		bool Path::convertGeneric(bool PTH_MAYBE_UNUSED(bWindows))
		{
			if(path_.empty())
				return true;

			#ifdef _WIN32
				static HMODULE hPsxDll = NULL;

				if(!convert_)
					return true;

				if(bWindows && path_[0] != '/')
					return true;
				if(!bWindows && path_[1] != ':')
					return true;

				std::string tmp;
				if(naiveInterixConverter(tmp, path_, bWindows)) {
					path_ = tmp;

					//
					// the naive converter may result in a relative path
					// if converting from foreign to native.
					//
					expand();
					return true;
				}

				try {
					bool bTryPsx5 = false;

					if(!hPsxDll) {
						Log::verbose("Loading psxwcl32.dll...\n");
						hPsxDll = LoadLibrary("psxwcl32.dll");
					}

					if(hPsxDll)
					{
						psx3_unixpath2win_func_t hPsx3Func = (bWindows ?
							(psx3_unixpath2win_func_t)GetProcAddress(hPsxDll, "WCLunixpath2win") :
							(psx3_unixpath2win_func_t)GetProcAddress(hPsxDll, "WCLwinpath2unix"));

						if(hPsx3Func)
						{
							char buffer[MAX_PATH];

							if(hPsx3Func(path_.c_str(), 0, buffer, MAX_PATH))
							{
								path_ = buffer;
								return true;
							} else {
								utils::Log::error("failed to convert %s to %s style path!\n", path_.c_str(), bWindows?"Windows":"UNIX");
							}

							/* otherwise fail without trying PSX5 */
						} else {
							bTryPsx5 = true;
						}
					} else {
						bTryPsx5 = true;
					}

					if(bTryPsx5)
					{
						if(!hPsxDll) {
							Log::verbose("Loading psxdll.dll...\n");
							hPsxDll = LoadLibrary("psxdll.dll");
						}

						if(hPsxDll)
						{
							psx5_unixpath2win_func_t hPsx5Func = (bWindows ?
								(psx5_unixpath2win_func_t)GetProcAddress(hPsxDll, "unixpath2win") :
								(psx5_unixpath2win_func_t)GetProcAddress(hPsxDll, "winpath2unix"));

							if(hPsx5Func)
							{
								char buffer[MAX_PATH];

								if(hPsx5Func(path_.c_str(), 0, buffer, MAX_PATH) == 0)
								{
									path_ = buffer;
									return true;
								}
							}
						} else {
							/* don't try again if both failed */
							convert_ = false;
						}
					}

					/* but wait.... what about cygwin? */
					if(!convert_)
					{
						static HMODULE hCygLib = NULL;
						static int inited = 0;
		
						if(!hCygLib)
							hCygLib = LoadLibrary("cygwin1.dll");

						if(hCygLib)
						{
							static cygwin_init_func_t init = NULL; 
							static cygwin_conv_func_t conv = NULL; 

							if(!init)
								init = (cygwin_init_func_t)GetProcAddress(hCygLib, "cygwin_dll_init");
							if(!conv)
								conv = (cygwin_conv_func_t)GetProcAddress(hCygLib, "cygwin_conv_to_full_win32_path");

							if(init && conv)
							{
								//
								// allocate a ring slot, and then convert using cygwin dll.
								//
								char buffer[_MAX_PATH];

								if(!inited++)
									init();

								conv(path_.c_str(), buffer);

								convert_ = true;
								path_ = buffer;
							}
						}
					}
				} catch(...) {
					convert_ = false;
					return false;
				}

				return false;
			#else
			return true;
			#endif
		}

		bool Path::naiveInterixConverter(std::string& target, std::string& source, bool bWindows)
		{
			//
			// try a very naive conversion from or to windows, which must only
			// succeed in very simple cases.
			//

			if((bWindows && source[0] != '/') || (!bWindows && source[1] != ':'))
			{
				target = source;
				return true;
			}

			if(bWindows)
			{
				if(source.compare(0, 8, "/dev/fs/") == 0)
				{
					target = source.substr(8);
					target.insert(1, ":");

					while(target.find('/') != std::string::npos)
						target.replace(target.find('/'), 1, 1, '\\');

					return true;
				} else {
					const char* root = getenv("INTERIX_ROOT_WIN");

					if(!root)
						return false;

					target = root;
					
					if(target[target.length() - 1] == '/' || target[target.length() - 1] == '\\')
						source = source.substr(1);

					while(source.find('/') != std::string::npos)
						source.replace(source.find('/'), 1, 1, '\\');

					target.append(source);
					return true;
				}
			} else {
				target = "/dev/fs/";
				
				source.replace(1, 1, "");
				while(source.find('\\') != std::string::npos)
					source.replace(source.find('\\'), 1, 1, '/');

				target.append(source);
				return true;
			}
		}

		bool Path::isWindows() const
		{
			if(path_.empty())
				return true;

			//
			// relative paths is windows.
			//
			if(path_[0] == '.')
				return true;

			if(::strchr(path_.c_str(), '\\'))
				return true;

			if(!::strchr(path_.c_str(), '/'))
				return true;

			return false;
		}

		bool Path::isUnix() const
		{
			if(path_.empty())
				return true;

			if(path_[0] == '.')
				return true;

			if(::strchr(path_.c_str(), '\\'))
				return false;

			return true;
		}

		char Path::getSeperator() const
		{
			//
			// Should never be called on an empty path!
			//
			if(path_.empty())
				throw Exception("Cannot determine seperator of empty path!");

			if(isWindows())
				return '\\';
			else
				return '/';
		}

		bool Path::isBackendWindows() const
		{
			Context& ctx = Context::getContext();
			const ToolchainType& type = ctx.getBackendType();

			switch(type)
			{
			case ToolchainInterixGNU:
				/* TODO: what about mingw, cygwin and others that understand windows paths? */
				return false;
			case ToolchainInterixMS:
			case ToolchainMicrosoft:
				return true;
			default:
				throw Exception("invalid backend type set, cannot determine path style!");
			}
		}

		bool Path::isBackendUnix() const
		{
			return !isBackendWindows();
		}

	}
}

