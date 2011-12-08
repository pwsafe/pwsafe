/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// CManagePSWDPolices.cpp : implementation file
//

#include "stdafx.h"
#include "ManagePSWDPolices.h"
#include "PasswordPolicyDlg.h"
#include "DboxMain.h"
#include "ThisMfcApp.h" // for online help
#include "GeneralMsgBox.h"

#include "core/PWCharPool.h"

#include "resource3.h"  // String resources

#include <algorithm>

using namespace std;

// CManagePSWDPolices dialog
CManagePSWDPolices::CManagePSWDPolices(CWnd* pParent, const bool bLongPPs)
  : CPWDialog(CManagePSWDPolices::IDD, pParent),
  m_pToolTipCtrl(NULL), m_iSelectedItem(-1), m_bChanged(false), m_iSortEntriesIndex(0),
  m_bSortEntriesAscending(true), m_iSortNamesIndex(0), m_bSortNamesAscending(true),
  m_bViewPolicy(true), m_bLongPPs(bLongPPs)
{
  ASSERT(pParent != NULL);

  m_pDbx = static_cast<DboxMain *>(pParent);

  m_MapPSWDPLC = m_pDbx->GetPasswordPolicies();

  PWSprefs *prefs = PWSprefs::GetInstance();
  m_st_default_pp.Empty();
  if (prefs->GetPref(PWSprefs::PWUseLowercase))
    m_st_default_pp.pwp.flags |= PWSprefs::PWPolicyUseLowercase;
  if (prefs->GetPref(PWSprefs::PWUseUppercase))
    m_st_default_pp.pwp.flags |= PWSprefs::PWPolicyUseUppercase;
  if (prefs->GetPref(PWSprefs::PWUseDigits))
    m_st_default_pp.pwp.flags |= PWSprefs::PWPolicyUseDigits;
  if (prefs->GetPref(PWSprefs::PWUseSymbols))
    m_st_default_pp.pwp.flags |= PWSprefs::PWPolicyUseSymbols;
  if (prefs->GetPref(PWSprefs::PWUseHexDigits))
    m_st_default_pp.pwp.flags |= PWSprefs::PWPolicyUseHexDigits;
  if (prefs->GetPref(PWSprefs::PWUseEasyVision))
    m_st_default_pp.pwp.flags |= PWSprefs::PWPolicyUseEasyVision;
  if (prefs->GetPref(PWSprefs::PWMakePronounceable))
    m_st_default_pp.pwp.flags |= PWSprefs::PWPolicyMakePronounceable;

  m_st_default_pp.pwp.length = prefs->GetPref(PWSprefs::PWDefaultLength);
  m_st_default_pp.pwp.digitminlength = prefs->GetPref(PWSprefs::PWDigitMinLength);
  m_st_default_pp.pwp.lowerminlength = prefs->GetPref(PWSprefs::PWLowercaseMinLength);
  m_st_default_pp.pwp.symbolminlength = prefs->GetPref(PWSprefs::PWSymbolMinLength);
  m_st_default_pp.pwp.upperminlength = prefs->GetPref(PWSprefs::PWUppercaseMinLength);

  m_st_default_pp.symbols = prefs->GetPref(PWSprefs::DefaultSymbols);
}

CManagePSWDPolices::~CManagePSWDPolices()
{
  delete m_pToolTipCtrl;
}

void CManagePSWDPolices::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_POLICYLIST, m_PolicyNames);
  DDX_Control(pDX, IDC_POLICYPROPERTIES, m_PolicyDetails);
  DDX_Control(pDX, IDC_POLICYENTRIES, m_PolicyEntries);
}

