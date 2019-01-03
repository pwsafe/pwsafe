/*
 * Created by Saurav Ghosh on 19/06/16.
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef __pwsafe_xcode6__strutils__
#define __pwsafe_xcode6__strutils__

#include <iosfwd>
#include <sstream>
#include <regex>

#include "../../core/StringX.h"

struct st_GroupTitleUser;

template <typename IntType>
std::string tostr(IntType i)
{
  std::ostringstream os;
  os << i;
  return os.str();
}

inline std::ostream& operator<<(std::ostream& os, const StringX& str)
{
  return os << str.c_str();
}

inline std::ostream& operator<<(std::ostream& os, const std::wstring& str)
{
  return os << str.c_str();
}

void Utf82StringX(const char* filename, StringX& sname);

std::wstring Utf82wstring(const char* utf8str);

template <class CallbackType>
void Split(const std::wstring &str, const std::wstring &sep, CallbackType cb)
{
  // we have to create a temp variable like this, or else it crashes in Xcode 6
  std::wregex r(sep);
  std::wsregex_token_iterator pos(str.cbegin(), str.cend(), r, -1);
  std::wsregex_token_iterator end;
  for_each( pos, end, [&cb](const std::wstring &token) { if (!token.empty()) cb(token);} );
}

const char *status_text(int status);

std::wostream & operator<<( std::wostream &os, const st_GroupTitleUser &gtu);

#endif /* defined(__pwsafe_xcode6__strutils__) */
