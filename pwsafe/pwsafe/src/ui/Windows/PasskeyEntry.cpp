/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PasskeyEntry.cpp
//-----------------------------------------------------------------------------

/*
Passkey?  That's Russian for 'pass'.  You know, passkey
down the streetsky.  [Groucho Marx]
*/

#include "PasswordSafe.h"
#include "PWFileDialog.h"
#include "corelib/PwsPlatform.h"
#include "corelib/Pwsdirs.h"
#include "os/file.h"
#include "os/env.h"
#include "os/dir.h"
#include "corelib/pwsprefs.h"
#include "ThisMfcApp.h"
#include "AdvancedDlg.h"
#include "VirtualKeyboard/VKeyBoardDlg.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#include "pocketpc/PocketPC.h"
#else
#include "resource.h"
#include "resource3.h"  // String resources
#endif

#include "SecString.h"

#include "SysColStatic.h"

#include "PasskeyEntry.h"
#include "PwFont.h"
#include "TryAgainDlg.h"
#include "DboxMain.h" // for CheckPassword()
#include "PasskeySetup.h"

#include "corelib/Util.h"

#include <iomanip>  // For setbase and setw

static wchar_t PSSWDCHAR = L'*';

// See DboxMain.h for the relevant enum
int CPasskeyEntry::dialog_lookup[5] = {
  IDD_PASSKEYENTRY_FIRST,          // GCP_FIRST
  IDD_PASSKEYENTRY,                // GCP_NORMAL
  IDD_PASSKEYENTRY,                // GCP_UNMINIMIZE
  IDD_PASSKEYENTRY_WITHEXIT,       // GCP_WITHEXIT
  IDD_PASSKEYENTRY_WITHEXIT};      // GCP_ADVANCED

//-----------------------------------------------------------------------------
CPasskeyEntry::CPasskeyEntry(CWnd* pParent, const CString& a_filespec, int index,
                             bool bReadOnly, bool bForceReadOnly, int adv_type)
  : CPWDialog(dialog_lookup[index], pParent),
  m_index(index),
  m_filespec(a_filespec), m_orig_filespec(a_filespec),
  m_tries(0),
  m_status(TAR_INVALID),
  m_PKE_ReadOnly(bReadOnly ? TRUE : FALSE),
  m_bForceReadOnly(bForceReadOnly),
  m_adv_type(adv_type), m_bAdvanced(false),
  m_subgroup_set(BST_UNCHECKED),
  m_subgroup_name(L""), m_subgroup_object(0), m_subgroup_function(0),
  m_treatwhitespaceasempty(BST_CHECKED), m_pVKeyBoardDlg(NULL)
{
  DBGMSG("CPasskeyEntry()\n");
  if (m_index == GCP_FIRST) {
    DBGMSG("** FIRST **\n");
  }

  m_passkey = L"";
  m_hIcon = app.LoadIcon(IDI_CORNERICON);
  m_message = a_filespec;
  m_bsFields.set();

  if (pws_os::getenv("PWS_PW_MODE", false) == L"NORMAL")
    m_pctlPasskey->SetSecure(false);

  m_pDbx = static_cast<DboxMain *>(pParent);
  ASSERT(m_pDbx != NULL);

  m_pctlPasskey = new CSecEditExtn;
}

CPasskeyEntry::~CPasskeyEntry()
{
  ::DestroyIcon(m_hIcon);
  delete m_pctlPasskey;
  
  if (m_pVKeyBoardDlg != NULL) {
    // Save Last Used Keyboard
    UINT uiKLID = m_pVKeyBoardDlg->GetKLID();
    std::wostringstream os;
    os.fill(L'0');
    os << std::nouppercase << std::hex << std::setw(8) << uiKLID;
    StringX cs_KLID = os.str().c_str();
    PWSprefs::GetInstance()->SetPref(PWSprefs::LastUsedKeyboard, cs_KLID);

    m_pVKeyBoardDlg->DestroyWindow();
    delete m_pVKeyBoardDlg;
  }
}

