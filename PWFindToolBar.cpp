/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// PWFindToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "DboxMain.h"
#include "PWFindToolBar.h"
#include "ControlExtns.h"
#include "AdvancedDlg.h"
#include "ThisMfcApp.h" // for disable/enable accel.
#include "resource.h"
#include "resource2.h"
#include "resource3.h"

#include <vector>
#include <algorithm>

// CPWFindToolBar

// ***** NOTE: THIS TOOLBAR IS NOT CUSTOMIZABLE *****
// *****        IT CAN ONLY BE TOGGLED ON/OFF   *****

// See comments in PWToolBar.cpp for details of these arrays

/*
* Note: Bad coding but if you change the position of the Case Sensitive button
* in the Find Toolbar, you must update the "m_iCase_Insensitive_BM_offset" variable
* with its new offest in the bitmap arrays.
* Also, the Case Sensitive bitmap must be the last bitmap in the arrays.

*/

#define EDITCTRL_WIDTH 100    // width of Edit control for Search Text
#define FINDRESULTS_WIDTH 400 // width of Edit control for Search Results

const UINT CPWFindToolBar::m_FindToolBarIDs[] = {
  ID_TOOLBUTTON_CLOSEFIND,
  ID_TOOLBUTTON_FINDEDITCTRL,
  ID_TOOLBUTTON_FIND,
  ID_TOOLBUTTON_FINDCASE_I,
  ID_TOOLBUTTON_FINDADVANCED,
  ID_TOOLBUTTON_FINDREPORT,
  ID_TOOLBUTTON_CLEARFIND,
  ID_SEPARATOR,
  ID_TOOLBUTTON_FINDRESULTS
};

const UINT CPWFindToolBar::m_FindToolBarClassicBMs[] = {
  IDB_FINDCLOSE_CLASSIC,
  IDB_FINDCTRLPLACEHOLDER,
  IDB_FIND_CLASSIC,
  IDB_FINDCASE_I_CLASSIC,       // m_iCase_Insensitive_BM_offset contains this offset
  IDB_FINDADVANCED_CLASSIC,
  IDB_FINDREPORT_CLASSIC,
  IDB_FINDCLEAR_CLASSIC,
  IDB_FINDCTRLPLACEHOLDER,

  // Additional bitmap for swapping image on "Case" button
  IDB_FINDCASE_S_CLASSIC        // Must be last!
};

const UINT CPWFindToolBar::m_FindToolBarNewBMs[] = {
  IDB_FINDCLOSE_NEW,
  IDB_FINDCTRLPLACEHOLDER,
  IDB_FIND_NEW,
  IDB_FINDCASE_I_NEW,           // m_iCase_Insensitive_BM_offset contains this offset
  IDB_FINDADVANCED_NEW,
  IDB_FINDREPORT_NEW,
  IDB_FINDCLEAR_NEW,
  IDB_FINDCTRLPLACEHOLDER,

  // Additional bitmap for swapping image on "Case" button
  IDB_FINDCASE_S_NEW            // Must be last!
};

IMPLEMENT_DYNAMIC(CPWFindToolBar, CToolBar)

CPWFindToolBar::CPWFindToolBar()
  : m_bitmode(1), m_bVisible(true), 
  m_bCaseSensitive(false), m_bAdvanced(false),
  m_lastshown(size_t(-1)), m_numFound(size_t(-1)),
  m_last_search_text(_T("")), m_last_cs_search(false),
  m_subgroup_name(_T("")), m_subgroup_set(BST_UNCHECKED),
  m_subgroup_object(0), m_subgroup_function(0),
  m_last_subgroup_name(_T("")), m_last_subgroup_set(BST_UNCHECKED),
  m_last_subgroup_object(0), m_last_subgroup_function(0),
  m_iCase_Insensitive_BM_offset(-1), m_iCase_Sensitive_BM_offset(-1)
{
  m_bsFields.reset();
  m_last_bsFields.reset();

  m_iMaxNumButtons = sizeof(m_FindToolBarIDs) / sizeof(UINT);
  m_pOriginalTBinfo = new TBBUTTON[m_iMaxNumButtons];

  ASSERT(sizeof(m_FindToolBarClassicBMs) / sizeof(UINT) ==
    sizeof(m_FindToolBarNewBMs) / sizeof(UINT));

  m_iNum_Bitmaps = sizeof(m_FindToolBarClassicBMs) / sizeof(UINT);

  LOGFONT lf;
  memset(&lf, 0, sizeof(lf));

  // Since design guide says toolbars are fixed height so is the font.
  lf.lfHeight = -11;
  lf.lfWeight = FW_LIGHT;
  lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
  CString strDefaultFont = _T("MS Sans Serif");
  lstrcpy(lf.lfFaceName, strDefaultFont);
  VERIFY(m_FindTextFont.CreateFontIndirect(&lf));
}