BEGIN_MESSAGE_MAP(CManagePSWDPolices, CPWDialog)
  ON_BN_CLICKED(IDHELP, OnHelp)
  ON_BN_CLICKED(IDCANCEL, OnCancel)
  ON_BN_CLICKED(IDC_DELETE, OnDelete)
  ON_BN_CLICKED(IDC_NEW, OnNew)
  ON_BN_CLICKED(IDC_EDIT, OnEdit)
  ON_BN_CLICKED(IDC_LIST_POLICYENTRIES, OnList)
  ON_BN_CLICKED(IDC_GENERATEPASSWORD, OnGeneratePassword)

  ON_NOTIFY(NM_CLICK, IDC_POLICYLIST, OnPolicySelected)
  ON_NOTIFY(NM_DBLCLK, IDC_POLICYENTRIES, OnEntryDoubleClicked)

  ON_NOTIFY(HDN_ITEMCLICK, IDC_POLICYNAMES_HEADER, OnColumnNameClick)
  ON_NOTIFY(HDN_ITEMCLICK, IDC_POLICYENTRIES_HEADER, OnColumnEntryClick)
END_MESSAGE_MAP()

// CManagePSWDPolices message handlers

BOOL CManagePSWDPolices::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, TTS_BALLOON | TTS_NOPREFIX)) {
    pws_os::Trace(L"Unable To create CManagePSWDPolices Dialog ToolTip\n");
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
    cs_ToolTip.LoadString(IDS_LISTPOLICY);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDC_LIST_POLICYENTRIES), cs_ToolTip);
    cs_ToolTip.LoadString(IDS_CANCELPOLICYCHANGES);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDCANCEL), cs_ToolTip);
    cs_ToolTip.LoadString(IDS_SAVEPOLICYCHANGES);
    m_pToolTipCtrl->AddTool(GetDlgItem(IDOK), cs_ToolTip);
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
  
  // Max. of 255 policy names allowed - only 2 hex digits used for number
  if (m_MapPSWDPLC.size() >= 255)
    GetDlgItem(IDC_NEW)->EnableWindow(FALSE);

  // Set focus on the policy names CListCtrl and so return FALSE
  m_PolicyNames.SetFocus();
  return FALSE;
}

BOOL CManagePSWDPolices::PreTranslateMessage(MSG* pMsg)
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

  return CPWDialog::PreTranslateMessage(pMsg);
}

