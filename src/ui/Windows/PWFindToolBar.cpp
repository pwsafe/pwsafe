/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// PWFindToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "ThisMfcApp.h" // for disable/enable accel.
#include "DboxMain.h"

#include "PWFindToolBar.h"
#include "ControlExtns.h"
#include "Fonts.h"

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
* with its new offset in the bitmap arrays.
* Also, the Case Sensitive bitmap must be the last bitmap in the arrays.
*/

#define EDITCTRL_WIDTH 100    // width of Edit control for Search Text
#define FINDRESULTS_WIDTH 400 // width of Edit control for Search Results

const UINT CPWFindToolBar::m_FindToolBarIDs[] = {
  ID_TOOLBUTTON_CLOSEFIND,
  ID_TOOLBUTTON_FINDEDITCTRL,
  ID_MENUITEM_FIND,
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
  IDB_FINDADVANCED_CLASSIC,     // m_iCase_Advanced_BM_offset contains this offset
  IDB_FINDREPORT_CLASSIC,
  IDB_FINDCLEAR_CLASSIC,
  IDB_FINDCTRLPLACEHOLDER,

  // Must be after normal bitmaps for buttons etc.
  IDB_FINDCASE_S_CLASSIC,       // m_iCase_ISensitive_BM_offset = 1 is this offset
  IDB_FINDADVANCEDON_CLASSIC,   // m_iCase_Advanced_BM_offset + 1 is this offset
};

const UINT CPWFindToolBar::m_FindToolBarNewBMs[] = {
  IDB_FINDCLOSE_NEW,
  IDB_FINDCTRLPLACEHOLDER,
  IDB_FIND_NEW,
  IDB_FINDCASE_I_NEW,           // m_iCase_Insensitive_BM_offset contains this offset
  IDB_FINDADVANCED_NEW,         // m_iAdvanced_BM_offset contains this offset
  IDB_FINDREPORT_NEW,
  IDB_FINDCLEAR_NEW,
  IDB_FINDCTRLPLACEHOLDER,

  // Must be after normal bitmaps for buttons etc.
  IDB_FINDCASE_S_NEW,           // m_iCase_Sensitive_BM_offset is this offset
  IDB_FINDADVANCEDON_NEW,       // m_iAdvancedOn_BM_offset is this offset
};

// CFindEditCtrl

CFindEditCtrl::CFindEditCtrl()
{
}

CFindEditCtrl::~CFindEditCtrl()
{
}

BEGIN_MESSAGE_MAP(CFindEditCtrl, CEditExtn)
END_MESSAGE_MAP()

LRESULT CFindEditCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  if (message == WM_SYSCOMMAND && wParam == SC_KEYMENU) {
    // This should be an entry keyboard shortcut!
    CWnd *pWnd = GetFocus();
    if (pWnd == NULL)
      return 0L;

    // But only if we have Focus
    if (pWnd->GetDlgCtrlID() != ID_TOOLBUTTON_FINDEDITCTRL)
      goto exit;

    // Need base character excluding special keys
    short siKeyStateVirtualKeyCode = VkKeyScan(lParam & 0xff);
    WORD wVirtualKeyCode = siKeyStateVirtualKeyCode & 0xff;

    if (wVirtualKeyCode != 0) {
      WORD wWinModifiers(0);
      if (GetKeyState(VK_CONTROL) & 0x8000)
        wWinModifiers |= MOD_CONTROL;

      if (GetKeyState(VK_MENU) & 0x8000)
        wWinModifiers |= MOD_ALT;

      if (GetKeyState(VK_SHIFT) & 0x8000)
        wWinModifiers |= MOD_SHIFT;

      if (!app.GetMainDlg()->ProcessEntryShortcut(wVirtualKeyCode, wWinModifiers))
        return 0;
    }
  }

exit:
  return CEditExtn::WindowProc(message, wParam, lParam);
}

// CPWFindToolBar

IMPLEMENT_DYNAMIC(CPWFindToolBar, CToolBar)

