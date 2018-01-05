/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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

#include "Fonts.h"
#include "SecString.h"
#include "GeneralMsgBox.h"
#include "PWFileDialog.h"
#include "PKBaseDlg.h"
#include "VirtualKeyboard/VKeyBoardDlg.h"

#include "core/core.h" // for IDSC_UNKNOWN_ERROR
#include "core/Util.h"
#include "core/PWSdirs.h"
#include "os/env.h"
#include "os/file.h"
#include "os/dir.h"
#include "os/windows/yubi/YkLib.h"

#include "resource3.h"

#include <sstream>
#include <iomanip>
#include <shlobj.h>

using namespace std;

const wchar_t CWZSelectDB::PSSWDCHAR = L'*';

IMPLEMENT_DYNAMIC(CWZSelectDB, CWZPropertyPage)

CWZSelectDB::CWZSelectDB(CWnd *pParent, int idd, UINT nIDCaption,
       const int nType)
 : CWZPropertyPage(idd, nIDCaption, nType), m_tries(0), m_state(0),
  m_pVKeyBoardDlg(NULL), m_bAdvanced(BST_UNCHECKED), m_bExportDBFilters(BST_UNCHECKED),
  m_bFileExistsUserAsked(false),
  m_filespec(L""), m_passkey(L""), m_passkey2(L""), m_verify2(L""),
  m_defexpdelim(L"\xbb"), m_pctlDB(new CEditExtn),
  m_pctlPasskey(new CSecEditExtn), m_pctlPasskey2(new CSecEditExtn), m_pctlVerify2(new CSecEditExtn),
  m_LastFocus(IDC_PASSKEY)
{
  m_pWZPSH = (CWZPropertySheet *)pParent;
  if (pws_os::getenv("PWS_PW_MODE", false) == L"NORMAL") {
    m_pctlPasskey->SetSecure(false);
    m_pctlPasskey2->SetSecure(false);
    m_pctlVerify2->SetSecure(false);
  }
  m_present = !IsYubiInserted(); // lie to trigger correct actions in timer event
}

