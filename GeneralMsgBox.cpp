/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

// GeneralMsgBox.cpp
/*
 * GeneralMsgBox.h
 *
 * Implements the extended Message Box class.
 *
 * This is a cut down version of TcxMsgBox by Thales P. Carvalho but then
 * significantly enhanced to support text with HTML formatting and links.
 * instead of a RTF string.
 * See www.codeproject.com for the orgininal code
*/


/*
  Supports the following HTML formatting:
    Bold:          <b>.....</b>
    Italic:        <i>.....</i>
    Underline:     <u>.....</u>
    Font:          <font face="FN" size="S" color="C">.....</font>
    Web reference: <a href="...url...">Friendly Name</a>

  Notes:
  1. If you wish to use the "less than" or "greater than" symbols in the
     message, use "&lt;" and "&gt;" instead.  These will be converted to the
     '<' and '>' symbols in the final text string.

  2. Bold, Italic, Underline and Font can be nested but MUST NOT overlap - i.e.
     "<b>bold</b> and <i>italic</i>" is OK
     "<b>bold and <i>bold & italic</i> and more bold</b>" is OK
     "<b>bold and <i>bold italic</b> & italic</i>" is NOT OK, since the bold end
      tag is in the middle of the italic tags.

  3. Fonts can also be nested. i.e.
     <font face="FN1">test1<font face="FN2">test2<font color="Red">red2</font></font></font>

  4. Any unsupported HTML tags will be retained e.g. "<q>test</q>" will be
     still be "<q>test</q>" in the final text string.
*/

#include "stdafx.h"
#include "GeneralMsgBox.h"
#include <RichEdit.h>
#include <algorithm>
#include <vector>
#include <string>
#include <bitset>
#include <stack>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Local constants

/*
  The following _dlgData corresponds to:

  DLGTEMPLATEEX dtex;

  dtex.wDlgVer = 1;
  dtex.wSignature = 0xFFFF;
  dtex.dwHelpID = 0;
  dtex.dwStyle = WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | DS_SETFONT | DS_MODALFRAME;
  dtex.dwExStyle = WS_EX_APPWINDOW;
  dtex.cDlgItems = 0;
  dtex.x = 0;
  dtex.y = 0;
  dtex.cx = 185;
  dtex.cy = 92;
  no menu ( (WORD)0 )
  no windowClass ( (WORD)0 )
  caption L"."
  point size (only if DS_SETFONT specified)
  weight                    "
  italic                    "
  charset                   "
  typeface                  "
*/

static const BYTE _dlgData[] =
{
  0x01, 0x00,                                      // wDlgVer (must be on DWORD boundary)
  0xff, 0xff,                                      // wSignature
  0x00, 0x00, 0x00, 0x00,                          // dwhelpID
  0x00, 0x00, 0x04, 0x00,                          // dsStyle
  0xc0, 0x00, 0xc8, 0x90,                          // dwExStyle
  0x00, 0x00,                                      // cDlgItems
  0x00, 0x00,                                      // x
  0x00, 0x00,                                      // y
  0xb9, 0x00,                                      // cx
  0x5c, 0x00,                                      // cy
  0x00, 0x00,                                      // no menu (must be on WORD boundary)
  0x00, 0x00,                                      // no windowClass
  0x2e, 0x00, 0x00, 0x00,                          // Caption = '.\0'
  0x08, 0x00,                                      // point size
  0x00, 0x00,                                      // weight
  0x00,                                            // italic
  0x01,                                            // charset
                                                   // typeface (must be on WORD boundary)
  0x4d, 0x00, 0x53, 0x00, 0x20, 0x00, 0x53, 0x00,  // 'MS S'
  0x61, 0x00, 0x6e, 0x00, 0x73, 0x00, 0x20, 0x00,  // 'ans '
  0x53, 0x00, 0x65, 0x00, 0x72, 0x00, 0x69, 0x00,  // 'Seri'
  0x66, 0x00, 0x00, 0x00                           // 'f\0'
};

/////////////////////////////////////////////////////////////////////////////
// CGeneralMsgBox

// Constructor
CGeneralMsgBox::CGeneralMsgBox(CWnd* pParentWnd)
  : m_uDefCmdId((UINT)IDC_STATIC), m_uEscCmdId((UINT)IDC_STATIC),
  m_hIcon(NULL), m_strTitle(AfxGetApp()->m_pszAppName)
{
  m_pParentWnd = pParentWnd;
  // default metric values
  const int _aDefMetrics[NUM_OF_METRICS] =
  {
     10,   // CX_LEFT_BORDER,
     10,   // CX_RIGHT_BORDER,
     10,   // CY_TOP_BORDER,
     7,    // CY_BOTTOM_BORDER,

     10,   // CX_ICON_MSG_SPACE,
     7,    // CY_BTNS_MSG_SPACE,

     7,    // CX_BTN_BORDER,
     4,    // CY_BTN_BORDER,

     7,    // CX_BTNS_SPACE,
     40,   // CX_MIN_BTN,
  };

  for (int i = 0; i < NUM_OF_METRICS; ++i) {
    m_aMetrics[i] = _aDefMetrics[i];
  }
}

