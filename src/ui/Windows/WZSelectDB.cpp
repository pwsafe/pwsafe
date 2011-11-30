/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "passwordsafe.h"
#include "DboxMain.h"

#include "WZSelectDB.h"
#include "WZPropertySheet.h"

#include "PwFont.h"
#include "TryAgainDlg.h"
#include "SecString.h"
#include "GeneralMsgBox.h"
#include "PWFileDialog.h"
#include "VirtualKeyboard/VKeyBoardDlg.h"

#include "core/Util.h"
#include "core/PWSdirs.h"
#include "os/env.h"
#include "os/file.h"
#include "os/dir.h"
#include "yubi/yklib.h"

#include "resource3.h"

#include <sstream>
#include <iomanip>
#include <shlobj.h>


using namespace std;

const wchar_t CWZSelectDB::PSSWDCHAR = L'*';

IMPLEMENT_DYNAMIC(CWZSelectDB, CWZPropertyPage)


CWZSelectDB::CWZSelectDB(CWnd *pParent, UINT nIDCaption, const int nType)
 : CWZPropertyPage(IDD, nIDCaption, nType), m_tries(0), m_state(0),
  m_pVKeyBoardDlg(NULL), m_bAdvanced(BST_UNCHECKED),
  m_bFileExistsUserAsked(false),
  m_filespec(L""), m_passkey(L""),
  m_defexpdelim(L"\xbb"), m_pctlDB(new CEditExtn), m_pending(false),
  m_pctlPasskey(new CSecEditExtn)
{
  m_pWZPSH = (CWZPropertySheet *)pParent;
  if (pws_os::getenv("PWS_PW_MODE", false) == L"NORMAL")
    m_pctlPasskey->SetSecure(false);
  m_present = !IsYubiInserted(); // lie to trigger correct actions in timer event
}

CWZSelectDB::~CWZSelectDB()
{
  delete m_pctlDB;
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

void CWZSelectDB::DoDataExchange(CDataExchange* pDX)
{
  CWZPropertyPage::DoDataExchange(pDX);

  // Can't use DDX_Text for CSecEditExtn
  m_pctlPasskey->DoDDX(pDX, m_passkey);

  //{{AFX_DATA_MAP(CWZSelectDB)
  DDX_Text(pDX, IDC_DATABASE, m_filespec);

  DDX_Control(pDX, IDC_PASSKEY, *m_pctlPasskey);
  DDX_Control(pDX, IDC_DATABASE, *m_pctlDB);
  DDX_Check(pDX, IDC_ADVANCED, m_bAdvanced);
  DDX_Control(pDX, IDC_YUBI_PROGRESS, m_yubi_timeout);
  DDX_Control(pDX, IDC_YUBI_STATUS, m_yubi_status);

  const UINT nID = m_pWZPSH->GetID();

  if (nID == ID_MENUITEM_SYNCHRONIZE         ||
      nID == ID_MENUITEM_EXPORT2PLAINTEXT    ||
      nID == ID_MENUITEM_EXPORTENT2PLAINTEXT ||
      nID == ID_MENUITEM_EXPORT2XML          ||
      nID == ID_MENUITEM_EXPORTENT2XML) {
    DDX_Control(pDX, IDC_STATIC_WZWARNING, m_stc_warning);

    if (nID != ID_MENUITEM_SYNCHRONIZE) {
      DDX_Text(pDX, IDC_WZDEFEXPDELIM, m_defexpdelim);
      DDV_MaxChars(pDX, m_defexpdelim, 1);
      DDV_CheckExpDelimiter(pDX, m_defexpdelim);
    }
  }
  //}}AFX_DATA_MAP
}

void AFXAPI CWZSelectDB::DDV_CheckExpDelimiter(CDataExchange* pDX, 
                                               const CString &delimiter)
{
  if (pDX->m_bSaveAndValidate) {
    if (delimiter.IsEmpty()) {
      CGeneralMsgBox gmb;
      gmb.AfxMessageBox(IDS_NEEDDELIMITER);
      pDX->Fail();
      return;
    }   
    if (delimiter[0] == '"') {
      CGeneralMsgBox gmb;
      gmb.AfxMessageBox(IDS_INVALIDDELIMITER);
      pDX->Fail();
    }
  }
}

BEGIN_MESSAGE_MAP(CWZSelectDB, CWZPropertyPage)
  //{{AFX_MSG_MAP(CWZSelectDB)
  ON_WM_CTLCOLOR()
  ON_EN_CHANGE(IDC_PASSKEY, OnPassKeyChange)
  ON_EN_CHANGE(IDC_DATABASE, OnDatabaseChange)
  ON_BN_CLICKED(IDC_BTN_BROWSE, OnOpenFileBrowser)
  ON_STN_CLICKED(IDC_VKB, OnVirtualKeyboard)
  ON_MESSAGE(PWS_MSG_INSERTBUFFER, OnInsertBuffer)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDC_ADVANCED, OnAdvanced)
  //}}AFX_MSG_MAP
  ON_BN_CLICKED(IDC_YUBIKEY_BTN, OnYubikeyBtn)
  ON_WM_TIMER()
