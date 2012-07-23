/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "core\pwsprefs.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource3.h"
#endif

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
  m_PreExpiryWarnDays = M_PreExpiryWarnDays();
  m_TreeDisplayStatusAtOpen = M_TreeDisplayStatusAtOpen();
  m_TrayIconColour = M_TrayIconColour();
}

COptionsDisplay::~COptionsDisplay()
{
}

void COptionsDisplay::DoDataExchange(CDataExchange* pDX)
{
  COptions_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsDisplay)
  
  DDX_Check(pDX, IDC_DEFUNSHOWINTREE, m_ShowUsernameInTree);
  DDX_Check(pDX, IDC_DEFPWSHOWINTREE, m_ShowPasswordInTree);
  DDX_Check(pDX, IDC_DEFPWSHOWINEDIT, m_ShowPasswordInEdit);
  DDX_Check(pDX, IDC_DEFNOTESSHOWINEDIT, m_ShowNotesInEdit);
  DDX_Radio(pDX, IDC_TREE_DISPLAY_COLLAPSED, m_TreeDisplayStatusAtOpen); // only first!
  DDX_Check(pDX, IDC_ALWAYSONTOP, m_AlwaysOnTop);
  DDX_Check(pDX, IDC_DEFNTSHOWASTIPSINVIEWS, m_ShowNotesAsTipsInViews);
  DDX_Check(pDX, IDC_DEFEXPLORERTREE, m_ExplorerTypeTree);
  DDX_Check(pDX, IDC_DEFNOTESWRAP, m_WordWrapNotes);
  DDX_Check(pDX, IDC_DEFENABLEGRIDLINES, m_EnableGrid);
  DDX_Check(pDX, IDC_PREWARNEXPIRY, m_PreExpiryWarn);
  DDX_Text(pDX, IDC_PREEXPIRYWARNDAYS, m_PreExpiryWarnDays);
  DDX_Check(pDX, IDC_HIGHLIGHTCHANGES, m_HighlightChanges);
  DDX_Radio(pDX, IDC_RST_BLK, m_TrayIconColour); // only first!

  DDX_Control(pDX, IDC_DEFUNSHOWINTREE, m_chkbox[0]);
  DDX_Control(pDX, IDC_DEFPWSHOWINTREE, m_chkbox[1]);
  DDX_Control(pDX, IDC_DEFPWSHOWINEDIT, m_chkbox[2]);
  DDX_Control(pDX, IDC_DEFNOTESSHOWINEDIT,m_chkbox[3]);
  DDX_Control(pDX, IDC_TREE_DISPLAY_COLLAPSED, m_radiobtn[0]);
  DDX_Control(pDX, IDC_TREE_DISPLAY_EXPANDED, m_radiobtn[1]);
  DDX_Control(pDX, IDC_TREE_DISPLAY_LASTSAVE, m_radiobtn[2]);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsDisplay, COptions_PropertyPage)
  //{{AFX_MSG_MAP(COptionsDisplay)
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED(ID_HELP, OnHelp)

  ON_BN_CLICKED(IDC_PREWARNEXPIRY, OnPreWarn)
  ON_BN_CLICKED(IDC_DEFUNSHOWINTREE, OnDisplayUserInTree)
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
    m_chkbox[i].SetBkgColour(COLOR_WINDOW);
  }
  for (int i = 0; i < 3; i++) {
    m_radiobtn[i].SetTextColour(CR_DATABASE_OPTIONS);
    m_radiobtn[i].SetType(BS_AUTORADIOBUTTON);
    m_radiobtn[i].SetBkgColour(COLOR_WINDOW);
  }

  if (m_ShowUsernameInTree == FALSE) {
    m_ShowPasswordInTree = FALSE;
    GetDlgItem(IDC_DEFPWSHOWINTREE)->EnableWindow(FALSE);
  }

  OnPreWarn();
  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PREWARNEXPIRYSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_PREEXPIRYWARNDAYS));
  pspin->SetRange(1, 30);
  pspin->SetBase(10);
  pspin->SetPos(m_PreExpiryWarnDays);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
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
          M_TrayIconColour()          != m_TrayIconColour          ||
          M_HighlightChanges()        != m_HighlightChanges)
        return 1L;
      break;
    case PP_UPDATE_VARIABLES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselfs here first
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
  M_PreExpiryWarnDays() = m_PreExpiryWarnDays;
  M_TreeDisplayStatusAtOpen() = m_TreeDisplayStatusAtOpen;
  M_TrayIconColour() = m_TrayIconColour;

  return COptions_PropertyPage::OnApply();
}

BOOL COptionsDisplay::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return COptions_PropertyPage::PreTranslateMessage(pMsg);
}

BOOL COptionsDisplay::OnKillActive()
{
  CGeneralMsgBox gmb;
  // Check that options, as set, are valid.
  if ((m_PreExpiryWarnDays < 1) || (m_PreExpiryWarnDays > 30)) {
    gmb.AfxMessageBox(IDS_INVALIDEXPIRYWARNDAYS);
    ((CEdit*)GetDlgItem(IDC_PREEXPIRYWARNDAYS))->SetFocus();
    return FALSE;
  }

  return COptions_PropertyPage::OnKillActive();
}

void COptionsDisplay::OnHelp()
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/display_tab.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
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
  //OnCustomDraw in CButtonExtn called only when themes are used, so we need to set colors manually when themes are off
  if (!IsThemeActive()) {
    switch (pWnd->GetDlgCtrlID()) {
      case IDC_DEFUNSHOWINTREE:
      case IDC_DEFPWSHOWINTREE:
      case IDC_DEFPWSHOWINEDIT:
      case IDC_DEFNOTESSHOWINEDIT:
      case IDC_TREE_DISPLAY_COLLAPSED:
      case IDC_TREE_DISPLAY_EXPANDED:
      case IDC_TREE_DISPLAY_LASTSAVE:
        pDC->SetTextColor(CR_DATABASE_OPTIONS);
        pDC->SetBkMode(TRANSPARENT);
        break;
    }
  }

  return hbr;
}