CWZSelectDB::~CWZSelectDB()
{
  delete m_pctlDB;
  delete m_pctlPasskey;
  delete m_pctlPasskey2;
  delete m_pctlVerify2;

  if (m_pVKeyBoardDlg != NULL && m_pVKeyBoardDlg->SaveKLID()) {
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

  const UINT nID = m_pWZPSH->GetID();

  //{{AFX_DATA_MAP(CWZSelectDB)
  DDX_Text(pDX, IDC_DATABASE, m_filespec);

  DDX_Control(pDX, IDC_DATABASE, *m_pctlDB);
  DDX_Check(pDX, IDC_ADVANCED, m_bAdvanced);

  if (nID != ID_MENUITEM_COMPARE && 
      nID != ID_MENUITEM_MERGE   && 
      nID != ID_MENUITEM_SYNCHRONIZE) {
    DDX_Check(pDX, IDC_EXPORTFILTERS, m_bExportDBFilters);
  }

  // Can't use DDX_Text for CSecEditExtn
  m_pctlPasskey->DoDDX(pDX, m_passkey);
  DDX_Control(pDX, IDC_PASSKEY, *m_pctlPasskey);
  m_pctlPasskey2->DoDDX(pDX, m_passkey2);
  DDX_Control(pDX, IDC_PASSKEY2, *m_pctlPasskey2);
  m_pctlVerify2->DoDDX(pDX, m_verify2);
  DDX_Control(pDX, IDC_VERIFY2, *m_pctlVerify2);

  DDX_Control(pDX, IDC_YUBI_PROGRESS, m_yubi_timeout);
  DDX_Control(pDX, IDC_YUBI_STATUS, m_yubi_status);

  if (nID == ID_MENUITEM_SYNCHRONIZE         ||
      nID == ID_MENUITEM_EXPORT2PLAINTEXT    ||
      nID == ID_MENUITEM_EXPORTENT2PLAINTEXT ||
      nID == ID_MENUITEM_EXPORTGRP2PLAINTEXT ||
      nID == ID_MENUITEM_EXPORT2XML          ||
      nID == ID_MENUITEM_EXPORTENT2XML       ||
      nID == ID_MENUITEM_EXPORTGRP2XML       ||
      nID == ID_MENUITEM_EXPORTENT2DB        ||
      nID == ID_MENUITEM_EXPORTGRP2DB        ||
      nID == ID_MENUITEM_EXPORTFILTERED2DB) {
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
  ON_WM_TIMER()
  ON_EN_CHANGE(IDC_DATABASE, OnDatabaseChange)
  ON_EN_CHANGE(IDC_PASSKEY, OnPassKeyChange)
  ON_EN_CHANGE(IDC_PASSKEY2, OnPassKey2Change)
  ON_EN_CHANGE(IDC_VERIFY2, OnVerify2Change)

  ON_EN_SETFOCUS(IDC_PASSKEY, OnPasskeySetfocus)
  ON_EN_SETFOCUS(IDC_PASSKEY2, OnPasskey2Setfocus)
  ON_EN_SETFOCUS(IDC_VERIFY2, OnVerify2keySetfocus)

  ON_BN_CLICKED(IDC_BTN_BROWSE, OnOpenFileBrowser)
  ON_STN_CLICKED(IDC_VKB, OnVirtualKeyboard)
  ON_STN_CLICKED(IDC_VKB2, OnVirtualKeyboard)
  ON_MESSAGE(PWS_MSG_INSERTBUFFER, OnInsertBuffer)

  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDC_ADVANCED, OnAdvanced)
  ON_BN_CLICKED(IDC_EXPORTFILTERS, OnExportFilters)

  ON_BN_CLICKED(IDC_YUBIKEY_BTN, OnYubikeyBtn)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CWZSelectDB::OnHelp()
{
  ShowHelp(L"::/html/wzselectdb.html");
}

BOOL CWZSelectDB::OnInitDialog()
{
  CWZPropertyPage::OnInitDialog();

  SetTimer(1, 250, 0); // Setup a timer to poll YubiKey every 250 ms

  const UINT nID = m_pWZPSH->GetID();

  Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_PASSKEY));
  Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_PASSKEY2));
  Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_VERIFY2));
  m_pctlPasskey->SetPasswordChar(PSSWDCHAR);
  m_pctlPasskey2->SetPasswordChar(PSSWDCHAR);
  m_pctlVerify2->SetPasswordChar(PSSWDCHAR);

  CString cs_text,cs_temp;

  bool bWARNINGTEXT(true), bEXPORTDBCTRLS(false);
  switch (nID) {
    case ID_MENUITEM_SYNCHRONIZE:
      cs_text.LoadString(IDS_WZSLCT_WARNING_SYNC);
      break;
    case ID_MENUITEM_EXPORT2PLAINTEXT:
    case ID_MENUITEM_EXPORT2XML:
      cs_temp.LoadString(IDS_WSSLCT_ALL);
      break;
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
    case ID_MENUITEM_EXPORTENT2XML:
    case ID_MENUITEM_EXPORTENT2DB:
      cs_temp.LoadString(IDS_WSSLCT_ENTRY);
      if (nID == ID_MENUITEM_EXPORTENT2DB) {
        bWARNINGTEXT = false;
        bEXPORTDBCTRLS = true;
      }
      break;
    case ID_MENUITEM_EXPORTGRP2PLAINTEXT:
    case ID_MENUITEM_EXPORTGRP2XML:
    case ID_MENUITEM_EXPORTGRP2DB:
      cs_temp.LoadString(IDS_WSSLCT_GROUP);
      if (nID == ID_MENUITEM_EXPORTGRP2DB) {
        bWARNINGTEXT = false;
        bEXPORTDBCTRLS = true;
      }
      break;
    case ID_MENUITEM_EXPORTFILTERED2DB:
      cs_temp.LoadString(IDS_WSSLCT_FILTERED);
      bWARNINGTEXT = false;
      bEXPORTDBCTRLS = true;
      break;
    case ID_MENUITEM_COMPARE:
    case ID_MENUITEM_MERGE:
      bWARNINGTEXT = false;
      break;
    default:
      bWARNINGTEXT = false;
      ASSERT(0);
  }

  if (bEXPORTDBCTRLS) {
    // Show & Enable Export Combination controls
    GetDlgItem(IDC_STATIC_NEWCOMBI)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_STATIC_VERIFY)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_STATIC_COMBI)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_PASSKEY2)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_PASSKEY2)->EnableWindow(TRUE);
    GetDlgItem(IDC_VERIFY2)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_VERIFY2)->EnableWindow(TRUE);
    GetDlgItem(IDC_VKB2)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_VKB2)->EnableWindow(TRUE);
  }

  if (bWARNINGTEXT) {
    if (nID != ID_MENUITEM_SYNCHRONIZE)
      cs_text.Format(IDS_WZSLCT_WARNING_EXP, static_cast<LPCWSTR>(cs_temp));

    GetDlgItem(IDC_STATIC_WZWARNING)->SetWindowText(cs_text);
    m_stc_warning.SetColour(RGB(255,0,0));
    if (nID != ID_MENUITEM_EXPORT2PLAINTEXT && nID != ID_MENUITEM_EXPORTENT2PLAINTEXT &&
        nID != ID_MENUITEM_EXPORTGRP2PLAINTEXT)
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
  std::wstring str_extn(L"");

  switch (nID) {
    case ID_MENUITEM_EXPORTENT2DB:
    case ID_MENUITEM_EXPORTGRP2DB:
    case ID_MENUITEM_EXPORTFILTERED2DB:
    {
      // Disable & hide - Advanced checkbox & specifying delimiter
      GetDlgItem(IDC_ADVANCED)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_ADVANCED)->EnableWindow(FALSE);

      GetDlgItem(IDC_STATIC_WZEXPDLM1)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_STATIC_WZEXPDLM2)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_WZDEFEXPDELIM)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_WZDEFEXPDELIM)->EnableWindow(FALSE);

      // Potentially allow user to export DB filters
      bool bEnableExportFilters = ((nID == ID_MENUITEM_EXPORTGRP2DB || nID == ID_MENUITEM_EXPORTFILTERED2DB) &&
                                   !app.GetCore()->GetDBFilters().empty());
      GetDlgItem(IDC_EXPORTFILTERS)->ShowWindow(bEnableExportFilters ? SW_SHOW : SW_HIDE);
      GetDlgItem(IDC_EXPORTFILTERS)->EnableWindow(bEnableExportFilters ? TRUE : FALSE);

      std::wstring drive, dir, file, ext;
      pws_os::splitpath(m_pWZPSH->WZPSHGetCurFile().c_str(), drive, dir, file, ext);

      std::wstring str_file = file + L".export";

      // Create new DB
      const PWSfile::VERSION current_version = m_pWZPSH->GetDBVersion();
      ExportFileName = pws_os::makepath(drive, dir, str_file, current_version == PWSfile::V30 ? L"psafe3" : L"psafe4");

      m_pctlDB->SetWindowText(ExportFileName.c_str());
      m_filespec = ExportFileName.c_str();
      uifilemsg = IDS_WZFILE;
      break;
    }
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
    case ID_MENUITEM_EXPORTGRP2XML:
      str_extn = L"xml";

      // Drop though intentionally
    case ID_MENUITEM_EXPORT2PLAINTEXT:
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
    case ID_MENUITEM_EXPORTGRP2PLAINTEXT:
      if (str_extn.empty())
        str_extn = L"txt";
      ExportFileName = PWSUtil::GetNewFileName(m_pWZPSH->WZPSHGetCurFile().c_str(),
                                               str_extn.c_str());
      m_pctlDB->SetWindowText(ExportFileName.c_str());
      m_filespec = ExportFileName.c_str();
      uifilemsg = IDS_WZFILE;

      GetDlgItem(IDC_EXPORTFILTERS)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_EXPORTFILTERS)->EnableWindow(FALSE);
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
    GetDlgItem(IDC_VKB2)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_VKB2)->EnableWindow(FALSE);
  }

  // Disable Next until fields set
  m_pWZPSH->SetWizardButtons(0);

  CString cs_tmp(MAKEINTRESOURCE(m_pWZPSH->GetButtonID()));
  m_pWZPSH->GetDlgItem(ID_WIZNEXT)->SetWindowText(cs_tmp);

  // Yubi-related initializations:
  m_yubiLogo.LoadBitmap(IDB_YUBI_LOGO);
  m_yubiLogoDisabled.LoadBitmap(IDB_YUBI_LOGO_DIS);

  // Disable passphrase until database name filled in
  m_pctlPasskey->EnableWindow(TRUE);

  CWnd *ybn = GetDlgItem(IDC_YUBIKEY_BTN);

  if (YubiExists()) {
    ybn->ShowWindow(SW_SHOW);
    m_yubi_status.ShowWindow(SW_SHOW);
  } else {
    ybn->ShowWindow(SW_HIDE);
    m_yubi_status.ShowWindow(SW_HIDE);
  }
  m_yubi_timeout.ShowWindow(SW_HIDE);
  m_yubi_timeout.SetRange(0, 15);
  bool bYubiInserted = IsYubiInserted();
  // MFC has ancient bug: can't render disabled version of bitmap,
  // so instead of showing drek, we roll our own, and leave enabled.
  ybn->EnableWindow(TRUE);

  if (bYubiInserted) {
    ((CButton*)ybn)->SetBitmap(m_yubiLogo);
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_CLICK_PROMPT)));
  } else {
    ((CButton*)ybn)->SetBitmap(m_yubiLogoDisabled);
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_INSERT_PROMPT)));
  }

  GotoDlgCtrl(GetDlgItem(IDC_DATABASE));

  return FALSE;
}

