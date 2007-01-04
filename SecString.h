/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

/// \file MyString.h
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class SecString
   : public string
{
public:
   /// default constructor
   SecString()
   {}
   /// copy constructor
   SecString(const SecString& src)
      : string(src)
   {}

   // I'll add others as needed...

   /// destructor
   ~SecString();

   TCHAR operator[](int nIndex) const;
   void SetAt(int nIndex, TCHAR ch);
   operator CString() const;
   operator LPCTSTR() const;
   BOOL IsEmpty() const;
   BOOL LoadString(UINT nID);

   const SecString& operator=(const SecString& stringSrc);
   const SecString& operator=(TCHAR ch);
   const SecString& operator=(LPCSTR lpsz);
   const SecString& operator=(LPCWSTR lpsz);
   const SecString& operator=(const unsigned char* psz);

   const SecString& operator+=(const SecString& string);
   const SecString& operator+=(TCHAR ch);
   const SecString& operator+=(LPCTSTR lpsz);

   // CMytring operator+(LPCTSTR lpsz);

   friend SecString AFXAPI operator+(const SecString& string1,const SecString& string2);
   friend SecString AFXAPI operator+(const SecString& string, TCHAR ch);
   friend SecString AFXAPI operator+(TCHAR ch, const SecString& string);
   friend SecString AFXAPI operator+(const SecString& string, LPCTSTR lpsz);
   friend SecString AFXAPI operator+(LPCTSTR lpsz, const SecString& string);
   SecString Mid(int nFirst, int nCount) const;

   LPTSTR GetBuffer(int nMinBufLength);
   void ReleaseBuffer(int nNewLength = -1);
   int GetLength() const;

   int Find( TCHAR ch ) const;
   int Find( LPCTSTR lpszSub ) const;
   CString Left( int nCount ) const;
   CString Right( int nCount ) const;

   CString m_mystring;
private:

   void trashstring();
};
//-----------------------------------------------------------------------------

bool operator==(const SecString& s1, const SecString& s2);
bool operator==(const SecString& s1, LPCTSTR s2);
bool operator==(LPCTSTR s1, const SecString& s2);
bool operator!=(const SecString& s1, const SecString& s2);
bool operator!=(const SecString& s1, LPCTSTR s2);
bool operator!=(LPCTSTR s1, const SecString& s2);

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
