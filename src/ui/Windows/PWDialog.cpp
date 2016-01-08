/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "PWDialog.h"
#include "GeneralMsgBox.h"

#include <algorithm>
#include <functional>

extern const wchar_t *EYE_CATCHER;

static CPWDialogTracker the_tracker;
CPWDialogTracker *CPWDialog::sm_tracker = &the_tracker; // static member

IMPLEMENT_DYNAMIC(CPWDialog, CDialog)

DboxMain *CPWDialog::GetMainDlg() const
{
  return app.GetMainDlg();
}

void CPWDialog::InitToolTip(int Flags, int delayTimeFactor)
{
  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, Flags)) {
    pws_os::Trace(L"Unable To create ToolTip\n");
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
  } else {
    EnableToolTips();
    // Delay initial show & reshow
    if (delayTimeFactor > 0) {
      int iTime = m_pToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
      m_pToolTipCtrl->SetDelayTime(TTDT_INITIAL, iTime);
      m_pToolTipCtrl->SetDelayTime(TTDT_RESHOW, iTime);
      m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, iTime * delayTimeFactor);
    }
    m_pToolTipCtrl->SetMaxTipWidth(300);
  }
}

void CPWDialog::AddTool(int DlgItemID, int ResID)
{
  if (m_pToolTipCtrl != NULL) {
    const CString cs(MAKEINTRESOURCE(ResID));
    m_pToolTipCtrl->AddTool(GetDlgItem(DlgItemID), cs);
  }
}

void CPWDialog::ActivateToolTip()
{
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->Activate(TRUE);
}

void CPWDialog::RelayToolTipEvent(MSG *pMsg)
{
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);
}

void CPWDialog::ShowHelp(const CString &topicFile)
{
  if (!app.GetHelpFileName().IsEmpty()) {
    const CString cs_HelpTopic = app.GetHelpFileName() + topicFile;
    HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
  } else {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_HELP_UNAVALIABLE, MB_ICONERROR);
  }
}

LRESULT CPWDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  if (GetMainDlg()->m_eye_catcher != NULL &&
      wcscmp(GetMainDlg()->m_eye_catcher, EYE_CATCHER) == 0) {
    GetMainDlg()->ResetIdleLockCounter(message);
  } else
    pws_os::Trace(L"CPWDialog::WindowProc - couldn't find DboxMain ancestor\n");

  return CDialog::WindowProc(message, wParam, lParam);
}

INT_PTR CPWDialog::DoModal()
{
  bool bAccEn = app.IsAcceleratorEnabled();
  if (bAccEn)
    app.DisableAccelerator();

  GetDialogTracker()->AddOpenDialog(this);
  INT_PTR rc = CDialog::DoModal();
  GetDialogTracker()->RemoveOpenDialog(this);

  if (bAccEn)
    app.EnableAccelerator();

  return rc;
}

CPWDialogTracker *CPWDialog::GetDialogTracker()
{
  return sm_tracker;
}

CPWDialogTracker::CPWDialogTracker()
{
}

CPWDialogTracker::~CPWDialogTracker()
{
}

bool CPWDialogTracker::AnyOpenDialogs() const
{
  bool retval;
  m_mutex.Lock();
  retval = !m_dialogs.empty();
  m_mutex.Unlock();
  return retval;
}

void CPWDialogTracker::AddOpenDialog(CWnd *dlg)
{
  m_mutex.Lock();
  m_dialogs.push_back(dlg);
  m_mutex.Unlock();
}

void CPWDialogTracker::RemoveOpenDialog(CWnd *dlg)
{
  m_mutex.Lock();
  m_dialogs.remove(dlg);
  m_mutex.Unlock();
}

void CPWDialogTracker::Apply(void (*f)(CWnd *))
{
  // we operate on a copy of the list of dialogs,
  // to avoid deadlocks and other nastiness
  std::list<CWnd *> dialogs;
  m_mutex.Lock();
  dialogs = m_dialogs;
  m_mutex.Unlock();
  std::for_each(dialogs.begin(), dialogs.end(), std::ptr_fun(f));
}
