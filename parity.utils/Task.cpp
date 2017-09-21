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

#include "Task.h"
#include "Exception.h"
#include "Color.h"
#include "Context.h"
#include "Log.h"
#include "Timing.h"
#include "Environment.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>

#ifdef _WIN32
#  include <windows.h>
#  include <io.h>
#  include <process.h>
#else
#  include <sys/wait.h>
#endif

#define READ_FD  0
#define WRITE_FD 1

//
// when building with parity itself, we need to be carfull, since
// all MS header files are included with _POSIX_ defined, and thus
// some things are different (fex. we need to use "old" POSIX names).
//
#ifdef __PARITY__
# define _fileno fileno
#endif

namespace parity
{
	namespace utils
	{
		Task::ProcessList Task::running_;
		unsigned int Task::taskCounter_;

		static void handleBrokenPipe(int sig) { exit(sig); }

		bool Task::prepareEnvironment()
		{
			static bool prepared = false;

			if(prepared)
				return true;

			prepared = true;

			Environment pth("PATH");
			PathVector& paths = Context::getContext().getAdditionalExecPaths();

			for(PathVector::iterator it = paths.begin(); it != paths.end(); ++it)
			{
				// this converts all paths to native ones as a side-effect!
				it->toNative();

				if(!pth.extend(*it))
					return false;

				Log::verbose("extended PATH with: %s\n", it->get().c_str());
			}

			Environment tmp("TMP");
			Path ptmp = tmp.getPath();
			ptmp.toForeign();
			tmp.set(ptmp.get());

			//
			// when exiting, terminate all running processes...
			//
			atexit(terminateRunningProcesses);

#ifdef SIGPIPE
			signal(SIGPIPE, handleBrokenPipe);
#endif

			return true;
		}