END_MESSAGE_MAP()

void CWZSelectDB::OnHelp()
{
  CString cs_HelpTopic = app.GetHelpFileName() + L"::/html/wzselectdb.html";
  ::HtmlHelp(this->GetSafeHwnd(), (LPCWSTR)cs_HelpTopic, HH_DISPLAY_TOPIC, 0);
}


BOOL CWZSelectDB::OnInitDialog()
{
  CWZPropertyPage::OnInitDialog();
  SetTimer(1, 250, 0); // Setup a timer to poll YubiKey every 250 ms
  ApplyPasswordFont(GetDlgItem(IDC_PASSKEY));
  m_pctlPasskey->SetPasswordChar(PSSWDCHAR);

  const UINT nID = m_pWZPSH->GetID();
  CString cs_text,cs_temp;

  bool bWARNINGTEXT(true);
  switch (nID) {
    case ID_MENUITEM_SYNCHRONIZE:
      cs_text.LoadString(IDS_WZSLCT_WARNING_SYNC);
      break;
    case ID_MENUITEM_EXPORT2PLAINTEXT:
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
      cs_temp.LoadString((nID == ID_MENUITEM_EXPORT2PLAINTEXT || nID == ID_MENUITEM_EXPORT2XML) ?
                              IDS_WSSLCT_ALL : IDS_WSSLCT_ENTRY);
      cs_text.Format(IDS_WZSLCT_WARNING_EXP, cs_temp);
      break;
    case ID_MENUITEM_COMPARE:
    case ID_MENUITEM_MERGE:
      bWARNINGTEXT = false;
      break;
    default:
      bWARNINGTEXT = false;
      ASSERT(0);
  }

  if (bWARNINGTEXT) {
    GetDlgItem(IDC_STATIC_WZWARNING)->SetWindowText(cs_text);
    m_stc_warning.SetColour(RGB(255,0,0));
    if (nID != ID_MENUITEM_EXPORT2PLAINTEXT && nID != ID_MENUITEM_EXPORTENT2PLAINTEXT)
      GetDlgItem(IDC_STATIC_WZEXPDLM2)->ShowWindow(SW_HIDE);

    LOGFONT LogFont;
    m_stc_warning.GetFont()->GetLogFont(&LogFont);
    LogFont.lfHeight = -14; // -14 stands for the size 14
    LogFont.lfWeight = FW_BOLD;

    m_WarningFont.CreateFontIndirect(&LogFont);
    m_stc_warning.SetFont(&m_WarningFont);
  } else
    GetDlgItem(IDC_STATIC_WZWARNING)->ShowWindow(SW_HIDE);

  std::wstring ExportFileName;
  UINT uifilemsg(IDS_WZDATABASE);
  switch (nID) {
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
        GetDlgItem(IDC_STATIC_WZEXPDLM2)->ShowWindow(SW_HIDE);
        // Drop though intentionally
    case ID_MENUITEM_EXPORT2PLAINTEXT:
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
        ExportFileName = PWSUtil::GetNewFileName(m_pWZPSH->WZPSHGetCurFile().c_str(),
            (nID == ID_MENUITEM_EXPORT2XML || nID == ID_MENUITEM_EXPORTENT2XML) ?
               L"xml" : L"txt");
        m_pctlDB->SetWindowText(ExportFileName.c_str());
        m_filespec = ExportFileName.c_str();
        uifilemsg = IDS_WZFILE;
      break;
    case ID_MENUITEM_SYNCHRONIZE:
    case ID_MENUITEM_COMPARE:
    case ID_MENUITEM_MERGE:
      GetDlgItem(IDC_STATIC_WZEXPDLM1)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_STATIC_WZEXPDLM2)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_WZDEFEXPDELIM)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_WZDEFEXPDELIM)->EnableWindow(FALSE);
      break;
    default:
      ASSERT(0);
  }
  cs_text.LoadString(uifilemsg);
  GetDlgItem(IDC_STATIC_WZFILE)->SetWindowText(cs_text);

  // Only show virtual Keyboard menu if we can load DLL
  if (!CVKeyBoardDlg::IsOSKAvailable()) {
    GetDlgItem(IDC_VKB)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_VKB)->EnableWindow(FALSE);
  }

  // Disble passphrase until database name filled in
  m_pctlPasskey->EnableWindow(TRUE);

  // Disable Next until fields set
  m_pWZPSH->SetWizardButtons(0);

  CString cs_tmp(MAKEINTRESOURCE(m_pWZPSH->GetButtonID()));
  m_pWZPSH->GetDlgItem(ID_WIZNEXT)->SetWindowText(cs_tmp);

  GetDlgItem(IDC_DATABASE)->SetFocus();

  // Yubi-related initializations:
  bool yubiEnabled = true;
  m_yubiLogo.LoadBitmap(IDB_YUBI_LOGO);
  CWnd *ybn = GetDlgItem(IDC_YUBIKEY_BTN);

  ((CButton*)ybn)->SetBitmap(m_yubiLogo);
  ybn->ShowWindow(yubiEnabled ? SW_SHOW : SW_HIDE);
  m_yubi_status.ShowWindow(yubiEnabled ? SW_SHOW : SW_HIDE);
  m_yubi_timeout.ShowWindow(SW_HIDE);
  m_yubi_timeout.SetRange(0, 15);
  bool yubiInserted = IsYubiInserted();
  ybn->EnableWindow(yubiInserted ? TRUE : FALSE);
  if (yubiInserted)
    m_yubi_status.SetWindowText(_T("Click, then activate your YubiKey"));
  else
    m_yubi_status.SetWindowText(_T("Please insert your YubiKey"));

  return FALSE;
}