void CPasskeyEntry::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  // Can't use DDX_Text for CSecEditExtn
  m_pctlPasskey->DoDDX(pDX, m_passkey);

#if !defined(POCKET_PC)
  if (m_index == GCP_FIRST)
    DDX_Control(pDX, IDC_STATIC_LOGOTEXT, m_ctlLogoText);
#endif

  //{{AFX_DATA_MAP(CPasskeyEntry)
#if !defined(POCKET_PC)
  DDX_Control(pDX, IDC_STATIC_LOGO, m_ctlLogo);
  DDX_Control(pDX, IDOK, m_ctlOK);
#endif
  DDX_Control(pDX, IDC_PASSKEY, *m_pctlPasskey);
  DDX_Text(pDX, IDC_MESSAGE, m_message);
  if (m_index == GCP_FIRST)
    DDX_Control(pDX, IDC_DATABASECOMBO, m_MRU_combo);
  DDX_Check(pDX, IDC_READONLY, m_PKE_ReadOnly);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPasskeyEntry, CPWDialog)
  //{{AFX_MSG_MAP(CPasskeyEntry)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDC_CREATE_DB, OnCreateDb)
  ON_BN_CLICKED(IDC_EXITADVANCED, OnExitAdvanced)
  ON_CBN_EDITCHANGE(IDC_DATABASECOMBO, OnComboEditChange)
  ON_CBN_SELCHANGE(IDC_DATABASECOMBO, OnComboSelChange)
  ON_BN_CLICKED(IDC_BTN_BROWSE, OnOpenFileBrowser)
  ON_MESSAGE(WM_INSERTBUFFER, OnInsertBuffer)
  ON_STN_CLICKED(IDC_VKB, OnVirtualKeyboard)
#if defined(POCKET_PC)
  ON_EN_SETFOCUS(IDC_PASSKEY, OnPasskeySetfocus)
  ON_EN_KILLFOCUS(IDC_PASSKEY, OnPasskeyKillfocus)
#endif
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

static CString NarrowPathText(const CString &text)
{
  const int Width = 48;
  CString retval;
  if (text.GetLength() > Width) {
    retval =  text.Left(Width / 2 - 5) +
                   L" ... " + text.Right(Width / 2);
  } else {
    retval = text;
  }
  return retval;
}

