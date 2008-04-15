/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file XStringStream.h
 *
 * STL-based implementation of secure string streams.
 * typedefs of secure versions of istringstream, ostringstream
 * and stringbuf.
 * Secure in the sense that memory is scrubbed before
 * being returned to system.
 */

#ifndef _XSTRINGSTREAM_H_
#define _XSTRINGSTREAM_H_
#include "XString.h"
#include <sstream>


#ifdef UNICODE
typedef std::basic_stringbuf<wchar_t,
                             std::char_traits<wchar_t>,
                             S_Alloc::SecureAlloc<wchar_t> > XStringBuf;

typedef std::basic_istringstream<wchar_t,
                                 std::char_traits<wchar_t>,
                                 S_Alloc::SecureAlloc<wchar_t> > iXStringStream;

typedef std::basic_ostringstream<wchar_t,
                                 std::char_traits<wchar_t>,
                                 S_Alloc::SecureAlloc<wchar_t> > oXStringStream;

typedef std::basic_stringstream<wchar_t,
                                std::char_traits<wchar_t>,
                                S_Alloc::SecureAlloc<wchar_t> > XStringStream;
#else
typedef std::basic_stringbuf<char,
                             std::char_traits<char>,
                             S_Alloc::SecureAlloc<char> > XStringBuf;

typedef std::basic_istringstream<char,
                                 std::char_traits<char>,
                                 S_Alloc::SecureAlloc<char> > iXStringStream;

typedef std::basic_ostringstream<char,
                                 std::char_traits<char>,
                                 S_Alloc::SecureAlloc<char> > oXStringStream;

typedef std::basic_stringstream<char,
                                std::char_traits<char>,
                                S_Alloc::SecureAlloc<char> > XStringStream;
#endif

#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
