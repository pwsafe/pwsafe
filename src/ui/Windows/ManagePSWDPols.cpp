/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// CManagePSWDPols.cpp : implementation file
//

#include "stdafx.h"
#include "DboxMain.h"

#include "ManagePSWDPols.h"
#include "PasswordPolicyDlg.h"
#include "PWPListEntries.h"

#include "GeneralMsgBox.h"
#include "Fonts.h"

#include "core/core.h"

#include "resource3.h"  // String resources

#include <algorithm>

using namespace std;

// CManagePSWDPols dialog
CManagePSWDPols::CManagePSWDPols(CWnd* pParent, const bool bLongPPs)
  : CPWDialog(CManagePSWDPols::IDD, pParent),
  m_iSelectedItem(-1), m_bChanged(false), m_iSortEntriesIndex(0),
  m_bSortEntriesAscending(true), m_iSortNamesIndex(0), m_bSortNamesAscending(true),
  m_bViewPolicy(true), m_bLongPPs(bLongPPs), m_iundo_pos(-1), m_pCopyBtn(NULL),
  m_bCopyPasswordEnabled(false), m_bImageLoaded(FALSE), m_bDisabledImageLoaded(FALSE)
{
  ASSERT(pParent != NULL);

  m_bReadOnly = GetMainDlg()->IsDBReadOnly();
  
  m_bUndoShortcut = GetMainDlg()->GetShortCut(ID_MENUITEM_UNDO, m_siUndoVirtKey, m_cUndoModifier);
  m_bRedoShortcut = GetMainDlg()->GetShortCut(ID_MENUITEM_REDO, m_siRedoVirtKey, m_cRedoModifier);

  m_MapPSWDPLC = GetMainDlg()->GetPasswordPolicies();

  m_st_default_pp = PWSprefs::GetInstance()->GetDefaultPolicy();
}

CManagePSWDPols::~CManagePSWDPols()
{
  if (m_bImageLoaded)
    m_CopyPswdBitmap.Detach();

  if (m_bDisabledImageLoaded)
    m_DisabledCopyPswdBitmap.Detach();
}

void CManagePSWDPols::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_POLICYLIST, m_PolicyNames);
  DDX_Control(pDX, IDC_POLICYPROPERTIES, m_PolicyDetails);
  DDX_Control(pDX, IDC_POLICYENTRIES, m_PolicyEntries);
  DDX_Control(pDX, IDC_PASSWORD, m_ex_password);
}

BEGIN_MESSAGE_MAP(CManagePSWDPols, CPWDialog)
  ON_BN_CLICKED(IDHELP, OnHelp)
  ON_BN_CLICKED(IDCANCEL, OnCancel)
  ON_BN_CLICKED(IDC_DELETE, OnDelete)
  ON_BN_CLICKED(IDC_NEW, OnNew)
  ON_BN_CLICKED(IDC_EDIT, OnEdit)
  ON_BN_CLICKED(IDC_LIST_POLICYENTRIES, OnList)
  ON_BN_CLICKED(IDC_GENERATEPASSWORD, OnGeneratePassword)
  ON_BN_CLICKED(IDC_UNDO, OnUndo)
  ON_BN_CLICKED(IDC_REDO, OnRedo)
  ON_BN_CLICKED(IDC_COPYPASSWORD, OnCopyPassword)

  ON_NOTIFY(NM_CLICK, IDC_POLICYLIST, OnPolicySelected)
  ON_NOTIFY(NM_RCLICK, IDC_POLICYLIST, OnPolicyRightClick)
  ON_NOTIFY(LVN_KEYDOWN, IDC_POLICYLIST, OnPolicySelected)
  ON_NOTIFY(NM_DBLCLK, IDC_POLICYENTRIES, OnEntryDoubleClicked)

  ON_NOTIFY(HDN_ITEMCLICK, IDC_POLICYNAMES_HEADER, OnColumnNameClick)
  ON_NOTIFY(HDN_ITEMCLICK, IDC_POLICYENTRIES_HEADER, OnColumnEntryClick)

  ON_COMMAND(ID_MENUITEM_LISTENTRIES, OnListEntries)
END_MESSAGE_MAP()

// CManagePSWDPols message handlers

