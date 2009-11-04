/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "corelib\pwsprefs.h"

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

IMPLEMENT_DYNCREATE(COptionsDisplay, COptions_PropertyPage)

COptionsDisplay::COptionsDisplay()
  : COptions_PropertyPage(COptionsDisplay::IDD)
{
  //{{AFX_DATA_INIT(COptionsDisplay)
  //}}AFX_DATA_INIT
}

COptionsDisplay::~COptionsDisplay()
{
}

void COptionsDisplay::DoDataExchange(CDataExchange* pDX)
{
  COptions_PropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsDisplay)
  DDX_Check(pDX, IDC_ALWAYSONTOP, m_alwaysontop);
  DDX_Check(pDX, IDC_DEFUNSHOWINTREE, m_showusernameintree);
  DDX_Check(pDX, IDC_DEFPWSHOWINTREE, m_showpasswordintree);
  DDX_Check(pDX, IDC_DEFNTSHOWASTIPSINVIEWS, m_shownotesastipsinviews);
  DDX_Check(pDX, IDC_DEFEXPLORERTREE, m_explorertree);
  DDX_Check(pDX, IDC_DEFPWSHOWINEDIT, m_pwshowinedit);
  DDX_Check(pDX, IDC_DEFNOTESSHOWINEDIT, m_notesshowinedit);
  DDX_Check(pDX, IDC_DEFNOTESWRAP, m_wordwrapnotes);
  DDX_Check(pDX, IDC_DEFENABLEGRIDLINES, m_enablegrid);
  DDX_Check(pDX, IDC_PREWARNEXPIRY, m_preexpirywarn);
  DDX_Text(pDX, IDC_PREEXPIRYWARNDAYS, m_preexpirywarndays);
  DDX_Check(pDX, IDC_HIGHLIGHTCHANGES, m_highlightchanges);
#if defined(POCKET_PC)
  DDX_Check(pDX, IDC_DCSHOWSPASSWORD, m_dcshowspassword);
#endif
  DDX_Radio(pDX, IDC_TREE_DISPLAY_COLLAPSED, m_treedisplaystatusatopen); // only first!
  DDX_Radio(pDX, IDC_RST_BLK, m_trayiconcolour); // only first!
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsDisplay, COptions_PropertyPage)
  //{{AFX_MSG_MAP(COptionsDisplay)
  ON_BN_CLICKED(IDC_PREWARNEXPIRY, OnPreWarn)
  ON_BN_CLICKED(IDC_DEFUNSHOWINTREE, OnDisplayUserInTree)
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDisplay message handlers

void COptionsDisplay::OnPreWarn() 
{
  BOOL enable = (((CButton*)GetDlgItem(IDC_PREWARNEXPIRY))->GetCheck() == 1) ? TRUE : FALSE;
  GetDlgItem(IDC_PREWARNEXPIRYSPIN)->EnableWindow(enable);
  GetDlgItem(IDC_PREEXPIRYWARNDAYS)->EnableWindow(enable);
}

BOOL COptionsDisplay::OnInitDialog() 
{
  COptions_PropertyPage::OnInitDialog();

  OnPreWarn();
  CSpinButtonCtrl* pspin = (CSpinButtonCtrl *)GetDlgItem(IDC_PREWARNEXPIRYSPIN);

  pspin->SetBuddy(GetDlgItem(IDC_PREEXPIRYWARNDAYS));
  pspin->SetRange(1, 30);
  pspin->SetBase(10);
  pspin->SetPos(m_preexpirywarndays);
  if (m_showusernameintree == FALSE) {
    m_showpasswordintree = FALSE;
    GetDlgItem(IDC_DEFPWSHOWINTREE)->EnableWindow(FALSE);
  }

  m_savealwaysontop = m_alwaysontop;
  m_saveshowusernameintree = m_showusernameintree;
  m_saveshowpasswordintree = m_showpasswordintree;
  m_saveshownotesastipsinviews = m_shownotesastipsinviews;
  m_saveexplorertree = m_explorertree;
  m_saveenablegrid = m_enablegrid;
  m_savepwshowinedit = m_pwshowinedit;
  m_savenotesshowinedit = m_notesshowinedit;
  m_savewordwrapnotes = m_wordwrapnotes;
  m_savepreexpirywarn = m_preexpirywarn;
  m_savetreedisplaystatusatopen = m_treedisplaystatusatopen;
  m_savepreexpirywarndays = m_preexpirywarndays;
  m_savetrayiconcolour = m_trayiconcolour;

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL COptionsDisplay::OnKillActive()
{
  CGeneralMsgBox gmb;
  // Check that options, as set, are valid.
  if ((m_preexpirywarndays < 1) || (m_preexpirywarndays > 30)) {
    gmb.AfxMessageBox(IDS_INVALIDEXPIRYWARNDAYS);
    ((CEdit*)GetDlgItem(IDC_PREEXPIRYWARNDAYS))->SetFocus();
    return FALSE;
  }

  return COptions_PropertyPage::OnKillActive();
}

void COptionsDisplay::OnDisplayUserInTree()
{
  if (((CButton*)GetDlgItem(IDC_DEFUNSHOWINTREE))->GetCheck() != 1) {
    GetDlgItem(IDC_DEFPWSHOWINTREE)->EnableWindow(FALSE);
    m_showpasswordintree = FALSE;
    ((CButton*)GetDlgItem(IDC_DEFPWSHOWINTREE))->SetCheck(BST_UNCHECKED);
  } else
    GetDlgItem(IDC_DEFPWSHOWINTREE)->EnableWindow(TRUE);
}

LRESULT COptionsDisplay::OnQuerySiblings(WPARAM wParam, LPARAM )
{
  UpdateData(TRUE);

  // Have any of my fields been changed?
  switch (wParam) {
    case PP_DATA_CHANGED:
      if (m_savealwaysontop             != m_alwaysontop             ||
          m_saveshowusernameintree      != m_showusernameintree      ||
          m_saveshowpasswordintree      != m_showpasswordintree      ||
          m_saveshownotesastipsinviews  != m_shownotesastipsinviews  ||
          m_saveexplorertree            != m_explorertree            ||
          m_saveenablegrid              != m_enablegrid              ||
          m_savepwshowinedit            != m_pwshowinedit            ||
          m_savenotesshowinedit         != m_notesshowinedit         ||
          m_savewordwrapnotes           != m_wordwrapnotes           ||
          m_savepreexpirywarn           != m_preexpirywarn           ||
          (m_preexpirywarn              == TRUE &&
           m_savepreexpirywarndays      != m_preexpirywarndays)     ||
          m_savetreedisplaystatusatopen != m_treedisplaystatusatopen ||
          m_savetrayiconcolour          != m_trayiconcolour)
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
