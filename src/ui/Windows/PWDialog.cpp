/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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

bool CPWDialog::InitToolTip(int Flags, int delayTimeFactor)
{
  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, Flags)) {
    pws_os::Trace(L"Unable To create ToolTip\n");
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
    return false;
  } else {
    EnableToolTips();
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
  GetMainDlg()->SetThreadDpiAwarenessContext();
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

static const char szDialog[] = "Dialog";
static const char szPropertySheet[] = "PropertySheet";

static const size_t len_Dialog = strlen(szDialog);
static const size_t len_PropertySheet = strlen(szPropertySheet);

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

bool CPWDialogTracker::VerifyCanCloseDialogs()
{
  // we operate on a copy of the list of dialogs,
  // to avoid deadlocks and other nastiness
  std::list<CWnd *> dialogs;
  m_mutex.Lock();
  dialogs = m_dialogs;
  m_mutex.Unlock();

  for (auto *pWnd : dialogs) {
    CRuntimeClass *prt = pWnd->GetRuntimeClass();
    size_t len_classname = strlen(prt->m_lpszClassName);
    bool bCanDo(false);

    if (len_classname > len_Dialog) {
      const char *last_chars = &prt->m_lpszClassName[len_classname - len_Dialog];
      if (strcmp(last_chars, szDialog) == 0) {
        bCanDo = true;
      }
    }

    if (len_classname > len_PropertySheet) {
      const char *last_chars = &prt->m_lpszClassName[len_classname - len_PropertySheet];
      if (strcmp(last_chars, szPropertySheet) == 0) {
        bCanDo = true;
      }
    }

    if (!bCanDo)
      return bCanDo;
  }
  //std::for_each(dialogs.begin(), dialogs.end(), std::ptr_fun(f));

  return true;
}

namespace {
  void Shower(CWnd *pWnd)
  {
    pWnd->ShowWindow(SW_SHOW);
  }

  void Hider(CWnd *pWnd)
  {
    pWnd->ShowWindow(SW_HIDE);
  }

  void Closer(CWnd *pWnd)
  {
    CRuntimeClass *prt = pWnd->GetRuntimeClass();
    size_t len_classname = strlen(prt->m_lpszClassName);

    if (len_classname > len_Dialog) {
      const char *last_chars = &prt->m_lpszClassName[len_classname - len_Dialog];
      if (strcmp(last_chars, szDialog) == 0) {
        ((CDialog *)pWnd)->EndDialog(IDCANCEL);
      }
    }

    if (len_classname > len_PropertySheet) {
      const char *last_chars = &prt->m_lpszClassName[len_classname - len_PropertySheet];
      if (strcmp(last_chars, szPropertySheet) == 0) {
        ((CPropertySheet *)pWnd)->EndDialog(IDCANCEL);
      }
    }
  }
}

void CPWDialogTracker::ShowOpenDialogs()
{
  Apply(Shower);
}

void CPWDialogTracker::HideOpenDialogs()
{
  Apply(Hider);
}

void CPWDialogTracker::CloseOpenDialogs()
{
  // Only close them if DB is R-O
  if (app.GetMainDlg()->IsDBReadOnly()) {
    Apply(Closer);
  }
}