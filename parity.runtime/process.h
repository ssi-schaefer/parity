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

#ifndef __PCRT_PROCESS_H__
#define __PCRT_PROCESS_H__

#include "internal/pcrt.h"

#pragma push_macro("__STDC__")
#pragma push_macro("_POSIX_")
#  ifdef __STDC__
#    undef __STDC__
#  endif
#  ifdef _POSIX_
#    undef _POSIX_
#  endif
#  pragma push_macro("execl")
#  pragma push_macro("execle")
#  pragma push_macro("execlp")
#  pragma push_macro("execlpe")
#  pragma push_macro("execv")
#  pragma push_macro("execve")
#  pragma push_macro("execvp")
#  pragma push_macro("execvpe")
#  pragma push_macro("spawnl")
#  pragma push_macro("spawnle")
#  pragma push_macro("spawnlp")
#  pragma push_macro("spawnlpe")
#  pragma push_macro("spawnv")
#  pragma push_macro("spawnve")
#  pragma push_macro("spawnvp")
#  pragma push_macro("spawnvpe")
#    define execl __crt_invalid_execl
#    define execle __crt_invalid_execle
#    define execlp __crt_invalid_execlp
#    define execlpe __crt_invalid_execlpe
#    define execv __crt_invalid_execv
#    define execve __crt_invalid_execve
#    define execvp __crt_invalid_execvp
#    define execvpe __crt_invalid_execvpe
#    define spawnl __crt_invalid_spawnl
#    define spawnle __crt_invalid_spawnle
#    define spawnlp __crt_invalid_spawnlp
#    define spawnlpe __crt_invalid_spawnlpe
#    define spawnv __crt_invalid_spawnv
#    define spawnve __crt_invalid_spawnve
#    define spawnvp __crt_invalid_spawnvp
#    define spawnvpe __crt_invalid_spawnvpe
#    include RUNTIME_INC(process.h)
#  pragma pop_macro("execl")
#  pragma pop_macro("execle")
#  pragma pop_macro("execlp")
#  pragma pop_macro("execlpe")
#  pragma pop_macro("execv")
#  pragma pop_macro("execve")
#  pragma pop_macro("execvp")
#  pragma pop_macro("execvpe")
#  pragma pop_macro("spawnl")
#  pragma pop_macro("spawnle")
#  pragma pop_macro("spawnlp")
#  pragma pop_macro("spawnlpe")
#  pragma pop_macro("spawnv")
#  pragma pop_macro("spawnve")
#  pragma pop_macro("spawnvp")
#  pragma pop_macro("spawnvpe")
#pragma pop_macro("__STDC__")
#pragma pop_macro("_POSIX_")

PCRT_BEGIN_C

#pragma push_macro("execl")
#pragma push_macro("execle")
#pragma push_macro("execlp")
#pragma push_macro("execlpe")
#pragma push_macro("execv")
#pragma push_macro("execve")
#pragma push_macro("execvp")
#pragma push_macro("execvpe")
#pragma push_macro("spawnl")
#pragma push_macro("spawnle")
#pragma push_macro("spawnlp")
#pragma push_macro("spawnlpe")
#pragma push_macro("spawnv")
#pragma push_macro("spawnve")
#pragma push_macro("spawnvp")
#pragma push_macro("spawnvpe")

#undef execl
#undef execle
#undef execlp
#undef execlpe
#undef execv
#undef execve
#undef execvp
#undef execvpe
#undef spawnl
#undef spawnle
#undef spawnlp
#undef spawnlpe
#undef spawnv
#undef spawnve
#undef spawnvp
#undef spawnvpe

#define __PCRT_MAX_EXEC_ARGS 255

#define __PCRT_TRANSFORM_VAR(first, list, args) { \
		size_t arg_count = 0; \
		char const * current = first; \
		while(current) { \
			args[arg_count++] = current; \
			current = va_arg(list, char const*); \
		} \
	}

#define __PCRT_EXECL(func_name, file, first) \
	intptr_t ret; \
	char const * args[__PCRT_MAX_EXEC_ARGS] = { 0 }; \
	va_list list; \
	va_start(list, first); \
	__PCRT_TRANSFORM_VAR(first, list, args); \
	va_end(list); \
	ret = func_name(PCRT_CONV(file), args); \
	return ret;

