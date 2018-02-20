/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "Report.h"
#include "Util.h"
#include "core.h"
#include "StringX.h"

#include "os/dir.h"
#include "os/debug.h"
#include "os/file.h"
#include "os/utf8conv.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sstream>
#include <fstream>

const TCHAR *CRLF = _T("\r\n");

/*
  It writes a header record and a "Start Report" record.
*/
void CReport::StartReport(LPCTSTR tcAction, const stringT &csDataBase)
{
  m_osxs.str(_T(""));

  m_tcAction = tcAction;
  m_csDataBase = csDataBase;

  stringT cs_title, sTimeStamp;
  PWSUtil::GetTimeStamp(sTimeStamp);
  Format(cs_title, IDSC_REPORT_TITLE1, tcAction, sTimeStamp.c_str());
  WriteLine();
  WriteLine(cs_title);
  Format(cs_title, IDSC_REPORT_TITLE2, csDataBase.c_str());
  WriteLine(cs_title);
  WriteLine();
  LoadAString(cs_title, IDSC_START_REPORT);
  WriteLine(cs_title);
  WriteLine();
}

static bool isFileUnicode(const stringT &fname)
{
  // Check if the first 2 characters are the BOM
  // (Need file to exist and length at least 2 for BOM)
  // Need to use FOpen as cannot pass wchar_t filename to a std::istream
  // and cannot convert a wchar_t filename/path to char if non-Latin characters
  // present
  const unsigned char BOM[] = {0xff, 0xfe};
  unsigned char buffer[] = {0x00, 0x00};
  bool retval = false;

  FILE *fn = pws_os::FOpen(fname, _T("rb"));
  if (fn == nullptr)
    return false;
  if (pws_os::fileLength(fn) < 2)
    retval = false;
  else {
    fread(buffer, 1, 2, fn);
    retval = (buffer[0] == BOM[0] && buffer[1] == BOM[1]);
  }
  fclose(fn);
  return retval;
}

/*
  SaveToDisk creates a new file of name "<tcAction>_Report.txt" e.g. "Merge_Report.txt"
  in the same directory as the current database or appends to this file if it already exists.
*/
bool CReport::SaveToDisk()
{
  FILE *fd;

  stringT path(m_csDataBase);
  stringT drive, dir, file, ext;
  if (!pws_os::splitpath(path, drive, dir, file, ext)) {
    pws_os::IssueError(_T("StartReport: Finding path to database"));
    return false;
  }

  Format(m_cs_filename, IDSC_REPORTFILENAME,
         drive.c_str(), dir.c_str(), m_tcAction.c_str());

  if ((fd = pws_os::FOpen(m_cs_filename, _T("a+b"))) == nullptr) {
    pws_os::IssueError(_T("StartReport: Opening log file"));
    return false;
  }

  // **** MOST LIKELY ACTION ****
  // If file is new/empty, write BOM, as some text editors insist!

  // **** LEAST LIKELY ACTIONS as it requires the user to use both U & NU versions ****
  // Text editors really don't like files with both UNICODE and ASCII characters, so -
  // If we are UNICODE and file is not, convert file to UNICODE before appending

  bool bFileIsUnicode = isFileUnicode(m_cs_filename);

  const unsigned int iBOM = 0xFEFF;
  if (pws_os::fileLength(fd) == 0) {
    // File is empty - write BOM
    putwc(iBOM, fd);
  } else
    if (!bFileIsUnicode) {
      // Convert ASCII contents to UNICODE
      // Close original first
      fclose(fd);

      // Open again to read
      FILE *f_in = pws_os::FOpen(m_cs_filename, _S("rb"));

      // Open new file
      stringT cs_out = m_cs_filename + _S(".tmp");
      FILE *f_out = pws_os::FOpen(cs_out, _S("wb"));

      // Write BOM
      putwc(iBOM, f_out);

      size_t nBytesRead;
      unsigned char inbuffer[4096];
      wchar_t outwbuffer[4096];

      // Now copy
      do {
        nBytesRead = fread(inbuffer, sizeof(inbuffer), 1, f_in);

        if (nBytesRead > 0) {
          size_t len = pws_os::mbstowcs(outwbuffer, 4096, reinterpret_cast<const char *>(inbuffer), nBytesRead);
          if (len != 0)
            fwrite(outwbuffer, sizeof(outwbuffer[0])*len, 1, f_out);
        } else
          break;

      } while(nBytesRead > 0);

      // Close files
      fclose(f_in);
      fclose(f_out);

      // Swap them
      pws_os::RenameFile(cs_out, m_cs_filename);

      // Re-open file
      if ((fd = pws_os::FOpen(m_cs_filename, _S("ab"))) == nullptr) {
        pws_os::IssueError(_T("StartReport: Opening log file"));
        return false;
      }
    }
  // Convert LF to CRLF
  StringX sxCRLF(L"\r\n"), sxLF(L"\n");
  StringX sx = m_osxs.rdbuf()->str();
  Replace(sx, sxCRLF, sxLF);
  Replace(sx, sxLF, sxCRLF);
  
  fwrite(reinterpret_cast<const void *>(sx.c_str()), sizeof(BYTE),
             sx.length() * sizeof(TCHAR), fd);
  fclose(fd);

  return true;
}

// Write a record with (default) or without a CRLF
void CReport::WriteLine(LPCTSTR tc_line, bool bCRLF)
{
  m_osxs << tc_line;
  if (bCRLF) {
    m_osxs << std::endl;
  }
}

// Write a new line
void CReport::WriteLine()
{
  m_osxs << std::endl;
}

//  EndReport writes a "End Report" record and closes the report file.
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
