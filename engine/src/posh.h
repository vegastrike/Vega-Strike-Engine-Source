/**
* posh.h
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

/** @file posh.h
 *   @author Brian Hook, poshlib@poshlib.org
 *   @version 0.0
 *   @brief Portable Open Source Harness header file
 *
 *  Header file for POSH, the Portable Open Source Harness project.
 *
 *  NOTE: Unlike most header files, this one is designed to be included
 *  multiple times, which is why it does not have the @#ifndef/@#define
 *  preamble.
 *
 *  POSH relies on environment specified preprocessor symbols in order
 *  to infer as much as possible about the target OS/architecture and
 *  the host compiler capabilities.
 *
 *  NOTE: POSH is simple and focused. It attempts to provide basic
 *  functionality and information, but it does NOT attempt to emulate
 *  missing functionality.  I am also not willing to make POSH dirty
 *  and hackish to support truly ancient and/or outmoded and/or bizarre
 *  technologies such as non-ANSI compilers, systems with non-IEEE
 *  floating point formats, 16-bit operating systems, etc.
 *
 *  Please refer to the accompanying HTML documentation or visit
 *  http://www.poshlib.org for more information on how to use POSH.
 *
 *  @internal
 *
 *  I have yet to find an authoritative reference on preprocessor
 *  symbols, but so far this is what I've gleaned:
 *
 *  GNU GCC/G++:
 *  - __GNUC__: GNU C version
 *  - __GNUG__: GNU C++ compiler
 *  - __sun__ : on Sun platforms
 *  - __svr4__: on Solaris and other SysV R4 platforms
 *  - __mips__: on MIPS processor platforms
 *  - __sparc_v9__: on Sparc 64-bit CPUs
 *  - __sparcv9: 64-bit Solaris
 *  - __MIPSEL__: mips processor, compiled for little endian
 *  - __MIPSEB__: mips processor, compiled for big endian
 *  - _R5900: MIPS/Sony/Toshiba R5900 (PS2)
 *  - mc68000: 68K
 *  - m68000: 68K
 *  - m68k: 68K
 *  - __palmos__: PalmOS
 *
 *  Apple's C/C++ Compiler for OS X:
 *  - __APPLE_CC__
 *  - __APPLE__
 *  - __BIG_ENDIAN__
 *  - __APPLE__
 *  - __ppc__
 *  - __MACH__
 *
 *  DJGPP:
 *  - __MSDOS__
 *  - __unix__
 *  - __unix
 *  - __GNUC__
 *  - __GO32
 *  - DJGPP
 *  - __i386, __i386, i386
 *
 *  Cray's C compiler:
 *  - _ADDR64: if 64-bit pointers
 *  - _UNICOS:
 *  - __unix:
 *
 *  SGI's CC compiler predefines the following (and more) with -ansi:
 *  - __sgi
 *  - __unix
 *  - __host_mips
 *  - _SYSTYPE_SVR4
 *  - __mips
 *  - _MIPSEB
 *
 *  MingW:
 *  - as GnuC but also defines _WIN32, __WIN32, WIN32, _X86_, __i386, __i386__, and several others
 *  - __MINGW32__
 *
 *  Cygwin:
 *  - as Gnu C, but also
 *  - __unix__
 *  - __CYGWIN32__
 *
 *
 *  Microsoft Visual Studio predefines the following:
 *  - _MSC_VER
 *  - _WIN32: on Win32
 *  - _M_IX6 (on x86 systems)
 *  - _M_ALPHA (on DEC AXP systems)
 *  - _SH3: WinCE, Hitachi SH-3
 *  - _MIPS: WinCE, MIPS
 *  - _ARM: WinCE, ARM
 *
 *  Sun's C Compiler:
 *  - sun and _sun
 *  - unix and _unix
 *  - sparc and _sparc (SPARC systems only)
 *  - i386 and _i386 (x86 systems only)
 *  - __SVR4 (Solaris only)
 *  - __sparcv9: 64-bit solaris
 *  - _LP64: defined in 64-bit LP64 mode, but only if <sys/types.h> is included
 *
 *  Borland C/C++ predefines the following:
 *  - __BORLANDC__:
 *
 *  DEC/Compaq C/C++ on Alpha:
 *  - __alpha
 *  - __arch64__
 *  - __unix__ (on Tru64 Unix)
 *  - __osf__
 *  - __DECC
 *  - __DECCXX
 *  - __DECC_VER
 *  - __DECCXX_VER
 *
 *  IBM's AIX compiler:
 *  - __64BIT__ if 64-bit mode
 *  - _AIX
 *  - __IBMC__: C compiler version
 *  - __IBMCPP__: C++ compiler version
 *  - _LONG_LONG: compiler allows long long
 *
 *  Watcom:
 *  - __WATCOMC__
 *  - __DOS__ : if targeting DOS
 *  - __386__ : if 32-bit support
 *  - __WIN32__ : if targetin 32-bit Windows
 *
 *  HP-UX C/C++ Compiler:
 *  - __hpux
 *  - __unix
 *  - __hppa (on PA-RISC)
 *  - __LP64__: if compiled in 64-bit mode
 *
 *  Metrowerks:
 *  - __MWERKS__
 *  - __powerpc__
 *  - _powerc
 *  - __MC68K__
 */

/** @defgroup OSSymbols Operating System Symbols
 *   @ingroup ExportedSymbols
 *   @{
 */

#if defined FORCE_DOXYGEN
#  define POSH_OS_STRING /**< a string constant describing the target CPU */
#endif

/* ------------------------------------------------------------------
** Linux
** ------------------------------------------------------------------
*/
#if defined linux || defined __linux__ || defined FORCE_DOXYGEN
#  define POSH_OS_LINUX /**<if defined, target OS is a Linux derivative */
#  if !defined FORCE_DOXYGEN
#     define POSH_OS_STRING "Linux"
#  endif
#endif

/* ------------------------------------------------------------------
** Haiku
** ------------------------------------------------------------------
*/
#if defined(__HAIKU__) || defined FORCE_DOXYGEN
#  define POSH_OS_HAIKU /**<if defined, target OS is a Haiku derivative */
#  if !defined FORCE_DOXYGEN
#     define POSH_OS_STRING "Haiku"
#  endif
#endif

/* ------------------------------------------------------------------
** Cygwin32
** ------------------------------------------------------------------
*/
#if defined __CYGWIN32__ || defined FORCE_DOXYGEN
#  define POSH_OS_CYGWIN32 /**<if defined, target OS is Cygwin */
#  if !defined FORCE_DOXYGEN
#     define POSH_OS_STRING "Cygwin"
#  endif
#endif

