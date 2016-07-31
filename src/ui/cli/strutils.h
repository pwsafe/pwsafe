//
//  strutils.h
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

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

template <class string_t>
inline string_t trimleft(const string_t &s)
{
    const typename string_t::value_type space = L' ';
    const auto nonspace = s.find_first_not_of(space);
    return nonspace == string_t::npos? string_t{}: string_t{s, nonspace};
}

template <class string_t>
inline string_t trimright(const string_t &s)
{
    const typename string_t::value_type space = L' ';
    const auto last = s.find_last_not_of(space);
    return string_t{s, 0, last == string_t::npos? 0: last+1};
}

template <class string_t>
inline string_t trim(const string_t &s)
{
    return trimright(trimleft(s));
}

#endif /* defined(__pwsafe_xcode6__strutils__) */
