// pws_platform.h
//------------------------------------------------------------------------------
//
// This header file exists to determine, at compile time, which target we're
// compiling to and to make the appropriate #defines and #includes.
//
// The following macros are defined:
//
//    BIG_ENDIAN       - Defined only if the target CPU is big-endian.
//    LITTLE_ENDIAN    - Defined only if the target CPU is little-endian.
//    PWS_PLATFORM     - A string, the target platform, e.g. "Pocket PC".
//    PWS_PLATFORM_EX  - A string, the target platform, e.g. "Pocket PC 2000".
//    POCKET_PC        - Defined only if target is Pocket PC 2000 or later.
//    POCKET_PC_VER    - Defined only if target is Pocket PC 2000 or later.
//
// Notes:
//
// 1. BIG_ENDIAN and LITTLE_ENDIAN are mutually exclusive.
// 2. BIG_ENDIAN and LITTLE_ENDIAN may be defined on the complier command line
//    but checks are made to ensure that one and only one is defined.
//
// Supported Configurations:
// -------------------------
//
// Pocket PC:
//
//    2000 ARM
//    2000 MIPS
//    2000 SH3
//    2000 x86 Emulator
//    2002 ARM
//    2002 x86 Emulator
//    2003 ARM (untested)
//    2003 x86 Emulator (untested)
//
// Windows
//
//    Win32 X86

#ifndef PwsPlatform_h
#define PwsPlatform_h

// WIN32_PLATFORM_WFSP
// PPC2003 don't have SHELL_AYGSHELL
#ifndef SHELL_AYGSHELL
//#error
#else
#endif

#undef PWS_PLATFORM
#undef POCKET_PC

//#define POCKET_PC
//#define PWS_PLATFORM	"Windows Mobile 2005"
//#define LITTLE_ENDIAN


// BIG_ENDIAN and LITTLE_ENDIAN can be specified on the 
#if defined(BIG_ENDIAN)
  #undef BIG_ENDIAN
  #define BIG_ENDIAN
#endif

#if defined(LITTLE_ENDIAN)
  #undef LITTLE_ENDIAN
  #define LITTLE_ENDIAN
#endif

#if defined(_WIN32_WCE_PSPC)
  // **********************************************
  // * Pocket PC 2000                             *
  // **********************************************
  #if (WIN32_PLATFORM_PSPC == 1)
    #define PWS_PLATFORM	"Pocket PC"
    #define PWS_PLATFORM_EX	"Pocket PC 2000"
    #define POCKET_PC_VER	2000
    #define POCKET_PC
    #if !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
      #if defined(ARM) || defined(_ARM)
        #define LITTLE_ENDIAN
      #elif defined(MIPS) || defined(_MIPS)
        #define BIG_ENDIAN
      #elif defined(SH3) || defined(_SH3)
        #define BIG_ENDIAN
      #elif defined(x86) || defined(_x86) || defined(_X86) || defined(_X86_)
        #define LITTLE_ENDIAN
      #endif
    #endif
  // **********************************************
  // * Pocket PC 2002 and later                   *
  // **********************************************
  #elif (WIN32_PLATFORM_PSPC >= 310)
    #define PWS_PLATFORM	"Pocket PC"
    #define PWS_PLATFORM_EX	"Pocket PC 2002"
    #define POCKET_PC_VER	2002
    #define POCKET_PC
    #if !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
      #if defined(ARM) || defined(_ARM)
        #define LITTLE_ENDIAN
      #elif defined(x86) || defined(_x86) || defined(_X86) || defined(_X86_)
        #define LITTLE_ENDIAN
      #endif
    #endif
  #else
    #error Only Pocket PC 2000 and later are supported
  #endif

  #if defined(_MSC_VER) && (_MSC_VER >= 1400)
    #define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA
  #endif
  #ifdef _WIN32_WCE
    #define WINVER _WIN32_WCE
  #endif
// **********************************************
// * Windows 32                                 *
// **********************************************
#elif defined(_WIN32)
  #if defined(x86) || defined(_x86) || defined(_X86) || defined(_X86_)
    #define PWS_PLATFORM	"Windows"
    #define LITTLE_ENDIAN
  #endif
// **********************************************
// * Add other platforms here...                *
// **********************************************
#endif

// 
#if !defined(PWS_PLATFORM)
  //#error Unable to determine the target platform - please fix PwsPlatform.h
#endif


#if !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
  #error Cannot determine whether the target CPU is big or little endian - please fix PwsPlatform.h
#endif

#if defined(BIG_ENDIAN) && defined(LITTLE_ENDIAN)
  #error Both BIG_ENDIAN and LITTLE_ENDIAN are defined, only one should be defined.
#endif

#endif

#if defined(_WIN32)
  #include "../stdafx.h"
#endif
