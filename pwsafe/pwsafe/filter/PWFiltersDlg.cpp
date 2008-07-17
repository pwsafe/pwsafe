/*
/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// PWFiltersDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "../ThisMfcApp.h"
#include "PWFiltersDlg.h"

#include "../resource2.h"
#include "../resource3.h"
#include "../corelib/corelib.h"
#include "../corelib/PWSFilters.h"

// CPWFiltersDlg dialog

IMPLEMENT_DYNAMIC(CPWFiltersDlg, CPWDialog)

CPWFiltersDlg::CPWFiltersDlg(CWnd* pParent /*=NULL*/,
                             const FilterType &filtertype /*=DFTYPE_MAIN*/,
                             const CString &filtername /*=_T("")*/)
  : CPWDialog(CPWFiltersDlg::IDD, pParent),
  m_numfilters(0), m_iType(filtertype), m_hAccel(NULL), 
  m_bInitDone(false), m_bStatusBarOK(false), m_bStopChange(false),
  m_filtername(filtername)
{
}

CPWFiltersDlg::~CPWFiltersDlg()
{
}

BOOL CPWFiltersDlg::OnInitDialog()
{
  // Do standard OnInitDialog & set appropriate window title
  CPWDialog::OnInitDialog();
  SetWindowText(m_cstitle);

  m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(),
                                MAKEINTRESOURCE(IDR_FILTERACCELERATOR));

  DWORD dwExStyle = m_FilterLC.GetExtendedStyle();
  dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
  m_FilterLC.SetExtendedStyle(dwExStyle);

  m_FilterLC.Init(this, m_pfilters, m_iType);
  m_filtername = m_pfilters->fname;
  m_bStopChange = true;

  // Add the status bar
  if (m_statusBar.CreateEx(this, SBARS_SIZEGRIP)) {
    statustext[0] = IDS_BLANK;
    m_statusBar.SetIndicators(statustext, 1);
    m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0), SBPS_STRETCH, NULL);
    m_statusBar.UpdateWindow();

    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
    m_bStatusBarOK = true;
  }

  CRect rcClientStart;
  CRect rcClientNow;
  GetClientRect(rcClientStart);

  // Arrange all the controls - needed for resizeable dialog
  CRect sbRect, ctrlRect, dlgRect;

  GetClientRect(&dlgRect);
  m_DialogMinWidth = dlgRect.Width();
  m_DialogMinHeight = dlgRect.Height();

  m_statusBar.GetWindowRect(&sbRect);

  m_FilterLC.GetWindowRect(&ctrlRect);
  ScreenToClient(&ctrlRect);

  m_cxBSpace = dlgRect.Size().cx - ctrlRect.Size().cx;
  m_cyBSpace = dlgRect.Size().cy - ctrlRect.Size().cy;
  m_cySBar = sbRect.Size().cy;

  m_FilterLC.SetWindowPos(NULL, NULL, NULL,
                          dlgRect.Size().cx - (2 * ctrlRect.TopLeft().x),
                          dlgRect.Size().cy - m_cyBSpace,
                          SWP_NOMOVE | SWP_NOZORDER);

  CRect rect;
  // Move over History & Policy dialogs so as not to obscure main filter dialog
  if (m_iType != DFTYPE_MAIN) {
    GetParent()->GetWindowRect(&rect);
    SetWindowPos(NULL, rect.left + rect.Width() / 3, rect.top + rect.Width() / 3,
                 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  }

  // Main window has Apply, OK, Cancel buttons
  // History/Policy have OK, Cancel, Help buttons
  // but only one dialog definition in resource file for both in
  // order to ensure that the dialogs look exactly the same and also
  // easier to maintain
  if (m_iType != DFTYPE_MAIN) {
    // Change buttons if History/Policy dialog
    GetDlgItem(IDC_APPLY)->EnableWindow(FALSE);
    GetDlgItem(IDC_APPLY)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_FILTERNAME)->EnableWindow(FALSE);
  }

  // Update dialog window text
  UpdateStatusText();
  UpdateData(FALSE);

  if (m_iType == DFTYPE_MAIN)
    GetDlgItem(IDC_APPLY)->EnableWindow(m_pfilters->num_Mactive == 0 ? FALSE : TRUE);

  m_bInitDone = true;

  // Set up controls nicely
  SetControls(dlgRect.Width(), dlgRect.Height());

  return TRUE;
}

void CPWFiltersDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CPWFiltersDlg)
  DDX_Text(pDX, IDC_FILTERNAME, m_filtername);
  DDX_Control(pDX, IDC_FILTERNAME, m_FNameEdit);
  DDX_Control(pDX, IDC_FILTERLC, m_FilterLC);
  DDV_MaxChars(pDX, m_filtername, 32);
  //{{AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPWFiltersDlg, CPWDialog)
  ON_WM_SIZE()
  ON_WM_GETMINMAXINFO()
  ON_BN_CLICKED(IDOK, OnOk)
  ON_EN_KILLFOCUS(IDC_FILTERNAME, OnFNameKillFocus)
  ON_NOTIFY(HDN_BEGINTRACK, IDC_FILTERLC_HEADER, OnBeginTrack)
  ON_NOTIFY(HDN_ITEMCHANGING, IDC_FILTERLC_HEADER, OnItemchanging)
  ON_COMMAND_RANGE(ID_FLC_CRITERIA, ID_FLC_NEXT, OnProcessKey)
END_MESSAGE_MAP()

void CPWFiltersDlg::OnOk()
{
  if (UpdateData(TRUE) == FALSE)
    return;

  if (m_filtername.IsEmpty()) {
    AfxMessageBox(IDS_FILTERNAMEEMPTY);
    return;
  }

  if (!VerifyFilters())
    return;

  if (m_iType == DFTYPE_MAIN)
    m_pfilters->fname = m_filtername;

  CPWDialog::OnOK();
}

bool CPWFiltersDlg::VerifyFilters()
{
  // Verify that the active filters have a criterion set
  if (UpdateData(TRUE) == FALSE)
    return false;

  // First non-History/non-Policy filters on the main filter dialog
  int i(0);
  int iHistory(-1), iPolicy(-1);
  vfilterdata *pvfilterdata(NULL);
  switch (m_iType) {
    case DFTYPE_MAIN:
      pvfilterdata = &m_pfilters->vMfldata;
      break;
    case DFTYPE_PWHISTORY:
      pvfilterdata = &m_pfilters->vHfldata;
      break;
    case DFTYPE_PWPOLICY:
      pvfilterdata = &m_pfilters->vPfldata;
      break;
    default:
      VERIFY(0);
  }

  CString cs_text;
  vfilterdata::iterator Flt_iter;
  for (Flt_iter = pvfilterdata->begin(); Flt_iter != pvfilterdata->end(); 
       Flt_iter++) {
    st_FilterData &st_fldata = *Flt_iter;
    if (st_fldata.bFilterActive &&
        (st_fldata.mtype != PWSMatch::MT_PWHIST &&
         st_fldata.mtype != PWSMatch::MT_POLICY) &&
        (st_fldata.mtype == PWSMatch::MT_INVALID ||
         st_fldata.rule == PWSMatch::MR_INVALID)) {
      cs_text.Format(IDS_FILTERINCOMPLETE, i + 1);
      AfxMessageBox(cs_text);
      return false;
    }
    if (st_fldata.mtype == PWSMatch::MT_PWHIST)
      iHistory = i;
    if (st_fldata.mtype == PWSMatch::MT_POLICY)
      iPolicy = i;
    i++;
  }

  if (m_iType == DFTYPE_MAIN) {
    // Now check that the filters were correct on the History/Polict sub-filter dialogs
    if (m_FilterLC.IsPWHIST_Set() && !m_FilterLC.IsHistoryGood()) {
      cs_text.Format(IDS_FILTERINCOMPLETE, iHistory + 1);
      AfxMessageBox(cs_text);
      return false;
    }
    if (m_FilterLC.IsPOLICY_Set() && !m_FilterLC.IsPolicyGood()) {
      cs_text.Format(IDS_FILTERINCOMPLETE, iPolicy + 1);
      AfxMessageBox(cs_text);
      return false;
    }
  }

  return true;
}

void CPWFiltersDlg::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/Welcome.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

void CPWFiltersDlg::OnFNameKillFocus()
{
  // Make sure the filter name is updated
  UpdateData(TRUE);
}

void CPWFiltersDlg::UpdateStatusText()
{
  // Update the status bar with number of filters defined and number active
  if (!m_bStatusBarOK)
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
        // Make totla number correct (i.e. if Policy on main filter
        // and 2 of these set then total is 2 not "1 + 2"
        if (m_FilterLC.IsPWHIST_Set() && m_FilterLC.IsHistoryGood()) {
          numactive += (m_pfilters->num_Hactive - 1);
          numfilters += ((int)m_pfilters->vHfldata.size() - 1);
        }
        if (m_FilterLC.IsPOLICY_Set() && m_FilterLC.IsPolicyGood()) {
          numactive += (m_pfilters->num_Pactive - 1);
          numfilters += ((int)m_pfilters->vPfldata.size() - 1);
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
    default:
      ASSERT(0);
  }

  m_statusBar.SetPaneText(0, s, TRUE);
  m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0), SBPS_STRETCH, NULL);
  m_statusBar.UpdateWindow();
}