BOOL CManagePSWDPols::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  m_pCopyBtn = (CButton *)GetDlgItem(IDC_COPYPASSWORD);

  if (m_bReadOnly) {
    GetDlgItem(IDC_NEW)->EnableWindow(FALSE);
    GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);

    // Hide cancel button & change OK button text
    GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);

    // Change button text
    CString cs_text(MAKEINTRESOURCE(IDS_CLOSE));
    GetDlgItem(IDOK)->SetWindowText(cs_text);
    cs_text.LoadString(IDS_VIEW);
    GetDlgItem(IDC_EDIT)->SetWindowText(cs_text);
  }

  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, TTS_BALLOON | TTS_NOPREFIX)) {
    pws_os::Trace(L"Unable To create CManagePSWDPols Dialog ToolTip\n");
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
  } else {
    EnableToolTips(TRUE);

    // Delay initial show & reshow
    int iTime = m_pToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
    m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, iTime * 4);
    m_pToolTipCtrl->Activate(TRUE);
    m_pToolTipCtrl->SetMaxTipWidth(500);

    CString cs_ToolTip(MAKEINTRESOURCE(IDS_NEWPOLICY));
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_NEW), cs_ToolTip);
    cs_ToolTip.LoadString(IDS_DELETEPOLICY);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_DELETE), cs_ToolTip);
    cs_ToolTip.LoadString(IDS_EDITPOLICY);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_EDIT), cs_ToolTip);
    cs_ToolTip.LoadString(IDS_UNDOPOLICY);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_UNDO), cs_ToolTip);
    cs_ToolTip.LoadString(IDS_REDOPOLICY);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_REDO), cs_ToolTip);
    cs_ToolTip.LoadString(IDS_LISTPOLICY);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_LIST_POLICYENTRIES), cs_ToolTip);
    cs_ToolTip.LoadString(IDS_TESTPOLICY);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_GENERATEPASSWORD), cs_ToolTip);
    cs_ToolTip.LoadString(IDS_CLICKTOCOPYGENPSWD);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_COPYPASSWORD), cs_ToolTip);
    cs_ToolTip.LoadString(IDS_CLICKTOLISTENTRIES);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_POLICYLIST), cs_ToolTip);

    if (!m_bReadOnly) {
      cs_ToolTip.LoadString(IDS_CANCELPOLICYCHANGES);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDCANCEL), cs_ToolTip);
      cs_ToolTip.LoadString(IDS_SAVEPOLICYCHANGES);
      m_pToolTipCtrl->AddTool(GetDlgItem(IDOK), cs_ToolTip);
    }
  }

  DWORD dwStyle = m_PolicyNames.GetExtendedStyle();
  dwStyle |= (LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
  m_PolicyNames.SetExtendedStyle(dwStyle);

  dwStyle = m_PolicyEntries.GetExtendedStyle();
  dwStyle |= (LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
  m_PolicyEntries.SetExtendedStyle(dwStyle);

  // Override default HeaderCtrl ID of 0
  m_PolicyNames.GetHeaderCtrl()->SetDlgCtrlID(IDC_POLICYNAMES_HEADER);
  m_PolicyEntries.GetHeaderCtrl()->SetDlgCtrlID(IDC_POLICYENTRIES_HEADER);

  // BR1108 - Allow user to config these fonts as well
  // Currently use same font as specified for Add/Edit
  m_PolicyNames.SetFont(Fonts::GetInstance()->GetAddEditFont());
  m_PolicyDetails.SetFont(Fonts::GetInstance()->GetAddEditFont());

  CString cs_text;

  // Add columns
  cs_text.LoadString(IDS_POLICYNAME);
  m_PolicyNames.InsertColumn(0, cs_text);
  cs_text.LoadString(IDS_USECOUNT);
  m_PolicyNames.InsertColumn(1, cs_text, LVCFMT_CENTER);

  UpdateNames();

  // Select default == current database
  m_iSelectedItem = 0;
  m_PolicyNames.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

  // Add columns to policy details CListCtrl - Name and value
  cs_text.LoadString(IDS_POLICYFIELD);
  m_PolicyDetails.InsertColumn(0, cs_text, LVCFMT_CENTER);
  cs_text.LoadString(IDS_VALUE);
  m_PolicyDetails.InsertColumn(1, cs_text, LVCFMT_LEFT);

  // Add columns to policy entries CListCtrl - Group, Title, Username
  cs_text.LoadString(IDS_GROUP);
  m_PolicyEntries.InsertColumn(0, cs_text, LVCFMT_CENTER);
  cs_text.LoadString(IDS_TITLE);
  m_PolicyEntries.InsertColumn(1, cs_text, LVCFMT_LEFT);
  cs_text.LoadString(IDS_USERNAME);
  m_PolicyEntries.InsertColumn(2, cs_text, LVCFMT_LEFT);

  m_bViewPolicy = true;

  // Show its details
  UpdateDetails(); 

  // Since we select the default, disable List & Delete
  GetDlgItem(IDC_LIST_POLICYENTRIES)->EnableWindow(FALSE);
  GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
  
  Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_PASSWORD));
  m_ex_password.SetSecure(false);

  // Remove password character so that the password is displayed
  m_ex_password.SetPasswordChar(0);

  // Max. of 255 policy names allowed - only 2 hex digits used for number
  if (m_MapPSWDPLC.size() >= 255)
    GetDlgItem(IDC_NEW)->EnableWindow(FALSE);

  // Load bitmap
  UINT nImageID = PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar) ?
    IDB_COPYPASSWORD_NEW : IDB_COPYPASSWORD_CLASSIC;

  m_bImageLoaded = m_CopyPswdBitmap.Attach(::LoadImage(
                  ::AfxFindResourceHandle(MAKEINTRESOURCE(nImageID), RT_BITMAP),
                  MAKEINTRESOURCE(nImageID), IMAGE_BITMAP, 0, 0,
                  (LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_SHARED)));
  
  ASSERT(m_bImageLoaded);
  if (m_bImageLoaded) {
    FixBitmapBackground(m_CopyPswdBitmap);
  }

  nImageID = PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar) ?
    IDB_COPYPASSWORD_NEW_D : IDB_COPYPASSWORD_CLASSIC_D;

  m_bDisabledImageLoaded = m_DisabledCopyPswdBitmap.Attach(
    ::LoadImage(::AfxFindResourceHandle(MAKEINTRESOURCE(nImageID), RT_BITMAP),
      MAKEINTRESOURCE(nImageID), IMAGE_BITMAP, 0, 0,
      (LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_SHARED)));

  ASSERT(m_bDisabledImageLoaded);
  if (m_bDisabledImageLoaded) {
    FixBitmapBackground(m_DisabledCopyPswdBitmap);
    m_pCopyBtn->SetBitmap(m_DisabledCopyPswdBitmap);
  }

  // No changes yet
  GetDlgItem(IDC_UNDO)->EnableWindow(FALSE);
  GetDlgItem(IDC_REDO)->EnableWindow(FALSE);

  // Set focus on the policy names CListCtrl and so return FALSE
  GotoDlgCtrl(GetDlgItem(IDC_POLICYLIST));
  return FALSE;
}

