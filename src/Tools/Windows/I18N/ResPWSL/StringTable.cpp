//
// This code was written by Denis Zabavchik as at 29 November 2006.
// It is unmodified except for possibly formatting.
// His article on "Modification of Version Information Resources in compiled binaries"
// can be found at: http://www.codeproject.com/library/VerInfoLib.asp
//

// StringTable.cpp: implementation of the CStringTable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringTable.h"
#include "VersionInfoBuffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

CStringTable::CStringTable(const CString& strKey):m_strKey(strKey)
{
}

CStringTable::CStringTable(WORD wLang, WORD wCodePage)
{
  CString strKey;
  strKey.Format(_T("%04x%04x"), wLang, wCodePage);

  SetKey(strKey);
}

CStringTable::CStringTable(StringTable* pStringTable)
{
  FromStringTable(pStringTable);
}

CStringTable::~CStringTable()
{
  while (!m_lstStrings.IsEmpty())
    delete m_lstStrings.RemoveTail();

  m_mapStrings.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// Loading/saving

void CStringTable::FromStringTable(StringTable* pStringTable)
{
  m_strKey = pStringTable->szKey;

  String* pString = (String*) DWORDALIGN(&pStringTable->szKey[wcslen(pStringTable->szKey)+1]);
  while ((DWORD_PTR)pString < ((DWORD_PTR) pStringTable + pStringTable->wLength)) {
    CVersionInfoString* pVIString = new CVersionInfoString(pString);
    m_lstStrings.AddTail(pVIString);
    m_mapStrings.SetAt(pVIString->GetKey(), pVIString);

    pString = (String*) DWORDALIGN((DWORD_PTR) pString + pString->wLength);
  }
}

void CStringTable::Write(CVersionInfoBuffer & viBuf)
{
  //Pad to DWORD and save position for wLength
  DWORD pos = viBuf.PadToDWORD();

  //Skip size for now;
  viBuf.Pad(sizeof WORD);

  //Write wValueLength
  viBuf.WriteWord(0);

  //Write wType
  viBuf.WriteWord(1);

  //Write key
  viBuf.WriteString(m_strKey);

  POSITION posString = m_lstStrings.GetHeadPosition();
  while (posString) {
    CVersionInfoString * pString = (CVersionInfoString*) m_lstStrings.GetNext(posString);
    pString->Write(viBuf);
  }
  //Set the size of the structure based on current offset from the position
  viBuf.WriteStructSize(pos);
}

//////////////////////////////////////////////////////////////////////////
// Operations

const CString& CStringTable::GetKey() const
{
  return m_strKey;
}

void CStringTable::SetKey(const CString& strKey)
{
  m_strKey = strKey;
}

CString & CStringTable::operator [] (const CString &strName)
{
  CVersionInfoString* pVIString = NULL;
  if (!m_mapStrings.Lookup(strName, (CObject*&)pVIString)) {
    pVIString = new CVersionInfoString(strName);
    m_lstStrings.AddTail(pVIString);
    m_mapStrings.SetAt(strName, pVIString);
  }

  return pVIString->GetValue();
}

const CString CStringTable::operator [] (const CString &strName) const
{
  CVersionInfoString* pVIString = NULL;
  if (!m_mapStrings.Lookup(strName, (CObject*&)pVIString)) {
    return "";
  }

  return pVIString->GetValue();
}

POSITION CStringTable::GetFirstStringPosition() const
{
  return m_lstStrings.GetHeadPosition();
}

const CVersionInfoString* CStringTable::GetNextString(POSITION &pos) const
{
  return (CVersionInfoString*)m_lstStrings.GetNext(pos);
}

CVersionInfoString* CStringTable::GetNextString(POSITION &pos)
{
  return (CVersionInfoString*)m_lstStrings.GetNext(pos);
}

void CStringTable::GetStringNames(CStringList &slNames, BOOL bMerge) const
{
  if (!bMerge)
    slNames.RemoveAll();

  POSITION pos = m_lstStrings.GetHeadPosition();
  while (pos) {
    CVersionInfoString* pviString = (CVersionInfoString*)m_lstStrings.GetNext(pos);
    slNames.AddTail(pviString->GetKey());
  }
}
