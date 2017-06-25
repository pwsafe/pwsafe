//
// This code was written by Denis Zabavchik as at 29 November 2006.
// It is unmodified except for possibly formatting.
// His article on "Modification of Version Information Resources in compiled binaries"
// can be found at: http://www.codeproject.com/library/VerInfoLib.asp
//

// StringFileInfo.cpp: implementation of the CStringFileInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringFileInfo.h"
#include "StringTable.h"
#include "VersionInfoBuffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

CStringFileInfo::CStringFileInfo()
{
}

CStringFileInfo::CStringFileInfo(StringFileInfo* pStringFI)
{
  FromStringFileInfo(pStringFI);
}

CStringFileInfo::~CStringFileInfo()
{
  Reset();
}

//////////////////////////////////////////////////////////////////////
// Loading/saving

void CStringFileInfo::FromStringFileInfo(StringFileInfo* pStringFI)
{
  ASSERT(pStringFI);

  StringTable* pStringTable = (StringTable*) DWORDALIGN(&pStringFI->szKey[wcslen(pStringFI->szKey)+1]);
  while ((DWORD_PTR)pStringTable < ((DWORD_PTR) pStringFI + pStringFI->wLength)) {
    CStringTable* pObStringTable = new CStringTable(pStringTable);
    AddStringTable(pObStringTable);

    pStringTable = (StringTable*) DWORDALIGN((DWORD_PTR)pStringTable + pStringTable->wLength);

  }
}

void CStringFileInfo::Write(CVersionInfoBuffer & viBuf)
{
  //Check string tables
  if (m_lstStringTables.IsEmpty())
    return;

  //Pad to DWORD and save position for wLength
  DWORD pos = viBuf.PadToDWORD();

  //Skip size for now;
  viBuf.Pad(sizeof WORD);

  //Write wValueLength
  viBuf.WriteWord(0);

  //Write wType
  viBuf.WriteWord(1);

  //Write key
  viBuf.WriteString(L"StringFileInfo");

  POSITION posString = m_lstStringTables.GetHeadPosition();
  while (posString) {
    CStringTable * pString = (CStringTable*) m_lstStringTables.GetNext(posString);
    pString->Write(viBuf);
  }
  //Set the size of the structure based on current offset from the position
  viBuf.WriteStructSize(pos);
}

//////////////////////////////////////////////////////////////////////
// Operations

BOOL CStringFileInfo::IsEmpty()
{
  return m_lstStringTables.IsEmpty();
}

DWORD CStringFileInfo::GetStringTableCount()
{
  return (DWORD)m_lstStringTables.GetCount();
}

CStringTable& CStringFileInfo::GetFirstStringTable()
{
  return (CStringTable&)*m_lstStringTables.GetHead();
}

const CStringTable& CStringFileInfo::GetFirstStringTable() const
{
  return (CStringTable&)*m_lstStringTables.GetHead();
}

CStringTable& CStringFileInfo::GetStringTable(const CString& strKey)
{
  CStringTable *pStringTable = NULL;
  if (!m_mapStringTables.Lookup(strKey, (CObject*&)pStringTable)) {
    pStringTable = new CStringTable(strKey);
    AddStringTable(pStringTable);
  }

  return *pStringTable;
}

const CStringTable& CStringFileInfo::GetStringTable(const CString& strKey) const
{
  CStringTable *pStringTable = NULL;
  m_mapStringTables.Lookup(strKey, (CObject*&)pStringTable);
  // This may return *NULL, be carefull
  return *pStringTable;
}

CStringTable& CStringFileInfo::operator [] (const CString &strKey)
{
  return GetStringTable(strKey);
}

const CStringTable& CStringFileInfo::operator [] (const CString &strKey) const
{
  return GetStringTable(strKey);
}

BOOL CStringFileInfo::HasStringTable(const CString &strKey) const
{
  CObject *pDummyObject;
  return m_mapStringTables.Lookup(strKey, pDummyObject);
}

CStringTable& CStringFileInfo::AddStringTable(const CString &strKey)
{
  return GetStringTable(strKey);
}

CStringTable& CStringFileInfo::AddStringTable(CStringTable* pStringTable)
{
  m_lstStringTables.AddTail(pStringTable);
  m_mapStringTables.SetAt(pStringTable->GetKey(), pStringTable);
  return *pStringTable;
}

POSITION CStringFileInfo::GetFirstStringTablePosition() const
{
  return m_lstStringTables.GetHeadPosition();
}

CStringTable* CStringFileInfo::GetNextStringTable(POSITION &pos)
{
  return (CStringTable*) m_lstStringTables.GetNext(pos);
}

const CStringTable* CStringFileInfo::GetNextStringTable(POSITION &pos) const
{
  return (CStringTable*) m_lstStringTables.GetNext(pos);
}

BOOL CStringFileInfo::SetStringTableKey(const CString &strOldKey, const CString &strNewKey)
{
  CStringTable *pStringTable = NULL;
  if (m_mapStringTables.Lookup(strOldKey, (CObject*&)pStringTable)) {
    pStringTable->SetKey(strNewKey);
    m_mapStringTables.RemoveKey(strOldKey);
    m_mapStringTables.SetAt(strNewKey, pStringTable);

    return TRUE;
  }

  return FALSE;
}

void CStringFileInfo::Reset()
{
  while (!m_lstStringTables.IsEmpty())
    delete m_lstStringTables.RemoveTail();

  m_mapStringTables.RemoveAll();
}
