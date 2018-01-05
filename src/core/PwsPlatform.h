/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// pws_platform.h
//------------------------------------------------------------------------------
//
// This header file exists to determine, at compile time, which target we're
// compiling to and to make the appropriate #defines and #includes.
//
// The following macros are defined:
//
//    PWS_BIG_ENDIAN    - Defined only if the target CPU is big-endian.
//    PWS_LITTLE_ENDIAN - Defined only if the target CPU is little-endian.
//    PWS_PLATFORM      - A string, the target platform, e.g. "Pocket PC".
//    PWS_PLATFORM_EX   - A string, the target platform, e.g. "Pocket PC 2000".
//    POCKET_PC         - Defined only if target is Pocket PC 2000 or later.
//    POCKET_PC_VER     - Defined only if target is Pocket PC 2000 or later.
//
// Notes:
//
// 1. PWS_BIG_ENDIAN and PWS_LITTLE_ENDIAN are mutually exclusive.
// 2. PWS_BIG_ENDIAN and PWS_LITTLE_ENDIAN may be defined on the complier
//    command line but checks are made to ensure that one and only one is
//    defined.
//
// Supported Configurations:
// -------------------------
//
// Windows
//
//    Win32 X86
//    Win32 and Win64 x64 (unofficial)
//
// Linux
// MacOS (unofficial)
// FreeBSD (unofficial)

#ifndef __PWSPLATFORM_H
#define __PWSPLATFORM_H

#if (defined(_WIN32) || defined (_WIN64))
// Make sure Windows.h does not define min & max macros
#define NOMINMAX
#endif

#if (defined(_WIN32) || defined (_WIN64)) && !defined(__WX__)
// ONLY place in core which refers to parent. Ugh.
#include "../ui/Windows/stdafx.h"
#else
// some globally useful includes for non-Windows
#include <cassert>
#endif

#ifndef _MSC_VER
#include <stdint.h>
#endif

#if defined(__linux__) || defined(__CYGWIN__)
#include <endian.h>
// Following seems needed on Linux/cygwin
#define LTC_NO_ROLC
#endif

#undef PWS_PLATFORM
#undef POCKET_PC

#if defined(_WIN32)
#ifdef BIG_ENDIAN
#define PWS_BIG_ENDIAN
#endif
#ifdef LITTLE_ENDIAN
#define PWS_LITTLE_ENDIAN
#endif
#endif

// PWS_BIG_ENDIAN and PWS_LITTLE_ENDIAN can be specified on the
#if defined(PWS_BIG_ENDIAN)
#undef PWS_BIG_ENDIAN
#define PWS_BIG_ENDIAN
#endif

#if defined(PWS_LITTLE_ENDIAN)
#undef PWS_LITTLE_ENDIAN
#define PWS_LITTLE_ENDIAN
#endif

// **********************************************
// * Windows 32                                 *
// **********************************************
#if defined(_WIN32)
  #if defined(x86) || defined(_x86) || defined(_X86) || defined(_X86_) || defined(_M_IX86) || defined(_M_X64)
    #define PWS_PLATFORM "Windows"
    #define PWS_LITTLE_ENDIAN
  #endif
// **********************************************
// * Linux                                      *
// **********************************************
#elif defined(__linux) || defined(__linux__)
  #define PWS_PLATFORM "Linux"
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define PWS_LITTLE_ENDIAN
  #else
    #define PWS_BIG_ENDIAN
#endif
// **********************************************
// * cygwin on Intel                             *
// **********************************************
#elif defined(__CYGWIN__)
#define PWS_PLATFORM "Cygwin"
#if defined(__i386__) || defined(__i486__)
#define PWS_LITTLE_ENDIAN
#endif
/* http://predef.sourceforge.net/preos.html*/
#elif defined (macintosh) || defined(Macintosh) || defined(__APPLE__) || defined(__MACH__)
#define PWS_PLATFORM "Mac"
#define __PWS_MACINTOSH__
/* following line was added to avoid "Impossible constraint in 'asm'" errors in ROLc()
   and RORc() functions below, just like it is defined for Linux and cygwin above */
