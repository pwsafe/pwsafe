/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#include "stdafx.h"
#include <afxadv.h>
#include "Shlwapi.h"
#include "PWSRecentFileList.h"
#include "corelib/PWSprefs.h"
#include "resource2.h" // for ID_FILE_MRU_*
	/*
	   NOTE: Using a DEBUG version of the executable, adding a file whose path 
	   does not exist (e.g. on USB stick that is no longer connected to the 
	   computer) will cause an ASSERT failure in filelist.cpp and
	   then a CFileException (badpath).

	   Tried to get MS to just verify the filename is correctly formed but they insist
	   in testing the volume is also mounted! They MAY fix it in a later version of VC8.

	   The ASSERT does not occur in the Release version of the executable but the
	   CFileException does - hence this code to still Add the filename.  If the volume
	   is mounted, we pass it on to MS's code to Add.

	   Note: If this happens, with MS's code, the valid filename will NOT be added
	   to the MRU list, although it would if read from the registry
	   (CRecentFileList::ReadList) since MS does NO checking in that routine and
	   will add any string!
	*/

void CPWSRecentFileList::Add(LPCTSTR lpszPathName, const bool bstartup)
{
	ASSERT(m_arrNames != NULL);
	ASSERT(AfxIsValidString(lpszPathName));

	// Do nothing if string is not there
	if ( lpszPathName == NULL || lstrlen(lpszPathName) == 0 )
		return;

	if (!bstartup) {
		// Get MS to Add it.
		CRecentFileList::Add(lpszPathName);
		return;
	}

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
		// Get MS to Add it.
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


void CPWSRecentFileList::ReadList()
{
    PWSprefs *pref = PWSprefs::GetInstance();
    // reads from registry or config file
    if (pref->IsUsingRegistry()) {
        CRecentFileList::ReadList();
    } else {
        const int nMRUItems = pref->GetPref(PWSprefs::MaxMRUItems);
        CString *csMRUFiles = new CString[nMRUItems];
        pref->GetMRUList(csMRUFiles);
        for (int i = nMRUItems; i > 0 ; i--)
            Add(csMRUFiles[i], true);
        delete[] csMRUFiles;
    }
}

void CPWSRecentFileList::WriteList()
{
    PWSprefs *pref = PWSprefs::GetInstance();
    // writes to registry or config file
    if (pref->IsUsingRegistry()) {
        CRecentFileList::WriteList();
    } else {
        const int num_MRU = GetSize();
        const int max_MRU = ID_FILE_MRU_ENTRYMAX - ID_FILE_MRU_ENTRY1;
        CString *csMRUFiles = new CString[num_MRU];

        for (int i = 0; i < num_MRU; i++) {
            csMRUFiles[i] = (*this)[i];
            csMRUFiles[i].Trim();
        }

        pref->SetMRUList(csMRUFiles, num_MRU, max_MRU);
        delete[] csMRUFiles;
    }
}
