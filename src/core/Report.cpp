/*
* Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

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

#include "pugixml/pugixml.hpp"


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
void CReport::StartReport(LPCTSTR tcAction, const stringT &csDataBase, bool writeHeader)
{
  m_osxs.str(_T(""));

  m_tcAction = tcAction;
  m_csDataBase = csDataBase;

  if(writeHeader) {
    stringT cs_title, sTimeStamp, sAction;
    PWSUtil::GetTimeStamp(sTimeStamp, true);
    sAction = _(tcAction);
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
    fread(buffer, 1, 4, fn);
      
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
bool CReport::SaveToDisk()
{
  FILE *fd;

  stringT path(m_csDataBase);
  stringT drive, dir, file, ext;
  if (!pws_os::splitpath(path, drive, dir, file, ext)) {
    pws_os::IssueError(_T("SaveToDisk: Finding path to database"));
    return false;
  }
    
  if(m_tcAction.empty()) {
    pws_os::IssueError(_T("SaveToDisk: Action not filled"));
    return false;
  }

  Format(m_cs_filename, IDSC_REPORTFILENAME,
         drive.c_str(), dir.c_str(), m_tcAction.c_str());

  if ((fd = pws_os::FOpen(m_cs_filename, _T("a+b"))) == nullptr) {
    pws_os::IssueError(_T("SaveToDisk: Opening log file"));
    return false;
  }

  // **** MOST LIKELY ACTION ****
  // If file is new/empty, write BOM, as some text editors insist!

  // **** LEAST LIKELY ACTIONS as it requires the user to use both U & NU versions ****
  // Text editors really don't like files with both UNICODE and ASCII characters, so -
  // If we are UNICODE and file is not, convert file to UNICODE before appending

  pugi::xml_encoding encoding;
  bool bFileIsUnicode = isFileUnicode(m_cs_filename, encoding);

  const unsigned char BOM[] = {0xef, 0xbb, 0xbf}; // Store UTF-8 as default
  if (pws_os::fileLength(fd) == 0) {
    // File is empty - write BOM for UTF-8
    fwrite(BOM, 1, 3, fd);
  } else {
    if (!bFileIsUnicode) {
      // Convert ASCII contents to UTF-8 (only adding inital BOM)
      // Close original first
      fclose(fd);

      // Open again to read
      FILE *f_in = pws_os::FOpen(m_cs_filename, _S("rb"));

      // Open new file
      stringT cs_out = m_cs_filename + _S(".tmp");
      FILE *f_out = pws_os::FOpen(cs_out, _S("wb"));
        
      if(f_out == nullptr) {
        fclose(f_in);
        pws_os::IssueError(_T("SaveToDisk: Opening tmp log file"));
        return false;
      }

      // Write BOM for UTF8
      fwrite(BOM, 1, 3, f_out);

      size_t nBytesRead;
      unsigned char inbuffer[4096];

      // Now copy
      do {
        nBytesRead = fread(inbuffer, 1, sizeof(inbuffer), f_in);

        if (nBytesRead > 0) {
          fwrite(inbuffer, nBytesRead, 1, f_out);
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
        pws_os::IssueError(_T("SaveToDisk: Opening log file"));
        return false;
      }
    }
    else if(encoding != pugi::encoding_utf8) {
      // Convert different coded contents to UTF-8
      // Close original first
      fclose(fd);

      // Open again to read
      FILE *f_in = pws_os::FOpen(m_cs_filename, _S("rb"));

      // Open new file
      stringT cs_out = m_cs_filename + _S(".tmp");
      FILE *f_out = pws_os::FOpen(cs_out, _S("wb"));
        
      if(f_out == nullptr) {
        fclose(f_in);
        pws_os::IssueError(_T("SaveToDisk: Opening tmp log file"));
        return false;
      }
        
      // Write BOM for UTF8
      fwrite(BOM, 1, 3, f_out);

      size_t nBytesRead;
      unsigned char inbuffer[4096];
        
      if((encoding == pugi::encoding_utf16_le) || (encoding == pugi::encoding_utf16_be) || (encoding == pugi::encoding_utf16)) {
        // Skip 2 byte header
        fread(inbuffer, 1, 2, f_in);
      }
      else if((encoding == pugi::encoding_utf16_le) || (encoding == pugi::encoding_utf16_be) || (encoding == pugi::encoding_utf16)) {
        // Skip 4 byte header
        fread(inbuffer, 1, 4, f_in);
      }

      // Now copy and convert
      do {
        // Read from UTF-16 or UTF-32 coded file
        nBytesRead = fread(inbuffer, 1, sizeof(inbuffer), f_in);

        if (nBytesRead > 0) {
            
            std::wstring_convert<
                deletable_facet<std::codecvt<char16_t, char, std::mbstate_t>>, char16_t> conv16;
          // get private buffer
          wchar_t* buffer = 0;
          size_t length = 0;
          // Convert first from UTF-16 or UTF-32 to machine wchar_t
          if(pugi::convertBuffer(buffer, length, encoding, inbuffer, nBytesRead, true)) {
            // Convert back to UTF-8
            size_t dstlen = pws_os::wcstombs(nullptr, 0, buffer, length);
            ASSERT(dstlen > 0);
            char *dst = new char[dstlen+1];
            dstlen = pws_os::wcstombs(dst, dstlen, buffer, length);
            ASSERT(dstlen != size_t(-1));
            if (dstlen && !dst[dstlen-1])
              dstlen--;
            // Write UTF-8 content
            fwrite(dst, dstlen, 1, f_out);
            delete[] dst;
            (*pugi::get_memory_deallocation_function())(buffer);
          }
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
        pws_os::IssueError(_T("SaveToDisk: Opening log file"));
        return false;
      }
    }
  }
  // Convert LF to CRLF
  StringX sxCRLF(L"\r\n"), sxLF(L"\n");
  StringX sx = m_osxs.rdbuf()->str();
  Replace(sx, sxCRLF, sxLF);
  Replace(sx, sxLF, sxCRLF);
    
  CUTF8Conv conv; // can't make a member, as no copy c'tor!
  const unsigned char *utf8;
  size_t utf8Len;
  if (conv.ToUTF8(sx.c_str(), utf8, utf8Len)) {
    fwrite(utf8, utf8Len, 1, fd);
  }
  else {
    pws_os::IssueError(_T("SaveToDisk: Conversion error"));
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
    
  if(m_tcAction.empty()) {
    pws_os::IssueError(_T("ReadFromDisk: Action not filled"));
    return false;
  }

  Format(m_cs_filename, IDSC_REPORTFILENAME,
         drive.c_str(), dir.c_str(), m_tcAction.c_str());

  if ((fd = pws_os::FOpen(m_cs_filename, _T("rb"))) == nullptr) {
    pws_os::IssueError(_T("ReadFromDisk: Opening log file"));
    return false;
  }

  pugi::xml_encoding encoding;
  bool bFileIsUnicode = isFileUnicode(m_cs_filename, encoding);
  size_t nBytesRead;
  wchar_t inbuffer[4096 / sizeof(wchar_t)];

  if (bFileIsUnicode) {
    if((encoding == pugi::encoding_utf16_le) || (encoding == pugi::encoding_utf16_be) || (encoding == pugi::encoding_utf16)) {
      // Skip 2 byte header
      fread(inbuffer, 1, 2, fd);
    }
    else if((encoding == pugi::encoding_utf16_le) || (encoding == pugi::encoding_utf16_be) || (encoding == pugi::encoding_utf16)) {
      // Skip 4 byte header
      fread(inbuffer, 1, 4, fd);
    }
    else if(encoding == pugi::encoding_utf8) {
      // Skip 3 byte header
      fread(inbuffer, 1, 3, fd);
    }
  }
  
  // Reset buffer
  m_osxs.str(_T(""));

  // Now copy
  do {
    nBytesRead = fread(inbuffer, 1, sizeof(inbuffer), fd);

    if (nBytesRead > 0) {
      // get private buffer
      wchar_t* buffer = 0;
      size_t length = 0;
      // Convert first from UTF-8, UTF-16 or UTF-32 to machine wchar_t
      if(pugi::convertBuffer(buffer, length, encoding, inbuffer, nBytesRead, true)) {
        // Write into report buffer
        m_osxs.write(buffer, length);
        (*pugi::get_memory_deallocation_function())(buffer);
      }
    } else
      break;

  } while(nBytesRead > 0);

  fclose(fd);
    
  StringX sxCRLF(L"\r\n"), sxLF(L"\n");
  StringX sx = m_osxs.rdbuf()->str();
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
    
  if(m_tcAction.empty()) {
    pws_os::IssueError(_T("PurgeFromDisk: Action not filled"));
    return false;
  }

  Format(m_cs_filename, IDSC_REPORTFILENAME,
         drive.c_str(), dir.c_str(), m_tcAction.c_str());
    
  return pws_os::DeleteAFile(m_cs_filename);
}

bool CReport::ReportExistsOnDisk()
{
  stringT path(m_csDataBase);
  stringT drive, dir, file, ext;
  if (!pws_os::splitpath(path, drive, dir, file, ext)) {
    pws_os::IssueError(_T("ReportExistsOnDisk: Finding path to database"));
    return false;
  }
    
  if(m_tcAction.empty()) {
    pws_os::IssueError(_T("ReportExistsOnDisk: Action not filled"));
    return false;
  }

  Format(m_cs_filename, IDSC_REPORTFILENAME,
         drive.c_str(), dir.c_str(), m_tcAction.c_str());
    
  return pws_os::FileExists(m_cs_filename);
}