		bool Task::execute(const Path& executable, const ArgumentVector& arguments)
		{
			Color col(Context::getContext().getColorMode());
			Path pth(executable);
			pth.toNative();

			if(!prepareEnvironment())
				throw Exception("cannot prepare the environment for execution...");

			std::ostringstream oss;
			oss << "[" << ++taskCounter_ << "] " << executable.file();

			Timing::instance().start(oss.str());

			Log::verbose("executing: %s (%s) with arguments:\n", col.red(executable.get()).c_str(), (pth == executable ? "..." : col.red(pth.get()).c_str()));

			for(ArgumentVector::const_iterator it = arguments.begin(); it != arguments.end(); ++it)
				Log::verbose(" * %s\n", it->c_str());

			#ifdef _WIN32
				int stdOutFd;
				int stdErrFd;
				int stdOutPipe[2];
				int stdErrPipe[2];
				DWORD exitCode = STILL_ACTIVE;
				HANDLE process = 0;

				//
				// Redirect any outputs to pipes
				//

				if(::_pipe(stdOutPipe, 512, _O_NOINHERIT) == -1)
					throw Exception("cannot open pipes for stdout redirection: %s", ::strerror(errno));
				if(::_pipe(stdErrPipe, 512, _O_NOINHERIT) == -1)
					throw Exception("cannot open pipes for stderr redirection: %s", ::strerror(errno));

				stdOutFd = ::_dup(::_fileno(stdout));
				stdErrFd = ::_dup(::_fileno(stderr));

				if(stdOutFd == -1 || stdErrFd == -1)
					throw Exception("cannot dup original file descriptors: %s", ::strerror(errno));

				if(::_dup2(stdOutPipe[WRITE_FD], ::_fileno(stdout)) == -1)
					throw Exception("cannot redirect stdout handle to pipe: %s", ::strerror(errno));
				if(::_dup2(stdErrPipe[WRITE_FD], ::_fileno(stderr)) == -1)
					throw Exception("cannot redirect stderr handle to pipe: %s", ::strerror(errno));

				::_close(stdOutPipe[WRITE_FD]);
				::_close(stdErrPipe[WRITE_FD]);

				//
				// Spawn the process here
				//
				const char ** translated = new const char*[arguments.size() + 2];
				std::string cmdname = pth.file();
				translated[0] = cmdname.c_str();
				ArgumentVector::size_type i;
				for(i = 0; i < arguments.size(); ++i)
					translated[i + 1] = arguments[i].data();

				translated[i + 1] = 0;

				process = (HANDLE)::_spawnv(_P_NOWAIT, pth.get().c_str(), translated);
				running_.push_back(process);

				//
				// Re-redirect output stream back to where they came from
				//
				
				if(::_dup2(stdOutFd, ::_fileno(stdout)) == -1)
					throw Exception("cannot restore stdout handle: %s", ::strerror(errno));
				if(::_dup2(stdErrFd, ::_fileno(stderr)) == -1)
					throw Exception("cannot restore stderr handle: %s", ::strerror(errno));

				::_close(stdOutFd);
				::_close(stdErrFd);

				//
				// Wait for process end, and in the meanwhile read output
				//
				if(!process || process == (HANDLE)-1)
					throw Exception("creation of process failed: %s", ::strerror(errno));

				int readOut = 1;
				int readErr = 1;
				char buffer[1024];
				bool bFinishedLast = false;

				//
				// TODO: how can i detect a broken pipe on win32??
				//

				while(exitCode == STILL_ACTIVE || bFinishedLast || (readOut > 0 || readErr > 0))
				{
					bFinishedLast = false;

					//
					// maybe block a few milliseconds?
					//
					Sleep(2);

					if(_eof(stdOutPipe[READ_FD]) == 0)
					{
						readOut = _read(stdOutPipe[READ_FD], buffer, 1024);
						if(readOut > 0)
							processBlock(buffer, readOut, true);
					} else {
						readOut = 0;
					}

					if(readOut == -1)
						throw Exception("cannot read from stdout pipe: %s", ::strerror(errno));

					if(_eof(stdErrPipe[READ_FD]) == 0)
					{
						readErr = _read(stdErrPipe[READ_FD], buffer, 1024);

						if(readErr > 0)
							processBlock(buffer, readErr, false);
					} else {
						readErr = 0;
					}

					if(readErr == -1)
						throw Exception("cannot read from stderr pipe: %s", ::strerror(errno));

					if(exitCode == STILL_ACTIVE)
					{
						if(!GetExitCodeProcess(process, &exitCode))
							throw Exception("cannot retreive exit status of child process: %s", ::strerror(errno));

						if(exitCode != STILL_ACTIVE)
							bFinishedLast = true;
					}
				}

				delete[] translated;

				out_->flush();

				::_close(stdOutPipe[READ_FD]);
				::_close(stdErrPipe[READ_FD]);

				Timing::instance().stop(oss.str());

				running_.remove(process);

				utils::Log::verbose(" * result: %d\n", exitCode);

				if(exitCode == 0)
					return true;

			#else
				int stdOutPipe[2];
				int stdErrPipe[2];

				if(pipe(stdOutPipe) == -1)
					throw Exception("cannot open pipes for stdout redirection: %s", ::strerror(errno));
				if(pipe(stdErrPipe) == -1)
					throw Exception("cannot open pipes for stderr redirection: %s", ::strerror(errno));

				ArgumentVector unescapedargs(arguments);
				for(ArgumentVector::iterator arg = unescapedargs.begin()
				  ; arg != unescapedargs.end()
				  ; ++arg
				) {
					bool unescaped = false;
					size_t escap = arg->find('\\');
					while (escap != arg->npos && escap < arg->length()) {
						switch(arg->at(escap+1)) {
						case '\\':
						case '"':
							unescaped = true;
							arg->erase(escap, 1);
							break;
						case '\n':
							unescaped = true;
							arg->erase(escap, 2);
							break;
						default:
							++escap;
							break;
						}
						escap = arg->find('\\', escap);
					}
					if (unescaped) {
						Log::verbose(" * with unescaped arg >%s<\n", arg->c_str());
					}
				}

				pid_t child = fork();

				switch(child)
				{
				case -1:
					throw Exception("cannot fork child process: %s", strerror(errno));
				case 0:
					{
						//
						// child process
						//
						close(stdOutPipe[READ_FD]);
						close(stdErrPipe[READ_FD]);

						if(dup2(stdOutPipe[WRITE_FD], fileno(stdout)) == -1)
							throw Exception("cannot redirect stdout: %s", strerror(errno));
						if(dup2(stdErrPipe[WRITE_FD], fileno(stderr)) == -1)
							throw Exception("cannot redirect stderr: %s", strerror(errno));

						close(stdOutPipe[WRITE_FD]);
						close(stdErrPipe[WRITE_FD]);
						
						// bug on interix 6.0 makes waitpid return 0 (nothing happened)
						// until there is a return on stdin of child (or there is no stdin)						
						close(fileno(stdin));

						//
						// execute child process
						//
						const char ** translated = new const char*[unescapedargs.size() + 2];
						std::string cmdname = pth.file();
						translated[0] = cmdname.c_str();
						ArgumentVector::size_type i;
						for(i = 0; i < unescapedargs.size(); ++i)
							translated[i + 1] = unescapedargs[i].data();

						translated[i + 1] = 0;

                        int t = 0;
                        int sleep = 100000;
                        while(t++ < 100) {
                            execv(pth.get().c_str(), const_cast<char* const*>(translated));
                            std::cerr << "warn: cannot execute " << pth.get().c_str() << " (retrying): " << strerror(errno) << std::endl;
                            usleep(sleep);
                            sleep += 100000;
                        }

                        std::cerr << "giving up..." << std::endl;

						exit(1);
					}
				default:
					//
					// parent process
					//
					running_.push_back(child);

					close(stdOutPipe[WRITE_FD]);
					close(stdOutPipe[WRITE_FD]);

					fcntl(stdOutPipe[READ_FD], F_SETFL, fcntl(stdOutPipe[READ_FD], F_GETFL, 0) | O_NONBLOCK);
					fcntl(stdErrPipe[READ_FD], F_SETFL, fcntl(stdErrPipe[READ_FD], F_GETFL, 0) | O_NONBLOCK);

					utils::Log::verbose(" * child %d started...\n", child);

					pid_t wpid = 0;

					int readOut = 1;
					int readErr = 1;
					char buffer[1024];

					bool bRunning = true;
					bool bFinishedLast = false;

					int exitStatus = -1;

					do
					{
						//
						// maybe block a few milliseconds?
						//
						usleep(2000);

						bFinishedLast = false;

						if(bRunning)
						{
							wpid = waitpid(child, &exitStatus, WUNTRACED | WNOHANG);

							if(wpid == -1)
								throw Exception("error waiting on child process: %s", strerror(errno));

							if(wpid == child)
							{
								bRunning = false;
								bFinishedLast = true;

								if(!WIFEXITED(exitStatus))
									exitStatus = 1;
								else
									exitStatus = WEXITSTATUS(exitStatus);

								utils::Log::verbose(" * child %d finished: %d\n", wpid, exitStatus);
							}
						}

						//
						// in any case read from pipes
						//
						readOut = read(stdOutPipe[READ_FD], buffer, 1024);
						
						if(readOut == -1 && errno != EAGAIN)
							throw Exception("cannot read from stdout pipe");
						
						if(readOut > 0)
							processBlock(buffer, readOut, true);
						
						readErr = read(stdErrPipe[READ_FD], buffer, 1024);

						if(readErr == -1 && errno != EAGAIN)
							throw Exception("cannot read from stderr pipe");

						if(readErr > 0)
							processBlock(buffer, readErr, false);

					} while(bRunning || bFinishedLast || (readOut != 0 && readErr != 0));

					out_->flush();

					close(stdOutPipe[READ_FD]);
					close(stdOutPipe[READ_FD]);
					
					Timing::instance().stop(oss.str());

					running_.remove(child);
					
					utils::Log::verbose(" * result: %d\n", exitStatus);
					
					if(exitStatus == 0)
						return true;

					break;
				}

			#endif

			return false;
		}