/* ------------------------------------------------------------------
** DJGPP/GO32
** ------------------------------------------------------------------
*/
#if defined GO32 && defined DJGPP && defined __MSDOS__ || defined FORCE_DOXYGEN
#  define POSH_OS_GO32 /**<if defined, target OS is DOS 32-bit using DJGPP/GO32 */
#  if !defined FORCE_DOXYGEN
#     define POSH_OS_STRING "GO32/MS-DOS"
#  endif
#endif

/* ------------------------------------------------------------------
** 32-bit DOS via Watcom
** NOTE: make sure you use /bt=DOS if compiling for 32-bit DOS,
**       otherwise Watcom assumes host=target
** ------------------------------------------------------------------
*/
#if (defined __WATCOMC__ && defined __386__ && defined __DOS__) || defined FORCE_DOXYGEN
#  define POSH_OS_DOS32 /**<if defined, target OS is 32-bit DOS built with Watcom */
#  if !defined FORCE_DOXYGEN
#     define POSH_OS_STRING "DOS/32-bit"
#  endif
#endif

/* ------------------------------------------------------------------
** Cray UNICOS
** ------------------------------------------------------------------
*/
#if defined _UNICOS || defined FORCE_DOXYGEN
#  define POSH_OS_UNICOS /**<if defined, target OS is Cray UNICOS */
#  if !defined FORCE_DOXYGEN
#  define POSH_OS_STRING "UNICOS"
#  endif
#endif

/* ------------------------------------------------------------------
** Mac OS X
** ------------------------------------------------------------------
*/
#if (defined __MWERKS__ && defined __powerc && !defined macintosh) || defined __APPLE_CC__ || defined macosx \
    || defined FORCE_DOXYGEN
#  define POSH_OS_OSX /**<if defined, target OS is Mac OS X */
#  if !defined FORCE_DOXYGEN
#    define POSH_OS_STRING "MacOS X"
#  endif
#endif

/* ------------------------------------------------------------------
** Solaris/SunOS
** ------------------------------------------------------------------
*/
#if defined __sun__ || defined sun || defined __sun || defined __solaris__ || defined FORCE_DOXYGEN
#  if defined __SVR4 || defined __svr4__ || defined __solaris__ || defined FORCE_DOXYGEN
#     if !defined FORCE_DOXYGEN
#        define POSH_OS_STRING "Solaris"
#     endif
#     define POSH_OS_SOLARIS /**<if defined, target OS is Sun Solaris */
#  endif
#  if defined FORCE_DOXYGEN || !defined POSH_OS_STRING
#     define POSH_OS_SUNOS /**<if defined, target OS is SunOS */
#     if !defined FORCE_DOXYGEN
#        define POSH_OS_STRING "SunOS"
#     endif
#  endif
#endif

/* ------------------------------------------------------------------
** SGI Irix
** ------------------------------------------------------------------
*/
#if defined __sgi__ || defined sgi || defined __sgi || defined FORCE_DOXYGEN
#  define POSH_OS_IRIX /**<if defined, target OS is SGI Irix */
#  if !defined FORCE_DOXYGEN
#     define POSH_OS_STRING "Irix"
#  endif
#endif

/* ------------------------------------------------------------------
** HP-UX
** ------------------------------------------------------------------
*/
#if defined __hpux__ || defined __hpux || defined FORCE_DOXYGEN
#  define POSH_OS_HPUX /**<if defined, target OS is HP-UX */
#  if !defined FORCE_DOXYGEN
#     define POSH_OS_STRING "HP-UX"
#  endif
#endif

/* ------------------------------------------------------------------
** AIX
** ------------------------------------------------------------------
*/
#if defined _AIX || defined FORCE_DOXYGEN
#  define POSH_OS_AIX /**<if defined, target OS is IBM AIX */
#  if !defined FORCE_DOXYGEN
#     define POSH_OS_STRING "AIX"
#  endif
#endif

/* ------------------------------------------------------------------
** DEC Alpha/Tru64
** ------------------------------------------------------------------
*/
#if (defined __alpha && defined __osf__) || defined FORCE_DOXYGEN
#  define POSH_OS_TRU64 /**<if defined, target OS is Compaq/DEC Tru64 Unix */
#  if !defined FORCE_DOXYGEN
#     define POSH_OS_STRING "Tru64"
#  endif
#endif

/* ------------------------------------------------------------------
** BeOS (untested)
** ------------------------------------------------------------------
*/
#if defined __BEOS__ || defined __beos__ || defined FORCE_DOXYGEN
#  define POSH_OS_BEOS /**<if defined, target OS is BeOS */
#  if !defined FORCE_DOXYGEN
#     define POSH_OS_STRING "BeOS"
#  endif
#endif

/* ------------------------------------------------------------------
** Amiga (untested)
** ------------------------------------------------------------------
*/
#if defined amiga || defined amigados || defined AMIGA || defined _AMIGA || defined FORCE_DOXYGEN
#  define POSH_OS_AMIGA  /**<if defined, target OS is AmigaDOS */
#  if !defined FORCE_DOXYGEN
#     define POSH_OS_STRING "Amiga"
#  endif
#endif

/* ------------------------------------------------------------------
** Generic UNIX-like fallback
** ------------------------------------------------------------------
*/
#if defined __unix__ || defined FORCE_DOXYGEN
#  define POSH_OS_UNIX /**<if defined, target OS is generic Unix variant */
#  if !defined POSH_OS_STRING
#     define POSH_OS_STRING "Unix-like(generic)"
#  endif
#endif

/* ------------------------------------------------------------------
** WinCE
** ------------------------------------------------------------------
*/
#if defined _WIN32_WCE || defined FORCE_DOXYGEN
#  define POSH_OS_WINCE /**<if defined, target OS is Windows CE */
#  if !defined FORCE_DOXYGEN
#     define POSH_OS_STRING "Windows CE"
#  endif
#endif

/* ------------------------------------------------------------------
** Win32/Win64
** ------------------------------------------------------------------
*/
#if defined _WIN32 || defined WIN32 || defined __NT__ || defined __WIN32__ || defined FORCE_DOXYGEN
#  define POSH_OS_WIN32 /**<if defined, target OS is Win32 (implies Win9x/Me/2K/XP)*/
#  if defined _WIN64 || defined FORCE_DOXYGEN
#     define POSH_OS_WIN64 /**<if defined, target OS is Win64 */
#     if !defined FORCE_DOXYGEN
#        define POSH_OS_STRING "Win64"
#     endif
#  else
#     if !defined FORCE_DOXYGEN
#        define POSH_OS_STRING "Win32"
#     endif
#  endif
#endif

