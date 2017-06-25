//
// This code was written by Denis Zabavchik as at 29 November 2006.
// It is unmodified except for possibly formatting.
// His article on "Modification of Version Information Resources in compiled binaries"
// can be found at: http://www.codeproject.com/library/VerInfoLib.asp
//

// VersionInfo.cpp: implementation of the CVersionInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringTable.h"
#include "StringFileInfo.h"
#include "VersionInfoHelperStructures.h"
#include "VersionInfoBuffer.h"
#include "VersionInfo.h"

#include <malloc.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CVersionInfo main class wrapping Version info for modules
CVersionInfo::CVersionInfo()
  : m_lpszResourceId(NULL), m_wLangId(0xFFFF), m_bRegularInfoOrder(TRUE)
{
  ZeroMemory(&m_vsFixedFileInfo, sizeof VS_VERSION_INFO);
}

CVersionInfo::CVersionInfo(const CString& strModulePath, LPCTSTR lpszResourceId, WORD wLangId)
  : m_strModulePath(strModulePath), m_lpszResourceId((LPTSTR)lpszResourceId),
  m_wLangId(wLangId), m_bRegularInfoOrder(TRUE)
{
  // LPCTSTR lpszResourceId may contain integer value pointer to string,
  // in case it's a string make a local copy of it
  if (IS_INTRESOURCE(lpszResourceId)) { 
    m_strStringResourceId = lpszResourceId; 
  } 

  ZeroMemory(&m_vsFixedFileInfo, sizeof VS_VERSION_INFO);

  FromFile(strModulePath, lpszResourceId, wLangId);
}

CVersionInfo::~CVersionInfo()
{
}

BOOL CVersionInfo::Save()
{
  return ToFile();
}

BOOL CVersionInfo::ToFile(const CString &strModulePath, LPCTSTR lpszResourceId, 
                          WORD wLangId, const bool bReplace)
{
  CString strUseModulePath(strModulePath);

  if (strUseModulePath.IsEmpty()) {
    strUseModulePath = m_strModulePath;
  }

  if (NULL == lpszResourceId) {
    //Try resource ID that we loaded from;
    lpszResourceId = m_lpszResourceId;

    if (NULL == lpszResourceId) {
      //Use default
      lpszResourceId = MAKEINTRESOURCE(1);
    }
  }

  if (0xFFFF == wLangId) {
    //Try using language that we loaded from
    wLangId = m_wLangId;

    if (0xFFFF == wLangId) {
      //Use neutral
      wLangId = NULL;
    }
  }

  CVersionInfoBuffer viSaveBuf;
  Write(viSaveBuf);

  return UpdateModuleResource(strUseModulePath, lpszResourceId, wLangId,
                              viSaveBuf.GetData(), viSaveBuf.GetPosition(), bReplace);
}

BOOL CVersionInfo::UpdateModuleResource(const CString &strFilePath, LPCTSTR lpszResourceId,
                                        WORD wLangId, LPVOID lpData, DWORD dwDataLength,
                                        const bool bReplace)
{
  HANDLE hUpdate = ::BeginUpdateResource(strFilePath, FALSE);

  if (hUpdate == NULL)
    return FALSE;

  BOOL bUpdateResult = TRUE;

  // If we need to replace the language - delete original first
  if (bReplace)
    bUpdateResult = UpdateResource(hUpdate, RT_VERSION, lpszResourceId, m_wLangId, NULL, 0);

  // Update or add new version information
  if (bUpdateResult)
    bUpdateResult = UpdateResource(hUpdate, RT_VERSION, lpszResourceId, wLangId, lpData, dwDataLength);

  return EndUpdateResource(hUpdate, FALSE) && bUpdateResult;
}