void CManagePSWDPolices::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/named_password_policies.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CManagePSWDPolices::OnCancel()
{
  if (m_bChanged) {
    // Are you sure?
    CGeneralMsgBox gmb;
    if (gmb.AfxMessageBox(IDS_AREYOUSURE_PN,
                          MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDNO)
      return;
  }

  CPWDialog::OnCancel();
}

void CManagePSWDPolices::OnNew()
{
  bool bLongPPs = m_pDbx->LongPPs();

  CPasswordPolicyDlg PasswordPolicy(IDS_PSWDPOLICY, this, bLongPPs, m_st_default_pp);

  // Pass default values, PolicyName map and indicate New (Blank policy name)
  CString cs_policyname(L"");
  PasswordPolicy.SetPolicyData(cs_policyname, m_MapPSWDPLC, m_pDbx);

  INT_PTR rc = PasswordPolicy.DoModal();

  if (rc == IDOK) {
    m_bChanged = true;
    CString cs_policyname;
    PasswordPolicy.GetPolicyData(m_st_default_pp, cs_policyname, m_MapPSWDPLC);

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
}

void CManagePSWDPolices::OnEdit()
{
  CString cs_policyname = m_PolicyNames.GetItemText(m_iSelectedItem, 0);

  PSWDPolicyMapIter iter = m_MapPSWDPLC.find(StringX((LPCWSTR)cs_policyname));
  if (m_iSelectedItem != 0 && iter == m_MapPSWDPLC.end())
    return;

  bool bLongPPs = m_pDbx->LongPPs();

  CPasswordPolicyDlg PasswordPolicy(m_iSelectedItem == 0 ? IDS_OPTIONS : IDS_PSWDPOLICY,
                                    this, bLongPPs, m_st_default_pp);

  // Pass default values and PolicyName map
  PasswordPolicy.SetPolicyData(cs_policyname, m_MapPSWDPLC, m_pDbx);

  INT_PTR rc = PasswordPolicy.DoModal();
  
  if (rc == IDOK) {
    m_bChanged = true;
    // Update default (if changed) or the named policies
    PasswordPolicy.GetPolicyData(m_st_default_pp, cs_policyname, m_MapPSWDPLC);

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
}

void CManagePSWDPolices::OnList()
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

void CManagePSWDPolices::OnDelete()
{
  // Must not delete first entry (current database password policy)
  // Use Manage -> Options
  if (m_iSelectedItem < 1)
    return;

  CString cs_text =  m_PolicyNames.GetItemText(m_iSelectedItem, 0);
  m_PolicyNames.SetItemState(m_iSelectedItem, 0, LVIS_SELECTED);
  m_PolicyNames.DeleteItem(m_iSelectedItem);

  // Note: m_iSelectedItem == 0 for default policy that is not in the map.
  PSWDPolicyMapIter it = m_MapPSWDPLC.find(StringX((LPCWSTR)cs_text));
  if (it != m_MapPSWDPLC.end())
    m_MapPSWDPLC.erase(it);
  m_iSelectedItem = -1;

  // Nothing selected now - disable buttons
  GetDlgItem(IDC_EDIT)->EnableWindow(FALSE);
  GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);

  // Max. of 255 policy names allowed - only 2 hex digits used for number
  if (m_MapPSWDPLC.size() >= 255)
    GetDlgItem(IDC_NEW)->EnableWindow(FALSE);

  m_bChanged = true;
}

void CManagePSWDPolices::OnGeneratePassword()
{
  st_PSWDPolicy st_pp;
  CString cs_policyname(L"");

  if (m_iSelectedItem == 0) {
    // Use Default Password policy
    PWSprefs *prefs = PWSprefs::GetInstance();
  
    if (prefs->GetPref(PWSprefs::PWUseLowercase))
      st_pp.pwp.flags |= PWSprefs::PWPolicyUseLowercase;
    if (prefs->GetPref(PWSprefs::PWUseUppercase))
      st_pp.pwp.flags |= PWSprefs::PWPolicyUseUppercase;
    if (prefs->GetPref(PWSprefs::PWUseDigits))
      st_pp.pwp.flags |= PWSprefs::PWPolicyUseDigits;
    if (prefs->GetPref(PWSprefs::PWUseSymbols))
      st_pp.pwp.flags |= PWSprefs::PWPolicyUseSymbols;
    if (prefs->GetPref(PWSprefs::PWUseHexDigits))
      st_pp.pwp.flags |= PWSprefs::PWPolicyUseHexDigits;
    if (prefs->GetPref(PWSprefs::PWUseEasyVision))
      st_pp.pwp.flags |= PWSprefs::PWPolicyUseEasyVision;
    if (prefs->GetPref(PWSprefs::PWMakePronounceable))
      st_pp.pwp.flags |= PWSprefs::PWPolicyMakePronounceable;
  
    st_pp.pwp.length = prefs->GetPref(PWSprefs::PWDefaultLength);
    st_pp.pwp.digitminlength = prefs->GetPref(PWSprefs::PWDigitMinLength);
    st_pp.pwp.lowerminlength = prefs->GetPref(PWSprefs::PWLowercaseMinLength);
    st_pp.pwp.symbolminlength = prefs->GetPref(PWSprefs::PWSymbolMinLength);
    st_pp.pwp.upperminlength = prefs->GetPref(PWSprefs::PWUppercaseMinLength);
  
    st_pp.symbols = prefs->GetPref(PWSprefs::DefaultSymbols);
  } else {
    // Named Password Policy
    cs_policyname = m_PolicyNames.GetItemText(m_iSelectedItem, 0);

    PSWDPolicyMapIter iter = m_MapPSWDPLC.find(StringX((LPCWSTR)cs_policyname));
    if (iter == m_MapPSWDPLC.end())
      return;

    st_pp = iter->second;
  }
  
  // Special case of Genrate Password - disable selection of policy
  UINT ui = IDS_GENERATEPASSWORD | 0x80000000;
  CPasswordPolicyDlg GenPswdPS(ui, this, m_bLongPPs, st_pp);

  // Pass this policy's values and PolicyName map
  GenPswdPS.SetPolicyData(cs_policyname, m_MapPSWDPLC, m_pDbx);

  GenPswdPS.DoModal();
}

void CManagePSWDPolices::OnPolicySelected(NMHDR *pNotifyStruct, LRESULT *pLResult)
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
      if (pLVKeyDown->wVKey == VK_DOWN)
        m_iSelectedItem = (m_iSelectedItem + 1) % nCount;
      if (pLVKeyDown->wVKey == VK_UP)
        m_iSelectedItem = (m_iSelectedItem - 1 + nCount) % nCount;
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

  switch (m_iSelectedItem) {
    case -1:
      // Can't Edit if nothing selected (-1)
      GetDlgItem(IDC_EDIT)->EnableWindow(FALSE);
      // Drop through by design!!!
    case 0:
      // Can't List or Delete the database default (0) or if nothing selected (-1)
      GetDlgItem(IDC_LIST_POLICYENTRIES)->EnableWindow(FALSE);
      GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
      break;
    default:
      // Can Edit any other but only...
      // List if use count is not zero & Delete if use count is zero
      PSWDPolicyMapCIter citer = m_MapPSWDPLC.cbegin();
      advance(citer, m_PolicyNames.GetItemData(m_iSelectedItem));

      // Always allow edit
      GetDlgItem(IDC_EDIT)->EnableWindow(TRUE);
      // Do not allow list of associated items if use count is zero
      GetDlgItem(IDC_LIST_POLICYENTRIES)->EnableWindow(citer->second.usecount == 0 ?
                                         FALSE : TRUE);
      // Do not allow delete of policy if use count is non-zero
      GetDlgItem(IDC_DELETE)->EnableWindow(citer->second.usecount != 0 ?
                                         FALSE : TRUE);
      break;
  }
  
  if (m_iSelectedItem == -1)
    return;

  m_bViewPolicy = true;
  UpdateDetails(); 
}

void CManagePSWDPolices::OnEntryDoubleClicked(NMHDR *, LRESULT *pLResult)
{
  // Set we have processed the event
  *pLResult = 1L;

  POSITION p = m_PolicyEntries.GetFirstSelectedItemPosition();
  if (p == NULL)
    return;

  int nIndex = m_PolicyEntries.GetNextSelectedItem(p);

  if (nIndex < 0)
    return;

  // Get entry's details
  StringX sxGroup = m_PolicyEntries.GetItemText(nIndex, 0);
  StringX sxTitle = m_PolicyEntries.GetItemText(nIndex, 1);
  StringX sxUser  = m_PolicyEntries.GetItemText(nIndex, 2);

  // Go and find it
  ItemListIter iter = m_pDbx->Find(sxGroup, sxTitle, sxUser);

  // Not there (weird!) - exit
  if (iter == m_pDbx->End())
    return;
  
  // Let user Edit/View entry
  CItemData ci = m_pDbx->GetEntryAt(iter);
  if (m_pDbx->EditItem(&ci)) {
    // User has edited the entry - need to refresh this list and the main list
    // Get updated Password Policies
    m_MapPSWDPLC = m_pDbx->GetPasswordPolicies();
    // Update names
    UpdateNames();
    // Update entry list
    UpdateEntryList();

    // Reselect the Named Policy - won't have changed position even if use count has
    m_PolicyNames.SetItemState(m_iSelectedItem, 0, LVIS_SELECTED | LVIS_DROPHILITED);
  }
}

void CManagePSWDPolices::OnColumnNameClick(NMHDR *pNotifyStruct, LRESULT *pLResult)
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
    // Turn off all previous sort arrrows
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

  m_PolicyNames.SortItems(&CManagePSWDPolices::SortNames, (DWORD_PTR)this);
}

