/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// FindReplaceDlg.cpp : implementation file
//

#include "stdafx.h"

#include "FindReplaceDlg.h"
#include "FieldchangesDlg.h"
#include "DboxMain.h"
#include "GeneralMsgBox.h"

// CFindReplaceDlg dialog

IMPLEMENT_DYNAMIC(CFindReplaceDlg, CPWDialog)

CFindReplaceDlg::CFindReplaceDlg(CWnd *pParent)
	: CPWDialog(IDD_FINDREPLACE, pParent),
  m_bCaseSensitive(false),
  m_rule(PWSMatch::MR_INVALID), m_ruleold(PWSMatch::MR_INVALID), 
  m_ft(CItem::START), m_ftold(CItem::START), m_state(0),
  m_bSortAscending(true), m_iSortedColumn(1), m_bResultsLoaded(false)
{
}

CFindReplaceDlg::~CFindReplaceDlg()
{
}

void CFindReplaceDlg::DoDataExchange(CDataExchange *pDX)
{
  CPWDialog::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CFindReplaceDlg)
  DDX_Control(pDX, IDC_FIELD, m_cbxField);
  DDX_Control(pDX, IDC_STRINGRULE, m_cbxRule);
  DDX_Text(pDX, IDC_OLDTEXT, m_secOldText);
  DDX_Text(pDX, IDC_NEWTEXT, m_secNewText);
  DDX_Control(pDX, IDC_OLDTEXT, m_edtOldText);
  DDX_Control(pDX, IDC_NEWTEXT, m_edtNewText);

  DDX_Control(pDX, IDC_CHANGELIST, m_lctChanges);

  DDX_Control(pDX, IDC_STRINGCASE, m_btnCase);
  DDX_Control(pDX, IDC_SEARCH, m_btnSearch);
  DDX_Control(pDX, IDC_CHANGESELECTED, m_btnChangeSelected);

  DDX_Control(pDX, IDC_FINDREPLACERULEHELP, m_Help1);
  DDX_Control(pDX, IDC_FINDREPLACENEWTEXTHELP, m_Help2);
  DDX_Control(pDX, IDC_FINDREPLACESEARCHHELP, m_Help3);
  DDX_Control(pDX, IDC_FINDREPLACECHANGEHELP, m_Help4);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFindReplaceDlg, CPWDialog)
  ON_CBN_SELCHANGE(IDC_FIELD, OnCbnSelchangeField)
  ON_CBN_SELCHANGE(IDC_STRINGRULE, OnCbnSelchangeStringRule)

  ON_EN_CHANGE(IDC_OLDTEXT, OnEdtChangeOldText)
  ON_EN_CHANGE(IDC_NEWTEXT, OnEdtChangeNewText)

  ON_BN_CLICKED(IDOK, OnOK)
  ON_BN_CLICKED(IDC_STRINGCASE, OnCase)
  ON_BN_CLICKED(IDC_SEARCH, OnSearch)
  ON_BN_CLICKED(IDC_CHANGESELECTED, OnChangeSelected)

  ON_NOTIFY(NM_CLICK, IDC_CHANGELIST, OnChangeRowClicked)
  ON_NOTIFY(NM_DBLCLK, IDC_CHANGELIST, OnChangeRowClicked)
  ON_NOTIFY(NM_RCLICK, IDC_CHANGELIST, OnChangeRowRightClicked)
  ON_NOTIFY(LVN_ITEMCHANGING, IDC_CHANGELIST, OnChangeRowChanging)
END_MESSAGE_MAP()

// CFindReplaceDlg message handlers

