/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsMisc.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "ThisMfcApp.h"    // For Help

#include "Options_PropertySheet.h"
#include "Options_PropertyPage.h"

#include "GeneralMsgBox.h"
#include "PWFileDialog.h"

#include "core/core.h"
#include "core/PwsPlatform.h"
#include "core/PWSprefs.h" // for DoubleClickAction enums

#include "os/dir.h"

#include "resource.h"
#include "resource3.h"  // String resources

#include "OptionsMisc.h" // Must be after resource.h

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsMisc property page

IMPLEMENT_DYNAMIC(COptionsMisc, COptions_PropertyPage)

COptionsMisc::COptionsMisc(CWnd *pParent, st_Opt_master_data *pOPTMD)
  : COptions_PropertyPage(pParent,
                          COptionsMisc::IDD, COptionsMisc::IDD_SHORT,
                          pOPTMD)
{
  m_DefUsername = (CString)M_DefUsername();
  m_OtherBrowserLocation = M_OtherBrowserLocation();
  m_OtherEditorLocation = M_OtherEditorLocation();
  m_OtherBrowserCmdLineParms = M_OtherBrowserCmdLineParms();
  m_OtherEditorCmdLineParms = M_OtherEditorCmdLineParms();
  m_AutotypeText = M_AutotypeText();
  m_AutotypeDelay = M_AutotypeDelay();
  m_ConfirmDelete = M_ConfirmDelete();
  m_MaintainDatetimeStamps = M_MaintainDatetimeStamps();
  m_EscExits = M_EscExits();
  m_UseDefUsername = M_UseDefUsername();
  m_QuerySetDefUsername = M_QuerySetDefUsername();
  m_AutotypeMinimize = M_AutotypeMinimize();
  m_DoubleClickAction = M_DoubleClickAction();
  m_ShiftDoubleClickAction = M_ShiftDoubleClickAction();
  
  if (m_DoubleClickAction < PWSprefs::minDCA ||
      m_DoubleClickAction > PWSprefs::maxDCA)
    m_DoubleClickAction = M_DoubleClickAction() = PWSprefs::DoubleClickCopyPassword;

  if (m_ShiftDoubleClickAction < PWSprefs::minDCA ||
      m_ShiftDoubleClickAction > PWSprefs::maxDCA)
    m_ShiftDoubleClickAction = M_ShiftDoubleClickAction() = PWSprefs::DoubleClickCopyPassword;
}