HBRUSH CWZSelectDB::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
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

void CWZSelectDB::OnPasskeySetfocus()
{
  m_LastFocus = IDC_PASSKEY;
}

void CWZSelectDB::OnPasskey2Setfocus()
{
  m_LastFocus = IDC_PASSKEY2;
}

void CWZSelectDB::OnVerify2keySetfocus()
{
  m_LastFocus = IDC_VERIFY2;
}

void CWZSelectDB::OnAdvanced()
{
  //m_bAdvanced = ((CButton*)GetDlgItem(IDC_ADVANCED))->GetCheck();
  UpdateData(TRUE);

  CString cs_tmp(MAKEINTRESOURCE(m_bAdvanced == BST_UNCHECKED ? m_pWZPSH->GetButtonID() :
                                        IDS_WZNEXT));

  m_pWZPSH->GetDlgItem(ID_WIZNEXT)->SetWindowText(cs_tmp);
}

void CWZSelectDB::OnExportFilters()
{
  m_bExportDBFilters = ((CButton*)GetDlgItem(IDC_EXPORTFILTERS))->GetCheck();
}

void CWZSelectDB::OnPassKeyChange()
{
  CSecString cs_Passkey;
  cs_Passkey =  m_pctlPasskey->GetSecureText();

  if (!cs_Passkey.IsEmpty())
    m_state |= KEYPRESENT;
  else
    m_state &= ~KEYPRESENT;

  m_passkey = cs_Passkey;

  const UINT nID = m_pWZPSH->GetID();
  bool bReady;

  if (nID == ID_MENUITEM_EXPORTENT2DB || nID == ID_MENUITEM_EXPORTGRP2DB ||
      nID == ID_MENUITEM_EXPORTFILTERED2DB) {
    bReady = m_state == ALLPRESENT;
  } else {
    bReady = m_state == BOTHPRESENT;
  }

  m_pWZPSH->SetWizardButtons(bReady ? PSWIZB_NEXT : 0);
}