CPWFindToolBar::CPWFindToolBar()
  : m_bitmode(1), m_bVisible(true), 
  m_bCaseSensitive(false), m_bAdvanced(false),
  m_lastshown(size_t(-1)), m_numFound(size_t(-1)),
  m_last_search_text(L""), m_last_cs_search(false),
  m_subgroup_name(L""), m_subgroup_bset(false),
  m_subgroup_object(CItemData::GROUP), m_subgroup_function(0),
  m_last_subgroup_name(L""), m_last_subgroup_bset(false),
  m_last_subgroup_object(CItemData::GROUP), m_last_subgroup_function(0),
  m_iCase_Insensitive_BM_offset(-1), m_iCase_Sensitive_BM_offset(-1),
  m_iAdvanced_BM_offset(-1), m_iAdvancedOn_BM_offset(-1),
  m_iFindDirection(FIND_DOWN), m_bFontSet(false), m_bUseSavedFindValues(false)
{
  m_last_bsFields.reset();
  m_last_bsAttFields.reset();

  m_iMaxNumButtons = _countof(m_FindToolBarIDs);
  m_pOriginalTBinfo = new TBBUTTON[m_iMaxNumButtons];

  static_assert(_countof(m_FindToolBarClassicBMs) == _countof(m_FindToolBarNewBMs),
                "FindToolBar bitmap array mismatch");

  m_iNum_Bitmaps = _countof(m_FindToolBarClassicBMs);
}

CPWFindToolBar::~CPWFindToolBar()
{
  delete[] m_pOriginalTBinfo;

  m_ImageLists[0].DeleteImageList();
  m_ImageLists[1].DeleteImageList();
  m_ImageLists[2].DeleteImageList();
}

void CPWFindToolBar::OnDestroy()
{
  if (m_edtFindText.GetSafeHwnd() != NULL) {
    m_edtFindText.DestroyWindow();
  }

  if (m_stcFindResults.GetSafeHwnd() != NULL) {
    m_stcFindResults.DestroyWindow();
  }
}

BEGIN_MESSAGE_MAP(CPWFindToolBar, CToolBar)
  ON_WM_DESTROY()
  ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

HBRUSH CPWFindToolBar::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CToolBar::OnCtlColor(pDC, pWnd, nCtlColor);

  // Only deal with Static controls and then
  // Only with our special one - change colour of warning message
  if (nCtlColor == CTLCOLOR_STATIC && pWnd->GetDlgCtrlID() == ID_TOOLBUTTON_FINDEDITCTRL) {
    if (((CStaticExtn *)pWnd)->GetColourState()) {
      COLORREF cfUser = ((CStaticExtn *)pWnd)->GetUserColour();
      pDC->SetTextColor(cfUser);
    }
  }
  return hbr;
}

// CPWFindToolBar message handlers

void CPWFindToolBar::RefreshImages()
{
  m_ImageLists[0].DeleteImageList();
  m_ImageLists[1].DeleteImageList();
  m_ImageLists[2].DeleteImageList();

  Init(m_NumBits, m_iWMSGID, m_pst_SADV);

  ChangeImages(m_toolbarMode);
}

BOOL CPWFindToolBar::PreTranslateMessage(MSG *pMsg)
{
  CWnd *pWnd = FromHandle(pMsg->hwnd);

  // Process User's AutoType shortcut
  if (app.GetMainDlg()->CheckPreTranslateAutoType(pMsg))
    return TRUE;

  if (pWnd->GetDlgCtrlID() == ID_TOOLBUTTON_FINDEDITCTRL) {
    if (pMsg->message == WM_KEYDOWN) {
      if (pMsg->wParam == VK_RETURN) {
        app.GetMainDlg()->SendMessage(m_iWMSGID);
        return TRUE;
      }
      if (pMsg->wParam == VK_DELETE) {
        CPoint pt_cursor = m_edtFindText.GetCaretPos();
        int iCaret = m_edtFindText.CharFromPos(pt_cursor);
        int iCharIndex = LOWORD(iCaret);
        int iTextLen = m_edtFindText.GetWindowTextLength();
        int iStartChar, iEndChar;
        m_edtFindText.GetSel(iStartChar, iEndChar);
        if (iCharIndex == iStartChar && iCharIndex == iEndChar) {
          // Nothing selected - forward backspace
          m_edtFindText.SetSel(iCharIndex, iCharIndex + 1);
          m_edtFindText.ReplaceSel(L"");
        } else if (iTextLen > (iEndChar - iStartChar)) {
          m_edtFindText.ReplaceSel(L"");
        } else {
          m_edtFindText.SetWindowText(L"");
        }
        m_edtFindText.Invalidate();
        return TRUE;
      }
    }
  }
  return CToolBar::PreTranslateMessage(pMsg);
}