HBRUSH CWZSelectDB::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CWZPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

  // Only deal with Static controls and then
  // Only with our special ones
  if (nCtlColor == CTLCOLOR_STATIC) {
    COLORREF *pcfOld;
    UINT nID = pWnd->GetDlgCtrlID();
    switch (nID) {
      case IDC_STATIC_WZWARNING:
        pcfOld = &m_syncwarning_cfOldColour;
        break;
      default:
        // Not one of ours - get out quick
        return hbr;
    }

    int iFlashing = ((CStaticExtn *)pWnd)->IsFlashing();
    BOOL bHighlight = ((CStaticExtn *)pWnd)->IsHighlighted();
    BOOL bMouseInWindow = ((CStaticExtn *)pWnd)->IsMouseInWindow();

    if (iFlashing != 0) {
      pDC->SetBkMode(iFlashing == 1 || (iFlashing && bHighlight && bMouseInWindow) ?
                     OPAQUE : TRANSPARENT);
      COLORREF cfFlashColour = ((CStaticExtn *)pWnd)->GetFlashColour();
      *pcfOld = pDC->SetBkColor(iFlashing == 1 ? cfFlashColour : *pcfOld);
    } else
    if (bHighlight) {
      pDC->SetBkMode(bMouseInWindow ? OPAQUE : TRANSPARENT);
      COLORREF cfHighlightColour = ((CStaticExtn *)pWnd)->GetHighlightColour();
      *pcfOld = pDC->SetBkColor(bMouseInWindow ? cfHighlightColour : *pcfOld);
    } else
    if (((CStaticExtn *)pWnd)->GetColourState()) {
      COLORREF cfUser = ((CStaticExtn *)pWnd)->GetUserColour();
      pDC->SetTextColor(cfUser);
    }
  }

  // Let's get out of here
  return hbr;
}

