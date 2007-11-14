/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include <vector>
#include <algorithm>

// CPWToolBar

/*
  To add a new Toolbar button to this class:
  1. Design new bitmaps ('Classic' and 2 x 'New' designs - ILC_COLOR8 and ILC_COLOR32).  ILC_COLOR8 has a
     background colour of RGB(192, 192, 192) and ILC_COLOR32 one of RGB(196, 196, 196) due to MS!
  2. Add them to PaswordSafe.rc as BITMAPs
  3. Assign new resource Bitmap IDs to these i.e. "IDB_<new name>_CLASSIC", "IDB_<new name>_NEW8" &
     "IDB_<new name>_NEW32"
  4. Assign a new resource ID for the corresponding button e.g. "ID_TOOLBUTTON_<new name>"
  5. Add the resource ID in the appropriate place in the m_MainToolBarIDs array
  6. Add the new bitmap IDs in the appropriate place in m_MainToolBarClassicBMs & m_MainToolBarNewBMs arrays
     (these should correspond to the position of the "resource ID" in step 4 (ignoring separators)).
  7. Add the new name in the appropriate place in the m_csMainButtons array (used for customization/preferences
     and '~' represents a separator).
  8. Add the new resource ID ("ID_TOOLBUTTON_<new name>") in PasswordSafe.rc2 "Toolbar Tooltips" section
     as these are used during ToolBar customization to describe the button in the standard Customization dialog.

 NOTE: In message handlers, the toolbar control ALWAYS asks for information based on the ORIGINAL configuration!!!
 This is not documented by MS.

*/

// The following is the Default toolbar up to HELP - buttons and separators.
// It should really be in PWSprefs but this is the only routine that uses it and
// it is best to keep it together.  These strings should NOT be translated to other
// languagues as they are used only in the configuration file.
// They should match m_MainToolBarIDs below.
// Note a separator is denoted by '~'
const CString CPWToolBar::m_csMainButtons[] = {
  _T("new"), _T("open"), _T("close"), _T("save"), _T("~"),
  _T("copypassword"), _T("copyuser"), _T("copynotes"), _T("clearclipboard"), _T("~"),
  _T("autotype"), _T("browseurl"), _T("~"),
  _T("add"), _T("viewedit"), _T("~"),
  _T("delete"), _T("~"),
  _T("expandall"), _T("collapseall"), _T("~"),
  _T("options"), _T("~"),
  _T("help"),
  // Optional (non-default) buttons next
  _T("exporttext"), _T("exportxml"), _T("importtext"), _T("importxml"), 
  _T("saveas"), _T("compare"), _T("merge"), _T("listtree"), _T("viewreports")
};

const UINT CPWToolBar::m_MainToolBarIDs[] = {
  ID_TOOLBUTTON_NEW,
  ID_TOOLBUTTON_OPEN,
  ID_TOOLBUTTON_CLOSE,
  ID_TOOLBUTTON_SAVE,
  ID_SEPARATOR,
  ID_TOOLBUTTON_COPYPASSWORD,
  ID_TOOLBUTTON_COPYUSERNAME,
  ID_TOOLBUTTON_COPYNOTESFLD,
  ID_TOOLBUTTON_CLEARCLIPBOARD,
  ID_SEPARATOR,
  ID_TOOLBUTTON_AUTOTYPE,
  ID_TOOLBUTTON_BROWSEURL,
  ID_SEPARATOR,
  ID_TOOLBUTTON_ADD,
  ID_TOOLBUTTON_EDIT,
  ID_SEPARATOR,
  ID_TOOLBUTTON_DELETE,
  ID_SEPARATOR,
  ID_TOOLBUTTON_EXPANDALL,
  ID_TOOLBUTTON_COLLAPSEALL,
  ID_SEPARATOR,
  ID_TOOLBUTTON_OPTIONS,
  ID_SEPARATOR,
  ID_HELP,
  // End of Default Toolbar
  // Following are not in the "default" toolbar but can be selected by the user
  ID_TOOLBUTTON_EXPORTTEXT,
  ID_TOOLBUTTON_EXPORTXML,
  ID_TOOLBUTTON_IMPORTTEXT,
  ID_TOOLBUTTON_IMPORTXML,
  ID_TOOLBUTTON_SAVEAS,
  ID_TOOLBUTTON_COMPARE,
  ID_TOOLBUTTON_MERGE,
  ID_TOOLBUTTON_LISTTREE,
  ID_TOOLBUTTON_VIEWREPORTS
};

