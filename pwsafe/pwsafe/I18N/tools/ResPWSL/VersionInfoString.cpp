//
// This code was written by Denis Zabavchik as at 29 November 2006.
// It is unmodified except for possibly formatting.
// His article on "Modification of Version Information Resources in compiled binaries"
// can be found at: http://www.codeproject.com/library/VerInfoLib.asp
//

// VersionInfoString.cpp: implementation of the CVersionInfoString class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VersionInfoString.h"
#include "VersionInfoBuffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

CVersionInfoString::CVersionInfoString(const CString& strKey, const CString& strValue):m_strKey(strKey), m_strValue(strValue)
{
}

CVersionInfoString::CVersionInfoString(String* pString)
{
	FromString(pString);
}

//////////////////////////////////////////////////////////////////////
// Loading/saving
void CVersionInfoString::FromString(String* pString)
{
	m_strKey = pString->szKey;
	LPWSTR lpwszValue = (LPWSTR) DWORDALIGN(&pString->szKey[wcslen(pString->szKey)+1]);
	CString strValue;
	if (pString->wValueLength)
		m_strValue = lpwszValue;
}

void CVersionInfoString::Write(CVersionInfoBuffer & viBuf)
{
	//Pad to DWORD and save position for wLength
	DWORD pos = viBuf.PadToDWORD();
	
	//Skip size for now;
	viBuf.Pad(sizeof WORD);

	//Write wValueLength
	if (!m_strValue.IsEmpty())
		viBuf.WriteWord(m_strValue.GetLength() + 1);
	else
		viBuf.WriteWord(0);

	//Write wType (Text)
	viBuf.WriteWord(1);

	//Write key
	viBuf.WriteString(m_strKey);

	//Pad for Value
	viBuf.PadToDWORD();

	//Write the value
	if (!m_strValue.IsEmpty())
		viBuf.WriteString(m_strValue);

	//Set the size of the structure based on current offset from the position
	viBuf.WriteStructSize(pos);
}

//////////////////////////////////////////////////////////////////////////
// Operations

const CString& CVersionInfoString::GetKey() const
{
	return m_strKey;
}

const CString& CVersionInfoString::GetValue() const
{
	return m_strValue;
}

CString& CVersionInfoString::GetValue()
{
	return m_strValue;
}