BOOL CManagePSWDPols::PreTranslateMessage(MSG *pMsg)
{
  // Do tooltips
  if (pMsg->message == WM_MOUSEMOVE) {
    if (m_pToolTipCtrl != NULL) {
      // Change to allow tooltip on disabled controls
      MSG msg = *pMsg;
      msg.hwnd = (HWND)m_pToolTipCtrl->SendMessage(TTM_WINDOWFROMPOINT, 0,
                                                   (LPARAM)&msg.pt);
      CPoint pt = pMsg->pt;
      ::ScreenToClient(msg.hwnd, &pt);

      msg.lParam = MAKELONG(pt.x, pt.y);

      // Let the ToolTip process this message.
      m_pToolTipCtrl->Activate(TRUE);
      m_pToolTipCtrl->RelayEvent(&msg);
    }
  }

  // Don't even look like it was pressed if it should be disabled
  if (pMsg->message == WM_LBUTTONDOWN && pMsg->hwnd == m_pCopyBtn->GetSafeHwnd() &&
    !m_bCopyPasswordEnabled) {
    return TRUE;
  }

  // Don't even process double click - looks bad
  if (pMsg->message == WM_LBUTTONDBLCLK && pMsg->hwnd == m_pCopyBtn->GetSafeHwnd()) {
    return TRUE;
  }

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  // If user hit the appropriate shortcut - do Undo/Redo
  if (pMsg->message == WM_KEYDOWN) {
    if (m_bUndoShortcut && pMsg->wParam == m_siUndoVirtKey) {
      if (((m_cUndoModifier & HOTKEYF_CONTROL) == HOTKEYF_CONTROL &&
          (GetKeyState(VK_CONTROL) & 0x8000) == 0) || 
          ((m_cUndoModifier & HOTKEYF_ALT) == HOTKEYF_ALT &&
          (GetKeyState(VK_MENU) & 0x8000) == 0) || 
          ((m_cUndoModifier & HOTKEYF_SHIFT) == HOTKEYF_SHIFT &&
          (GetKeyState(VK_SHIFT) & 0x8000) == 0))
        goto exit;

      // Do Undo
      OnUndo();

      // Tell Windows we have processed it
      return TRUE;
    }

    if (m_bRedoShortcut && pMsg->wParam == m_siRedoVirtKey) {
      if (((m_cRedoModifier & HOTKEYF_CONTROL) == HOTKEYF_CONTROL &&
          (GetKeyState(VK_CONTROL) & 0x8000) == 0) || 
          ((m_cRedoModifier & HOTKEYF_ALT) == HOTKEYF_ALT &&
          (GetKeyState(VK_MENU) & 0x8000) == 0) || 
          ((m_cRedoModifier & HOTKEYF_SHIFT) == HOTKEYF_SHIFT &&
          (GetKeyState(VK_SHIFT) & 0x8000) == 0))
        goto exit;

      // Do Redo
      OnRedo();

      // Tell Windows we have processed it
      return TRUE;
    }
  }
 
exit:
  return CPWDialog::PreTranslateMessage(pMsg);
}

void CManagePSWDPols::OnHelp()
{
  ShowHelp(L"::/html/named_password_policies.html");
}

void CManagePSWDPols::OnOK()
{
  if (m_bReadOnly)
    CPWDialog::OnCancel();
  else
    CPWDialog::OnOK();
}

