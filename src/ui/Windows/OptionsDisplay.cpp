/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsDisplay.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "GeneralMsgBox.h"
#include "ThisMfcApp.h"
#include "Options_PropertySheet.h"

#include "core/pwsprefs.h"

#include "resource.h"
#include "resource3.h"

#include "OptionsDisplay.h" // Must be after resource.h

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDisplay property page

IMPLEMENT_DYNAMIC(COptionsDisplay, COptions_PropertyPage)

COptionsDisplay::COptionsDisplay(CWnd *pParent, st_Opt_master_data *pOPTMD)
  : COptions_PropertyPage(pParent,
                          COptionsDisplay::IDD, COptionsDisplay::IDD_SHORT,
                          pOPTMD)
{
  m_AlwaysOnTop = M_AlwaysOnTop();
  m_ShowPasswordInEdit = M_ShowPasswordInEdit();
  m_ShowUsernameInTree =  M_ShowUsernameInTree();
  m_ShowPasswordInTree = M_ShowPasswordInTree();
  m_ShowNotesAsTipsInViews = M_ShowNotesAsTipsInViews();
  m_ExplorerTypeTree = M_ExplorerTypeTree();
  m_EnableGrid = M_EnableGrid();
  m_ShowNotesInEdit = M_NotesShowInEdit();
  m_WordWrapNotes = M_WordWrapNotes();
  m_PreExpiryWarn = M_PreExpiryWarn();
  m_HighlightChanges = M_HighlightChanges();
  m_EnableTransparency = M_EnableTransparency();
  m_PreExpiryWarnDays = M_PreExpiryWarnDays();
  m_TreeDisplayStatusAtOpen = M_TreeDisplayStatusAtOpen();
  m_PercentTransparency = M_PercentTransparency();
}

COptionsDisplay::~COptionsDisplay()
{
}

void COptionsDisplay::DoDataExchange(CDataExchange *pDX)
{
  COptions_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsDisplay)
  DDX_Text(pDX, IDC_PREEXPIRYWARNDAYS, m_PreExpiryWarnDays);

  DDX_Check(pDX, IDC_DEFUNSHOWINTREE, m_ShowUsernameInTree);
  DDX_Check(pDX, IDC_DEFPWSHOWINTREE, m_ShowPasswordInTree);
  DDX_Check(pDX, IDC_DEFPWSHOWINEDIT, m_ShowPasswordInEdit);
  DDX_Check(pDX, IDC_DEFNOTESSHOWINEDIT, m_ShowNotesInEdit);
  DDX_Check(pDX, IDC_ALWAYSONTOP, m_AlwaysOnTop);
  DDX_Check(pDX, IDC_DEFNTSHOWASTIPSINVIEWS, m_ShowNotesAsTipsInViews);
  DDX_Check(pDX, IDC_DEFEXPLORERTREE, m_ExplorerTypeTree);
  DDX_Check(pDX, IDC_DEFNOTESWRAP, m_WordWrapNotes);
  DDX_Check(pDX, IDC_DEFENABLEGRIDLINES, m_EnableGrid);
  DDX_Check(pDX, IDC_PREWARNEXPIRY, m_PreExpiryWarn);
  DDX_Check(pDX, IDC_HIGHLIGHTCHANGES, m_HighlightChanges);
  DDX_Check(pDX, IDC_ENABLETRANSPARENCY, m_EnableTransparency);

  DDX_Radio(pDX, IDC_TREE_DISPLAY_COLLAPSED, m_TreeDisplayStatusAtOpen); // only first!

  DDX_Control(pDX, IDC_DEFUNSHOWINTREE, m_chkbox[0]);
  DDX_Control(pDX, IDC_DEFPWSHOWINTREE, m_chkbox[1]);
  DDX_Control(pDX, IDC_DEFPWSHOWINEDIT, m_chkbox[2]);
  DDX_Control(pDX, IDC_DEFNOTESSHOWINEDIT,m_chkbox[3]);
  DDX_Control(pDX, IDC_TREE_DISPLAY_COLLAPSED, m_radiobtn[0]);
  DDX_Control(pDX, IDC_TREE_DISPLAY_EXPANDED, m_radiobtn[1]);
  DDX_Control(pDX, IDC_TREE_DISPLAY_LASTSAVE, m_radiobtn[2]);

  DDX_Slider(pDX, IDC_TRANSPARENCY, m_PercentTransparency);

  DDX_Control(pDX, IDC_TRANSPARENCYHELP, m_Help1);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsDisplay, COptions_PropertyPage)
  //{{AFX_MSG_MAP(COptionsDisplay)
  ON_WM_CTLCOLOR()

  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_BN_CLICKED(IDC_PREWARNEXPIRY, OnPreWarn)
  ON_BN_CLICKED(IDC_DEFUNSHOWINTREE, OnDisplayUserInTree)
  ON_BN_CLICKED(IDC_ENABLETRANSPARENCY, OnEnabletransparency)

  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDisplay message handlers