BOOL CVersionInfo::FromFile(const CString &strModulePath, LPCTSTR lpszResourceId, WORD wLangId)
{
  CVersionInfoBuffer viLoadBuf;

  m_wLangId = wLangId;
  m_lpszResourceId = (LPTSTR)lpszResourceId;

  // LoadVersionInfoResource will update member variables m_wLangId, m_lpszResourceId, 
  // which is awkward, need to change this flow
  if (!LoadVersionInfoResource(strModulePath, viLoadBuf, lpszResourceId, wLangId))
    return FALSE;

  m_strModulePath = strModulePath;

  VERSION_INFO_HEADER* pVI = (VERSION_INFO_HEADER*) viLoadBuf.GetData();

  ASSERT(!wcscmp(pVI->szKey, L"VS_VERSION_INFO"));

  VS_FIXEDFILEINFO* pFixedInfo = (VS_FIXEDFILEINFO*)DWORDALIGN(&pVI->szKey[wcslen(pVI->szKey)+1]);

  memcpy(&m_vsFixedFileInfo, pFixedInfo, sizeof(VS_FIXEDFILEINFO));

  // Iterate children StringFileInfo or VarFileInfo
  BaseFileInfo *pChild = (BaseFileInfo*) DWORDALIGN((DWORD_PTR)pFixedInfo + pVI->wValueLength);

  BOOL bHasVar = FALSE;
  BOOL bHasStrings = FALSE;
  BOOL bBlockOrderKnown = FALSE;
  CStringList lstTranslations;

  while ((DWORD_PTR)pChild < ((DWORD_PTR)(pVI) + pVI->wLength)) {
    if (!wcscmp(pChild->szKey, L"StringFileInfo")) {
      //It is a StringFileInfo
      ASSERT(1 == pChild->wType);

      StringFileInfo* pStringFI = (StringFileInfo*)pChild;
      ASSERT(!pStringFI->wValueLength);

      // MSDN says: Specifies an array of zero or one StringFileInfo structures.
      // So there should be only one StringFileInfo at most
      ASSERT(m_stringFileInfo.IsEmpty());

      m_stringFileInfo.FromStringFileInfo(pStringFI);
      bHasStrings = TRUE;
    } else {
      VarFileInfo* pVarInfo = (VarFileInfo*)pChild;
      ASSERT(1 == pVarInfo->wType);
      ASSERT(!wcscmp(pVarInfo->szKey, L"VarFileInfo"));
      ASSERT(!pVarInfo->wValueLength);
      // Iterate Var elements
      // There really must be only one
      Var* pVar = (Var*) DWORDALIGN(&pVarInfo->szKey[wcslen(pVarInfo->szKey)+1]);
      while ((DWORD_PTR)pVar < ((DWORD_PTR) pVarInfo + pVarInfo->wLength)) {
        ASSERT(!bHasVar && "Multiple Vars in VarFileInfo");
        ASSERT(!wcscmp(pVar->szKey, L"Translation"));
        ASSERT(pVar->wValueLength);

        DWORD *pValue = (DWORD*) DWORDALIGN(&pVar->szKey[wcslen(pVar->szKey)+1]);
        DWORD *pdwTranslation = pValue;
        while ((LPBYTE)pdwTranslation < (LPBYTE)pValue + pVar->wValueLength) {
          CString strStringTableKey;
          strStringTableKey.Format(_T("%04x%04x"), LOWORD(*pdwTranslation), HIWORD(*pdwTranslation));

          lstTranslations.AddTail(strStringTableKey);
          pdwTranslation++;
        }

        bHasVar = TRUE;
        pVar = (Var*) DWORDALIGN((DWORD_PTR)pVar + pVar->wLength);
      }

      ASSERT(bHasVar && "No Var in VarFileInfo");

    }

    if (!bBlockOrderKnown) {
      bBlockOrderKnown = TRUE;
      m_bRegularInfoOrder = bHasStrings;
    }
    pChild = (BaseFileInfo*) DWORDALIGN((DWORD_PTR)pChild + pChild->wLength);
  }

#ifdef _DEBUG
  ASSERT((DWORD)lstTranslations.GetCount() == m_stringFileInfo.GetStringTableCount());

  CString strKey = m_stringFileInfo.GetFirstStringTable().GetKey();
  POSITION posTranslation = lstTranslations.GetHeadPosition();
  while (posTranslation) {
    CString strTranslation = lstTranslations.GetNext(posTranslation);
    CString strTranslationUpper (strTranslation);
    strTranslation.MakeUpper();

    ASSERT(m_stringFileInfo.HasStringTable(strTranslation) || 
           m_stringFileInfo.HasStringTable(strTranslationUpper));
  }
  //Verify Write
  CVersionInfoBuffer viSaveBuf;
  Write(viSaveBuf);
  ASSERT(viSaveBuf.GetPosition() == viLoadBuf.GetPosition());
  ASSERT(!memcmp(viSaveBuf.GetData(), viLoadBuf.GetData(), viSaveBuf.GetPosition()));

  CFile fOriginal(_T("f1.res"), CFile::modeCreate | CFile::modeWrite);
  fOriginal.Write(viLoadBuf.GetData(), viLoadBuf.GetPosition());
  fOriginal.Close();

  CFile fSaved(_T("f2.res"), CFile::modeCreate | CFile::modeWrite);
  fSaved.Write(viSaveBuf.GetData(), viSaveBuf.GetPosition());
  fSaved.Close();

#endif
  return TRUE;
}

