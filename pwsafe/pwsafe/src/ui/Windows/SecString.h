/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

// SecString.h
// A drop-in replacement for CString, the main difference being that the
// data is scrubbed by trashstring() in the destructor, thus leaving an attacker
// with a little less info to grovel for in the swap file / core dump / whatever
//
// Note that CSecString should ONLY be used for dialog box class member variables
// that are mapped to Edit/Text controls. All other secure strings should
// be of class StringX, defined and implemented in corelib (for portability).
//

//-----------------------------------------------------------------------------

#ifndef _SECSTRING_H_
#define _SECSTRING_H_
#ifdef _WIN32
#include <afx.h>
#endif
#include "corelib/StringX.h"
#include "os/typedefs.h"
//-----------------------------------------------------------------------------
class CSecString
{
public:
  CSecString() : m_mystring(_S("")) {}
  CSecString(LPCTSTR lpsz) : m_mystring(lpsz) {}
  CSecString(LPCTSTR lpsz, int nLength) : m_mystring(lpsz, nLength) {}
  CSecString(const CSecString& stringSrc) : m_mystring(stringSrc.m_mystring) {}
  CSecString(const CString& stringSrc) : m_mystring(stringSrc) {}
  CSecString(const StringX& sx) : m_mystring(sx.c_str()) {}

  ~CSecString() {trashstring();}

  TCHAR operator[](int nIndex) const {return m_mystring[nIndex];}

  // Following are dependent on M'soft's CString
  // We'll keep them inline for performance/code size, at the cost of ifdefs
  // in the header...
#ifdef _WIN32
  TCHAR GetAt(int nIndex) {return m_mystring.GetAt(nIndex);}
  void SetAt(int nIndex, TCHAR ch) {m_mystring.SetAt(nIndex,ch);}
  operator LPCTSTR() const {return (LPCTSTR)m_mystring;}
  operator StringX() const {return StringX((LPCTSTR)m_mystring);}
  BOOL IsEmpty() const {return m_mystring.IsEmpty();}
  LPTSTR GetBuffer(int nMinBufLength) {return m_mystring.GetBuffer(nMinBufLength);}
  LPTSTR GetBuffer() {return m_mystring.GetBuffer();}

  void ReleaseBuffer(int nNewLength = -1) {m_mystring.ReleaseBuffer(nNewLength);}

  int GetLength() const {return m_mystring.GetLength();}
  int Find(TCHAR ch) const {return m_mystring.Find(ch);}
  int Find(LPCTSTR lpszSub) const {return m_mystring.Find(lpszSub);}
  int Find(TCHAR ch, int nstart) const {return m_mystring.Find(ch, nstart);}
  int Find(LPCTSTR lpszSub, int nstart) const {return m_mystring.Find(lpszSub, nstart);}
  int FindOneOf(LPCTSTR lpszSub) const {return m_mystring.FindOneOf(lpszSub);}
  int Replace(const TCHAR chOld, const TCHAR chNew) {return m_mystring.Replace(chOld,chNew);}
  int Replace(const LPCTSTR lpszOld, const LPCTSTR lpszNew)
  {return m_mystring.Replace(lpszOld,lpszNew);}
  int Remove(TCHAR ch) {return m_mystring.Remove(ch);}
  void TrimRight() {m_mystring.TrimRight();}
  void TrimLeft() {m_mystring.TrimLeft();}
#if _MSC_VER >= 1400
  CSecString &Trim() {m_mystring.Trim(); return *this;}
#else
  CSecString &Trim()
  {m_mystring.TrimLeft(); m_mystring.TrimRight(); return *this}
#endif
  void MakeLower() {m_mystring.MakeLower();}
  void MakeUpper() {m_mystring.MakeUpper();}
  int Compare(const LPCTSTR lpszOther) const {return m_mystring.Compare(lpszOther);}
  int CompareNoCase(const LPCTSTR lpszOther) const {return m_mystring.CompareNoCase(lpszOther);}
  BOOL IsOnlyWhiteSpace() const
  {CSecString t(*this); return t.Trim().IsEmpty();}
  void EmptyIfOnlyWhiteSpace()
  {if (IsOnlyWhiteSpace() == TRUE) Empty();}
  CSecString SpanIncluding(LPCTSTR lpszCharSet)
  {return CSecString(m_mystring.SpanIncluding(lpszCharSet));}
  CSecString SpanExcluding(LPCTSTR lpszCharSet)
  {return CSecString(m_mystring.SpanExcluding(lpszCharSet));}
#else
  TCHAR GetAt(int nIndex);
  void SetAt(int nIndex, TCHAR ch);
  operator LPCTSTR() const;
  BOOL IsEmpty() const;
  LPTSTR GetBuffer(int nMinBufLength);
  LPTSTR GetBuffer();
  void ReleaseBuffer(int nNewLength = -1);
  int GetLength() const;
  int Find(TCHAR ch) const;
  int Find(LPCTSTR lpszSub) const;
  int Find(TCHAR ch, int nstart) const;
  int Find(LPCTSTR lpszSub, int nstart) const;
  int FindOneOf(LPCTSTR lpszSub) const;
  int Replace(const TCHAR chOld, const TCHAR chNew);
  int Replace(const LPCTSTR lpszOld, const LPCTSTR lpszNew);
  int Remove(TCHAR ch);
  void TrimRight();
  void TrimLeft();
  CSecString &Trim();
  void MakeLower();
  int Compare(const LPCTSTR lpszOther) const;
  int CompareNoCase(const LPCTSTR lpszOther) const;
  BOOL IsOnlyWhiteSpace() const;
  void EmptyIfOnlyWhiteSpace();
  CSecString SpanIncluding(LPCTSTR lpszCharSet);
  CSecString SpanExcluding(LPCTSTR lpszCharSet);
#endif

