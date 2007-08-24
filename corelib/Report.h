/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
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

  bool StartReport(LPTSTR tcAction, const CString &csDataBase);
  void EndReport();
  void WriteLine(CString &cs_line, bool bCRLF = true);
  void WriteLine(LPTSTR &tc_line, bool bCRLF = true);
  void WriteLine();
 
private:
  FILE *m_fd;
};
