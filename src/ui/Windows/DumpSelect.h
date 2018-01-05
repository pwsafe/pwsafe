/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/// \file DumpSelect.h
//-----------------------------------------------------------------------------

#include "PWDialog.h"

// CDumpSelect dialog

class CDumpSelect : public CPWDialog
{
  DECLARE_DYNAMIC(CDumpSelect)

public:
  CDumpSelect(CWnd* pParent = NULL);   // standard constructor
  ~CDumpSelect();

  // Dialog Data
  enum { IDD = IDD_DUMPSELECTOR };

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

  afx_msg void OnBnClickedOK();
  afx_msg void OnBnClickedCancel();

  DECLARE_MESSAGE_MAP()

private:
  int m_dumptype;
};
