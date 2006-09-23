#include "stdafx.h"
#include <afxadv.h>
#include "Shlwapi.h"
#include "PWSRecentFileList.h"

void CPWSRecentFileList::Add(LPCTSTR lpszPathName)
{
	ASSERT(m_arrNames != NULL);
	ASSERT(AfxIsValidString(lpszPathName));

	// Do nothing if string is not there
	if ( lpszPathName == NULL || lstrlen(lpszPathName) == 0 )
		return;

	TCHAR szTemp[_MAX_PATH];
	DWORD dwRet = GetFullPathName(lpszPathName, _MAX_PATH, szTemp, NULL);
	if (dwRet == 0 || dwRet >= _MAX_PATH) {
		// error - return without doing anything
		return;
	}

	// determine the root name of the volume
	CString csRoot;
	LPTSTR lpszRoot = csRoot.GetBuffer(_MAX_PATH);
	memset(lpszRoot, 0, _MAX_PATH);
#if _MSC_VER >= 1400
	_tcsncpy_s(lpszRoot, _MAX_PATH, lpszPathName, _TRUNCATE);
#else
	_tcsncpy(lpszRoot, lpszPathName, _MAX_PATH);
#endif
	::PathStripToRoot(lpszRoot);
	csRoot.ReleaseBuffer();

	bool bGetMSToAddIt;
	if (!::PathIsUNC(csRoot)) {
		// hopefully it is in the form "<Drive letter>:\"
		if (csRoot.GetLength() != 3 || csRoot.Right(2) != _T(":\\")) {
			// error - return without doing anything
			return;
		}

		// Stop system asking for a volume (floppy or CD) to be inserted in the drive
		// when we enquire about the volume!
		UINT uMode = SetErrorMode(SEM_FAILCRITICALERRORS);

		// get file system information for the volume
		DWORD dwFlags, dwDummy;
		if (!GetVolumeInformation(csRoot, NULL, 0, NULL, &dwDummy, &dwFlags, NULL, 0)) {
			// error - we add it as the vloume is not available at the moment!
			bGetMSToAddIt = false;
		} else {
			// Let MS add it
			bGetMSToAddIt = true;
		}
		// Reset ErrorMode
		SetErrorMode(uMode);
	} else {
		// Again - let MS add it
		bGetMSToAddIt = true;
	}

	if (bGetMSToAddIt) {
		CRecentFileList::Add(lpszPathName);
		return;
	}

	// We have to do it!!!
	// update the MRU list, if an existing MRU string matches file name
	int i;
	for (i = 0; i < m_nSize - 1; i++) {
		if (lstrcmpi(m_arrNames[i], szTemp) == 0)
			break;      // i will point to matching entry
	}

	// move MRU strings before this one down
	for (; i > 0; i--) {
		ASSERT(i > 0);
		ASSERT(i < m_nSize);
		m_arrNames[i] = m_arrNames[i-1];
	}

	// place this one at the beginning
	m_arrNames[0] = lpszPathName;
}
