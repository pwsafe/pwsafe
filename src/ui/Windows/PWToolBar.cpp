/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// PWToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "PWToolBar.h"
#include "resource.h"
#include "resource2.h"

#include <map>
#include <algorithm>
#include <iterator>

// CPWToolBarX

/*
  To add a new Toolbar button to this class:
  1. Design new bitmaps (1 x 'Classic', 1 x 'New' designs & 1 x 'New Disabled' 
     design).  All have a background colour of RGB(192, 192, 192).  Note: Create
     the 'New Disabled' from the 'New' by using a program to convert the bitmap
     to 8-bit greyscale.
  2. Add them to PaswordSafe.rc as BITMAPs
  3. Assign new resource Bitmap IDs to these i.e. "IDB_<new name>_CLASSIC",
     "IDB_<new name>_NEW" and "IDB_<new name>_NEW_D"
  4. Assign a new resource ID for the corresponding button e.g. 
     "ID_TOOLBUTTON_<new name>" or "ID_MENUITEM_<name>" if also on a Menu.
  5. Add a new record in the MainGuiInfo or OtherGuiInfo array, the latter if
     the command is on the menu only (not on the toolbar). The record contains
     a text name for the command, the command ID and bitmap resource IDs as defined above.
  6. Add the new resource ID ("ID_TOOLBUTTON_<new name>" or "ID_MENUITEM_<name>")
     in PasswordSafe.rc2 "Toolbar Tooltips" section as these are used during ToolBar
     customization to describe the button in the standard Customization dialog.
    
  NOTE: In message handlers, the toolbar control ALWAYS asks for information based 
  on the ORIGINAL configuration!!! This is not documented by MS.
  
*/

// Names are for the Default toolbar up to HELP - buttons and separators.
// It should really be in PWSprefs but this is the only routine that uses it and
// it is best to keep it together.  These strings should NOT be translated to other
// languages as they are used only in the configuration file.
// Note a separator is denoted by '~'

