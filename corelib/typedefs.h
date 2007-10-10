/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#ifndef _TYPEDEFS_H
#ifdef _WIN32
//Some extra typedefs -- I'm addicted to typedefs
typedef char    int8;
typedef short   int16;
typedef int     int32;
typedef __int64 int64;

typedef unsigned char    uint8;
typedef unsigned short   uint16;
typedef unsigned int     uint32;
typedef unsigned __int64 uint64;

typedef unsigned __int64   ulong64;
typedef unsigned long      ulong32;
#else
#include <sys/types.h>
typedef int8_t	int8;
typedef int16_t	int16;
typedef int32_t	int32;
typedef int64_t	int64;

typedef u_int8_t	uint8;
typedef u_int16_t	uint16;
typedef u_int32_t	uint32;
typedef u_int64_t	uint64;
#endif
#endif // _TYPEDEFS_H
