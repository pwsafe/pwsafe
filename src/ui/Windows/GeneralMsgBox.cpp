/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// GeneralMsgBox.cpp

/*
*
* Implements the extended Message Box class.
*
* This is a cut down version of TcxMsgBox by Thales P. Carvalho but then
* significantly enhanced to support text with HTML formatting and links
* instead of a RTF string by using a CRichEditCtrlExtn control.
* See www.codeproject.com for the original code
*/

#include "stdafx.h"

#include "ThisMfcApp.h"
#include "GeneralMsgBox.h"
#include "RichEditCtrlExtn.h"
#include "PWDialog.h"

#include "winutils.h"
#include <RichEdit.h>

#include "resource3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Local constants

const UINT TIMER_GENMSGBOX_ENABLE_CONTROLS = 1;

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

static const BYTE dlg_data[] =
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
  0x54, 0x00, 0x61, 0x00, 0x68, 0x00, 0x6f, 0x00,  // 'Taho'
  0x6d, 0x00, 0x61, 0x00, 0x00, 0x00               // 'ma\0'
};

/////////////////////////////////////////////////////////////////////////////
// CGeneralMsgBox

// Constructor
CGeneralMsgBox::CGeneralMsgBox(CWnd *pParentWnd)
  : m_uiDefCmdId((UINT)IDC_STATIC), m_uiEscCmdId((UINT)IDC_STATIC),
  m_hIcon(nullptr), m_strTitle(AfxGetApp()->m_pszAppName),
  m_bDelayAcceptAnswer(false), m_bTextBeforeAllowedSet(false)
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
  ::DestroyIcon(m_hIcon);
}

INT_PTR CGeneralMsgBox::MessageBoxTimeOut(LPCWSTR lpText, LPCWSTR lpCaption,
                                          UINT uiFlags, DWORD dwMilliseconds)
{
  if (dwMilliseconds == 0)
    return MessageBox(lpText, lpCaption, uiFlags);

  m_dwTimeOut = dwMilliseconds;
  m_bTimedOut = false;
  m_nResult = 0;

  DWORD dwThreadId;
  HANDLE hThread = CreateThread(nullptr, 0, ThreadFunction, (LPVOID)this, 0, &dwThreadId);
  if (hThread == nullptr)
    return -1;

  m_nResult = MessageBox(lpText, lpCaption, uiFlags);

  WaitForSingleObject(hThread, INFINITE);

  CloseHandle(hThread);

  if (m_bTimedOut)
    return IDTIMEOUT;

  return m_nResult;
}

DWORD WINAPI CGeneralMsgBox::ThreadFunction(LPVOID lpParameter)
{
  if (lpParameter) {
    CGeneralMsgBox *pObject = static_cast<CGeneralMsgBox *>(lpParameter);
    DWORD dwInitTime = GetTickCount();
    DWORD dwStartTime = dwInitTime;
    while (pObject->m_nResult != 0) {
      DWORD dwCurrentTime = GetTickCount();
      DWORD dwDeltaTime = dwCurrentTime - dwStartTime;
      if (dwDeltaTime >= 1000) {
        if ((dwCurrentTime - dwInitTime) >= pObject->m_dwTimeOut) {
          pObject->m_bTimedOut = true;
          pObject->PostMessage(WM_COMMAND, IDTIMEOUT, 0);
          return WAIT_OBJECT_0;
        }
        dwStartTime = GetTickCount();
      }
      Sleep(50);
    }
  }
  return WAIT_OBJECT_0;
}

void CGeneralMsgBox::EnableButtons(bool bEnable)
{
  for (int i = 0; i < m_aBtns.GetSize(); ++i) {
    CButton& btn = *(CButton*)GetDlgItem(m_aBtns[i].uiIDC);
    if (!::IsWindow(btn.m_hWnd))
      continue;
    if (!bEnable) {
      m_aBtns[i].bWasEnabled = !(btn.GetStyle() & WS_DISABLED);
      btn.ModifyStyle(0, WS_DISABLED);
    } else if (m_aBtns[i].bWasEnabled)
      btn.ModifyStyle(WS_DISABLED, 0);
  }
}

