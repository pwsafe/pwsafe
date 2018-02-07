/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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
// be of class StringX, defined and implemented in core (for portability).
//

//-----------------------------------------------------------------------------

#ifndef __SECSTRING_H
#define __SECSTRING_H

#ifdef _WIN32
#include <afx.h>
#endif

#include "core/StringX.h"
#include "os/typedefs.h"

//-----------------------------------------------------------------------------
class CSecString
{
public:
  CSecString() : m_mystring(_S("")) {}
  CSecString(LPCWSTR lpsz) : m_mystring(lpsz) {}
  CSecString(LPCWSTR lpsz, int nLength) : m_mystring(lpsz, nLength) {}
  CSecString(const CSecString& stringSrc) : m_mystring(stringSrc.m_mystring) {}
  CSecString(const CString& stringSrc) : m_mystring(stringSrc) {}
  CSecString(const StringX& sx) : m_mystring(sx.c_str()) {}
  CSecString(const std::wstring& st) : m_mystring(st.c_str()) {}

  ~CSecString() {trashstring();}

  wchar_t operator[](int nIndex) const {return m_mystring[nIndex];}

  // Following are dependent on M'soft's CString
  // We'll keep them inline for performance/code size, at the cost of ifdefs
  // in the header...
#ifdef _WIN32
  wchar_t GetAt(int nIndex) {return m_mystring.GetAt(nIndex);}
  void SetAt(int nIndex, wchar_t ch) {m_mystring.SetAt(nIndex,ch);}
  operator LPCWSTR() const {return static_cast<LPCWSTR>(m_mystring);}
  operator StringX() const {return static_cast<LPCWSTR>(*this);}
  operator std::wstring() const {return static_cast<LPCWSTR>(*this);}
  BOOL IsEmpty() const {return m_mystring.IsEmpty();}
  LPWSTR GetBuffer(int nMinBufLength) {return m_mystring.GetBuffer(nMinBufLength);}
  LPWSTR GetBuffer() {return m_mystring.GetBuffer();}

  void ReleaseBuffer(int nNewLength = -1) {m_mystring.ReleaseBuffer(nNewLength);}

  int GetLength() const {return m_mystring.GetLength();}
  int Find(wchar_t ch) const {return m_mystring.Find(ch);}
  int Find(LPCWSTR lpszSub) const {return m_mystring.Find(lpszSub);}
  int Find(wchar_t ch, int nstart) const {return m_mystring.Find(ch, nstart);}
  int Find(LPCWSTR lpszSub, int nstart) const {return m_mystring.Find(lpszSub, nstart);}
  int FindOneOf(LPCWSTR lpszSub) const {return m_mystring.FindOneOf(lpszSub);}
  int Replace(const wchar_t chOld, const wchar_t chNew) {return m_mystring.Replace(chOld,chNew);}
  int Replace(const LPCWSTR lpszOld, const LPCWSTR lpszNew)
  {return m_mystring.Replace(lpszOld,lpszNew);}
  int Remove(wchar_t ch) {return m_mystring.Remove(ch);}
  void TrimRight() {m_mystring.TrimRight();}
  void TrimLeft() {m_mystring.TrimLeft();}
  CSecString &Trim() {m_mystring.Trim(); return *this;}
  void MakeLower() {m_mystring.MakeLower();}
  void MakeUpper() {m_mystring.MakeUpper();}
  int Compare(const LPCWSTR lpszOther) const {return m_mystring.Compare(lpszOther);}
  int CompareNoCase(const LPCWSTR lpszOther) const {return m_mystring.CompareNoCase(lpszOther);}
  BOOL IsOnlyWhiteSpace() const
  {CSecString t(*this); return t.Trim().IsEmpty();}
  void EmptyIfOnlyWhiteSpace()
  {if (IsOnlyWhiteSpace() == TRUE) Empty();}
  CSecString SpanIncluding(LPCWSTR lpszCharSet)
  {return CSecString(m_mystring.SpanIncluding(lpszCharSet));}
  CSecString SpanExcluding(LPCWSTR lpszCharSet)
  {return CSecString(m_mystring.SpanExcluding(lpszCharSet));}
  int Delete(int start, int count = 1)
  {return m_mystring.Delete(start, count);}
  int Insert(int index, const CSecString &ss)
  {return m_mystring.Insert(index, ss.m_mystring);}
#else  // _WIN32
  wchar_t GetAt(int nIndex);
  void SetAt(int nIndex, wchar_t ch);
  operator LPCWSTR() const;
  BOOL IsEmpty() const;
  LPWSTR GetBuffer(int nMinBufLength);
  LPWSTR GetBuffer();
  void ReleaseBuffer(int nNewLength = -1);
  int GetLength() const;
  int Find(wchar_t ch) const;
  int Find(LPCWSTR lpszSub) const;
  int Find(wchar_t ch, int nstart) const;
  int Find(LPCWSTR lpszSub, int nstart) const;
  int FindOneOf(LPCWSTR lpszSub) const;
  int Replace(const wchar_t chOld, const wchar_t chNew);
  int Replace(const LPCWSTR lpszOld, const LPCWSTR lpszNew);
  int Remove(wchar_t ch);
  void TrimRight();
  void TrimLeft();
  CSecString &Trim();
  void MakeLower();
  int Compare(const LPCWSTR lpszOther) const;
  int CompareNoCase(const LPCWSTR lpszOther) const;
  BOOL IsOnlyWhiteSpace() const;
  void EmptyIfOnlyWhiteSpace();
  CSecString SpanIncluding(LPCWSTR lpszCharSet);
  CSecString SpanExcluding(LPCWSTR lpszCharSet);
  int Delete(int start, int count = 1);
  int Insert(int index, const CSecString &ss);
#endif // _WIN32