// Destructor
CGeneralMsgBox::~CGeneralMsgBox()
{
}

// Replaces CDialog::DoModal
int CGeneralMsgBox::DoModal()
{
  InitModalIndirect((LPCDLGTEMPLATE)_dlgData, m_pParentWnd);
  return CDialog::DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CGeneralMsgBox - Button operations

// Add a button
void CGeneralMsgBox::AddButton(
  UINT uIDC,           // button command ID
  LPCTSTR pszText,     // button text
  BOOL bIsDefault,     // set the button as default
  BOOL bIsEscape       // return this command if user press escape
 )
{
  ASSERT(uIDC != (UINT)IDC_STATIC);

  BTNDATA btndata;
  btndata.uIDC = uIDC;
  btndata.strBtn = pszText;

  m_aBtns.Add(btndata);

  if (bIsEscape)
    m_uEscCmdId = uIDC;

  if (bIsDefault)
    m_uDefCmdId = uIDC;
}

// Add a button
void CGeneralMsgBox::AddButton(
  UINT uIDC,           // button command ID
  UINT uIdText,        // string ID of button's text
  BOOL bIsDefault,     // set the button as default
  BOOL bIsEscape       // return this command if user press escape
 )
{
  CString str;

  if (uIdText == (UINT)-1)
    uIdText = uIDC;

  VERIFY(str.LoadString(uIdText));

  AddButton(uIDC, str, bIsDefault, bIsEscape);
}

/////////////////////////////////////////////////////////////////////////////
// CGeneralMsgBox - Plain text operations

BOOL CGeneralMsgBox::SetMsg(UINT uMsgId)
{
  return m_strMsg.LoadString(uMsgId);
}

BOOL CGeneralMsgBox::SetMsg(LPCTSTR pszMsg)
{
  m_strMsg = pszMsg;
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGeneralMsgBox - Icon operations

void CGeneralMsgBox::SetIcon(HICON hIcon)
{
  m_hIcon = hIcon;

  if (m_hIcon != NULL) {
    // loading the icon and extracting it's dimensions
    ICONINFO ii;
    GetIconInfo(m_hIcon, &ii);

    BITMAP bm;
    GetObject(ii.hbmColor, sizeof(bm), &bm);

    m_dimIcon.cx = bm.bmWidth;
    m_dimIcon.cy = bm.bmHeight;
    DeleteObject(ii.hbmColor);
    DeleteObject(ii.hbmMask);
  } else {
    m_dimIcon.cx = 0;
    m_dimIcon.cy = 0;
  }
}

void CGeneralMsgBox::SetIcon(UINT uIcon)
{
  SetIcon(AfxGetApp()->LoadIcon(uIcon));
}

void CGeneralMsgBox::SetStandardIcon(LPCTSTR pszIconName)
{
  SetIcon(AfxGetApp()->LoadStandardIcon(pszIconName));
}

void CGeneralMsgBox::SetStandardIcon(UINT uIcon)
{
  LPCTSTR pszIconName;
  switch (uIcon) {
    case MB_ICONEXCLAMATION:   // Also: MB_ICONWARNING
      pszIconName = IDI_WARNING;
      break;
    case MB_ICONASTERISK:      // Also: MB_ICONINFORMATION,
      pszIconName = IDI_INFORMATION;
      break;
    case MB_ICONQUESTION:
      pszIconName = IDI_QUESTION;
      break;
    case MB_ICONHAND:          // Also: ICONSTOP, MB_ICONERROR
      pszIconName = IDI_ERROR;
      break;
    default:
      pszIconName = IDI_ERROR; // to satisfy the compiler!
      ASSERT(0);
  }
  SetIcon(AfxGetApp()->LoadStandardIcon(pszIconName));
}

/////////////////////////////////////////////////////////////////////////////
// CGeneralMsgBox - Overrides

BOOL CGeneralMsgBox::OnInitDialog()
{
  if (!CDialog::OnInitDialog())
    return FALSE;

  SetWindowText(m_strTitle);

  // Getting the base dialog unit used in pixel <-> d.u. conversion

  CRect rc(0, 0, CX_DLGUNIT_BASE, CY_DLGUNIT_BASE);
  MapDialogRect(rc);

  m_dimDlgUnit = rc.Size();

  // Creating the nested controls

  CreateRtfCtrl();
  CreateIcon();
  CreateBtns();

  // Updating the layout - preparing to show

  UpdateLayout();

  // Disabling the ESC key

  if (m_uEscCmdId == (UINT)IDC_STATIC)
    ModifyStyle(WS_SYSMENU, NULL);

  // Focusing and setting the defaul button
  if (m_uDefCmdId != (UINT)IDC_STATIC) {
    GetDlgItem(m_uDefCmdId)->SetFocus();
    SetDefID(m_uDefCmdId);

    return FALSE;
  }

  return TRUE;
}

BOOL CGeneralMsgBox::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  if (message == WM_NOTIFY) {
    REQRESIZE* prr = (REQRESIZE*)lParam;
    if (prr->nmhdr.code == EN_REQUESTRESIZE) {
      // (1)
      // The rich edit control requested a resize.
      // Store the requested resize.
      m_dimMsg.cx = prr->rc.right - prr->rc.left;
      m_dimMsg.cy = prr->rc.bottom - prr->rc.top;

      *pResult = NULL;
      return TRUE;
    }
  } else if (message == WM_CLOSE) {
    if (m_uEscCmdId != (UINT)IDC_STATIC)
      EndDialog(m_uEscCmdId);

    return TRUE;
  }

  return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}

BOOL CGeneralMsgBox::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  if (nCode == CN_COMMAND) {
    if (pHandlerInfo == NULL && nID != (WORD)IDC_STATIC) {
      EndDialog(nID);
      return TRUE;
    }
  }

  return CDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CGeneralMsgBox::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN) {
    if (pMsg->wParam == VK_RETURN) {
      CWnd* pWnd = GetFocus();

      if (pWnd != NULL) {
        UINT uIDC = (UINT)pWnd->GetDlgCtrlID();

        for (int i = 0; i < m_aBtns.GetSize(); ++i)
          if (m_aBtns[i].uIDC == uIDC) {
            m_uDefCmdId = uIDC;
            break;
          }
      }

      EndDialog(m_uDefCmdId);

      return TRUE;
    }
    else if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_CANCEL) {
      if (m_uEscCmdId != (UINT)IDC_STATIC)
        EndDialog(m_uEscCmdId);

      return TRUE;
    }
  }

  return CDialog::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// CGeneralMsgBox - Overrides