const CPWToolBarX::GuiRecord CPWToolBarX::MainGuiInfo[] =
  // CString name; UINT ID, classicBM, newBM, disBM
  {
    {L"new", ID_MENUITEM_NEW, IDB_NEW_CLASSIC, IDB_NEW_NEW, IDB_NEW_NEW_D},
    {L"open", ID_MENUITEM_OPEN, IDB_OPEN_CLASSIC, IDB_OPEN_NEW, IDB_OPEN_NEW_D},
    {L"close", ID_MENUITEM_CLOSE, IDB_CLOSE_CLASSIC, IDB_CLOSE_NEW, IDB_CLOSE_NEW_D},
    {L"lock", ID_MENUITEM_LOCK, IDB_LOCK_CLASSIC, IDB_TRAYLOCK_NEW, IDB_TRAYLOCK_NEW_D},
    {L"save", ID_MENUITEM_SAVE, IDB_SAVE_CLASSIC, IDB_SAVE_NEW, IDB_SAVE_NEW_D},
    {L"~", ID_SEPARATOR, 0, 0, 0},
    {L"copypassword", ID_MENUITEM_COPYPASSWORD, IDB_COPYPASSWORD_CLASSIC, IDB_COPYPASSWORD_NEW, IDB_COPYPASSWORD_NEW_D},
    {L"copyuser", ID_MENUITEM_COPYUSERNAME, IDB_COPYUSER_CLASSIC, IDB_COPYUSER_NEW, IDB_COPYUSER_NEW_D},
    {L"copynotes", ID_MENUITEM_COPYNOTESFLD, IDB_COPYNOTES_CLASSIC, IDB_COPYNOTES_NEW, IDB_COPYNOTES_NEW_D},
    {L"clearclipboard", ID_MENUITEM_CLEARCLIPBOARD, IDB_CLEARCLIPBOARD_CLASSIC, IDB_CLEARCLIPBOARD_NEW, IDB_CLEARCLIPBOARD_NEW_D},
    {L"~", ID_SEPARATOR, 0, 0, 0},
    {L"autotype", ID_MENUITEM_AUTOTYPE, IDB_AUTOTYPE_CLASSIC, IDB_AUTOTYPE_NEW, IDB_AUTOTYPE_NEW_D},
    {L"browseurl", ID_MENUITEM_BROWSEURL, IDB_BROWSEURL_CLASSIC, IDB_BROWSEURL_NEW, IDB_BROWSEURL_NEW_D},
    {L"~", ID_SEPARATOR, 0, 0, 0},
    {L"add", ID_MENUITEM_ADD, IDB_ADD_CLASSIC, IDB_ADD_NEW, IDB_ADD_NEW_D},
    {L"viewedit", ID_MENUITEM_EDITENTRY, IDB_VIEWEDIT_CLASSIC, IDB_VIEWEDIT_NEW, IDB_VIEWEDIT_NEW_D},
    {L"~", ID_SEPARATOR, 0, 0, 0},
    {L"delete", ID_MENUITEM_DELETEENTRY, IDB_DELETE_CLASSIC, IDB_DELETE_NEW, IDB_DELETE_NEW_D},
    {L"~", ID_SEPARATOR, 0, 0, 0},
    {L"expandall", ID_MENUITEM_EXPANDALL, IDB_EXPANDALL_CLASSIC, IDB_EXPANDALL_NEW, IDB_EXPANDALL_NEW_D},
    {L"collapseall", ID_MENUITEM_COLLAPSEALL, IDB_COLLAPSEALL_CLASSIC, IDB_COLLAPSEALL_NEW, IDB_COLLAPSEALL_NEW_D},
    {L"~", ID_SEPARATOR, 0, 0, 0},
    {L"options", ID_MENUITEM_OPTIONS, IDB_OPTIONS_CLASSIC, IDB_OPTIONS_NEW, IDB_OPTIONS_NEW_D},
    {L"~", ID_SEPARATOR, 0, 0, 0},
    {L"help", ID_HELP, IDB_HELP_CLASSIC, IDB_HELP_NEW, IDB_HELP_NEW_D},
    // Optional (non-default) buttons from here on:
    // Following are not in the "default" toolbar but can be selected by the user
    {L"exporttext", ID_MENUITEM_EXPORT2PLAINTEXT, IDB_EXPORTTEXT_CLASSIC, IDB_EXPORTTEXT_NEW, IDB_EXPORTTEXT_NEW_D },
    {L"exportxml", ID_MENUITEM_EXPORT2XML, IDB_EXPORTXML_CLASSIC, IDB_EXPORTXML_NEW, IDB_EXPORTXML_NEW_D},
    {L"exportV1", ID_MENUITEM_EXPORT2OLD1XFORMAT, IDB_EXPORTV1_CLASSIC, IDB_EXPORTV1_NEW, IDB_EXPORTV1_NEW_D},
    {L"exportV2", ID_MENUITEM_EXPORT2V2FORMAT, IDB_EXPORTV2_CLASSIC, IDB_EXPORTV2_NEW, IDB_EXPORTV2_NEW_D},
    {L"exportV3", ID_MENUITEM_EXPORT2V3FORMAT, IDB_EXPORTV3_CLASSIC, IDB_EXPORTV3_NEW, IDB_EXPORTV3_NEW_D},
    {L"exportV4", ID_MENUITEM_EXPORT2V4FORMAT, IDB_EXPORTV4_CLASSIC, IDB_EXPORTV4_NEW, IDB_EXPORTV4_NEW_D},
    {L"importtext", ID_MENUITEM_IMPORT_PLAINTEXT, IDB_IMPORTTEXT_CLASSIC, IDB_IMPORTTEXT_NEW, IDB_IMPORTTEXT_NEW_D},
    {L"importxml", ID_MENUITEM_IMPORT_XML, IDB_IMPORTXML_CLASSIC, IDB_IMPORTXML_NEW, IDB_IMPORTXML_NEW_D}, 
    {L"saveas", ID_MENUITEM_SAVEAS, IDB_SAVEAS_CLASSIC, IDB_SAVEAS_NEW, IDB_SAVEAS_NEW_D},
    {L"compare", ID_MENUITEM_COMPARE, IDB_COMPARE_CLASSIC, IDB_COMPARE_NEW, IDB_COMPARE_NEW_D},
    {L"merge", ID_MENUITEM_MERGE, IDB_MERGE_CLASSIC, IDB_MERGE_NEW, IDB_MERGE_NEW_D},
    {L"synchronize", ID_MENUITEM_SYNCHRONIZE, IDB_SYNCHRONIZE_CLASSIC, IDB_SYNCHRONIZE_NEW, IDB_SYNCHRONIZE_NEW_D},
    {L"undo", ID_MENUITEM_UNDO, IDB_UNDO_CLASSIC, IDB_UNDO_NEW, IDB_UNDO_NEW_D},
    {L"redo", ID_MENUITEM_REDO, IDB_REDO_CLASSIC, IDB_REDO_NEW, IDB_REDO_NEW_D},
    {L"passwordsubset", ID_MENUITEM_PASSWORDSUBSET, IDB_PASSWORDCHARS_CLASSIC, IDB_PASSWORDCHARS_NEW, IDB_PASSWORDCHARS_NEW_D},
    {L"browse+autotype", ID_MENUITEM_BROWSEURLPLUS, IDB_BROWSEURLPLUS_CLASSIC, IDB_BROWSEURLPLUS_NEW, IDB_BROWSEURLPLUS_NEW_D},
    {L"runcommand", ID_MENUITEM_RUNCOMMAND, IDB_RUNCMD_CLASSIC, IDB_RUNCMD_NEW, IDB_RUNCMD_NEW_D},
    {L"sendemail", ID_MENUITEM_SENDEMAIL, IDB_SENDEMAIL_CLASSIC, IDB_SENDEMAIL_NEW, IDB_SENDEMAIL_NEW_D},
    {L"listtree", ID_TOOLBUTTON_LISTTREE, IDB_LISTTREE_CLASSIC, IDB_LISTTREE_NEW, IDB_LISTTREE_NEW_D},
    {L"find", ID_MENUITEM_FINDELLIPSIS, IDB_FIND_CLASSIC, IDB_FIND_NEW, IDB_FIND_NEW_D},
    {L"viewreports", ID_TOOLBUTTON_VIEWREPORTS, IDB_VIEWREPORTS_CLASSIC, IDB_VIEWREPORTS_NEW, IDB_VIEWREPORTS_NEW_D}, 
    {L"applyfilters", ID_MENUITEM_APPLYFILTER, IDB_APPLYFILTERS_CLASSIC, IDB_APPLYFILTERS_NEW, IDB_APPLYFILTERS_NEW_D},
    {L"clearfilters", ID_MENUITEM_CLEARFILTER, IDB_CLEARFILTERS_CLASSIC, IDB_CLEARFILTERS_NEW, IDB_CLEARFILTERS_NEW_D},
    {L"setfilters", ID_MENUITEM_EDITFILTER, IDB_SETFILTERS_CLASSIC, IDB_SETFILTERS_NEW, IDB_SETFILTERS_NEW_D},
    {L"managefilters", ID_MENUITEM_MANAGEFILTERS, IDB_MANAGEFILTERS_CLASSIC, IDB_MANAGEFILTERS_NEW, IDB_MANAGEFILTERS_NEW_D},
    {L"addgroup", ID_MENUITEM_ADDGROUP, IDB_ADDGROUP_CLASSIC, IDB_ADDGROUP_NEW, IDB_ADDGROUP_NEW_D},
    {L"managepolicies", ID_MENUITEM_PSWD_POLICIES, IDB_PSWD_POLICIES_CLASSIC, IDB_PSWD_POLICIES_NEW, IDB_PSWD_POLICIES_NEW_D},
  };