/* ------------------------------------------------------------------
** PalmOS
** ------------------------------------------------------------------
*/
#if defined __palmos__ || defined FORCE_DOXYGEN
#  define POSH_OS_PALM
#  if !defined FORCE_DOXYGEN
#     define POSH_OS_STRING "PalmOS"
#  endif
#endif

/* ------------------------------------------------------------------
** MacOS
** ------------------------------------------------------------------
*/
#if (defined __MWERKS__ && !defined __powerc) || defined THINK_C || defined macintosh || defined FORCE_DOXYGEN
#  define POSH_OS_MACOS /**<if defined, target OS is MacOS */
#  if !defined FORCE_DOXYGEN
#     define POSH_OS_STRING "MacOS"
#  endif
#endif

/** @} */

/** @defgroup CPUSymbols CPU
 *   @ingroup ExportedSymbols
 *   @{
 */
#if defined FORCE_DOXYGEN
#  define POSH_CPU_STRING /**< a string constant describing the target CPU */
#endif

/* ------------------------------------------------------------------
** Motorola 68000
** ------------------------------------------------------------------
*/
#if defined mc68000 || defined m68k || defined __MC68K__ || defined m68000 || defined FORCE_DOXYGEN
#  define POSH_CPU_68K /**< if defined, target processor is a Motorola 68000 derivative */
#  if !defined FORCE_DOXYGEN
#     define POSH_CPU_STRING "MC68000"
#  endif
#endif

/* ------------------------------------------------------------------
** PowerPC
** ------------------------------------------------------------------
*/
#if defined __PPC__ || defined __POWERPC__ || defined powerpc || defined _POWER || defined __ppc__ || defined __powerpc__ \
    || defined FORCE_DOXYGEN
#  define POSH_CPU_PPC /**< if defined, target processor is a PowerPC derivative */
#  if !defined FORCE_DOXYGEN
#     if defined __powerpc64__
#        define POSH_CPU_STRING "PowerPC64"
#     else
#        define POSH_CPU_STRING "PowerPC"
#     endif
#  endif
#endif

/* ------------------------------------------------------------------
** Cray T3E
** ------------------------------------------------------------------
*/
#if defined _CRAYT3E || defined _CRAYMPP || defined FORCE_DOXYGEN
#  define POSH_CPU_CRAYT3E /**< if defined, target processor is a DEC Alpha 21164 used in a Cray T3E*/
#  if !defined FORCE_DOXYGEN
#     define POSH_CPU_STRING "Cray T3E (Alpha 21164)"
#  endif
#endif

/* ------------------------------------------------------------------
** Cray (non-21164)
** ------------------------------------------------------------------
*/
#if defined CRAY || defined _CRAY && !defined _CRAYT3E
#  error Non-AXP Cray systems not supported
#endif

/* ------------------------------------------------------------------
** Hitachi SH3
** ------------------------------------------------------------------
*/
#if defined _SH3 || defined FORCE_DOXYGEN
#  define POSH_CPU_SH3 /**<if defined, target CPU is a member of the Hitachi SH-3 family */
#  if !defined FORCE_DOXYGEN
#     define POSH_CPU_STRING "Hitachi SH-3"
#  endif
#endif

/* ------------------------------------------------------------------
** Hitachi SH4 (presumably Dreamcast)
** ------------------------------------------------------------------
*/
#if defined __sh4__ || defined __SH4__ || defined FORCE_DOXYGEN
#  define POSH_CPU_SH3 /**<if defined, target CPU is a member of the Hitachi SH-3 family */
#  define POSH_CPU_SH4 /**<if defined, target CPU is a member of the Hitachi SH-4 family */
#  if !defined FORCE_DOXYGEN
#     define POSH_CPU_STRING "Hitachi SH-4"
#  endif
#endif

/* ------------------------------------------------------------------
** Sun SPARC
** ------------------------------------------------------------------
*/
#if defined __sparc__ || defined __sparc || defined FORCE_DOXYGEN
#  if defined __arch64__ || defined __sparcv9 || defined __sparc_v9__ || defined FORCE_DOXYGEN
#     define POSH_CPU_SPARC64 /**<if defined, target CPU is Sun 64-bit Sparc */
#     if !defined FORCE_DOXYGEN
#        define POSH_CPU_STRING "Sparc/64"
#     endif
#  else
#     if !defined FORCE_DOXYGEN
#        define POSH_CPU_STRING "Sparc/32"
#     endif
#  endif
#  define POSH_CPU_SPARC /**<if defined, target CPU is Sun Sparc */
#endif

/* ------------------------------------------------------------------
** StrongARM
** ------------------------------------------------------------------
*/
#if defined __arm__ || defined _ARM || defined FORCE_DOXYGEN
#  define POSH_CPU_STRONGARM /**<if defined, target CPU is StrongARM */
#  if !defined FORCE_DOXYGEN
#     define POSH_CPU_STRING "ARM"
#  endif
#endif

/* ------------------------------------------------------------------
** MIPS (various, including SGI and Sony PS2)
** ------------------------------------------------------------------
*/
#if defined mips || defined __mips__ || defined __MIPS__ || defined _MIPS || defined FORCE_DOXYGEN
#  define POSH_CPU_MIPS /**<if defined, target CPU is MIPS variant */
#  if defined _R5900 && !defined FORCE_DOXYGEN
#    define POSH_CPU_STRING "MIPS R5900 (PS2)"
#  elif !defined FORCE_DOXYGEN
#    define POSH_CPU_STRING "MIPS"
#  endif
#endif

/* ------------------------------------------------------------------
** Intel Itanium IA64
** ------------------------------------------------------------------
*/
#if defined __ia64 || defined _M_IA64 || defined __ia64__ || defined FORCE_DOXYGEN
#  define POSH_CPU_IA64 /**<if defined, target CPU is Intel Itanium/IA64 */
#  if !defined FORCE_DOXYGEN
#     define POSH_CPU_STRING "IA64"
#  endif
#endif

/* ------------------------------------------------------------------
** Intel x86 and AMD x86-64
** ------------------------------------------------------------------
*/
#if defined __X86__ || defined __i386__ || defined i386 || defined _M_IX86 || defined __386__ || defined FORCE_DOXYGEN \
    || defined __amd64__ || defined __AMD64__ || defined __x86_64__