// Return whether first starting point is greater than the second
bool CGeneralMsgBox::iStartCompare(st_format elem1, st_format elem2)
{
  if (elem1.iStart != elem2.iStart)
    return (elem1.iStart < elem2.iStart);

  return (elem1.iEnd < elem2.iEnd);
}

void CGeneralMsgBox::CreateRtfCtrl()
{
  // Creating the Rich Edit control

  CRect rcDummy; // dimesion doesn't matter here

  m_edCtrl.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_READONLY,
                  rcDummy, this, (UINT)IDC_STATIC);
  m_edCtrl.SetBackgroundColor(FALSE, ::GetSysColor(COLOR_3DFACE));
  m_edCtrl.SetFont(GetFont());

  m_strMsg.Trim();

  int iError;
  CString cs_text = GetTextFormatting(m_strMsg, iError);
  if (iError != 0) {
    // Had an error - show unchanged text
     m_edCtrl.SetWindowText(m_strMsg);
  } else {
    m_edCtrl.SetWindowText(cs_text);

    CHARFORMAT2 cf2;
    // Now apply formating
    if (!m_vFormat.empty()) {
      std::sort(m_vFormat.begin(), m_vFormat.end(), iStartCompare);
      std::vector<st_format>::const_iterator format_iter;
      memset((void *)&cf2, 0x00, sizeof(cf2));
      cf2.cbSize = sizeof(cf2);

      for (format_iter = m_vFormat.begin();
           format_iter != m_vFormat.end(); format_iter++) {
        m_edCtrl.SetSel(format_iter->iStart, format_iter->iEnd);
        m_edCtrl.GetSelectionCharFormat(cf2);
        if (format_iter->entrytype == Bold) {
          cf2.dwMask |= CFM_BOLD;
          cf2.dwEffects |= CFE_BOLD;
        } else if (format_iter->entrytype == Italic) {
          cf2.dwMask |= CFM_ITALIC;
          cf2.dwEffects |= CFE_ITALIC;
        } else if (format_iter->entrytype == Underline) {
          cf2.dwMask |= CFM_UNDERLINE;
          cf2.dwEffects |= CFE_UNDERLINE;
        } else if (format_iter->entrytype == Colour) {
          cf2.dwMask = CFM_COLOR;
          cf2.crTextColor = format_iter->cr;
          cf2.dwEffects &= ~CFE_AUTOCOLOR;
        } else if (format_iter->entrytype == Size) {
          cf2.dwMask = CFM_SIZE;
          cf2.yHeight = (format_iter->iSize) * 20;
        } else if (format_iter->entrytype == Name) {
          cf2.dwMask = CFM_FACE;
#if (_MSC_VER >= 1400)
          memcpy_s(cf2.szFaceName, LF_FACESIZE * sizeof(TCHAR),
                   format_iter->tcszFACENAME, LF_FACESIZE * sizeof(TCHAR));
#else
          memcpy(cf2.szFaceName, Name_iter->tcszFACENAME, LF_FACESIZE * sizeof(TCHAR));
#endif
        }
        m_edCtrl.SetSelectionCharFormat(cf2);
      }
    }

    if (!m_vALink.empty()) {
      std::vector<ALink>::const_iterator ALink_iter;
      memset((void *)&cf2, 0x00, sizeof(cf2));
      cf2.cbSize = sizeof(cf2);
      cf2.dwMask = CFM_LINK;
      cf2.dwEffects = CFE_LINK;

      for (ALink_iter = m_vALink.begin(); ALink_iter != m_vALink.end(); ALink_iter++) {
        m_edCtrl.SetSel(ALink_iter->iStart, ALink_iter->iEnd);
        m_edCtrl.SetSelectionCharFormat(cf2);
      }
    }
  }

  /////////////////////////////////////////////////////////
  // Calculating the best Rich Edit control dimension
  //
  // Note:
  //   There's no Rich Edit operation to get the minimum
  //   width it needs to show its contents without line
  //   breaks. However, given a width, we can check
  //   how much height it needs to show its text (see
  //   bottomless rich edit controls).
  //   We'll this mechanism to perform a binary search
  //   and find that best width value.

  m_edCtrl.SetEventMask(ENM_REQUESTRESIZE);

  m_dimMsg.cx = 0;
  m_dimMsg.cy = 0;

  // Performing a binary search for the best dimension

  int cxFirst = 0;
  int cxLast = ::GetSystemMetrics(SM_CXFULLSCREEN);
  int cyMax = 0;

  do {
    // taking a guess
    int cx = (cxFirst + cxLast) / 2;

    // testing it
    CRect rc(0, 0, cx, 1);
    m_edCtrl.MoveWindow(rc);
    m_edCtrl.SetRect(rc);
    m_edCtrl.RequestResize();

    // if it's the first guess, take it anyway
    if (cyMax == 0)
      cyMax = m_dimMsg.cy;

    // refining the guess
    if (m_dimMsg.cy > cyMax)
      cxFirst = cx + 1;
    else
      cxLast = cx - 1;
  }
  while(cxFirst < cxLast);
}