// Additional Controls NOT on ToolBar and can't be added by the user during customise.
// They are listed here ONLY to provide images for the menu items!
const CPWToolBarX::GuiRecord CPWToolBarX::OtherGuiInfo[] =
  // CString name; UINT ID, classicBM, newBM, disBM
  {
    {L"", ID_MENUITEM_PROPERTIES, IDB_PROPERTIES_CLASSIC, IDB_PROPERTIES_NEW, IDB_PROPERTIES_NEW_D},
    {L"", ID_MENUITEM_GROUPENTER, IDB_GROUPENTER_CLASSIC, IDB_GROUPENTER_NEW, IDB_GROUPENTER_NEW_D},
    {L"", ID_MENUITEM_DUPLICATEENTRY, IDB_DUPLICATE_CLASSIC, IDB_DUPLICATE_NEW, IDB_DUPLICATE_NEW_D},
    {L"", ID_CHANGEFONTMENU, IDB_CHANGEFONTMENU_CLASSIC, IDB_CHANGEFONTMENU_NEW, IDB_CHANGEFONTMENU_NEW_D},
    {L"", ID_MENUITEM_CHANGETREEFONT, IDB_CHANGEFONTMENU_CLASSIC, IDB_CHANGEFONTMENU_NEW, IDB_CHANGEFONTMENU_NEW_D},
    {L"", ID_MENUITEM_CHANGEADDEDITFONT, IDB_CHANGEFONTMENU_CLASSIC, IDB_CHANGEFONTMENU_NEW, IDB_CHANGEFONTMENU_NEW_D},
    {L"", ID_MENUITEM_CHANGEPSWDFONT, IDB_CHANGEPSWDFONTMENU_CLASSIC, IDB_CHANGEPSWDFONTMENU_NEW, IDB_CHANGEPSWDFONTMENU_NEW_D},
    {L"", ID_MENUITEM_REPORT_COMPARE, IDB_COMPARE_CLASSIC, IDB_COMPARE_NEW, IDB_COMPARE_NEW_D},
    {L"", ID_MENUITEM_REPORT_FIND, IDB_FIND_CLASSIC, IDB_FIND_NEW, IDB_FIND_NEW_D},
    {L"", ID_MENUITEM_REPORT_IMPORTTEXT, IDB_IMPORTTEXT_CLASSIC, IDB_IMPORTTEXT_NEW, IDB_IMPORTTEXT_NEW_D},
    {L"", ID_MENUITEM_REPORT_IMPORTXML, IDB_IMPORTXML_CLASSIC, IDB_IMPORTXML_NEW, IDB_IMPORTXML_NEW_D},
    {L"", ID_MENUITEM_REPORT_MERGE, IDB_MERGE_CLASSIC, IDB_MERGE_NEW, IDB_MERGE_NEW_D},
    {L"", ID_MENUITEM_REPORT_SYNCHRONIZE, IDB_SYNCHRONIZE_CLASSIC, IDB_SYNCHRONIZE_NEW, IDB_SYNCHRONIZE_NEW_D},
    {L"", ID_MENUITEM_REPORT_VALIDATE, IDB_VALIDATE_CLASSIC, IDB_VALIDATE_NEW, IDB_VALIDATE_NEW_D},
    {L"", ID_MENUITEM_CHANGECOMBO, IDB_CHANGECOMBO_CLASSIC, IDB_CHANGECOMBO_NEW, IDB_CHANGECOMBO_NEW_D},
    {L"", ID_MENUITEM_BACKUPSAFE, IDB_BACKUPSAFE_CLASSIC, IDB_BACKUPSAFE_NEW, IDB_BACKUPSAFE_NEW_D},
    {L"", ID_MENUITEM_RESTORESAFE, IDB_RESTORE_CLASSIC, IDB_RESTORE_NEW, IDB_RESTORE_NEW_D},
    {L"", ID_MENUITEM_EXIT, IDB_EXIT_CLASSIC, IDB_EXIT_NEW, IDB_EXIT_NEW_D},
    {L"", ID_MENUITEM_ABOUT, IDB_ABOUT_CLASSIC, IDB_ABOUT_NEW, IDB_ABOUT_NEW_D},
    {L"", ID_MENUITEM_TRAYUNLOCK, IDB_TRAYUNLOCK_CLASSIC, IDB_TRAYUNLOCK_NEW, IDB_TRAYUNLOCK_NEW_D},
    {L"", ID_MENUITEM_TRAYLOCK, IDB_TRAYLOCK_CLASSIC, IDB_TRAYLOCK_NEW, IDB_TRAYLOCK_NEW_D},
    {L"", ID_REPORTSMENU, IDB_VIEWREPORTS_CLASSIC, IDB_VIEWREPORTS_NEW, IDB_VIEWREPORTS_NEW_D},
    {L"", ID_MENUITEM_MRUENTRY, IDB_PWSDB, IDB_PWSDB, IDB_PWSDB},
    {L"", ID_EXPORTMENU, IDB_EXPORT_CLASSIC, IDB_EXPORT_NEW, IDB_EXPORT_NEW_D},
    {L"", ID_IMPORTMENU, IDB_IMPORT_CLASSIC, IDB_IMPORT_NEW, IDB_IMPORT_NEW_D},
    {L"", ID_MENUITEM_CREATESHORTCUT, IDB_CREATESHORTCUT_CLASSIC, IDB_CREATESHORTCUT_NEW, IDB_CREATESHORTCUT_NEW_D},
    {L"", ID_MENUITEM_CUSTOMIZETOOLBAR, IDB_CUSTOMIZETBAR_CLASSIC, IDB_CUSTOMIZETBAR_NEW, IDB_CUSTOMIZETBAR_NEW},
    {L"", ID_MENUITEM_VKEYBOARDFONT, IDB_CHANGEVKBDFONTMENU_CLASSIC, IDB_CHANGEVKBDFONTMENU_NEW, IDB_CHANGEVKBDFONTMENU_NEW_D},
    {L"", ID_MENUITEM_COMPVIEWEDIT, IDB_VIEWEDIT_CLASSIC, IDB_VIEWEDIT_NEW, IDB_VIEWEDIT_NEW_D},
    {L"", ID_MENUITEM_COPY_TO_ORIGINAL, IDB_IMPORT_CLASSIC, IDB_IMPORT_NEW, IDB_IMPORT_NEW_D},
    {L"", ID_MENUITEM_EXPORTENT2PLAINTEXT, IDB_EXPORTTEXT_CLASSIC, IDB_EXPORTTEXT_NEW, IDB_EXPORTTEXT_NEW_D},
    {L"", ID_MENUITEM_EXPORTGRP2PLAINTEXT, IDB_EXPORTTEXT_CLASSIC, IDB_EXPORTTEXT_NEW, IDB_EXPORTTEXT_NEW_D},
    {L"", ID_MENUITEM_EXPORTENT2XML, IDB_EXPORTXML_CLASSIC, IDB_EXPORTXML_NEW, IDB_EXPORTXML_NEW_D},
    {L"", ID_MENUITEM_EXPORTGRP2XML, IDB_EXPORTXML_CLASSIC, IDB_EXPORTXML_NEW, IDB_EXPORTXML_NEW_D},
    {L"", ID_MENUITEM_EXPORTENT2DB, IDB_EXPORTDB_CLASSIC, IDB_EXPORTDB_NEW, IDB_EXPORTDB_NEW_D},
    {L"", ID_MENUITEM_EXPORTGRP2DB, IDB_EXPORTDB_CLASSIC, IDB_EXPORTDB_NEW, IDB_EXPORTDB_NEW_D},
    {L"", ID_MENUITEM_EXPORTFILTERED2DB, IDB_EXPORTDB_CLASSIC, IDB_EXPORTDB_NEW, IDB_EXPORTDB_NEW_D},
    {L"", ID_MENUITEM_DUPLICATEGROUP, IDB_DUPLICATEGROUP_CLASSIC, IDB_DUPLICATEGROUP_NEW, IDB_DUPLICATEGROUP_NEW_D},
    {L"", ID_MENUITEM_REPORT_EXPORTTEXT, IDB_EXPORTTEXT_CLASSIC, IDB_EXPORTTEXT_NEW, IDB_EXPORTTEXT_NEW_D},
    {L"", ID_MENUITEM_REPORT_EXPORTXML, IDB_EXPORTXML_CLASSIC, IDB_EXPORTXML_NEW, IDB_EXPORTXML_NEW_D},
    {L"", ID_MENUITEM_REPORT_EXPORTDB, IDB_EXPORTDB_CLASSIC, IDB_EXPORTDB_NEW, IDB_EXPORTDB_NEW_D },
    {L"", ID_MENUITEM_COPYALL_TO_ORIGINAL, IDB_IMPORT_CLASSIC, IDB_IMPORT_NEW, IDB_IMPORT_NEW_D},
    {L"", ID_MENUITEM_SYNCHRONIZEALL, IDB_IMPORT_CLASSIC, IDB_IMPORT_NEW, IDB_IMPORT_NEW_D},
  };