#  define POSH_CPU_X86 /**<if defined, target CPU is Intel 386+ compatible */
#  if defined __x86_64__ || defined FORCE_DOXYGEN || defined __amd64__ || defined __AMD64__
#     define POSH_CPU_X86_64 /**<if defined, target CPU is AMD x86-64 */
#  endif
#  if !defined FORCE_DOXYGEN
#     if defined POSH_CPU_X86_64
#        define POSH_CPU_STRING "AMD x86-64"
#     else
#        define POSH_CPU_STRING "Intel 386+"
#     endif
#  endif
#endif

/* ------------------------------------------------------------------
** IBM S/390
** ------------------------------------------------------------------
*/
#if defined __s390__ || defined FORCE_DOXYGEN
#  define POSH_CPU_S390/**<if defined, target CPU is IBM S/390 */
#  if !defined FORCE_DOXYGEN
#     define POSH_CPU_STRING "S/390"
#  endif
#endif

/* ------------------------------------------------------------------
** DEC/Compaq Alpha
** ------------------------------------------------------------------
*/
#if defined __alpha || defined alpha || defined _M_ALPHA || defined __alpha__ || defined FORCE_DOXYGEN
#  define POSH_CPU_AXP /**<if defined, target CPU is DEC Alpha */
#  if !defined FORCE_DOXYGEN
#     define POSH_CPU_STRING "AXP"
#  endif
#endif

/* ------------------------------------------------------------------
** HP PA-RISC
** ------------------------------------------------------------------
*/
#if defined __hppa || defined hppa || defined FORCE_DOXYGEN
#  define POSH_CPU_HPPA /**<if defined, target CPU is HP PA-RISC */
#  if !defined FORCE_DOXYGEN
#     define POSH_CPU_STRING "PA-RISC"
#  endif
#endif

/* ------------------------------------------------------------------
** AArch64
** ------------------------------------------------------------------
*/
#if defined __aarch64__ || defined __arm64__ || defined FORCE_DOXYGEN
#  define POSH_CPU_AARCH64 /**<if defined, target CPU is AArch64 */
#  if !defined FORCE_DOXYGEN
#     define POSH_CPU_STRING "AArch64"
#  endif
#endif

/** @} */

#if !defined POSH_CPU_STRING
#  error POSH cannot determine target CPU
#endif

/* Attempt to autodetect building for embedded on Sony PS2 */
#if (!defined POSH_OS_STRING) || defined FORCE_DOXYGEN
#  define POSH_OS_EMBEDDED /**<if defined, target OS is embedded or non-existent such as Sony PS2.  @ingroup OSSymbols */
#  if !defined FORCE_DOXYGEN
#     if defined _R5900
#        define POSH_OS_STRING "Sony PS2(embedded)"
#     elif !defined
#        define POSH_OS_STRING "Embedded/Unknown"
#     endif
#  endif
#endif

/* ---------------------------------------------------------------------------
** Handle cdecl, stdcall, fastcall, etc.
** ---------------------------------------------------------------------------
*/
/** @def POSH_CDECL
 *   @ingroup ExportedSymbols
 *
 *  Specifies that a function should use the C calling convention.  The cdecl
 *  convention passes parameters on the stack, with the caller responsible for
 *  cleaning up the stack (which in turn allows for variable argument lists).
 *  It's marginally slower than the other calling conventions, and is the
 *  default calling convention for the C language.
 *
 *  <code><pre>
 *  @verbatim
 *  extern void POSH_CDECL someFunction( const char *, ... );
 *  @endverbatim
 *  </code></pre>
 */

/** @def POSH_STDCALL
 *   @ingroup ExportedSymbols
 *
 *  Specifies that a function should use the PASCAL/"stdcall" calling convention.
 *  The stdcall convention passes parameters on the stack, however the called
 *  function is responsible for stack cleanup.  This is slightly faster than
 *  cdecl, but without the ability to handle variable argument lists.  It is
 *  also the default calling convention for the Pascal language.
 *
 *  <code><pre>
 *  @verbatim
 *  extern void POSH_STDCALL someFunction( void );
 *  @endverbatim
 *  </code></pre>
 */

/** @def POSH_FASTCALL
 *   @ingroup ExportedSymbols
 *
 *  Specifies that a function should use the register/"fastcall" calling convention.
 *  The fastcall convention passes as many parameters as possible via registers.
 *  If there is an overflow, the remaining parameters are passed on the stack.  It
 *  is the called function's responsibility to remove clean up the stack.
 *
 *  <code><pre>
 *  @verbatim
 *  extern void POSH_FASTCALL someFunction( int x, int y, int z );
 *  @endverbatim
 *  </code></pre>
 */

/** @todo need to get a comprehensive list of how cdecl, et. al. are specified */
#if !defined FORCE_DOXYGEN && (defined _MSC_VER || defined __WATCOMC__ || defined __GNUC__ || defined __BORLANDC__ \
                               || defined __MWERKS__)
#  define POSH_CDECL __cdecl
#  define POSH_STDCALL __stdcall
#  define POSH_FASTCALL __fastcall
#else
#  define POSH_CDECL
#  define POSH_STDCALL
#  define POSH_FASTCALL
#endif

/* ---------------------------------------------------------------------------
** Define POSH_IMPORTEXPORT signature based on POSH_DLL and POSH_BUILDING_LIB
** ---------------------------------------------------------------------------
*/
/**
 *  @def POSH_DLL
 *
 *  define this if your library is being built or used as a DLL
 *
 *  @ingroup ImportedSymbols
 *  <b><code>POSH_DLL</code></b> is read by POSH to determine the proper
 *  import/export linkage specifications for Windows DLLs.
 *
 *  The correct way to do this is to have each library handle this with its
 *  it own constants:
 *
 *  <code><pre>
 *  @#ifdef MYLIB_DLL
 *  @#define POSH_DLL
 *  @#else
 *  @#undef POSH_DLL
 *  @#endif
 *  @#include "posh.h"
 *  </code></pre>
 *
 *  An application using your library should <i>never</i> be required to
 *  explicitly set <code><b>POSH_DLL</b></code>.  Instead, it should be
 *  responsible for setting your library's own appropriate DLL constant
 *  which is then used to set <code><b>POSH_DLL</b></code> as necessary.
 */

/**
 *  @def POSH_BUILDING_LIB
 *
 *  define this when actually building a library (as opposed to building an app).
 *
 *  @ingroup ImportedSymbols
 *
 *  <b><code>POSH_BUILDING_LIB</code></b> is read by POSH to determine the
 *  proper import/export specification for a library.  If you are using POSH
 *  to build a library then you should define this symbol before including
 *  posh.h in your library's source files.
 *
 */