void CGeneralMsgBox::CreateBtns()
{
  // The minimum button's dimension
  m_dimBtn = CSize(FromDlgX(m_aMetrics[CX_MIN_BTN]), 0);

  // DC and Font for use in some dimension calculations
  CClientDC dc(this);

  CFont* pWndFont = GetFont();
  dc.SelectObject(pWndFont);

  CRect rcDummy; // dimesion doesn't matter here

  int cBtns = m_aBtns.GetSize();

  for (int i = 0; i < cBtns; ++i) {
    BTNDATA& btndata = m_aBtns[i];

    // Finding the minimum dimension needed to
    // properly show any button

    CSize dimBtn = dc.GetTextExtent(btndata.strBtn);

    m_dimBtn.cx = max(m_dimBtn.cx, dimBtn.cx);
    m_dimBtn.cy = max(m_dimBtn.cy, dimBtn.cy);

    // Creating the button with MFC's CButton help.

    CButton btnCtrl;

    btnCtrl.Create(btndata.strBtn,
       WS_CHILD|WS_VISIBLE|WS_TABSTOP,
         rcDummy, this, btndata.uIDC);

    btnCtrl.SetFont(pWndFont);

    btnCtrl.UnsubclassWindow();
  }

  // Add the button margins
  m_dimBtn.cx += 2 * FromDlgX(m_aMetrics[CX_BTN_BORDER]);
  m_dimBtn.cy += 2 * FromDlgY(m_aMetrics[CY_BTN_BORDER]);
}

void CGeneralMsgBox::CreateIcon()
{
  if (m_hIcon != NULL) {
    CRect rcDummy; // dimesion doesn't matter here

    // Creating the icon control
    m_stIconCtrl.Create(NULL, WS_CHILD|WS_VISIBLE|WS_DISABLED|SS_ICON, rcDummy, this);
    m_stIconCtrl.SetIcon(m_hIcon);
  }
}


// Converts d.u. in pixel (x axe)
int CGeneralMsgBox::FromDlgX(int x)
  { return x * m_dimDlgUnit.cx / CX_DLGUNIT_BASE; }

// Converts d.u. in pixel (y axe)
int CGeneralMsgBox::FromDlgY(int y)
  { return y * m_dimDlgUnit.cy / CY_DLGUNIT_BASE; }


