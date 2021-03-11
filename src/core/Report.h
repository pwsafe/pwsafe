/*
* Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __REPORT_H
#define __REPORT_H

// Create an action report file

#include "os/typedefs.h"
#include "StringXStream.h"

/*
 * The name of the reports are listed (not translated), values used for tcAction in StartReport. To stay compatible with Windows, the interface is left as string
 */

#define REPORT_SYNCHRONIZE_NAME        (L"Synchronize")
#define REPORT_COMPARE_NAME            (L"Compare")
#define REPORT_MERGE_NAME              (L"Merge")
#define REPORT_IMPORTTEXT_NAME         (L"Import Text")
#define REPORT_IMPORTXML_NAME          (L"Import XML")
#define REPORT_IMPORTKEEPASS_TXT_NAME  (L"Import KeePassV1 TXT")
#define REPORT_IMPORTKEEPASS_CSV_NAME  (L"Import KeePassV1 CSV")
#define REPORT_EXPORTTEXT_NAME         (L"Export Text")
#define REPORT_EXPORTXML_NAME          (L"Export XML")
#define REPORT_EXPORT_DB_NAME          (L"Export DB")
#define REPORT_FIND_NAME               (L"Find")
#define REPORT_VALIDATE_NAME           (L"Validate")

class CReport
{
public:
  CReport() {}
  ~CReport() {}

  void StartReport(LPCTSTR tcAction, const stringT &csDataBase, bool writeHeader = true);
  void EndReport();
  void WriteLine(const stringT &cs_line, bool bCRLF = true)
  {WriteLine(cs_line.c_str(), bCRLF);}
  void WriteLine(LPCTSTR tc_line, bool bCRLF = true);
  void WriteLine();
  bool SaveToDisk();
  bool ReadFromDisk();
  bool PurgeFromDisk();
  bool ReportExistsOnDisk();
  StringX GetString() {return m_osxs.rdbuf()->str();}
  bool StringEmpty() {return !m_osxs.rdbuf() || m_osxs.rdbuf()->str().empty();}
  const stringT GetFileName() {return m_cs_filename;}

private:
  oStringXStream m_osxs;
  stringT m_cs_filename;
  stringT m_tcAction;
  stringT m_csDataBase;
};

#endif /* __REPORT_H */