BOOL CFindReplaceDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  // Get Add/Edit font
  CFont *pFont = Fonts::GetInstance()->GetAddEditFont();

  m_cbxField.SetFont(pFont);
  m_cbxRule.SetFont(pFont);
  m_edtOldText.SetFont(pFont);
  m_edtNewText.SetFont(pFont);

  // Set up Field combobox
  if (m_cbxField.GetCount() == 0) {
    const CItem::FieldType fldx[] = { CItem::AUTOTYPE, CItem::EMAIL, CItem::NOTES,
      CItem::POLICYNAME, CItem::RUNCMD, CItem::URL};

    CString cs_text;
    for (size_t i = 0; i < _countof(fldx); i++) {
      cs_text = CItemData::FieldName(fldx[i]).c_str();
      int iItem = m_cbxField.AddString(cs_text);
      m_cbxField.SetItemData(iItem, fldx[i]);
    }
  }

  // Setup Rule combobox
  // NOTE: This ComboBox is NOT sorted by design !
  if (m_cbxRule.GetCount() == 0) {
    const PWSMatch::MatchRule mrx[] = { PWSMatch::MR_EQUALS, 
      PWSMatch::MR_BEGINS, PWSMatch::MR_ENDS, PWSMatch::MR_CONTAINS};

    CString cs_text;
    for (size_t i = 0; i < _countof(mrx); i++) {
      UINT iumsg = PWSMatch::GetRule(mrx[i]);
      cs_text.LoadString(iumsg);
      int iItem = m_cbxRule.AddString(cs_text);
      m_cbxRule.SetItemData(iItem, mrx[i]);
    }
  }

  m_cbxRule.SetCurSel(-1);
  m_cbxField.SetCurSel(-1);
  m_bCaseSensitive = false;
  m_secOldText = m_secNewText = L"";

  // Set up CListCtrl grid lines
  m_lctChanges.SetExtendedStyle(m_lctChanges.GetExtendedStyle() | LVS_EX_GRIDLINES);

  // Add first column
  m_lctChanges.InsertColumn(0, L"", LVCFMT_CENTER, 20);

  // Add the rest of the columns
  for (int i = 0; i < _countof(uiColumns); i++) {
    CString cs_header;
    cs_header.LoadString(uiColumns[i]);
    m_lctChanges.InsertColumn(i + 1, cs_header);
  }

  // Resize columns
  for (int i = 0; i < _countof(uiColumns) + 1; i++) {
    m_lctChanges.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int nColumnWidth = m_lctChanges.GetColumnWidth(i);
    m_lctChanges.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int nHeaderWidth = m_lctChanges.GetColumnWidth(i);
    m_lctChanges.SetColumnWidth(i, std::max(nColumnWidth, nHeaderWidth));
  }

  // Initialise CListCtrl &  CHeaderCtrl ImageList
  m_lctChanges.Init(this);

  // Don't yet allow user to search/change anything yet. Only cancel
  m_btnChangeSelected.EnableWindow(FALSE);
  m_btnSearch.EnableWindow(FALSE);

  if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
    m_Help1.Init(IDB_QUESTIONMARK);
    m_Help2.Init(IDB_QUESTIONMARK);
    m_Help3.Init(IDB_QUESTIONMARK);
    m_Help4.Init(IDB_QUESTIONMARK);

    // Note naming convention: string IDS_xxx corresponds to control IDC_xxx_HELP
    AddTool(IDC_FINDREPLACERULEHELP, IDS_FINDREPLACERULEHELP);
    AddTool(IDC_FINDREPLACENEWTEXTHELP, IDS_FINDREPLACENEWTEXTHELP);
    AddTool(IDC_FINDREPLACESEARCHHELP, IDS_FINDREPLACESEARCHHELP);
    AddTool(IDC_FINDREPLACECHANGEHELP, IDS_FINDREPLACECHANGEHELP);
    ActivateToolTip();
  } else {
    m_Help1.EnableWindow(FALSE);
    m_Help1.ShowWindow(SW_HIDE);
    m_Help2.EnableWindow(FALSE);
    m_Help2.ShowWindow(SW_HIDE);
    m_Help3.EnableWindow(FALSE);
    m_Help3.ShowWindow(SW_HIDE);
    m_Help4.EnableWindow(FALSE);
    m_Help4.ShowWindow(SW_HIDE);
  }

  UpdateData(FALSE);

  GotoDlgCtrl(&m_cbxField);

  return FALSE;  // return TRUE unless you set the focus to a control
}

BOOL CFindReplaceDlg::PreTranslateMessage(MSG *pMsg)
{
  RelayToolTipEvent(pMsg);

  // Don't allow Enter of Escape prematurely close dialog
  if (pMsg->message == WM_KEYDOWN && pMsg->hwnd != m_btnSearch.GetSafeHwnd()) {
    if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE) {
      return TRUE;
    }
  }

  return CPWDialog::PreTranslateMessage(pMsg);
}

void CFindReplaceDlg::OnCase()
{
  UpdateData(TRUE);

  m_bCaseSensitive = m_btnCase.GetCheck() == BST_CHECKED;
}