BOOL CPasskeyEntry::OnInitDialog(void)
{
#if defined(POCKET_PC)
  // If displaying IDD_PASSKEYENTRY_FIRST then bypass superclass and go
  // directly to CDialog::OnInitDialog() and display the dialog fullscreen
  // otherwise display as a centred dialogue.
  if (m_nIDHelp == IDD) {
    CDialog::OnInitDialog();
  } else {
    CPWDialog::OnInitDialog();
  }
#else
  CPWDialog::OnInitDialog();
#endif

  ApplyPasswordFont(GetDlgItem(IDC_PASSKEY));

  m_pctlPasskey->SetPasswordChar(PSSWDCHAR);

  switch(m_index) {
    case GCP_FIRST:
      // At start up - give the user the option unless file is R/O
      GetDlgItem(IDC_READONLY)->EnableWindow(m_bForceReadOnly ? FALSE : TRUE);
      GetDlgItem(IDC_READONLY)->ShowWindow(SW_SHOW);
      GetDlgItem(IDC_VERSION)->SetWindowText(m_appversion);
#ifdef DEMO
      GetDlgItem(IDC_SPCL_TXT)->
          SetWindowText(CString(MAKEINTRESOURCE(IDS_DEMO)));
#endif
      break;
    case GCP_NORMAL:
    case GCP_ADVANCED:
      // otherwise during open - user can - again unless file is R/O
      GetDlgItem(IDC_READONLY)->EnableWindow(m_bForceReadOnly ? FALSE : TRUE);
      GetDlgItem(IDC_READONLY)->ShowWindow(SW_SHOW);
      break;
    case GCP_UNMINIMIZE:
    case GCP_WITHEXIT:
      // on UnMinimize - user can't change status
      GetDlgItem(IDC_READONLY)->EnableWindow(FALSE);
      GetDlgItem(IDC_READONLY)->ShowWindow(SW_HIDE);
      break;
    default:
      ASSERT(FALSE);
  }

  // Only show virtual Keyboard menu if we can load DLL
  if (!CVKeyBoardDlg::IsOSKAvailable()) {
    GetDlgItem(IDC_VKB)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_VKB)->EnableWindow(FALSE);
  }

  if (m_message.IsEmpty() && m_index == GCP_FIRST) {
    m_pctlPasskey->EnableWindow(FALSE);
#if !defined(POCKET_PC)
    m_ctlOK.EnableWindow(FALSE);
#endif
    m_message.LoadString(IDS_NOCURRENTSAFE);
  }

  if (m_index == GCP_FIRST) {
    GetDlgItem(IDC_MESSAGE)->ShowWindow(SW_HIDE);

    CRecentFileList *mru = app.GetMRU();
    ASSERT(mru != NULL);

    const int N = mru->GetSize();

    std::vector<CSecString> cs_tooltips;

    if (!m_filespec.IsEmpty()) {
      cs_tooltips.push_back(m_filespec);
      m_MRU_combo.AddString(NarrowPathText(m_filespec));
      m_MRU_combo.SelectString(-1, NarrowPathText(m_filespec));
      m_MRU_combo.SetItemData(0, DWORD_PTR(-1));
    }

    for (int i = 0; i < N; i++) {
      const CString &str = (*mru)[i];
      if (str != m_filespec && !str.IsEmpty()) {
        cs_tooltips.push_back(str);
        int li = m_MRU_combo.AddString(NarrowPathText(str));
        if (li != CB_ERR && li != CB_ERRSPACE)
          m_MRU_combo.SetItemData(li, i);
      }
    }
    if (N > 0) {
      SetHeight(N);
    }
    m_MRU_combo.SetToolTipStrings(cs_tooltips);
  }

  // Change Exit button to say "Advanced" if necessary!
  if (m_index == GCP_ADVANCED) {
    GetDlgItem(IDC_EXITADVANCED)->
      SetWindowText(CString(MAKEINTRESOURCE(IDS_ADVANCED)));
  }

  /*
  * this bit makes the background come out right on
  * the bitmaps
  */

#if !defined(POCKET_PC)
  if (m_index == GCP_FIRST) {
    m_ctlLogoText.ReloadBitmap(IDB_PSLOGO);
    m_ctlLogo.ReloadBitmap(IDB_CLOGO);
  } else {
    m_ctlLogo.ReloadBitmap(IDB_CLOGO_SMALL);
  }
#endif

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog

  SetIcon(m_hIcon, TRUE);  // Set big icon
  SetIcon(m_hIcon, FALSE); // Set small icon

  if (app.WasHotKeyPressed()) {
    // Reset it
    app.SetHotKeyPressed(false);

    // Need to get it to the top as a result of a hotkey.
    // This is "stronger" than BringWindowToTop().
    SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    SetActiveWindow();
    SetForegroundWindow();
  }

  if (m_pctlPasskey->IsWindowEnabled() == TRUE) {
    m_pctlPasskey->SetFocus();
    return FALSE;
  } else
    return TRUE;
}

#if defined(POCKET_PC)
/************************************************************************/
/* Restore the state of word completion when the password field loses   */
/* focus.                                                               */
/************************************************************************/
void CPasskeyEntry::OnPasskeyKillfocus()
{
  EnableWordCompletion( m_hWnd );
}

/************************************************************************/
/* When the password field is activated, pull up the SIP and disable    */
/* word completion.                                                     */
/************************************************************************/
void CPasskeyEntry::OnPasskeySetfocus()
{
  DisableWordCompletion( m_hWnd );
}
#endif

