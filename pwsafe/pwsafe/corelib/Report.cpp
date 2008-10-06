/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "Report.h"
#include "Util.h"
#include "Debug.h"
#include "corelib.h"
#include "os/dir.h"
#include "StringX.h"

#include <stdio.h>
#include <stdlib.h>
#include <share.h>
#include <sys/stat.h>
#include <errno.h>

#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const TCHAR *CRLF = _T("\r\n");

CReport::~CReport()
{
  free(m_tcAction);
}
/*
  It writes a header record and a "Start Report" record.
*/
void CReport::StartReport(LPCTSTR tcAction, const stringT &csDataBase)
{
  m_osxs.str(_T(""));

  m_tcAction = _tcsdup(tcAction);
  m_csDataBase = csDataBase;

  stringT cs_title;
  Format(cs_title, IDSC_REPORT_TITLE1, tcAction,
                  PWSUtil::GetTimeStamp());
  WriteLine();
  WriteLine(cs_title);
  Format(cs_title, IDSC_REPORT_TITLE2, csDataBase.c_str());
  WriteLine(cs_title);
  WriteLine();
  LoadAString(cs_title, IDSC_START_REPORT);
  WriteLine(cs_title);
  WriteLine();
}

/*
  SaveToDisk creates a new file of name "<tcAction>_Report.txt" e.g. "Merge_Report.txt"
  in the same directory as the current database or appends to this file if it already exists.
*/
bool CReport::SaveToDisk()
{
  if (m_pdfile != NULL) {
    fclose(m_pdfile);
    m_pdfile = NULL;
  }

  stringT path(m_csDataBase);
  stringT drive, dir, file, ext;
  if (!pws_os::splitpath(path, drive, dir, file, ext)) {
    PWSDebug::IssueError(_T("StartReport: Finding path to database"));
    return false;
  }

  Format(m_cs_filename, IDSC_REPORTFILENAME,
         drive.c_str(), dir.c_str(), m_tcAction);

  if ((m_pdfile = _tfsopen(m_cs_filename.c_str(),
                           _T("a+b"), _SH_DENYWR)) == NULL) {
    PWSDebug::IssueError(_T("StartReport: Opening log file"));
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

  ::_tstat(m_cs_filename.c_str(), &statbuf);

  // No need to check result of _tstat, since the _tfsopen above would have failed if 
  // the file/directory did not exist
  if (statbuf.st_size >= 2) {
    // Has data - but is it UNICODE or not?
    fpos_t pos;
    BYTE buffer[] = {0x00, 0x00, 0x00};

    fgetpos(m_pdfile, &pos);
    rewind(m_pdfile);

    int numread = fread(buffer, sizeof(BYTE), 2, m_pdfile);
    ASSERT(numread == 2);

    if (buffer[0] == 0xff && buffer[1] == 0xfe) {
      // BOM present - File is UNICODE
      bFileIsUnicode = true;
    }
    fsetpos(m_pdfile, &pos);
  }

#ifdef UNICODE
  const unsigned int iBOM = 0xFEFF;
  if (statbuf.st_size == 0) {
    // File is empty - write BOM
    putwc(iBOM, m_pdfile);
  } else
    if (!bFileIsUnicode) {
      // Convert ASCII contents to UNICODE
      FILE *f_in, *f_out;

      // Close original first
      fclose(m_pdfile);

      // Open again to read
      f_in = _wfsopen(m_cs_filename.c_str(), L"rb", _SH_DENYWR);

      // Open new file
      stringT cs_out = m_cs_filename + _T(".tmp");
      f_out = _wfsopen(cs_out.c_str(), L"wb", _SH_DENYWR);

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
      _tremove(m_cs_filename.c_str());
      _trename(cs_out.c_str(), m_cs_filename.c_str());

      // Re-open file
      if ((m_pdfile = _wfsopen(m_cs_filename.c_str(),
                               L"ab", _SH_DENYWR)) == NULL) {
        PWSDebug::IssueError(_T("StartReport: Opening log file"));
        return false;
      }
    }
#else
  if (bFileIsUnicode) {
    // Convert UNICODE contents to ASCII
    FILE *f_in, *f_out;

    // Close original first
    fclose(m_pdfile);

    // Open again to read
    f_in = _fsopen(m_cs_filename.c_str(), "rb", _SH_DENYWR);

    // Open new file
    stringT cs_out = m_cs_filename + _T(".tmp");
    f_out = _fsopen(cs_out.c_str(), "wb", _SH_DENYWR);

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
    _tremove(m_cs_filename.c_str());
    _trename(cs_out.c_str(), m_cs_filename.c_str());

    // Re-open file
    if ((m_pdfile = _fsopen(m_cs_filename.c_str(), "ab", _SH_DENYWR)) == NULL) {
      PWSDebug::IssueError(_T("StartReport: Opening log file"));
      return false;
    }
  }
#endif
  StringX sx = m_osxs.rdbuf()->str();
  fwrite((void *)sx.c_str(), sizeof(BYTE), sx.length() * sizeof(TCHAR), m_pdfile);
  fclose(m_pdfile);

  return true;
}

// Write a record with(default) or without a CRLF
void CReport::WriteLine(const stringT &cs_line, bool bCRLF)
{
  m_osxs << cs_line.c_str();
  if (bCRLF) {
    m_osxs << CRLF;
  }
}

// Write a record with(default) or without a CRLF
void CReport::WriteLine(const LPTSTR &tc_line, bool bCRLF)
{
  m_osxs << tc_line;
  if (bCRLF) {
    m_osxs << CRLF;
  }
}

// Write a new line
void CReport::WriteLine()
{
  m_osxs << CRLF;
}

/*
  EndReport writes a "End Report" record and closes the report file.
*/
void CReport::EndReport()
{
  WriteLine();
  stringT cs_title;
  LoadAString(cs_title, IDSC_END_REPORT1);
  WriteLine(cs_title);
  LoadAString(cs_title, IDSC_END_REPORT2);
  WriteLine(cs_title);

  m_osxs.flush();
}