//  Other routines

void CPWFindToolBar::Init(const int NumBits, int iWMSGID,
                          st_SaveAdvValues *pst_SADV)
{
  ASSERT(pst_SADV != NULL);
  
  int i, j;

  COLORREF crClassicBackground = RGB(192, 192, 192);
  COLORREF crNewBackground = RGB(192, 192, 192);
  UINT iClassicFlags = ILC_MASK | ILC_COLOR8;
  UINT iNewFlags1 = ILC_MASK | ILC_COLOR8;
  UINT iNewFlags2 = ILC_MASK | ILC_COLOR32;

  m_pst_SADV = pst_SADV;
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
    bmTemp.DeleteObject();
    if (m_FindToolBarClassicBMs[i] == IDB_FINDCASE_I_CLASSIC)
      m_iCase_Insensitive_BM_offset = i;
    if (m_FindToolBarClassicBMs[i] == IDB_FINDCASE_S_CLASSIC)
      m_iCase_Sensitive_BM_offset = i;
    if (m_FindToolBarClassicBMs[i] == IDB_FINDADVANCED_CLASSIC)
      m_iAdvanced_BM_offset = i;
    if (m_FindToolBarClassicBMs[i] == IDB_FINDADVANCEDON_CLASSIC)
      m_iAdvancedOn_BM_offset = i;
  }

  for (i = 0; i < m_iNum_Bitmaps; i++) {
    bmTemp.LoadBitmap(m_FindToolBarNewBMs[i]);
    m_ImageLists[1].Add(&bmTemp, crNewBackground);
    bmTemp.DeleteObject();
  }

  for (i = 0; i < m_iNum_Bitmaps; i++) {
    bmTemp.LoadBitmap(m_FindToolBarNewBMs[i]);
    m_ImageLists[2].Add(&bmTemp, crNewBackground);
    bmTemp.DeleteObject();
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

  m_iWMSGID = iWMSGID;
}