CPWFindToolBar::~CPWFindToolBar()
{
  m_ImageLists[0].DeleteImageList();
  m_ImageLists[1].DeleteImageList();
  m_ImageLists[2].DeleteImageList();
  delete [] m_pOriginalTBinfo;
  m_FindTextFont.DeleteObject();
  m_findedit.DestroyWindow();
}

BEGIN_MESSAGE_MAP(CPWFindToolBar, CToolBar)
END_MESSAGE_MAP()

// CPWFindToolBar message handlers

void CPWFindToolBar::RefreshImages()
{
  m_ImageLists[0].DeleteImageList();
  m_ImageLists[1].DeleteImageList();
  m_ImageLists[2].DeleteImageList();

  Init(m_NumBits, m_pDbx, m_iWMSGID);

  ChangeImages(m_toolbarMode);
}

BOOL CPWFindToolBar::PreTranslateMessage(MSG *pMsg)
{
  CWnd *pWnd = FromHandle(pMsg->hwnd);
  if (pWnd->GetDlgCtrlID() == ID_TOOLBUTTON_FINDEDITCTRL) {
    if (pMsg->message == WM_KEYDOWN) {
      if (pMsg->wParam == VK_RETURN) {
        m_pDbx->SendMessage(m_iWMSGID);
        return TRUE;
      }
      if (pMsg->wParam == VK_DELETE) {
        int iTextLen, iStartChar, iEndChar, iCaret, iCharIndex;
        CPoint pt_cursor;
        pt_cursor = m_findedit.GetCaretPos();
        iCaret = m_findedit.CharFromPos(pt_cursor);
        iCharIndex = LOWORD(iCaret);
        iTextLen = m_findedit.GetWindowTextLength();
        m_findedit.GetSel(iStartChar, iEndChar);
        if (iCharIndex == iStartChar && iCharIndex == iEndChar) {
          // Nothing selected - forward backspace
          m_findedit.SetSel(iCharIndex, iCharIndex + 1);
          m_findedit.ReplaceSel(_T(""));
        } else if (iTextLen > (iEndChar - iStartChar)) {
          m_findedit.ReplaceSel(_T(""));
        } else {
          m_findedit.SetWindowText(_T(""));
        }
        m_findedit.Invalidate();
        return TRUE;
      }
    }
  }

  return CToolBar::PreTranslateMessage(pMsg);
}

//  Other routines

