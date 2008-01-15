//
// This code was written by Denis Zabavchik as at 29 November 2006.
// It is unmodified except for possibly formatting.
// His article on "Modification of Version Information Resources in compiled binaries"
// can be found at: http://www.codeproject.com/library/VerInfoLib.asp
//

// VersionInfoString.h: interface for the CVersionInfoString class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "VersionInfoHelperStructures.h"

class CVersionInfoBuffer;

class CVersionInfoString: public CObject
{
public:
  CVersionInfoString(String* pString);
  CVersionInfoString(const CString& strKey, const CString& strValue = "");

  const CString& GetKey() const;
  const CString& GetValue() const;

  CString& GetValue();

  void FromString(String* pString);
  void Write(CVersionInfoBuffer & viBuf);
private:
  CString m_strKey;
  CString m_strValue;
};