#define LTC_NO_ROLC
#if defined(__APPLE__) && defined(__MACH__)
#define PWS_PLATFORM_EX MacOSX
#else
#define PWS_PLATFORM_EX MacOS9
#endif
/* gcc shipped with snow leopard defines this.  Try "cpp -dM dummy.h"*/
#if defined (__LITTLE_ENDIAN__) && (__LITTLE_ENDIAN__ == 1)
#define PWS_LITTLE_ENDIAN
#else
#define PWS_BIG_ENDIAN
#endif
// **********************************************
// * FreeBSD on Intel                           *
// **********************************************
#elif defined(__FreeBSD) || defined(__FreeBSD__)
#define PWS_PLATFORM "FreeBSD"
#define LTC_NO_ROLC
#if defined(__i386__) || defined(__amd64__)
#define PWS_LITTLE_ENDIAN
#endif
// **********************************************
// * Add other platforms here...                *
// **********************************************
#endif

//
#if !defined(PWS_PLATFORM)
#error Unable to determine the target platform - please fix PwsPlatform.h
#endif

#if !defined(PWS_LITTLE_ENDIAN) && !defined(PWS_BIG_ENDIAN)
#error Cannot determine whether the target CPU is big or little endian - please fix PwsPlatform.h
#endif

#if defined(PWS_BIG_ENDIAN) && defined(PWS_LITTLE_ENDIAN)
#error Both PWS_BIG_ENDIAN and PWS_LITTLE_ENDIAN are defined, only one should be defined.
#endif

// Following from libtomcrypt, for twofish & SHA256
/* Controls endianness and size of registers.  Leave uncommented to get platform neutral [slower] code
*
* Note: in order to use the optimized macros your platform must support unaligned 32 and 64 bit read/writes.
* The x86 platforms allow this but some others [ARM for instance] do not.  On those platforms you **MUST**
* use the portable [slower] macros.
*/

/* detect x86-32 machines somewhat */
#if defined(INTEL_CC) || (defined(_MSC_VER) && defined(WIN32)) || (defined(__GNUC__) && (defined(__DJGPP__) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__i386__)))
#define ENDIAN_LITTLE
#define ENDIAN_32BITWORD
#endif

/* detects MIPS R5900 processors (PS2) */
#if (defined(__R5900) || defined(R5900) || defined(__R5900__)) && (defined(_mips) || defined(__mips__) || defined(mips))
#define ENDIAN_LITTLE
#define ENDIAN_64BITWORD
#endif

/* detect amd64 */
#if defined(__x86_64__) || defined(__amd64__) || (defined(_MSC_VER) && defined(WIN64))
#define ENDIAN_LITTLE
#define ENDIAN_64BITWORD
#endif

/* #define ENDIAN_LITTLE */
/* #define ENDIAN_BIG */

/* #define ENDIAN_32BITWORD */
/* #define ENDIAN_64BITWORD */

#if (defined(ENDIAN_BIG) || defined(ENDIAN_LITTLE)) && !(defined(ENDIAN_32BITWORD) || defined(ENDIAN_64BITWORD))
#error You must specify a word size as well as endianness in mycrypt_cfg.h
#endif

#if !(defined(ENDIAN_BIG) || defined(ENDIAN_LITTLE))
#define ENDIAN_NEUTRAL
#endif

/* fix for MSVC ...evil! */
#ifdef _MSC_VER
#define CONST64(n) n ## ui64
typedef unsigned __int64 ulong64;
#else
#define CONST64(n) n ## ULL
typedef uint64_t ulong64;
#endif

/* this is the "32-bit at least" data type
* Re-define it to suit your platform but it must be at least 32-bits
*/
#if defined(__x86_64__)
typedef unsigned ulong32;
#else
typedef unsigned long ulong32;
#endif

/* ---- HELPER MACROS ---- */
#ifdef ENDIAN_NEUTRAL

#define STORE32L(x, y)                                                                     \
{ (y)[3] = static_cast<unsigned char>(((x)>>24)&255); (y)[2] = static_cast<unsigned char>(((x)>>16)&255);   \
  (y)[1] = static_cast<unsigned char>(((x)>>8)&255); (y)[0] = static_cast<unsigned char>((x)&255); }

#define LOAD32L(x, y)                            \
{ x = (static_cast<unsigned long>((y)[3] & 255)<<24) | \
  (static_cast<unsigned long>((y)[2] & 255)<<16) | \
  (static_cast<unsigned long>((y)[1] & 255)<<8)  | \
  (static_cast<unsigned long>((y)[0] & 255)); }

