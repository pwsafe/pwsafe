/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "ThisMfcApp.h"
#include "GeneralMsgBox.h"
#include "DboxMain.h"

#include "WZPropertyPage.h"
#include "WZPropertySheet.h"

extern const wchar_t *EYE_CATCHER;

IMPLEMENT_DYNAMIC(CWZPropertyPage, CPropertyPage)

CWZPropertyPage::CWZPropertyPage(UINT nID, UINT nIDCaption, const int nType)
  : CPropertyPage(nID, nIDCaption), m_nID(nID), m_nType(nType),
  m_pToolTipCtrl(NULL)
{
}

void CWZPropertyPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWZPropertyPage, CPropertyPage)
  //{{AFX_MSG_MAP(CWZPropertyPage)
  ON_WM_CTLCOLOR()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CWZPropertyPage::OnSetActive()
{
  CString cs_text;
  
  bool bBackButton(false);
  UINT nID_NEXT(0);
  switch (m_nType) {
    case START:
      nID_NEXT = m_pWZPSH->GetAdvanced() ? IDS_WZNEXT : m_pWZPSH->GetButtonID();
      m_pWZPSH->SetWizardButtons(PSWIZB_NEXT);
      break;
    case MIDDLE:
      nID_NEXT = IDS_WZNEXT;
      m_pWZPSH->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
      bBackButton = true;
      break;
    case PENULTIMATE:
      nID_NEXT = m_pWZPSH->GetButtonID();
      m_pWZPSH->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
      bBackButton = true;
      break;
    case LAST:
      m_pWZPSH->SetWizardButtons(m_pWZPSH->GetCompleted() ? PSWIZB_FINISH : PSWIZB_DISABLEDFINISH);
      break;
  }

  if (nID_NEXT != 0) {
    cs_text.LoadString(nID_NEXT);
    m_pWZPSH->GetDlgItem(ID_WIZNEXT)->SetWindowText(cs_text);
  }

  // Enable/Disable Back button as required
  m_pWZPSH->GetDlgItem(ID_WIZBACK)->EnableWindow(bBackButton ? TRUE : FALSE);
  m_pWZPSH->GetDlgItem(ID_WIZBACK)->ShowWindow(bBackButton ? SW_SHOW : SW_HIDE);

  return CPropertyPage::OnSetActive();
}

void CWZPropertyPage::ShowHelp(const CString &topicFile)
{
  if (!app.GetHelpFileName().IsEmpty()) {
    const CString cs_HelpTopic = app.GetHelpFileName() + topicFile;
    HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
  } else {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_HELP_UNAVALIABLE, MB_ICONERROR);
  }
}

LRESULT CWZPropertyPage::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  if (app.GetMainDlg()->m_eye_catcher != NULL &&
      wcscmp(app.GetMainDlg()->m_eye_catcher, EYE_CATCHER) == 0) {
    app.GetMainDlg()->ResetIdleLockCounter(message);
  } else {
    pws_os::Trace(L"CWZPropertyPage::WindowProc - couldn't find DboxMain ancestor\n");
  }

  return CPropertyPage::WindowProc(message, wParam, lParam);
}

BOOL CWZPropertyPage::PreTranslateMessage(MSG *pMsg)
{
  // Show/hide caps lock indicator
  CWnd *pCapsLock = GetDlgItem(IDC_CAPSLOCK);
  if (pCapsLock != NULL) {
    pCapsLock->ShowWindow(((GetKeyState(VK_CAPITAL) & 0x0001) == 0x0001) ?
                            SW_SHOW : SW_HIDE);
  }

  return CPropertyPage::PreTranslateMessage(pMsg);
}

HBRUSH CWZPropertyPage::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

  if (nCtlColor == CTLCOLOR_STATIC) {
    UINT nID = pWnd->GetDlgCtrlID();
    if (nID == IDC_CAPSLOCK) {
      pDC->SetTextColor(RGB(255, 0, 0));
      pDC->SetBkMode(TRANSPARENT);
    }
  }
  return hbr;
}