void CManagePSWDPols::OnCancel()
{
  // There may be no more left if the user has undone them all (if any)
  if (m_iundo_pos >= 0 && m_bChanged) {
    // Are you sure?
    CGeneralMsgBox gmb;
    if (gmb.AfxMessageBox(IDS_AREYOUSURE_PN,
                          MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDNO)
      return;
  }

  CPWDialog::OnCancel();
}

void CManagePSWDPols::OnNew()
{
  CPasswordPolicyDlg *pDlg(NULL);

  // Try Tall version
  pDlg = new CPasswordPolicyDlg(IDS_PSWDPOLICY, this, true, m_bReadOnly, m_st_default_pp);

  // Pass default values, PolicyName map and indicate New (Blank policy name)
  CString cs_policyname(L"");
  pDlg->SetPolicyData(cs_policyname, m_MapPSWDPLC);

  INT_PTR rc = pDlg->DoModal();
  
  if (rc < 0) {
    // Try again with Wide version
    delete pDlg;
    pDlg = new CPasswordPolicyDlg(IDS_PSWDPOLICY, this, false, m_bReadOnly, m_st_default_pp);

    // Pass default values, PolicyName map and indicate New (Blank policy name)
    pDlg->SetPolicyData(cs_policyname, m_MapPSWDPLC);

    rc = pDlg->DoModal(); 
  }

  if (rc == IDOK) {
    m_bChanged = true;
    
    // Get new named password policy
    pDlg->GetPolicyData(m_st_default_pp, cs_policyname, m_MapPSWDPLC);

    // Save changes for Undo/Redo
    st_PSWDPolicyChange st_change;
    st_change.name = cs_policyname;
    st_change.mode = st_PSWDPolicyChange::Mode::ADD;
    st_change.st_pp_save.Empty();

    // Added a named password policy
    PSWDPolicyMapIter iter_new = m_MapPSWDPLC.find(StringX((LPCWSTR)cs_policyname));
    if (iter_new == m_MapPSWDPLC.end())
      ASSERT(0);

    st_change.st_pp_new = iter_new->second;

    if (m_iundo_pos != (int)m_vchanges.size() - 1) {
      // We did have changes that could have been redone
      // But not anymore - delete all these to add new change on the end
      m_vchanges.resize(m_iundo_pos + 1);
    }

    // Add new change
    m_vchanges.push_back(st_change);
    // Update pointer to the one that is next to be undone
    m_iundo_pos++;
    // Update buttons appropriately
    GetDlgItem(IDC_UNDO)->EnableWindow(TRUE);
    GetDlgItem(IDC_REDO)->EnableWindow(FALSE);

    // Update lists
    UpdateNames();
    LVFINDINFO st_lvfindinfo;
    st_lvfindinfo.flags = LVFI_STRING;
    st_lvfindinfo.psz = cs_policyname;

    m_iSelectedItem = m_PolicyNames.FindItem(&st_lvfindinfo);
    ASSERT(m_iSelectedItem != -1);
    m_PolicyNames.SetItemState(m_iSelectedItem, LVIS_SELECTED, LVIS_SELECTED);

    UpdateDetails();
  }
  delete pDlg;
}

void CManagePSWDPols::OnEdit()
{
  CString cs_policyname = m_PolicyNames.GetItemText(m_iSelectedItem, 0);

  PSWDPolicyMapIter iter = m_MapPSWDPLC.find(StringX((LPCWSTR)cs_policyname));
  if (m_iSelectedItem != 0 && iter == m_MapPSWDPLC.end())
    return;

  CPasswordPolicyDlg *pDlg(NULL);

  // Try Tall version
  pDlg = new CPasswordPolicyDlg(m_iSelectedItem == 0 ? IDS_OPTIONS : IDS_PSWDPOLICY,
                                this, true, m_bReadOnly, m_st_default_pp);

  // Pass default values and PolicyName map
  pDlg->SetPolicyData(cs_policyname, m_MapPSWDPLC);

  INT_PTR rc = pDlg->DoModal();
  
  if (rc < 0) {
    // Try again with Wide version
    delete pDlg;
    pDlg = new CPasswordPolicyDlg(m_iSelectedItem == 0 ? IDS_OPTIONS : IDS_PSWDPOLICY,
                                  this, false, m_bReadOnly, m_st_default_pp);

    // Pass default values, PolicyName map and indicate New (Blank policy name)
    pDlg->SetPolicyData(cs_policyname, m_MapPSWDPLC);

    rc = pDlg->DoModal(); 
  }

  if (rc == IDOK) {
    m_bChanged = true;

    // Save changes for Undo/Redo
    st_PSWDPolicyChange st_change;
    st_change.name = m_iSelectedItem != 0 ? cs_policyname : L"";
    st_change.mode = st_PSWDPolicyChange::Mode::MODIFIED;
    st_change.st_pp_save = m_iSelectedItem != 0 ?iter->second : m_st_default_pp;

    // Update default (if changed) or the named policies
    pDlg->GetPolicyData(m_st_default_pp, cs_policyname, m_MapPSWDPLC);

    if (m_iSelectedItem != 0) {
      // Changed a named password policy
      PSWDPolicyMapIter iter_new = m_MapPSWDPLC.find(StringX((LPCWSTR)cs_policyname));
      if (iter_new == m_MapPSWDPLC.end())
        ASSERT(0);
      st_change.st_pp_new = iter_new->second;
    } else {
      // Changed the database default policy
      st_change.st_pp_new = m_st_default_pp;
    }

    if (m_iundo_pos != (int)m_vchanges.size() - 1) {
      // We did have changes that could have been redone
      // But not anymore
      m_vchanges.resize(m_iundo_pos + 1);
    }

    // Add new change
    m_vchanges.push_back(st_change);
    // Update pointer to the one that is next to be undone
    m_iundo_pos++;
    // Update buttons appropriately
    GetDlgItem(IDC_UNDO)->EnableWindow(TRUE);
    GetDlgItem(IDC_REDO)->EnableWindow(FALSE);

    if (m_iSelectedItem != 0) {
      // Update lists
      UpdateNames();
      LVFINDINFO st_lvfindinfo;
      st_lvfindinfo.flags = LVFI_STRING;
      st_lvfindinfo.psz = cs_policyname;

      m_iSelectedItem = m_PolicyNames.FindItem(&st_lvfindinfo);
      ASSERT(m_iSelectedItem != -1);
    }
    m_PolicyNames.SetItemState(m_iSelectedItem, LVIS_SELECTED, LVIS_SELECTED);

    UpdateDetails();
  }
  delete pDlg;
}

void CManagePSWDPols::OnList()
{
  // Must not list first entry (current database password policy)
  // Use "Manage -> Options" instead
  if (m_iSelectedItem < 1)
    return;

  m_bViewPolicy = !m_bViewPolicy;

  if (m_bViewPolicy)
    UpdateDetails();
  else
    UpdateEntryList();

  CString cs_label(MAKEINTRESOURCE(m_bViewPolicy ? IDS_LIST : IDC_DETAILS));
  GetDlgItem(IDC_LIST_POLICYENTRIES)->SetWindowText(cs_label);
}

void CManagePSWDPols::OnDelete()
{
  // Must not delete first entry (current database password policy)
  // Use Manage -> Options
  if (m_iSelectedItem < 1)
    return;

  CString cs_policyname =  m_PolicyNames.GetItemText(m_iSelectedItem, 0);
  m_PolicyNames.SetItemState(m_iSelectedItem, 0, LVIS_SELECTED);
  m_PolicyNames.DeleteItem(m_iSelectedItem);

  // Note: m_iSelectedItem == 0 for default policy that is not in the map.
  // Can't be deleted anyway
  PSWDPolicyMapIter iter = m_MapPSWDPLC.find(StringX((LPCWSTR)cs_policyname));

  // Save changes for Undo/Redo
  st_PSWDPolicyChange st_change;
  st_change.name = cs_policyname;
  st_change.mode = st_PSWDPolicyChange::Mode::REMOVE;
  st_change.st_pp_save.Empty();
  st_change.st_pp_new = iter->second;

  if (m_iundo_pos != (int)m_vchanges.size() - 1) {
    // We did have changes that could have been redone
    // But not anymore
    m_vchanges.resize(m_iundo_pos + 1);
  }

  // Add new change
  m_vchanges.push_back(st_change);
  // Update pointer to the one that is next to be undone
  m_iundo_pos++;
  // Update buttons appropriately
  GetDlgItem(IDC_UNDO)->EnableWindow(TRUE);
  GetDlgItem(IDC_REDO)->EnableWindow(FALSE);

  // Delete it
  if (iter != m_MapPSWDPLC.end())
    m_MapPSWDPLC.erase(iter);
  m_iSelectedItem = -1;

  // Nothing selected now - disable buttons
  GetDlgItem(IDC_EDIT)->EnableWindow(FALSE);
  GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);

  // Max. of 255 policy names allowed - only 2 hex digits used for number
  if (m_MapPSWDPLC.size() >= 255)
    GetDlgItem(IDC_NEW)->EnableWindow(FALSE);

  m_bChanged = true;
}