const UINT CPWToolBar::m_MainToolBarClassicBMs[] = {
  IDB_NEW_CLASSIC,
  IDB_OPEN_CLASSIC,
  IDB_CLOSE_CLASSIC,
  IDB_SAVE_CLASSIC,
  IDB_COPYPASSWORD_CLASSIC,
  IDB_COPYUSER_CLASSIC,
  IDB_COPYNOTES_CLASSIC,
  IDB_CLEARCLIPBOARD_CLASSIC,
  IDB_AUTOTYPE_CLASSIC,
  IDB_BROWSEURL_CLASSIC,
  IDB_ADD_CLASSIC,
  IDB_VIEWEDIT_CLASSIC,
  IDB_DELETE_CLASSIC,
  IDB_EXPANDALL_CLASSIC,
  IDB_COLLAPSEALL_CLASSIC,
  IDB_OPTIONS_CLASSIC,
  IDB_HELP_CLASSIC,
  // End of Default Toolbar
  // Following are not in the "default" toolbar but can be selected by the user
  IDB_EXPORTTEXT_CLASSIC,
  IDB_EXPORTXML_CLASSIC,
  IDB_IMPORTTEXT_CLASSIC,
  IDB_IMPORTXML_CLASSIC,
  IDB_SAVEAS_CLASSIC,
  IDB_COMPARE_CLASSIC,
  IDB_MERGE_CLASSIC,
  IDB_LISTTREE_CLASSIC,
  IDB_VIEWREPORTS_CLASSIC,

  // Additional bitmap for swapping image if entry's URL == email
  IDB_SENDEMAIL_CLASSIC
};

const UINT CPWToolBar::m_MainToolBarNew8BMs[] = {
  IDB_NEW_NEW8,
  IDB_OPEN_NEW8,
  IDB_CLOSE_NEW8,
  IDB_SAVE_NEW8,
  IDB_COPYPASSWORD_NEW8,
  IDB_COPYUSER_NEW8,
  IDB_COPYNOTES_NEW8,
  IDB_CLEARCLIPBOARD_NEW8,
  IDB_AUTOTYPE_NEW8,
  IDB_BROWSEURL_NEW8,
  IDB_ADD_NEW8,
  IDB_VIEWEDIT_NEW8,
  IDB_DELETE_NEW8,
  IDB_EXPANDALL_NEW8,
  IDB_COLLAPSEALL_NEW8,
  IDB_OPTIONS_NEW8,
  IDB_HELP_NEW8,
  // End of Default Toolbar
  // Following are not in the "default" toolbar but can be selected by the user
  IDB_EXPORTTEXT_NEW8,
  IDB_EXPORTXML_NEW8,
  IDB_IMPORTTEXT_NEW8,
  IDB_IMPORTXML_NEW8,
  IDB_SAVEAS_NEW8,
  IDB_COMPARE_NEW8,
  IDB_MERGE_NEW8,
  IDB_LISTTREE_NEW8,
  IDB_VIEWREPORTS_NEW8,

  // Additional bitmap for swapping image if entry's URL == email
  IDB_SENDEMAIL_NEW8
};

