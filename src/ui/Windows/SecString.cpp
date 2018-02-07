/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file SecString.cpp
//-----------------------------------------------------------------------------

#include "SecString.h"
#include "core/Util.h"

#include <cstdarg>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CSecString::trashstring()
{
  trashMemory((unsigned char*)m_mystring.GetBuffer(m_mystring.GetLength()),
    m_mystring.GetLength());
}

void CSecString::Empty()
{
  trashstring();
  m_mystring.Empty();
}

BOOL CSecString::LoadString(const UINT &nID)
{
  return m_mystring.LoadString(nID);
}

void CSecString::Format(LPCWSTR lpszFormat, ... )
{
  va_list args;
  va_start(args, lpszFormat);
  m_mystring.FormatV(lpszFormat, args);
  va_end(args);
}

void CSecString::Format(UINT nID, ... )
{
  va_list args;
  va_start(args, nID);
  CString csFormat(MAKEINTRESOURCE(nID));
  m_mystring.FormatV(csFormat, args);
  va_end(args);
}

CSecString& CSecString::operator=(const CSecString& stringSrc)
{
  if (this != &stringSrc) {
    trashstring();
    m_mystring = stringSrc.m_mystring;
  }
  return *this;
}

const CSecString& CSecString::operator=(wchar_t ch)
{
  trashstring();
  m_mystring = ch;
  return *this;
}

const CSecString& CSecString::operator=(LPCWSTR lpsz)
{
  trashstring();
  m_mystring = lpsz;
  return *this;
}

CSecString operator+(const CSecString& string1,const CSecString& string2)
{
  CSecString s(string1.m_mystring+string2.m_mystring);
  return s;
}

CSecString operator+(const CSecString& string, wchar_t ch)
{
  CSecString s(string.m_mystring + ch);
  return s;
}

CSecString operator+(wchar_t ch, const CSecString& string)
{
  CSecString s(ch + string.m_mystring);
  return s;
}

CSecString operator+(const CSecString& string, LPCWSTR lpsz)
{
  CSecString s(string.m_mystring + lpsz);
  return s;
}

CSecString operator+(LPCWSTR lpsz, const CSecString& string)
{
  CSecString s(lpsz + string.m_mystring);
  return s;
}

//Can't properly trash the memory here, so it is better to just return a CString
CSecString CSecString::Left(int nCount) const
{
  CSecString s;
  s.m_mystring = m_mystring.Left(nCount);
  return s;
}

CSecString CSecString::Right(int nCount) const
{
  CSecString s;
  s.m_mystring = m_mystring.Right(nCount);
  return s;
}

CSecString CSecString::Mid(int nFirst) const
{
  CSecString s;
  s.m_mystring = m_mystring.Mid(nFirst);
  return s;
}

CSecString CSecString::Mid(int nFirst, int nCount) const
{
  CSecString s;
  s.m_mystring = m_mystring.Mid(nFirst, nCount);
  return s;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