  operator CString() const {return m_mystring;}
  operator CString&() {return m_mystring;}

  const CSecString& operator=(const CSecString& stringSrc);
  const CSecString& operator=(TCHAR ch);
  const CSecString& operator=(LPCTSTR lpsz);
#ifndef UNICODE // do we need this at all?
  const CSecString& operator=(const unsigned char* psz);
#endif
  const CSecString& operator+=(const CSecString& s) {m_mystring += s.m_mystring; return *this;}
  const CSecString& operator+=(TCHAR ch) {m_mystring += ch; return *this;}
  const CSecString& operator+=(LPCTSTR lpsz) {m_mystring += lpsz; return *this;}

  // CSecString operator+(LPCTSTR lpsz);

  friend CSecString operator+(const CSecString& string1,
    const CSecString& string2);
  friend CSecString operator+(const CSecString& string,
    TCHAR ch);
  friend CSecString operator+(TCHAR ch,
    const CSecString& string);
  friend CSecString operator+(const CSecString& string,
    LPCTSTR lpsz);
  friend CSecString operator+(LPCTSTR lpsz,
    const CSecString& string);


  CSecString Left(int nCount) const;
  CSecString Right(int nCount) const;
  CSecString Mid(int nFirst) const;
  CSecString Mid(int nFirst, int nCount) const;
  void Empty();
  BOOL LoadString(const UINT &nID);
  void Format(LPCTSTR lpszFormat, ... );
  void Format(UINT nID, ... );
  void Trash() {trashstring();}

private:
  CString m_mystring;
  //  StringX m_stringX;
  void trashstring();
};
//-----------------------------------------------------------------------------

inline bool operator==(const CSecString& s1, const CSecString& s2)
{return (const CString)s1 == (const CString)s2;}
inline bool operator==(const CSecString& s1, LPCTSTR s2)
{return (const CString)s1==s2;}
inline bool operator==(LPCTSTR s1, const CSecString& s2)
{return s1==(const CString)s2;}
inline bool operator!=(const CSecString& s1, const CSecString& s2)
{return (const CString)s1 != (const CString)s2;}
inline bool operator!=(const CSecString& s1, LPCTSTR s2)
{return (const CString)s1 != s2;}
inline bool operator!=(LPCTSTR s1, const CSecString& s2)
{return s1 != (const CString)s2;}

//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