/*
** We undefine this so that multiple inclusions will work
*/
#if defined POSH_IMPORTEXPORT
#  undef POSH_IMPORTEXPORT
#endif

#if defined POSH_DLL
#   if defined POSH_OS_WIN32
#      if defined _MSC_VER
#         if (_MSC_VER >= 800)
#            if defined POSH_BUILDING_LIB
#               define POSH_IMPORTEXPORT __declspec( dllexport )
#            else
#               define POSH_IMPORTEXPORT __declspec( dllimport )
#            endif
#         else
#            if defined POSH_BUILDING_LIB
#               define POSH_IMPORTEXPORT __export
#            else
#               define POSH_IMPORTEXPORT
#            endif
#         endif
#      endif /* defined _MSC_VER */
#      if defined __BORLANDC__
#         if (__BORLANDC__ >= 0x500)
#            if defined POSH_BUILDING_LIB
#               define POSH_IMPORTEXPORT __declspec( dllexport )
#            else
#               define POSH_IMPORTEXPORT __declspec( dllimport )
#            endif
#         else
#            if defined POSH_BUILDING_LIB
#               define POSH_IMPORTEXPORT __export
#            else
#               define POSH_IMPORTEXPORT
#            endif
#         endif
#      endif /* defined __BORLANDC__ */
/* for all other compilers, we're just making a blanket assumption */
#      if defined __GNUC__ || defined __WATCOMC__ || defined __MWERKS__
#         if defined POSH_BUILDING_LIB
#            define POSH_IMPORTEXPORT __declspec( dllexport )
#         else
#            define POSH_IMPORTEXPORT __declspec( dllimport )
#         endif
#      endif /* all other compilers */
#      if !defined POSH_IMPORTEXPORT
#         error Building DLLs not supported on this compiler (poshlib@poshlib.org if you know how)
#      endif
#   endif /* defined POSH_OS_WIN32 */
#endif /* defined POSH_DLL */

/* On pretty much everything else, we can thankfully just ignore this */
#if !defined POSH_IMPORTEXPORT
#  define POSH_IMPORTEXPORT
#endif

#if defined FORCE_DOXYGEN
#  define POSH_DLL
#  define POSH_BUILDING_LIB
#endif

/* ----------------------------------------------------------------------------
** (Re)define POSH_PUBLIC_API export signature
** ----------------------------------------------------------------------------
*/
#ifdef POSH_PUBLIC_API
#  undef POSH_PUBLIC_API
#endif

#if ( ( defined _MSC_VER) && (_MSC_VER < 800 ) ) || (defined __BORLANDC__ && (__BORLANDC__ < 0x500 ) )
#  define POSH_PUBLIC_API( rtype ) extern rtype POSH_IMPORTEXPORT
#else
#  define POSH_PUBLIC_API( rtype ) extern POSH_IMPORTEXPORT rtype
#endif

/** @def POSH_PUBLIC_API
 *   use as a wrapper for all publicly exported functions and data
 *   @ingroup ExportedSymbols
 *   <code><b>POSH_PUBLIC_API</code></b> should be used as a wrapper
 *   around any data or function return type values that are exported
 *   from a DLL.  This macro ensures proper linkage irrespective of
 *   compiler.
 *
 *   Example usage:
 *   <pre><code>
 *   @verbatim
 *   POSH_PUBLIC_API(int) functionThatReturnsInt( void );
 *   POSH_PUBLIC_API(int) globalIntVariable;
 *   @endverbatim
 *   </pre></code>
 */

/* ----------------------------------------------------------------------------
** Try to infer endianess.  Basically we just go through the CPUs we know are
** little endian, and assume anything that isn't one of those is big endian.
** As a sanity check, we also do this with operating systems we know are
** little endian, such as Windows.  Some processors are bi-endian, such as
** the MIPS series, so we have to be careful about those.
** ----------------------------------------------------------------------------
*/
/**@defgroup EndianessSymbols Endianess
 *  @ingroup ExportedSymbols
 *  @{
 */
#if defined FORCE_DOXYGEN
#  define POSH_LITTLE_ENDIAN
#  define POSH_BIG_ENDIAN
#elif defined POSH_CPU_X86 || defined POSH_CPU_AXP || defined POSH_CPU_STRONGARM || defined POSH_OS_WIN32 \
    || defined POSH_OS_WINCE || defined __MIPSEL__ || (defined POSH_CPU_PPC && defined __LITTLE_ENDIAN__)
#  define POSH_LITTLE_ENDIAN 1
#  define POSH_ENDIAN_STRING "little"
#else
#  define POSH_BIG_ENDIAN 1
#  define POSH_ENDIAN_STRING "big"
#endif

/** @def POSH_LITTLE_ENDIAN
 *   if defined, target CPU is little endian
 */

/** @def POSH_BIG_ENDIAN
 *   if defined, target CPU is big endian
 */

/** @} */

/* ----------------------------------------------------------------------------
** Cross-platform compile time assertion macro
** ----------------------------------------------------------------------------
*/
#if defined FORCE_DOXYGEN
#  define POSH_COMPILE_TIME_ASSERT( name, x )
#else
#  define POSH_COMPILE_TIME_ASSERT( name, x ) typedef int _POSH_dummy_##name[(x) ? 1 : -1]
#endif

/** @def POSH_COMPILE_TIME_ASSERT(name,x)
 *   Cross-platform compile time assertion macro that will cause a syntax
 *   error at compile time if the assertion 'x' is not true.
 */

/* ----------------------------------------------------------------------------
** 64-bit Integer
**
** We don't require its presence, nor do we emulate its functionality, we
** simply export it if it's available.
** ----------------------------------------------------------------------------
*/
/** @defgroup SixtyFourBitSupport 64-bit Symbols
 *   @ingroup ExportedSymbols
 */
#if defined FORCE_DOXYGEN
#  define POSH_64BIT_INTEGER
typedef struct compiler_specific posh_i64_t;
typedef struct compiler_specific posh_s64_t;
typedef struct compiler_specific posh_u64_t;
#  define POSH_S64( x )
#  define POSH_U64( x )
#elif defined (__LP64__) || defined (__powerpc64__) || defined POSH_CPU_SPARC64
#  define POSH_64BIT_INTEGER 1
typedef long          posh_i64_t;
typedef long          posh_s64_t;
typedef unsigned long posh_u64_t;
#  define POSH_S64( x ) ( (posh_s64_t) x )
#  define POSH_U64( x ) ( (posh_u64_t) x )
#elif defined __GNUC__ || defined __MWERKS__ || defined __SUNPRO_C || defined __SUNPRO_CC || defined __APPLE_CC__ \
    || defined POSH_OS_IRIX || defined _LONG_LONG || defined _CRAYC