void CFindReplaceDlg::OnSearch()
{
  UpdateData(TRUE);

  if ((m_state & ALL) != ALL)
    return;

  // Don't allow changes just yet
  m_btnChangeSelected.EnableWindow(FALSE);

  // Disable all other controls whilst search in progress
  m_btnSearch.EnableWindow(FALSE);
  UpdateButtons(FALSE);

  // clear last search
  m_vFRResults.clear();
  m_lctChanges.DeleteAllItems();
  m_bResultsLoaded = false;

  // Now do search
  DboxMain *pDbx = (DboxMain *)GetParent();
  size_t num_found = pDbx->DoFindReplaceSearch(m_ft, m_rule, m_secOldText, m_bCaseSensitive,
                                               m_vFRResults);

  // Allow buttons again
  m_btnSearch.EnableWindow(TRUE);
  UpdateButtons(TRUE);
  
  if (num_found == 0) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(CString(MAKEINTRESOURCE(IDS_NO_MATCHES)),
      CString(MAKEINTRESOURCE(IDS_FINDREPLACE)), MB_OK);
    return;
  }

  // Stop flashing
  m_lctChanges.SetRedraw(FALSE);

  // Update CListCtrl
  int index = 0;
  for (size_t iresult = 0; iresult < m_vFRResults.size(); iresult++) {
    st_FRResults &st_fr = m_vFRResults[iresult];

    StringX sxOldFieldValue = st_fr.pci->GetFieldValue(m_ft);
    StringX sxOldText = m_secOldText;
    StringX sxNewText = m_secNewText;
    StringX sxNewFieldValue;

    sxNewFieldValue = ChangeField(sxOldFieldValue, sxOldText, sxNewText);

    // Now add to CListCtrl
    index = m_lctChanges.InsertItem(index, L"");
    m_lctChanges.SetItemText(index, 1, st_fr.pci->GetGroup().c_str());
    m_lctChanges.SetItemText(index, 2, st_fr.pci->GetTitle().c_str());
    m_lctChanges.SetItemText(index, 3, st_fr.pci->GetUser().c_str());
    m_lctChanges.SetItemText(index, 4, sxOldFieldValue.c_str());
    m_lctChanges.SetItemText(index, 5, sxNewFieldValue.c_str());
    m_lctChanges.SetItemData(index, iresult);

    index++;
  }

  // Now draw them
  m_lctChanges.SetRedraw(TRUE);

  m_bResultsLoaded = true;

  // Resize columns
  for (int i = 0; i < _countof(uiColumns) + 1; i++) {
    m_lctChanges.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int nColumnWidth = m_lctChanges.GetColumnWidth(i);
    m_lctChanges.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int nHeaderWidth = m_lctChanges.GetColumnWidth(i);
    m_lctChanges.SetColumnWidth(i, std::max(nColumnWidth, nHeaderWidth));
  }

  m_lctChanges.SortItems(CompareFunc, (LPARAM)this);

  // Are any selected?
  size_t numselected = std::count_if(m_vFRResults.begin(), m_vFRResults.end(),
    [](const st_FRResults& st_fr) {
    return st_fr.state != FR_CHANGED && st_fr.state == FR_CHECKED; });

  // Maybe nothing to do!
  m_btnChangeSelected.EnableWindow(numselected == 0 ? FALSE : TRUE);
}

