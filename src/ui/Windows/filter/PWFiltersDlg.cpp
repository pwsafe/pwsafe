/*
/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// PWFiltersDlg.cpp : implementation file
//

#include <algorithm>

#include "../stdafx.h"

#include "../ThisMfcApp.h" // for online help
#include "../GeneralMsgBox.h"
#include "../PWHdrCtrlNoChng.h"
#include "PWFiltersDlg.h"

#include "../resource2.h"
#include "../resource3.h"

#include "core/PWSFilters.h"

// CPWFiltersDlg dialog

IMPLEMENT_DYNAMIC(CPWFiltersDlg, CPWResizeDialog)

CPWFiltersDlg::CPWFiltersDlg(CWnd* pParent /* = NULL */,
                             const FilterType &filtertype /* = DFTYPE_MAIN */,
                             const CString &filtername /* = L"" */,
                             bool bCanHaveAttachments /* = false */,
                             const std::set<StringX> *psMediaTypes /* = NULL */)
  : CPWResizeDialog(CPWFiltersDlg::IDD, pParent),
  m_numfilters(0), m_iType(filtertype), m_hAccel(NULL), 
  m_filtername(filtername), m_bAllowSet(true),
  m_bCanHaveAttachments(bCanHaveAttachments), m_psMediaTypes(psMediaTypes)
{
}

CPWFiltersDlg::~CPWFiltersDlg()
{
}

BOOL CPWFiltersDlg::OnInitDialog()
{
  std::vector<UINT> vibottombtns;
  UINT main_bns[] = {IDC_APPLY, IDOK, IDCANCEL, ID_HELP};
  UINT other_bns[] = {IDOK, IDCANCEL, ID_HELP};

  if (m_iType == DFTYPE_MAIN && m_bAllowSet)
    vibottombtns.assign(main_bns, main_bns + _countof(main_bns));
  else
    vibottombtns.assign(other_bns, other_bns + _countof(other_bns));

  AddMainCtrlID(IDC_FILTERLC);
  AddBtnsCtrlIDs(vibottombtns);

  UINT statustext[1] = {IDS_BLANK};
  SetStatusBar(&statustext[0], 1);

  CPWResizeDialog::OnInitDialog();

  SetWindowText(m_cstitle);

  m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(),
                                MAKEINTRESOURCE(IDR_FILTERACCELERATOR));

  DWORD dwExStyle = m_FilterLC.GetExtendedStyle();
  dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_SUBITEMIMAGES;
  m_FilterLC.SetExtendedStyle(dwExStyle);

  m_FilterLC.Init(this, m_pfilters, m_iType, m_bCanHaveAttachments, m_psMediaTypes);
  if (m_filtername.IsEmpty() || m_pfilters->fname.empty())
    m_filtername.LoadString(IDS_FILTER_NAME);
  else
    m_filtername = m_pfilters->fname.c_str();

  CHeaderCtrl* pHCtrl;
  pHCtrl = m_FilterLC.GetHeaderCtrl();
  ASSERT(pHCtrl != NULL);
  pHCtrl->SetDlgCtrlID(IDC_FILTERLC_HEADER);
  m_FLCHeader.SubclassWindow(pHCtrl->GetSafeHwnd());
  m_FLCHeader.SetStopChangeFlag(true);

  CRect rect;
  // Move over History & Policy dialogs so as not to obscure main filter dialog
  if (m_iType != DFTYPE_MAIN) {
    GetParent()->GetWindowRect(&rect);
    SetWindowPos(NULL, rect.left + rect.Width() / 3, rect.top + rect.Width() / 3,
                 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  }

  // Main window has Apply (if not called via Manage), OK, Cancel buttons
  // History/Policy/Attachment have OK, Cancel, Help buttons
  // but only one dialog definition in resource file for both in
  // order to ensure that the dialogs look exactly the same and also
  // easier to maintain
  if (m_iType != DFTYPE_MAIN) {
    GetDlgItem(IDC_FILTERNAME)->EnableWindow(FALSE);
  }

  if (!m_bAllowSet) {
    // Change buttons if called via ManageFilters or 
    // is a History/Policy dialog
    GetDlgItem(IDC_APPLY)->EnableWindow(FALSE);
    GetDlgItem(IDC_APPLY)->ShowWindow(SW_HIDE);
  }

  int itotalwidth = 0;
  for (int i = 0; i < FLC_NUM_COLUMNS; i++) {
    itotalwidth += m_FilterLC.GetColumnWidth(i);
  }

  int iMaxWidth = itotalwidth + 16;
  int iMaxHeight = 1024;
  SetMaxHeightWidth(iMaxHeight, iMaxWidth);

  // Update dialog window text
  UpdateStatusText();
  UpdateData(FALSE);

  if (m_iType == DFTYPE_MAIN)
    GetDlgItem(IDC_APPLY)->EnableWindow(m_pfilters->num_Mactive == 0 ? FALSE : TRUE);

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CPWFiltersDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWResizeDialog::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CPWFiltersDlg)
  DDX_Text(pDX, IDC_FILTERNAME, m_filtername);
  DDX_Control(pDX, IDC_FILTERNAME, m_FNameEdit);
  DDX_Control(pDX, IDC_FILTERLC, m_FilterLC);
  DDV_MaxChars(pDX, m_filtername, 32);
  //{{AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPWFiltersDlg, CPWResizeDialog)
  ON_WM_SIZE()
  ON_BN_CLICKED(IDOK, OnOk)
  ON_COMMAND(ID_HELP, OnHelp)
  ON_EN_KILLFOCUS(IDC_FILTERNAME, OnFNameKillFocus)
  ON_COMMAND_RANGE(ID_FLC_CRITERIA, ID_FLC_NEXT, OnProcessKey)
