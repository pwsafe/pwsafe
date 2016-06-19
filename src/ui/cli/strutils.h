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

#include "../../core/StringX.h"

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


#endif /* defined(__pwsafe_xcode6__strutils__) */