void CVersionInfo::WriteVarInfo(CVersionInfoBuffer & viBuf)
{
  //Check string tables
  if (m_stringFileInfo.IsEmpty())
    return;

  //Prepare to write VarFileInfo
  DWORD posVarInfo = viBuf.PadToDWORD();

  //Skip size of VarFileInfo for now;
  viBuf.Pad(sizeof WORD);

  //Write wValueLength
  viBuf.WriteWord(0);

  //Write type
  viBuf.WriteWord(1);
  viBuf.WriteString(L"VarFileInfo");

  //Save offset of Var structure (Translation)
  DWORD posTranslation = viBuf.PadToDWORD();
  viBuf.Pad(sizeof WORD);

  //Write size of translation, that is number of string tables * size of DWORD
  DWORD dwTableCount = m_stringFileInfo.GetStringTableCount();
  viBuf.WriteWord(LOWORD(dwTableCount * sizeof DWORD));

  //Write type
  viBuf.WriteWord(0);

  //Write key (Translation)
  viBuf.WriteString(L"Translation");

  //Pad for value
  viBuf.PadToDWORD();

  //Collect all id's in one DWORD array
  DWORD *pTranslationBuf = (DWORD*)_alloca(dwTableCount * sizeof DWORD);
  DWORD *pTranslation = pTranslationBuf;
  POSITION posTable = m_stringFileInfo.GetFirstStringTablePosition();
  while (posTable) {
    CStringTable * pStringTable = m_stringFileInfo.GetNextStringTable(posTable);
    TCHAR* pchEnding = NULL;
    DWORD dwKey = _tcstol(pStringTable->GetKey(),&pchEnding, 16);
    *pTranslation = (LOWORD(dwKey) << 16) | (HIWORD(dwKey));
    pTranslation++;
  }
  viBuf.Write(pTranslationBuf, dwTableCount * sizeof DWORD);

  //Write structure sizes
  viBuf.WriteStructSize(posTranslation);
  viBuf.WriteStructSize(posVarInfo);
}

void CVersionInfo::Write(CVersionInfoBuffer & viBuf)
{
  //Pad to DWORD and save position for wLength
  DWORD pos = viBuf.PadToDWORD();

  //Skip size for now;
  viBuf.Pad(sizeof WORD);

  //Write wValueLength
  viBuf.WriteWord(sizeof VS_FIXEDFILEINFO);

  //Write wType
  viBuf.WriteWord(0);

  //Write key
  viBuf.WriteString(L"VS_VERSION_INFO");

  //Pad Fixed info
  viBuf.PadToDWORD();

  //Write Fixed file info
  viBuf.Write(&m_vsFixedFileInfo, sizeof VS_FIXEDFILEINFO);

  if (m_bRegularInfoOrder) {
    //Write string file info, it will pad as needed
    m_stringFileInfo.Write(viBuf);

    WriteVarInfo(viBuf);
  } else {
    WriteVarInfo(viBuf);

    //Write string file info, it will pad as needed
    m_stringFileInfo.Write(viBuf);
  }

  //Set the size of the Version Info
  viBuf.WriteStructSize(pos);
}