void COptionsMisc::DoDataExchange(CDataExchange* pDX)
{
  COptions_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsMisc)
  DDX_Check(pDX, IDC_MAINTAINDATETIMESTAMPS, m_MaintainDatetimeStamps);
  DDX_Check(pDX, IDC_USEDEFUSER, m_UseDefUsername);
  DDX_Text(pDX, IDC_DEFUSERNAME, m_DefUsername);
  DDX_Text(pDX, IDC_DB_DEF_AUTOTYPE_TEXT, m_AutotypeText);
  DDX_Text(pDX, IDC_DB_DEF_AUTOTYPE_DELAY, m_AutotypeDelay);
  DDX_Check(pDX, IDC_CONFIRMDELETE, m_ConfirmDelete);
  DDX_Check(pDX, IDC_ESC_EXITS, m_EscExits);
  DDX_Control(pDX, IDC_DOUBLE_CLICK_ACTION, m_dblclk_cbox);
  DDX_Control(pDX, IDC_SHIFT_DOUBLE_CLICK_ACTION, m_shiftdblclk_cbox);
  DDX_Check(pDX, IDC_QUERYSETDEF, m_QuerySetDefUsername);

  DDX_Text(pDX, IDC_OTHERBROWSERLOCATION, m_OtherBrowserLocation);
  DDX_Text(pDX, IDC_OTHEREDITORLOCATION, m_OtherEditorLocation);
  DDX_Text(pDX, IDC_ALTBROWSER_CMDLINE, m_OtherBrowserCmdLineParms);
  DDX_Text(pDX, IDC_ALTEDITOR_CMDLINE, m_OtherEditorCmdLineParms);

  DDX_Check(pDX, IDC_MINIMIZEONAUTOTYPE, m_AutotypeMinimize);

  DDX_Control(pDX, IDC_MAINTAINDATETIMESTAMPS, m_chkbox[0]);
  DDX_Control(pDX, IDC_USEDEFUSER, m_chkbox[1]);

  DDX_Control(pDX, IDC_MAINTAINDATETIMESTAMPSHELP, m_Help1);
  DDX_Control(pDX, IDC_OTHERBROWSERLOCATIONHELP, m_Help2);
  DDX_Control(pDX, IDC_OTHEREDITORLOCATIONHELP, m_Help3);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsMisc, COptions_PropertyPage)
  //{{AFX_MSG_MAP(COptionsMisc)
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_USEDEFUSER, OnUseDefUser)
  ON_COMMAND_RANGE(IDC_BROWSEFORLOCATION_BROWSER, 
                   IDC_BROWSEFORLOCATION_EDITOR, OnBrowseForLocation)
  ON_CBN_SELCHANGE(IDC_DOUBLE_CLICK_ACTION, OnDCAComboChanged)
  ON_CBN_SELCHANGE(IDC_SHIFT_DOUBLE_CLICK_ACTION, OnShiftDCAComboChanged)
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL COptionsMisc::OnInitDialog() 
{
  COptions_PropertyPage::OnInitDialog();

  for (int i = 0; i < 2; i++) {
    m_chkbox[i].SetTextColour(CR_DATABASE_OPTIONS);
    m_chkbox[i].ResetBkgColour(); // Use current window's background
  }

  // Database preferences - can't change in R/O mode of if no DB is open
  if (!GetMainDlg()->IsDBOpen() || GetMainDlg()->IsDBReadOnly()) {
    GetDlgItem(IDC_DEFUSERNAME)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_USERNAME)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_DEFAUTOTYPE)->EnableWindow(FALSE);
    GetDlgItem(IDC_DB_DEF_AUTOTYPE_TEXT)->EnableWindow(FALSE);
    GetDlgItem(IDC_MAINTAINDATETIMESTAMPS)->EnableWindow(FALSE);
    GetDlgItem(IDC_USEDEFUSER)->EnableWindow(FALSE);
  }

  OnUseDefUser();

  // For some reason, MFC calls us twice when initializing.
  // Populate the combo box only once.
  if (m_dblclk_cbox.GetCount() == 0) {
    SetupCombo(&m_dblclk_cbox);
  }

  if (m_shiftdblclk_cbox.GetCount() == 0) {
    SetupCombo(&m_shiftdblclk_cbox);
  }

  m_dblclk_cbox.SetCurSel(m_DCA_to_Index[m_DoubleClickAction]);
  m_shiftdblclk_cbox.SetCurSel(m_DCA_to_Index[m_ShiftDoubleClickAction]);

  GetDlgItem(IDC_OTHERBROWSERLOCATION)->SetWindowText(m_OtherBrowserLocation);
  GetDlgItem(IDC_OTHEREDITORLOCATION)->SetWindowText(m_OtherEditorLocation);

  CSpinButtonCtrl *pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_DADSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_DB_DEF_AUTOTYPE_DELAY));
  pspin->SetRange32(M_prefminAutotypeDelay(), M_prefmaxAutotypeDelay());
  pspin->SetBase(10);
  pspin->SetPos(m_AutotypeDelay);

  if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
    m_Help1.Init(IDB_QUESTIONMARK);
    m_Help2.Init(IDB_QUESTIONMARK);
    m_Help3.Init(IDB_QUESTIONMARK);

    // Note naming convention: string IDS_xxx corresponds to control IDC_xxx_HELP
    AddTool(IDC_MAINTAINDATETIMESTAMPSHELP, IDS_MAINTAINDATETIMESTAMPS);
    AddTool(IDC_OTHERBROWSERLOCATIONHELP, IDS_OTHERBROWSERLOCATION);
    AddTool(IDC_OTHEREDITORLOCATIONHELP, IDS_OTHEREDITORLOCATION);
    ActivateToolTip();
  } else {
    m_Help1.EnableWindow(FALSE);
    m_Help1.ShowWindow(SW_HIDE);
    m_Help2.EnableWindow(FALSE);
    m_Help2.ShowWindow(SW_HIDE);
    m_Help3.EnableWindow(FALSE);
    m_Help3.ShowWindow(SW_HIDE);
  }

  return TRUE;  // return TRUE unless you set the focus to a control
}

