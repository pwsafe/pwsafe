//
// This code was written by Denis Zabavchik as at 29 November 2006.
// It is unmodified except for possibly formatting.
// His article on "Modification of Version Information Resources in compiled binaries"
// can be found at: http://www.codeproject.com/library/VerInfoLib.asp
//

// VersionInfoBuffer.cpp: implementation of the CVersionInfoBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VersionInfoBuffer.h"
#include <malloc.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVersionInfoBuffer::CVersionInfoBuffer():m_dwBufSize(1024), m_dwPosition(0)
{
  m_lpData = (LPBYTE) new BYTE[m_dwBufSize];
}

CVersionInfoBuffer::~CVersionInfoBuffer()
{
  delete [] m_lpData;
}

void CVersionInfoBuffer::Write(LPVOID lpData, DWORD dwSize)
{
  if (dwSize+m_dwPosition > m_dwBufSize)
    ReallocBuffer(dwSize+m_dwPosition);

  memcpy(m_lpData + m_dwPosition, lpData, dwSize);
  m_dwPosition += dwSize;
}

void CVersionInfoBuffer::ReallocBuffer(DWORD dwMinimumSize)
{
  //Allocate extra 1k or so
  DWORD dwNewSize = (dwMinimumSize + 0x7ff) & ~0x3ff;

  LPBYTE lpNewData = new BYTE[dwNewSize];

  //Copy everything that is already in the buffer
  memcpy(lpNewData, m_lpData, m_dwPosition);

  delete [] m_lpData;
  m_dwBufSize = dwNewSize;
  m_lpData = lpNewData;
}

DWORD CVersionInfoBuffer::PadToDWORD()
{

  if (m_dwPosition % 4) {
    DWORD dwNull = 0L;
    Write(&dwNull, 4 - m_dwPosition % 4);
  }

  return m_dwPosition;
}

DWORD CVersionInfoBuffer::Pad(WORD wLength)
{
  DWORD dwNull = 0L;
  while (wLength--)
    Write(&dwNull, 1);

  return m_dwPosition;
}

DWORD CVersionInfoBuffer::GetPosition()
{
  return m_dwPosition;
}

void CVersionInfoBuffer::WriteStructSize(DWORD dwOffsetOfSizeMemember)
{
  WORD wSize = LOWORD(m_dwPosition - dwOffsetOfSizeMemember);

  WORD *pSizeMember = (WORD*) (&m_lpData[dwOffsetOfSizeMemember]);
  *pSizeMember = wSize;
}

void CVersionInfoBuffer::WriteWord(WORD wData)
{
  Write(&wData, sizeof WORD);
}

WORD CVersionInfoBuffer::WriteString(const CString &strValue)
{
#ifndef _UNICODE
  DWORD dwLength = MultiByteToWideChar(CP_ACP, 0, strValue, -1, NULL, 0);
  WCHAR *pszwValue = (WCHAR*)_alloca(dwLength * sizeof (WCHAR));
  MultiByteToWideChar(CP_ACP, 0, strValue, -1, pszwValue, dwLength);

  Write(pszwValue, dwLength * sizeof (WCHAR));

  return LOWORD(dwLength);
#else
  DWORD dwLength = (strValue.GetLength()+ 1) * sizeof WCHAR;
  Write ((LPVOID)(LPCWSTR) strValue, dwLength);
  return LOWORD(dwLength);
#endif
}

const LPBYTE CVersionInfoBuffer::GetData()
{
  return m_lpData;
}