const UINT CPWToolBar::m_MainToolBarNew32BMs[] = {
  IDB_NEW_NEW32,
  IDB_OPEN_NEW32,
  IDB_CLOSE_NEW32,
  IDB_SAVE_NEW32,
  IDB_COPYPASSWORD_NEW32,
  IDB_COPYUSER_NEW32,
  IDB_COPYNOTES_NEW32,
  IDB_CLEARCLIPBOARD_NEW32,
  IDB_AUTOTYPE_NEW32,
  IDB_BROWSEURL_NEW32,
  IDB_ADD_NEW32,
  IDB_VIEWEDIT_NEW32,
  IDB_DELETE_NEW32,
  IDB_EXPANDALL_NEW32,
  IDB_COLLAPSEALL_NEW32,
  IDB_OPTIONS_NEW32,
  IDB_HELP_NEW32,
  // End of Default Toolbar
  // Following are not in the "default" toolbar but can be selected by the user
  IDB_EXPORTTEXT_NEW32,
  IDB_EXPORTXML_NEW32,
  IDB_IMPORTTEXT_NEW32,
  IDB_IMPORTXML_NEW32,
  IDB_SAVEAS_NEW32,
  IDB_COMPARE_NEW32,
  IDB_MERGE_NEW32,
  IDB_LISTTREE_NEW32,
  IDB_VIEWREPORTS_NEW32,

  // Additional bitmap for swapping image if entry's URL == email
  IDB_SENDEMAIL_NEW32
};

IMPLEMENT_DYNAMIC(CPWToolBar, CToolBar)

CPWToolBar::CPWToolBar()
  :  m_ClassicFlags(0), m_NewFlags(0), m_bitmode(1),
     m_iBrowseURL_BM_offset(-1), m_iSendEmail_BM_offset(-1)
{
  ASSERT(sizeof(m_MainToolBarIDs) / sizeof(UINT) ==
         sizeof(m_csMainButtons) / sizeof(m_csMainButtons[0]));

  m_iMaxNumButtons = sizeof(m_MainToolBarIDs) / sizeof(UINT);
  m_pOriginalTBinfo = new TBBUTTON[m_iMaxNumButtons];

  ASSERT(sizeof(m_MainToolBarClassicBMs) / sizeof(UINT) ==
         sizeof(m_MainToolBarNew8BMs) / sizeof(UINT));
  ASSERT(sizeof(m_MainToolBarClassicBMs) / sizeof(UINT) ==
         sizeof(m_MainToolBarNew32BMs) / sizeof(UINT));

  m_iNum_Bitmaps = sizeof(m_MainToolBarClassicBMs) / sizeof(UINT);
}

CPWToolBar::~CPWToolBar()
{
  m_ImageList.DeleteImageList();
  delete [] m_pOriginalTBinfo;
}

BEGIN_MESSAGE_MAP(CPWToolBar, CToolBar)
  ON_NOTIFY_REFLECT(TBN_GETBUTTONINFO, OnToolBarGetButtonInfo)
  ON_NOTIFY_REFLECT(TBN_QUERYINSERT, OnToolBarQueryInsert)
  ON_NOTIFY_REFLECT(TBN_QUERYDELETE, OnToolBarQueryDelete)
  ON_NOTIFY_REFLECT(TBN_GETBUTTONINFO, OnToolBarQueryInfo)
  ON_NOTIFY_REFLECT(TBN_RESET, OnToolBarReset)
END_MESSAGE_MAP()

// CPWToolBar message handlers

void
CPWToolBar::OnToolBarQueryInsert(NMHDR* /* pNotifyStruct */, LRESULT *pResult)
{
  *pResult = TRUE;
}

void
CPWToolBar::OnToolBarQueryDelete(NMHDR* pNotifyStruct, LRESULT *pResult)
{
  NMTOOLBAR* pNMToolbar = (NMTOOLBAR *)pNotifyStruct;

  if ((pNMToolbar->tbButton.idCommand != ID_SEPARATOR) &&
    GetToolBarCtrl().IsButtonHidden(pNMToolbar->tbButton.idCommand))
    *pResult = FALSE;
  else
   *pResult = TRUE;
}