IMPLEMENT_DYNAMIC(CPWToolBarX, CToolBar)

CPWToolBarX::CPWToolBarX()
:  m_bitmode(1), m_iBrowseURL_BM_offset(-1), m_iSendEmail_BM_offset(-1)
{
  m_pOriginalTBinfo = new TBBUTTON[_countof(MainGuiInfo)];
  m_iNum_Bitmaps = _countof(MainGuiInfo) + _countof(OtherGuiInfo);
}

CPWToolBarX::~CPWToolBarX()
{
  delete[] m_pOriginalTBinfo;
}

void CPWToolBarX::OnDestroy()
{
  m_ImageLists[0].DeleteImageList();
  m_ImageLists[1].DeleteImageList();
  m_ImageLists[2].DeleteImageList();
  m_DisabledImageLists[0].DeleteImageList();
  m_DisabledImageLists[1].DeleteImageList();
}

BEGIN_MESSAGE_MAP(CPWToolBarX, CToolBar)
  ON_NOTIFY_REFLECT(TBN_GETBUTTONINFO, OnToolBarGetButtonInfo)
  ON_NOTIFY_REFLECT(TBN_QUERYINSERT, OnToolBarQueryInsert)
  ON_NOTIFY_REFLECT(TBN_QUERYDELETE, OnToolBarQueryDelete)
  ON_NOTIFY_REFLECT(TBN_GETBUTTONINFO, OnToolBarQueryInfo)
  ON_NOTIFY_REFLECT(TBN_RESET, OnToolBarReset)
  ON_WM_DESTROY()
