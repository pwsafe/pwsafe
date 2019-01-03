/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "PWResizeDialog.h"
#include "core/report.h"

#include "resource.h"

// CViewReport dialog

class CViewReport : public CPWResizeDialog
{
  DECLARE_DYNAMIC(CViewReport)

public:
  CViewReport(CWnd* pParent = NULL,
  CReport *pRpt = NULL);   // standard constructor
  virtual ~CViewReport();

// Dialog Data
  enum { IDD = IDD_VIEWREPORT };

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  CReport *m_pRpt;
  StringX m_pString;
  DWORD m_dwDatasize;
  CEdit m_editreport;
  CBrush m_backgroundbrush;
  COLORREF m_textcolor, m_backgroundcolour;
  bool m_bMemoryAllocOK;

  afx_msg void Save();
  afx_msg void Finish();
  afx_msg void SendToClipboard();
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);

  DECLARE_MESSAGE_MAP()
};
