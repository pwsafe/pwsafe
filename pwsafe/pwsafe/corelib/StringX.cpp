/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include <ctype.h>
#include <string.h>
#include "StringX.h"

// A few convenience functions for StringX
// Perhaps change these to member functions in the future?

int CompareNoCase(const StringX &s1, const StringX &s2)
{
  // case insensitive string comparison
  return _tcsicmp(s1.c_str(), s2.c_str());
}

void ToLower(StringX &s)
{
  for (StringX::iterator iter = s.begin(); iter != s.end(); iter++)
    *iter = TCHAR(_totlower(*iter));
}

StringX &Trim(StringX &s, const TCHAR *set)
{
  const TCHAR *ws = _T(" \t\r\n");
  const TCHAR *tset = (set == NULL) ? ws : set;

  StringX::size_type b = s.find_first_not_of(tset);
  if (b == StringX::npos) {
    s.clear();
  } else {
    StringX::size_type e = s.find_last_not_of(tset);
    StringX t(s.begin() + b, s.end() - (s.length() - e) + 1);
    s = t;
  }
  return s;
}

StringX &TrimRight(StringX &s, const TCHAR *set)
{
  const TCHAR *ws = _T(" \t\r\n");
  const TCHAR *tset = (set == NULL) ? ws : set;

  StringX::size_type e = s.find_last_not_of(tset);
  if (e == StringX::npos) {
    s.clear();
  } else {
    StringX t(s.begin(), s.end() - (s.length() - e) + 1);
    s = t;
  }
  return s;
}

StringX &TrimLeft(StringX &s, const TCHAR *set)
{
  const TCHAR *ws = _T(" \t\r\n");
  const TCHAR *tset = (set == NULL) ? ws : set;

  StringX::size_type b = s.find_first_not_of(tset);
  if (b == StringX::npos) {
    s.clear();
  } else {
    StringX t(s.begin() + b, s.end());
    s = t;
  }
  return s;
}

void EmptyIfOnlyWhiteSpace(StringX &s)
{
  const TCHAR *ws = _T(" \t\r\n");
  StringX::size_type b = s.find_first_not_of(ws);
  if (b == StringX::npos)
    s.clear();
}

int Replace(StringX &s, TCHAR from, TCHAR to)
{
  int retval = 0;
  StringX r;
  r.reserve(s.length());
  for (StringX::iterator iter = s.begin(); iter != s.end(); iter++)
    if (*iter == from) {
      r.append(1, to);
      retval++;
    } else
      r.append(1, *iter);
  s = r;
  return retval;
}

int Replace(StringX &s, const StringX &from, const StringX &to)
{
  int retval = 0;
  StringX r;
  StringX::size_type i = 0;
  do {
    StringX::size_type j = s.find(from, i);
    r.append(s, i, j - i);
    if (j != StringX::npos) {
      r.append(to);
      retval++;
      i = j + from.length();
    } else
      i = j;
  } while (i != StringX::npos);
  s = r;
  return retval;
}

int Remove(StringX &s, TCHAR c)
{
  int retval = 0;
  StringX t;
  for (StringX::iterator iter = s.begin(); iter != s.end(); iter++)
    if (*iter != c)
      t += *iter;
    else
      retval++;
  if (retval != 0)
    s = t;
  return retval;
}


#ifdef TEST_TRIM
int main(int argc, char *argv[])
{
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " string-to-trim" << endl;
    exit(1);
  }

  StringX s(argv[1]), sl(argv[1]), sr(argv[1]);
  Trim(s); TrimLeft(sl); TrimRight(sr);
  cout << "Trim(\"" << argv[1] << "\") = \"" << s <<"\"" << endl;
  cout << "TrimLeft(\"" << argv[1] << "\") = \"" << sl <<"\"" << endl;
  cout << "TrimRight(\"" << argv[1] << "\") = \"" << sr <<"\"" << endl;
  return 0;
}
#endif