END_MESSAGE_MAP()

// CPWToolBarX message handlers

void CPWToolBarX::RefreshImages()
{
  m_ImageLists[0].DeleteImageList();
  m_ImageLists[1].DeleteImageList();
  m_ImageLists[2].DeleteImageList();
  m_DisabledImageLists[0].DeleteImageList();
  m_DisabledImageLists[1].DeleteImageList();

  Init(m_NumBits, true);

  ChangeImages(m_toolbarMode);
}

void CPWToolBarX::OnToolBarQueryInsert(NMHDR *, LRESULT *pLResult)
{
  *pLResult = TRUE;
}

void CPWToolBarX::OnToolBarQueryDelete(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMTOOLBAR *pNMToolbar = (NMTOOLBAR *)pNotifyStruct;

  if ((pNMToolbar->tbButton.idCommand != ID_SEPARATOR) &&
      GetToolBarCtrl().IsButtonHidden(pNMToolbar->tbButton.idCommand))
    *pLResult = FALSE;
  else
    *pLResult = TRUE;
}

void CPWToolBarX::OnToolBarQueryInfo(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMTOOLBAR *pNMToolbar = (NMTOOLBAR *)pNotifyStruct;

  ASSERT(pNMToolbar->iItem < _countof(MainGuiInfo));

  if ((pNMToolbar->iItem >= 0) &&
      (pNMToolbar->iItem < _countof(MainGuiInfo))) {
    pNMToolbar->tbButton = m_pOriginalTBinfo[pNMToolbar->iItem];
    *pLResult = TRUE;
  } else {
    *pLResult = FALSE;
  }
}

void CPWToolBarX::OnToolBarGetButtonInfo(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMTOOLBAR *pNMToolbar = (NMTOOLBAR *)pNotifyStruct;

  ASSERT(pNMToolbar->iItem <= _countof(MainGuiInfo));

  // if the index is valid
  if ((pNMToolbar->iItem >= 0) && (pNMToolbar->iItem < _countof(MainGuiInfo))) {
    // copy the stored button structure
    pNMToolbar->tbButton = m_pOriginalTBinfo[pNMToolbar->iItem];
    *pLResult = TRUE;
  } else {
    *pLResult = FALSE;
  }
}

void CPWToolBarX::OnToolBarReset(NMHDR *, LRESULT *)
{
  Reset();
}

//  Other routines

