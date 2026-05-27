/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "Report.h"
#include "Util.h"
#include "core.h"
#include "StringX.h"
#include "UTF8Conv.h"

#include "os/dir.h"
#include "os/debug.h"
#include "os/file.h"
#include "os/utf8conv.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sstream>
#include <fstream>

#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <iomanip>
#include <codecvt>
#include <algorithm>

#include "pugixml/pugixml.hpp"

const std::map<int, LPCTSTR> CReport::ReportNames = {
  {IDSC_RPTCOMPARE, L"Compare"},
  {IDSC_RPTFIND, L"Find"},
  {IDSC_RPTIMPORTTEXT, L"Import_Text"},
  {IDSC_RPTIMPORTXML, L"Import_XML"},
  {IDSC_RPTMERGE, L"Merge"},
  {IDSC_RPTVALIDATE, L"Validate"},
  {IDSC_RPTSYNCH, L"Synchronize"},
  {IDSC_RPTEXPORTTEXT, L"Export_Text"},
  {IDSC_RPTEXPORTXML, L"Export_XML"},
  {IDSC_RPTIMPORTKPV1TXT, L"Import_KeePassV1_TXT"},
  {IDSC_RPTIMPORTKPV1CSV, L"Import_KeePassV1_CSV"},
  {IDSC_RPTEXPORTDB, L"Export_DB"},
};



template<class Facet>
struct deletable_facet : Facet
{
    template<class ...Args>
    deletable_facet(Args&& ...args) : Facet(std::forward<Args>(args)...) {}
    ~deletable_facet() {}
};

/*
  It writes a header record and a "Start Report" record.
*/
void CReport::StartReport(int iAction, const stringT &csDataBase, bool writeHeader)
{
  m_osxs.str(_T(""));

  // iAction refers to the report's untranslated name as defined in ReportNames
  // here we make sure it's in the map - if it's not, that's because a new report was defined and
  // ReportNames wasn't updated (or a silly programming error).
  ASSERT(ReportNames.find(iAction) != ReportNames.end());

  m_iAction = iAction;
  m_csDataBase = csDataBase;

  if(writeHeader) {
    stringT cs_title, sTimeStamp, sAction;

    PWSUtil::GetTimeStamp(sTimeStamp, true);
    LoadAString(sAction, iAction);
    Format(cs_title, IDSC_REPORT_TITLE1, sAction.c_str(), sTimeStamp.c_str());

    WriteLine();
    WriteLine(cs_title);
    Format(cs_title, IDSC_REPORT_TITLE2, csDataBase.c_str());
    WriteLine(cs_title);
    WriteLine();
    LoadAString(cs_title, IDSC_START_REPORT);
    WriteLine(cs_title);
    WriteLine();
  }
}

static pugi::xml_encoding guessBufferEncoding(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  if (d0 == 0 && d1 == 0 && d2 == 0xfe && d3 == 0xff) return pugi::encoding_utf32_be;
  if (d0 == 0xff && d1 == 0xfe && d2 == 0 && d3 == 0) return pugi::encoding_utf32_le;
  if (d0 == 0xfe && d1 == 0xff) return pugi::encoding_utf16_be;
  if (d0 == 0xff && d1 == 0xfe) return pugi::encoding_utf16_le;
  if (d0 == 0xef && d1 == 0xbb && d2 == 0xbf) return pugi::encoding_utf8;
  // no known BOM detected, return auto
  return pugi::encoding_auto;
}