void CPWFiltersDlg::OnBeginTrack(NMHDR * /*pNotifyStruct*/, LRESULT* pResult)
{
  // Don't allow user to change the size of any columns!
  *pResult = TRUE;
}

void CPWFiltersDlg::OnItemchanging(NMHDR * pNotifyStruct, LRESULT* pResult)
{
  NMHEADER *pHdr = reinterpret_cast<NMHEADER *>(pNotifyStruct);

  // Only allow last column to resize - it has variable data (filter description)
  if (pHdr->iItem == FLC_CRITERIA_TEXT)
    *pResult = FALSE;
  else
    *pResult = m_bStopChange ? TRUE : FALSE;
}

void CPWFiltersDlg::OnSize(UINT nType, int cx, int cy)
{
  CPWDialog::OnSize(nType, cx, cy);

  SetControls(cx, cy);
}

void CPWFiltersDlg::SetControls(int cx, int cy)
{
  if (!m_bInitDone || !m_bStatusBarOK)
    return;

  CWnd *pwnd1, *pwnd2, *pwnd3, *pwnd4;

  if (!IsWindow(m_FilterLC.GetSafeHwnd()))
    return;

  CRect ctrlRect, dlgRect, ctrlrect2;
  CPoint pt_top;

  GetWindowRect(&dlgRect);

  // Allow ListCtrl to grow/shrink but leave room for the buttons underneath!
  m_FilterLC.GetWindowRect(&ctrlRect);
  
  pt_top.x = ctrlRect.left;
  pt_top.y = ctrlRect.top;
  ScreenToClient(&pt_top);

  m_FilterLC.MoveWindow(pt_top.x, pt_top.y,
                        cx - (2 * pt_top.x), cy - m_cyBSpace, TRUE);

  m_FilterLC.SetColumnWidth(FLC_CRITERIA_TEXT, LVSCW_AUTOSIZE_USEHEADER);

  // Keep buttons in the bottom area
  int xleft, ytop;

  ytop = dlgRect.Height() - m_cyBSpace / 2 - m_cySBar;

  if (m_iType == DFTYPE_MAIN) {
    pwnd1 = GetDlgItem(IDC_APPLY);
    pwnd2 = GetDlgItem(IDOK);
    pwnd3 = GetDlgItem(IDCANCEL);
    pwnd4 = GetDlgItem(ID_HELP);

    pwnd1->GetWindowRect(&ctrlRect);
    xleft = (cx / 5) - (ctrlRect.Width() / 2);
    pwnd1->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);

    pwnd2->GetWindowRect(&ctrlRect);
    xleft = (2 * cx / 5) - (ctrlRect.Width() / 2);
    pwnd2->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);

    pwnd3->GetWindowRect(&ctrlRect);
    xleft = (3 * cx / 5) - (ctrlRect.Width() / 2);
    pwnd3->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);

    pwnd4->GetWindowRect(&ctrlRect);
    xleft = (4 * cx / 5) - (ctrlRect.Width() / 2);
    pwnd4->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
  } else {
    pwnd1 = GetDlgItem(IDOK);
    pwnd2 = GetDlgItem(IDCANCEL);
    pwnd3 = GetDlgItem(ID_HELP);
    pwnd4 = NULL;

    pwnd1->GetWindowRect(&ctrlRect);
    xleft = (cx / 4) - (ctrlRect.Width() / 2);
    pwnd1->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);

    pwnd2->GetWindowRect(&ctrlRect);
    xleft = (cx / 2) - (ctrlRect.Width() / 2);
    pwnd2->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);

    pwnd3->GetWindowRect(&ctrlRect);
    xleft = (3 * cx / 4) - (ctrlRect.Width() / 2);
    pwnd3->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
  }

  // Now move the status bar
  m_statusBar.GetWindowRect(&ctrlRect);
  pt_top.x = ctrlRect.left;
  pt_top.y = ctrlRect.top;
  ScreenToClient(&pt_top);

  m_statusBar.MoveWindow(pt_top.x, cy - ctrlRect.Height(),
                         cx - (2 * pt_top.x),
                         ctrlRect.Height(), TRUE);
}

void CPWFiltersDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
  if (m_bInitDone) {
    lpMMI->ptMinTrackSize = CPoint(m_DialogMinWidth, m_DialogMinHeight);
  } else
    CPWDialog::OnGetMinMaxInfo(lpMMI);
}

BOOL CPWFiltersDlg::PreTranslateMessage(MSG* pMsg)
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
  if (m_FilterLC.m_pComboBox && 
      pMsg->hwnd == m_FilterLC.m_pComboBox->m_hWnd)
    return CWnd::PreTranslateMessage(pMsg);

  // Otherwise - give to the Dialog!
  return CPWDialog::PreTranslateMessage(pMsg);
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