void CPWToolBarX::Init(const int NumBits, bool bRefresh)
{
  int i, j;
  const UINT iClassicFlags = ILC_MASK | ILC_COLOR8;
  const UINT iNewFlags1 = ILC_MASK | ILC_COLOR8;
  const UINT iNewFlags2 = ILC_MASK | ILC_COLOR24;

  m_NumBits = NumBits;

  if (NumBits >= 32) {
    m_bitmode = 2;
  }

  m_ImageLists[0].Create(16, 16, iClassicFlags, m_iNum_Bitmaps, 2);
  m_ImageLists[1].Create(16, 16, iNewFlags1, m_iNum_Bitmaps, 2);
  m_ImageLists[2].Create(16, 16, iNewFlags2, m_iNum_Bitmaps, 2);
  m_DisabledImageLists[0].Create(16, 16, iNewFlags1, m_iNum_Bitmaps, 2);
  m_DisabledImageLists[1].Create(16, 16, iNewFlags2, m_iNum_Bitmaps, 2);

  int iNum_Main = _countof(MainGuiInfo);
  int iNum_Other = _countof(OtherGuiInfo);

  for (i = 0; i < iNum_Main; i++) {
    if (MainGuiInfo[i].classicBM == IDB_BROWSEURL_CLASSIC) {
      m_iBrowseURL_BM_offset = i;
      break;
    }
  }

  m_iSendEmail_BM_offset = iNum_Main;  // First of the "Others"

  SetupImageList(MainGuiInfo, &GuiRecord::GetClassicBM, NULL, iNum_Main, 0);
  SetupImageList(OtherGuiInfo, &GuiRecord::GetClassicBM, NULL, iNum_Other, 0);

  SetupImageList(MainGuiInfo, &GuiRecord::GetNewBM, &GuiRecord::GetDisBM, iNum_Main, 1);
  SetupImageList(OtherGuiInfo, &GuiRecord::GetNewBM, &GuiRecord::GetDisBM, iNum_Other, 1);

  SetupImageList(MainGuiInfo, &GuiRecord::GetNewBM, &GuiRecord::GetDisBM, iNum_Main, 2);
  SetupImageList(OtherGuiInfo, &GuiRecord::GetNewBM, &GuiRecord::GetDisBM, iNum_Other, 2);

  if (bRefresh)
    return;

  j = 0;
  m_csDefaultButtonString.Empty();
  m_iNumDefaultButtons = _countof(MainGuiInfo);
  for (i = 0; i < _countof(MainGuiInfo); i++) {
    bool bIsSeparator = MainGuiInfo[i].ID == ID_SEPARATOR;
    BYTE fsStyle = bIsSeparator ? TBSTYLE_SEP : TBSTYLE_BUTTON;
    fsStyle &= ~BTNS_SHOWTEXT;
    if (!bIsSeparator) {
      fsStyle |= TBSTYLE_AUTOSIZE;
    }
    m_pOriginalTBinfo[i].iBitmap = bIsSeparator ? -1 : j;
    m_pOriginalTBinfo[i].idCommand = MainGuiInfo[i].ID;
    m_pOriginalTBinfo[i].fsState = TBSTATE_ENABLED;
    m_pOriginalTBinfo[i].fsStyle = fsStyle;
    m_pOriginalTBinfo[i].dwData = 0;
    m_pOriginalTBinfo[i].iString = bIsSeparator ? -1 : j;

    if (i <= m_iNumDefaultButtons)
      m_csDefaultButtonString += MainGuiInfo[i].name + L" ";

    if (MainGuiInfo[i].ID == ID_HELP)
      m_iNumDefaultButtons = i;

    if (!bIsSeparator)
      j++;
  }
}

void CPWToolBarX::CustomizeButtons(CString csButtonNames)
{
  if (csButtonNames.IsEmpty()) {
    // Add all buttons
    Reset();
    return;
  }

  int i, nCount;
  CToolBarCtrl& tbCtrl = GetToolBarCtrl();

  // Remove all of the existing buttons
  nCount = tbCtrl.GetButtonCount();

  for (i = nCount - 1; i >= 0; i--) {
    tbCtrl.DeleteButton(i);
  }

  csButtonNames.MakeLower();
  int curPos(0);
  // Note all separators will be treated as the first!
  i = 0;
  CString csToken = csButtonNames.Tokenize(L" ", curPos);
  while (csToken != L"" && curPos != -1) {
    GuiInfoFinder finder(csToken);
    const GuiRecord *iter = std::find_if(MainGuiInfo,
                                         MainGuiInfo + _countof(MainGuiInfo), finder);
    if (iter != MainGuiInfo + _countof(MainGuiInfo)) {
      int index = int(std::distance(MainGuiInfo, iter));
      tbCtrl.AddButtons(1, &m_pOriginalTBinfo[index]);
    }
    csToken = csButtonNames.Tokenize(L" ", curPos);
  }

  tbCtrl.AutoSize();
}