void CFindReplaceDlg::OnChangeSelected()
{
  size_t numselected = std::count_if(m_vFRResults.begin(), m_vFRResults.end(), 
    [](const st_FRResults& st_fr) {
    return st_fr.state != FR_CHANGED && st_fr.state == FR_CHECKED; });

  // Maybe nothing to do - but then shouldn't be here in that case!
  if (numselected == 0)
    return;

  // Disable all other controls whilst change is in progress
  m_btnSearch.EnableWindow(FALSE);
  UpdateButtons(FALSE);

  DboxMain *pDbx = (DboxMain *)GetParent();
  size_t num_changed_now = pDbx->DoFindReplaceEdit(m_ft, m_rule, m_secOldText, m_secNewText,
                                                   m_bCaseSensitive, m_vFRResults);

  // Allow buttons again
  m_btnSearch.EnableWindow(TRUE);
  UpdateButtons(TRUE);

  // Now change state of image in header if all selected/unselected
  numselected = std::count_if(m_vFRResults.begin(), m_vFRResults.end(),
    [](const st_FRResults& st_fr) {
    return st_fr.state != FR_CHANGED && st_fr.state == FR_CHECKED; });

  ASSERT(numselected == 0);

  size_t numchanged = std::count_if(m_vFRResults.begin(), m_vFRResults.end(),
    [](const st_FRResults& st_fr) {
    return st_fr.state == FR_CHANGED; });

  if (numchanged == m_vFRResults.size()) {
    m_lctChanges.SetHeaderImage(FR_CHANGED);
  } else {
    m_lctChanges.SetHeaderImage(numselected == 0 ? FR_UNCHECKED : FR_CHECKED);
  }

  CGeneralMsgBox gmb;
  if (num_changed_now == 0) {
    gmb.AfxMessageBox(CString(MAKEINTRESOURCE(IDS_NO_CHANGES_MADE)),
      CString(MAKEINTRESOURCE(IDS_FINDREPLACE)), MB_OK);
  } else {
    CString csMessage;
    csMessage.Format(IDS_CHANGES_MADE, num_changed_now);
    gmb.AfxMessageBox(csMessage, CString(MAKEINTRESOURCE(IDS_FINDREPLACE)), MB_OK);
  }

  // Allow changes if at least one selected - but shouldn't be now
  // as just changed all those previously selected
  m_btnChangeSelected.EnableWindow(((m_state & ALL) == ALL && numselected > 0) ? TRUE : FALSE);
}

void CFindReplaceDlg::OnCbnSelchangeField()
{
  int isel = m_cbxField.GetCurSel();

  if (isel != CB_ERR) {
    m_ft = (CItem::FieldType)m_cbxField.GetItemData(isel);
    m_state |= FIELDSELECTED;
  } else {
    m_ft = CItem::START;
    m_state &= ~FIELDSELECTED;
  }

  if (m_ftold != m_ft && m_lctChanges.GetItemCount() > 0) {
    m_lctChanges.DeleteAllItems();
    m_lctChanges.SetHeaderImage(FR_UNCHECKED);
  }

  m_btnSearch.EnableWindow((m_state & ALL) == ALL ? TRUE : FALSE);

  m_ftold = m_ft;
}

void CFindReplaceDlg::OnCbnSelchangeStringRule()
{
  int isel = m_cbxRule.GetCurSel();

  if (isel != CB_ERR) {
    m_rule = (PWSMatch::MatchRule)m_cbxRule.GetItemData(isel);
    m_state |= RULESELECTED;
  } else {
    m_rule = PWSMatch::MR_INACTIVE;
    m_state &= ~RULESELECTED;
  }

  if (m_ruleold != m_rule && m_lctChanges.GetItemCount() > 0) {
    m_lctChanges.DeleteAllItems();
    m_lctChanges.SetHeaderImage(FR_UNCHECKED);
  }

  m_btnSearch.EnableWindow((m_state & ALL) == ALL ? TRUE : FALSE);

  m_ruleold = m_rule;
}

void CFindReplaceDlg::OnEdtChangeOldText()
{
  UpdateData(TRUE);

  if (!m_secOldText.IsEmpty()) {
    m_state |= ORIGINALTEXTPRESENT;
  } else {
    m_state &= ~ORIGINALTEXTPRESENT;
  }

  if (m_secOldTextOld != m_secOldText && m_lctChanges.GetItemCount() > 0) {
    m_lctChanges.DeleteAllItems();
    m_lctChanges.SetHeaderImage(FR_UNCHECKED);
  }

  m_btnChangeSelected.EnableWindow(FALSE);

  m_btnSearch.EnableWindow((m_state & ALL) == ALL ? TRUE : FALSE);

  m_secOldTextOld = m_secOldText;
}

void CFindReplaceDlg::OnEdtChangeNewText()
{
  UpdateData(TRUE);

  // Stop flashing
  m_lctChanges.SetRedraw(FALSE);

  // Update ListCtrl
  for (int iItem = 0; iItem < m_lctChanges.GetItemCount(); iItem++) {
    size_t iresult = m_lctChanges.GetItemData(iItem);
    st_FRResults &st_fr = m_vFRResults[iresult];

    StringX sxOldFieldValue = st_fr.pci->GetFieldValue(m_ft);
    StringX sxOldText = m_secOldText;
    StringX sxNewText = m_secNewText;
    StringX sxNewFieldValue;

    sxNewFieldValue = ChangeField(sxOldFieldValue, sxOldText, sxNewText);

    // Now change new test in CListCtrl
    m_lctChanges.SetItemText(iItem, 5, sxNewFieldValue.c_str());
  }

  // Now draw them
  m_lctChanges.SetRedraw(TRUE);
}

