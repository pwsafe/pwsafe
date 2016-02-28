/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "PasswordSafe.h"
#include "ThisMfcApp.h"
#include "DboxMain.h"

#include "WZPropertySheet.h"
#include "WZPropertyPage.h"
#include "WZAdvanced.h"
#include "WZSelectDB.h"
#include "WZFinish.h"

IMPLEMENT_DYNAMIC(CWZPropertySheet, CPropertySheet)

extern const wchar_t *EYE_CATCHER;

CWZPropertySheet::CWZPropertySheet(UINT nID, CWnd* pParent, WZAdvanced::AdvType iadv_type,
                                   st_SaveAdvValues *pst_SADV)
  : CPropertySheet(nID, pParent), m_nID(nID), m_passkey(L""), m_exportpasskey(L""), m_filespec(L""),
  m_pst_SADV(pst_SADV), m_bAdvanced(false), m_bCompleted(false),
  m_numProcessed(-1)
{
  // common 'other' file processing for Compare, Merge & Synchronize
  UINT uimsgid_select(0), uimsgid_advanced(0), uimsgid_finish(0);
  int idd_select(IDD_WZSELECTDB); // change when a pwsafe is input
  //                                  (merge/compare/sync)

  switch (nID) {
    case ID_MENUITEM_COMPARE:
      uimsgid_select = IDS_PICKCOMPAREFILE;
      uimsgid_advanced = IDS_COMPAREX;
      uimsgid_finish = IDS_WZCOMPARE;
      idd_select = IDD_WZINPUTDB;
      break;
    case ID_MENUITEM_MERGE:
      uimsgid_select = IDS_PICKMERGEFILE;
      uimsgid_advanced = IDS_MERGEX;
      uimsgid_finish = IDS_WZMERGE;
      idd_select = IDD_WZINPUTDB;
      break;
    case ID_MENUITEM_SYNCHRONIZE:
      uimsgid_select = IDS_PICKSYNCHRONIZEEFILE;
      uimsgid_advanced = IDS_SYNCHRONIZEX;
      uimsgid_finish = IDS_WZSYNCH;
      idd_select = IDD_WZINPUTDB;
      break;
    case ID_MENUITEM_EXPORT2PLAINTEXT:
      uimsgid_select = IDS_NAMETEXTFILE;
      uimsgid_advanced = IDS_EXPORT_TEXTX;
      uimsgid_finish = IDS_WZEXPORTTEXT;
      break;
    case ID_MENUITEM_EXPORTENT2PLAINTEXT:
      uimsgid_select = IDS_NAMETEXTFILE;
      uimsgid_advanced = IDS_EXPORT_TEXTX_SINGLE;
      uimsgid_finish = IDS_WZEXPORTTEXT;
      break;
    case ID_MENUITEM_EXPORTGRP2PLAINTEXT:
      uimsgid_select = IDS_NAMETEXTFILE;
      uimsgid_advanced = IDS_EXPORT_TEXTX_GROUP;
      uimsgid_finish = IDS_WZEXPORTTEXT;
      break;
    case ID_MENUITEM_EXPORT2XML:
      uimsgid_select = IDS_NAMEXMLFILE;
      uimsgid_advanced = IDS_EXPORT_XMLX;
      uimsgid_finish = IDS_WZEXPORTXML;
      break;
    case ID_MENUITEM_EXPORTENT2XML:
      uimsgid_select = IDS_NAMEXMLFILE;
      uimsgid_advanced = IDS_EXPORT_XMLX_SINGLE;
      uimsgid_finish = IDS_WZEXPORTXML;
      break;
    case ID_MENUITEM_EXPORTGRP2XML:
      uimsgid_select = IDS_NAMEXMLFILE;
      uimsgid_advanced = IDS_EXPORT_XMLX_GROUP;
      uimsgid_finish = IDS_WZEXPORTXML;
      break;
    case ID_MENUITEM_EXPORTENT2DB:
      uimsgid_select = IDS_NAMEDBFILE;
      uimsgid_finish = IDS_WZEXPORTDB;
      break;
    case ID_MENUITEM_EXPORTGRP2DB:
      uimsgid_select = IDS_NAMEDBFILE;
      uimsgid_finish = IDS_WZEXPORTDB;
      break;
    default:
      ASSERT(0);
  }

  // Setup up wizard property pages
  m_nButtonID = uimsgid_finish;
  m_pp_selectdb = new CWZSelectDB(this, idd_select, uimsgid_select,
          CWZPropertyPage::START);
  AddPage(m_pp_selectdb);

  if (nID != ID_MENUITEM_EXPORTENT2DB && nID != ID_MENUITEM_EXPORTGRP2DB) {
    m_pp_advanced = new CWZAdvanced(this, uimsgid_advanced, CWZPropertyPage::PENULTIMATE,
                                    iadv_type, m_pst_SADV);
    AddPage(m_pp_advanced);
  } else {
    // No Advanced property page when exporting to current DB format
    m_pp_advanced = NULL;
  }

  m_pp_finish   = new CWZFinish(this, uimsgid_finish, CWZPropertyPage::LAST);
  AddPage(m_pp_finish);

  SetWizardMode();
}

CWZPropertySheet::~CWZPropertySheet()
{
  delete m_pp_selectdb;
  delete m_pp_advanced;
  delete m_pp_finish;
}

BEGIN_MESSAGE_MAP(CWZPropertySheet, CPropertySheet)
  //{{AFX_MSG_MAP(CWZPropertySheet)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CWZPropertySheet::PreSubclassWindow() 
{
  if(m_hWnd != NULL) {
    // First get the current Window Styles
    LONG lStyle = GetWindowLong(m_hWnd, GWL_STYLE);

    // Remove the SYSMENU to have a close button
    lStyle &= ~WS_SYSMENU;
                        
    //Now set the Modified Window Style
    SetWindowLong(m_hWnd, GWL_STYLE, lStyle);  
  }
  CPropertySheet::PreSubclassWindow();
}

LRESULT CWZPropertySheet::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  if (app.GetMainDlg()->m_eye_catcher != NULL &&
      wcscmp(app.GetMainDlg()->m_eye_catcher, EYE_CATCHER) == 0) {
    app.GetMainDlg()->ResetIdleLockCounter(message);
  } else
    pws_os::Trace(L"CWZPropertySheet::WindowProc - couldn't find DboxMain ancestor\n");

  return CPropertySheet::WindowProc(message, wParam, lParam);
}

INT_PTR CWZPropertySheet::DoModal()
{
  bool bAccEn = app.IsAcceleratorEnabled();
  if (bAccEn)
    app.DisableAccelerator();

  CPWDialog::GetDialogTracker()->AddOpenDialog(this);
  INT_PTR rc = CPropertySheet::DoModal();
  CPWDialog::GetDialogTracker()->RemoveOpenDialog(this);

  if (bAccEn)
    if (bAccEn)app.EnableAccelerator();

  return rc;
}

BOOL CWZPropertySheet::PreTranslateMessage(MSG *pMsg)
{
  if (pMsg->message == WM_KEYDOWN) {
    if (pMsg->wParam == VK_F1) {
      CWZPropertyPage *pp = (CWZPropertyPage *)GetActivePage();
      pp->PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
      return TRUE;
    }

    if (pMsg->wParam == VK_ESCAPE) {
      PostMessage(WM_COMMAND, MAKELONG(IDCANCEL, BN_CLICKED), NULL);
      return TRUE;
    }
  }

  return CPropertySheet::PreTranslateMessage(pMsg);
}