// Updates the layout
void CGeneralMsgBox::UpdateLayout()
{
  // Caching the borders
  int cxLeft = FromDlgX(m_aMetrics[CX_LEFT_BORDER]);
  int cxRight = FromDlgX(m_aMetrics[CX_RIGHT_BORDER]);
  int cyTop = FromDlgY(m_aMetrics[CY_TOP_BORDER]);
  int cyBottom = FromDlgY(m_aMetrics[CY_BOTTOM_BORDER]);

  // Caching the space between buttons
  int cxBtnsSpace = FromDlgX(m_aMetrics[CX_BTNS_SPACE]);

  // The minimum dimensios of the client area
  // (starting with the message)
  CSize dimClient = m_dimMsg;
  int xMsg = 0;

  if (m_hIcon != NULL) {
    // Adding the icon
    xMsg = m_dimIcon.cx + FromDlgX(m_aMetrics[CX_ICON_MSG_SPACE]);

    dimClient.cx += xMsg;

    if (dimClient.cy < m_dimIcon.cy)
      dimClient.cy = m_dimIcon.cy;
  }

  xMsg += cxLeft;

  // Caching the minimum width needed for all buttons

  int cBtns = m_aBtns.GetSize();

  int cxBtns = (cBtns - 1) * FromDlgX(m_aMetrics[CX_BTNS_SPACE]) +
    cBtns * m_dimBtn.cx;

  if (dimClient.cx < cxBtns)
    dimClient.cx = cxBtns;

  dimClient.cx += cxLeft + cxRight;
  dimClient.cy += cyTop + cyBottom + m_dimBtn.cy + FromDlgY(m_aMetrics[CY_BTNS_MSG_SPACE]);

  // Set client dimensions

  CRect rc(0, 0, dimClient.cx, dimClient.cy);

  CalcWindowRect(rc);
  MoveWindow(rc);
  CenterWindow();

  // Icon layout

  if (m_hIcon != NULL)
    m_stIconCtrl.MoveWindow(cxLeft, cyTop, m_dimIcon.cx, m_dimIcon.cy);

  // Message layout

  m_dimMsg.cx += 2;
  xMsg = (xMsg + dimClient.cx - cxRight - m_dimMsg.cx) / 2;
  m_edCtrl.MoveWindow(xMsg, cyTop, m_dimMsg.cx, m_dimMsg.cy);

  // Buttons layout

  int x = (dimClient.cx - cxBtns) / 2;
  int y = dimClient.cy - cyBottom - m_dimBtn.cy;

  for (int i = 0; i < cBtns; ++i) {
    CWnd* pWndCtrl = GetDlgItem(m_aBtns[i].uIDC);
    pWndCtrl->MoveWindow(x, y, m_dimBtn.cx, m_dimBtn.cy);
    x += m_dimBtn.cx + cxBtnsSpace;
  }
}