INT_PTR CGeneralMsgBox::AfxMessageBox(LPCWSTR lpszText, LPCWSTR lpCaption, const std::vector<std::tuple<int, int>>& tuples, int defBtn, UINT uiIcon)
{
  if (lpszText != nullptr)
    SetMsg(lpszText);

  if (lpCaption != nullptr)
    SetTitle(lpCaption);
  else
    SetTitle(IDS_ERROR);

  if (uiIcon == 0)
    uiIcon = MB_ICONEXCLAMATION;
  SetStandardIcon(uiIcon);

  if (defBtn >= tuples.size())
    defBtn = 0;

  CString cs_text;
  int i = 0;
  for (const auto & tuple : tuples)
  {
    const int id_c = std::get<0>(tuple);
    const int id_s = std::get<1>(tuple);
    cs_text.LoadString(id_s);
    AddButton(id_c, cs_text, i == defBtn, id_c == IDCANCEL);
    i++;
  }

  INT_PTR rc = DoModal();
  return rc;
}



INT_PTR CGeneralMsgBox::MessageBox(LPCWSTR lpText, LPCWSTR lpCaption, UINT uiFlags)
{
  UINT uiType = uiFlags & MB_TYPEMASK;
  UINT uiIcon = uiFlags & MB_ICONMASK;
  int iDefB = ((int)uiFlags & MB_DEFMASK) / 256;


  std::vector<std::tuple<int, int>> tuples;
  
  switch (uiType) {
    case MB_OK:
      tuples.push_back(std::make_tuple(IDOK, IDS_OK));
      break;
    case MB_OKCANCEL:
      tuples.push_back(std::make_tuple(IDOK, IDS_OK));
      tuples.push_back(std::make_tuple(IDCANCEL, IDS_CANCEL));
      break;
    case MB_ABORTRETRYIGNORE:
      tuples.push_back(std::make_tuple(IDABORT, IDS_ABORT));
      tuples.push_back(std::make_tuple(IDRETRY, IDS_RETRY));
      tuples.push_back(std::make_tuple(IDIGNORE, IDS_IGNORE));
      break;
    case MB_YESNOCANCEL:
      tuples.push_back(std::make_tuple(IDYES, IDS_YES));
      tuples.push_back(std::make_tuple(IDNO, IDS_NO));
      tuples.push_back(std::make_tuple(IDCANCEL, IDS_CANCEL));
      break;
    case MB_YESNO:
      tuples.push_back(std::make_tuple(IDYES, IDS_YES));
      tuples.push_back(std::make_tuple(IDNO, IDS_NO));
      break;
    case MB_RETRYCANCEL:
      tuples.push_back(std::make_tuple(IDRETRY, IDS_RETRY));
      tuples.push_back(std::make_tuple(IDCANCEL, IDS_CANCEL));
      break;
    case MB_CANCELTRYCONTINUE:
      tuples.push_back(std::make_tuple(IDCANCEL, IDS_CANCEL));
      tuples.push_back(std::make_tuple(IDTRYAGAIN, IDS_TRYAGAIN));
      tuples.push_back(std::make_tuple(IDCONTINUE, IDS_CONTINUE));
      break;
    default:
      ASSERT(0);
  }

  return AfxMessageBox(lpText, lpCaption, tuples, iDefB, uiIcon);
}

INT_PTR CGeneralMsgBox::AfxMessageBox(LPCWSTR lpszText, LPCWSTR lpCaption, UINT uiFlags)
{
  SetMsg(lpszText);
  if (lpCaption == nullptr)
    lpCaption = AfxGetApp()->m_pszAppName;
  INT_PTR rc = MessageBox(nullptr, lpCaption, uiFlags);
  return rc;
}

INT_PTR CGeneralMsgBox::AfxMessageBox(UINT uiIDPrompt, UINT uiFlags)
{
  SetMsg(uiIDPrompt);
  INT_PTR rc = MessageBox(nullptr, AfxGetApp()->m_pszAppName, uiFlags);
  return rc;
}

// Replaces CDialog::DoModal
INT_PTR CGeneralMsgBox::DoModal()
{
  InitModalIndirect((LPCDLGTEMPLATE)dlg_data, m_pParentWnd);
  bool bAccEn = app.IsAcceleratorEnabled();
  if (bAccEn)
    app.DisableAccelerator();

  CPWDialog::GetDialogTracker()->AddOpenDialog(this);
  INT_PTR rc = CDialog::DoModal();
  CPWDialog::GetDialogTracker()->RemoveOpenDialog(this);

  if (bAccEn)
    app.EnableAccelerator();

  return rc;
}

/////////////////////////////////////////////////////////////////////////////
// CGeneralMsgBox - Button operations

// Add a button
void CGeneralMsgBox::AddButton(UINT uiIDC,          // button command ID
                               LPCWSTR pszText,     // button text
                               BOOL bIsDefault,     // set the button as default
                               BOOL bIsEscape)      // return this command if user press escape
{
  ASSERT(uiIDC != (UINT)IDC_STATIC);

  BTNDATA btndata;
  btndata.uiIDC = uiIDC;
  btndata.strBtn = pszText;

  m_aBtns.Add(btndata);

  if (bIsEscape)
    m_uiEscCmdId = uiIDC;

  if (bIsDefault)
    m_uiDefCmdId = uiIDC;
}