#define __PCRT_EXECLE(func_name, file, first) \
	intptr_t ret; \
	char const * args[__PCRT_MAX_EXEC_ARGS] = { 0 }; \
	char const * const * env; \
	va_list list; \
	va_start(list, first); \
	__PCRT_TRANSFORM_VAR(first, list, args); \
	env = va_arg(list, char const * const *); \
	va_end(list); \
	ret = func_name(PCRT_CONV(file), args, env); \
	return ret;

static PCRT_INLINE intptr_t execl(const char* f, const char* l, ...) { __PCRT_EXECL(_execv, f, l); }
static PCRT_INLINE intptr_t execle(const char* f, const char* l, ...) { __PCRT_EXECLE(_execve, f, l); }
static PCRT_INLINE intptr_t execlp(const char* f, const char* l, ...) { __PCRT_EXECL(_execvp, f, l); }
static PCRT_INLINE intptr_t execlpe(const char* f, const char* l, ...) { __PCRT_EXECLE(_execvpe, f, l); }

static PCRT_INLINE intptr_t execv(const char* f, const char* const* a) { return _execv(PCRT_CONV(f), a); }
static PCRT_INLINE intptr_t execve(const char* f, const char* const* a, const char* const* e) { return _execve(PCRT_CONV(f), a, e); }
static PCRT_INLINE intptr_t execvp(const char* f, const char* const* a) { return _execvp(PCRT_CONV(f), a); }
static PCRT_INLINE intptr_t execvpe(const char* f, const char* const* a, const char* const* e) { return _execvpe(PCRT_CONV(f), a, e); }

#define __PCRT_SPAWNL(func_name, mode, file, first) \
	intptr_t ret; \
	char const * args[__PCRT_MAX_EXEC_ARGS] = { 0 }; \
	va_list list; \
	va_start(list, first); \
	__PCRT_TRANSFORM_VAR(first, list, args); \
	va_end(list); \
	ret = func_name(mode, PCRT_CONV(file), args); \
	return ret;

#define __PCRT_SPAWNLE(func_name, mode, file, first) \
	intptr_t ret; \
	char const * args[__PCRT_MAX_EXEC_ARGS] = { 0 }; \
	char const * const * env; \
	va_list list; \
	va_start(list, first); \
	__PCRT_TRANSFORM_VAR(first, list, args); \
	env = va_arg(list, char const * const *); \
	va_end(list); \
	ret = func_name(mode, PCRT_CONV(file), args, env); \
	return ret;

static PCRT_INLINE intptr_t spawnl(int m, const char* f, const char* l, ...) { __PCRT_SPAWNL(_spawnv, m, f, l); }
static PCRT_INLINE intptr_t spawnle(int m, const char* f, const char* l, ...) { __PCRT_SPAWNLE(_spawnve, m, f, l); }
static PCRT_INLINE intptr_t spawnlp(int m, const char* f, const char* l, ...) { __PCRT_SPAWNL(_spawnvp, m, f, l); }
static PCRT_INLINE intptr_t spawnlpe(int m, const char* f, const char* l, ...) { __PCRT_SPAWNLE(_spawnvpe, m, f, l); }

static PCRT_INLINE intptr_t spawnv(int m, const char* f, const char* const* a) { return _spawnv(m, PCRT_CONV(f), a); }
static PCRT_INLINE intptr_t spawnve(int m, const char* f, const char* const* a, const char* const* e) { return _spawnve(m, PCRT_CONV(f), a, e); }
static PCRT_INLINE intptr_t spawnvp(int m, const char* f, const char* const* a) { return _spawnvp(m, PCRT_CONV(f), a); }
static PCRT_INLINE intptr_t spawnvpe(int m, const char* f, const char* const* a, const char* const* e) { return _spawnvpe(m, PCRT_CONV(f), a, e); }

#pragma pop_macro("execl")
#pragma pop_macro("execle")
#pragma pop_macro("execlp")
#pragma pop_macro("execlpe")
#pragma pop_macro("execv")
#pragma pop_macro("execve")
#pragma pop_macro("execvp")
#pragma pop_macro("execvpe")
#pragma pop_macro("spawnl")
#pragma pop_macro("spawnle")
#pragma pop_macro("spawnlp")
#pragma pop_macro("spawnlpe")
#pragma pop_macro("spawnv")
#pragma pop_macro("spawnve")
#pragma pop_macro("spawnvp")
#pragma pop_macro("spawnvpe")

PCRT_END_C

#endif