void CPWFindToolBar::Init(const int NumBits, CWnd *pDbx, int iWMSGID)
{
  int i, j;
  COLORREF crClassicBackground = RGB(192, 192, 192);
  COLORREF crNewBackground = RGB(192, 192, 192);
  UINT iClassicFlags = ILC_MASK | ILC_COLOR8;
  UINT iNewFlags1 = ILC_MASK | ILC_COLOR8;
  UINT iNewFlags2 = ILC_MASK | ILC_COLOR32;

  m_NumBits = NumBits;

  if (NumBits >= 32) {
    m_bitmode = 2;
  }

  CBitmap bmTemp;
  m_ImageLists[0].Create(16, 16, iClassicFlags, m_iNum_Bitmaps, 2);
  m_ImageLists[1].Create(16, 16, iNewFlags1, m_iNum_Bitmaps, 2);
  m_ImageLists[2].Create(16, 16, iNewFlags2, m_iNum_Bitmaps, 2);
  for (i = 0; i < m_iNum_Bitmaps; i++) {
    bmTemp.LoadBitmap(m_FindToolBarClassicBMs[i]);
    m_ImageLists[0].Add(&bmTemp, crClassicBackground);
    bmTemp.Detach();
    if (m_FindToolBarClassicBMs[i] == IDB_FINDCASE_I_CLASSIC)
      m_iCase_Insensitive_BM_offset = i;
    if (m_FindToolBarClassicBMs[i] == IDB_FINDCASE_S_CLASSIC)
      m_iCase_Sensitive_BM_offset = i;
  }

  for (i = 0; i < m_iNum_Bitmaps; i++) {
    bmTemp.LoadBitmap(m_FindToolBarNewBMs[i]);
    m_ImageLists[1].Add(&bmTemp, crNewBackground);
    bmTemp.Detach();
  }

  for (i = 0; i < m_iNum_Bitmaps; i++) {
    bmTemp.LoadBitmap(m_FindToolBarNewBMs[i]);
    m_ImageLists[2].Add(&bmTemp, crNewBackground);
    bmTemp.Detach();
  }

  j = 0;
  for (i = 0; i < m_iMaxNumButtons; i++) {
    const bool bIsSeparator = m_FindToolBarIDs[i] == ID_SEPARATOR;
    BYTE fsStyle = bIsSeparator ? TBSTYLE_SEP : TBSTYLE_BUTTON;
    fsStyle &= ~BTNS_SHOWTEXT;
    if (!bIsSeparator) {
      fsStyle |= TBSTYLE_AUTOSIZE;
    }
    m_pOriginalTBinfo[i].iBitmap = bIsSeparator ? -1 : j;
    m_pOriginalTBinfo[i].idCommand = m_FindToolBarIDs[i];
    m_pOriginalTBinfo[i].fsState = TBSTATE_ENABLED;
    m_pOriginalTBinfo[i].fsStyle = fsStyle;
    m_pOriginalTBinfo[i].dwData = 0;
    m_pOriginalTBinfo[i].iString = bIsSeparator ? -1 : j;
    if (!bIsSeparator)
      j++;
  }

  m_pDbx = pDbx;
  m_iWMSGID = iWMSGID;
}

void CPWFindToolBar::LoadDefaultToolBar(const int toolbarMode)
{
  CToolBarCtrl& tbCtrl = GetToolBarCtrl();
  m_toolbarMode = toolbarMode;
  const int nImageListNum = (m_toolbarMode == ID_MENUITEM_OLD_TOOLBAR) ? 0 : m_bitmode;
  tbCtrl.SetImageList(&m_ImageLists[nImageListNum]);

  tbCtrl.AddButtons(m_iMaxNumButtons, &m_pOriginalTBinfo[0]);

  TBBUTTONINFO tbinfo;
  memset(&tbinfo, 0x00, sizeof(tbinfo));
  tbinfo.cbSize = sizeof(tbinfo);
  tbinfo.dwMask = TBIF_STYLE;

  tbCtrl.GetButtonInfo(ID_TOOLBUTTON_FINDCASE_I, &tbinfo);
  tbinfo.fsStyle = TBSTYLE_CHECK;
  tbCtrl.GetButtonInfo(ID_TOOLBUTTON_FINDCASE_I, &tbinfo);

  AddExtraControls();

  tbCtrl.AutoSize();
  tbCtrl.SetMaxTextRows(0);
}