void CVersionInfo::Reset()
{
  m_stringFileInfo.Reset();
  m_strModulePath.Empty();
  m_lpszResourceId = NULL;
  m_wLangId = 0xFFFF;
  ZeroMemory(&m_vsFixedFileInfo, sizeof VS_FIXEDFILEINFO);
}

BOOL CVersionInfo::IsValid() const
{
  return (m_vsFixedFileInfo.dwSignature == 0xFEEF04BD);
}

BOOL CVersionInfo::GetInfoBlockOrder() const
{
  return m_bRegularInfoOrder;
}

void CVersionInfo::SetInfoBlockOrder(BOOL bRegularStringsFirst)
{
  m_bRegularInfoOrder = bRegularStringsFirst;
}

BOOL CVersionInfo::EnumResourceNamesFuncFindFirst(HANDLE /*hModule*/,   // module handle 
                                                  LPCTSTR /*lpType*/,   // address of resource type 
                                                  LPTSTR lpName,        // address of resource name 
                                                  LONG_PTR lParam)      // extra parameter, could be 
{ 
  CVersionInfo * pVI= (CVersionInfo *)lParam;

  pVI->m_lpszResourceId = lpName;

  if (!IS_INTRESOURCE(lpName)) { 
    pVI->m_strStringResourceId = lpName;

    //And repoint lpszResourceId to the string
    pVI->m_lpszResourceId = (LPTSTR)(LPCTSTR)pVI->m_strStringResourceId;
  } 

  //Stop enumeration
  return FALSE; 
} 

BOOL CVersionInfo::EnumResourceLangFuncFindFirst(HANDLE /*hModule*/,     // module handle
                                                 LPCTSTR /*lpszType*/,   // resource type
                                                 LPCTSTR /*lpszName*/,   // resource name
                                                 WORD wIDLanguage,       // language identifier
                                                 LONG_PTR lParam)        // application-defined parameter
{
  CVersionInfo * pVI= (CVersionInfo *)lParam;

  pVI->m_wLangId = wIDLanguage;

  //Stop enumeration
  return FALSE;
}

BOOL CVersionInfo::LoadVersionInfoResource(const CString& strModulePath,
                                           CVersionInfoBuffer &viBuf, LPCTSTR lpszResourceId,
                                           WORD wLangId)
{
  HRSRC hResInfo; 

  HMODULE hModule = LoadLibraryEx(strModulePath, NULL, 
                                  DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE);
  if (NULL == hModule)
    return FALSE;

  if ((NULL == lpszResourceId) && (wLangId == 0xFFFF)) {
    //Load first RT_VERSION resource that will be found

    m_lpszResourceId = NULL;

    EnumResourceNames(hModule, RT_VERSION,
                      (ENUMRESNAMEPROC)EnumResourceNamesFuncFindFirst, (LONG_PTR)this);

    if (NULL == m_lpszResourceId) {
      FreeLibrary(hModule);
      return FALSE;
    }

    // Now the m_lpszResourceId must be the name of the resource
    m_wLangId = 0xFFFF;
    EnumResourceLanguages(hModule, RT_VERSION, m_lpszResourceId,
                          (ENUMRESLANGPROC)EnumResourceLangFuncFindFirst, (LONG_PTR)this);

    // Found resource, copy the ID's to local vars
    lpszResourceId = m_lpszResourceId;
    wLangId = m_wLangId;
  }

  hResInfo = FindResourceEx(hModule, RT_VERSION, lpszResourceId, wLangId); 
  // Write the resource language to the resource information file. 

  DWORD dwSize = SizeofResource(hModule, hResInfo);
  if (dwSize) {
    HGLOBAL hgRes = LoadResource(hModule, hResInfo);
    if (hgRes) {
      LPVOID lpMemory = LockResource(hgRes);
      if (lpMemory) {
        viBuf.Write(lpMemory,dwSize);

        UnlockResource(hgRes);
        FreeLibrary(hModule);
        return TRUE;
      }
    }
  }

  FreeLibrary(hModule);
  return FALSE;

}