void CWZSelectDB::OnPassKey2Change()
{
  CSecString cs_Passkey2, cs_Verify2;
  cs_Passkey2 = m_pctlPasskey2->GetSecureText();
  cs_Verify2 = m_pctlVerify2->GetSecureText();

  if (!cs_Passkey2.IsEmpty())
    m_state |= KEY2PRESENT;
  else
    m_state &= ~KEY2PRESENT;

  if (!cs_Verify2.IsEmpty() && cs_Passkey2 == cs_Verify2)
    m_state |= KEY2_EQ_VERIFY2;
  else
    m_state &= ~KEY2_EQ_VERIFY2;

  m_passkey2 = cs_Passkey2;

  m_pWZPSH->SetWizardButtons(m_state == ALLPRESENT ? PSWIZB_NEXT : 0);
}

void CWZSelectDB::OnVerify2Change()
{
  CSecString cs_Passkey2, cs_Verify2;
  cs_Passkey2 = m_pctlPasskey2->GetSecureText();
  cs_Verify2 = m_pctlVerify2->GetSecureText();

  if (!cs_Verify2.IsEmpty())
    m_state |= VERIFY2PRESENT;
  else
    m_state &= ~VERIFY2PRESENT;

  if (!cs_Passkey2.IsEmpty() && cs_Passkey2 == cs_Verify2)
    m_state |= KEY2_EQ_VERIFY2;
  else
    m_state &= ~KEY2_EQ_VERIFY2;

  m_verify2 = cs_Verify2;

  m_pWZPSH->SetWizardButtons(m_state == ALLPRESENT ? PSWIZB_NEXT : 0);
}

