//
// This code was written by Denis Zabavchik as at 29 November 2006.
// It is unmodified except for possibly formatting.
// His article on "Modification of Version Information Resources in compiled binaries"
// can be found at: http://www.codeproject.com/library/VerInfoLib.asp
//

// StringTable.h: interface for the CStringTable class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "VersionInfoString.h"

#include "VersionInfoHelperStructures.h"

class CStringTable: public CObject
{
public:
  //Construction
  CStringTable(const CString& strKey);
  CStringTable(WORD wLang, WORD wCodePageC);
  CStringTable(StringTable* pStringTable);
  virtual ~CStringTable();

  // Returns table key (language ID/codepage)
  const CString& GetKey() const;
  
  // Loads string table from resource structure in memory
  void FromStringTable(StringTable* pStringTable);
  
  // Saves string table to version info buffer
  void Write(CVersionInfoBuffer & viBuf);

  // Overloaded bracket operators used to access strings in the table
  const CString operator [] (const CString &strName) const;
  CString &operator [] (const CString &strName);

  // Iterative access to string objects in table
  POSITION GetFirstStringPosition() const;
  const CVersionInfoString* GetNextString(POSITION &pos) const;
  CVersionInfoString* GetNextString(POSITION &pos);

  // Retrieves the list of string names into a CStringList
  void GetStringNames(CStringList &slNames, BOOL bMerge = FALSE) const;

  friend class CStringFileInfo;
private:
  //Set key renames/changes the language/codepage for the table, accessible
  // only via CStringFileInfo::SetStringTableKey()
  void SetKey(const CString& strKey);

  CObList m_lstStrings;
  CMapStringToOb m_mapStrings;
  CString m_strKey;
};