#define STORE64L(x, y)                                                                     \
{ (y)[7] = static_cast<unsigned char>(((x)>>56)&255); (y)[6] = static_cast<unsigned char>(((x)>>48)&255);   \
  (y)[5] = static_cast<unsigned char>(((x)>>40)&255); (y)[4] = static_cast<unsigned char>(((x)>>32)&255);   \
  (y)[3] = static_cast<unsigned char>(((x)>>24)&255); (y)[2] = static_cast<unsigned char>(((x)>>16)&255);   \
  (y)[1] = static_cast<unsigned char>(((x)>>8)&255); (y)[0] = static_cast<unsigned char>((x)&255); }

#define LOAD64L(x, y)                                                       \
{ x = ((static_cast<ulong64>((y)[7] & 255))<<56)|((static_cast<ulong64>((y)[6] & 255))<<48)| \
  ((static_cast<ulong64>((y)[5] & 255))<<40)|((static_cast<ulong64>((y)[4] & 255))<<32)| \
  ((static_cast<ulong64>((y)[3] & 255))<<24)|((static_cast<ulong64>((y)[2] & 255))<<16)| \
  ((static_cast<ulong64>((y)[1] & 255))<<8)|((static_cast<ulong64>((y)[0] & 255))); }

#define STORE32H(x, y)                                                                     \
{ (y)[0] = static_cast<unsigned char>(((x)>>24)&255); (y)[1] = static_cast<unsigned char>(((x)>>16)&255);   \
  (y)[2] = static_cast<unsigned char>(((x)>>8)&255); (y)[3] = static_cast<unsigned char>((x)&255); }

#define LOAD32H(x, y)                            \
{ x = (static_cast<unsigned long>((y)[0] & 255)<<24) | \
  (static_cast<unsigned long>((y)[1] & 255)<<16) | \
  (static_cast<unsigned long>((y)[2] & 255)<<8)  | \
  (static_cast<unsigned long>((y)[3] & 255)); }

#define STORE64H(x, y)                                                                     \
{ (y)[0] = static_cast<unsigned char>(((x)>>56)&255); (y)[1] = static_cast<unsigned char>(((x)>>48)&255);     \
  (y)[2] = static_cast<unsigned char>(((x)>>40)&255); (y)[3] = static_cast<unsigned char>(((x)>>32)&255);     \
  (y)[4] = static_cast<unsigned char>(((x)>>24)&255); (y)[5] = static_cast<unsigned char>(((x)>>16)&255);     \
  (y)[6] = static_cast<unsigned char>(((x)>>8)&255); (y)[7] = static_cast<unsigned char>((x)&255); }

#define LOAD64H(x, y)                                                      \
{ x = ((static_cast<ulong64>((y)[0] & 255))<<56)|((static_cast<ulong64>((y)[1] & 255))<<48) | \
  ((static_cast<ulong64>((y)[2] & 255))<<40)|((static_cast<ulong64>((y)[3] & 255))<<32) | \
  ((static_cast<ulong64>((y)[4] & 255))<<24)|((static_cast<ulong64>((y)[5] & 255))<<16) | \
  ((static_cast<ulong64>((y)[6] & 255))<<8)|((static_cast<ulong64>((y)[7] & 255))); }

#endif /* ENDIAN_NEUTRAL */

#ifdef ENDIAN_LITTLE

#define STORE32H(x, y)                                                                     \
{ (y)[0] = static_cast<unsigned char>(((x)>>24)&255); (y)[1] = static_cast<unsigned char>(((x)>>16)&255);   \
  (y)[2] = static_cast<unsigned char>(((x)>>8)&255); (y)[3] = static_cast<unsigned char>((x)&255); }

#define LOAD32H(x, y)                            \
{ x = (static_cast<unsigned long>((y)[0] & 255)<<24) | \
  (static_cast<unsigned long>((y)[1] & 255)<<16) | \
  (static_cast<unsigned long>((y)[2] & 255)<<8)  | \
  (static_cast<unsigned long>((y)[3] & 255)); }