		void Task::terminateRunningProcesses() {
			if(running_.size() > 0) {
				utils::Log::warning("%ld child processes are still alive, terminating them.\n", running_.size());
			} else {
				return;
			}

			while(running_.size() > 0) {
				ProcessList::iterator process = running_.begin();

#ifdef _WIN32
				if(!TerminateProcess(*process, 99)) {
					utils::Log::warning("cannot terminate process %d. it may keep blocking file handles.\n", GetProcessId(*process));
				}
#else
				if(kill(*process, SIGTERM) != 0) {
					utils::Log::warning("cannot terminate process %d (%s). it may keep blocking file handles.\n", *process, strerror(errno));
				}
#endif

				//
				// after terminating the process, remove it from the list.
				//
				running_.erase(process);
			}
		}

		void Task::createCommandScript(ArgumentVector& vec)
		{
			utils::Path pth = utils::Path::getTemporary(".parity.commands.XXXXXX");

			if(pth.exists())
				if(!pth.remove())
					throw Exception("cannot delete temporary file %s", pth.get().c_str());

			pth.toNative();
			std::ofstream ofs(pth.get().c_str());

			if(!ofs.good())
				throw Exception("cannot open temporary file %s", pth.get().c_str());

			Log::verbose("routing argument vector to temporary command script %s.\n", pth.get().c_str());

			for(ArgumentVector::const_iterator it = vec.begin(); it != vec.end(); ++it)
			{
				if(it->find(' ') != std::string::npos)
					ofs << "\"" << *it << "\"" << std::endl;
				else
					ofs << *it << std::endl;
			}

			ofs.close();

			pth.toForeign();

			vec.clear();
			vec.push_back("@" + pth.get());

			//
			// this file is temporary, so mark it for removal at program end.
			//
			Context::getContext().getTemporaryFiles().push_back(pth);
		}