void CPWFindToolBar::AddExtraControls()
{
  CRect rect, rt;
  int index, iBtnHeight;

  GetItemRect(0, &rt);
  iBtnHeight = rt.Height();

  // Add find search text CEdit control (CEditExtn)
  // Get the index of the placeholder's position in the toolbar
  index = CommandToIndex(ID_TOOLBUTTON_FINDEDITCTRL);
  ASSERT(index != -1);

  // If we have been here before, destroy it first
  if (m_findedit.GetSafeHwnd() != NULL) {
    m_findedit.DestroyWindow();
  }

  // Convert that button to a separator
  SetButtonInfo(index, ID_TOOLBUTTON_FINDEDITCTRL, TBBS_SEPARATOR, EDITCTRL_WIDTH);

  // Note: "ES_WANTRETURN | ES_MULTILINE".  This is to allow the return key to be
  // trapped by PreTranslateMessage and treated as if the Find button had been
  // pressed
  rect = CRect(0, 0, EDITCTRL_WIDTH, iBtnHeight);
  VERIFY(m_findedit.Create(WS_CHILD | WS_VISIBLE |
                           ES_AUTOHSCROLL | ES_LEFT | ES_WANTRETURN | ES_MULTILINE,
                           CRect(rect.left + 2, rect.top + 2, rect.right - 2, rect.bottom - 2),
                           this, ID_TOOLBUTTON_FINDEDITCTRL));

  GetItemRect(index, &rect);
  rect.top += max((rect.top - rt.top) / 2, 0);
  m_findedit.SetWindowPos(NULL, rect.left + 2, rect.top + 2, 0, 0,
                          SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOCOPYBITS );

  m_findedit.SetFont(&m_FindTextFont);

  // Add find search results CStatic control
  // Get the index of the placeholder's position in the toolbar
  index = CommandToIndex(ID_TOOLBUTTON_FINDRESULTS);
  ASSERT(index != -1);

  // If we have been here before, destroy it first
  if (m_findresults.GetSafeHwnd() != NULL) {
    m_findresults.DestroyWindow();
  }

  // Convert that button to a separator
  SetButtonInfo(index, ID_TOOLBUTTON_FINDRESULTS, TBBS_SEPARATOR, FINDRESULTS_WIDTH);

  rect = CRect(0, 0, FINDRESULTS_WIDTH, iBtnHeight);
  VERIFY(m_findresults.Create(_T(""), WS_CHILD | WS_VISIBLE |
                              SS_LEFTNOWORDWRAP | SS_CENTERIMAGE,
                              CRect(rect.left + 2, rect.top + 2, rect.right - 2, rect.bottom - 2),
                              this, ID_TOOLBUTTON_FINDEDITCTRL));

  GetItemRect(index, &rect);
  rect.top += max((rect.top - rt.top) / 2, 0);
  m_findresults.SetWindowPos(NULL, rect.left + 2, rect.top + 2, 0, 0,
                             SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOCOPYBITS );

  m_findresults.SetFont(&m_FindTextFont);

  ModifyStyle(0, WS_CLIPCHILDREN);
}

void CPWFindToolBar::ShowFindToolBar(bool bShow)
{
  if (this->GetSafeHwnd() == NULL)
    return;

  ::ShowWindow(this->GetSafeHwnd(), bShow ? SW_SHOW : SW_HIDE);
  ::EnableWindow(this->GetSafeHwnd(), bShow ? TRUE : FALSE);

  if (bShow) {
    m_findedit.ChangeColour();
    m_findedit.SetFocus();
    m_findedit.SetSel(0, -1);  // Select all text
    m_findedit.Invalidate();
  }
  m_bVisible = bShow;
}

void CPWFindToolBar::ToggleToolBarFindCase()
{
  CToolBarCtrl& tbCtrl = GetToolBarCtrl();

  int ID, index;
  if (m_bCaseSensitive)
    ID = ID_TOOLBUTTON_FINDCASE_S;
  else
    ID = ID_TOOLBUTTON_FINDCASE_I;
  index = CommandToIndex(ID);

  m_bCaseSensitive = !m_bCaseSensitive;
  tbCtrl.CheckButton(ID, m_bCaseSensitive ? TRUE : FALSE);

  TBBUTTONINFO tbinfo;
  memset(&tbinfo, 0x00, sizeof(tbinfo));
  tbinfo.cbSize = sizeof(tbinfo);
  tbinfo.dwMask = TBIF_IMAGE;
  tbCtrl.GetButtonInfo(ID, &tbinfo);

  // Note: The case_sensitive bitmap MUST be the LAST in the array of bitmaps
  // Note: "m_iCase_Insensitive_BM_offset" points to the case_insensitive bitmap in the array
  tbinfo.iImage = m_bCaseSensitive ? m_iNum_Bitmaps - 1 : m_iCase_Insensitive_BM_offset;
  tbinfo.dwMask = TBIF_IMAGE;
  tbCtrl.SetButtonInfo(ID, &tbinfo);

  tbCtrl.SetCmdID(index, m_bCaseSensitive ? ID_TOOLBUTTON_FINDCASE_S : ID_TOOLBUTTON_FINDCASE_I);
}

void CPWFindToolBar::ChangeImages(const int toolbarMode)
{
  CToolBarCtrl& tbCtrl = GetToolBarCtrl();
  m_toolbarMode = toolbarMode;
  const int nImageListNum = (m_toolbarMode == ID_MENUITEM_OLD_TOOLBAR) ? 0 : m_bitmode;
  tbCtrl.SetImageList(&m_ImageLists[nImageListNum]);
}