void CManagePSWDPols::OnGeneratePassword()
{
  PWPolicy st_pp;
  CString cs_policyname(L"");

  if (m_iSelectedItem == 0) { // Use Default Password policy
    st_pp = m_st_default_pp;
  } else { // Named Password Policy
    cs_policyname = m_PolicyNames.GetItemText(m_iSelectedItem, 0);

    PSWDPolicyMapIter iter = m_MapPSWDPLC.find(StringX((LPCWSTR)cs_policyname));
    if (iter == m_MapPSWDPLC.end())
      return;

    st_pp = iter->second;
  }
  
  StringX passwd;
  GetMainDlg()->MakeRandomPassword(passwd, st_pp);
  m_password = passwd.c_str();
  m_ex_password.SetWindowText(m_password);
  m_ex_password.Invalidate();

  m_bCopyPasswordEnabled = m_password.GetLength() > 0;

  // Enable/Disable Copy to Clipboard
  m_pCopyBtn->SetBitmap(m_bCopyPasswordEnabled ? m_CopyPswdBitmap : m_DisabledCopyPswdBitmap);
}

void CManagePSWDPols::OnCopyPassword()
{
  if (!m_bCopyPasswordEnabled)
    return;

  UpdateData(TRUE);

  GetMainDlg()->SetClipboardData(m_password);
  GetMainDlg()->UpdateLastClipboardAction(CItemData::PASSWORD);
}

void CManagePSWDPols::OnPolicySelected(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  *pLResult = 0L;

  m_iSelectedItem = -1;
  switch (pNotifyStruct->code) {
    case NM_CLICK:
    {
      LPNMITEMACTIVATE pLVItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNotifyStruct);
      int iItem = pLVItemActivate->iItem;
      if (iItem < 0) {
        m_PolicyNames.SetItemState(m_iSelectedItem, 0, LVIS_SELECTED | LVIS_DROPHILITED);
      }
      m_iSelectedItem = iItem;
      break;
    }
    case LVN_KEYDOWN:
    {
      LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNotifyStruct);
      m_iSelectedItem = m_PolicyNames.GetNextItem(-1, LVNI_SELECTED);
      int nCount = m_PolicyNames.GetItemCount();
      if ((pLVKeyDown->wVKey == VK_DOWN) && (m_iSelectedItem + 1 < nCount))
        m_iSelectedItem++;
      if ((pLVKeyDown->wVKey == VK_UP) && (m_iSelectedItem - 1 >= 0))
        m_iSelectedItem--;
      break;
    }
    default:
      // No idea how we got here!
      return;
  }

  CString cs_label(MAKEINTRESOURCE(IDS_LIST));
  GetDlgItem(IDC_LIST_POLICYENTRIES)->SetWindowText(cs_label);

  // Clear both lists
  m_PolicyDetails.DeleteAllItems();
  m_PolicyEntries.DeleteAllItems();

  // Remove the tooltip for the entry CListCtrl
  m_pToolTipCtrl->DelTool(GetDlgItem(IDC_POLICYENTRIES));

  // Allow edit/view iff something's selected
  GetDlgItem(IDC_EDIT)->EnableWindow(m_iSelectedItem != -1 ? TRUE : FALSE);

  switch (m_iSelectedItem) {
    case -1:
    case 0:
      // Can't List or Delete the database default (0) or if nothing selected (-1)
      GetDlgItem(IDC_LIST_POLICYENTRIES)->EnableWindow(FALSE);
      GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
      break;
    default:
      // Can Edit any other but only...
      // List if use count is not zero & Delete if use count is zero
      CString cs_policyname = m_PolicyNames.GetItemText(m_iSelectedItem, 0);
      PSWDPolicyMapCIter citer = m_MapPSWDPLC.find(StringX((LPCWSTR)cs_policyname));
      // Do not allow delete of policy if use count is non-zero
      GetDlgItem(IDC_DELETE)->EnableWindow(((citer == m_MapPSWDPLC.end()) || citer->second.usecount != 0 || m_bReadOnly) ? FALSE : TRUE);
      // Do not allow list of associated items if use count is zero
      GetDlgItem(IDC_LIST_POLICYENTRIES)->EnableWindow(((citer == m_MapPSWDPLC.end()) ||
        (citer->second.usecount == 0)) ? FALSE : TRUE);
      break;
  }
  
  if (m_iSelectedItem == -1)
    return;

  m_bViewPolicy = true;
  UpdateDetails(); 
}

void CManagePSWDPols::OnPolicyRightClick(NMHDR * /*pNotifyStruct*/, LRESULT *pLResult)
{
  *pLResult = 0; // Perform default processing on return
  POSITION pos = m_PolicyNames.GetFirstSelectedItemPosition();

  if (pos == NULL)
    return;

  int nItem = m_PolicyNames.GetNextSelectedItem(pos);

  // Ignore is default policy (first entry)
  if (nItem == 0)
    return;

  const StringX sxPolicyName = m_PolicyNames.GetItemText(nItem, 0);

  // Ignore if no entries using this policy
  if (m_MapPSWDPLC[sxPolicyName].usecount == 0)
    return;

  PWScore *pcore = (PWScore *)GetMainDlg()->GetCore();
  m_ventries.clear();
  // Ignore if can't find any even if there should be!
  if (!pcore->GetEntriesUsingNamedPasswordPolicy(sxPolicyName, m_ventries))
    return;

  CPoint msg_pt = ::GetMessagePos();
  CMenu menu;
  int ipopup = IDR_POPLISTENTRIES;

  if (menu.LoadMenu(ipopup)) {
    MENUINFO minfo = { 0 };
    minfo.cbSize = sizeof(MENUINFO);
    minfo.fMask = MIM_MENUDATA;
    minfo.dwMenuData = ipopup;
    VERIFY(menu.SetMenuInfo(&minfo));

    CMenu *pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);

    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, msg_pt.x, msg_pt.y, this);
  }
}