LRESULT COptionsMisc::OnQuerySiblings(WPARAM wParam, LPARAM lParam)
{
  UpdateData(TRUE);

  // Security asked for DoubleClickAction value
  switch (wParam) {
    case PPOPT_GET_DCA:
      {
      int * pDCA = (int *)lParam;
      ASSERT(pDCA != NULL);
      *pDCA = (int)m_DoubleClickAction;
      return 1L;
      }
    case PP_DATA_CHANGED:
      if (M_ConfirmDelete()            != m_ConfirmDelete            || 
          M_MaintainDatetimeStamps()   != m_MaintainDatetimeStamps   ||
          M_EscExits()                 != m_EscExits                 ||
          M_UseDefUsername()           != m_UseDefUsername           ||
          (M_UseDefUsername()          == TRUE &&
           M_DefUsername()             != CSecString(m_DefUsername)) ||
          M_QuerySetDefUsername()      != m_QuerySetDefUsername      ||
          M_DoubleClickAction()        != m_DoubleClickAction        ||
          M_ShiftDoubleClickAction()   != m_ShiftDoubleClickAction   ||
          M_OtherBrowserLocation()     != m_OtherBrowserLocation     ||
          M_OtherEditorLocation()      != m_OtherEditorLocation      ||
          M_OtherBrowserCmdLineParms() != m_OtherBrowserCmdLineParms ||
          M_OtherEditorCmdLineParms()  != m_OtherEditorCmdLineParms  ||
          M_AutotypeText()             != m_AutotypeText             ||
          M_AutotypeDelay()            != m_AutotypeDelay            ||
          M_AutotypeMinimize()         != m_AutotypeMinimize)
        return 1L;
      break;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselves here first
      if (OnApply() == FALSE)
        return 1L;
    default:
      break;
  }
  return 0L;
}

BOOL COptionsMisc::OnApply() 
{
  UpdateData(TRUE);

  // Go ask Security for ClearClipboardOnMinimize value
  BOOL bClearClipboardOnMinimize;
  if (QuerySiblings(PPOPT_GET_CCOM, (LPARAM)&bClearClipboardOnMinimize) == 0L) {
    // Security not loaded - get from Prefs
    bClearClipboardOnMinimize = 
        PWSprefs::GetInstance()->GetPref(PWSprefs::ClearClipboardOnMinimize);
  }

  if (m_DoubleClickAction == PWSprefs::DoubleClickCopyPasswordMinimize &&
      bClearClipboardOnMinimize) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_MINIMIZECONFLICT);

    // Are we the current page, if not activate this page
    COptions_PropertySheet *pPS = (COptions_PropertySheet *)GetParent();
    if (pPS->GetActivePage() != (COptions_PropertyPage *)this)
      pPS->SetActivePage(this);

    m_dblclk_cbox.SetFocus();
    return FALSE;
  }

  M_DefUsername() = (CSecString)m_DefUsername;
  M_OtherBrowserLocation() = m_OtherBrowserLocation;
  M_OtherEditorLocation() = m_OtherEditorLocation;
  M_OtherBrowserCmdLineParms() = m_OtherBrowserCmdLineParms;
  M_OtherEditorCmdLineParms() = m_OtherEditorCmdLineParms;
  M_AutotypeText() = m_AutotypeText;
  M_AutotypeDelay() = m_AutotypeDelay;
  M_ConfirmDelete() = m_ConfirmDelete;
  M_MaintainDatetimeStamps() = m_MaintainDatetimeStamps;
  M_EscExits() = m_EscExits;
  M_UseDefUsername() = m_UseDefUsername;
  M_QuerySetDefUsername() = m_QuerySetDefUsername;
  M_AutotypeMinimize() = m_AutotypeMinimize;
  M_DoubleClickAction() = m_DoubleClickAction;
  M_ShiftDoubleClickAction() = m_ShiftDoubleClickAction;

  return COptions_PropertyPage::OnApply();
}

BOOL COptionsMisc::PreTranslateMessage(MSG *pMsg)
{
  RelayToolTipEvent(pMsg);

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return COptions_PropertyPage::PreTranslateMessage(pMsg);
}

void COptionsMisc::OnHelp()
{
  ShowHelp(L"::/html/misc_tab.html");
}

/////////////////////////////////////////////////////////////////////////////
// COptionsMisc message handlers

void COptionsMisc::SetupCombo(CComboBox *pcbox)
{
  if (pcbox == NULL)
    return;

  // ComboBox now sorted - no need to add in English alphabetical order
  int nIndex;
  CString cs_text;

  cs_text.LoadString(IDSC_DCAAUTOTYPE);
  nIndex = pcbox->AddString(cs_text);
  pcbox->SetItemData(nIndex, PWSprefs::DoubleClickAutoType);

  cs_text.LoadString(IDSC_DCABROWSE);
  nIndex = pcbox->AddString(cs_text);
  pcbox->SetItemData(nIndex, PWSprefs::DoubleClickBrowse);

  cs_text.LoadString(IDSC_DCABROWSEPLUS);
  nIndex = pcbox->AddString(cs_text);
  pcbox->SetItemData(nIndex, PWSprefs::DoubleClickBrowsePlus);

  cs_text.LoadString(IDSC_DCACOPYNOTES);
  nIndex = pcbox->AddString(cs_text);
  pcbox->SetItemData(nIndex, PWSprefs::DoubleClickCopyNotes);

  cs_text.LoadString(IDSC_DCACOPYPASSWORD);
  nIndex = pcbox->AddString(cs_text);
  pcbox->SetItemData(nIndex, PWSprefs::DoubleClickCopyPassword);

  cs_text.LoadString(IDSC_DCACOPYPASSWORDMIN);
  nIndex = pcbox->AddString(cs_text);
  pcbox->SetItemData(nIndex, PWSprefs::DoubleClickCopyPasswordMinimize);

  cs_text.LoadString(IDSC_DCACOPYUSERNAME);
  nIndex = pcbox->AddString(cs_text);
  pcbox->SetItemData(nIndex, PWSprefs::DoubleClickCopyUsername);

  cs_text.LoadString(IDSC_DCAVIEWEDIT);
  nIndex = pcbox->AddString(cs_text);
  pcbox->SetItemData(nIndex, PWSprefs::DoubleClickViewEdit);
 
  cs_text.LoadString(IDSC_DCARUN);
  nIndex = pcbox->AddString(cs_text);
  pcbox->SetItemData(nIndex, PWSprefs::DoubleClickRun);

  cs_text.LoadString(IDSC_DCASENDEMAIL);
  nIndex = pcbox->AddString(cs_text);
  pcbox->SetItemData(nIndex, PWSprefs::DoubleClickSendEmail);

  for (int i = 0; i < pcbox->GetCount(); i++) {
    int ival = (int)pcbox->GetItemData(i);
    m_DCA_to_Index[ival] = i;
  }
}

