/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "passwordsafe.h"
#include "GeneralMsgBox.h"
#include "DboxMain.h"

#include "WZPropertyPage.h"
#include "WZPropertySheet.h"

extern const wchar_t *EYE_CATCHER;

IMPLEMENT_DYNAMIC(CWZPropertyPage, CPropertyPage)

CWZPropertyPage::CWZPropertyPage(UINT nID, UINT nIDCaption, const int nType)
 : CPropertyPage(nID, nIDCaption), m_nID(nID), m_nType(nType)
{
}

void CWZPropertyPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWZPropertyPage, CPropertyPage)
  //{{AFX_MSG_MAP(CWZPropertyPage)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CWZPropertyPage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  // Set up buttons
  OnSetActive();

  return TRUE;
}

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

LRESULT CWZPropertyPage::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  CWnd *pParent = GetParent();
  while (pParent != NULL) {
    DboxMain *pDbx = dynamic_cast<DboxMain *>(pParent);
    if (pDbx != NULL && pDbx->m_eye_catcher != NULL &&
        wcscmp(pDbx->m_eye_catcher, EYE_CATCHER) == 0) {
      pDbx->ResetIdleLockCounter(message);
      break;
    } else
      pParent = pParent->GetParent();
  }
  if (pParent == NULL)
    pws_os::Trace(L"CWZPropertyPage::WindowProc - couldn't find DboxMain ancestor\n");

  return CPropertyPage::WindowProc(message, wParam, lParam);
}
