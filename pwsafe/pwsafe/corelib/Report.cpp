/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

#include "Report.h"
#include "Util.h"
#include "corelib.h"

#include <stdio.h>
#include <stdlib.h>
#include <share.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const TCHAR *CRLF = _T("\r\n");

/*
   StartReport creates a new file of name "<tcAction>_Report.txt" e.g. "Merge_Report.txt"
   in the same directory as the current database.
   
   It writes a header record and a "Start Report" record.
 */
bool
CReport::StartReport(LPTSTR tcAction, const CString &csDataBase)
{
  if (m_fd != NULL) {
    fclose(m_fd);
    m_fd = NULL;
  }

  CString cs_filename;
  TCHAR tc_drive[_MAX_DRIVE];
  TCHAR tc_dir[_MAX_DIR];
  errno_t err;

#if _MSC_VER >= 1400
  err = _tsplitpath_s(csDataBase, tc_drive, _MAX_DRIVE, tc_dir, _MAX_DIR, NULL, 0, NULL, 0);
  if (err != 0) {
    PWSUtil::IssueError(_T("StartReport: Finding path to database"));
    return false;
  }
#else
  _tsplitpath(csDataBase, sz_drive, sz_dir, NULL, NULL);
#endif

  cs_filename.Format(IDSC_REPORTFILENAME, tc_drive, tc_dir, tcAction);

  if ((m_fd = _tfsopen((LPCTSTR) cs_filename, _T("ab"), _SH_DENYWR)) == NULL) {
  	PWSUtil::IssueError(_T("StartReport: Opening log file"));
  	return false;
  }

  CString cs_title;
  cs_title.Format(IDSC_REPORT_TITLE, tcAction, csDataBase, PWSUtil::GetTimeStamp());
  WriteLine();
  WriteLine(cs_title);
  WriteLine();
  cs_title.LoadString(IDSC_START_REPORT);
  WriteLine(cs_title);
  WriteLine();
  
  return true;
}

// Write a record with(default) or without a CRLF
void
CReport::WriteLine(CString &cs_line, bool bCRLF)
{
  if (m_fd == NULL)
    return;

  LPTSTR tc_line = cs_line.GetBuffer(cs_line.GetLength() + sizeof(TCHAR));
#if _MSC_VER >= 1400
	_ftprintf_s(m_fd, _T("%s"), tc_line);
  if (bCRLF)
    _ftprintf_s(m_fd, _T("%s"), CRLF);
#else
	_ftprintf(m_fd, _T("%s"), tc_line);
  if (bCRLF)
    _ftprintf(m_fd, _T("%s"), CRLF);
#endif
	cs_line.ReleaseBuffer();
}

// Write a record with(default) or without a CRLF
void
CReport::WriteLine(LPTSTR &tc_line, bool bCRLF)
{
  if (m_fd == NULL)
    return;

#if _MSC_VER >= 1400
	_ftprintf_s(m_fd, _T("%s"), tc_line);
  if (bCRLF)
    _ftprintf_s(m_fd, _T("%s"), CRLF);
#else
	_ftprintf(m_fd, _T("%s"), tc_line);
  if (bCRLF)
    _ftprintf(m_fd, _T("%s"), CRLF);
#endif
}

// Write a new line
void
CReport::WriteLine()
{
  if (m_fd == NULL)
    return;

#if _MSC_VER >= 1400
  _ftprintf_s(m_fd, _T("%s"), CRLF);
#else
  _ftprintf(m_fd, _T("%s"), CRLF);
#endif
}

/*
   EndReport writes a "End Report" record and closes the report file.
 */
void
CReport::EndReport()
{
  WriteLine();
  CString cs_title;
  cs_title.LoadString(IDSC_END_REPORT);
  WriteLine(cs_title);

  if (m_fd != NULL)
    fclose(m_fd);
}