void CPasskeyEntry::OnCreateDb()
{
  // 1. Get a filename from a file dialog box
  // 2. Get a password
  // 3. Set m_filespec && m_passkey to returned value!
  INT_PTR rc;
  CString newfile;
  CString cs_msg, cs_title, cs_temp;
  CString cs_text(MAKEINTRESOURCE(IDS_CREATENAME));

  CString cf(MAKEINTRESOURCE(IDS_DEFDBNAME)); // reasonable default for first time user
  std::wstring v3FileName = PWSUtil::GetNewFileName(LPCWSTR(cf), DEFAULT_SUFFIX);

  while (1) {
    CPWFileDialog fd(FALSE,
                     DEFAULT_SUFFIX,
                     v3FileName.c_str(),
                     OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                        OFN_LONGNAMES | OFN_OVERWRITEPROMPT,
                     CString(MAKEINTRESOURCE(IDS_FDF_V3_ALL)),
                     this);
    fd.m_ofn.lpstrTitle = cs_text;
    std::wstring dir = PWSdirs::GetSafeDir();
    if (!dir.empty())
      fd.m_ofn.lpstrInitialDir = dir.c_str();
    rc = fd.DoModal();
    if (((DboxMain*) GetParent())->ExitRequested()) {
      // If U3ExitNow called while in CPWFileDialog,
      // PostQuitMessage makes us return here instead
      // of exiting the app. Try resignalling
      PostQuitMessage(0);
      return;
    }
    if (rc == IDOK) {
      newfile = fd.GetPathName();
      break;
    } else
      return;
  }

  // 2. Get a password
  CPasskeySetup dbox_pksetup(this);
  rc = dbox_pksetup.DoModal();

  if (rc != IDOK)
    return;  //User cancelled password entry

  // 3. Set m_filespec && m_passkey to returned value!
  m_filespec = newfile;
  m_passkey = dbox_pksetup.m_passkey;
  ((CEdit*)GetDlgItem(IDC_PASSKEY))->SetWindowText(m_passkey);
  m_status = TAR_NEW;
  CPWDialog::OnOK();
}

void CPasskeyEntry::OnCancel()
{
  m_status = TAR_CANCEL;
  CPWDialog::OnCancel();
}

void CPasskeyEntry::OnExitAdvanced()
{
  if (m_index == GCP_WITHEXIT) {
    m_status = TAR_EXIT;
    CPWDialog::OnCancel();
    return;
  }

  CAdvancedDlg Adv(this, m_adv_type, m_bsFields, m_subgroup_name,
    m_subgroup_set, m_subgroup_object, m_subgroup_function);

  INT_PTR rc = Adv.DoModal();

  if (rc == IDOK) {
    m_bAdvanced = true;
    m_bsFields = Adv.m_bsFields;
    m_subgroup_set = Adv.m_subgroup_set;
    m_treatwhitespaceasempty = Adv.m_treatwhitespaceasempty;
    if (m_subgroup_set == BST_CHECKED) {
      m_subgroup_name = Adv.m_subgroup_name;
      m_subgroup_object = Adv.m_subgroup_object;
      m_subgroup_function = Adv.m_subgroup_function;
    }
  } else {
    m_bAdvanced = false;
  }
}

void CPasskeyEntry::OnOK()
{
  UpdateData(TRUE);

  if (m_passkey.IsEmpty()) {
    AfxMessageBox(IDS_CANNOTBEBLANK);
    m_pctlPasskey->SetFocus();
    return;
  }

  if (!pws_os::FileExists(m_filespec.GetString())) {
    AfxMessageBox(IDS_FILEPATHNOTFOUND);
    if (m_MRU_combo.IsWindowVisible())
      m_MRU_combo.SetFocus();
    return;
  }

  ProcessPhrase();
}