CString
CGeneralMsgBox::GetTextFormatting(CString csHTML, int &iError)
{
  CString csText, csToken, csHTMLTag;
  int iCurrentFontSize, iCurrentFontPointSize;
  int iDefaultFontSize, iDefaultFontPointSize;
  int iTextPosition(0);
  int curPos, oldPos;

  int ierrorcode(0);
  bool bHTMLTag;

  st_format this_format;

  ALink this_ALink;

  std::bitset<3> bsFontChange;  // facename, size, color

  std::vector<CString> vLastFacenames;
  std::vector<COLORREF> vLastColours;
  std::vector<int> vLastSizes;
  std::vector<int> vFontChangeStart;
  std::vector<std::bitset<3>> vFontChange;
  std::stack<st_format> format_stack;
  std::stack<int> type_stack;

  // Validate the HTML
  curPos = 0;
  oldPos = 0;
  int iBold(0), iItalic(0), iUnderline(0), iFont(0), iAnchor(0);
  bool bNestedBold(false), bNestedItalic(false), bNestedUnderline(false),
       bNestedAnchor(false), bOverlapped(false);

  csToken = csHTML.Tokenize(_T("<>"), curPos);
  while (csToken != "" && curPos != -1) {
    oldPos = curPos - csToken.GetLength() - 1;
    CString a = csHTML.Mid(oldPos - 1, 1);
    CString b = csHTML.Mid(curPos - 1, 1);
    if (csHTML.Mid(oldPos - 1, 1) == _T("<") &&
        csHTML.Mid(curPos - 1, 1) == _T(">")) {
      bHTMLTag = true;
    } else
      bHTMLTag = false;

    if (bHTMLTag) {
      // Must be a HTML Tag
      csHTMLTag = csToken;
      csHTMLTag.MakeLower();
      if (csHTMLTag == _T("b")) {
        type_stack.push(Bold);
        iBold++;
        if (iBold != 1) bNestedBold = true;
        goto vnext;
      } else if (csHTMLTag == _T("/b")) {
        int &iLastType = type_stack.top();
        if (iLastType != Bold)
          bOverlapped = true;
        iBold--;
        type_stack.pop();
        goto vnext;
      } else if (csHTMLTag == _T("i")) {
        type_stack.push(Italic);
        iItalic++;
        if (iItalic != 1) bNestedItalic = true;
        goto vnext;
      } else if (csHTMLTag == _T("/i")) {
        int &iLastType = type_stack.top();
        if (iLastType != Italic)
          bOverlapped = true;
        iItalic--;
        type_stack.pop();
        goto vnext;
      } else if (csHTMLTag == _T("u")) {
        type_stack.push(Underline);
        iUnderline++;
        if (iUnderline != 1) bNestedUnderline = true;
        goto vnext;
      } else if (csHTMLTag == _T("/u")) {
        int &iLastType = type_stack.top();
        if (iLastType != Underline)
          bOverlapped = true;
        iUnderline--;
        type_stack.pop();
        goto vnext;
      } else if (csHTMLTag.Left(4) == _T("font")) {
        type_stack.push(Font);
        iFont++;
        goto vnext;
      } else if (csHTMLTag == _T("/font")) {
        int &iLastType = type_stack.top();
        if (iLastType != Font)
          bOverlapped = true;
        iFont--;
        type_stack.pop();
        goto vnext;
      } else if (csHTMLTag.Left(6) == _T("a href")) {
        type_stack.push(Link);
        iAnchor++;
        if (iAnchor != 1) bNestedAnchor = true;
        goto vnext;
      } else if (csHTMLTag == _T("/a")) {
        int &iLastType = type_stack.top();
        if (iLastType != Link)
          bOverlapped = true;
        iAnchor--;
        type_stack.pop();
        goto vnext;
      }
    }

vnext:
    oldPos = curPos;
    csToken = csHTML.Tokenize(_T("<>"), curPos);
  };

  if (bNestedBold)
    ierrorcode += 1;
  if (iBold != 0)
    ierrorcode += 2;
  if (bNestedItalic)
    ierrorcode += 4;
  if (iItalic != 0)
    ierrorcode += 8;
  if (bNestedUnderline)
    ierrorcode += 16;
  if (iUnderline != 0)
    ierrorcode += 32;
  if (bNestedAnchor)
    ierrorcode += 64;
  if (iAnchor != 0)
    ierrorcode += 128;
  if (iFont != 0)
    ierrorcode += 256;
  if (bOverlapped)
    ierrorcode +=512;

  iError = ierrorcode;

  if (ierrorcode != 0) {
    return _T("");
  }

  // Now really process the HTML
  NONCLIENTMETRICS ncm;
  memset(&ncm, 0, sizeof(NONCLIENTMETRICS));
  ncm.cbSize = sizeof(NONCLIENTMETRICS);

  SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
                       sizeof(NONCLIENTMETRICS), &ncm, NULL);

  CWnd *pWndDesk = GetDesktopWindow();
  CDC *pDCDesk = pWndDesk->GetWindowDC();
  int logPixY = pDCDesk->GetDeviceCaps(LOGPIXELSY);
  pWndDesk->ReleaseDC(pDCDesk);

  iDefaultFontPointSize = MulDiv(ncm.lfMessageFont.lfHeight, 72, logPixY);
  iCurrentFontPointSize = iDefaultFontPointSize;
  iDefaultFontSize = ConvertPointsToSize(iCurrentFontPointSize);
  iCurrentFontSize = iDefaultFontSize;

  curPos = 0;
  oldPos = 0;

  csToken = csHTML.Tokenize(_T("<>"), curPos);
  while (csToken != "" && curPos != -1) {
    oldPos = curPos - csToken.GetLength() - 1;
    CString a = csHTML.Mid(oldPos - 1, 1);
    CString b = csHTML.Mid(curPos - 1, 1);
    if (csHTML.Mid(oldPos - 1, 1) == _T("<") &&
        csHTML.Mid(curPos - 1, 1) == _T(">")) {
      bHTMLTag = true;
    } else
      bHTMLTag = false;

    if (!bHTMLTag && iAnchor != 0)
      goto next;

    if (bHTMLTag) {
      // Must be a HTML Tag
      csHTMLTag = csToken;
      csHTMLTag.MakeLower();
      if (csHTMLTag == _T("b")) {
        this_format.entrytype = Bold;
        this_format.iStart = iTextPosition;
        format_stack.push(this_format);
        goto next;
      } else if (csHTMLTag == _T("/b")) {
        st_format& cur_format = format_stack.top();
        cur_format.iEnd = iTextPosition;
        m_vFormat.push_back(cur_format);
        format_stack.pop();
        goto next;
      } else if (csHTMLTag == _T("i")) {
        this_format.entrytype = Italic;
        this_format.iStart = iTextPosition;
        format_stack.push(this_format);
        goto next;
      } else if (csHTMLTag == _T("/i")) {
        st_format& cur_format = format_stack.top();
        cur_format.iEnd = iTextPosition;
        m_vFormat.push_back(cur_format);
        format_stack.pop();
        goto next;
      } else if (csHTMLTag == _T("u")) {
        this_format.entrytype = Underline;
        this_format.iStart = iTextPosition;
        format_stack.push(this_format);
        goto next;
      } else if (csHTMLTag == _T("/u")) {
        st_format& cur_format = format_stack.top();
        cur_format.iEnd = iTextPosition;
        m_vFormat.push_back(cur_format);
        format_stack.pop();
        goto next;
      } else if (csHTMLTag == _T("/font")) {
        std::bitset<3> &bsLastFont = vFontChange.back();
        int &iFontChangeStart = vFontChangeStart.back();
        st_format& cur_format = format_stack.top();
        if (bsLastFont.test(FACENAMECHANGED)) {
          CString &csLastFaceName = vLastFacenames.back();
          cur_format.entrytype = Name;
          cur_format.iStart = iFontChangeStart;
          cur_format.iEnd = iTextPosition;
          memset(cur_format.tcszFACENAME, 0x00, sizeof(cur_format.tcszFACENAME));
#if (_MSC_VER >= 1400)
          _tcscpy_s(cur_format.tcszFACENAME, LF_FACESIZE, (LPCTSTR)csLastFaceName);
#else
          _tcscpy(cur_format.tcszFACENAME, (LPCTSTR)csLastFaceName);
#endif
          m_vFormat.push_back(cur_format);
          vLastFacenames.pop_back();
        }
        if (bsLastFont.test(SIZECHANGED)) {
          int &iLastSize = vLastSizes.back();
          cur_format.entrytype = Size;
          cur_format.iStart = iFontChangeStart;
          cur_format.iEnd = iTextPosition;
          cur_format.iSize = iLastSize;
          m_vFormat.push_back(cur_format);
          vLastSizes.pop_back();
          if (!vLastSizes.empty()) {
            iCurrentFontPointSize = vLastSizes.back();
            iCurrentFontSize = ConvertPointsToSize(iCurrentFontPointSize);
          } else {
            iCurrentFontPointSize = iDefaultFontPointSize;
            iCurrentFontSize = iDefaultFontSize;
          }
        }
        if (bsLastFont.test(COLOURCHANGED)) {
          COLORREF &c = vLastColours.back();
          cur_format.entrytype = Colour;
          cur_format.iStart = iFontChangeStart;
          cur_format.iEnd = iTextPosition;
          cur_format.cr = c;
          m_vFormat.push_back(cur_format);
          vLastColours.pop_back();
        }
        vFontChange.pop_back();
        vFontChangeStart.pop_back();
        format_stack.pop();
        goto next;
      } else if (csHTMLTag == _T("/a")) {
        goto next;
      }

      // Check for fonts
      // <font face="xxxxx" size="n" color="xxxxx">....</font>
      if (csHTMLTag.Left(5) == _T("font ")) {
        CString csFontToken, csFontVerb, csFontVerbValue;
        int curFontPos(0);

        bsFontChange.reset();
        csFontToken = csHTMLTag.Tokenize(_T("\""), curFontPos);
        csFontVerb = csFontToken.Right(6);
        csFontVerb.TrimLeft();
        // Skip over first token of 'font verb='
        csFontVerbValue = csHTMLTag.Tokenize(_T("\""), curFontPos);
        while (csFontVerbValue != "" && curFontPos != -1) {
          if (csFontVerb == _T("face=")) {
            bsFontChange.set(FACENAMECHANGED);
            vLastFacenames.push_back(csFontVerbValue);
          } else
          if (csFontVerb == _T("size=")) {
            bsFontChange.set(SIZECHANGED);
            iCurrentFontPointSize = ConvertSizeToPoints(csFontVerbValue, iCurrentFontSize);
            vLastSizes.push_back(iCurrentFontPointSize);
          } else
          if (csFontVerb == _T("color=")) {
            bsFontChange.set(COLOURCHANGED);
            COLORREF crNewFontColour = ConvertColourToColorRef(csFontVerbValue);
            vLastColours.push_back(crNewFontColour);
          }

          csFontVerb = csHTMLTag.Tokenize(_T("\""), curFontPos);
          if (csFontVerb.IsEmpty() && curFontPos == -1)
            break;
          csFontVerbValue = csHTMLTag.Tokenize(_T("\""), curFontPos);
        };
        vFontChange.push_back(bsFontChange);
        vFontChangeStart.push_back(iTextPosition);
        format_stack.push(this_format);
        goto next;
      }

      // check for hyperlink
      // <a href="http://www.microsoft.com">Friendly name</a>

      if (csHTMLTag.Left(7) == _T("a href=")) {
        long dwTStart, dwTEnd;
        dwTStart = csHTMLTag.Find(_T("href="), 0);
        if (dwTStart >= 0) {
          CString csURL;
          dwTEnd = csHTMLTag.Find(_T("\""), dwTStart + sizeof(_T("href=")));
          if (dwTEnd >= 0) {
            csURL = csHTMLTag.Mid(dwTStart + sizeof(_T("href=")),
                                 dwTEnd - (dwTStart + sizeof(_T("href="))));
            if (!csURL.IsEmpty()) {
              csURL.MakeLower();
#if (_MSC_VER >= 1400)
              _tcscpy_s(this_ALink.tcszURL, _MAX_PATH, csURL);
#else
              _tcscpy(this_ALink.tcszURL, csURL);
#endif
            }
          }
        }
        // Now get Friendly Name (note doing this within the while loop!)
        oldPos = curPos;
        csToken = csHTML.Tokenize(_T("<>"), curPos);
        csText += csToken;
        this_ALink.iStart = iTextPosition;
        iTextPosition += csToken.GetLength();
        this_ALink.iEnd = iTextPosition;
        m_vALink.push_back(this_ALink);
        goto next;
      }
    }

    csToken.Replace(_T("&lt;"), _T("<"));
    csToken.Replace(_T("&gt;"), _T(">"));
    // Real text or unknown HTML tag
    if (bHTMLTag) {
      // We didn't recognise it! Put back the < & >
      csText += _T("<") + csToken + _T(">");
      iTextPosition += csToken.GetLength() + 2;
    } else {
      csText += csToken;
      iTextPosition += csToken.GetLength();
    }

next:
    oldPos = curPos;
    csToken = csHTML.Tokenize(_T("<>"), curPos);
  };

  return csText;
}

