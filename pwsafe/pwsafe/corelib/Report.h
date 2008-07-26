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
#include "SMemFile.h"
#include <stdio.h>

class CReport
{
  // Construction
public:
  CReport()
    : m_psfile(NULL), m_pdfile(NULL), m_pData(NULL), m_dwDatasize(0) {}
  ~CReport();

  void StartReport(LPCTSTR tcAction, const CString &csDataBase);
  void EndReport();
  void WriteLine(const CString &cs_line, bool bCRLF = true);
  void WriteLine(const LPTSTR &tc_line, bool bCRLF = true);
  void WriteLine();
  bool SaveToDisk();
  BYTE *GetData() {return m_pData;}
  DWORD GetDataLength() {return m_dwDatasize;}

private:
  FILE *m_pdfile;
  CSMemFile *m_psfile;
  CString m_cs_filename;
  int m_imode;
  BYTE *m_pData;
  DWORD m_dwDatasize;
  LPCTSTR m_tcAction;
  CString m_csDataBase;
};