#  define POSH_64BIT_INTEGER 1
typedef long long          posh_i64_t;
typedef long long          posh_s64_t;
typedef unsigned long long posh_u64_t;
#  define POSH_U64( x ) x##LL
#  define POSH_S64( x ) x##LL
#elif defined _MSC_VER || defined __BORLANDC__ || defined __WATCOMC__ || (defined __alpha && defined __DECC)
#  define POSH_64BIT_INTEGER 1
typedef __int64          posh_i64_t;
typedef __int64          posh_s64_t;
typedef unsigned __int64 posh_u64_t;
#  define POSH_S64( x ) ( (posh_s64_t) x )
#  define POSH_U64( x ) ( (posh_u64_t) x )
#endif

/** @def POSH_S64
 *   Macro to make a constant signed 64-bit value
 *   @ingroup SixtyFourBit
 *   This is a compiler specific type definition for a 64-bit constant
 *   value.  You can test for the presence of this macro
 *   by seeing if @ref POSH_64BIT_INTEGER "POSH_64BIT_INTEGER" is defined.
 */
/** @def POSH_U64
 *   Macro to make a constant unsigned 64-bit value
 *   @ingroup SixtyFourBit
 *   This is a compiler specific type definition for a 64-bit constant
 *   value.  You can test for the presence of this macro
 *   by seeing if @ref POSH_64BIT_INTEGER "POSH_64BIT_INTEGER" is defined.
 */

/** @def POSH_64BIT_INTEGER
 *   indicates presence of compiler supported 64-bit integer types.
 *   @ingroup SixtyFourBitSupport
 *   POSH defines this symbol if the underlying compiler supports 64-bit
 *   integer types.  However, it <i>does not</i> indicate whether the
 *   actual hardware supports 64-bit integer operations natively, so
 *   this should not be used as a performance hint.
 */
/** @typedef posh_i64_t
 *   signed 64-bit integer
 *   @ingroup SixtyFourBitSupport
 *   This is a compiler specific type definition for a 64-bit signed integer
 *   type.  On some platforms this may map to <code>long, long long, __int64</code> or,
 *   in some cases, nothing at all.  You can test for the presence of this type
 *   by seeing if @ref POSH_64BIT_INTEGER "POSH_64BIT_INTEGER" is defined.
 */
/** @typedef posh_s64_t
 *   signed 64-bit integer
 *   @ingroup SixtyFourBitSupport
 *   This is a compiler specific type definition for a 64-bit signed integer
 *   type.  On some platforms this may map to <code>long, long long, __int64</code> or,
 *   in some cases, nothing at all.  You can test for the presence of this type
 *   by seeing if @ref POSH_64BIT_INTEGER "POSH_64BIT_INTEGER" is defined.
 */
/** @typedef posh_u64_t
 *   unsigned 64-bit integer
 *   @ingroup SixtyFourBitSupport
 *   This is a compiler specific type definition for a 64-bit unsigned integer
 *   type.  On some platforms this may map to <code>unsigned long, unsigned long
 *   long, unsigned __int64</code> or, in some cases, nothing at all.  You can
 *   test for the presence of this type by seeing if @ref POSH_64BIT_INTEGER "POSH_64BIT_INTEGER"
 *   is defined.
 */

/* ----------------------------------------------------------------------------
** Basic Sized Types
**
** These types are expected to be EXACTLY sized so you can use them for
** serialization.
** ----------------------------------------------------------------------------
*/
/** @defgroup PoshTypes Basic Types
 *   @{ */
typedef unsigned char  posh_byte_t;        /**< unsigned 8-bit integer */
typedef unsigned char  posh_u8_t;          /**< unsigned 8-bit integer */
typedef signed char    posh_i8_t;          /**< signed 8-bit integer */
typedef signed char    posh_s8_t;          /**< signed 8-bit integer */
typedef unsigned short posh_u16_t;         /**< unsigned 16-bit integer */
typedef short          posh_i16_t;         /**< signed 16-bit integer */
typedef short          posh_s16_t;         /**< signed 16-bit integer */

#if !defined POSH_OS_PALM
typedef unsigned       posh_u32_t;         /**< unsigned 32-bit integer */
typedef int            posh_i32_t;         /**< signed 32-bit integer */
typedef int            posh_s32_t;         /**< signed 32-bit integer */
#else
typedef unsigned long  posh_u32_t;         /**< unsigned 32-bit integer */
typedef long           posh_i32_t;         /**< signed 32-bit integer */
typedef long           posh_s32_t;         /**< signed 32-bit integer */
#endif
/** @} */

/* ----------------------------------------------------------------------------
** Floating Point
**
** The safe and portable of handling floating point across architectures that
** may or may not properly spuport IEEE floating point formats is problematic
** at best.  Because of this, we do what we can to error out during compilation
** on those platforms.
**
** If you find that you HAVE to support such an architecture, then
** define POSH_NO_FLOAT and the floating point assertions and
** routines will be immediately stubbed out
** ----------------------------------------------------------------------------
*/
#if defined FORCE_DOXYGEN
#  define POSH_NO_FLOAT
/** @def POSH_NO_FLOAT
 *   define this if you want to disable floating point support
 *   @ingroup ImportedSymbols
 *   <code><b>POSH_NO_FLOAT</b></code> should be defined by the user if they would like
 *   to disable floating point support in POSH.  This removes some (optional)
 *   functionality, but allows POSH to operate safely on a slightly wider variety
 *   of platforms such as Cray and the MIPS R5900 (Sony PS2).
 */
#  undef POSH_NO_FLOAT
#endif

/* ----------------------------------------------------------------------------
** Do sanity checks on expected sizes
** ----------------------------------------------------------------------------
*/
POSH_COMPILE_TIME_ASSERT( posh_byte_t, sizeof (posh_byte_t) == 1 );
POSH_COMPILE_TIME_ASSERT( posh_u8_t, sizeof (posh_u8_t) == 1 );
POSH_COMPILE_TIME_ASSERT( posh_i8_t, sizeof (posh_i8_t) == 1 );
POSH_COMPILE_TIME_ASSERT( posh_s8_t, sizeof (posh_s8_t) == 1 );
POSH_COMPILE_TIME_ASSERT( posh_u16_t, sizeof (posh_u16_t) == 2 );
POSH_COMPILE_TIME_ASSERT( posh_s16_t, sizeof (posh_s16_t) == 2 );
POSH_COMPILE_TIME_ASSERT( posh_i16_t, sizeof (posh_i16_t) == 2 );
POSH_COMPILE_TIME_ASSERT( posh_u32_t, sizeof (posh_u32_t) == 4 );
POSH_COMPILE_TIME_ASSERT( posh_s32_t, sizeof (posh_s32_t) == 4 );
POSH_COMPILE_TIME_ASSERT( posh_i32_t, sizeof (posh_i32_t) == 4 );