COLORREF
CGeneralMsgBox::ConvertColourToColorRef(CString &csValue)
{
  // Vlaue is either a colour name or "#RRGGBB"
  // Note COLORREF = 0x00bbggrr but HTML = 0x00rrggbb
  // Values for named colours here are in COLORREF format
  long retval(0L);
  if (csValue.Left(1) == _T("#")) {
    // Convert HTML to COLORREF
    ASSERT(csValue.GetLength() == 7);
    int icolour;
#if _MSC_VER >= 1400
    _stscanf_s(csValue.Mid(1), _T("%06x"), &icolour);
#else
    _stscanf(csValue.Mid(1), _T("%06x"), &retval);
#endif
    int ired = (icolour & 0xff0000) >> 16;
    int igreen = (icolour & 0xff00);
    int iblue = (icolour & 0xff) << 16;
    return (COLORREF)(iblue + igreen + ired);
  }

  if (csValue.CompareNoCase(_T("Black")) == 0)
    retval = 0L;
  else if (csValue.CompareNoCase(_T("Green")) == 0)
    retval = 0x008000L;
  else if (csValue.CompareNoCase(_T("Silver")) == 0)
    retval = 0xc0c0c0L;
  else if (csValue.CompareNoCase(_T("Lime")) == 0)
    retval = 0x00ff00L;
  else if (csValue.CompareNoCase(_T("Gray")) == 0)
    retval = 0x808080L;
  else if (csValue.CompareNoCase(_T("Olive")) == 0)
    retval = 0x008080L;
  else if (csValue.CompareNoCase(_T("White")) == 0)
    retval = 0xffffffL;
  else if (csValue.CompareNoCase(_T("Yellow")) == 0)
    retval = 0x00ffffL;
  else if (csValue.CompareNoCase(_T("Maroon")) == 0)
    retval = 0x000080L;
  else if (csValue.CompareNoCase(_T("Navy")) == 0)
    retval = 0x800000L;
  else if (csValue.CompareNoCase(_T("Red")) == 0)
    retval = 0x0000ffL;
  else if (csValue.CompareNoCase(_T("Blue")) == 0)
    retval = 0xff0000L;
  else if (csValue.CompareNoCase(_T("Purple")) == 0)
    retval = 0x800080L;
  else if (csValue.CompareNoCase(_T("Teal")) == 0)
    retval = 0x808000L;
  else if (csValue.CompareNoCase(_T("Fuchsia")) == 0)
    retval = 0xff00ffL;
  else if (csValue.CompareNoCase(_T("Aqua")) == 0)
    retval = 0xffff00L;

  return (COLORREF)retval;
}