BOOL CWZSelectDB::OnSetActive()
{
  CWZPropertyPage::OnSetActive();

  m_pWZPSH->SetWizardButtons(m_state == BOTHPRESENT ? PSWIZB_NEXT : 0);

  return TRUE;
}

void CWZSelectDB::OnAdvanced()
{
  //m_bAdvanced = ((CButton*)GetDlgItem(IDC_ADVANCED))->GetCheck();
  UpdateData(TRUE);

  CString cs_tmp(MAKEINTRESOURCE(m_bAdvanced == BST_UNCHECKED ? m_pWZPSH->GetButtonID() :
                                        IDS_WZNEXT));

  m_pWZPSH->GetDlgItem(ID_WIZNEXT)->SetWindowText(cs_tmp);
}

void CWZSelectDB::OnPassKeyChange()
{
  CString cs_Passkey;
  m_pctlPasskey->GetWindowText(cs_Passkey);

  if (cs_Passkey.GetLength() > 0)
    m_state |= KEYPRESENT;
  else
    m_state &= ~KEYPRESENT;

  m_passkey = CSecString(cs_Passkey);

  m_pWZPSH->SetWizardButtons(m_state == BOTHPRESENT ? PSWIZB_NEXT : 0);
}

void CWZSelectDB::OnDatabaseChange()
{
  CString cs_DB;
  m_pctlDB->GetWindowText(cs_DB);

  if (cs_DB.GetLength() > 0)
    m_state |= DBPRESENT;
  else
    m_state &= ~DBPRESENT;

  m_filespec = cs_DB;

  m_pWZPSH->SetWizardButtons(m_state == BOTHPRESENT ? PSWIZB_NEXT : 0);
}