static bool isFileUnicode(const stringT &fname, pugi::xml_encoding& encoding)
{
  // Check if the first 2 characters are the BOM
  // (Need file to exist and length at least 2 for BOM)
  // Need to use FOpen as cannot pass wchar_t filename to a std::istream
  // and cannot convert a wchar_t filename/path to char if non-Latin characters
  // present
  unsigned char buffer[4] = {0x00, 0x00, 0x00, 0x00};
  bool retval = false;

  FILE *fn = pws_os::FOpen(fname, _T("rb"));
  if (fn == nullptr)
    return false;
  if (pws_os::fileLength(fn) < 4) {
    retval = false;
  }
  else {
    if (fread(buffer, 1, 4, fn) != 4) {
      fclose(fn);
      return false;
    }
      
    encoding = guessBufferEncoding(buffer[0], buffer[1], buffer[2], buffer[3]);
    if(encoding == pugi::encoding_auto) {
      encoding = pugi::encoding_utf8; // Take UTF-8 as default
      retval = false;
    }
    else {
      retval = true;
    }
  }
  fclose(fn);
  return retval;
}

/*
  SaveToDisk creates a new file of name "<tcAction>_Report.txt" e.g. "Merge_Report.txt"
  in the same directory as the current database or appends to this file if it already exists.
*/
bool CReport::SaveToDisk(const stringT &out_dir)
{
  FILE *fd;

  stringT path(m_csDataBase);
  stringT drive, dir, file, ext;
  if (!pws_os::splitpath(path, drive, dir, file, ext)) {
    pws_os::IssueError(_T("SaveToDisk: Finding path to database"));
    return false;
  }

  ASSERT(m_iAction != -1); // really a programming error, fail gracefully in non-debug
  if(m_iAction == -1) {
    pws_os::IssueError(_T("SaveToDisk: Action not filled"));
    return false;
  }

  if (!out_dir.empty() && pws_os::FileExists(out_dir)) {
    dir = (out_dir.back() == pws_os::PathSeparator) ? out_dir : out_dir + pws_os::PathSeparator;
  }
  m_cs_filename = BuildPathToReport(drive.c_str(), dir.c_str(), m_iAction);

  if ((fd = pws_os::FOpen(m_cs_filename, _T("a+b"))) == nullptr) {
    pws_os::IssueError(_T("SaveToDisk: Opening log file"));
    return false;
  }

  // Convert LF to CRLF
  StringX sxCRLF(L"\r\n"), sxLF(L"\n");
  StringX sx = m_osxs.rdbuf()->str();
  Replace(sx, sxCRLF, sxLF);
  Replace(sx, sxLF, sxCRLF);
    
  CUTF8Conv conv;
  const unsigned char *utf8 = nullptr;
  size_t utf8Len;
  if (conv.ToUTF8(sx, utf8, utf8Len)) {
    const unsigned char BOM[] = {0xef, 0xbb, 0xbf}; // utf-8 bom
    if (pws_os::fileLength(fd) == 0) {
      // If file is new/empty, write BOM, as some text editors insist!
      fwrite(BOM, 1, 3, fd);
    }
    fwrite(utf8, utf8Len, 1, fd);
  }
  else {
    fclose(fd);
    pws_os::IssueError(_T("SaveToDisk: Conversion error"));
    return false;
  }
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

bool CReport::ReadFromDisk()
{
  FILE *fd;

  stringT path(m_csDataBase);
  stringT drive, dir, file, ext;
  if (!pws_os::splitpath(path, drive, dir, file, ext)) {
    pws_os::IssueError(_T("ReadFromDisk: Finding path to database"));
    return false;
  }

  ASSERT(m_iAction != -1); // really a programming error, fail gracefully in non-debug
  if(m_iAction == -1) {
    pws_os::IssueError(_T("ReadFromDisk: Action not filled"));
    return false;
  }

  m_cs_filename = BuildPathToReport(drive.c_str(), dir.c_str(), m_iAction);

  if ((fd = pws_os::FOpen(m_cs_filename, _T("rb"))) == nullptr) {
    pws_os::IssueError(_T("ReadFromDisk: Opening log file"));
    return false;
  }

  // read the utf-8 file contents into a string stream
  std::ostringstream ss;
  const unsigned char BOM[] = {0xef, 0xbb, 0xbf}; // utf-8 BOM
  const size_t bom = sizeof(BOM);
  char inbuffer[4096];
  size_t nBytesRead = fread(inbuffer, 1, bom, fd);
  if ((nBytesRead == bom) && (memcmp(BOM, inbuffer, bom) == 0)) {
    // ignore the utf-8 bom
  }
  else {
    // those bytes are part of the utf-8 input file
    ss.write(inbuffer, nBytesRead);
  }
  
  do {
    nBytesRead = fread(inbuffer, 1, sizeof(inbuffer), fd);
    if (nBytesRead > 0) {
      ss.write(inbuffer, nBytesRead);
    }
  } while(nBytesRead > 0);
  fclose(fd);
    
  // convert utf-8 to StringX
  StringX sx;
  CUTF8Conv conv;
  std::streampos size = ss.tellp();
  if (!conv.FromUTF8(reinterpret_cast<const unsigned char*>(ss.str().c_str()), size, sx)) {
    pws_os::IssueError(_T("ReadFromDisk: Conversion error"));
    return false;
  }

  StringX sxCRLF(L"\r\n"), sxLF(L"\n");
  Replace(sx, sxCRLF, sxLF);
  m_osxs.str(sx);
    
  return true;
}


bool CReport::PurgeFromDisk()
{
  stringT path(m_csDataBase);
  stringT drive, dir, file, ext;
  if (!pws_os::splitpath(path, drive, dir, file, ext)) {
    pws_os::IssueError(_T("PurgeFromDisk: Finding path to database"));
    return false;
  }

  ASSERT(m_iAction != -1); // really a programming error, fail gracefully in non-debug
  if(m_iAction == -1) {
    pws_os::IssueError(_T("PurgeFromDisk: Action not filled"));
    return false;
  }

  m_cs_filename = BuildPathToReport(drive.c_str(), dir.c_str(), m_iAction);
    
  return pws_os::DeleteAFile(m_cs_filename);
}

bool CReport::ReportExistsOnDisk() const
{
  stringT path(m_csDataBase);
  stringT drive, dir, file, ext;
  if (!pws_os::splitpath(path, drive, dir, file, ext)) {
    pws_os::IssueError(_T("ReportExistsOnDisk: Finding path to database"));
    return false;
  }

  ASSERT(m_iAction != -1); // really a programming error, fail gracefully in non-debug
  if(m_iAction == -1) {
    pws_os::IssueError(_T("ReportExistsOnDisk: Action not filled"));
    return false;
  }

  const stringT filename = BuildPathToReport(drive.c_str(), dir.c_str(), m_iAction);
    
  return pws_os::FileExists(filename);
}

void CReport::AppendPasskeyValidationResults(const std::vector<st_GroupTitleUser> &incomplete)
{
  stringT cs_Error;

  WriteLine();
  LoadAString(cs_Error, IDSC_VALIDATE_PASSKEY);
  WriteLine(cs_Error);
  for_each(incomplete.begin(), incomplete.end(), [&](const st_GroupTitleUser &gtu) {
    Format(cs_Error, IDSC_VALIDATE_ENTRY,
           gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
    WriteLine(cs_Error);
  } );
}

stringT CReport::BuildPathToReport(LPCTSTR drive, LPCTSTR dir, int report_id) {
  stringT result;
  const auto name_iter = ReportNames.find(report_id);

  if (name_iter == ReportNames.end()) {
    ASSERT(false);
    pws_os::IssueError(_T("Uexpected report id"));
    return result;
  }

  Format(result, IDSC_REPORTFILENAME, drive, dir, name_iter->second);
  return result;
}

const stringT CReport::GetDatabasePath() const
{
  stringT path(m_csDataBase);
  stringT drive, dir, file, ext;
  if (!pws_os::splitpath(path, drive, dir, file, ext)) {
    pws_os::IssueError(_T("GetDatabasePath: Finding path to database"));
    return L"";
  }
  return dir;
}
