//
//  strutils.cpp
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#include <string>
#include <iostream>

#include "strutils.h"

#include "../../core/UTF8Conv.h"
#include "../../core/StringX.h"

using namespace std;

void Utf82StringX(const char* str, StringX& sname)
{
  CUTF8Conv conv;
  if (!conv.FromUTF8((const unsigned char *)str, strlen(str),
                     sname)) {
    cerr << "Could not convert " << str << " to StringX" << endl;
    exit(2);
  }
}

wstring Utf82wstring(const char* utf8str)
{
  StringX sx;
  Utf82StringX(utf8str, sx);
  return stringx2std(sx);
}
