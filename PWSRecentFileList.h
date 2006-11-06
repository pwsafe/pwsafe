/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

class CPWSRecentFileList : public CRecentFileList
{
public:
	CPWSRecentFileList(UINT nStart, LPCTSTR lpszSection,
		LPCTSTR lpszEntryFormat, int nSize,
		int nMaxDispLen = AFX_ABBREV_FILENAME_LEN)
	: CRecentFileList(nStart, lpszSection, lpszEntryFormat,
        		nSize, nMaxDispLen){}

	virtual void Add(LPCTSTR lpszPathName, const bool bstartup = false);
};