		void Task::processLine(std::string& line, bool stdOut)
		{
			std::string::size_type posR;

			while((posR = line.find('\r')) != std::string::npos)
				line.replace(posR, 1, "");

			filterLine(line);
			colorizeLine(line);

			if(!line.empty()) {
				if(stdOut)
					*out_ << line << std::endl;
				else
					*err_ << line << std::endl;
			}
		}
		
		void Task::processBlock(const char* input, size_t size, bool stdOut)
		{
			static std::string unfinished[2];

			if(!input)
			{
				//
				// This is the instruction to flush the last unfinished things
				// and treat them like a whole line, even though they had no \n
				//
				if(stdOut) {
					if(!unfinished[0].empty()) {
						processLine(unfinished[0], stdOut);
						unfinished[0].clear();
					}
				} else {
					if(!unfinished[1].empty()) {
						processLine(unfinished[1], stdOut);
						unfinished[1].clear();
					}
				}
			}

			std::string buffer = std::string(input, size);
			std::string::size_type posN;
			std::string::size_type posStart = 0;

			while((posN = buffer.find('\n', posStart)) != std::string::npos)
			{
				//
				// posN is garantueed to be euqal or greater than posStart
				// or something went terribly wrong. So don't check ;o)
				//
				std::string line = buffer.substr(posStart, posN - posStart);

				if(stdOut) {
					if(!unfinished[0].empty()) {
						line = unfinished[0] + line;
						unfinished[0].clear();
					}
				} else {
					if(!unfinished[1].empty()) {
						line = unfinished[1] + line;
						unfinished[1].clear();
					}
				}

				processLine(line, stdOut);
				posStart = posN + 1;
			}

			if(posStart < buffer.length())
			{
				//
				// unfinished line! We need to remember this, and prepend
				// this to the next line we process.
				//
				if(stdOut)
					unfinished[0] = buffer.substr(posStart);
				else
					unfinished[1] = buffer.substr(posStart);
			}
		}

		void Task::colorizeLine(std::string& input)
		{
			utils::Color col(Context::getContext().getColorMode());

			if(input.empty() || !col.isColored())
				return;

			bool bIsError = false;
			std::ostringstream oss;
			std::string::size_type posCol = input.find(" warning ");

			if(posCol == std::string::npos)
			{
				posCol = input.find(" error ");
				bIsError = true;
			}

			if(posCol != std::string::npos)
			{
				posCol = input.rfind(':', posCol);
			}

			if(posCol != std::string::npos)
			{
				++posCol; // skip ":"
				oss << col.cyan(input.substr(0, posCol));

				std::string::size_type posEnd = input.find(':', posCol);
				if(posEnd != std::string::npos) {
					if(bIsError)
						oss << col.red(input.substr(posCol, posEnd - posCol));
					else
						oss << col.yellow(input.substr(posCol, posEnd - posCol));

					posCol = posEnd;

					oss << input.substr(posCol);
				} else {
					// something else, not conforming to paritys format...
					// for example: parity.gnu.gcc: error in loading shared libraries.
					// when running from the testsuite.
					return;
				}
				input = oss.str();
				return;
			}
		}

		void Task::filterLine(std::string& input)
		{
			if(input.empty())
				return;

			if(Context::getContext().getIgnoreOutputFilters())
				return;

			for(FilterMap::const_iterator it = filters_.begin(); it != filters_.end(); ++it)
			{
				if(it->second)
				{
					if(it->first == input)
						input = "";
				} else {
					if(::strstr(input.c_str(), it->first.c_str()))
						input = "";
				}
			}
		}
	}
}

