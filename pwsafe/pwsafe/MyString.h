// MyString.h
// A drop-in replacement for CString, the main difference being that the
// data is scrubbed by trashstring() in the destructor, thus leaving an attacker
// with a little less info to grovel for in the swap file / core dump / whatever
//
// First priority for porting is to implement this without CString
//-----------------------------------------------------------------------------

#ifndef _MYSTRING_H_
#define _MYSTRING_H_
#include <AFX.H>
//-----------------------------------------------------------------------------
class CMyString
{
public:
   CMyString();
   CMyString(LPCTSTR lpsz);
   CMyString(LPCTSTR lpsz, int nLength);
   CMyString(const CMyString& stringSrc);
   CMyString(const CString& stringSrc);
   ~CMyString();

   TCHAR operator[](int nIndex) const;
   void SetAt(int nIndex, TCHAR ch);
   operator CString() const;
   operator CString&();
   operator LPCTSTR() const;
   BOOL IsEmpty() const;

   const CMyString& operator=(const CMyString& stringSrc);
   const CMyString& operator=(TCHAR ch);
   const CMyString& operator=(LPCTSTR lpsz);
   const CMyString& operator=(const unsigned char* psz);

   const CMyString& operator+=(const CMyString& string);
   const CMyString& operator+=(TCHAR ch);
   const CMyString& operator+=(LPCTSTR lpsz);

   // CMytring operator+(LPCTSTR lpsz);

   friend CMyString AFXAPI operator+(const CMyString& string1,
                                     const CMyString& string2);
   friend CMyString AFXAPI operator+(const CMyString& string,
                                     TCHAR ch);
   friend CMyString AFXAPI operator+(TCHAR ch,
                                     const CMyString& string);
   friend CMyString AFXAPI operator+(const CMyString& string,
                                     LPCTSTR lpsz);
   friend CMyString AFXAPI operator+(LPCTSTR lpsz,
                                     const CMyString& string);

   CMyString Mid(int nFirst, int nCount) const;

   LPTSTR GetBuffer(int nMinBufLength);
   void ReleaseBuffer(int nNewLength = -1);
   int GetLength() const;

   int Find(TCHAR ch) const;
   int Find(LPCTSTR lpszSub) const;
   CString Left(int nCount) const;
   CString Right(int nCount) const;
  void TrimRight() {m_mystring.TrimRight();}
  void TrimLeft() {m_mystring.TrimLeft();}
  void MakeLower() {m_mystring.MakeLower();}

  void Trash() {trashstring();}

private:
  CString m_mystring;
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