void CPWFindToolBar::ChangeFont()
{
  // User has changed the Add/Edit font
  m_FindTextFont.DeleteObject();

  CRect rt;
  GetItemRect(0, &rt);
  const int iBtnHeight = rt.Height();

  // Use Add/Edit font rather than Tree/List font to determine height
  LOGFONT lf = { 0 };
  Fonts::GetInstance()->GetAddEditFont(&lf);
  VERIFY(m_FindTextFont.CreateFontIndirect(&lf));
  m_edtFindText.SetFont(&m_FindTextFont);

  bool bFontChanged(false);

  do {
    // Does it fit?  If not, keep reducing point size until it does.
    TEXTMETRIC tm;
    HDC hDC = ::GetDC(m_edtFindText);

    HFONT hFontOld = (HFONT)SelectObject(hDC, m_FindTextFont.GetSafeHandle());

    GetTextMetrics(hDC, &tm);
    LONG iFontHeight = tm.tmHeight + tm.tmExternalLeading;

    if (iFontHeight > iBtnHeight) {
      if (lf.lfHeight < 0) {
        int iPointSize = -::MulDiv(lf.lfHeight, 72, GetDeviceCaps(hDC, LOGPIXELSY));
        iPointSize--;
        lf.lfHeight = -MulDiv(iPointSize, ::GetDeviceCaps(hDC, LOGPIXELSY), 72);
      } else {
        int iPointSize = ::MulDiv(lf.lfHeight - tm.tmInternalLeading, 72, GetDeviceCaps(hDC, LOGPIXELSY));
        iPointSize--;
        lf.lfHeight = tm.tmInternalLeading + MulDiv(iPointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
      }

      m_FindTextFont.DeleteObject();
      VERIFY(m_FindTextFont.CreateFontIndirect(&lf));
      m_edtFindText.SetFont(&m_FindTextFont);

      bFontChanged = true;
    } else {
      bFontChanged = false;
    }

    SelectObject(hDC, hFontOld);
    ::ReleaseDC(NULL, hDC);
  } while(bFontChanged);

  m_bFontSet = true;
}

void CPWFindToolBar::LoadDefaultToolBar(const int toolbarMode)
{
  CToolBarCtrl& tbCtrl = GetToolBarCtrl();
  m_toolbarMode = toolbarMode;
  const int nImageListNum = (m_toolbarMode == ID_MENUITEM_OLD_TOOLBAR) ? 0 : m_bitmode;
  tbCtrl.SetImageList(&m_ImageLists[nImageListNum]);

  tbCtrl.AddButtons(m_iMaxNumButtons, &m_pOriginalTBinfo[0]);

  TBBUTTONINFO tbinfo = {0};
  tbinfo.cbSize = sizeof(tbinfo);
  tbinfo.dwMask = TBIF_STYLE;

  tbCtrl.GetButtonInfo(ID_TOOLBUTTON_FINDCASE_I, &tbinfo);
  tbinfo.fsStyle = TBSTYLE_CHECK;
  tbCtrl.SetButtonInfo(ID_TOOLBUTTON_FINDCASE_I, &tbinfo);

  tbCtrl.GetButtonInfo(ID_TOOLBUTTON_FINDADVANCED, &tbinfo);
  tbinfo.fsStyle = TBSTYLE_CHECK;
  tbCtrl.SetButtonInfo(ID_TOOLBUTTON_FINDADVANCED, &tbinfo);

  AddExtraControls();
  tbCtrl.AutoSize();
}

void CPWFindToolBar::AddExtraControls()
{
  CRect rect, rt;
  int index;

  GetItemRect(0, &rt);
  const int iBtnHeight = rt.Height();
  const int iHeight = iBtnHeight;

  // Add find search text CEdit control (CEditExtn)
  // Get the index of the placeholder's position in the toolbar
  index = CommandToIndex(ID_TOOLBUTTON_FINDEDITCTRL);
  ASSERT(index != -1);

  // If we have been here before, destroy it first
  if (m_edtFindText.GetSafeHwnd() != NULL) {
    if (m_bFontSet) {
      m_FindTextFont.DeleteObject();
      m_bFontSet = false;
    }
    m_edtFindText.DestroyWindow();
  }

  // Convert that button to a separator
  SetButtonInfo(index, ID_TOOLBUTTON_FINDEDITCTRL, TBBS_SEPARATOR, EDITCTRL_WIDTH);

  // Note: "ES_WANTRETURN | ES_MULTILINE".  This is to allow the return key to be
  // trapped by PreTranslateMessage and treated as if the Find button had been
  // pressed
  rect = CRect(0, 0, EDITCTRL_WIDTH, iHeight);
  VERIFY(m_edtFindText.Create(WS_CHILD | WS_VISIBLE |
                           ES_AUTOHSCROLL | ES_LEFT | ES_WANTRETURN | ES_MULTILINE,
                           CRect(rect.left + 2, rect.top, rect.right - 2, rect.bottom),
                           this, ID_TOOLBUTTON_FINDEDITCTRL));

  GetItemRect(index, &rect);
  m_edtFindText.SetWindowPos(NULL, rect.left + 2, rect.top , 0, 0,
                          SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOCOPYBITS);

  // Set font
  ChangeFont();

  // Add find search results CStatic control
  // Get the index of the placeholder's position in the toolbar
  index = CommandToIndex(ID_TOOLBUTTON_FINDRESULTS);
  ASSERT(index != -1);

  // If we have been here before, destroy it first
  if (m_stcFindResults.GetSafeHwnd() != NULL) {
    m_FindResultsFont.DeleteObject();
    m_stcFindResults.DestroyWindow();
  }

  // Convert that button to a separator
  SetButtonInfo(index, ID_TOOLBUTTON_FINDRESULTS, TBBS_SEPARATOR, FINDRESULTS_WIDTH);

  rect = CRect(0, 0, FINDRESULTS_WIDTH, iHeight);
  VERIFY(m_stcFindResults.Create(L"", WS_CHILD | WS_VISIBLE |
                              SS_LEFTNOWORDWRAP | SS_CENTERIMAGE,
                              CRect(rect.left + 2, rect.top, rect.right - 2, rect.bottom),
                              this, ID_TOOLBUTTON_FINDEDITCTRL));

  GetItemRect(index, &rect);
  m_stcFindResults.SetWindowPos(NULL, rect.left + 2, rect.top, 0, 0,
                             SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOCOPYBITS);

  NONCLIENTMETRICS ncm;
  ncm.cbSize = sizeof(NONCLIENTMETRICS);
  if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0)) {
    m_FindResultsFont.CreateFontIndirect(&ncm.lfMessageFont);
  }
}