LRESULT CWZSelectDB::OnWizardNext()
{
  UpdateData(TRUE);

  CGeneralMsgBox gmb;

  const UINT nID = m_pWZPSH->GetID();
  bool bFileExists = pws_os::FileExists(m_filespec.GetString());
  bool bExportXML(true);

  switch (nID) {
    case ID_MENUITEM_COMPARE:
    case ID_MENUITEM_MERGE:
    case ID_MENUITEM_SYNCHRONIZE:
      if (!bFileExists) {
        // Database must exit for these if other database does not exist
        gmb.AfxMessageBox(IDS_FILEPATHNOTFOUND);
        m_pctlDB->SetFocus();
        return -1;
      }
      break;
    case ID_MENUITEM_EXPORT2PLAINTEXT:
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
      bExportXML = false; // Fall through on purpose
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
      if (bFileExists) {
        // Check if OK to overwrite existing file - if not already asked by user clicking
        // file browser button
        if (!m_bFileExistsUserAsked) {
          CString cs_msg, cs_title(MAKEINTRESOURCE(bExportXML ? IDS_WZEXPORTXML : IDS_WZEXPORTTEXT));
          cs_msg.Format(IDS_REPLACEEXPORTFILE, m_filespec);
          INT_PTR rc = gmb.AfxMessageBox(cs_msg, cs_title,
                         MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
          if (rc == IDNO) {
            m_pctlDB->SetFocus();
            return -1;
          }
        }
      } else {
        // File does not exist - but need to check if the path exists as
        // pws_os::FOpen(...) cannot create any missing directories
        std::wstring dir, cdrive, cdir, cfile, cextn;
        pws_os::splitpath(m_filespec.GetString(), cdrive, cdir, cfile, cextn);
        dir = cdrive + cdir;
        int rc = ::SHCreateDirectoryEx(NULL, dir.c_str(), NULL);
        switch (rc) {
          case ERROR_SUCCESS:          // Path created
          case ERROR_FILE_EXISTS:      // Directory already there
          case ERROR_ALREADY_EXISTS:   // Directory already there
            break;
          default:
            // Could not create path!
            gmb.AfxMessageBox(IDS_CANNOTCREATEDIR);
            return -1;
        }
      }
      break;
    default:
      // No idea why here!
      ASSERT(0);
      return -1;
  }

  if (m_passkey.IsEmpty()) {
    gmb.AfxMessageBox(IDS_CANNOTBEBLANK);
    m_pctlPasskey->SetFocus();
    return -1;
  }

  // Check that this file isn't already open
  const StringX sx_Filename1(m_pWZPSH->WZPSHGetCurFile());
  const StringX sx_Filename2 = m_filespec.GetString();
  const StringX sx_passkey = StringX(m_passkey);
  const bool bOtherIsDB = nID == ID_MENUITEM_COMPARE ||
                          nID == ID_MENUITEM_MERGE   ||
                          nID == ID_MENUITEM_SYNCHRONIZE;

  if (bOtherIsDB && sx_Filename2 == sx_Filename1) {
    // It is the same damn file
    gmb.AfxMessageBox(IDS_COMPARESAME, MB_OK | MB_ICONWARNING);
    return -1;
  }

  if (!ProcessPhrase(bOtherIsDB ? sx_Filename2 : sx_Filename1, sx_passkey))
    return -1;

  m_pWZPSH->SetPassKey(sx_passkey);
  m_pWZPSH->SetOtherDB(sx_Filename2);
  m_pWZPSH->SetDelimiter(m_defexpdelim.GetAt(0));
  m_pWZPSH->SetAdvanced(m_bAdvanced == BST_CHECKED);

  return m_bAdvanced == BST_CHECKED ? 0 : IDD_WZFINISH;
}

bool CWZSelectDB::ProcessPhrase(const StringX &filename, const StringX &passkey)
{
  PWScore tmpcore;
  if (m_pWZPSH->WZPSHCheckPasskey(filename, passkey,
                                  &tmpcore) == PWScore::SUCCESS) {
    m_tries = 0;
    return true;
  }

  if (m_tries >= 2) {
    CTryAgainDlg errorDlg(this);

    INT_PTR nResponse = errorDlg.DoModal();
    if (nResponse == IDCANCEL) {
      int status = errorDlg.GetCancelReturnValue();
      if (status == TAR_OPEN) { // open another
        PostMessage(WM_COMMAND, IDC_BTN_BROWSE);
        return false;
      }
    }
  } else {
    m_tries++;
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_INCORRECTKEY);
    m_pctlPasskey->SetSel(MAKEWORD(-1, 0));
    m_pctlPasskey->SetFocus();
  }
  return false;
}