void CFindReplaceDlg::SetAllSelected(FRState new_state)
{
  if (m_vFRResults.empty())
    return;

  // Stop flashing
  m_lctChanges.SetRedraw(FALSE);

  for (int iItem = 0; iItem < m_lctChanges.GetItemCount(); iItem++) {
    size_t iresult = m_lctChanges.GetItemData(iItem);
    if (m_vFRResults[iresult].state != FR_CHANGED) {
      m_vFRResults[iresult].state = new_state;
      m_lctChanges.Update(iItem);
    }
  }

  // Redraw CListCtrl
  m_lctChanges.SetRedraw(TRUE);

  if (new_state == FR_CHECKED) {
    // Now change state of image in header if all selected/unselected
    size_t numselected = std::count_if(m_vFRResults.begin(), m_vFRResults.end(),
      [](const st_FRResults& st_fr) {
      return st_fr.state != FR_CHANGED && st_fr.state == FR_CHECKED; });

    size_t numchanged = std::count_if(m_vFRResults.begin(), m_vFRResults.end(),
      [](const st_FRResults& st_fr) {
      return st_fr.state == FR_CHANGED; });

    if (numselected == 0 || (numselected + numchanged) == m_vFRResults.size()) {
      if (numchanged == m_vFRResults.size()) {
        m_lctChanges.SetHeaderImage(FR_CHANGED);
      } else {
        m_lctChanges.SetHeaderImage(numselected == 0 ? FR_UNCHECKED : FR_CHECKED);
      }
    }

    // Allow changes if at least one selected
    m_btnChangeSelected.EnableWindow(((m_state & ALL) == ALL && numselected > 0) ? TRUE : FALSE);
  } else {
    m_btnChangeSelected.EnableWindow(FALSE);
  }
}

void CFindReplaceDlg::SortRows(int iColumn)
{
  if (m_lctChanges.GetItemCount() == 0 || !m_bResultsLoaded)
    return;

  if (iColumn == m_iSortedColumn)
    m_bSortAscending = !m_bSortAscending;
  else
    m_bSortAscending = true;

  m_iSortedColumn = iColumn;
  m_lctChanges.SortItems(CompareFunc, (LPARAM)this);

  m_lctChanges.SetHeaderSortArrows(m_iSortedColumn, m_bSortAscending);
}

int CALLBACK CFindReplaceDlg::CompareFunc(LPARAM lParam1, LPARAM lParam2,
                                         LPARAM closure)
{
  CFindReplaceDlg *self = (CFindReplaceDlg *)closure;
  int nSortColumn = self->m_iSortedColumn;

  CItemData *pLHS_pci = self->m_vFRResults[lParam1].pci;
  CItemData *pRHS_pci = self->m_vFRResults[lParam2].pci;

  int iResult(0);
  switch (nSortColumn) {
  case 0:
    ASSERT(0);
    break;
  case 1:
    iResult = CompareNoCase(pLHS_pci->GetGroup(), pRHS_pci->GetGroup());
    break;
  case 2:
    iResult = CompareNoCase(pLHS_pci->GetTitle(), pRHS_pci->GetTitle());
    break;
  case 3:
    iResult = CompareNoCase(pLHS_pci->GetUser(), pRHS_pci->GetUser());
    break;
  case 4:
    iResult = CompareNoCase(pLHS_pci->GetFieldValue(self->m_ft), pRHS_pci->GetFieldValue(self->m_ft));
    break;
  case 5:
  {
    StringX sxOldText = self->m_secOldText;
    StringX sxNewText = self->m_secNewText;

    StringX sxLHSOldFieldValue = pLHS_pci->GetFieldValue(self->m_ft);
    StringX sxLHSNewFieldValue = self->ChangeField(sxLHSOldFieldValue, sxOldText, sxNewText);

    StringX sxRHSOldFieldValue = pLHS_pci->GetFieldValue(self->m_ft);
    StringX sxRHSNewFieldValue = self->ChangeField(sxRHSOldFieldValue, sxOldText, sxNewText);

    iResult = CompareNoCase(sxLHSNewFieldValue, sxRHSNewFieldValue);
    break;
  }
  default:
    ASSERT(0);
  }

  if (!self->m_bSortAscending && iResult != 0)
    iResult *= -1;

  return iResult;
}