END_MESSAGE_MAP()

void CPWFiltersDlg::OnOk()
{
  if (UpdateData(TRUE) == FALSE)
    return;

  if (m_iType == DFTYPE_MAIN && m_filtername.IsEmpty()) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_FILTERNAMEEMPTY);
    return;
  }

  if (!VerifyFilters())
    return;

  if (m_iType == DFTYPE_MAIN)
    m_pfilters->fname = m_filtername;

  CPWResizeDialog::OnOK();
}

// Small validation functor for finding
// first row that fails validation
struct FilterValidator
{
  FilterValidator(CString &text, int &iHistory, int &iPolicy, int &iAttachment)
    : i(0), text(text), iHistory(iHistory), iPolicy(iPolicy),
    iAttachment(iAttachment) {}
  bool operator()(const st_FilterRow &st_fldata) {
    // return true if FAILS validation, so that find_if will
    // "find" it.
    if ((st_fldata.mtype != PWSMatch::MT_PWHIST &&
         st_fldata.mtype != PWSMatch::MT_POLICY &&
         st_fldata.mtype != PWSMatch::MT_ATTACHMENT) &&
        (st_fldata.mtype == PWSMatch::MT_INVALID ||
         st_fldata.rule  == PWSMatch::MR_INVALID)) {
      text.Format(IDS_FILTERINCOMPLETE, i + 1);
      return true;
    }
    if (st_fldata.mtype == PWSMatch::MT_PWHIST)
      iHistory = i;
    if (st_fldata.mtype == PWSMatch::MT_POLICY)
      iPolicy = i;
    if (st_fldata.mtype == PWSMatch::MT_ATTACHMENT)
      iAttachment = i;
    i++;
    return false;
  }
private:
  int i;
  int &iHistory;
  int &iPolicy;
  int &iAttachment;
  CString &text;
};