CString CPWToolBarX::GetButtonString() const
{
  CString cs_buttonnames(L"");
  TBBUTTONINFO tbinfo;
  int num_buttons, i;

  CToolBarCtrl& tbCtrl = GetToolBarCtrl();

  num_buttons = tbCtrl.GetButtonCount();

  SecureZeroMemory(&tbinfo, sizeof(tbinfo));
  tbinfo.cbSize = sizeof(tbinfo);
  tbinfo.dwMask = TBIF_BYINDEX | TBIF_COMMAND | TBIF_STYLE;

  for (i = 0; i < num_buttons; i++) {
    tbCtrl.GetButtonInfo(i, &tbinfo);

    if (tbinfo.fsStyle & TBSTYLE_SEP) {
      cs_buttonnames += L"~ ";
      continue;
    }

    // Change control ID as only ID_MENUITEM_EDITENTRY is in MainGuiInfo
    // but we may have changed the button ID to ID_MENUITEM_VIEWENTRY
    UINT nID = tbinfo.idCommand;
    if (nID == ID_MENUITEM_VIEWENTRY)
      nID = ID_MENUITEM_EDITENTRY;

    int index = -1;
    for (int j = 0; j < _countof(MainGuiInfo); j++) {
      if (MainGuiInfo[j].ID == nID) {
        index = j;
        break;
      }
    }
    ASSERT(index != -1);
    cs_buttonnames += MainGuiInfo[index].name + L" ";
  }

  if (cs_buttonnames.CompareNoCase(m_csDefaultButtonString) == 0) {
    cs_buttonnames.Empty();
  }

  return cs_buttonnames;
}

void CPWToolBarX::Reset()
{
  int nCount, i;
  CToolBarCtrl& tbCtrl = GetToolBarCtrl();

  // Remove all of the existing buttons
  nCount = tbCtrl.GetButtonCount();

  for (i = nCount - 1; i >= 0; i--) {
    tbCtrl.DeleteButton(i);
  }

  // Restore the buttons
  for (i = 0; i <= m_iNumDefaultButtons; i++) {
    tbCtrl.AddButtons(1, &m_pOriginalTBinfo[i]);
  }

  tbCtrl.AutoSize();
}

void CPWToolBarX::ChangeImages(const int toolbarMode)
{
  CToolBarCtrl& tbCtrl = GetToolBarCtrl();
  m_toolbarMode = toolbarMode;
  const int nImageListNum = (m_toolbarMode == ID_MENUITEM_OLD_TOOLBAR) ? 0 : m_bitmode;
  tbCtrl.SetImageList(&m_ImageLists[nImageListNum]);

  // We only do the New toolbar disabled images.  MS can handle the Classic OK
  if (nImageListNum != 0)
    tbCtrl.SetDisabledImageList(&m_DisabledImageLists[nImageListNum - 1]);
  else
    tbCtrl.SetDisabledImageList(NULL);
}

void CPWToolBarX::LoadDefaultToolBar(const int toolbarMode)
{
  int nCount, i, j;
  CToolBarCtrl& tbCtrl = GetToolBarCtrl();
  nCount = tbCtrl.GetButtonCount();

  for (i = nCount - 1; i >= 0; i--) {
    tbCtrl.DeleteButton(i);
  }

  m_toolbarMode = toolbarMode;
  const int nImageListNum = (m_toolbarMode == ID_MENUITEM_OLD_TOOLBAR) ? 0 : m_bitmode;
  tbCtrl.SetImageList(&m_ImageLists[nImageListNum]);
  // We only do the New toolbar disabled images.  MS can handle the Classic OK
  if (nImageListNum != 0)
    tbCtrl.SetDisabledImageList(&m_DisabledImageLists[nImageListNum - 1]);
  else
    tbCtrl.SetDisabledImageList(NULL);

  // Create text for customization dialog using button tooltips.
  // Assume no button tooltip description exceeds 64 characters, also _countof(MainGuiInfo)
  // includes separators which don't have strings giving an even bigger buffer!
  // Because they are a concatenation of null terminated strings terminated by a double
  // null, they cannot be stored in a CString variable,
  wchar_t *lpszTBCustomizationStrings = new wchar_t[_countof(MainGuiInfo) * 64];
  const int maxlength = _countof(MainGuiInfo) * 64;

  // By clearing, ensures string ends with a double NULL
  SecureZeroMemory(lpszTBCustomizationStrings, maxlength * sizeof(wchar_t));

  j = 0;
  for (i = 0; i < _countof(MainGuiInfo); i++) {
    if (MainGuiInfo[i].ID != ID_SEPARATOR) {
      CString cs_buttondesc;
      cs_buttondesc.LoadString(MainGuiInfo[i].ID);
      int iPos = cs_buttondesc.ReverseFind(L'\n');
      if (iPos < 0) // could happen with incomplete translation
        continue;
      cs_buttondesc = cs_buttondesc.Right(cs_buttondesc.GetLength() - iPos - 1);
      int idesclen = cs_buttondesc.GetLength();
      wchar_t *szDescription = cs_buttondesc.GetBuffer(idesclen);
      memcpy_s(&lpszTBCustomizationStrings[j], maxlength - j, szDescription, 
               idesclen * sizeof(wchar_t));
      cs_buttondesc.ReleaseBuffer();
      j += idesclen + 1;
    }
  }

  tbCtrl.AddStrings(lpszTBCustomizationStrings);
  tbCtrl.AddButtons(_countof(MainGuiInfo), &m_pOriginalTBinfo[0]);

  delete [] lpszTBCustomizationStrings;

  DWORD dwStyle, dwStyleEx;
  dwStyle = tbCtrl.GetStyle();
  dwStyle &= ~TBSTYLE_AUTOSIZE;
  tbCtrl.SetStyle(dwStyle | TBSTYLE_LIST);

  dwStyleEx = tbCtrl.GetExtendedStyle();
  tbCtrl.SetExtendedStyle(dwStyleEx | TBSTYLE_EX_MIXEDBUTTONS);
}