void CFindReplaceDlg::OnChangeRowClicked(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  *pLResult = 0; // Perform default processing on return

  NMITEMACTIVATE *pNMIA = reinterpret_cast<NMITEMACTIVATE *>(pNotifyStruct);

  if (m_vFRResults.empty())
    return;

  LVHITTESTINFO hti = { 0 };
  hti.pt = pNMIA->ptAction;
  int iItem = m_lctChanges.SubItemHitTest(&hti);

  // Ignore any clicks not in an item's checkbox column
  if (iItem == -1 || hti.iSubItem != 0) {
    return;
  }

  // Clicked on check box
  size_t iresult = m_lctChanges.GetItemData(iItem);

  FRState result_state = GetResultState(iresult);
  ASSERT(result_state != FR_INVALID);

  // Ignore if row disabled
  if (result_state == FR_CHANGED)
    return;

  // Toggle checked/unchecked and update entry
  m_vFRResults[iresult].state = (result_state == FR_UNCHECKED) ? FR_CHECKED : FR_UNCHECKED;
  m_lctChanges.Update(iItem);

  // Now change state of image in header if all selected/unselected
  size_t numselected = std::count_if(m_vFRResults.begin(), m_vFRResults.end(),
    [](const st_FRResults& st_fr) {
    return st_fr.state != FR_CHANGED && st_fr.state == FR_CHECKED; });

  size_t numchanged = std::count_if(m_vFRResults.begin(), m_vFRResults.end(),
    [](const st_FRResults& st_fr) {
    return st_fr.state == FR_CHANGED; });

  if (numselected == 0 || (numselected + numchanged) == m_vFRResults.size()) {
    if (numchanged == m_vFRResults.size()) {
      m_lctChanges.SetHeaderImage(FR_CHANGED);
    } else {
      m_lctChanges.SetHeaderImage(numselected == 0 ? FR_UNCHECKED : FR_CHECKED);
    }
  }

  // Allow changes if at least one selected
  m_btnChangeSelected.EnableWindow(((m_state & ALL) == ALL && numselected > 0) ? TRUE : FALSE);
}

void CFindReplaceDlg::OnChangeRowRightClicked(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  *pLResult = 0; // Perform default processing on return

  NMITEMACTIVATE *pNMIA = reinterpret_cast<NMITEMACTIVATE *>(pNotifyStruct);

  LVHITTESTINFO hti = { 0 };
  hti.pt = pNMIA->ptAction;
  int iItem = m_lctChanges.SubItemHitTest(&hti);

  if (iItem >= 0) {
    size_t iresult = m_lctChanges.GetItemData(iItem);
    st_FRResults &st_fr = m_vFRResults[iresult];

    // SHow the changes - can't really do this after the change escpecially if the user
    // used this function to remove characters as we do not save the fields themselves
    // but retireve as necessary and a changed entry will already have been altered.
    if (st_fr.state != FR_CHANGED) {
      StringX sxOldFieldValue = st_fr.pci->GetFieldValue(m_ft);
      StringX sxOldText = m_secOldText;
      StringX sxNewText = m_secNewText;
      StringX sxNewFieldValue;

      // Get it colour coded!
      sxNewFieldValue = ChangeField(sxOldFieldValue, sxOldText, sxNewText, true);

      // Show the user
      CFieldchangesDlg notesDlg(this, st_fr.pci, sxOldFieldValue, sxNewFieldValue,
        m_ft == CItemData::NOTES);

      notesDlg.DoModal();
    }
  }
}

void CFindReplaceDlg::OnChangeRowChanging(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  *pLResult = 0; // Perform default processing on return

  NMITEMACTIVATE *pNMIA = reinterpret_cast<NMITEMACTIVATE *>(pNotifyStruct);

  // Need this as row changed during insert before ItemData set
  if (m_vFRResults.empty() || !m_bResultsLoaded)
    return;

  size_t iresult = m_lctChanges.GetItemData(pNMIA->iItem);

  // Don't allow any changes to a disabled row
  if (GetResultState(iresult) == FR_CHANGED)
    *pLResult = 1;
}

void CFindReplaceDlg::UpdateButtons(const BOOL bEnable)
{
  m_cbxField.EnableWindow(bEnable);
  m_cbxRule.EnableWindow(bEnable);
  m_edtOldText.EnableWindow(bEnable);
  m_edtNewText.EnableWindow(bEnable);
  m_lctChanges.EnableWindow(bEnable);
  m_btnCase.EnableWindow(bEnable);
  GetDlgItem(IDOK)->EnableWindow(bEnable);
}

