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
 * significantly enhanced to support text with HTML formatting and links
 * instead of a RTF string by using a CRichEditCtrlExtn control.
 * See www.codeproject.com for the orgininal code
*/


#include "stdafx.h"
#include "GeneralMsgBox.h"
#include "RichEditCtrlExtn.h"
#include <RichEdit.h>

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

void CGeneralMsgBox::CreateRtfCtrl()
{
  // Creating the Rich Edit control

  CRect rcDummy; // dimension doesn't matter here

  m_edCtrl.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_READONLY,
                  rcDummy, this, (UINT)IDC_STATIC);
  m_edCtrl.SetBackgroundColor(FALSE, ::GetSysColor(COLOR_3DFACE));
  m_edCtrl.SetFont(GetFont());

  m_strMsg.Trim();

  m_edCtrl.SetWindowText(m_strMsg);

  /////////////////////////////////////////////////////////
  // Calculating the best Rich Edit control dimension
  //
  // Note:
  //   There's no Rich Edit operation to get the minimum
  //   width it needs to show its contents without line
  //   breaks. However, given a width, we can check
  //   how much height it needs to show its text (see
  //   bottomless rich edit controls).
  //   We'll use this mechanism to perform a binary search
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