void
CPWToolBar::OnToolBarQueryInfo(NMHDR* pNotifyStruct, LRESULT *pResult)
{
  NMTOOLBAR* pNMToolbar = (NMTOOLBAR *)pNotifyStruct;

  ASSERT(pNMToolbar->iItem < m_iMaxNumButtons);

  if ((pNMToolbar->iItem >= 0) &&
      (pNMToolbar->iItem < m_iMaxNumButtons)) {
    pNMToolbar->tbButton =m_pOriginalTBinfo[pNMToolbar->iItem];
    *pResult = TRUE;
  } else {
    *pResult = FALSE;
  }
}

void
CPWToolBar::OnToolBarGetButtonInfo(NMHDR *pNotifyStruct, LRESULT *pResult)
{
  NMTOOLBAR* pNMToolbar = (NMTOOLBAR *)pNotifyStruct;

  ASSERT(pNMToolbar->iItem <= m_iMaxNumButtons);

  // if the index is valid
  if ((pNMToolbar->iItem >= 0) && (pNMToolbar->iItem < m_iMaxNumButtons)) {
    // copy the stored button structure
    pNMToolbar->tbButton = m_pOriginalTBinfo[pNMToolbar->iItem];
    *pResult = TRUE;
  } else {
    *pResult = FALSE;
  }
}

void
CPWToolBar::OnToolBarReset(NMHDR* /* pNotifyStruct */, LRESULT* /* pResult */)
{
  Reset();
}

//  Other routines

void
CPWToolBar::Init(const int NumBits)
{
  int i, j;
  m_ClassicBackground = RGB(192, 192, 192);
  m_NewBackground = RGB(192, 192, 192);

  m_ClassicFlags = ILC_MASK | ILC_COLOR8;

  if (NumBits >= 32) {
    m_NewFlags = ILC_MASK | ILC_COLOR32;
    m_NewBackground = RGB(196, 196, 196);
    m_bitmode = 2;
  } else {
    m_NewFlags = ILC_MASK | ILC_COLOR8;
  }

  CBitmap bmTemp;
  // Classic images are first in the ImageList followed by the New8 and then New32 images.
  m_ImageList.Create(16, 16, m_ClassicFlags, m_iNum_Bitmaps * 3, 2);
  for (i = 0; i < m_iNum_Bitmaps; i++) {
    bmTemp.LoadBitmap(m_MainToolBarClassicBMs[i]);
    m_ImageList.Add(&bmTemp, m_ClassicBackground);
    bmTemp.Detach();
    if (m_MainToolBarClassicBMs[i] == IDB_BROWSEURL_CLASSIC)
      m_iBrowseURL_BM_offset = i;
    if (m_MainToolBarClassicBMs[i] == IDB_SENDEMAIL_CLASSIC)
      m_iSendEmail_BM_offset = i;
  }

  for (i = 0; i < m_iNum_Bitmaps; i++) {
    bmTemp.LoadBitmap(m_MainToolBarNew8BMs[i]);
    m_ImageList.Add(&bmTemp, m_NewBackground);
    bmTemp.Detach();
  }

  for (i = 0; i < m_iNum_Bitmaps; i++) {
    bmTemp.LoadBitmap(m_MainToolBarNew32BMs[i]);
    m_ImageList.Add(&bmTemp, m_NewBackground);
    bmTemp.Detach();
  }

  j = 0;
  m_csDefaultButtonString.Empty();
  m_iNumDefaultButtons = m_iMaxNumButtons;
  for (i = 0; i < m_iMaxNumButtons; i++) {
    const bool bIsSeparator = m_MainToolBarIDs[i] == ID_SEPARATOR;
    BYTE fsStyle = bIsSeparator ? TBSTYLE_SEP : TBSTYLE_BUTTON;
    fsStyle &= ~BTNS_SHOWTEXT;
    if (!bIsSeparator) {
      fsStyle |= TBSTYLE_AUTOSIZE;
    }
    m_pOriginalTBinfo[i].iBitmap = bIsSeparator ? -1 : j;
    m_pOriginalTBinfo[i].idCommand = m_MainToolBarIDs[i];
    m_pOriginalTBinfo[i].fsState = TBSTATE_ENABLED;
    m_pOriginalTBinfo[i].fsStyle = fsStyle;
    m_pOriginalTBinfo[i].dwData = 0;
    m_pOriginalTBinfo[i].iString = bIsSeparator ? -1 : j;

    if (i <= m_iNumDefaultButtons)
      m_csDefaultButtonString += m_csMainButtons[i] + _T(" ");

    if (m_MainToolBarIDs[i] == ID_HELP)
      m_iNumDefaultButtons = i;

    if (!bIsSeparator)
      j++;
  }
}

