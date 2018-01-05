/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// PasswordSubsetDlg.h : header file
//-----------------------------------------------------------------------------

#include "PWDialog.h"
#include "ControlExtns.h"
#include "TBMStatic.h"

#include "core/PwsPlatform.h"

// Simple class to ensure only numbers, space, comma and semi-colons
// are entered
class CNumEdit : public CEdit
{
public:
  CNumEdit();

  // Generated message map functions
protected:
  //{{AFX_MSG(CNumEdit)
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnPaste();
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

private:
  bool m_bLastMinus, m_bLastSeparator;
};

class CPasswordSubsetDlg : public CPWDialog
{
public:
  CPasswordSubsetDlg(CWnd* pParent, const StringX &passwd); // standard constructor
  ~CPasswordSubsetDlg();

  enum { IDD = IDD_PASSWORDSUBSET };

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG *pMsg);
  virtual void OnCancel();

  CTBMStatic m_Help1, m_Help2;

  //{{AFX_MSG(CPasswordSubsetDlg)
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
  afx_msg void OnCopy();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  LRESULT OnDisplayStatus(WPARAM /* wParam */, LPARAM /* lParam */);

  CNumEdit m_neSubsetPositions;
  CStaticExtn m_stcWarningMsg;
  CBitmap m_CopyPswdBitmap, m_DisabledCopyPswdBitmap;
  CEdit m_edResults;
  CButton *m_pCopyBtn;

  const StringX m_sxPassword;
  CString m_csSubsetPositions, m_csWarningMsg;
  bool m_bShown, m_bCopyPasswordEnabled;
  BOOL m_bImageLoaded, m_bDisabledImageLoaded;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
