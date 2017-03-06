/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// HotKeyConflictDlg.cpp : implementation file
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "HotKeyConflictDlg.h"

#include "resource.h"

// CHotKeyConflictDlg dialog

IMPLEMENT_DYNAMIC(CHotKeyConflictDlg, CPWDialog)

CHotKeyConflictDlg::CHotKeyConflictDlg(CWnd *pParent, int iRC,
  CString csAPPVALUE, CString csATVALUE,
  CString csAPPMENU, CString csAPPENTRY, CString csATMENU, CString csATENTRY)
	: CPWDialog(CHotKeyConflictDlg::IDD, pParent), m_iRC(iRC),
  m_csAPPVALUE(csAPPVALUE), m_csATVALUE(csATVALUE),
  m_csAPPMENU(csAPPMENU), m_csAPPENTRY(csAPPENTRY), m_csATMENU(csATMENU), m_csATENTRY(csATENTRY)
{
}

CHotKeyConflictDlg::~CHotKeyConflictDlg()
{
}

void CHotKeyConflictDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHotKeyConflictDlg, CPWDialog)
  ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CHotKeyConflictDlg message handlers

BOOL CHotKeyConflictDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  if (m_iRC == 0) {
    // Shouldn't be here!
    EndDialog(0);
  }

  // Make message Bold
  LOGFONT lf;
  CWnd *pWnd = GetDlgItem(IDC_STATIC_MESSAGE);
  CFont *pFont = pWnd->GetFont();
  pFont->GetLogFont(&lf);
  lf.lfWeight = FW_BOLD;
  m_BoldFont.CreateFontIndirect(&lf);

  GetDlgItem(IDC_STATIC_APPMENU)->SetWindowText(m_csAPPMENU);
  if (m_csAPPMENU.IsEmpty()) {
    GetDlgItem(IDC_STATIC_APPMENU)->EnableWindow(FALSE);
  }

  GetDlgItem(IDC_STATIC_APPENTRY)->SetWindowText(m_csAPPENTRY);
  if (m_csAPPENTRY.IsEmpty()) {
    GetDlgItem(IDC_STATIC_APPENTRY)->EnableWindow(FALSE);
  }

  GetDlgItem(IDC_STATIC_ATMENU)->SetWindowText(m_csATMENU);
  if (m_csATMENU.IsEmpty()) {
    GetDlgItem(IDC_STATIC_ATMENU)->EnableWindow(FALSE);
  }

  GetDlgItem(IDC_STATIC_ATENTRY)->SetWindowText(m_csATENTRY);
  if (m_csATENTRY.IsEmpty()) {
    GetDlgItem(IDC_STATIC_ATENTRY)->EnableWindow(FALSE);
  }

  // Update Group text with Application HotKey value
  if (!m_csAPPVALUE.IsEmpty()) {
    CWnd *pWndAPP = GetDlgItem(IDC_STATIC_APPHK);
    CString csText;
    pWndAPP->GetWindowText(csText);
    csText += L" (" + m_csAPPVALUE + L")";
    pWndAPP->SetWindowText(csText);
  }

  // Update Group text with Autotype HotKey value
  CWnd *pWndAT = GetDlgItem(IDC_STATIC_ATHK);
  CString csText;
  pWndAT->GetWindowText(csText);
  csText += L" (" + m_csATVALUE + L")";
  pWndAT->SetWindowText(csText);

  return TRUE;
}

HBRUSH CHotKeyConflictDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CPWDialog::OnCtlColor(pDC, pWnd, nCtlColor);

  // Only deal with Static controls and then
  if (nCtlColor == CTLCOLOR_STATIC) {
    pDC->SetBkMode(TRANSPARENT);
    switch (pWnd->GetDlgCtrlID()) {
    case IDC_STATIC_APPMENU:
      if (m_iRC & HKE_APPMENU) {
        pDC->SetTextColor(RGB(255, 0, 0));
      }
      break;
    case IDC_STATIC_APPENTRY:
      if (m_iRC & HKE_APPENTRY) {
        pDC->SetTextColor(RGB(255, 0, 0));
      }
      break;
    case IDC_STATIC_ATMENU:
      if (m_iRC & HKE_ATMENU) {
        pDC->SetTextColor(RGB(255, 0, 0));
      }
      break;
    case IDC_STATIC_ATENTRY:
      if (m_iRC & HKE_ATENTRY) {
        pDC->SetTextColor(RGB(255, 0, 0));
      }
      break;
    case IDC_STATIC_MESSAGE:
      pDC->SelectObject(&m_BoldFont);
      break;
    }
  }

  return hbr;
}