bool CPWFiltersDlg::VerifyFilters()
{
  // Verify that the active filters have a criterion set
  if (UpdateData(TRUE) == FALSE)
    return false;

  // First non-History/non-Policy filters on the main filter dialog
  vFilterRows *pvFilterRows(NULL);
  switch (m_iType) {
  case DFTYPE_MAIN:
    pvFilterRows = &m_pfilters->vMfldata;
    break;
  case DFTYPE_PWHISTORY:
    pvFilterRows = &m_pfilters->vHfldata;
      break;
    case DFTYPE_PWPOLICY:
      pvFilterRows = &m_pfilters->vPfldata;
      break;
    case DFTYPE_ATTACHMENT:
      pvFilterRows = &m_pfilters->vAfldata;
      break;
    default:
      VERIFY(0);
  }

  CGeneralMsgBox gmb;
  CString cs_text;
  int iHistory(-1), iPolicy(-1), iAttachment(-1);
  FilterValidator fv(cs_text, iHistory, iPolicy, iAttachment);
  if (find_if(pvFilterRows->begin(), pvFilterRows->end(), fv) !=
    pvFilterRows->end()) {
    gmb.AfxMessageBox(cs_text);
    return false;
  }

  if (m_iType == DFTYPE_MAIN) {
    // Now check that the filters were correct on
    // History/Policy/Attachment sub-filter dialogs
    if (m_FilterLC.IsPWHIST_Set() && !m_FilterLC.IsHistoryGood()) {
      cs_text.Format(IDS_FILTERINCOMPLETE, iHistory + 1);
      gmb.AfxMessageBox(cs_text);
      return false;
    }
    if (m_FilterLC.IsPOLICY_Set() && !m_FilterLC.IsPolicyGood()) {
      cs_text.Format(IDS_FILTERINCOMPLETE, iPolicy + 1);
      gmb.AfxMessageBox(cs_text);
      return false;
    }
    if (m_FilterLC.IsAttachment_Set() && !m_FilterLC.IsAttachmentGood()) {
      cs_text.Format(IDS_FILTERINCOMPLETE, iPolicy + 1);
      gmb.AfxMessageBox(cs_text);
      return false;
    }
  }

  return true;
}

void CPWFiltersDlg::OnHelp()
{
  ShowHelp(L"::/html/filters.html");
}

void CPWFiltersDlg::OnFNameKillFocus()
{
  // Make sure the filter name is updated
  UpdateData(TRUE);
}

void CPWFiltersDlg::UpdateStatusText()
{
  // Update the status bar with number of filters defined and number active
  if (!IsStatusBarOK())
    return;

  CString s;
  int numfilters;

  switch (m_iType) {
    case DFTYPE_MAIN:
      numfilters = (int)m_pfilters->vMfldata.size();
      if (numfilters == 0)
        s.LoadString(IDS_FILTERINFO_NONE);
      else {
        int numactive = m_pfilters->num_Mactive;
        // Make total number correct (i.e. if Policy on main filter
        // and 2 of these set then total is 2 not "1 + 2"
        if (m_FilterLC.IsPWHIST_Set() && m_FilterLC.IsHistoryGood()) {
          numactive += (m_pfilters->num_Hactive - 1);
          numfilters += ((int)m_pfilters->vHfldata.size() - 1);
        }
        if (m_FilterLC.IsPOLICY_Set() && m_FilterLC.IsPolicyGood()) {
          numactive += (m_pfilters->num_Pactive - 1);
          numfilters += ((int)m_pfilters->vPfldata.size() - 1);
        }
        if (m_FilterLC.IsAttachment_Set() && m_FilterLC.IsAttachmentGood()) {
          numactive += (m_pfilters->num_Aactive - 1);
          numfilters += ((int)m_pfilters->vAfldata.size() - 1);
        }
        s.Format(IDS_FILTERINFO, numfilters, numactive);
      }
      break;
    case DFTYPE_PWHISTORY:
      s.Format(IDS_HFILTERINFO, (int)m_pfilters->vHfldata.size(), m_pfilters->num_Hactive);
      break;
    case DFTYPE_PWPOLICY:
      s.Format(IDS_PFILTERINFO, (int)m_pfilters->vPfldata.size(), m_pfilters->num_Pactive);
      break;
    case DFTYPE_ATTACHMENT:
      s.Format(IDS_AFILTERINFO, (int)m_pfilters->vAfldata.size(), m_pfilters->num_Aactive);
      break;
    default:
      ASSERT(0);
  }

  m_RSDStatusBar.SetPaneText(0, s, TRUE);
  m_RSDStatusBar.SetPaneInfo(0, m_RSDStatusBar.GetItemID(0), SBPS_STRETCH, NULL);
  m_RSDStatusBar.UpdateWindow();
}

