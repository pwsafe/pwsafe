//
// This code was written by Denis Zabavchik as at 29 November 2006.
// It is unmodified except for possibly formatting.
// His article on "Modification of Version Information Resources in compiled binaries"
// can be found at: http://www.codeproject.com/library/VerInfoLib.asp
//

// StringFileInfo.h: interface for the CStringFileInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "VersionInfoHelperStructures.h"

class CVersionInfoBuffer;
class CStringTable;

class CStringFileInfo: public CObject
{
public:
	void Reset();
	CStringFileInfo();
	CStringFileInfo(StringFileInfo* pStringFI);
	virtual ~CStringFileInfo();

	void FromStringFileInfo(StringFileInfo* pStringFI);
	void Write(CVersionInfoBuffer & viBuf);

	BOOL IsEmpty();

	// Table count
	DWORD GetStringTableCount();

	// Iterative Access to StringTables
	POSITION GetFirstStringTablePosition() const;

	const CStringTable* GetNextStringTable(POSITION &pos) const;
	CStringTable* GetNextStringTable(POSITION &pos);

	// Convenient references to first usually the only string table
	const CStringTable& GetFirstStringTable() const;
	CStringTable& GetFirstStringTable();

	// Access string tables by keys (language ID + Code Page)
	const CStringTable& GetStringTable(const CString& strKey) const;
	CStringTable& GetStringTable(const CString& strKey);

	// Bracket operators allowing easy access to string tables
	const CStringTable& operator [] (const CString &strKey) const;
	CStringTable &operator [] (const CString &strKey);

	// Checks if string table for specified key allready defined
	BOOL HasStringTable(const CString &strKey) const;

	// Add new String table
	CStringTable& AddStringTable(const CString &strKey);
	CStringTable& AddStringTable(CStringTable* pStringTable);

	// Change language of the string table (the proper way, do not use CStringTable::SetKey() directly)
	BOOL SetStringTableKey(const CString &strOldKey, const CString &strNewKey);

private:
	CObList m_lstStringTables;
	CMapStringToOb m_mapStringTables;
};