void CWZSelectDB::OnDatabaseChange()
{
  CString cs_DB;
  m_pctlDB->GetWindowText(cs_DB);

  if (!cs_DB.IsEmpty())
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

  // Current DB passkey
  if (m_passkey.IsEmpty()) {
    gmb.AfxMessageBox(IDS_CANNOTBEBLANK);
    m_pctlPasskey->SetFocus();
    return -1;
  }

  if (nID == ID_MENUITEM_EXPORTENT2DB || nID == ID_MENUITEM_EXPORTGRP2DB ||
      nID == ID_MENUITEM_EXPORTFILTERED2DB) {
    // New Export DB passkey
    if (m_passkey2 != m_verify2) {
      // This shouldn't happen as the Wizard Next button shouldn't be
      // active unless they are equal!
      gmb.AfxMessageBox(IDS_ENTRIESDONOTMATCH);
      ((CEdit *)GetDlgItem(IDC_VERIFY2))->SetFocus();
      return -1;
    }

    if (m_passkey2.IsEmpty()) {
      gmb.AfxMessageBox(IDS_ENTERKEYANDVERIFY);
      ((CEdit *)GetDlgItem(IDC_PASSKEY2))->SetFocus();
      return -1;
    }
  }

  bool bFileExists = pws_os::FileExists(m_filespec.GetString());
  int iExportType(-1);  // -1 = Not set, IDS_WZEXPORTTEXT = Text, IDS_WZEXPORTXML = XML, IDS_WZEXPORTDB = DB

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
    case ID_MENUITEM_EXPORTGRP2PLAINTEXT:
      iExportType = IDS_WZEXPORTTEXT; // Fall through on purpose
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
    case ID_MENUITEM_EXPORTGRP2XML:
      if (iExportType < 0)
        iExportType = IDS_WZEXPORTXML;
    case ID_MENUITEM_EXPORTENT2DB:
    case ID_MENUITEM_EXPORTGRP2DB:
    case ID_MENUITEM_EXPORTFILTERED2DB:
      if (iExportType < 0)
        iExportType = IDS_WZEXPORTDB;
      if (bFileExists) {
        // Check if OK to overwrite existing file - if not already asked by user clicking
        // file browser button
        if (!m_bFileExistsUserAsked) {
          // As possibility to drop through after using gmb, need our own
          // in case another message is required - CGeneralMsgBox is NOT reusable
          CGeneralMsgBox gmbx;
          CString cs_msg, cs_title(MAKEINTRESOURCE(iExportType));
          cs_msg.Format(IDS_REPLACEEXPORTFILE, static_cast<LPCWSTR>(m_filespec));

          INT_PTR rc = gmbx.AfxMessageBox(cs_msg, cs_title,
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
        //  If root directory - nothing to create
        if (cdir == L"\\")
          break;

        // Try and create necessary directories
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

  // Check that this file isn't already open
  const StringX sx_Filename1(m_pWZPSH->WZPSHGetCurFile());
  const StringX sx_Filename2 = m_filespec.GetString();
  const StringX sx_passkey = (LPCWSTR)m_passkey;
  const StringX sx_exportpasskey = (LPCWSTR)m_passkey2;
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
  m_pWZPSH->SetExportPassKey(sx_exportpasskey);
  m_pWZPSH->SetOtherDB(sx_Filename2);
  m_pWZPSH->SetDelimiter(m_defexpdelim.GetAt(0));
  m_pWZPSH->SetAdvanced(m_bAdvanced == BST_CHECKED);
  m_pWZPSH->SetExportDBFilters(m_bExportDBFilters == BST_CHECKED);

  return m_bAdvanced == BST_CHECKED ? 0 : IDD_WZFINISH;
}

bool CWZSelectDB::ProcessPhrase(const StringX &filename, const StringX &passkey)
{
  PWSAuxCore tmpcore; // doesn't really need to be PWSAuxCore, since we only check the passkey
  if (m_pWZPSH->WZPSHCheckPasskey(filename, passkey,
                                  &tmpcore) == PWScore::SUCCESS) {
    m_tries = 0;
    return true;
  } else {
    CGeneralMsgBox gmb;
    if (m_tries++ >= 2) {
      CString cs_toomany;
      cs_toomany.Format(IDS_TOOMANYTRIES, m_tries);
      gmb.AfxMessageBox(cs_toomany);
    } else {
      gmb.AfxMessageBox(IDS_INCORRECTKEY);
    }
    m_pctlPasskey->SetSel(MAKEWORD(-1, 0));
    m_pctlPasskey->SetFocus();
    return false;
  }
}

void CWZSelectDB::OnOpenFileBrowser()
{
  //Open-type dialog box
  CString cs_suffix, cs_filter;
  DWORD dwflags(0);
  UINT uimsgid(IDS_CHOOSEDATABASE);

  const UINT nID = m_pWZPSH->GetID();
  BOOL bTYPE_OPEN(FALSE); // TRUE = Open, FALSE = Save
  const PWSfile::VERSION current_version = m_pWZPSH->GetDBVersion();

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
    case ID_MENUITEM_EXPORTGRP2PLAINTEXT:
      cs_suffix = L"txt";
      cs_filter.LoadString(IDS_FDF_T_C_ALL);
      dwflags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                            OFN_LONGNAMES | OFN_OVERWRITEPROMPT;
      uimsgid = IDS_NAMETEXTFILE;
      break;
    case ID_MENUITEM_EXPORT2XML:
    case ID_MENUITEM_EXPORTENT2XML:
    case ID_MENUITEM_EXPORTGRP2XML:
      cs_suffix = L"xml";
      cs_filter.LoadString(IDS_FDF_X_ALL);
      dwflags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                            OFN_LONGNAMES | OFN_OVERWRITEPROMPT;
      uimsgid = IDS_NAMEXMLFILE;
      break;
    case ID_MENUITEM_EXPORTENT2DB:
    case ID_MENUITEM_EXPORTGRP2DB:
      cs_suffix = L"export";
      cs_filter.LoadString(current_version == PWSfile::V30 ? IDS_FDF_V3_ALL : IDS_FDF_V4_ALL);
      dwflags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
        OFN_LONGNAMES | OFN_OVERWRITEPROMPT;
      uimsgid = IDS_NAMEXMLFILE;
      break;
    default:
      ASSERT(0);
  }

  CString cs_text(MAKEINTRESOURCE(uimsgid));

  std::wstring ExportFileName = PWSUtil::GetNewFileName(m_pWZPSH->WZPSHGetCurFile().c_str(),
                                                        CString::PCXSTR(cs_suffix));
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
    m_pVKeyBoardDlg->SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
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
  m_pVKeyBoardDlg->SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

  return;
}

LRESULT CWZSelectDB::OnInsertBuffer(WPARAM, LPARAM)
{
  // Update the variables
  UpdateData(TRUE);

  // Get the buffer
  CSecString vkbuffer = m_pVKeyBoardDlg->GetPassphrase();

  CSecEditExtn *m_pSecCtl(NULL);
  CSecString *m_pSecString;

  switch (m_LastFocus) {
    case IDC_PASSKEY:
      m_pSecCtl = m_pctlPasskey;
      m_pSecString = &m_passkey;
      break;
    case IDC_PASSKEY2:
      m_pSecCtl = m_pctlPasskey2;
      m_pSecString = &m_passkey2;
      break;
    case IDC_VERIFY2:
      m_pSecCtl = m_pctlVerify2;
      m_pSecString = &m_verify2;
      break;
    default:
      // Error!
      ASSERT(0);
      return 0L;
  }

  // Find the selected characters - if any
  int nStartChar, nEndChar;
  m_pSecCtl->GetSel(nStartChar, nEndChar);

  // If any characters selected - delete them
  if (nStartChar != nEndChar)
    m_pSecString->Delete(nStartChar, nEndChar - nStartChar);

  // Insert the buffer
  m_pSecString->Insert(nStartChar, vkbuffer);

  // Put cursor at end of inserted text
  m_pSecCtl->SetSel(nStartChar + vkbuffer.GetLength(),
                    nStartChar + vkbuffer.GetLength());

  // Update the dialog
  UpdateData(FALSE);

  // Ensure flags set so that buttons are activated as required
  if (!vkbuffer.IsEmpty()) {
    switch (m_LastFocus) {
      case IDC_PASSKEY:
        OnPassKeyChange();
        break;
      case IDC_PASSKEY2:
        OnPassKey2Change();
        break;
      case IDC_VERIFY2:
        OnVerify2Change();
        break;
      default:
        // Error!
        ASSERT(0);
        return 0L;
    }
  }

  // Make us on top
  SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

  return 0L;
}

void CWZSelectDB::yubiInserted(void)
{
  CButton *ybn = (CButton *)GetDlgItem(IDC_YUBIKEY_BTN);
  ybn->SetBitmap(m_yubiLogo);
  m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_CLICK_PROMPT)));
  // In case this is the first time:
  ybn->ShowWindow(SW_SHOW);
  m_yubi_status.ShowWindow(SW_SHOW);
}

