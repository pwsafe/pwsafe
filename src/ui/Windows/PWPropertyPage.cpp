/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "PWPropertyPage.h"
#include "GeneralMsgBox.h"

extern const wchar_t *EYE_CATCHER;

IMPLEMENT_DYNAMIC(CPWPropertyPage, CPropertyPage)

CPWPropertyPage::CPWPropertyPage(UINT nID)
: CPropertyPage(nID), m_pToolTipCtrl(NULL)
{
  m_psp.dwFlags |= PSP_HASHELP;
}

DboxMain *CPWPropertyPage::GetMainDlg() const
{
  return app.GetMainDlg();
}

bool CPWPropertyPage::InitToolTip(int Flags, int delayTimeFactor)
{
  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, Flags)) {
    pws_os::Trace(L"Unable To create ToolTip\n");
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
    return false;
  } else {
    EnableToolTips();
    // Delay initial show & reshow
    if (delayTimeFactor == 0) {
      // Special case for Question Mark 'button'
      m_pToolTipCtrl->SetDelayTime(TTDT_INITIAL, 0);
      m_pToolTipCtrl->SetDelayTime(TTDT_RESHOW, 0);
      m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, 30000);
    } else {
      int iTime = m_pToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
      m_pToolTipCtrl->SetDelayTime(TTDT_INITIAL, iTime);
      m_pToolTipCtrl->SetDelayTime(TTDT_RESHOW, iTime);
      m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, iTime * delayTimeFactor);
    }
    m_pToolTipCtrl->SetMaxTipWidth(300);
  }
  return true;
}

void CPWPropertyPage::AddTool(int DlgItemID, int ResID)
{
  if (m_pToolTipCtrl != NULL) {
    const CString cs(MAKEINTRESOURCE(ResID));
    m_pToolTipCtrl->AddTool(GetDlgItem(DlgItemID), cs);
  }
}

void CPWPropertyPage::ActivateToolTip()
{
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->Activate(TRUE);
}

void CPWPropertyPage::RelayToolTipEvent(MSG *pMsg)
{
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);
}

void CPWPropertyPage::ShowHelp(const CString &topicFile)
{
  if (!app.GetHelpFileName().IsEmpty()) {
    const CString cs_HelpTopic = app.GetHelpFileName() + topicFile;
    HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
  } else {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_HELP_UNAVALIABLE, MB_ICONERROR);
  }
}

LRESULT CPWPropertyPage::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  if (GetMainDlg()->m_eye_catcher != NULL &&
      wcscmp(GetMainDlg()->m_eye_catcher, EYE_CATCHER) == 0) {
    GetMainDlg()->ResetIdleLockCounter(message);
  } else
    pws_os::Trace(L"CPWPropertyPage::WindowProc - couldn't find DboxMain ancestor\n");

  return CPropertyPage::WindowProc(message, wParam, lParam);
}