const CStringFileInfo& CVersionInfo::GetStringFileInfo() const
{
  return m_stringFileInfo;
}

CStringFileInfo& CVersionInfo::GetStringFileInfo()
{
  return m_stringFileInfo;
}

const CString CVersionInfo::operator [] (const CString &strName) const
{
  return m_stringFileInfo.GetFirstStringTable().operator[] (strName);
}

CString & CVersionInfo::operator [] (const CString &strName)
{
  return m_stringFileInfo.GetFirstStringTable().operator[] (strName);
}

const VS_FIXEDFILEINFO& CVersionInfo::GetFixedFileInfo() const
{
  return m_vsFixedFileInfo;
}

VS_FIXEDFILEINFO& CVersionInfo::GetFixedFileInfo()
{
  return m_vsFixedFileInfo;
}

void CVersionInfo::SetFileVersion(WORD dwFileVersionMSHi, WORD dwFileVersionMSLo,
                                  WORD dwFileVersionLSHi, WORD dwFileVersionLSLo,
                                  BOOL bUpdateStringTables /* =TRUE */, LPCTSTR lpszDelim /*= _T(", ") */)
{
  SetFileVersion((dwFileVersionMSHi << 16) | dwFileVersionMSLo,
                 (dwFileVersionLSHi << 16) | dwFileVersionLSLo,
                 bUpdateStringTables, lpszDelim);
}

void CVersionInfo::SetFileVersion(DWORD dwFileVersionMS, DWORD dwFileVersionLS,
                                  BOOL bUpdateStringTables /* =TRUE */, LPCTSTR lpszDelim /*= _T(", ") */)
{
  m_vsFixedFileInfo.dwFileVersionMS = dwFileVersionMS;
  m_vsFixedFileInfo.dwFileVersionLS = dwFileVersionLS;

  if (bUpdateStringTables) {
    POSITION posTable = m_stringFileInfo.GetFirstStringTablePosition();
    CString strVersion;
    strVersion.Format(_T("%d%s%d%s%d%s%d"), HIWORD(dwFileVersionMS), lpszDelim,
                                            LOWORD(dwFileVersionMS), lpszDelim,
                                            HIWORD(dwFileVersionLS), lpszDelim,
                                            LOWORD(dwFileVersionLS));
    while (posTable != NULL) {
      CStringTable * pStringTable = m_stringFileInfo.GetNextStringTable(posTable);
      (*pStringTable)[L"FileVersion"] = strVersion;
    }
  }
}

void CVersionInfo::SetProductVersion(WORD dwProductVersionMSHi, WORD dwProductVersionMSLo,
                                     WORD dwProductVersionLSHi, WORD dwProductVersionLSLo,
                                     BOOL bUpdateStringTables /* =TRUE */, LPCTSTR lpszDelim /*= _T(", ") */)
{
  SetProductVersion((dwProductVersionMSHi << 16) | dwProductVersionMSLo,
                    (dwProductVersionLSHi << 16) | dwProductVersionLSLo,
                    bUpdateStringTables, lpszDelim);
}

void CVersionInfo::SetProductVersion(DWORD dwProductVersionMS, DWORD dwProductVersionLS,
                                     BOOL bUpdateStringTables /* =TRUE */, LPCTSTR lpszDelim /*= _T(", ") */)
{
  m_vsFixedFileInfo.dwProductVersionMS = dwProductVersionMS;
  m_vsFixedFileInfo.dwProductVersionLS = dwProductVersionLS;

  if (bUpdateStringTables) {
    POSITION posTable = m_stringFileInfo.GetFirstStringTablePosition();
    CString strVersion;
    strVersion.Format(_T("%d%s%d%s%d%s%d"), HIWORD(dwProductVersionMS), lpszDelim,
                                            LOWORD(dwProductVersionMS), lpszDelim,
                                            HIWORD(dwProductVersionLS), lpszDelim,
                                            LOWORD(dwProductVersionLS));
    while (posTable != NULL) {
      CStringTable * pStringTable = m_stringFileInfo.GetNextStringTable(posTable);
      (*pStringTable)[L"ProductVersion"] = strVersion;
    }
  }
}
