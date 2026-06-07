/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "StdAfx.h"
#include "PasswordSafe.h"

#include "AddEdit_Basic_NotesPage.h"
#include "AddEdit_PropertySheet.h"

#include "DboxMain.h"
#include "Fonts.h"
#include "GeneralMsgBox.h"
#include "resource3.h"
#include "winutils.h"

#include "core/PWSprefs.h"
#include "os/file.h"
#include "os/debug.h"

#include <Shlwapi.h>
#include <fstream>
#include <limits>

#include "Richedit.h"

IMPLEMENT_DYNAMIC(CAddEdit_Basic_NotesPage, CAddEdit_Basic_SubPage)

HANDLE CAddEdit_Basic_NotesPage::ghEvents[2];
CString CAddEdit_Basic_NotesPage::CS_EXTERNAL_EDITOR;
CString CAddEdit_Basic_NotesPage::CS_HIDDEN_NOTES;

CAddEdit_Basic_NotesPage::CAddEdit_Basic_NotesPage(CWnd *pParent, st_AE_master_data *pAEMD)
  : CAddEdit_Basic_SubPage(pParent, IDD, IDD_SHORT, pAEMD),
    m_thread(nullptr), m_isNotesHidden(false),
    m_bWordWrap(PWSprefs::GetInstance()->GetPref(PWSprefs::NotesWordWrap)),
    m_bUsingNotesExternalEditor(false), m_bOKSave(FALSE), m_bOKCancel(FALSE),
    m_bInitdone(false), m_iPointSize(0)
{
  if (CS_HIDDEN_NOTES.IsEmpty()) {
    CS_HIDDEN_NOTES.LoadString(IDS_HIDDENNOTES);
    CS_EXTERNAL_EDITOR.LoadString(IDS_NOTES_IN_EXTERNAL_EDITOR);
  }

  ghEvents[0] = ghEvents[1] = nullptr;

  std::vector<st_context_menu> vmenu_items;
  st_context_menu st_cm;
  std::wstring cs_menu_string;

  LoadAString(cs_menu_string, IDS_WORD_WRAP);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = PWS_MSG_EDIT_WORDWRAP;
  st_cm.flags = m_bWordWrap ? MF_CHECKED : MF_UNCHECKED;
  vmenu_items.push_back(st_cm);

  st_cm.Empty();
  LoadAString(cs_menu_string, IDS_NOTESZOOMIN);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = PWS_MSG_CALL_NOTESZOOMIN;
  st_cm.lParam = 1;
  vmenu_items.push_back(st_cm);

  st_cm.Empty();
  LoadAString(cs_menu_string, IDS_NOTESZOOMOUT);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = PWS_MSG_CALL_NOTESZOOMOUT;
  st_cm.lParam = -1;
  vmenu_items.push_back(st_cm);

  st_cm.Empty();
  LoadAString(cs_menu_string, IDS_EDITEXTERNALLY);
  st_cm.menu_string = cs_menu_string;
  st_cm.message_number = PWS_MSG_CALL_EXTERNAL_EDITOR;
  vmenu_items.push_back(st_cm);

  m_ex_notes.SetContextMenu(vmenu_items);
}

void CAddEdit_Basic_NotesPage::DoDataExchange(CDataExchange *pDX)
{
  CPWPropertyPage::DoDataExchange(pDX);

  DDX_Text(pDX, IDC_NOTES, static_cast<CString &>(M_notes()));
  DDX_Control(pDX, IDC_NOTES, m_ex_notes);
  DDX_Control(pDX, IDC_HIDDEN_NOTES, m_ex_hidden_notes);
  DDX_Control(pDX, IDC_NOTESHELP, m_Help);

  if (pDX->m_bSaveAndValidate == TRUE) {
    M_notes().Replace(L"\r\n", L"\n");
    M_notes().Remove(L'\r');
  }
}