BOOL CPWFiltersDlg::PreTranslateMessage(MSG *pMsg)
{
  // CListCtrl accelerator processing
  if (pMsg->hwnd == m_FilterLC.m_hWnd &&
      pMsg->message >= WM_KEYFIRST && 
      pMsg->message <= WM_KEYLAST) {
    if (m_hAccel && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
      return TRUE;

    // Listctrl Up & Down arrows are only processed by me via Accelerator table
    if (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN)
      return TRUE;
  }

  // Escape does not close dialog
  if (pMsg->message == WM_KEYDOWN) {
    if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_CANCEL)
      return TRUE;
  }

  // Make sure ComboBox messages go to ComboBox
  if (m_FilterLC.m_ComboBox.GetSafeHwnd() != NULL && 
      pMsg->hwnd == m_FilterLC.m_ComboBox.m_hWnd)
    return CWnd::PreTranslateMessage(pMsg);

  // Otherwise - give to the Dialog!
  return CPWResizeDialog::PreTranslateMessage(pMsg);
}

void CPWFiltersDlg::OnProcessKey(UINT nID)
{
  switch (nID) {
    case ID_FLC_CRITERIA:
    case ID_FLC_ENABLE:
    case ID_FLC_FIELD:
    case ID_FLC_LOGIC:
    case ID_FLC_SELECT:
    case ID_FLC_DELETE:
    case ID_FLC_INSERT:
    case ID_FLC_PREVIOUS:
    case ID_FLC_NEXT:
      m_FilterLC.OnProcessKey(nID);
      break;
    default:
      ASSERT(0);
  }
}

void CPWFiltersDlg::UpdateDialogMaxWidth()
{
  bool bSave = m_FLCHeader.GetStopChangeFlag();
  m_FLCHeader.SetStopChangeFlag(true);
  int itotalwidth = 0;
  for (int i = 0; i < FLC_NUM_COLUMNS - 1; i++) {
    m_FilterLC.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int iw1 =  m_FilterLC.GetColumnWidth(i);
    m_FilterLC.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int iw2 =  m_FilterLC.GetColumnWidth(i);
    m_FilterLC.SetColumnWidth(i, std::max(iw1, iw2));
    itotalwidth += std::max(iw1, iw2);
  }
  m_FilterLC.SetColumnWidth(FLC_NUM_COLUMNS - 1, LVSCW_AUTOSIZE_USEHEADER);
  itotalwidth += m_FilterLC.GetColumnWidth(FLC_NUM_COLUMNS - 1);
  m_FLCHeader.SetStopChangeFlag(bSave);

  int iMaxWidth = itotalwidth + 32;
  int iMaxHeight = 1024;
  SetMaxHeightWidth(iMaxHeight, iMaxWidth);
}

void CPWFiltersDlg::OnSize(UINT nType, int cx, int cy)
{
  CPWResizeDialog::OnSize(nType, cx, cy);

  if (!IsWindow(m_FilterLC.GetSafeHwnd()))
    return;

  // As main control is a CListCtrl, need to do this on the last column
  m_FilterLC.SetColumnWidth(FLC_NUM_COLUMNS - 1, LVSCW_AUTOSIZE_USEHEADER);
}