void CPWToolBarX::MapControlIDtoImage(ID2ImageMap &IDtoImages)
{
  int i, j(0);
  for (i = 0; i < _countof(MainGuiInfo); i++) {
    UINT ID = MainGuiInfo[i].ID;
    if (ID == ID_SEPARATOR)
      continue;
    IDtoImages[ID] = j;
    j++;
  }

  int iNum_OtherIDs  = _countof(OtherGuiInfo);
  for (i = 0; i < iNum_OtherIDs; i++) {
    UINT ID = OtherGuiInfo[i].ID;
    IDtoImages[ID] = j;
    j++;
  }

  // Delete Group has same image as Delete Entry
  ID2ImageMapIter iter;
  iter = IDtoImages.find(ID_MENUITEM_DELETEENTRY);
  if (iter != IDtoImages.end()) {
    IDtoImages[ID_MENUITEM_DELETEGROUP] = iter->second;
  }
  // View has same image as Edit
  iter = IDtoImages.find(ID_MENUITEM_EDITENTRY);
  if (iter != IDtoImages.end()) {
    IDtoImages[ID_MENUITEM_VIEWENTRY] = iter->second;
  }
}

void CPWToolBarX::SetupImageList(const GuiRecord *guiInfo,
                                 GuiRecordGetter GetBM, GuiRecordGetter GetDisBM,
                                 const int numBMs, const int nImageList)
{
  // See http://www.parashift.com/c++-faq/macro-for-ptr-to-memfn.html
#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

  const COLORREF crCOLOR_3DFACE = GetSysColor(COLOR_3DFACE);

  CBitmap bmNormal, bmDisabled;

  for (int i = 0; i < numBMs; i++) {
    UINT bmID = CALL_MEMBER_FN(guiInfo[i], GetBM)();
    if (bmID == 0)
      continue; // skip over separator

    VERIFY(bmNormal.Attach(::LoadImage(::AfxFindResourceHandle(MAKEINTRESOURCE(bmID), RT_BITMAP),
                                       MAKEINTRESOURCE(bmID), IMAGE_BITMAP, 0, 0,
                                       (LR_DEFAULTSIZE | LR_CREATEDIBSECTION))));
    SetBitmapBackground(bmNormal, crCOLOR_3DFACE);
    m_ImageLists[nImageList].Add(&bmNormal, crCOLOR_3DFACE);
    bmNormal.DeleteObject();

    if (nImageList != 0) {
      bmID = CALL_MEMBER_FN(guiInfo[i], GetDisBM)();
      bmDisabled.Attach(::LoadImage(::AfxFindResourceHandle(MAKEINTRESOURCE(bmID), RT_BITMAP),
                                    MAKEINTRESOURCE(bmID), IMAGE_BITMAP, 0, 0,
                                    (LR_DEFAULTSIZE | LR_CREATEDIBSECTION)));
      SetBitmapBackground(bmDisabled, crCOLOR_3DFACE);
      m_DisabledImageLists[nImageList - 1].Add(&bmDisabled, crCOLOR_3DFACE);
      bmDisabled.DeleteObject();
    }
  }
}

void CPWToolBarX::SetBitmapBackground(CBitmap &bm, const COLORREF newbkgrndColour)
{
  // Get how many pixels in the bitmap
  BITMAP bmInfo;
  bm.GetBitmap(&bmInfo);

  const UINT numPixels(bmInfo.bmHeight * bmInfo.bmWidth);

  // get a pointer to the pixels
  DIBSECTION ds;
  VERIFY(bm.GetObject(sizeof(DIBSECTION), &ds) == sizeof(DIBSECTION));

  RGBTRIPLE *pixels = reinterpret_cast<RGBTRIPLE*>(ds.dsBm.bmBits);
  ASSERT(pixels != NULL);

  const RGBTRIPLE newbkgrndColourRGB = {GetBValue(newbkgrndColour),
    GetGValue(newbkgrndColour),
    GetRValue(newbkgrndColour)};

  for (UINT i = 0; i < numPixels; ++i) {
    if (pixels[i].rgbtBlue  == 192 &&
        pixels[i].rgbtGreen == 192 &&
        pixels[i].rgbtRed   == 192) {
      pixels[i] = newbkgrndColourRGB;
    }
  }
}
