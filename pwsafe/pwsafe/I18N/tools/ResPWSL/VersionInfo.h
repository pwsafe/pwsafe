//
// This code was written by Denis Zabavchik as at 29 November 2006.
// It is unmodified except for possibly formatting.
// His article on "Modification of Version Information Resources in compiled binaries"
// can be found at: http://www.codeproject.com/library/VerInfoLib.asp
//

// VersionInfo.h: interface for the CVersionInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <AfxTempl.h>
#include "VersionInfoString.h"
#include "StringTable.h"
#include "StringFileInfo.h"

class CVersionInfo : public CObject  
{
public:
  // Construction/Destruction
  CVersionInfo();
  CVersionInfo(const CString& strModulePath, LPCTSTR lpszResourceId = NULL /*Auto*/, WORD wLangId = 0xFFFF /*Auto*/);
  virtual ~CVersionInfo();

  //Read version information from module
  BOOL FromFile(const CString& strModulePath, LPCTSTR lpszResourceId = NULL /*Auto*/, WORD wLangId = 0xFFFF /*Auto*/);

  //Save version information to module resource (specify strModulePath, lpszResourceId & wLangId to copy resource to different module, resource, language)
  BOOL ToFile(const CString& strModulePath = "", LPCTSTR lpszResourceId = NULL /*Auto*/, WORD wLangId = 0xFFFF /*Auto*/);

  //Quick save (saves to the same module, resource, and language that it was loaded from)
  BOOL Save();

  //Resets (removes all string tables and cleans fixed version info
  void Reset();

  BOOL IsValid() const;

  // Get/Set the order of blocks (Regular (TRUE) = StringFileInfo first, VarFileInfo 2nd)
  BOOL GetInfoBlockOrder() const;
  void SetInfoBlockOrder(BOOL bRegularStringsFirst);

  // Get reference to CStringFileInfo 
  const CStringFileInfo& GetStringFileInfo() const;
  CStringFileInfo& GetStringFileInfo();

  // Overloaded bracket operators allow quick access to first string table in StringFileInfo r/w
  const CString operator [] (const CString &strName) const;
  CString &operator [] (const CString &strName);

  // Get reference to VS_FIXEDFILEINFO
  const VS_FIXEDFILEINFO& GetFixedFileInfo() const;
  VS_FIXEDFILEINFO& GetFixedFileInfo();

  // SetFileVersion - Updates file version in VS_FIXEDFILEINFO and in stringtables when bUpdateStringTables == TRUE
  void SetFileVersion(WORD dwFileVersionMSHi, WORD dwFileVersionMSLo, WORD dwFileVersionLSHi, WORD dwFileVersionLSLo, BOOL bUpdateStringTables = TRUE, LPCTSTR lpszDelim = _T(", "));
  void SetFileVersion(DWORD dwFileVersionMS, DWORD dwFileVersionLS, BOOL bUpdateStringTables = TRUE, LPCTSTR lpszDelim = _T(", "));

  // SetProductVersion - Updates product version in VS_FIXEDFILEINFO and ALL stringtables when bUpdateStringTables == TRUE
  void SetProductVersion(WORD dwProductVersionMSHi, WORD dwProductVersionMSLo, WORD dwProductVersionLSHi, WORD dwProductVersionLSLo, BOOL bUpdateStringTables = TRUE, LPCTSTR lpszDelim = _T(", "));
  void SetProductVersion(DWORD dwProductVersionMS, DWORD dwProductVersionLS, BOOL bUpdateStringTables = TRUE, LPCTSTR lpszDelim = _T(", "));

protected:
  // Loads all structures from specified module, resource, language to version buffer
  BOOL LoadVersionInfoResource(const CString& strModulePath, CVersionInfoBuffer &viBuf, LPCTSTR lpszResourceId = NULL /*Auto*/, WORD wLangId = 0xFFFF);

  // Updates module RT_VERSION resource with specified ID with data in lpData
  BOOL UpdateModuleResource(const CString &strFilePath, LPCTSTR lpszResourceId, WORD wLangId, LPVOID lpData, DWORD dwDataLength);

  // Writes structures to version info buffer in order specified in m_bRegularInfoOrder (Get/SetInfoBlockOrder())
  void Write(CVersionInfoBuffer & viBuf);

  // Writes computed VarFileInfo structure to buffer based on the contents of String table
  void WriteVarInfo(CVersionInfoBuffer & viBuf);

  // Helper functions for automatic loading of first RT_VERSION resource 
  static BOOL CALLBACK EnumResourceNamesFuncFindFirst(HANDLE hModule, LPCTSTR lpType, LPTSTR lpName, LONG_PTR lParam);
  static BOOL CALLBACK EnumResourceLangFuncFindFirst(HANDLE hModule, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIDLanguage, LONG_PTR lParam);

  // Main version info data members
  CStringFileInfo m_stringFileInfo;
  VS_FIXEDFILEINFO m_vsFixedFileInfo;

  // Information about loaded version info (Module, resource id, lang id, and order in which VarFileInfo and StringFileInfo appeared in the module)
  CString m_strModulePath;
  CString m_strStringResourceId;
  LPTSTR	m_lpszResourceId;
  WORD	m_wLangId;
  BOOL	m_bRegularInfoOrder;
};