void CPWFindToolBar::ShowFindToolBar(bool bShow)
{
  if (this->GetSafeHwnd() == NULL)
    return;

  if (bShow) {
    CRect rt;
    GetItemRect(0, &rt);
    const int iBtnHeight = rt.Height();

    /**
     * Bjorne's suggestion: Set Find fonts to tree/list view font
     * However, from BR 1371 it is better to use the Add/Edit font
     * which is already used for CEdit controls.
     *
     * Unfortunately, as unable to change size of the CEdit controls so
     * that the text fits, we will dynamically reduce the font size
     * until it fits in the CEdit controls.
     *
     * We also do this if the user changes the Add/Edit font.
     *
     * Note: It is better this way as we did not do any sanity checks that
     * the user hadn't set an enormous point size e.g. 72!
     */
    if (!m_bFontSet) {
      ChangeFont();
    }

    SetHeight(iBtnHeight + 4);  // Add border
    m_edtFindText.ChangeColour();
    m_edtFindText.SetWindowPos(NULL, 0, 0, EDITCTRL_WIDTH, iBtnHeight,
                            SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

    m_edtFindText.SetSel(0, -1);  // Select all text
    m_edtFindText.Invalidate();

    m_stcFindResults.SetWindowPos(NULL, 0, 0, FINDRESULTS_WIDTH, iBtnHeight,
                               SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
  }

  GetToolBarCtrl().AutoSize();

  ::ShowWindow(this->GetSafeHwnd(), bShow ? SW_SHOW : SW_HIDE);
  ::EnableWindow(this->GetSafeHwnd(), bShow ? TRUE : FALSE);
  m_edtFindText.SetFocus();

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

  TBBUTTONINFO tbinfo = {0};
  tbinfo.cbSize = sizeof(tbinfo);
  tbinfo.dwMask = TBIF_IMAGE;
  tbCtrl.GetButtonInfo(ID, &tbinfo);

  tbinfo.iImage = m_bCaseSensitive ? m_iCase_Sensitive_BM_offset : m_iCase_Insensitive_BM_offset;
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

  m_edtFindText.SetWindowText(L"");
  m_stcFindResults.ResetColour();
  m_stcFindResults.SetWindowText(L"");

  m_bCaseSensitive = m_last_cs_search = false;
  m_bAdvanced = false;
  m_numFound = size_t(-1);
  m_last_search_text = L"";
  m_subgroup_name = m_last_subgroup_name = L"";
  m_subgroup_bset = m_last_subgroup_bset;
  m_subgroup_object = m_subgroup_function = 0;
  m_last_subgroup_object = m_last_subgroup_function = 0;
  m_lastshown = size_t(-1);
  m_vIndices.clear();
  m_vFoundUUIDs.clear();

  app.GetMainDlg()->SetFilterFindEntries(NULL);
}

int CPWFindToolBar::GetLastSelectedFoundItem(pws_os::CUUID &entry_uuid)
{
  entry_uuid = pws_os::CUUID::NullUUID();

  if (m_lastshown >= 0 && m_lastshown < (int)m_vFoundUUIDs.size()) {
    entry_uuid = m_vFoundUUIDs[m_lastshown];
    return (int)m_lastshown;
  }

  return -1;
}

void CPWFindToolBar::Find()
{
  if (this->GetSafeHwnd() == NULL)
    return;

  CString cs_status, cs_temp;
  m_edtFindText.GetWindowText(m_search_text);

  if (m_search_text.IsEmpty()) {
    cs_status.LoadString(IDS_ENTERSEARCHSTRING);
    m_stcFindResults.SetColour(RGB(255, 0, 0));
    m_stcFindResults.SetWindowText(cs_status);
    return;
  }

  m_cs_search = m_bCaseSensitive;

  // If the user changes the search text or cs, then this is a new search:
  if (m_search_text != m_last_search_text ||
      m_cs_search != m_last_cs_search ||
      m_pst_SADV->bsFields != m_last_bsFields ||
      m_pst_SADV->bsAttFields != m_last_bsAttFields ||
      m_pst_SADV->subgroup_name != m_last_subgroup_name ||
      m_pst_SADV->subgroup_bset != m_last_subgroup_bset ||
      m_pst_SADV->subgroup_object != m_last_subgroup_object ||
      m_pst_SADV->subgroup_function != m_last_subgroup_function) {
    m_last_search_text = m_search_text;
    m_last_cs_search = m_cs_search;
    m_last_bsFields = m_pst_SADV->bsFields;
    m_last_bsAttFields = m_pst_SADV->bsAttFields;
    m_last_subgroup_name = m_pst_SADV->subgroup_name;
    m_last_subgroup_bset = m_pst_SADV->subgroup_bset;
    m_last_subgroup_object = m_pst_SADV->subgroup_object;
    m_last_subgroup_function = m_pst_SADV->subgroup_function;
    m_lastshown = size_t(-1);
  }

  if (m_lastshown == size_t(-1)) {
    m_vIndices.clear();
    m_vFoundUUIDs.clear();

    if (m_bAdvanced)
      m_numFound = app.GetMainDlg()->FindAll(m_search_text, m_cs_search, m_vIndices, m_vFoundUUIDs,
                                 m_pst_SADV->bsFields, m_pst_SADV->bsAttFields,
                                 m_pst_SADV->subgroup_bset,
                                 m_pst_SADV->subgroup_name, m_pst_SADV->subgroup_object, 
                                 m_pst_SADV->subgroup_function);
    else
      m_numFound = app.GetMainDlg()->FindAll(m_search_text, m_cs_search, m_vIndices, m_vFoundUUIDs);

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
    app.GetMainDlg()->ResumeOnDBNotification();
  } // m_lastshown == size_t(-1)

  // OK, so now we have a (possibly empty) list of items to select.
  if (m_numFound > 0) {
    if (m_numFound == 1) {
      app.GetMainDlg()->SelectFindEntry(m_vIndices[0], TRUE);
    } else { // m_numFound > 1
      if (m_iFindDirection == FIND_DOWN) {
        m_lastshown++;
      } else {
        if (m_lastshown != size_t(-1)) // prevent m_lastshown <- -2 !
          m_lastshown--;
      }
      if (m_iFindDirection == FIND_DOWN && m_lastshown >= m_numFound) {
        cs_temp.LoadString(IDS_SEARCHTOP);
        cs_status.Format(IDS_SEARCHWRAPPED, static_cast<LPCWSTR>(cs_temp));
        m_lastshown = 0;
      } else
        if (m_iFindDirection == FIND_UP && m_lastshown == size_t(-1)) {
        cs_temp.LoadString(IDS_SEARCHBOTTOM);
        cs_status.Format(IDS_SEARCHWRAPPED, static_cast<LPCWSTR>(cs_temp));
        m_lastshown = m_numFound - 1;
      } else
        cs_status.Format(IDS_FOUNDMATCHES, m_lastshown + 1, m_numFound);

      app.GetMainDlg()->SelectFindEntry(m_vIndices[m_lastshown], TRUE);
    }
  }

  if (m_numFound == 0) {
    m_stcFindResults.SetColour(RGB(255, 0, 0));
    app.GetMainDlg()->SetFilterFindEntries(NULL);
  } else {
    m_stcFindResults.ResetColour();
    app.GetMainDlg()->SetFilterFindEntries(&m_vFoundUUIDs);
  }

  m_stcFindResults.SetWindowText(cs_status);
  Invalidate();
}

void CPWFindToolBar::Find(const int ilastshown)
{
  if (ilastshown >= 0 && ilastshown < (int)m_vIndices.size()) {
    CString cs_status;
    m_lastshown = ilastshown;
    cs_status.Format(IDS_FOUNDMATCHES, m_lastshown + 1, m_numFound);
    m_stcFindResults.SetWindowText(cs_status);
    Invalidate();

    app.GetMainDlg()->SelectFindEntry(m_vIndices[m_lastshown], TRUE);
  }
}

void CPWFindToolBar::ShowFindAdvanced()
{
  CToolBarCtrl& tbCtrl = GetToolBarCtrl();

  TBBUTTONINFO tbinfo = {0};
  tbinfo.cbSize = sizeof(tbinfo);
  tbinfo.dwMask = TBIF_STATE;

  tbCtrl.GetButtonInfo(ID_TOOLBUTTON_FINDADVANCED, &tbinfo);

  bool bAdvanced = (tbinfo.fsState & TBSTATE_CHECKED) == TBSTATE_CHECKED;

  if (bAdvanced) {
    const CItemData::FieldBits old_bsFields(m_pst_SADV->bsFields);
    const CItemAtt::AttFieldBits old_bsAttFields(m_pst_SADV->bsAttFields);
    const std::wstring old_subgroup_name(m_pst_SADV->subgroup_name);
    const bool old_subgroup_bset(m_pst_SADV->subgroup_bset);
    const int old_subgroup_object(m_pst_SADV->subgroup_object);
    const int old_subgroup_function(m_pst_SADV->subgroup_function);

    CAdvancedDlg Adv(this, CAdvancedDlg::FIND, m_pst_SADV);

    INT_PTR rc = Adv.DoModal();

    if (rc == IDOK) {
      m_pst_SADV->bsFields = Adv.m_bsFields;
      m_pst_SADV->bsAttFields = Adv.m_bsAttFields;
      m_pst_SADV->subgroup_bset = Adv.m_subgroup_set == BST_CHECKED;
      if (m_pst_SADV->subgroup_bset) {
        m_pst_SADV->subgroup_name = Adv.m_subgroup_name;
        m_pst_SADV->subgroup_object = Adv.m_subgroup_object;
        m_pst_SADV->subgroup_function = Adv.m_subgroup_function;
      }
 
      // Check if anything changed
      if (old_bsFields != m_pst_SADV->bsFields ||
          old_bsAttFields != m_pst_SADV->bsAttFields ||
          old_subgroup_bset != m_pst_SADV->subgroup_bset ||
          (old_subgroup_bset == m_pst_SADV->subgroup_bset &&
           old_subgroup_bset &&
           (old_subgroup_name != m_pst_SADV->subgroup_name ||
            old_subgroup_object != m_pst_SADV->subgroup_object ||
            old_subgroup_function != m_pst_SADV->subgroup_function))) {
        m_stcFindResults.ResetColour();
        m_stcFindResults.SetWindowText(L"");

        m_numFound = size_t(-1);
        m_lastshown = size_t(-1);
        m_vIndices.clear();
        m_vFoundUUIDs.clear();
      }
    } else {
      bAdvanced = false;
    }
  }

  if ((m_bAdvanced != bAdvanced) || !bAdvanced) {
    // State has changed or user doesn't want advanced selection criteria!
    m_stcFindResults.ResetColour();
    m_stcFindResults.SetWindowText(L"");

    m_numFound = size_t(-1);
    m_lastshown = size_t(-1);
    m_vIndices.clear();
    m_vFoundUUIDs.clear();
  }

  // Set new state
  m_bAdvanced = bAdvanced;

  // Now set button state
  tbCtrl.CheckButton(ID_TOOLBUTTON_FINDADVANCED, m_bAdvanced ? TRUE : FALSE);

  tbinfo.cbSize = sizeof(tbinfo);
  tbinfo.dwMask = TBIF_IMAGE;
  tbCtrl.GetButtonInfo(ID_TOOLBUTTON_FINDADVANCED, &tbinfo);

  tbinfo.iImage = m_bAdvanced ? m_iAdvancedOn_BM_offset : m_iAdvanced_BM_offset;
  tbinfo.dwMask = TBIF_IMAGE;
  tbCtrl.SetButtonInfo(ID_TOOLBUTTON_FINDADVANCED, &tbinfo);
}