void CManagePSWDPols::OnListEntries()
{
  POSITION pos = m_PolicyNames.GetFirstSelectedItemPosition();
  int nItem = m_PolicyNames.GetNextSelectedItem(pos);
  const StringX sxPolicyName = m_PolicyNames.GetItemText(nItem, 0);

  CPWPListEntries dlg(NULL, sxPolicyName, &m_ventries);

  // SHow the user which entries are using this named password policy
  dlg.DoModal();

  // Clear the data
  m_ventries.clear();
}

void CManagePSWDPols::OnEntryDoubleClicked(NMHDR *, LRESULT *pLResult)
{
  // Set we have processed the event
  *pLResult = 1L;

  POSITION pos = m_PolicyEntries.GetFirstSelectedItemPosition();
  if (pos == NULL)
    return;

  int nIndex = m_PolicyEntries.GetNextSelectedItem(pos);

  if (nIndex < 0)
    return;

  // Get entry's details
  StringX sxGroup = m_PolicyEntries.GetItemText(nIndex, 0);
  StringX sxTitle = m_PolicyEntries.GetItemText(nIndex, 1);
  StringX sxUser  = m_PolicyEntries.GetItemText(nIndex, 2);

  // Go and find it
  ItemListIter iter = GetMainDlg()->Find(sxGroup, sxTitle, sxUser);

  // Not there (weird!) - exit
  if (iter == GetMainDlg()->End())
    return;
  
  // Let user Edit/View entry
  CItemData ci = GetMainDlg()->GetEntryAt(iter);
  if (GetMainDlg()->EditItem(&ci)) {
    // User has edited the entry - need to refresh this list and the main list
    // Get updated Password Policies
    m_MapPSWDPLC = GetMainDlg()->GetPasswordPolicies();
    // Update names
    UpdateNames();
    // Update entry list
    UpdateEntryList();

    // Reselect the Named Policy - won't have changed position even if use count has
    m_PolicyNames.SetItemState(m_iSelectedItem, 0, LVIS_SELECTED | LVIS_DROPHILITED);
  }
}

void CManagePSWDPols::OnColumnNameClick(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  *pLResult = 1L;

  NMHEADER *pNMHeader = (NMHEADER *)pNotifyStruct;

  int iIndex = pNMHeader->iItem;

  HDITEM hdi;
  hdi.mask = HDI_FORMAT;
  CHeaderCtrl *pHdrCtrl = m_PolicyNames.GetHeaderCtrl();

  if (iIndex == m_iSortNamesIndex) {
    m_bSortNamesAscending = !m_bSortNamesAscending;
  } else {
    // Turn off all previous sort arrows
    for (int i = 0; i < pHdrCtrl->GetItemCount(); i++) {
      pHdrCtrl->GetItem(i, &hdi);
      if ((hdi.fmt & (HDF_SORTUP | HDF_SORTDOWN)) != 0) {
        hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
        pHdrCtrl->SetItem(i, &hdi);
      }
    }
    m_iSortNamesIndex = iIndex;
    m_bSortNamesAscending = true;
  }

  pHdrCtrl->GetItem(iIndex, &hdi);

  // Turn off all arrows
  hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
  // Turn on the correct arrow
  hdi.fmt |= (m_bSortNamesAscending ? HDF_SORTUP : HDF_SORTDOWN);
  pHdrCtrl->SetItem(iIndex, &hdi);

  m_PolicyNames.SortItems(&CManagePSWDPols::SortNames, (DWORD_PTR)this);
}

void CManagePSWDPols::OnColumnEntryClick(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  *pLResult = 1L;

  NMHEADER *pNMHeader = (NMHEADER *)pNotifyStruct;

  int iIndex = pNMHeader->iItem;

  HDITEM hdi;
  hdi.mask = HDI_FORMAT;
  CHeaderCtrl *pHdrCtrl = m_PolicyEntries.GetHeaderCtrl();

  if (iIndex == m_iSortEntriesIndex) {
    m_bSortEntriesAscending = !m_bSortEntriesAscending;
  } else {
    // Turn off all previous sort arrows
    for (int i = 0; i < pHdrCtrl->GetItemCount(); i++) {
      pHdrCtrl->GetItem(i, &hdi);
      if ((hdi.fmt & (HDF_SORTUP | HDF_SORTDOWN)) != 0) {
        hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
        pHdrCtrl->SetItem(i, &hdi);
      }
    }
    m_iSortEntriesIndex = iIndex;
    m_bSortEntriesAscending = true;
  }

  pHdrCtrl->GetItem(iIndex, &hdi);

  // Turn off all arrows
  hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
  // Turn on the correct arrow
  hdi.fmt |= (m_bSortEntriesAscending ? HDF_SORTUP : HDF_SORTDOWN);
  pHdrCtrl->SetItem(iIndex, &hdi);

  m_PolicyEntries.SortItems(&CManagePSWDPols::SortEntries, (DWORD_PTR)this);
}