BOOL COptionsDisplay::OnInitDialog() 
{
  COptions_PropertyPage::OnInitDialog();

  for (int i = 0; i < 4; i++) {
    m_chkbox[i].SetTextColour(CR_DATABASE_OPTIONS);
    m_chkbox[i].ResetBkgColour(); // Use current window's background
  }

  for (int i = 0; i < 3; i++) {
    m_radiobtn[i].SetTextColour(CR_DATABASE_OPTIONS);
    m_radiobtn[i].SetType(BS_AUTORADIOBUTTON);
    m_radiobtn[i].ResetBkgColour(); // Use current window's background
  }

  // Database preferences - can't change in R/O mode of if no DB is open
  if (!GetMainDlg()->IsDBOpen() || GetMainDlg()->IsDBReadOnly()) {
    GetDlgItem(IDC_DEFUNSHOWINTREE)->EnableWindow(FALSE);
    GetDlgItem(IDC_DEFPWSHOWINTREE)->EnableWindow(FALSE);
    GetDlgItem(IDC_DEFPWSHOWINEDIT)->EnableWindow(FALSE);
    GetDlgItem(IDC_DEFNOTESSHOWINEDIT)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_INITIALTREEVIEW)->EnableWindow(FALSE);
    GetDlgItem(IDC_TREE_DISPLAY_COLLAPSED)->EnableWindow(FALSE);
    GetDlgItem(IDC_TREE_DISPLAY_EXPANDED)->EnableWindow(FALSE);
    GetDlgItem(IDC_TREE_DISPLAY_LASTSAVE)->EnableWindow(FALSE);
  }

  if (m_ShowUsernameInTree == FALSE) {
    m_ShowPasswordInTree = FALSE;
    GetDlgItem(IDC_DEFPWSHOWINTREE)->EnableWindow(FALSE);
  }

  OnPreWarn();

  CSpinButtonCtrl *pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PREWARNEXPIRYSPIN);
  pspin->SetBuddy(GetDlgItem(IDC_PREEXPIRYWARNDAYS));
  pspin->SetRange(M_prefminExpiryDays(), M_prefmaxExpiryDays());
  pspin->SetBase(10);
  pspin->SetPos(m_PreExpiryWarnDays);

  CSliderCtrl *pslider = (CSliderCtrl *)GetDlgItem(IDC_TRANSPARENCY);
  pslider->SetRange(M_prefminPercentTransparency(), M_prefmaxPercentTransparency());
  pslider->SetTicFreq((M_prefminPercentTransparency() - M_prefmaxPercentTransparency()) / 10);
  pslider->SetPos(m_PercentTransparency);
  pslider->SetTipSide(TBTS_TOP);

  CString csText;
  csText.Format(L"%d%%", M_prefmaxPercentTransparency());
  GetDlgItem(IDC_STATIC_MAXTRANSPARENCY)->SetWindowText(csText);

  if (!app.GetMainDlg()->GetInitialTransparencyState()) {
    // Don't allow use of slider until transparency enabled at startup
    GetDlgItem(IDC_TRANSPARENCY)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_MINTRANSPARENCY)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_MAXTRANSPARENCY)->EnableWindow(FALSE);
  }

  if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
    m_Help1.Init(IDB_QUESTIONMARK);

    // Note naming convention: string IDS_xxx corresponds to control IDC_xxx_HELP
    AddTool(IDC_TRANSPARENCYHELP, IDS_TRANSPARENCYHELP);
    ActivateToolTip();
  } else {
    m_Help1.EnableWindow(FALSE);
    m_Help1.ShowWindow(SW_HIDE);
  }

  return TRUE;  // return TRUE unless you set the focus to a control
}

LRESULT COptionsDisplay::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);
  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      if (M_AlwaysOnTop()             != m_AlwaysOnTop             ||
          M_ShowUsernameInTree()      != m_ShowUsernameInTree      ||
          M_ShowPasswordInTree()      != m_ShowPasswordInTree      ||
          M_ShowNotesAsTipsInViews()  != m_ShowNotesAsTipsInViews  ||
          M_ExplorerTypeTree()        != m_ExplorerTypeTree        ||
          M_EnableGrid()              != m_EnableGrid              ||
          M_ShowPasswordInEdit()      != m_ShowPasswordInEdit      ||
          M_NotesShowInEdit()         != m_ShowNotesInEdit         ||
          M_WordWrapNotes()           != m_WordWrapNotes           ||
          M_PreExpiryWarn()           != m_PreExpiryWarn           ||
          (m_PreExpiryWarn            == TRUE &&
           M_PreExpiryWarnDays()      != m_PreExpiryWarnDays)      ||
          M_TreeDisplayStatusAtOpen() != m_TreeDisplayStatusAtOpen ||
          M_HighlightChanges()        != m_HighlightChanges        ||
          M_EnableTransparency()      != m_EnableTransparency      ||
          M_PercentTransparency()     != m_PercentTransparency)
        return 1L;
      break;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselves here first
      if (OnApply() == FALSE)
        return 1L;
  }
  return 0L;
}