void CManagePSWDPolices::OnColumnEntryClick(NMHDR *pNotifyStruct, LRESULT *pLResult)
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
    // Turn off all previous sort arrrows
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

  m_PolicyEntries.SortItems(&CManagePSWDPolices::SortEntries, (DWORD_PTR)this);
}

void CManagePSWDPolices::UpdateNames()
{
  m_PolicyNames.DeleteAllItems();
  int nPos = 0;

  // Add in the default policy as the first entry
  CString cs_text(MAKEINTRESOURCE(IDS_DATABASE_DEFAULT));
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

void CManagePSWDPolices::UpdateDetails()
{
  // Make sure correct ListCtrl and title are visible
  m_PolicyDetails.ShowWindow(SW_SHOW);
  m_PolicyDetails.EnableWindow(TRUE);
  GetDlgItem(IDC_STATIC_POLICYDETAILS)->ShowWindow(SW_SHOW);
  m_PolicyEntries.ShowWindow(SW_HIDE);
  m_PolicyEntries.EnableWindow(FALSE);
  GetDlgItem(IDC_STATIC_POLICYENTRIES)->ShowWindow(SW_HIDE);

  // Now fill in the details!

  /*
    If m_iSelectedItem = 0, then fill in with the database default,
    otherwise use the name entry
  */

  st_PSWDPolicy st_pp;

  if (m_iSelectedItem != 0) {
    CString cs_policyname = m_PolicyNames.GetItemText(m_iSelectedItem, 0);

    PSWDPolicyMapIter iter = m_MapPSWDPLC.find(StringX(cs_policyname));
    if (iter == m_MapPSWDPLC.end())
      return;

    st_pp = iter->second;
  } else {
    st_pp = m_st_default_pp;
  }

  CString cs_yes(MAKEINTRESOURCE(IDS_YES)), cs_no(MAKEINTRESOURCE(IDS_NO));
  cs_yes.Remove(L'&');
  cs_no.Remove(L'&');

  const bool bEV_PR = (st_pp.pwp.flags & PWSprefs::PWPolicyUseEasyVision) ||
                      (st_pp.pwp.flags & PWSprefs::PWPolicyMakePronounceable);
  int nPos = 0;

  // Clear out previous info
  m_PolicyDetails.DeleteAllItems();

  if (m_iSelectedItem == -1)
    return;

  // Length, Lowercase, Uppercase, Digits, Symbols, EasyVision, Pronounceable, Hexadecimal
  CString cs_text(MAKEINTRESOURCE(IDS_PLENGTH));
  m_PolicyDetails.InsertItem(nPos, cs_text);
  cs_text.Format(L"%d", st_pp.pwp.length);
  m_PolicyDetails.SetItemText(nPos, 1, cs_text);
  nPos++;

  cs_text.LoadString(IDS_PUSELOWER);
  m_PolicyDetails.InsertItem(nPos, cs_text);
  if ((st_pp.pwp.flags & PWSprefs::PWPolicyUseLowercase) != 0) {
    if (bEV_PR)
      cs_text = cs_yes;
    else
      cs_text.Format(IDS_YESNUMBER, st_pp.pwp.lowerminlength);
  } else {
    cs_text = cs_no;
  }
  m_PolicyDetails.SetItemText(nPos, 1, cs_text);
  nPos++;

  cs_text.LoadString(IDS_PUSEUPPER);
  m_PolicyDetails.InsertItem(nPos, cs_text);
  if ((st_pp.pwp.flags & PWSprefs::PWPolicyUseUppercase) != 0) {
    if (bEV_PR)
      cs_text = cs_yes;
    else
      cs_text.Format(IDS_YESNUMBER, st_pp.pwp.upperminlength);
  } else {
    cs_text = cs_no;
  }
  m_PolicyDetails.SetItemText(nPos, 1, cs_text);
  nPos++;

  cs_text.LoadString(IDS_PUSEDIGITS);
  m_PolicyDetails.InsertItem(nPos, cs_text);
  if ((st_pp.pwp.flags & PWSprefs::PWPolicyUseDigits) != 0) {
    if (bEV_PR)
      cs_text = cs_yes;
    else
      cs_text.Format(IDS_YESNUMBER, st_pp.pwp.digitminlength);
  } else {
    cs_text = cs_no;
  }
  m_PolicyDetails.SetItemText(nPos, 1, cs_text);
  nPos++;

  cs_text.LoadString(IDS_PUSESYMBOL);
  m_PolicyDetails.InsertItem(nPos, cs_text);
  if ((st_pp.pwp.flags & PWSprefs::PWPolicyUseSymbols) != 0) {
    if (bEV_PR)
      cs_text = cs_yes;
    else
      cs_text.Format(IDS_YESNUMBER, st_pp.pwp.symbolminlength);
  } else {
    cs_text = cs_no;
  }
  m_PolicyDetails.SetItemText(nPos, 1, cs_text);
  nPos++;

  cs_text.LoadString(IDS_PEASYVISION);
  m_PolicyDetails.InsertItem(nPos, cs_text);
  m_PolicyDetails.SetItemText(nPos, 1,
          (st_pp.pwp.flags & PWSprefs::PWPolicyUseEasyVision) != 0 ? cs_yes : cs_no);
  nPos++;

  cs_text.LoadString(IDS_PPRONOUNCEABLE);
  m_PolicyDetails.InsertItem(nPos, cs_text);
  m_PolicyDetails.SetItemText(nPos, 1,
          (st_pp.pwp.flags & PWSprefs::PWPolicyMakePronounceable) != 0 ? cs_yes : cs_no);
  nPos++;

  cs_text.LoadString(IDS_PHEXADECIMAL);
  m_PolicyDetails.InsertItem(nPos, cs_text);
  m_PolicyDetails.SetItemText(nPos, 1,
          (st_pp.pwp.flags & PWSprefs::PWPolicyUseHexDigits) != 0 ? cs_yes : cs_no);
  nPos++;

  cs_text.LoadString(IDS_USEDEFAULTSYMBOLS);
  m_PolicyDetails.InsertItem(nPos, cs_text);
  m_PolicyDetails.SetItemText(nPos, 1, st_pp.symbols.empty() ? cs_yes : cs_no);
  nPos++;

  cs_text.LoadString(IDS_SYMBOLS);
  m_PolicyDetails.InsertItem(nPos, cs_text);
  stringT std_symbols;
  CPasswordCharPool::GetDefaultSymbols(std_symbols);
  m_PolicyDetails.SetItemText(nPos, 1,
          st_pp.symbols.empty() ? std_symbols.c_str() : st_pp.symbols.c_str());
  nPos++;

  m_PolicyDetails.SetColumnWidth(0, LVSCW_AUTOSIZE);
  m_PolicyDetails.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
}

void CManagePSWDPolices::UpdateEntryList()
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
  if (!m_pDbx->MakeMatchingGTUSet(m_setGTU, StringX(cs_policyname)))
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

  m_PolicyEntries.SortItems(&CManagePSWDPolices::SortEntries, (DWORD_PTR)this);

  m_PolicyEntries.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
  m_PolicyEntries.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
  m_PolicyEntries.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);
}

int CALLBACK CManagePSWDPolices::SortNames(LPARAM lParam1, LPARAM lParam2,
                                           LPARAM lParamSort)
{
  CManagePSWDPolices *self = (CManagePSWDPolices *)lParamSort;
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

int CALLBACK CManagePSWDPolices::SortEntries(LPARAM lParam1, LPARAM lParam2,
                                             LPARAM lParamSort)
{
  CManagePSWDPolices *self = (CManagePSWDPolices *)lParamSort;
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
