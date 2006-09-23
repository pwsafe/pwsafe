#pragma once

class CPWSRecentFileList : public CRecentFileList
{
public:
	CPWSRecentFileList(UINT nStart, LPCTSTR lpszSection,
		LPCTSTR lpszEntryFormat, int nSize,
		int nMaxDispLen = AFX_ABBREV_FILENAME_LEN)
	: CRecentFileList(nStart, lpszSection, lpszEntryFormat,
        		nSize, nMaxDispLen){}

	virtual void Add(LPCTSTR lpszPathName);
};