// Add a button
void CGeneralMsgBox::AddButton(UINT uiIDC,          // button command ID
                               UINT uiIDText,       // string ID of button's text
                               BOOL bIsDefault,     // set the button as default
                               BOOL bIsEscape)      // return this command if user press escape
{
  CString str;

  if (uiIDText == (UINT)-1)
    uiIDText = uiIDC;

  VERIFY(str.LoadString(uiIDText));

  AddButton(uiIDC, str, bIsDefault, bIsEscape);
}

/////////////////////////////////////////////////////////////////////////////
// CGeneralMsgBox - Plain text operations

BOOL CGeneralMsgBox::SetMsg(UINT uiMsgId)
{
  return m_strMsg.LoadString(uiMsgId);
}

BOOL CGeneralMsgBox::SetMsg(LPCWSTR pszMsg)
{
  m_strMsg = pszMsg;
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGeneralMsgBox - Icon operations

void CGeneralMsgBox::SetIcon(HICON hIcon)
{
  m_hIcon = hIcon;

  if (m_hIcon != nullptr) {
    // loading the icon and extracting it's dimensions
    ICONINFO ii;
    GetIconInfo(m_hIcon, &ii);

    BITMAP bm;
    GetObject(ii.hbmColor, sizeof(bm), &bm);

    m_dimIcon.cx = bm.bmWidth;
    m_dimIcon.cy = bm.bmHeight;
    ::DeleteObject(ii.hbmColor);
    ::DeleteObject(ii.hbmMask);
  } else {
    m_dimIcon.cx = 0;
    m_dimIcon.cy = 0;
  }
}

void CGeneralMsgBox::SetIcon(UINT uiIcon)
{
  SetIcon(AfxGetApp()->LoadIcon(uiIcon));
}

void CGeneralMsgBox::SetStandardIcon(LPCWSTR pszIconName)
{
  SetIcon(AfxGetApp()->LoadStandardIcon(pszIconName));
}

void CGeneralMsgBox::SetStandardIcon(UINT uiIcon)
{
  LPCWSTR pszIconName;
  switch (uiIcon) {
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

  if (m_bDelayAcceptAnswer) {
    EnableButtons(false);
    SetTimer(TIMER_GENMSGBOX_ENABLE_CONTROLS, (UINT)1000, NULL);
  }

  // Disabling the ESC key
  if (m_uiEscCmdId == (UINT)IDC_STATIC)
    ModifyStyle(WS_SYSMENU, NULL);

  if (m_uiDefCmdId != (UINT)IDC_STATIC) {
    SetGotoDefaultControl();
    return FALSE;
  }

  return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CGeneralMsgBox::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, 
                              LRESULT* pLResult)
{
  if (message == WM_NOTIFY) {
    REQRESIZE *prr = (REQRESIZE *)lParam;
    if (prr->nmhdr.code == EN_REQUESTRESIZE) {
      // (1)
      // The rich edit control requested a resize.
      // Store the requested resize.
      m_dimMsg.cx = prr->rc.right - prr->rc.left;
      m_dimMsg.cy = prr->rc.bottom - prr->rc.top;

      *pLResult = NULL;
      return TRUE;
    }
  } else if (message == WM_CLOSE) {
    if (m_uiEscCmdId != (UINT)IDC_STATIC)
      EndDialog(m_uiEscCmdId);

    return TRUE;
  } else if (message == WM_TIMER && wParam == TIMER_GENMSGBOX_ENABLE_CONTROLS) {
    m_dwTimeOut -= 1000;
    if (m_dwTimeOut == 0) {
      KillTimer(TIMER_GENMSGBOX_ENABLE_CONTROLS);
      m_edCtrl.SetWindowText(m_sTextAfterAllowed);
      EnableButtons(true);
      SetGotoDefaultControl();
    } else {
      UpdateBeforeAllowedMessage();
    }
  }

  return CDialog::OnWndMsg(message, wParam, lParam, pLResult);
}

BOOL CGeneralMsgBox::OnCmdMsg(UINT uiID, int nCode, void* pExtra, 
                              AFX_CMDHANDLERINFO* pHandlerInfo)
{
  if (nCode == CN_COMMAND) {
    if (pHandlerInfo == nullptr && uiID != (WORD)IDC_STATIC) {
      EndDialog(uiID);
      return TRUE;
    }
  }

  if (nCode == CN_COMMAND) {
    if (pHandlerInfo == nullptr && uiID == IDTIMEOUT) {
      EndDialog(IDTIMEOUT);
      return TRUE;
    }
  }

  return CDialog::OnCmdMsg(uiID, nCode, pExtra, pHandlerInfo);
}

BOOL CGeneralMsgBox::PreTranslateMessage(MSG *pMsg)
{
  if (pMsg->message == WM_KEYDOWN) {
    if (pMsg->wParam == VK_RETURN) {
      CWnd *pWnd = GetFocus();

      if (pWnd != nullptr) {
        UINT uiIDC = (UINT)pWnd->GetDlgCtrlID();

        for (int i = 0; i < m_aBtns.GetSize(); ++i)
          if (m_aBtns[i].uiIDC == uiIDC) {
            m_uiDefCmdId = uiIDC;
            break;
          }
      }

      EndDialog(m_uiDefCmdId);

      return TRUE;
    }
    else if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_CANCEL) {
      if (m_uiEscCmdId != (UINT)IDC_STATIC)
        EndDialog(m_uiEscCmdId);

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

  DWORD dwStyles = WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_READONLY;
  // Delaying for answer causes default buttons (i.e., a sole OK button) to
  // be initially disabled, causing rich text edit to gain focus with full
  // selection. Prevent that with ES_SAVESEL.
  if (m_bDelayAcceptAnswer)
    dwStyles |= ES_SAVESEL;
  m_edCtrl.Create(dwStyles, rcDummy, this, (UINT)IDC_STATIC);
  m_edCtrl.SetBackgroundColor(FALSE, ::GetSysColor(COLOR_3DFACE));
  m_edCtrl.SetFont(GetFont());

  m_strMsg.Trim();

  if (m_bDelayAcceptAnswer)
    UpdateBeforeAllowedMessage();
  else
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
  int cxLast = WinUtil::GetSystemMetrics(SM_CXFULLSCREEN, m_hWnd);
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
  while (cxFirst < cxLast);
}

void CGeneralMsgBox::CreateBtns()
{
  // The minimum button's dimension
  m_dimBtn = CSize(FromDlgX(m_aMetrics[CX_MIN_BTN]), 0);

  // DC and Font for use in some dimension calculations
  CClientDC dc(this);

  CFont *pWndFont = GetFont();
  CFont *poldFont = dc.SelectObject(pWndFont);

  CRect rcDummy; // dimension doesn't matter here

  INT_PTR cBtns = m_aBtns.GetSize();

  for (int i = 0; i < cBtns; ++i) {
    BTNDATA &btndata = m_aBtns[i];

    // Finding the minimum dimension needed to properly show any button
    CSize dimBtn = dc.GetTextExtent(btndata.strBtn);

    m_dimBtn.cx = std::max(m_dimBtn.cx, dimBtn.cx);
    m_dimBtn.cy = std::max(m_dimBtn.cy, dimBtn.cy);

    // Creating the button with MFC's CButton help.
    CButton btnCtrl;

    btnCtrl.Create(btndata.strBtn,
                   WS_CHILD|WS_VISIBLE|WS_TABSTOP,
                   rcDummy, this, btndata.uiIDC);
    btnCtrl.SetFont(pWndFont);
    btnCtrl.UnsubclassWindow();
  }

  dc.SelectObject(poldFont);

  // Add the button margins
  m_dimBtn.cx += 2 * FromDlgX(m_aMetrics[CX_BTN_BORDER]);
  m_dimBtn.cy += 2 * FromDlgY(m_aMetrics[CY_BTN_BORDER]);
}

void CGeneralMsgBox::CreateIcon()
{
  if (m_hIcon != nullptr) {
    CRect rcDummy; // dimension doesn't matter here

    // Creating the icon control
    m_stIconCtrl.Create(nullptr, WS_CHILD | WS_VISIBLE | WS_DISABLED | SS_ICON, 
                        rcDummy, this);
    m_stIconCtrl.SetIcon(m_hIcon);
  }
}

// Converts d.u. in pixel (x axe)
int CGeneralMsgBox::FromDlgX(int x)
{ return x * m_dimDlgUnit.cx / CX_DLGUNIT_BASE; }

// Converts d.u. in pixel (y axe)
int CGeneralMsgBox::FromDlgY(int y)
{ return y * m_dimDlgUnit.cy / CY_DLGUNIT_BASE; }

void CGeneralMsgBox::SetGotoDefaultControl()
{
  // If a default command ID is valid, establish it
  // as the default, and go to the control.
  if (m_uiDefCmdId != (UINT)IDC_STATIC) {
    GotoDlgCtrl(GetDlgItem(m_uiDefCmdId));
    SetDefID(m_uiDefCmdId);
  }
}

// Updates the layout
void CGeneralMsgBox::UpdateLayout()
{
  // Caching the borders
  int cxLeft   = FromDlgX(m_aMetrics[CX_LEFT_BORDER]);
  int cxRight  = FromDlgX(m_aMetrics[CX_RIGHT_BORDER]);
  int cyTop    = FromDlgY(m_aMetrics[CY_TOP_BORDER]);
  int cyBottom = FromDlgY(m_aMetrics[CY_BOTTOM_BORDER]);

  // Caching the space between buttons
  int cxBtnsSpace = FromDlgX(m_aMetrics[CX_BTNS_SPACE]);

  // The minimum dimensions of the client area
  // (starting with the message)
  CSize dimClient = m_dimMsg;
  int xMsg = 0;

  if (m_hIcon != nullptr) {
    // Adding the icon
    xMsg = m_dimIcon.cx + FromDlgX(m_aMetrics[CX_ICON_MSG_SPACE]);

    dimClient.cx += xMsg;

    if (dimClient.cy < m_dimIcon.cy)
      dimClient.cy = m_dimIcon.cy;
  }

  xMsg += cxLeft;

  // Caching the minimum width needed for all buttons
  int cBtns = (int)m_aBtns.GetSize();

  int cxBtns = (cBtns - 1) * FromDlgX(m_aMetrics[CX_BTNS_SPACE]) +
                   cBtns * m_dimBtn.cx;

  if (dimClient.cx < cxBtns)
    dimClient.cx = cxBtns;

  dimClient.cx += cxLeft + cxRight;
  dimClient.cy += cyTop + cyBottom + m_dimBtn.cy + 
                        FromDlgY(m_aMetrics[CY_BTNS_MSG_SPACE]);

  // Set client dimensions
  CRect rc(0, 0, dimClient.cx, dimClient.cy);

  CalcWindowRect(rc);
  MoveWindow(rc);
  CenterWindow();

  // Icon layout
  if (m_hIcon != nullptr)
    m_stIconCtrl.MoveWindow(cxLeft, cyTop, m_dimIcon.cx, m_dimIcon.cy);

  // Message layout
  m_dimMsg.cx += 2;
  xMsg = (xMsg + dimClient.cx - cxRight - m_dimMsg.cx) / 2;
  m_edCtrl.MoveWindow(xMsg, cyTop, m_dimMsg.cx, m_dimMsg.cy);

  // Buttons layout
  int x = (dimClient.cx - cxBtns) / 2;
  int y = dimClient.cy - cyBottom - m_dimBtn.cy;

  for (int i = 0; i < cBtns; ++i) {
    CWnd* pWndCtrl = GetDlgItem(m_aBtns[i].uiIDC);
    pWndCtrl->MoveWindow(x, y, m_dimBtn.cx, m_dimBtn.cy);
    x += m_dimBtn.cx + cxBtnsSpace;
  }
}

void CGeneralMsgBox::UpdateBeforeAllowedMessage()
{
  if (m_sTextBeforeAllowed.Find(L"%d") == -1) {
    // No integer format detected, just update message once.
    if (!m_bTextBeforeAllowedSet) {
      m_edCtrl.SetWindowText(m_sTextBeforeAllowed);
      m_bTextBeforeAllowedSet = true;
    }
  } else {
    // Update the message to reflect the remaining seconds.
    CString cs_temp;
    cs_temp.Format(m_sTextBeforeAllowed, m_dwTimeOut / 1000);
    m_edCtrl.SetWindowText(cs_temp);
  }
}

INT_PTR CGeneralMsgBox::MessageBoxDelayAcceptAnswer(
  LPCWSTR lpTextBeforeAllowed,
  LPCWSTR lpTextAfterAllowed,
  LPCWSTR lpCaption,
  UINT uiFlags,
  DWORD dwSeconds
)
{
  ASSERT(dwSeconds != 0);
  if (dwSeconds == 0)
    dwSeconds = 1;
  m_bDelayAcceptAnswer = true;
  m_dwTimeOut = dwSeconds * 1000;
  m_sTextBeforeAllowed = lpTextBeforeAllowed;
  m_bTextBeforeAllowedSet = false;
  m_sTextAfterAllowed = lpTextAfterAllowed;
  m_nResult = MessageBox(lpTextBeforeAllowed, lpCaption, uiFlags);
  return m_nResult;
}