BEGIN_MESSAGE_MAP(CAddEdit_Basic_NotesPage, CAddEdit_Basic_SubPage)
  ON_EN_CHANGE(IDC_NOTES, OnENChangeNotes)
  ON_EN_KILLFOCUS(IDC_NOTES, OnENKillFocusNotes)
  ON_MESSAGE(PWS_MSG_CALL_EXTERNAL_EDITOR, OnCallExternalEditor)
  ON_MESSAGE(PWS_MSG_EXTERNAL_EDITOR_ENDED, OnExternalEditorEnded)
  ON_MESSAGE(PWS_MSG_EDIT_WORDWRAP, OnWordWrap)
  ON_MESSAGE(PWS_MSG_CALL_NOTESZOOMIN, OnZoomNotes)
  ON_MESSAGE(PWS_MSG_CALL_NOTESZOOMOUT, OnZoomNotes)
END_MESSAGE_MAP()

BOOL CAddEdit_Basic_NotesPage::OnInitDialog()
{
  CAddEdit_Basic_SubPage::OnInitDialog();

  ModifyStyleEx(0, WS_EX_CONTROLPARENT);

  Fonts *pFonts = Fonts::GetInstance();
  m_ex_notes.SetEventMask(ENM_CHANGE | m_ex_notes.GetEventMask());
  m_ex_notes.SetTextMode(TM_PLAINTEXT);

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::NotesFont).empty()) {
    m_ex_notes.SetFont(pFonts->GetAddEditFont());
    m_ex_hidden_notes.SetFont(pFonts->GetAddEditFont());
  } else {
    m_ex_notes.SetFont(pFonts->GetNotesFont());
    m_ex_hidden_notes.SetFont(pFonts->GetNotesFont());
  }

  if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
    m_Help.Init(IDB_QUESTIONMARK);
    AddTool(IDC_NOTESHELP, IDS_NOTESHELP);
    ActivateToolTip();
  } else {
    m_Help.EnableWindow(FALSE);
    m_Help.ShowWindow(SW_HIDE);
  }

  if (M_uicaller() == IDS_VIEWENTRY ||
      (M_uicaller() == IDS_EDITENTRY && M_protected() != 0)) {
    m_ex_notes.SendMessage(EM_SETREADONLY, TRUE, 0);
    m_ex_hidden_notes.SendMessage(EM_SETREADONLY, TRUE, 0);
  }

  m_ex_hidden_notes.SetWindowText(CS_HIDDEN_NOTES);
  m_ex_hidden_notes.SendMessage(EM_SETREADONLY, TRUE, 0);

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ShowNotesDefault)) {
    ShowNotes(true);
  } else {
    HideNotes(true);
  }

  CHARFORMAT2 cf = {};
  cf.cbSize = sizeof(cf);
  m_ex_notes.GetDefaultCharFormat(cf);
  m_iPointSize = cf.yHeight / 20;
  SetZoomMenu();

  m_bInitdone = true;
  return TRUE;
}

BOOL CAddEdit_Basic_NotesPage::PreTranslateMessage(MSG *pMsg)
{
  RelayToolTipEvent(pMsg);

  if (m_ex_hidden_notes.m_hWnd == pMsg->hwnd) {
    switch (pMsg->message) {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
      if (!m_bUsingNotesExternalEditor)
        ShowNotes();
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
      return TRUE;
    default:
      break;
    }
  }

  if (pMsg->message == WM_KEYDOWN &&
      (GetKeyState(VK_CONTROL) & 0x8000) == 0x8000 &&
      m_ex_notes.m_hWnd == ::GetFocus()) {
    switch (pMsg->wParam) {
    case 'A':
      SelectAllNotes();
      return TRUE;
    case 'V':
      m_ex_notes.PasteSpecial(CF_UNICODETEXT);
      return TRUE;
    case VK_ADD:
    case VK_SUBTRACT:
      OnZoomNotes(0, pMsg->wParam == VK_ADD ? 1 : -1);
      return TRUE;
    default:
      break;
    }
  }

  return CAddEdit_Basic_SubPage::PreTranslateMessage(pMsg);
}

