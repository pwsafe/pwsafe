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

#ifndef _STRINGXSTREAM_H_
#define _STRINGXSTREAM_H_
#include "StringX.h"
#include <sstream>

// stringstream typedefs for StringX 
#ifdef UNICODE
typedef std::basic_stringbuf<wchar_t,
                             std::char_traits<wchar_t>,
                             S_Alloc::SecureAlloc<wchar_t> > StringXBuf;

typedef std::basic_istringstream<wchar_t,
                                 std::char_traits<wchar_t>,
                                 S_Alloc::SecureAlloc<wchar_t> > iStringXStream;

typedef std::basic_ostringstream<wchar_t,
                                 std::char_traits<wchar_t>,
                                 S_Alloc::SecureAlloc<wchar_t> > oStringXStream;

typedef std::basic_stringstream<wchar_t,
                                std::char_traits<wchar_t>,
                                S_Alloc::SecureAlloc<wchar_t> > StringXStream;
#else
typedef std::basic_stringbuf<char,
                             std::char_traits<char>,
                             S_Alloc::SecureAlloc<char> > StringXBuf;

typedef std::basic_istringstream<char,
                                 std::char_traits<char>,
                                 S_Alloc::SecureAlloc<char> > iStringXStream;

typedef std::basic_ostringstream<char,
                                 std::char_traits<char>,
                                 S_Alloc::SecureAlloc<char> > oStringXStream;

typedef std::basic_stringstream<char,
                                std::char_traits<char>,
                                S_Alloc::SecureAlloc<char> > StringXStream;
#endif

// Following not related to StringX, but putting it here
// is the lesser of two evils (other is creating a new file
// just for this)
// hide w_char/char differences where possible:
#ifdef UNICODE
typedef std::wistringstream istringstreamT;
typedef std::wostringstream ostringstreamT;
#else
typedef std::istringstream istringstreamT;
typedef std::ostringstream ostringstreamT;
#endif

#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
