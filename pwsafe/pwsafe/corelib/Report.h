/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// Create an action report file

#include "afx.h"
#include <stdio.h>

class CReport
{
  // Construction
public:
  CReport()
    : m_fd(NULL) {}
  ~CReport() {}

  bool StartReport(const LPCTSTR tcAction, const CString &csDataBase);
  void EndReport();
  void WriteLine(const CString &cs_line, bool bCRLF = true);
  void WriteLine(const LPTSTR &tc_line, bool bCRLF = true);
  void WriteLine();
  CString GetReportFileName() const {return m_cs_filename;}

private:
  FILE *m_fd;
  CString m_cs_filename;
};
