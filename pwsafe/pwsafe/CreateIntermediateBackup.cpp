/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// file CreateIntermediateBackup.cpp

#include "PasswordSafe.h"

#include "DboxMain.h"

#include "corelib/PWSprefs.h"
#include "corelib/Util.h"

#include <shellapi.h>

#include <vector>
#include <algorithm>
#include <shlwapi.h>

void
DboxMain::CreateIntermediateBackup()
{
	int i_backuplocation, i_backupprefix, i_backupsuffix, i_maxnumincbackups;
	CString cs_userbackupprefix, cs_userbackupsubdirectory, cs_userbackupotherlocation;
	CString cs_temp, cs_currentfile(m_core.GetCurFile()), cs_newfile;

	PWSprefs *prefs = PWSprefs::GetInstance();

	i_backupprefix = prefs->
		GetPref(PWSprefs::BackupPrefix);
	i_backupsuffix = prefs->
		GetPref(PWSprefs::BackupSuffix);
	i_backuplocation = prefs->
		GetPref(PWSprefs::BackupLocation);
	i_maxnumincbackups = prefs->
		GetPref(PWSprefs::BackupMaxIncremented);
	cs_userbackupprefix = CString(prefs->
		GetPref(PWSprefs::BackupPrefixValue));
	cs_userbackupsubdirectory = CString(prefs->
		GetPref(PWSprefs::BackupSubDirectoryValue));
	cs_userbackupotherlocation = CString(prefs->
		GetPref(PWSprefs::BackupOtherLocationValue));

	// Get location for intermediate backup
	if (i_backuplocation < 2) {
		// Get directory containing database
		cs_temp = cs_currentfile;
		TCHAR *lpszTemp = cs_temp.GetBuffer(_MAX_PATH);
		PathRemoveFileSpec(lpszTemp);
		cs_temp.ReleaseBuffer();
		cs_temp += _T("\\");
		if (i_backuplocation == 1)
			cs_temp += cs_userbackupsubdirectory + _T("\\");
	} else {
		cs_temp = cs_userbackupotherlocation;
	}

	// generate prefix of intermediate backup file name
	if (i_backupprefix == 0) {
		TCHAR fname[_MAX_FNAME];

#if _MSC_VER >= 1400
		_tsplitpath_s( cs_currentfile, NULL, 0, NULL, 0, fname, _MAX_FNAME, NULL, 0 );
#else
		_tsplitpath( cs_currentfile, NULL, NULL, fname, NULL );
#endif
		cs_temp += CString(fname);
	} else {
		cs_temp += cs_userbackupprefix;
	}

	// Add on suffix
	switch (i_backupsuffix) {
		case 1:
			{
				time_t now;
				time(&now);
				CString cs_datetime = (CString)PWSUtil::ConvertToDateTimeString(now, TMC_EXPORT_IMPORT);
				cs_temp += _T("_");
				cs_newfile = cs_temp + cs_datetime.Left(4) +	// YYYY
									cs_datetime.Mid(5,2) +	// MM
									cs_datetime.Mid(8,2) +	// DD
									_T("_") +
									cs_datetime.Mid(11,2) +	// HH
									cs_datetime.Mid(14,2) +	// MM
									cs_datetime.Mid(17,2);	// SS
			}
			break;
		case 2:
			if (GetIncBackupFileName(cs_temp, i_maxnumincbackups, cs_newfile) == FALSE) {
				AfxMessageBox(_T("Unable to create intermediate backup. Feature disabled."), MB_OK);
				prefs->SetPref(PWSprefs::BackupBeforeEverySave, false);
				return;
			}
			break;
		case 0:
		default:
			cs_newfile = cs_temp;
			break;
	}

	cs_newfile +=  _T(".ibak");

	// Now copy file and create any intervening directories as necessary & automatically
	TCHAR szSource[_MAX_PATH];
	TCHAR szDestination[_MAX_PATH];

	TCHAR *lpsz_current = cs_currentfile.GetBuffer(_MAX_PATH);
	TCHAR *lpsz_new = cs_newfile.GetBuffer(_MAX_PATH);
#if _MSC_VER >= 1400
	_tcscpy_s(szSource, _MAX_PATH, lpsz_current);
	_tcscpy_s(szDestination, _MAX_PATH, lpsz_new);
#else
	_tcscpy(szSource, lpsz_current);
	_tcscpy(szDestination, lpsz_new);
#endif
	cs_currentfile.ReleaseBuffer();
	cs_newfile.ReleaseBuffer();

	// Must end with double NULL
	szSource[cs_currentfile.GetLength() + 1] = '\0';
	szDestination[cs_newfile.GetLength() + 1] = '\0';

	SHFILEOPSTRUCT sfop;
	memset(&sfop, 0, sizeof(sfop));
	sfop.hwnd = GetActiveWindow()->m_hWnd;
	sfop.wFunc = FO_COPY;
	sfop.pFrom = szSource;
	sfop.pTo = szDestination;
	sfop.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_SILENT;

	if (SHFileOperation(&sfop) != 0) {
		AfxMessageBox(_T("Unable to create intermediate backup. Feature disabled."), MB_OK);
		prefs->SetPref(PWSprefs::BackupBeforeEverySave, false);
	}
}

BOOL
DboxMain::GetIncBackupFileName(const CString &cs_filenamebase,
							   const int &i_maxnumincbackups,
							   CString &cs_newname)
{
	CString cs_filenamemask(cs_filenamebase), cs_filename, cs_ibak_number;
	CFileFind finder;
	BOOL bWorking, brc(TRUE);
	int num_found(0), n;
	std::vector<int> file_nums;

	cs_filenamemask += _T("_???.ibak");

	bWorking = finder.FindFile(cs_filenamemask);
	while (bWorking) {
		bWorking = finder.FindNextFile();
		num_found++;
		cs_filename = finder.GetFileName();
		cs_ibak_number = cs_filename.Mid(cs_filename.GetLength() - 8, 3);
		if (cs_ibak_number.SpanIncluding(CString("0123456789")) != cs_ibak_number)
			continue;
		n = atoi(cs_ibak_number);
		file_nums.push_back(n);
	}

	if (num_found == 0) {
		cs_newname = cs_filenamebase + _T("_001");
		return brc;
	}

	sort(file_nums.begin(), file_nums.end());

	int &nnn = file_nums.back();
	nnn++;
	if (nnn > 999) nnn = 1;

	cs_newname.Format(_T("%s_%03d"), cs_filenamebase, nnn);

	int i = 0;
	while (num_found >= i_maxnumincbackups) {
		nnn = file_nums.at(i);
		cs_filename.Format(_T("%s_%03d.ibak"), cs_filenamebase, nnn);
		brc = DeleteFile(cs_filename);
		if (brc == FALSE) {
			PWSUtil::IssueError(_T("DeleteFile"));
			break;
		}
		i++;
		num_found--;
	}

	return brc;
}