#define STORE64H(x, y)                                                                     \
{ (y)[0] = static_cast<unsigned char>(((x)>>56)&255); (y)[1] = static_cast<unsigned char>(((x)>>48)&255);     \
  (y)[2] = static_cast<unsigned char>(((x)>>40)&255); (y)[3] = static_cast<unsigned char>(((x)>>32)&255);     \
  (y)[4] = static_cast<unsigned char>(((x)>>24)&255); (y)[5] = static_cast<unsigned char>(((x)>>16)&255);     \
  (y)[6] = static_cast<unsigned char>(((x)>>8)&255); (y)[7] = static_cast<unsigned char>((x)&255); }

#define LOAD64H(x, y)                                                      \
{ x = ((static_cast<ulong64>((y)[0] & 255))<<56)|((static_cast<ulong64>((y)[1] & 255))<<48) | \
  ((static_cast<ulong64>((y)[2] & 255))<<40)|((static_cast<ulong64>((y)[3] & 255))<<32) | \
  ((static_cast<ulong64>((y)[4] & 255))<<24)|((static_cast<ulong64>((y)[5] & 255))<<16) | \
  ((static_cast<ulong64>((y)[6] & 255))<<8)|((static_cast<ulong64>((y)[7] & 255))); }

#ifdef ENDIAN_32BITWORD

#define STORE32L(x, y)        \
{ unsigned long __t = (x); memcpy(y, &__t, 4); }

#define LOAD32L(x, y)         \
  memcpy(&(x), y, 4);

#define STORE64L(x, y)                                                                     \
{ (y)[7] = static_cast<unsigned char>(((x)>>56)&255); (y)[6] = static_cast<unsigned char>(((x)>>48)&255);   \
  (y)[5] = static_cast<unsigned char>(((x)>>40)&255); (y)[4] = static_cast<unsigned char>(((x)>>32)&255);   \
  (y)[3] = static_cast<unsigned char>(((x)>>24)&255); (y)[2] = static_cast<unsigned char>(((x)>>16)&255);   \
  (y)[1] = static_cast<unsigned char>(((x)>>8)&255); (y)[0] = static_cast<unsigned char>((x)&255); }

#define LOAD64L(x, y)                                                       \
{ x = ((static_cast<ulong64>((y)[7] & 255))<<56)|((static_cast<ulong64>((y)[6] & 255))<<48)| \
  ((static_cast<ulong64>((y)[5] & 255))<<40)|((static_cast<ulong64>((y)[4] & 255))<<32)| \
  ((static_cast<ulong64>((y)[3] & 255))<<24)|((static_cast<ulong64>((y)[2] & 255))<<16)| \
  ((static_cast<ulong64>((y)[1] & 255))<<8)|((static_cast<ulong64>((y)[0] & 255))); }

#else /* 64-bit words then  */

#define STORE32L(x, y)        \
{ unsigned long __t = (x); memcpy(y, &__t, 4); }

#define LOAD32L(x, y)         \
{ memcpy(&(x), y, 4); x &= 0xFFFFFFFF; }

#define STORE64L(x, y)        \
{ ulong64 __t = (x); memcpy(y, &__t, 8); }

#define LOAD64L(x, y)         \
{ memcpy(&(x), y, 8); }

#endif /* ENDIAN_64BITWORD */

#endif /* ENDIAN_LITTLE */

#ifdef ENDIAN_BIG
#define STORE32L(x, y)                                                                     \
{ (y)[3] = static_cast<unsigned char>(((x)>>24)&255); (y)[2] = static_cast<unsigned char>(((x)>>16)&255);   \
  (y)[1] = static_cast<unsigned char>(((x)>>8)&255); (y)[0] = static_cast<unsigned char>((x)&255); }

#define LOAD32L(x, y)                            \
{ x = (static_cast<unsigned long>((y)[3] & 255)<<24) | \
  (static_cast<unsigned long>((y)[2] & 255)<<16) | \
  (static_cast<unsigned long>((y)[1] & 255)<<8)  | \
  (static_cast<unsigned long>((y)[0] & 255)); }

#define STORE64L(x, y)                                                                     \
{ (y)[7] = static_cast<unsigned char>(((x)>>56)&255); (y)[6] = static_cast<unsigned char>(((x)>>48)&255);     \
  (y)[5] = static_cast<unsigned char>(((x)>>40)&255); (y)[4] = static_cast<unsigned char>(((x)>>32)&255);     \
  (y)[3] = static_cast<unsigned char>(((x)>>24)&255); (y)[2] = static_cast<unsigned char>(((x)>>16)&255);     \
  (y)[1] = static_cast<unsigned char>(((x)>>8)&255); (y)[0] = static_cast<unsigned char>((x)&255); }