void CManagePSWDPols::UpdateNames()
{
  m_PolicyNames.DeleteAllItems();
  int nPos = 0;

  // Add in the default policy as the first entry
  CString cs_text(MAKEINTRESOURCE(IDSC_DEFAULT_POLICY));
  nPos = m_PolicyNames.InsertItem(nPos, cs_text);
  cs_text.LoadString(IDS_NA);
  m_PolicyNames.SetItemText(nPos, 1, cs_text);
  m_PolicyNames.SetItemData(nPos, (DWORD)-1);
  nPos++;

  // Add in all other policies - ItemData == offset into map
  PSWDPolicyMapIter iter;
  int n = 0;
  for (iter = m_MapPSWDPLC.begin(); iter != m_MapPSWDPLC.end(); iter++, nPos++) {
    nPos = m_PolicyNames.InsertItem(nPos, iter->first.c_str());
    if (iter->second.usecount != 0)
      cs_text.Format(L"%d", iter->second.usecount);
    else
      cs_text.LoadString(IDS_NOT_USED);
    m_PolicyNames.SetItemText(nPos, 1, cs_text);
    m_PolicyNames.SetItemData(nPos, n);
    n++;
  }

  // Resize columns
  m_PolicyNames.SetColumnWidth(0, LVSCW_AUTOSIZE);
  m_PolicyNames.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
}

static void WindowsRowPutter(int row, const std::wstring &name, const std::wstring &value,
                             void *table)
{
  // Callback function used by st_PSWDPolicy::Policy2Table
  CListCtrl *tableControl = (CListCtrl *)table;
  tableControl->InsertItem(row, name.c_str());
  tableControl->SetItemText(row, 1, value.c_str());
}

void CManagePSWDPols::UpdateDetails()
{
  // Make sure correct ListCtrl and title are visible
  m_PolicyDetails.ShowWindow(SW_SHOW);
  m_PolicyDetails.EnableWindow(TRUE);
  GetDlgItem(IDC_STATIC_POLICYDETAILS)->ShowWindow(SW_SHOW);
  m_PolicyEntries.ShowWindow(SW_HIDE);
  m_PolicyEntries.EnableWindow(FALSE);
  GetDlgItem(IDC_STATIC_POLICYENTRIES)->ShowWindow(SW_HIDE);

  // Now fill in the details!

  if (m_iSelectedItem == -1) // or not...
    return;

  /*
    If m_iSelectedItem = 0, then fill in with the database default,
    otherwise use the name entry
  */

  PWPolicy st_pp;

  if (m_iSelectedItem != 0) {
    CString cs_policyname = m_PolicyNames.GetItemText(m_iSelectedItem, 0);

    PSWDPolicyMapIter iter = m_MapPSWDPLC.find(StringX(cs_policyname));
    if (iter == m_MapPSWDPLC.end())
      return;

    st_pp = iter->second;
  } else {
    st_pp = m_st_default_pp;
  }

  // Clear out previous info
  m_PolicyDetails.DeleteAllItems();
  st_pp.Policy2Table(WindowsRowPutter, &m_PolicyDetails);

  m_PolicyDetails.SetColumnWidth(0, LVSCW_AUTOSIZE);
  m_PolicyDetails.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
}

void CManagePSWDPols::UpdateEntryList()
{
  // When list of policy entries is selected,
  // make sure correct ListCtrl and title are visible
  m_PolicyEntries.ShowWindow(SW_SHOW);
  m_PolicyEntries.EnableWindow(TRUE);
  GetDlgItem(IDC_STATIC_POLICYENTRIES)->ShowWindow(SW_SHOW);
  m_PolicyDetails.ShowWindow(SW_HIDE);
  m_PolicyDetails.EnableWindow(FALSE);
  GetDlgItem(IDC_STATIC_POLICYDETAILS)->ShowWindow(SW_HIDE);

  // Add Tooltip for this CListCtrl
  CString cs_ToolTip(MAKEINTRESOURCE(IDS_POLICY_VIEWENTRY));
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_POLICYENTRIES), cs_ToolTip);

  // Clear out previous info
  m_PolicyEntries.DeleteAllItems();

  if (m_iSelectedItem == -1)
    return;

  // Get selected policy name
  CString cs_policyname = m_PolicyNames.GetItemText(m_iSelectedItem, 0);

  // Get all entries that reference it
  if (!GetMainDlg()->MakeMatchingGTUSet(m_setGTU, StringX(cs_policyname)))
    return;

  // Update the ListCtrl
  int nPos(0);
  for (GTUSet::iterator iter = m_setGTU.begin(); iter != m_setGTU.end(); iter++) {
    const st_GroupTitleUser &gtu = *iter;
    int n = m_PolicyEntries.InsertItem(nPos, gtu.group.c_str());
    m_PolicyEntries.SetItemText(n, 1, gtu.title.c_str());
    m_PolicyEntries.SetItemText(n, 2, gtu.user.c_str());
    m_PolicyEntries.SetItemData(n, (DWORD_PTR)&gtu);
    nPos++;
  }

  m_PolicyEntries.SortItems(&CManagePSWDPols::SortEntries, (DWORD_PTR)this);

  m_PolicyEntries.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
  m_PolicyEntries.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
  m_PolicyEntries.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);
}

void CManagePSWDPols::OnUndo()
{
  // Nothing to undo if no changes saved or already at first entry
  if (m_vchanges.size() == 0 && m_iundo_pos < 0)
    return;

  // Get previous change
  st_PSWDPolicyChange st_last_change = m_vchanges[m_iundo_pos];

  bool bDefaultPolicy = st_last_change.name.empty();

  switch (st_last_change.mode) {
    case st_PSWDPolicyChange::Mode::ADD:
    {
      // We added a new policy - delete it
      PSWDPolicyMapIter iter = m_MapPSWDPLC.find(st_last_change.name);
      if (iter != m_MapPSWDPLC.end())
        m_MapPSWDPLC.erase(iter);

      // Select default policy when list refreshed
      m_iSelectedItem = 0;
      break;
    }
    case st_PSWDPolicyChange::Mode::REMOVE:
      // We deleted a policy - add it
      m_MapPSWDPLC[st_last_change.name] = st_last_change.st_pp_save;

      // Select it - but we do not yet know the m_PolicyNames index yet
      m_iSelectedItem = -1;
      break;
    case st_PSWDPolicyChange::Mode::MODIFIED:
      if (bDefaultPolicy) {
        m_st_default_pp = st_last_change.st_pp_save;
        m_iSelectedItem = 0;
      } else {
        m_MapPSWDPLC[st_last_change.name] = st_last_change.st_pp_save;

        // Select it - but we do not yet know the m_PolicyNames index yet
        m_iSelectedItem = -1;
      }
      break;
    default:
      ASSERT(0);
  }

  // Now point back to previous change for next Undo (unless result < 0)
  m_iundo_pos--;
  // Can't do Undo if undo position is before the start of the changes
  GetDlgItem(IDC_UNDO)->EnableWindow(m_iundo_pos < 0 ? FALSE : TRUE);
  GetDlgItem(IDC_REDO)->EnableWindow(TRUE);

  UpdateNames();

  if (m_iSelectedItem != 0) {
    LVFINDINFO st_lvfindinfo;
    st_lvfindinfo.flags = LVFI_STRING;
    st_lvfindinfo.psz = st_last_change.name.c_str();

    m_iSelectedItem = m_PolicyNames.FindItem(&st_lvfindinfo);
    ASSERT(m_iSelectedItem != -1);
  }
  m_PolicyNames.SetItemState(m_iSelectedItem, LVIS_SELECTED, LVIS_SELECTED);

  UpdateDetails();
}