int
CGeneralMsgBox::ConvertSizeToPoints(CString &csValue, int &iCurrentSize)
{
  int retval(0), iSize, absSize;

  iSize = _tstoi(csValue);
  absSize = iSize < 0 ? (-iSize) : iSize;
  ASSERT(absSize > 0 && absSize < 8);

  if (csValue.Left(1) == _T("+") || csValue.Left(1) == _T("-")) {
    // It is a "relative" change
    iSize = iCurrentSize + iSize;
    if (iSize < 1)
      iSize = 1;
    else
    if (iSize > 7)
      iSize = 7;
  }
  switch (iSize) {
    case 1:
      retval = 15;  // "7.5"
      break;
    case 2:
      retval = 20;  // "10"
      break;
    case 3:
      retval = 24;  // "12"
      break;
    case 4:
      retval = 27;  // "13.5"
      break;
    case 5:
      retval = 36;  // "18"
      break;
    case 6:
      retval = 48;  // "24"
      break;
    case 7:
      retval = 72;  // "36"
      break;
    default:
      ASSERT(0);
  }

  // Return value in "size"
  iCurrentSize = iSize;
  // Return value in points
  return retval;
}

int
CGeneralMsgBox::ConvertPointsToSize(const int iCurrentPoints)
{
  if (iCurrentPoints <= 15)
    return 1;
  else if (iCurrentPoints <= 20)
    return 2;
  else if (iCurrentPoints <= 24)
    return 3;
  else if (iCurrentPoints <= 27)
    return 4;
  else if (iCurrentPoints <= 36)
    return 5;
  else if (iCurrentPoints <= 48)
    return 6;
  else
    return 7;
}
