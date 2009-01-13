//
// This code was written by Denis Zabavchik as at 29 November 2006.
// It is unmodified except for possibly formatting.
// His article on "Modification of Version Information Resources in compiled binaries"
// can be found at: http://www.codeproject.com/library/VerInfoLib.asp
//

// VersionInfoBuffer.h: interface for the CVersionInfoBuffer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

// Helper class for VersionInfo (auto reallocation of buffer, and helpful functions like Pad, PadToDWORD, WriteStructSize
class CVersionInfoBuffer : public CObject  
{
public:

  CVersionInfoBuffer();
  virtual ~CVersionInfoBuffer();

  // Writes data to the buffer
  void Write(LPVOID lpData, DWORD dwSize);

  // Writes string to the buffer (converts to Unicode)
  WORD WriteString(const CString& strValue);

  // Writes a WORD to the buffer
  void WriteWord(WORD wData);

  // Writes the difference between specified offset and current length to a WORD at given offset
  // this writing the structure size wLength
  void WriteStructSize(DWORD dwOffsetOfSizeMemember);

  // Returns current position
  DWORD GetPosition();

  // Allings to DWORD (pads with 0s)
  DWORD PadToDWORD();

  // Pads with zeroes 
  DWORD Pad(WORD wLength);

  // Get pointer to data (pointer can not be used after any writes made after calling GetData() due to possible relocation)
  const LPBYTE GetData();
private:
  DWORD m_dwPosition;
  DWORD m_dwBufSize;
  LPBYTE m_lpData;
protected:
  void ReallocBuffer(DWORD dwMinimumSize);
};