BOOL COptionsDisplay::OnApply()
{
  UpdateData(TRUE);

  M_AlwaysOnTop() = m_AlwaysOnTop;
  M_ShowPasswordInEdit() = m_ShowPasswordInEdit;
  M_ShowUsernameInTree() = m_ShowUsernameInTree;
  M_ShowPasswordInTree() = m_ShowPasswordInTree;
  M_ShowNotesAsTipsInViews() = m_ShowNotesAsTipsInViews;
  M_ExplorerTypeTree() = m_ExplorerTypeTree;
  M_EnableGrid() = m_EnableGrid;
  M_NotesShowInEdit() = m_ShowNotesInEdit;
  M_WordWrapNotes() = m_WordWrapNotes;
  M_PreExpiryWarn() = m_PreExpiryWarn;
  M_HighlightChanges() = m_HighlightChanges;
  M_EnableTransparency() = m_EnableTransparency;
  M_PreExpiryWarnDays() = m_PreExpiryWarnDays;
  M_TreeDisplayStatusAtOpen() = m_TreeDisplayStatusAtOpen;
  M_PercentTransparency() = m_PercentTransparency;

  return COptions_PropertyPage::OnApply();
}

BOOL COptionsDisplay::PreTranslateMessage(MSG *pMsg)
{
  RelayToolTipEvent(pMsg);

  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return COptions_PropertyPage::PreTranslateMessage(pMsg);
}

BOOL COptionsDisplay::OnKillActive()
{
  if (UpdateData(TRUE) == FALSE)
    return FALSE;

  CGeneralMsgBox gmb;

  // Update variable from text box
  CString csText;
  ((CEdit *)GetDlgItem(IDC_PREEXPIRYWARNDAYS))->GetWindowText(csText);
  m_PreExpiryWarnDays = _wtoi(csText);

  // Check that options, as set, are valid.
  if (m_PreExpiryWarn == TRUE &&
      (m_PreExpiryWarnDays < M_prefminExpiryDays() || m_PreExpiryWarnDays > M_prefmaxExpiryDays())) {
    csText.Format(IDS_INVALIDEXPIRYWARNDAYS, M_prefminExpiryDays(), M_prefmaxExpiryDays());
    gmb.AfxMessageBox(csText);
    ((CEdit *)GetDlgItem(IDC_PREEXPIRYWARNDAYS))->SetFocus();
    return FALSE;
  }

  return COptions_PropertyPage::OnKillActive();
}

void COptionsDisplay::OnHelp()
{
  ShowHelp(L"::/html/display_tab.html");
}

void COptionsDisplay::OnPreWarn() 
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_PREWARNEXPIRY))->GetCheck() == 1) ? TRUE : FALSE;
  GetDlgItem(IDC_PREWARNEXPIRYSPIN)->EnableWindow(enable);
  GetDlgItem(IDC_PREEXPIRYWARNDAYS)->EnableWindow(enable);
}

void COptionsDisplay::OnDisplayUserInTree()
{
  if (((CButton*)GetDlgItem(IDC_DEFUNSHOWINTREE))->GetCheck() != 1) {
    GetDlgItem(IDC_DEFPWSHOWINTREE)->EnableWindow(FALSE);
    m_ShowPasswordInTree = FALSE;
    ((CButton*)GetDlgItem(IDC_DEFPWSHOWINTREE))->SetCheck(BST_UNCHECKED);
  } else
    GetDlgItem(IDC_DEFPWSHOWINTREE)->EnableWindow(TRUE);
}

HBRUSH COptionsDisplay::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CPWPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

  // Database preferences - controls + associated static text
  switch (pWnd->GetDlgCtrlID()) {
    case IDC_DEFUNSHOWINTREE:
    case IDC_DEFPWSHOWINTREE:
    case IDC_DEFPWSHOWINEDIT:
    case IDC_DEFNOTESSHOWINEDIT:
    case IDC_STATIC_INITIALTREEVIEW:
    case IDC_TREE_DISPLAY_COLLAPSED:
    case IDC_TREE_DISPLAY_EXPANDED:
    case IDC_TREE_DISPLAY_LASTSAVE:
      pDC->SetTextColor(CR_DATABASE_OPTIONS);
      pDC->SetBkMode(TRANSPARENT);
      break;
  }

  return hbr;
}

void COptionsDisplay::OnEnabletransparency()
{
  UpdateData(TRUE);

  BOOL bEnable = m_EnableTransparency == TRUE && app.GetMainDlg()->GetInitialTransparencyState();

  GetDlgItem(IDC_TRANSPARENCY)->EnableWindow(bEnable);
  GetDlgItem(IDC_STATIC_MINTRANSPARENCY)->EnableWindow(bEnable);
  GetDlgItem(IDC_STATIC_MAXTRANSPARENCY)->EnableWindow(bEnable);
}
