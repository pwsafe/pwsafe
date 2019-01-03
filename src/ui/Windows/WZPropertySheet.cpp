/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include <map>
#include <tuple>

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
  enum {SELECT_IDS = 0, ADVANCED_IDS = 1, FINISH_IDS = 2, SELECT_IDD = 3};
  std::map<UINT, std::tuple<UINT, UINT, UINT, UINT>> specifics = {
    {ID_MENUITEM_COMPARE, {IDS_PICKCOMPAREFILE, IDS_COMPAREX, IDS_WZCOMPARE, IDD_WZINPUTDB}},
    {ID_MENUITEM_MERGE, {IDS_PICKMERGEFILE, IDS_MERGEX, IDS_WZMERGE, IDD_WZINPUTDB}},
    {ID_MENUITEM_SYNCHRONIZE, {IDS_PICKSYNCHRONIZEEFILE, IDS_SYNCHRONIZEX, IDS_WZSYNCH, IDD_WZINPUTDB}},
    {ID_MENUITEM_EXPORT2PLAINTEXT, {IDS_NAMETEXTFILE, IDS_EXPORT_TEXTX, IDS_WZEXPORTTEXT, IDD_WZSELECTDB}},
    {ID_MENUITEM_EXPORTENT2PLAINTEXT, {IDS_NAMETEXTFILE, IDS_EXPORT_TEXTX_SINGLE, IDS_WZEXPORTTEXT, IDD_WZSELECTDB}},
    {ID_MENUITEM_EXPORTGRP2PLAINTEXT, {IDS_NAMETEXTFILE, IDS_EXPORT_TEXTX_GROUP, IDS_WZEXPORTTEXT, IDD_WZSELECTDB}},
    {ID_MENUITEM_EXPORT2XML, {IDS_NAMEXMLFILE, IDS_EXPORT_XMLX, IDS_WZEXPORTXML, IDD_WZSELECTDB}},
    {ID_MENUITEM_EXPORTENT2XML, {IDS_NAMEXMLFILE, IDS_EXPORT_XMLX_SINGLE, IDS_WZEXPORTXML, IDD_WZSELECTDB}},
    {ID_MENUITEM_EXPORTGRP2XML, {IDS_NAMEXMLFILE, IDS_EXPORT_XMLX_GROUP, IDS_WZEXPORTXML, IDD_WZSELECTDB}},
    {ID_MENUITEM_EXPORTENT2DB, {IDS_NAMEDBFILE, 0, IDS_WZEXPORTDB, IDD_WZSELECTDB}},
    {ID_MENUITEM_EXPORTGRP2DB, {IDS_NAMEDBFILE, 0, IDS_WZEXPORTDB, IDD_WZSELECTDB}},
    {ID_MENUITEM_EXPORTFILTERED2DB, {IDS_NAMEDBFILE, 0, IDS_WZEXPORTDB, IDD_WZSELECTDB}},
  };
  
  // Setup up wizard property pages
  m_nButtonID = std::get<FINISH_IDS>(specifics[nID]);
  m_pp_selectdb = new CWZSelectDB(this, std::get<SELECT_IDD>(specifics[nID]),
                                  std::get<SELECT_IDS>(specifics[nID]), CWZPropertyPage::START);
  AddPage(m_pp_selectdb);

  if (nID != ID_MENUITEM_EXPORTENT2DB && nID != ID_MENUITEM_EXPORTGRP2DB &&
      nID != ID_MENUITEM_EXPORTFILTERED2DB) {
    m_pp_advanced = new CWZAdvanced(this, std::get<ADVANCED_IDS>(specifics[nID]),
                                    CWZPropertyPage::PENULTIMATE, iadv_type, m_pst_SADV);
    AddPage(m_pp_advanced);
  } else { // No Advanced property page when exporting to current DB format
    m_pp_advanced = NULL;
  }

  m_pp_finish   = new CWZFinish(this, std::get<FINISH_IDS>(specifics[nID]), CWZPropertyPage::LAST);
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