  operator CString() const {return m_mystring;}
  operator CString&() {return m_mystring;}

  CSecString& operator=(const CSecString& stringSrc);
  const CSecString& operator=(wchar_t ch);
  const CSecString& operator=(LPCWSTR lpsz);
  const CSecString& operator+=(const CSecString& s) {m_mystring += s.m_mystring; return *this;}
  const CSecString& operator+=(wchar_t ch) {m_mystring += ch; return *this;}
  const CSecString& operator+=(LPCWSTR lpsz) {m_mystring += lpsz; return *this;}

  friend CSecString operator+(const CSecString& string1,
                              const CSecString& string2);
  friend CSecString operator+(const CSecString& string,
                              wchar_t ch);
  friend CSecString operator+(wchar_t ch,
                              const CSecString& string);
  friend CSecString operator+(const CSecString& string,
                              LPCWSTR lpsz);
  friend CSecString operator+(LPCWSTR lpsz,
                              const CSecString& string);

  CSecString Left(int nCount) const;
  CSecString Right(int nCount) const;
  CSecString Mid(int nFirst) const;
  CSecString Mid(int nFirst, int nCount) const;
  void Empty();
  BOOL LoadString(const UINT &nID);
  void Format(LPCWSTR lpszFormat, ... );
  void Format(UINT nID, ... );
  void Trash() {trashstring();}

private:
  CString m_mystring;
  void trashstring();
};
//-----------------------------------------------------------------------------

inline bool operator==(const CSecString& s1, const CSecString& s2)
    {return (const CString)s1 == (const CString)s2;}
inline bool operator==(const CSecString& s1, LPCWSTR s2)
    {return (const CString)s1==s2;}
inline bool operator==(LPCWSTR s1, const CSecString& s2)
    {return s1==(const CString)s2;}
inline bool operator!=(const CSecString& s1, const CSecString& s2)
    {return (const CString)s1 != (const CString)s2;}
inline bool operator!=(const CSecString& s1, LPCWSTR s2)
    {return (const CString)s1 != s2;}
inline bool operator!=(LPCWSTR s1, const CSecString& s2)
    {return s1 != (const CString)s2;}

//-----------------------------------------------------------------------------
#endif /* __SECSTRING_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