void CWZSelectDB::OnOpenFileBrowser()
{
  //Open-type dialog box
  CString cs_suffix, cs_filter;
  DWORD dwflags(0);
  UINT uimsgid(IDS_CHOOSEDATABASE);

  const UINT nID = m_pWZPSH->GetID();
  BOOL bTYPE_OPEN(FALSE); // TRUE = Open, FALSE = Save

  switch (nID) {
    case ID_MENUITEM_SYNCHRONIZE:
    case ID_MENUITEM_COMPARE:
    case ID_MENUITEM_MERGE:
      cs_suffix = DEFAULT_SUFFIX;
      cs_filter.LoadString(IDS_FDF_DB_BU_ALL);
      dwflags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY |
                            OFN_LONGNAMES | OFN_READONLY;
      bTYPE_OPEN = TRUE;
      break;
    case ID_MENUITEM_EXPORT2PLAINTEXT:
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
      cs_suffix = L"txt";
      cs_filter.LoadString(IDS_FDF_T_C_ALL);
      dwflags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                            OFN_LONGNAMES | OFN_OVERWRITEPROMPT;
      uimsgid = IDS_NAMETEXTFILE;
      break;
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
      cs_suffix = L"xml";
      cs_filter.LoadString(IDS_FDF_X_ALL);
      dwflags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                            OFN_LONGNAMES | OFN_OVERWRITEPROMPT;
      uimsgid = IDS_NAMEXMLFILE;
      break;
    default:
      ASSERT(0);
  }

  CString cs_text(MAKEINTRESOURCE(uimsgid));

  std::wstring ExportFileName = PWSUtil::GetNewFileName(m_pWZPSH->WZPSHGetCurFile().c_str(), L"txt");
  CPWFileDialog fd(bTYPE_OPEN, cs_suffix, uimsgid != IDS_CHOOSEDATABASE ? ExportFileName.c_str() : NULL, 
                   dwflags, cs_filter, this);

  fd.m_ofn.lpstrTitle = cs_text;

  std::wstring dir, fname;
  if (m_filespec.IsEmpty())
    fname = m_pWZPSH->WZPSHGetCurFile().c_str();
  else
    fname = m_filespec;

  if (fname.empty())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, cfile, cextn;
    pws_os::splitpath(fname.c_str(), cdrive, cdir, cfile, cextn);
    dir = cdrive + cdir;
  }

  if (!dir.empty())
    fd.m_ofn.lpstrInitialDir = dir.c_str();

  INT_PTR rc = fd.DoModal();

  if (m_pWZPSH->WZPSHExitRequested()) {
    // If U3ExitNow called while in CPWFileDialog,
    // PostQuitMessage makes us return here instead
    // of exiting the app. Try resignalling
    PostQuitMessage(0);
    return;
  }

  if (rc == IDOK) {
    m_filespec = fd.GetPathName();
    m_pctlDB->SetWindowText(m_filespec);
    m_pctlPasskey->EnableWindow(TRUE);
    if (m_pctlPasskey->IsWindowEnabled() == TRUE) {
      m_pctlPasskey->SetFocus();
    }
    // If the file exists and we are doing a save, CFileDialog
    // would have prompted the user
    if (bTYPE_OPEN == FALSE && pws_os::FileExists(m_filespec.GetString()))
      m_bFileExistsUserAsked = true;
  } // rc == IDOK
}

void CWZSelectDB::OnVirtualKeyboard()
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

LRESULT CWZSelectDB::OnInsertBuffer(WPARAM, LPARAM)
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

// Yubi-related stuff, copied from PKBaseDlg because MFC broke multiple-inheritance
bool CWZSelectDB::IsYubiInserted() const
{
  if (m_pending)
    return true; // can't check in the middle of a request
  else {
    CYkLib yk;
    CSingleLock singeLock(&m_mutex);
    singeLock.Lock();
    return (yk.enumPorts() == 1);
  }
}

void CWZSelectDB::yubiInserted(void)
{
  GetDlgItem(IDC_YUBIKEY_BTN)->EnableWindow(TRUE);
  m_yubi_status.SetWindowText(_T("Click, then activate your YubiKey"));
}

void CWZSelectDB::yubiRemoved(void)
{
  GetDlgItem(IDC_YUBIKEY_BTN)->EnableWindow(FALSE);
  m_yubi_status.SetWindowText(_T("Please insert your YubiKey"));
}

static StringX Bin2Hex(const unsigned char *buf, int len)
{
  wostringstream os;
  os << setw(2);
  os << setfill(L'0');
  for (int i = 0; i < len; i++) {
    os << hex << setw(2) << int(buf[i]);
  }
  return StringX(os.str().c_str());
}