void CAddEdit_Basic_NotesPage::OnENChangeNotes()
{
  if (!m_bInitdone || M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return;

  if (m_ex_hidden_notes.IsWindowVisible()) {
    return;
  } else {
    CSecString current_notes;
    m_ex_notes.GetWindowText(current_notes);
    if (current_notes == M_notes())
      return;
  }

  m_ae_psh->SetChanged(true);
  m_ae_psh->SetNotesChanged(true);
  UpdateData(TRUE);
}

void CAddEdit_Basic_NotesPage::SelectAllNotes()
{
  static_cast<CEdit *>(GetDlgItem(IDC_NOTES))->SetSel(0, -1, TRUE);
}

LRESULT CAddEdit_Basic_NotesPage::OnWordWrap(WPARAM, LPARAM)
{
  m_bWordWrap = !m_bWordWrap;
  m_ex_notes.SetTargetDevice(nullptr, m_bWordWrap ? 0 : 1);
  m_ex_notes.UpdateState(PWS_MSG_EDIT_WORDWRAP, m_bWordWrap);

  m_ex_notes.EnableWindow(FALSE);
  m_ex_notes.EnableWindow(TRUE);

  if (m_isNotesHidden)
    ShowNotes();

  return 0L;
}

void CAddEdit_Basic_NotesPage::OnENKillFocusNotes()
{
  if (!PWSprefs::GetInstance()->GetPref(PWSprefs::ShowNotesDefault))
    HideNotes();
}

void CAddEdit_Basic_NotesPage::SetZoomMenu()
{
  m_ex_notes.EnableMenuItem(PWS_MSG_CALL_NOTESZOOMIN, m_iPointSize < 72);
  m_ex_notes.EnableMenuItem(PWS_MSG_CALL_NOTESZOOMOUT, m_iPointSize > 6);
}

LRESULT CAddEdit_Basic_NotesPage::OnZoomNotes(WPARAM, LPARAM lParam)
{
  UpdateData(TRUE);

  if ((lParam < 0 && m_iPointSize <= 6) || (lParam > 0 && m_iPointSize >= 72))
    return 0L;

  const UINT wp_increment = (lParam > 0 ? 1 : -1) * 2;

  CHARFORMAT2 cf = {};
  cf.cbSize = sizeof(cf);
  cf.dwMask = CFM_SIZE;
  cf.yHeight = (m_iPointSize + wp_increment) * 20;
  m_ex_notes.SetDefaultCharFormat(cf);

  memset(&cf, 0, sizeof(cf));
  cf.cbSize = sizeof(cf);
  m_ex_notes.GetDefaultCharFormat(cf);
  m_iPointSize = cf.yHeight / 20;

  SetZoomMenu();
  return 0L;
}

void CAddEdit_Basic_NotesPage::ShowNotes(const bool bForceShow)
{
  if (bForceShow || m_isNotesHidden) {
    m_isNotesHidden = false;

    m_ex_notes.ShowWindow(SW_SHOW);
    m_ex_notes.EnableWindow(TRUE);
    m_ex_hidden_notes.ShowWindow(SW_HIDE);
    m_ex_hidden_notes.EnableWindow(FALSE);

    m_ex_notes.SetFocus();
    SetZoomMenu();
  }
}

void CAddEdit_Basic_NotesPage::HideNotes(const bool bForceHide)
{
  if (bForceHide || !m_isNotesHidden) {
    m_isNotesHidden = true;

    m_ex_notes.ShowWindow(SW_HIDE);
    m_ex_notes.EnableWindow(FALSE);
    m_ex_hidden_notes.ShowWindow(SW_SHOW);
    m_ex_hidden_notes.EnableWindow(TRUE);
  }
}

LRESULT CAddEdit_Basic_NotesPage::OnCallExternalEditor(WPARAM, LPARAM)
{
  CGeneralMsgBox gmb;
  if (gmb.AfxMessageBox(IDS_EXTERNAL_EDITOR_WARNING,
                        MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) != IDYES)
    return 0L;

  m_bOKSave = m_ae_psh->GetDlgItem(IDOK)->EnableWindow(FALSE);
  m_bOKCancel = m_ae_psh->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

  m_ex_hidden_notes.SetWindowText(CS_EXTERNAL_EDITOR);
  HideNotes(true);
  m_bUsingNotesExternalEditor = true;

  ghEvents[0] = ghEvents[1] = nullptr;
  m_thread = CExtThread::BeginThread(ExternalEditorThread, this);
  return 0L;
}

UINT CAddEdit_Basic_NotesPage::ExternalEditorThread(LPVOID me)
{
  auto *self = static_cast<CAddEdit_Basic_NotesPage *>(me);

  wchar_t szExecName[MAX_PATH + 1];
  wchar_t lpPathBuffer[4096];
  const DWORD dwBufSize = 4096;

  StringX sxEditorCmdLineParms = PWSprefs::GetInstance()->GetPref(PWSprefs::AltNotesEditorCmdLineParms);
  StringX sxEditor = PWSprefs::GetInstance()->GetPref(PWSprefs::AltNotesEditor);
  if (sxEditor.empty()) {
    DWORD dwSize(MAX_PATH);
    HRESULT stat = ::AssocQueryString(0, ASSOCSTR_EXECUTABLE, L".txt", L"Open", szExecName, &dwSize);
    if (static_cast<int>(stat) != S_OK) {
      self->SendMessage(PWS_MSG_EXTERNAL_EDITOR_ENDED, 8, 0);
      self->ResetHiddenNotes();
      return 8;
    }
    sxEditor = szExecName;
  }

  DWORD dwResult = ExpandEnvironmentStrings(sxEditor.c_str(), szExecName, MAX_PATH + 1);
  if (dwResult == 0 || dwResult > (MAX_PATH + 1) || !pws_os::FileExists(szExecName)) {
    CGeneralMsgBox gmb;
    CString cs_msg, cs_title(MAKEINTRESOURCE(IDS_EDITEXTERNALLY));
    cs_msg.Format(IDS_CANT_FIND_EXT_EDITOR, sxEditor.c_str());
    gmb.MessageBox(cs_msg, cs_title, MB_OK | MB_ICONEXCLAMATION);

    self->SendMessage(PWS_MSG_EXTERNAL_EDITOR_ENDED, 12, 0);
    self->ResetHiddenNotes();
    return 12;
  }

  GetTempPath(dwBufSize, lpPathBuffer);
  GetTempFileName(lpPathBuffer, L"NTE", 0, self->m_szTempName);

  FILE *fd = pws_os::FOpen(self->m_szTempName, L"w+b");
  if (fd == nullptr) {
    self->SendMessage(PWS_MSG_EXTERNAL_EDITOR_ENDED, 20, 0);
    self->ResetHiddenNotes();
    return 20;
  }

  const unsigned int iBOM = 0xFEFF;
  putwc(iBOM, fd);
  fwrite(reinterpret_cast<const void *>(static_cast<LPCWSTR>(self->M_notes())),
         sizeof(BYTE), self->M_notes().GetLength() * sizeof(wchar_t), fd);
  fclose(fd);

  STARTUPINFO si = {};
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi = {};

  CString cs_CommandLine;
  cs_CommandLine.Format(L"\"%s\" %s \"%s\"", szExecName,
                        sxEditorCmdLineParms.c_str(), self->m_szTempName);
  LPWSTR pszCommandLine = cs_CommandLine.GetBuffer(cs_CommandLine.GetLength());

  if (!CreateProcess(nullptr, pszCommandLine, nullptr, nullptr, FALSE,
                     CREATE_UNICODE_ENVIRONMENT | CREATE_SUSPENDED,
                     nullptr, lpPathBuffer, &si, &pi)) {
    pws_os::IssueError(L"External Editor CreateProcess", false);
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_CANT_FIND_EXT_EDITOR, MB_OK | MB_ICONEXCLAMATION);

    _wremove(self->m_szTempName);
    SecureZeroMemory(self->m_szTempName, sizeof(self->m_szTempName));
    self->SendMessage(PWS_MSG_EXTERNAL_EDITOR_ENDED, 24, 0);
    self->ResetHiddenNotes();
    return 24;
  }

  ResumeThread(pi.hThread);
  WaitForInputIdle(pi.hProcess, INFINITE);

  self->EnableWindow(FALSE);

  ghEvents[0] = pi.hProcess;
  ghEvents[1] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  const DWORD dwEvent = WaitForMultipleObjects(2, ghEvents, FALSE, INFINITE);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  cs_CommandLine.ReleaseBuffer();

  self->SendMessage(PWS_MSG_EXTERNAL_EDITOR_ENDED,
                    dwEvent == (WAIT_OBJECT_0 + 0) ? 0 : 28, 0);
  self->ResetHiddenNotes();
  return 0;
}