#if !defined POSH_NO_FLOAT
POSH_COMPILE_TIME_ASSERT( posh_testfloat_t, sizeof (float) == 4 );
POSH_COMPILE_TIME_ASSERT( posh_testdouble_t, sizeof (double) == 8 );
#endif

#if defined POSH_64BIT_INTEGER
POSH_COMPILE_TIME_ASSERT( posh_u64_t, sizeof (posh_u64_t) == 8 );
POSH_COMPILE_TIME_ASSERT( posh_s64_t, sizeof (posh_s64_t) == 8 );
POSH_COMPILE_TIME_ASSERT( posh_i64_t, sizeof (posh_i64_t) == 8 );
#endif

/* ----------------------------------------------------------------------------
** 64-bit pointer support
**
** NOTE: Just because a pointer is 64-bits does NOT mean that you have a
** a 64-bit address space.  In practice on most platforms this may be true,
** but on some platforms, such as the Cray vector processors, this is not
** the case.  Similarly, 32-bit pointers do not necessarily mean 32-bit
** address spaces, such as on early 68K processors that had 24-bit address
** space.
** ----------------------------------------------------------------------------
*/
#if defined FORCE_DOXYGEN

/** indicates the presence of 64-bit pointers
 *   @ingroup SixtyFourBitSupport
 *   POSH defines this symbol if the target platform supports 64-bit pointer
 *   types and, implicitly, 64-bit addressing.
 */
#   define POSH_64BIT_POINTER

#endif /* defined FORCE_DOXYGEN */

#if defined POSH_CPU_AXP && (defined POSH_OS_TRU64 || defined POSH_OS_LINUX || defined __FreeBSD__)
#  define POSH_64BIT_POINTER 1
#endif

#if defined POSH_CPU_X86_64 && defined POSH_OS_LINUX
#  define POSH_64BIT_POINTER 1
#endif

#if defined POSH_CPU_SPARC64 || defined POSH_OS_WIN64 || defined __64BIT__ || defined __LP64 || defined _LP64 \
    || defined __LP64__ || defined _ADDR64 || defined _CRAYC
#   define POSH_64BIT_POINTER 1
#endif

#if defined POSH_64BIT_POINTER
POSH_COMPILE_TIME_ASSERT( posh_64bit_pointer, sizeof (void*) == 8 );
#else
/* if this assertion is hit then you're on a system that either has 64-bit
 *  addressing and we didn't catch it, or you're on a system with 16-bit
 *  pointers.  In the latter case, POSH doesn't actually care, we're just
 *  triggering this assertion to make sure you're aware of the situation,
 *  so feel free to just delete the following line.
 *
 *  If this assertion is triggered on a known 32 or 64-bit architecture,
 *  please let us know (poshlib@poshlib.org) */
POSH_COMPILE_TIME_ASSERT( posh_32bit_pointer, sizeof (void*) == 4 );
#endif

/* ----------------------------------------------------------------------------
** POSH Utility Functions
**
** These are optional POSH utility functions that are not required if you don't
** need anything except static checking of your host and target environment.
**
** These functions are NOT wrapped with POSH_PUBLIC_API because I didn't want
** to enforce their export if your own library is only using them internally.
** ----------------------------------------------------------------------------
*/

/** @defgroup PoshFunctions Utility Functions
 *   @{
 */
extern const char * POSH_GetArchString( void );
/** @} */

/** @defgroup FloatingPoint Floating Point
 *   Unavailable if @ref POSH_NO_FLOAT "POSH_NO_FLOAT" is defined.  By
 *   default floating point support is enabled, but if you find that
 *   this causes problems or is inconvenient and you're not using
 *   the support, you can define <code><b>POSH_NO_FLOAT</code></b> to disable
 *   POSH's use of any floating point types or operations.
 *
 *   Here are some code examples:
 *
 *  @verbatim
 *
 *  //read a little-endian float from disk
 *  float LoadFloat( FILE *fp )
 *  {
 *  float      f;
 *  posh_u32_t u32;
 *
 *  fread( &u32, sizeof( u32 ), 1, fp );
 *
 *  f = POSH_FloatFromLittleBits( u32 );
 *
 *  return f;
 *  }
 *
 *  //write a little-endian float to disk
 *  void WriteFloat( FILE *fp, float f )
 *  {
 *  posh_u32_t u32;
 *
 *  u32 = POSH_LittleFloatBits( f );
 *
 *  fwrite( &u32, sizeof( u32 ), 1, fp );
 *  }
 *
 *  @endverbatim
 *
 *  @{
 */

#if !defined POSH_NO_FLOAT
extern posh_u32_t POSH_LittleFloatBits( float f );
extern posh_u32_t POSH_BigFloatBits( float f );
extern float POSH_FloatFromLittleBits( posh_u32_t bits );
extern float POSH_FloatFromBigBits( posh_u32_t bits );

extern void POSH_DoubleBits( double d, posh_byte_t dst[8] );
extern double POSH_DoubleFromBits( const posh_byte_t src[8] );

extern float * POSH_WriteFloatToLittle( void *dst, float f );
extern float * POSH_WriteFloatToBig( void *dst, float f );
extern float POSH_ReadFloatFromLittle( const void *src );
extern float POSH_ReadFloatFromBig( const void *src );

extern double * POSH_WriteDoubleToLittle( void *dst, double d );
extern double * POSH_WriteDoubleToBig( void *dst, double d );
extern double POSH_ReadDoubleFromLittle( const void *src );
extern double POSH_ReadDoubleFromBig( const void *src );
#endif /* !defined POSH_NO_FLOAT */

/** @} */

/** @defgroup ByteSwapFunctions Byte Swapping Functions
 *   These functions perform byte swapping of 16 and 32-bit values.
 *   The 64-bit versions of these functions are documented under
 *   @ref SixtyFourBit
 *   @{
 */

extern posh_u16_t POSH_SwapU16( posh_u16_t u );
extern posh_s16_t POSH_SwapS16( posh_s16_t u );

extern posh_u32_t POSH_SwapU32( posh_u32_t u );
extern posh_s32_t POSH_SwapS32( posh_s32_t u );

/** @} */

