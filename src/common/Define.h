/*
 * This file is part of the SyphrenaCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SYPHRENA_DEFINE_H
#define SYPHRENA_DEFINE_H

#include "CompilerDefs.h"

#if SYPHRENA_COMPILER == SYPHRENA_COMPILER_GNU
#  if !defined(__STDC_FORMAT_MACROS)
#    define __STDC_FORMAT_MACROS
#  endif
#  if !defined(__STDC_CONSTANT_MACROS)
#    define __STDC_CONSTANT_MACROS
#  endif
#  if !defined(_GLIBCXX_USE_NANOSLEEP)
#    define _GLIBCXX_USE_NANOSLEEP
#  endif
#  if defined(HELGRIND)
#    include <valgrind/helgrind.h>
#    undef _GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE
#    undef _GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER
#    define _GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE(A) ANNOTATE_HAPPENS_BEFORE(A)
#    define _GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER(A)  ANNOTATE_HAPPENS_AFTER(A)
#  endif
#endif

#include <cstddef>
#include <cinttypes>
#include <climits>

#define SYPHRENA_LITTLEENDIAN 0
#define SYPHRENA_BIGENDIAN    1

#if !defined(SYPHRENA_ENDIAN)
#  if defined (BOOST_BIG_ENDIAN)
#    define SYPHRENA_ENDIAN SYPHRENA_BIGENDIAN
#  else
#    define SYPHRENA_ENDIAN SYPHRENA_LITTLEENDIAN
#  endif
#endif

#if SYPHRENA_PLATFORM == SYPHRENA_PLATFORM_WINDOWS
#  define SYPHRENA_PATH_MAX 260
#else // SYPHRENA_PLATFORM != SYPHRENA_PLATFORM_WINDOWS
#  define SYPHRENA_PATH_MAX PATH_MAX
#endif // SYPHRENA_PLATFORM

#if !defined(COREDEBUG)
#  define SYPHRENA_INLINE inline
#else //COREDEBUG
#  if !defined(SYPHRENA_DEBUG)
#    define SYPHRENA_DEBUG
#  endif //SYPHRENA_DEBUG
#  define SYPHRENA_INLINE
#endif //!COREDEBUG

#if SYPHRENA_COMPILER == SYPHRENA_COMPILER_GNU
#  define ATTR_PRINTF(F, V) __attribute__ ((__format__ (__printf__, F, V)))
#else //SYPHRENA_COMPILER != SYPHRENA_COMPILER_GNU
#  define ATTR_PRINTF(F, V)
#endif //SYPHRENA_COMPILER == SYPHRENA_COMPILER_GNU

#ifdef SYPHRENA_API_USE_DYNAMIC_LINKING
#  if SYPHRENA_COMPILER == SYPHRENA_COMPILER_MICROSOFT
#    define SC_API_EXPORT __declspec(dllexport)
#    define SC_API_IMPORT __declspec(dllimport)
#  elif SYPHRENA_COMPILER == SYPHRENA_COMPILER_GNU
#    define SC_API_EXPORT __attribute__((visibility("default")))
#    define SC_API_IMPORT
#  else
#    error compiler not supported!
#  endif
#else
#  define SC_API_EXPORT
#  define SC_API_IMPORT
#endif

#ifdef SYPHRENA_API_EXPORT_COMMON
#  define SC_COMMON_API SC_API_EXPORT
#else
#  define SC_COMMON_API SC_API_IMPORT
#endif

#ifdef SYPHRENA_API_EXPORT_DATABASE
#  define SC_DATABASE_API SC_API_EXPORT
#else
#  define SC_DATABASE_API SC_API_IMPORT
#endif

#ifdef SYPHRENA_API_EXPORT_SHARED
#  define SC_SHARED_API SC_API_EXPORT
#else
#  define SC_SHARED_API SC_API_IMPORT
#endif

#ifdef SYPHRENA_API_EXPORT_GAME
#  define SC_GAME_API SC_API_EXPORT
#else
#  define SC_GAME_API SC_API_IMPORT
#endif

#define UI64FMTD "%" PRIu64
#define UI64LIT(N) UINT64_C(N)

#define SI64FMTD "%" PRId64
#define SI64LIT(N) INT64_C(N)

#define SZFMTD "%" PRIuPTR

#define STRING_VIEW_FMT "%.*s"
#define STRING_VIEW_FMT_ARG(str) static_cast<int>((str).length()), (str).data()

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

#endif //SYPHRENA_DEFINE_H