void CWZSelectDB::yubiCheckCompleted()
{
  // We now wait for a response with the HMAC-SHA1 digest
  BYTE respBuf[SHA1_DIGEST_SIZE];
  unsigned short timer;
  CSingleLock singeLock(&m_mutex);
  CYkLib yk;
  singeLock.Lock();
  if (yk.openKey() != YKLIB_OK) {
    m_yubi_status.SetWindowText(_T("Failed to access YubiKey"));
      return;
  }
  YKLIB_RC rc = yk.waitForCompletion(YKLIB_NO_WAIT,
                                     respBuf, sizeof(respBuf), &timer);
  switch (rc) {
  case YKLIB_OK:
    m_yubi_status.ShowWindow(SW_SHOW);
    m_yubi_timeout.ShowWindow(SW_HIDE);
    m_yubi_timeout.SetPos(0);
    m_yubi_status.SetWindowText(_T(""));
    TRACE(_T("yubiCheckCompleted: YKLIB_OK"));
    m_pending = false;
    m_passkey = Bin2Hex(respBuf, SHA1_DIGEST_SIZE);
    // The returned hash is the passkey
    m_pWZPSH->SetWizardButtons(PSWIZB_NEXT); // enable 
    m_yubi_status.SetWindowText(_T("YubiKey data received")); // shouldn't really show
    // This will check the password, etc.:
    UpdateData(FALSE); // passwd -> control
    m_pWZPSH->PostMessage(WM_COMMAND, MAKELONG(ID_WIZNEXT, BN_CLICKED), 0);
    break;
  case YKLIB_PROCESSING:  // Still processing or waiting for the result
    break;
  case YKLIB_TIMER_WAIT:  // A given number of seconds remain 
    m_yubi_timeout.SetPos(timer);
    break;

  case YKLIB_INVALID_RESPONSE:  // Invalid or no response
    m_pending = false;
    m_yubi_timeout.ShowWindow(SW_HIDE);
    m_yubi_status.SetWindowText(_T("YubiKey timed out"));
    m_yubi_status.ShowWindow(SW_SHOW);
    break;

  default:                // A non-recoverable error has occured
    m_pending = false;
    m_yubi_timeout.ShowWindow(SW_HIDE);
    m_yubi_status.ShowWindow(SW_SHOW);
    yk.closeKey();
    // Generic error message
    TRACE(_T("yubiCompleted(%d)\n"), rc);
    m_yubi_status.SetWindowText(_T("Internal error: Unknown return code"));
    break;
  }
}

void CWZSelectDB::yubiRequestHMACSha1()
{
  if (m_pending) {
    // no-op if a request's already in the air
  } else {
    CYkLib yk;
    CSingleLock singeLock(&m_mutex);
    singeLock.Lock();
    // open key
    // if zero or >1 key, we'll fail
    if (yk.openKey() != YKLIB_OK) {
      return;
    }

    // Prepare the HMAC-SHA1 challenge here

    BYTE chalBuf[SHA1_MAX_BLOCK_SIZE];
    BYTE chalLength = BYTE(m_passkey.GetLength()*sizeof(TCHAR));
    memset(chalBuf, 0, SHA1_MAX_BLOCK_SIZE);
    if (chalLength > SHA1_MAX_BLOCK_SIZE)
      chalLength = SHA1_MAX_BLOCK_SIZE;

    memcpy(chalBuf, m_passkey, chalLength);

    // Initiate HMAC-SHA1 operation now

    if (yk.writeChallengeBegin(YKLIB_SECOND_SLOT, YKLIB_CHAL_HMAC,
                                 chalBuf, chalLength) != YKLIB_OK) {
      TRACE(_T("yk.writeChallengeBegin() failed"));
      return;
    }
    // request's in the air, setup GUI to wait for reply
    m_pending = true;
    m_yubi_status.ShowWindow(SW_HIDE);
    m_yubi_status.SetWindowText(_T(""));
    m_yubi_timeout.ShowWindow(SW_SHOW);
    m_yubi_timeout.SetPos(15);
  }
}

void CWZSelectDB::OnYubikeyBtn()
{
  UpdateData(TRUE);
  yubiRequestHMACSha1(); // request HMAC of m_passkey
}

void CWZSelectDB::OnTimer(UINT_PTR)
{
  // If an operation is pending, check if it has completed

  if (m_pending) {
    yubiCheckCompleted();
  } else {
    // No HMAC operation is pending - check if one and only one key is present
    bool inserted = IsYubiInserted();
    // call relevant callback if something's changed
    if (inserted != m_present) {
      m_present = inserted;
      if (m_present)
        yubiInserted();
      else
        yubiRemoved();
    }
  }
}