/* These aren't in the byte swapping group, but are in the (optional)
 *  64-bit support group */
#if defined POSH_64BIT_INTEGER

extern posh_u64_t POSH_SwapU64( posh_u64_t u );
extern posh_s64_t POSH_SwapS64( posh_s64_t u );

#endif /* defined POSH_64BIT_INTEGER */

/** @defgroup MemoryBuffer In Memory Serialization/Deserialization  Functions
 *   These functions take host endian values and serialize them into
 *   explicit endianess buffers and vice versa.  The 64-bit versions
 *   of these functions can be found under @ref SixtyFourBit
 *
 *  Here's an example usage that serializes a struct into a big-endian buffer:
 *
 *  @verbatim
 *
 *  struct mystruct
 *  {
 *  posh_u16_t u16;
 *  posh_s32_t s32;
 *  };
 *
 *  void WriteStruct( void *buffer, const struct mystruct *s )
 *  {
 *  void *p = buffer;
 *
 *  //The POSH_Write??? functions return a pointer to the next write address
 *  p = POSH_WriteU16ToBig( p, &s->u16 );
 *  p = POSH_WriteS32ToBig( p, &s->s32 );
 *  }
 *
 *  @endverbatim
 *
 *  @{
 */
extern posh_u16_t * POSH_WriteU16ToLittle( void *dst, posh_u16_t value );
extern posh_s16_t * POSH_WriteS16ToLittle( void *dst, posh_s16_t value );
extern posh_u32_t * POSH_WriteU32ToLittle( void *dst, posh_u32_t value );
extern posh_s32_t * POSH_WriteS32ToLittle( void *dst, posh_s32_t value );

extern posh_u16_t * POSH_WriteU16ToBig( void *dst, posh_u16_t value );
extern posh_s16_t * POSH_WriteS16ToBig( void *dst, posh_s16_t value );
extern posh_u32_t * POSH_WriteU32ToBig( void *dst, posh_u32_t value );
extern posh_s32_t * POSH_WriteS32ToBig( void *dst, posh_s32_t value );

extern posh_u16_t POSH_ReadU16FromLittle( const void *src );
extern posh_s16_t POSH_ReadS16FromLittle( const void *src );
extern posh_u32_t POSH_ReadU32FromLittle( const void *src );
extern posh_s32_t POSH_ReadS32FromLittle( const void *src );

extern posh_u16_t POSH_ReadU16FromBig( const void *src );
extern posh_s16_t POSH_ReadS16FromBig( const void *src );
extern posh_u32_t POSH_ReadU32FromBig( const void *src );
extern posh_s32_t POSH_ReadS32FromBig( const void *src );

/** @} */

#if defined POSH_64BIT_INTEGER
extern posh_u64_t * POSH_WriteU64ToLittle( void *dst, posh_u64_t value );
extern posh_s64_t * POSH_WriteS64ToLittle( void *dst, posh_s64_t value );
extern posh_u64_t * POSH_WriteU64ToBig( void *dst, posh_u64_t value );
extern posh_s64_t * POSH_WriteS64ToBig( void *dst, posh_s64_t value );

extern posh_u64_t POSH_ReadU64FromLittle( const void *src );
extern posh_s64_t POSH_ReadS64FromLittle( const void *src );
extern posh_u64_t POSH_ReadU64FromBig( const void *src );
extern posh_s64_t POSH_ReadS64FromBig( const void *src );
#endif /* POSH_64BIT_INTEGER */

/** @defgroup ByteSwapMacros Endianess Conversion Macros
 *   The actual definitions of these macros depend on the underlying platform.
 *   They will either map to a no-op, or they will map to the appropriate
 *   byte swapping function.
 *   @{
 */
#if defined FORCE_DOXYGEN

#define POSH_LittleU16( x ) /**< Converts from little endian to host endian.*/
#define POSH_LittleU32( x ) /**< Converts from little endian to host endian.*/
#define POSH_LittleU64( x ) /**< Converts from little endian to host endian.*/
#define POSH_LittleS16( x ) /**< Converts from little endian to host endian.*/
#define POSH_LittleS32( x ) /**< Converts from little endian to host endian.*/
#define POSH_LittleS64( x ) /**< Converts from little endian to host endian.*/

#define POSH_BigU16( x ) /**< Converts from big endian to host endian.*/
#define POSH_BigU32( x ) /**< Converts from big endian to host endian.*/
#define POSH_BigU64( x ) /**< Converts from big endian to host endian.*/
#define POSH_BigS16( x ) /**< Converts from big endian to host endian.*/
#define POSH_BigS32( x ) /**< Converts from big endian to host endian.*/
#define POSH_BigS64( x ) /**< Converts from big endian to host endian.*/

#elif defined POSH_LITTLE_ENDIAN

#define POSH_LittleU16( x ) (x)
#define POSH_LittleU32( x ) (x)
#define POSH_LittleS16( x ) (x)
#define POSH_LittleS32( x ) (x)
#if defined POSH_64BIT_INTEGER
#  define POSH_LittleU64( x ) (x)
#  define POSH_LittleS64( x ) (x)
#endif /* defined POSH_64BIT_INTEGER */

#define POSH_BigU16( x ) POSH_SwapU16( x )
#define POSH_BigU32( x ) POSH_SwapU32( x )
#define POSH_BigS16( x ) POSH_SwapS16( x )
#define POSH_BigS32( x ) POSH_SwapS32( x )
#if defined POSH_64BIT_INTEGER
#  define POSH_BigU64( x ) POSH_SwapU64( x )
#  define POSH_BigS64( x ) POSH_SwapS64( x )
#endif /* defined POSH_64BIT_INTEGER */

#else

#define POSH_BigU16( x ) (x)
#define POSH_BigU32( x ) (x)
#define POSH_BigS16( x ) (x)
#define POSH_BigS32( x ) (x)

#if defined POSH_64BIT_INTEGER
#  define POSH_BigU64( x ) (x)
#  define POSH_BigS64( x ) (x)
#endif /* POSH_64BIT_INTEGER */

#define POSH_LittleU16( x ) POSH_SwapU16( x )
#define POSH_LittleU32( x ) POSH_SwapU32( x )
#define POSH_LittleS16( x ) POSH_SwapS16( x )
#define POSH_LittleS32( x ) POSH_SwapS32( x )

#if defined POSH_64BIT_INTEGER
#  define POSH_LittleU64( x ) POSH_SwapU64( x )
#  define POSH_LittleS64( x ) POSH_SwapS64( x )
#endif /* POSH_64BIT_INTEGER */

#endif

/** @} */

