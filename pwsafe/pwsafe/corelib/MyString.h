// MyString.h
//-----------------------------------------------------------------------------

#ifndef _MYSTRING_H_
#define _MYSTRING_H_

//-----------------------------------------------------------------------------
class CMyString
{
public:
   CMyString();
   CMyString(LPCSTR lpsz);
   CMyString(const CMyString &stringSrc);
   CMyString(const CString &stringSrc);
   ~CMyString();

   TCHAR operator[](int nIndex) const;
   void SetAt(int nIndex, TCHAR ch);
   operator CString() const;
   operator LPCTSTR() const;
   BOOL IsEmpty() const;
   BOOL LoadString(UINT nID);

   const CMyString& operator=(const CMyString& stringSrc);
   const CMyString& operator=(TCHAR ch);
   const CMyString& operator=(LPCSTR lpsz);
   const CMyString& operator=(LPCWSTR lpsz);
   const CMyString& operator=(const unsigned char* psz);

   const CMyString& operator+=(const CMyString& string);
   const CMyString& operator+=(TCHAR ch);
   const CMyString& operator+=(LPCTSTR lpsz);

   // CMytring operator+(LPCTSTR lpsz);

   friend CMyString AFXAPI operator+(const CMyString& string1,const CMyString& string2);
   friend CMyString AFXAPI operator+(const CMyString& string, TCHAR ch);
   friend CMyString AFXAPI operator+(TCHAR ch, const CMyString& string);
   friend CMyString AFXAPI operator+(const CMyString& string, LPCTSTR lpsz);
   friend CMyString AFXAPI operator+(LPCTSTR lpsz, const CMyString& string);
   CMyString Mid(int nFirst, int nCount) const;

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

bool operator==(const CMyString& s1, const CMyString& s2);
bool operator==(const CMyString& s1, LPCTSTR s2);
bool operator==(LPCTSTR s1, const CMyString& s2);
bool operator!=(const CMyString& s1, const CMyString& s2);
bool operator!=(const CMyString& s1, LPCTSTR s2);
bool operator!=(LPCTSTR s1, const CMyString& s2);

//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