void CPWFindToolBar::ClearFind()
{
  if (this->GetSafeHwnd() == NULL)
    return;

  m_findedit.SetWindowText(_T(""));
  m_findresults.ResetColour();
  m_findresults.SetWindowText(_T(""));

  m_bCaseSensitive = m_bAdvanced = m_last_cs_search = false;
  m_numFound = size_t(-1);
  m_last_search_text = _T("");
  m_subgroup_name = m_last_subgroup_name = _T("");
  m_subgroup_set = m_last_subgroup_set = BST_UNCHECKED;
  m_subgroup_object = m_subgroup_function = 0;
  m_last_subgroup_object = m_last_subgroup_function = 0;
  m_lastshown = size_t(-1);
}

void CPWFindToolBar::Find()
{
  if (this->GetSafeHwnd() == NULL)
    return;

  DboxMain* pDbx = static_cast<DboxMain *>(m_pDbx);
  ASSERT(pDbx != NULL);

  CString cs_status;
  m_findedit.GetWindowText(m_search_text);
  if (m_search_text.IsEmpty()) {
    cs_status.LoadString(IDS_ENTERSEARCHSTRING);
    m_findresults.SetColour(RGB(255, 0, 0));
    m_findresults.SetWindowText(cs_status);
    return;
  }

  m_cs_search = m_bCaseSensitive;

  // If the user changes the search text or cs, then this is a new search:
  if (m_search_text != m_last_search_text ||
      m_cs_search != m_last_cs_search ||
      m_bsFields != m_last_bsFields ||
      m_subgroup_name != m_last_subgroup_name ||
      m_subgroup_set != m_last_subgroup_set ||
      m_subgroup_object != m_last_subgroup_object ||
      m_subgroup_function != m_last_subgroup_function) {
    m_last_search_text = m_search_text;
    m_last_cs_search = m_cs_search;
    m_last_bsFields = m_bsFields;
    m_last_subgroup_name = m_subgroup_name;
    m_last_subgroup_set = m_subgroup_set;
    m_last_subgroup_object = m_subgroup_object;
    m_last_subgroup_function = m_subgroup_function;
    m_lastshown = size_t(-1);
  }

  if (m_lastshown == -1) {
    m_indices.clear();

    if (m_bAdvanced)
      m_numFound = pDbx->FindAll(m_search_text, m_cs_search, m_indices,
                                 m_bsFields, m_subgroup_set,
                                 m_subgroup_name, m_subgroup_object, 
                                 m_subgroup_function);
    else
      m_numFound = pDbx->FindAll(m_search_text, m_cs_search, m_indices);

    switch (m_numFound) {
      case 0:
        cs_status.LoadString(IDS_NOMATCHFOUND);
        break;
      case 1:
        cs_status.LoadString(IDS_FOUNDAMATCH);
        break;
      default:
        cs_status.Format(IDS_FOUNDMATCHES, 1, m_numFound);
        break;
    }
    pDbx->ResumeOnListNotification();
  } // m_lastshown == -1

  // OK, so now we have a (possibly empty) list of items to select.
  if (m_numFound > 0) {
    if (m_numFound == 1) {
      pDbx->SelectFindEntry(m_indices[0], TRUE);
    } else {
      m_lastshown++;
      if (m_lastshown >= m_numFound) {
        cs_status.LoadString(IDS_SEARCHWRAPPED);
        m_lastshown = 0;
      } else
        cs_status.Format(IDS_FOUNDMATCHES, m_lastshown + 1, m_numFound);
      pDbx->SelectFindEntry(m_indices[m_lastshown], TRUE);
    }
  }
  if (m_numFound == 0)
    m_findresults.SetColour(RGB(255, 0, 0));
  else
    m_findresults.ResetColour();

  m_findresults.SetWindowText(cs_status);
  Invalidate();
}

void CPWFindToolBar::ShowFindAdvanced()
{
  CAdvancedDlg Adv(this, ADV_FIND, m_bsFields, m_subgroup_name, m_subgroup_set,
    m_subgroup_object, m_subgroup_function);

  app.DisableAccelerator(); // don't allow accelerator keys
  INT_PTR rc = Adv.DoModal();
  app.EnableAccelerator();  // allow accelerator keys again

  if (rc == IDOK) {
    m_bAdvanced = true;
    m_bsFields = Adv.m_bsFields;
    m_subgroup_set = Adv.m_subgroup_set;
    if (m_subgroup_set == BST_CHECKED) {
      m_subgroup_name = Adv.m_subgroup_name;
      m_subgroup_object = Adv.m_subgroup_object;
      m_subgroup_function = Adv.m_subgroup_function;
    }
  } else {
    m_bAdvanced = false;
  }
}