#define LOAD64L(x, y)                                                      \
{ x = ((static_cast<ulong64>((y)[7] & 255))<<56)|((static_cast<ulong64>((y)[6] & 255))<<48) | \
  ((static_cast<ulong64>((y)[5] & 255))<<40)|((static_cast<ulong64>((y)[4] & 255))<<32) | \
  ((static_cast<ulong64>((y)[3] & 255))<<24)|((static_cast<ulong64>((y)[2] & 255))<<16) | \
  ((static_cast<ulong64>((y)[1] & 255))<<8)|((static_cast<ulong64>((y)[0] & 255))); }

#ifdef ENDIAN_32BITWORD

#define STORE32H(x, y)        \
{ unsigned long __t = (x); memcpy(y, &__t, 4); }

#define LOAD32H(x, y)         \
  memcpy(&(x), y, 4);

#define STORE64H(x, y)                                                                     \
{ (y)[0] = static_cast<unsigned char>(((x)>>56)&255); (y)[1] = static_cast<unsigned char>(((x)>>48)&255);   \
  (y)[2] = static_cast<unsigned char>(((x)>>40)&255); (y)[3] = static_cast<unsigned char>(((x)>>32)&255);   \
  (y)[4] = static_cast<unsigned char>(((x)>>24)&255); (y)[5] = static_cast<unsigned char>(((x)>>16)&255);   \
  (y)[6] = static_cast<unsigned char>(((x)>>8)&255);  (y)[7] = static_cast<unsigned char>((x)&255); }

#define LOAD64H(x, y)                                                       \
{ x = ((static_cast<ulong64>((y)[0] & 255))<<56)|((static_cast<ulong64>((y)[1] & 255))<<48)| \
  ((static_cast<ulong64>((y)[2] & 255))<<40)|((static_cast<ulong64>((y)[3] & 255))<<32)| \
  ((static_cast<ulong64>((y)[4] & 255))<<24)|((static_cast<ulong64>((y)[5] & 255))<<16)| \
  ((static_cast<ulong64>((y)[6] & 255))<<8)| ((static_cast<ulong64>((y)[7] & 255))); }

#else /* 64-bit words then  */

#define STORE32H(x, y)        \
{ unsigned long __t = (x); memcpy(y, &__t, 4); }

#define LOAD32H(x, y)         \
{ memcpy(&(x), y, 4); x &= 0xFFFFFFFF; }

#define STORE64H(x, y)        \
{ ulong64 __t = (x); memcpy(y, &__t, 8); }

#define LOAD64H(x, y)         \
{ memcpy(&(x), y, 8); }

#endif /* ENDIAN_64BITWORD */
#endif /* ENDIAN_BIG */

#define BSWAP(x)  ( ((x>>24)&0x000000FFUL) | ((x<<24)&0xFF000000UL)  | \
  ((x>>8)&0x0000FF00UL)  | ((x<<8)&0x00FF0000UL) )

/* 32-bit Rotates */
#if defined(_MSC_VER)

/* instrinsic rotate */
#include <stdlib.h>
#pragma intrinsic(_lrotr,_lrotl)
#define ROR(x,n) _lrotr(x,n)
#define ROL(x,n) _lrotl(x,n)
#define RORc(x,n) _lrotr(x,n)
#define ROLc(x,n) _lrotl(x,n)

#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)) && !defined(INTEL_CC) && !defined(LTC_NO_ASM)

static inline unsigned ROL(unsigned word, int i)
{
  asm ("roll %%cl,%0"
    :"=r" (word)
    :"0" (word),"c" (i));
  return word;
}

static inline unsigned ROR(unsigned word, int i)
{
  asm ("rorl %%cl,%0"
    :"=r" (word)
    :"0" (word),"c" (i));
  return word;
}

#ifndef LTC_NO_ROLC

static inline unsigned ROLc(unsigned word, const int i)
{
  asm ("roll %2,%0"
    :"=r" (word)
    :"0" (word),"I" (i));
  return word;
}