void
CPWToolBar::CustomizeButtons(CString csButtonNames)
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

  std::vector<CString> vcsButtonNameArray;

  csButtonNames.MakeLower();

  for (i = 0; i < m_iMaxNumButtons; i++) {
    vcsButtonNameArray.push_back(m_csMainButtons[i]);
  }

  std::vector<CString>::const_iterator cstring_iter;

  int curPos(0);
  // Note all separators will be treated as the first!
  i = 0;
  CString csToken = csButtonNames.Tokenize(_T(" "), curPos);
  while (csToken != _T("") && curPos != -1) {
    cstring_iter = std::find(vcsButtonNameArray.begin(), vcsButtonNameArray.end(), csToken);
    if (cstring_iter != vcsButtonNameArray.end()) {
      int index = (int)(cstring_iter - vcsButtonNameArray.begin());
      tbCtrl.AddButtons(1, &m_pOriginalTBinfo[index]);
    }
    csToken = csButtonNames.Tokenize(_T(" "), curPos);
  }

  tbCtrl.AutoSize();
}

CString
CPWToolBar::GetButtonString()
{
  CString cs_buttonnames(_T(""));
  TBBUTTONINFO tbinfo;
  int num_buttons, i;

  CToolBarCtrl& tbCtrl = GetToolBarCtrl();

  num_buttons = tbCtrl.GetButtonCount();

  std::vector<UINT> vcsButtonIDArray;
  std::vector<UINT>::const_iterator uint_iter;

  for (i = 0; i < m_iMaxNumButtons; i++) {
    vcsButtonIDArray.push_back(m_MainToolBarIDs[i]);
  }

  memset(&tbinfo, 0x00, sizeof(tbinfo));
  tbinfo.cbSize = sizeof(tbinfo);
  tbinfo.dwMask = TBIF_BYINDEX | TBIF_COMMAND | TBIF_STYLE;

  for (i = 0; i < num_buttons; i++) {
    tbCtrl.GetButtonInfo(i, &tbinfo);

    if (tbinfo.fsStyle & TBSTYLE_SEP) {
       cs_buttonnames += _T("~ ");
       continue;
    }

    uint_iter = std::find(vcsButtonIDArray.begin(), vcsButtonIDArray.end(), tbinfo.idCommand);
    if (uint_iter != vcsButtonIDArray.end()) {
      int index = (int)(uint_iter - vcsButtonIDArray.begin());
      cs_buttonnames += m_csMainButtons[index] + _T(" ");
    }
  }

  if (cs_buttonnames.CompareNoCase(m_csDefaultButtonString) == 0) {
    cs_buttonnames.Empty();
  }

  return cs_buttonnames;
}

void
CPWToolBar::Reset()
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

void
CPWToolBar::ChangeImages(const int toolbarMode)
{
  m_toolbarMode = toolbarMode;

  int nCount, i, j;
  // Classic images are first in the ImageList followed by the New images.
  const int iOffset = (m_toolbarMode == ID_MENUITEM_OLD_TOOLBAR) ? 0 : m_bitmode * m_iNum_Bitmaps;

  j = 0;
  for (i = 0; i < m_iMaxNumButtons; i++) {
    if (m_MainToolBarIDs[i] != ID_SEPARATOR) {
      m_pOriginalTBinfo[i].iBitmap = j + iOffset;
      j++;
    }
  }

  TBBUTTONINFO tbinfo;
  memset(&tbinfo, 0x00, sizeof(tbinfo));
  tbinfo.cbSize = sizeof(tbinfo);
  tbinfo.dwMask = TBIF_BYINDEX | TBIF_IMAGE | TBIF_COMMAND | TBIF_STYLE;

  CToolBarCtrl& tbCtrl = GetToolBarCtrl();
  nCount = tbCtrl.GetButtonCount();
  for (i = 0; i < nCount; i++) {
    tbCtrl.GetButtonInfo(i, &tbinfo);
    if (tbinfo.fsStyle & TBSTYLE_SEP)
      continue;

    tbinfo.iImage %= m_iNum_Bitmaps;
    tbinfo.iImage += iOffset;
    tbCtrl.SetButtonInfo(i, &tbinfo);
  }
}