void CWZSelectDB::yubiRemoved(void)
{
  ((CButton*)GetDlgItem(IDC_YUBIKEY_BTN))->SetBitmap(m_yubiLogoDisabled);
  m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_INSERT_PROMPT)));
}

void CWZSelectDB::yubiShowChallengeSent()
{
  // A request's in the air, setup GUI to wait for reply
  m_yubi_status.ShowWindow(SW_HIDE);
  m_yubi_status.SetWindowText(L"");
  m_yubi_timeout.ShowWindow(SW_SHOW);
  m_yubi_timeout.SetPos(15);
}

void CWZSelectDB::yubiProcessCompleted(YKLIB_RC yrc, unsigned short ts, const BYTE *respBuf)
{
  switch (yrc) {
  case YKLIB_OK:
    m_yubi_status.ShowWindow(SW_SHOW);
    m_yubi_timeout.ShowWindow(SW_HIDE);
    m_yubi_timeout.SetPos(0);
    m_yubi_status.SetWindowText(L"");
    pws_os::Trace(L"yubiCheckCompleted: YKLIB_OK");
    m_pending = false;
    m_passkey = Bin2Hex(respBuf, SHA1_DIGEST_SIZE);
    m_state |= KEYPRESENT;
    // The returned hash is the passkey
    m_pWZPSH->SetWizardButtons(PSWIZB_NEXT); // enable
    // This will check the password, etc.:
    UpdateData(FALSE); // passwd -> control
    m_pWZPSH->PostMessage(WM_COMMAND, MAKELONG(ID_WIZNEXT, BN_CLICKED), 0);
    break;
  case YKLIB_PROCESSING:  // Still processing or waiting for the result
    break;
  case YKLIB_TIMER_WAIT:  // A given number of seconds remain
    m_yubi_timeout.SetPos(ts);
    break;

  case YKLIB_INVALID_RESPONSE:  // Invalid or no response
    m_state &= ~KEYPRESENT;
    m_pending = false;
    m_yubi_timeout.ShowWindow(SW_HIDE);
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDS_YUBI_TIMEOUT)));
    m_yubi_status.ShowWindow(SW_SHOW);
    break;

  default:                // A non-recoverable error has occured
    m_state &= ~KEYPRESENT;
    m_pending = false;
    m_yubi_timeout.ShowWindow(SW_HIDE);
    m_yubi_status.ShowWindow(SW_SHOW);
    // Generic error message
    pws_os::Trace(L"yubiCompleted(%d)\n", yrc);
    m_yubi_status.SetWindowText(CString(MAKEINTRESOURCE(IDSC_UNKNOWN_ERROR)));
    break;
  }
}

void CWZSelectDB::OnYubikeyBtn()
{
  UpdateData(TRUE);
  yubiRequestHMACSha1(m_passkey);
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
      if (m_present) {
        CPKBaseDlg::SetYubiExists();
        yubiInserted();
      } else
        yubiRemoved();
    }
  }
}