void CPasskeyEntry::ProcessPhrase()
{
  if (m_pDbx->CheckPassword(LPCWSTR(m_filespec), LPCWSTR(m_passkey)) != PWScore::SUCCESS) {
    if (m_tries >= 2) {
      CTryAgainDlg errorDlg(this);

      INT_PTR nResponse = errorDlg.DoModal();
      if (nResponse == IDOK) {
      } else if (nResponse == IDCANCEL) {
        m_status = errorDlg.GetCancelReturnValue();
        if (m_status == TAR_OPEN) { // open another
          PostMessage(WM_COMMAND, IDC_BTN_BROWSE);
          return;
        } else if (m_status == TAR_NEW) { // create new
          PostMessage(WM_COMMAND, IDC_CREATE_DB);
          return;
        }
        CPWDialog::OnCancel();
      }
    } else {
      m_tries++;
      AfxMessageBox(IDS_INCORRECTKEY);
      m_pctlPasskey->SetSel(MAKEWORD(-1, 0));
      m_pctlPasskey->SetFocus();
    }
  } else {
    CPWDialog::OnOK();
  }
}

void CPasskeyEntry::OnHelp()
{
#if defined(POCKET_PC)
  CreateProcess(L"PegHelp.exe", L"pws_ce_help.html#comboentry", NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);
#else
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/create_new_db.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
#endif
}

//-----------------------------------------------------------------------------

void CPasskeyEntry::UpdateRO()
{
  if (!m_bForceReadOnly) { // if allowed, changed r-o state to reflect file's permission
    bool fro;
    if (pws_os::FileExists(LPCWSTR(m_filespec), fro) && fro) {
      m_PKE_ReadOnly = TRUE;
      GetDlgItem(IDC_READONLY)->EnableWindow(FALSE);
    } else { // no file or write-enabled
      m_PKE_ReadOnly = FALSE;
      GetDlgItem(IDC_READONLY)->EnableWindow(TRUE);
    }
    UpdateData(FALSE);
  } // !m_bForceReadOnly
}

void CPasskeyEntry::OnComboEditChange()
{
  m_MRU_combo.m_edit.GetWindowText(m_filespec);
  m_pctlPasskey->EnableWindow(TRUE);
  m_ctlOK.EnableWindow(TRUE);
  UpdateRO();
}

void CPasskeyEntry::OnComboSelChange()
{
  CRecentFileList *mru = app.GetMRU();
  int curSel = m_MRU_combo.GetCurSel();
  const int N = mru->GetSize();
  if (curSel == CB_ERR || curSel >= N) {
    ASSERT(0);
  } else {
    int i = int(m_MRU_combo.GetItemData(curSel));
    if (i >= 0) // -1 means original m_filespec
      m_filespec = (*mru)[i];
    else
      m_filespec = m_orig_filespec;
  }
  m_pctlPasskey->EnableWindow(TRUE);
  m_pctlPasskey->SetFocus();
  m_ctlOK.EnableWindow(TRUE);
  UpdateRO();
}

void CPasskeyEntry::OnOpenFileBrowser()
{
  CString cs_text(MAKEINTRESOURCE(IDS_CHOOSEDATABASE));

  //Open-type dialog box
  CPWFileDialog fd(TRUE,
                   DEFAULT_SUFFIX,
                   NULL,
                   OFN_FILEMUSTEXIST | OFN_LONGNAMES,
                   CString(MAKEINTRESOURCE(IDS_FDF_DB_BU_ALL)),
                   this);
  fd.m_ofn.lpstrTitle = cs_text;
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultOpenRO))
      fd.m_ofn.Flags |= OFN_READONLY;
    else
      fd.m_ofn.Flags &= ~OFN_READONLY;
  std::wstring dir = PWSdirs::GetSafeDir();
  if (!dir.empty())
    fd.m_ofn.lpstrInitialDir = dir.c_str();

  INT_PTR rc = fd.DoModal();
  if (((DboxMain*) GetParent())->ExitRequested()) {
    // If U3ExitNow called while in CPWFileDialog,
    // PostQuitMessage makes us return here instead
    // of exiting the app. Try resignalling
    PostQuitMessage(0);
    return;
  }
  if (rc == IDOK) {
    m_PKE_ReadOnly = fd.GetReadOnlyPref();
    ((CButton *)GetDlgItem(IDC_READONLY))->SetCheck(m_PKE_ReadOnly == TRUE ? 
                                           BST_CHECKED : BST_UNCHECKED);
    m_filespec = fd.GetPathName();
    m_MRU_combo.m_edit.SetWindowText(m_filespec);
    m_pctlPasskey->EnableWindow(TRUE);
    m_ctlOK.EnableWindow(TRUE);
    if (m_pctlPasskey->IsWindowEnabled() == TRUE) {
      m_pctlPasskey->SetFocus();
    }
    UpdateRO();
  } // rc == IDOK
}