void CManagePSWDPols::OnRedo()
{
  // Nothing to redo if no changes saved or already at last entry
  if (m_vchanges.size() == 0 || m_iundo_pos == (int)m_vchanges.size() - 1)
    return;

  // Get next change
  st_PSWDPolicyChange st_next_change = m_vchanges[m_iundo_pos + 1];

  bool bDefaultPolicy = st_next_change.name.empty();

  switch (st_next_change.mode) {
    case st_PSWDPolicyChange::Mode::ADD:
      // We need to add a new policy
      m_MapPSWDPLC[st_next_change.name] = st_next_change.st_pp_new;

      // Select it - but we do not yet know the m_PolicyNames index yet
      m_iSelectedItem = -1;
      break;
    case st_PSWDPolicyChange::Mode::REMOVE:
      {
      // We need to delete a policy
      PSWDPolicyMapIter iter = m_MapPSWDPLC.find(st_next_change.name);
      if (iter != m_MapPSWDPLC.end())
        m_MapPSWDPLC.erase(iter);

      // Select default policy
      m_iSelectedItem = 0;
      break;
      }
    case st_PSWDPolicyChange::Mode::MODIFIED:
      if (bDefaultPolicy) {
        m_st_default_pp = st_next_change.st_pp_new;
        m_iSelectedItem = 0;
      } else {
        m_MapPSWDPLC[st_next_change.name] = st_next_change.st_pp_new;

        // Select it - but we do not yet know the m_PolicyNames index yet
        m_iSelectedItem = -1;
      }
      break;
    default:
      ASSERT(0);
  }

  // Now point back to next change for next Undo (unless result < 0)
  m_iundo_pos++;
  GetDlgItem(IDC_UNDO)->EnableWindow(TRUE);
  // Can't do Redo if undo position is at he end of the changes
  GetDlgItem(IDC_REDO)->EnableWindow(m_iundo_pos == (int)m_vchanges.size() - 1 ? FALSE : TRUE);

  UpdateNames();

  if (m_iSelectedItem != 0) {
    LVFINDINFO st_lvfindinfo;
    st_lvfindinfo.flags = LVFI_STRING;
    st_lvfindinfo.psz = st_next_change.name.c_str();

    m_iSelectedItem = m_PolicyNames.FindItem(&st_lvfindinfo);
    ASSERT(m_iSelectedItem != -1);
  }
  m_PolicyNames.SetItemState(m_iSelectedItem, LVIS_SELECTED, LVIS_SELECTED);

  UpdateDetails();
}

int CALLBACK CManagePSWDPols::SortNames(LPARAM lParam1, LPARAM lParam2,
                                           LPARAM lParamSort)
{
  CManagePSWDPols *self = (CManagePSWDPols *)lParamSort;
  ASSERT(self != NULL);

  // Default password policy always first!
  if (lParam1 < 0)
    return -1;
  
  if (lParam2 < 0)
    return 1;

  int iretval(0);
  
  // Mustn't go past the end!
  ASSERT((size_t)lParam1 < self->m_MapPSWDPLC.size());
  ASSERT((size_t)lParam2 < self->m_MapPSWDPLC.size());

  PSWDPolicyMapCIter lhs, rhs;
  lhs = rhs = self->m_MapPSWDPLC.begin();
  advance(lhs, lParam1);
  advance(rhs, lParam2);

  switch (self->m_iSortNamesIndex) {
    case 0:
      // Policy name
      iretval = wcscmp(lhs->first.c_str(), rhs->first.c_str());
      break;
    case 1:
      // Use count
      iretval = lhs->second.usecount < rhs->second.usecount ? -1 : 1;
      break;
  }

  if (!self->m_bSortNamesAscending)
    iretval *= -1;

  return iretval;
}

int CALLBACK CManagePSWDPols::SortEntries(LPARAM lParam1, LPARAM lParam2,
                                             LPARAM lParamSort)
{
  CManagePSWDPols *self = (CManagePSWDPols *)lParamSort;
  ASSERT(self != NULL);

  int iretval(0);
  
  st_GroupTitleUser *lhs = reinterpret_cast<st_GroupTitleUser *>(lParam1);
  st_GroupTitleUser *rhs = reinterpret_cast<st_GroupTitleUser *>(lParam2);

  switch (self->m_iSortEntriesIndex) {
    case 0:
      // Group
      iretval = wcscmp(lhs->group.c_str(), rhs->group.c_str());
      break;
    case 1:
      // Title
      iretval = wcscmp(lhs->title.c_str(), rhs->title.c_str());
      break;
    case 2:
      // User
      iretval = wcscmp(lhs->user.c_str(), rhs->user.c_str());
      break;
  }

  if (!self->m_bSortEntriesAscending)
    iretval *= -1;

  return iretval;
}