void COptionsMisc::OnDCAComboChanged()
{
  int nIndex = m_dblclk_cbox.GetCurSel();
  m_DoubleClickAction = (int)m_dblclk_cbox.GetItemData(nIndex);
}

void COptionsMisc::OnShiftDCAComboChanged()
{
  int nIndex = m_shiftdblclk_cbox.GetCurSel();
  m_ShiftDoubleClickAction = (int)m_shiftdblclk_cbox.GetItemData(nIndex);
}

void COptionsMisc::OnUseDefUser()
{
  if (((CButton*)GetDlgItem(IDC_USEDEFUSER))->GetCheck() == 1) {
    GetDlgItem(IDC_DEFUSERNAME)->EnableWindow(TRUE);
    GetDlgItem(IDC_STATIC_USERNAME)->EnableWindow(TRUE);
    GetDlgItem(IDC_QUERYSETDEF)->EnableWindow(FALSE);
  } else {
    GetDlgItem(IDC_DEFUSERNAME)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_USERNAME)->EnableWindow(FALSE);
    GetDlgItem(IDC_QUERYSETDEF)->EnableWindow(TRUE);
  }
}

void COptionsMisc::OnBrowseForLocation(UINT nID)
{
  CString cs_initiallocation, cs_title;
  std::wstring path;
  INT_PTR rc;

  if (nID == IDC_BROWSEFORLOCATION_BROWSER)
    path = m_OtherBrowserLocation;
  else
    path = m_OtherEditorLocation;
 
  if (path.empty())
    cs_initiallocation = L"C:\\";
  else {
    std::wstring drive, dir, name, ext;
    pws_os::splitpath(path, drive, dir, name, ext);
    path = pws_os::makepath(drive, dir, L"", L"");
    cs_initiallocation = path.c_str();
  }

  CPWFileDialog fd(TRUE, NULL, NULL,
                   OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_DONTADDTORECENT | 
                      OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
                   CString(MAKEINTRESOURCE(IDS_FDF_PR_ALL)),
                   this);

  cs_title.LoadString(IDS_SELECTBROWSER);
  fd.m_ofn.lpstrTitle = cs_title;
  fd.m_ofn.lpstrInitialDir = cs_initiallocation;

  rc = fd.DoModal();

  if (rc == IDOK) {
    if (nID == IDC_BROWSEFORLOCATION_BROWSER)
      m_OtherBrowserLocation = fd.GetPathName();
    else
      m_OtherEditorLocation = fd.GetPathName();

    GetDlgItem(nID == IDC_BROWSEFORLOCATION_BROWSER ? IDC_OTHERBROWSERLOCATION :
                        IDC_OTHEREDITORLOCATION)->SetWindowText(fd.GetPathName());
  }
}

HBRUSH COptionsMisc::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CPWPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

  // Database preferences - associated static text
  switch (pWnd->GetDlgCtrlID()) {
    case IDC_DEFUSERNAME:
    case IDC_STATIC_USERNAME:
    case IDC_STATIC_DEFAUTOTYPE:
      pDC->SetTextColor(CR_DATABASE_OPTIONS);
      pDC->SetBkMode(TRANSPARENT);
      break;
    case IDC_MAINTAINDATETIMESTAMPS:
    case IDC_USEDEFUSER:
      pDC->SetTextColor(CR_DATABASE_OPTIONS);
      pDC->SetBkMode(TRANSPARENT);
      break;
  }

  return hbr;
}