void CPasskeyEntry::SetHeight(const int num)
{
  // Find the longest string in the list box.
  CString str;
  CRect rect;
  CSize sz;

  // Try to ensure that dropdown list is big enough for all entries and
  // therefore no scrolling
  int ht = m_MRU_combo.GetItemHeight(0);

  m_MRU_combo.GetWindowRect(&rect);

  sz.cx = rect.Width();
  sz.cy = ht * (num + 2);

  if ((rect.top - sz.cy) < 0 || 
      (rect.bottom + sz.cy > ::GetSystemMetrics(SM_CYSCREEN))) {
    int ifit = max((rect.top / ht), (::GetSystemMetrics(SM_CYSCREEN) - rect.bottom) / ht);
    int ht2 = ht * ifit;
    sz.cy = min(ht2, sz.cy);
  }

  m_MRU_combo.SetWindowPos(NULL, 0, 0, sz.cx, sz.cy, SWP_NOMOVE | SWP_NOZORDER);
}

void CPasskeyEntry::OnVirtualKeyboard()
{
  // Shouldn't be here if couldn't load DLL. Static control disabled/hidden
  if (!CVKeyBoardDlg::IsOSKAvailable())
    return;

  if (m_pVKeyBoardDlg != NULL && m_pVKeyBoardDlg->IsWindowVisible()) {
    // Already there - move to top
    m_pVKeyBoardDlg->SetWindowPos(&wndTop , 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    return;
  }

  // If not already created - do it, otherwise just reset it
  if (m_pVKeyBoardDlg == NULL) {
    StringX cs_LUKBD = PWSprefs::GetInstance()->GetPref(PWSprefs::LastUsedKeyboard);
    m_pVKeyBoardDlg = new CVKeyBoardDlg(this, cs_LUKBD.c_str());
    m_pVKeyBoardDlg->Create(CVKeyBoardDlg::IDD);
  } else {
    m_pVKeyBoardDlg->ResetKeyboard();
  }

  // Now show it and make it top
  m_pVKeyBoardDlg->SetWindowPos(&wndTop , 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

  return;
}

LRESULT CPasskeyEntry::OnInsertBuffer(WPARAM, LPARAM)
{
  // Update the variables
  UpdateData(TRUE);

  // Get the buffer
  CSecString vkbuffer = m_pVKeyBoardDlg->GetPassphrase();

  // Find the selected characters - if any
  int nStartChar, nEndChar;
  m_pctlPasskey->GetSel(nStartChar, nEndChar);

  // If any characters selected - delete them
  if (nStartChar != nEndChar)
    m_passkey.Delete(nStartChar, nEndChar - nStartChar);

  // Insert the buffer
  m_passkey.Insert(nStartChar, vkbuffer);

  // Update the dialog
  UpdateData(FALSE);

  // Put cursor at end of inserted text
  m_pctlPasskey->SetSel(nStartChar + vkbuffer.GetLength(), 
                        nStartChar + vkbuffer.GetLength());

  return 0L;
}

