/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "Report.h"
#include "Util.h"
#include "corelib.h"

#include <stdio.h>
#include <stdlib.h>
#include <share.h>
#include <sys/stat.h>
#include <errno.h>

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

  m_cs_filename.Format(IDSC_REPORTFILENAME, tc_drive, tc_dir, tcAction);

  if ((m_fd = _tfsopen((LPCTSTR) m_cs_filename, _T("a+b"), _SH_DENYWR)) == NULL) {
  	PWSUtil::IssueError(_T("StartReport: Opening log file"));
  	return false;
  }

  // **** MOST LIKELY ACTION ****
  // If file is new/emtpy AND we are UNICODE, write BOM, as some text editors insist!

  // **** LEAST LIKELY ACTIONS as it requires the user to use both U & NU versions ****
  // Test editors really don't like files with both UNICODE and ASCII characters, so -
  // If we are UNICODE and file is not, convert file to UNICODE before appending
  // If we are not UNICODE but file is, convert file to ASCII before appending
  
  bool bFileIsUnicode(false);

  struct _stat statbuf;
  
  ::_tstat(m_cs_filename, &statbuf);

  // No need to check result of _tstat, since the _tfsopen above would have failed if 
  // the file/directory did not exist
  if (statbuf.st_size >= 2) {
    // Has data - but is it UNICODE or not?
    fpos_t pos;
    BYTE buffer[] = {0x00, 0x00, 0x00};

    fgetpos(m_fd, &pos);
    rewind(m_fd);

    int numread = fread(buffer, sizeof(BYTE), 2, m_fd);
    ASSERT(numread == 2);

    if (buffer[0] == 0xff && buffer[1] == 0xfe) {
      // BOM present - File is UNICODE
      bFileIsUnicode = true;
    }
    fsetpos(m_fd, &pos);
  }

#ifdef UNICODE
  const unsigned int iBOM = 0xFEFF;
  if (statbuf.st_size == 0) {
    // File is empty - write BOM
	  putwc(iBOM, m_fd);
  } else
  if (!bFileIsUnicode) {
    // Convert ASCII contents to UNICODE
    FILE *f_in, *f_out;

    // Close original first
    fclose(m_fd);

    // Open again to read
    f_in = _wfsopen((LPCWSTR)m_cs_filename, L"rb", _SH_DENYWR);

    // Open new file
    CString cs_out = m_cs_filename + _T(".tmp");
    f_out = _wfsopen((LPCWSTR)cs_out, L"wb", _SH_DENYWR);

    // Write BOM
    putwc(iBOM, f_out);

    UINT nBytesRead;
    unsigned char inbuffer[4096];
    WCHAR outwbuffer[4096];

    // Now copy
    do {
      nBytesRead = fread(inbuffer, sizeof(inbuffer), 1, f_in);

      if (nBytesRead > 0) {
        int len = MultiByteToWideChar(CP_ACP, 0, (LPSTR)inbuffer, 
                      nBytesRead, (LPWSTR)outwbuffer, 4096);
        if (len > 0)
          fwrite(outwbuffer, sizeof(outwbuffer[0])*len, 1, f_out);
      } else
        break;

    } while(nBytesRead > 0);

    // Close files
    fclose(f_in);
    fclose(f_out);

    // Swap them
    _tremove(m_cs_filename);
    _trename(cs_out, m_cs_filename);

    // Re-open file
    if ((m_fd = _wfsopen((LPCTSTR)m_cs_filename, L"ab", _SH_DENYWR)) == NULL) {
  	  PWSUtil::IssueError(_T("StartReport: Opening log file"));
    	return false;
    }
  }
#else
  if (bFileIsUnicode) {
    // Convert UNICODE contents to ASCII
    FILE *f_in, *f_out;

    // Close original first
    fclose(m_fd);

    // Open again to read
    f_in = _fsopen((LPCSTR)m_cs_filename, "rb", _SH_DENYWR);

    // Open new file
    CString cs_out = m_cs_filename + _T(".tmp");
    f_out = _fsopen((LPCSTR)cs_out, "wb", _SH_DENYWR);

    UINT nBytesRead;
    WCHAR inwbuffer[4096];
    unsigned char outbuffer[4096];

    // Skip over BOM
    fseek(f_in, 2, SEEK_SET);

    // Now copy
    do {
      nBytesRead = fread(inwbuffer, sizeof(inwbuffer[0])*sizeof(inwbuffer),
                         1, f_in);

      if (nBytesRead > 0) {
        int len = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)inwbuffer, 
                      nBytesRead,
                      (LPSTR)outbuffer, 4096, NULL, NULL);
        if (len > 0)
          fwrite(outbuffer, len, 1, f_out);
      } else
        break;

    } while(nBytesRead > 0);

    // Close files
    fclose(f_in);
    fclose(f_out);

    // Swap them
    _tremove(m_cs_filename);
    _trename(cs_out, m_cs_filename);

    // Re-open file
    if ((m_fd = _fsopen((LPCSTR) m_cs_filename, "ab", _SH_DENYWR)) == NULL) {
  	  PWSUtil::IssueError(_T("StartReport: Opening log file"));
    	return false;
    }
  }
#endif

  CString cs_title;
  cs_title.Format(IDSC_REPORT_TITLE1, tcAction, PWSUtil::GetTimeStamp());
  WriteLine();
  WriteLine(cs_title);
  cs_title.Format(IDSC_REPORT_TITLE2, csDataBase);
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
  cs_title.LoadString(IDSC_END_REPORT1);
  WriteLine(cs_title);
  cs_title.LoadString(IDSC_END_REPORT2);
  WriteLine(cs_title);

  if (m_fd != NULL)
    fclose(m_fd);
}
