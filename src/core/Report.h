/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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
#include <map>

class CReport
{
public:
  CReport() : m_iAction(-1) {}
  ~CReport() {}

  void StartReport(int iAction, const stringT &csDataBase, bool writeHeader = true);
  void EndReport();
  void WriteLine(const stringT &cs_line, bool bCRLF = true)
  {WriteLine(cs_line.c_str(), bCRLF);}
  void WriteLine(LPCTSTR tc_line, bool bCRLF = true);
  void WriteLine();
  bool SaveToDisk();
  bool ReadFromDisk();
  bool PurgeFromDisk();
  bool ReportExistsOnDisk() const;
  StringX GetString() {return m_osxs.rdbuf()->str();}
  bool StringEmpty() const {return !m_osxs.rdbuf() || m_osxs.rdbuf()->str().empty();}
  const stringT GetFileName() const {return m_cs_filename;}

  static const std::map<int, LPCTSTR> ReportNames;

private:
  oStringXStream m_osxs;
  stringT m_cs_filename;
  int m_iAction;
  stringT m_csDataBase;
};

#endif /* __REPORT_H */