StringX CFindReplaceDlg::ChangeField(StringX &sxOldFieldValue, StringX &sxOldText, StringX &sxNewText,
  const bool bAddColour)
{
  size_t lenField = sxOldFieldValue.length();
  size_t lenOldText = sxOldText.length();

  const StringX sxFontStart = L"<font color=\"Red\">";
  const StringX sxFontEnd = L"</font>";

  StringX sxNewFieldValue, sxOldTextColour, sxNewTextColour;

  switch (m_rule) {
  case PWSMatch::MR_EQUALS:
    sxNewFieldValue = sxNewText;
    if (bAddColour) {
      if (!sxNewText.empty()) {
        sxNewFieldValue = sxFontStart + sxNewFieldValue + sxFontEnd;
      }
      sxOldFieldValue = sxFontStart + sxOldFieldValue + sxFontEnd;
    }
    break;
  case PWSMatch::MR_BEGINS:
    sxNewFieldValue = sxNewText + sxOldFieldValue.substr(lenOldText);

    if (bAddColour) {
      if (!sxNewText.empty()) {
        sxNewFieldValue = sxFontStart + sxNewText + sxFontEnd + sxOldFieldValue.substr(lenOldText);
      }
      sxOldFieldValue = sxFontStart + sxOldFieldValue.substr(0, lenOldText) +
        sxFontEnd + sxOldFieldValue.substr(lenOldText);
    }
    break;
  case PWSMatch::MR_ENDS:
    sxNewFieldValue = sxOldFieldValue.substr(lenField - lenOldText) + sxNewText;

    if (bAddColour) {
      if (!sxNewText.empty()) {
        sxNewFieldValue = sxOldFieldValue.substr(lenField - lenOldText) + sxFontStart + sxNewText +
          sxFontEnd;
      }
      sxOldFieldValue = sxOldFieldValue.substr(lenField - lenOldText) + sxFontStart + sxOldText +
        sxFontEnd;
    }
    break;
  case PWSMatch::MR_CONTAINS:
    if (m_bCaseSensitive) {
      sxNewFieldValue = sxOldFieldValue;
      Replace(sxNewFieldValue, sxOldText, sxNewText);
    } else {
      sxNewFieldValue = sxOldFieldValue;
      ReplaceNoCase(sxNewFieldValue, sxOldText, sxNewText);
    }

    if (bAddColour) {
      if (m_bCaseSensitive) {
        sxNewFieldValue = sxOldFieldValue;
        sxOldTextColour = sxFontStart + sxOldText + sxFontEnd;

        if (!sxNewText.empty()) {
          sxNewTextColour = sxFontStart + sxNewText + sxFontEnd;
        } else {
          sxNewTextColour = sxNewText;
        }

        // First create the new text with colour
        Replace(sxNewFieldValue, sxOldText, sxNewTextColour);
        // Then change the old text with colour
        Replace(sxOldFieldValue, sxOldText, sxOldTextColour);
      } else {
        sxNewFieldValue = sxOldFieldValue;
        sxOldTextColour = sxFontStart + sxOldText + sxFontEnd;
        sxNewTextColour = sxFontStart + sxNewText + sxFontEnd;

        if (!sxNewText.empty()) {
          sxNewTextColour = sxFontStart + sxNewText + sxFontEnd;
        } else {
          sxNewTextColour = sxNewText;
        }

        // First create the new text with colour
        ReplaceNoCase(sxNewFieldValue, sxOldText, sxNewTextColour);
        // Then change the old text with colour
        ReplaceNoCase(sxOldFieldValue, sxOldText, sxOldTextColour);
      }
    }
    break;
  default:
    ASSERT(0);
  }

  return sxNewFieldValue;
}

FRState CFindReplaceDlg::GetResultState(const size_t iresult)
{ 
  if (m_vFRResults.empty() || iresult < 0 || iresult > (int)m_vFRResults.size())
    return FR_INVALID;

  return m_vFRResults[iresult].state;
}

bool CFindReplaceDlg::SetResultState(const size_t iresult, const FRState state)
{
  if (m_vFRResults.empty() || iresult < 0 || iresult >(int)m_vFRResults.size())
    return false;

  m_vFRResults[iresult].state = state;
  return true;
}
