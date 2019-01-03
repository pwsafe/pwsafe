/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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

class CReport
{
public:
  CReport() {}
  ~CReport() {}

  void StartReport(LPCTSTR tcAction, const stringT &csDataBase);
  void EndReport();
  void WriteLine(const stringT &cs_line, bool bCRLF = true)
  {WriteLine(cs_line.c_str(), bCRLF);}
  void WriteLine(LPCTSTR tc_line, bool bCRLF = true);
  void WriteLine();
  bool SaveToDisk();
  StringX GetString() {return m_osxs.rdbuf()->str();}

private:
  oStringXStream m_osxs;
  stringT m_cs_filename;
  stringT m_tcAction;
  stringT m_csDataBase;
};

#endif /* __REPORT_H */