static inline unsigned RORc(unsigned word, const int i)
{
  asm ("rorl %2,%0"
    :"=r" (word)
    :"0" (word),"I" (i));
  return word;
}

#else

#define ROLc ROL
#define RORc ROR

#endif

#else

/* rotates the hard way */
#define ROL(x, y) ( ((static_cast<unsigned long>(x)<<static_cast<unsigned long>((y)&31)) | ((static_cast<unsigned long>(x)&0xFFFFFFFFUL)>>static_cast<unsigned long>(32-((y)&31)))) & 0xFFFFFFFFUL)
#define ROR(x, y) ( (((static_cast<unsigned long>(x)&0xFFFFFFFFUL)>>static_cast<unsigned long>((y)&31)) | (static_cast<unsigned long>(x)<<static_cast<unsigned long>(32-((y)&31)))) & 0xFFFFFFFFUL)
#define ROLc(x, y) ( ((static_cast<unsigned long>(x)<<static_cast<unsigned long>((y)&31)) | ((static_cast<unsigned long>(x)&0xFFFFFFFFUL)>>static_cast<unsigned long>(32-((y)&31)))) & 0xFFFFFFFFUL)
#define RORc(x, y) ( (((static_cast<unsigned long>(x)&0xFFFFFFFFUL)>>static_cast<unsigned long>((y)&31)) | (static_cast<unsigned long>(x)<<static_cast<unsigned long>(32-((y)&31)))) & 0xFFFFFFFFUL)

#endif

/* 64-bit Rotates */
#if defined(__GNUC__) && defined(__x86_64__) && !defined(LTC_NO_ASM)

static inline unsigned long ROL64(unsigned long word, int i)
{
  asm("rolq %%cl,%0"
    :"=r" (word)
    :"0" (word),"c" (i));
  return word;
}

static inline unsigned long ROR64(unsigned long word, int i)
{
  asm("rorq %%cl,%0"
    :"=r" (word)
    :"0" (word),"c" (i));
  return word;
}

#ifndef LTC_NO_ROLC

static inline unsigned long ROL64c(unsigned long word, const int i)
{
  asm("rolq %2,%0"
    :"=r" (word)
    :"0" (word),"J" (i));
  return word;
}

static inline unsigned long ROR64c(unsigned long word, const int i)
{
  asm("rorq %2,%0"
    :"=r" (word)
    :"0" (word),"J" (i));
  return word;
}

#else /* LTC_NO_ROLC */

#define ROL64c ROL
#define ROR64c ROR

#endif

#else /* Not x86_64  */

#define ROL64(x, y) \
  ( (((x)<<(static_cast<ulong64>(y)&63)) | \
  (((x)&CONST64(0xFFFFFFFFFFFFFFFF))>>(static_cast<ulong64>64-((y)&63)))) & CONST64(0xFFFFFFFFFFFFFFFF))

#define ROR64(x, y) \
  ( ((((x)&CONST64(0xFFFFFFFFFFFFFFFF))>>(static_cast<ulong64>(y)&CONST64(63))) | \
  ((x)<<(static_cast<ulong64>(64-((y)&CONST64(63)))))) & CONST64(0xFFFFFFFFFFFFFFFF))

#define ROL64c(x, y) \
  ( (((x)<<(static_cast<ulong64>(y)&63)) | \
  (((x)&CONST64(0xFFFFFFFFFFFFFFFF))>>(static_cast<ulong64>64-((y)&63)))) & CONST64(0xFFFFFFFFFFFFFFFF))

#define ROR64c(x, y) \
  ( ((((x)&CONST64(0xFFFFFFFFFFFFFFFF))>>(static_cast<ulong64>(y)&CONST64(63))) | \
  ((x)<<(static_cast<ulong64>(64-((y)&CONST64(63)))))) & CONST64(0xFFFFFFFFFFFFFFFF))

#endif

/* extract a byte portably */
#ifdef _MSC_VER
#define byte(x, n) (static_cast<unsigned char>((x) >> (8 * (n))))
#else
#define byte(x, n) (((x) >> (8 * (n))) & 255)
#endif

#define NumberOf(array) ((sizeof array) / sizeof(array[0]))

#if !defined(_MFC_VER) && !defined(_WIN32)
#define UNREFERENCED_PARAMETER(P) (void)(P)
#endif

#endif /* __PWSPLATFORM_H */
