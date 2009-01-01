/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __REPORT_H
#define __REPORT_H

// Create an action report file

#ifdef _WIN32
#include "afx.h"
#endif
#include "os/typedefs.h"
#include "StringXStream.h"

class CReport
{
public:
  CReport() {}
  ~CReport() {}

  void StartReport(LPCTSTR tcAction, const stringT &csDataBase);
  void EndReport();
  void WriteLine(const stringT &cs_line, bool bCRLF = true);
  void WriteLine(const LPTSTR &tc_line, bool bCRLF = true);
  void WriteLine();
  bool SaveToDisk();
  StringX GetString() {return m_osxs.rdbuf()->str();}

private:
  oStringXStream m_osxs;
  stringT m_cs_filename;
  int m_imode;
  stringT m_tcAction;
  stringT m_csDataBase;
};

#endif /* __REPORT_H */