void
CPWToolBar::LoadDefaultToolBar(const int toolbarMode)
{
  m_toolbarMode = toolbarMode;

  int nCount, i, j;
  CToolBarCtrl& tbCtrl = GetToolBarCtrl();
  nCount = tbCtrl.GetButtonCount();

  for (i = nCount - 1; i >= 0; i--) {
    tbCtrl.DeleteButton(i);
  }

  tbCtrl.SetImageList(&m_ImageList);
  m_ImageList.Detach();

  j = 0;
  const int iOffset = (m_toolbarMode == ID_MENUITEM_OLD_TOOLBAR) ? 0 : m_bitmode * m_iNum_Bitmaps;
  for (i = 0; i < m_iMaxNumButtons; i++) {
    if (m_MainToolBarIDs[i] != ID_SEPARATOR) {
      m_pOriginalTBinfo[i].iBitmap = j + iOffset;
      j++;
    }
  }

  // Create text for customization dialog using button tooltips.
  // Assume no button tooltip description exceeds 64 characters, also m_iMaxNumButtons
  // includes separators which don't have strings giving an even bigger buffer!
  // Because they are a concatenation of null terminated strings terminated by a double
  // null, they cannot be stored in a CString variable,
  TCHAR *lpszTBCustomizationStrings = new TCHAR[m_iMaxNumButtons * 64];
  const int maxlength = m_iMaxNumButtons * 64;

  // By clearing, ensures string ends with a double NULL
  memset(lpszTBCustomizationStrings, 0x00, maxlength * sizeof(TCHAR));

  j = 0;
  for (i = 0; i < m_iMaxNumButtons; i++) {
    if (m_MainToolBarIDs[i] != ID_SEPARATOR) {
      CString cs_buttondesc;
      cs_buttondesc.LoadString(m_MainToolBarIDs[i]);
      int iPos = cs_buttondesc.ReverseFind(_T('\n'));
      ASSERT(iPos >= 0);
      cs_buttondesc = cs_buttondesc.Right(cs_buttondesc.GetLength() - iPos - 1);
      int idesclen = cs_buttondesc.GetLength();
      TCHAR *szDescription = cs_buttondesc.GetBuffer(idesclen);
#if _MSC_VER >= 1400
      memcpy_s(&lpszTBCustomizationStrings[j], maxlength - j, szDescription, idesclen * sizeof(TCHAR));
#else
      ASSERT((maxlength - j) > idesclen * sizeof(TCHAR));
      memcpy(&lpszTBCustomizationStrings[j], szDescription, idesclen * sizeof(TCHAR));
#endif
      cs_buttondesc.ReleaseBuffer();
      j += idesclen + 1;
    }
  }

  tbCtrl.AddStrings(lpszTBCustomizationStrings);
  tbCtrl.AddButtons(m_iMaxNumButtons, &m_pOriginalTBinfo[0]);

  delete [] lpszTBCustomizationStrings;

  DWORD dwStyle, dwStyleEx;
  dwStyle = tbCtrl.GetStyle();
  dwStyle &= ~TBSTYLE_AUTOSIZE;
  tbCtrl.SetStyle(dwStyle | TBSTYLE_LIST);

  dwStyleEx = tbCtrl.GetExtendedStyle();
  tbCtrl.SetExtendedStyle(dwStyleEx | TBSTYLE_EX_MIXEDBUTTONS);
}