void CAddEdit_Basic_NotesPage::ResetHiddenNotes()
{
  m_ex_hidden_notes.SetWindowText(CS_HIDDEN_NOTES);
  ShowNotes(true);
  m_bUsingNotesExternalEditor = false;
  EnableWindow(TRUE);
}

LRESULT CAddEdit_Basic_NotesPage::OnExternalEditorEnded(WPARAM wParam, LPARAM)
{
  CSecString sOldNotes = M_notes();

  if (wParam == 0) {
    FILE *fd = pws_os::FOpen(m_szTempName, L"r+b");
    if (fd != nullptr) {
      M_notes().Empty();

      const ulong64 flength = pws_os::fileLength(fd);
      size_t slength = flength > ((std::numeric_limits<size_t>::max)() - 2) ?
        (std::numeric_limits<size_t>::max)() - 2 : static_cast<size_t>(flength);

      BYTE *pBuffer = new BYTE[slength + sizeof(wchar_t)];
      memset(pBuffer, 0, slength + sizeof(wchar_t));

      if (slength >= 2) {
        const unsigned char BOM[] = { 0xff, 0xfe };
        fread(pBuffer, 1, 2, fd);
        if (pBuffer[0] != BOM[0] || pBuffer[1] != BOM[1]) {
          fseek(fd, 0, SEEK_SET);
          slength += 2;
        }
      }

      memset(pBuffer, 0, 2);
      fread(pBuffer, sizeof(BYTE), slength - 2, fd);

      std::wstring sNewNotes = reinterpret_cast<const LPCWSTR>(pBuffer);
      delete[] pBuffer;
      fclose(fd);

      if ((!M_pcore()->IsReadOnly() && M_protected() == 0) &&
          sNewNotes.length() > MAXTEXTCHARS) {
        sNewNotes = sNewNotes.substr(0, MAXTEXTCHARS);

        CGeneralMsgBox gmb;
        CString cs_text, cs_title(MAKEINTRESOURCE(IDS_WARNINGTEXTLENGTH));
        cs_text.Format(IDS_TRUNCATETEXT, MAXTEXTCHARS);
        gmb.MessageBox(cs_text, cs_title, MB_OK | MB_ICONEXCLAMATION);
      }

      M_notes() = sNewNotes.c_str();
      UpdateData(FALSE);
      m_ex_notes.Invalidate();
    }
  } else {
    UpdateData(FALSE);
    m_ex_notes.Invalidate();
  }

  _wremove(m_szTempName);
  SecureZeroMemory(m_szTempName, sizeof(m_szTempName));

  m_ae_psh->GetDlgItem(IDOK)->EnableWindow(m_bOKSave == 0 ? TRUE : FALSE);
  m_ae_psh->GetDlgItem(IDCANCEL)->EnableWindow(m_bOKCancel == 0 ? TRUE : FALSE);

  if (sOldNotes != M_notes()) {
    m_ae_psh->SetChanged(true);
    m_ae_psh->SetNotesChanged(true);
  }

  return 0L;
}
