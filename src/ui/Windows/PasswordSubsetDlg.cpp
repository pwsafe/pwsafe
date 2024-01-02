/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PasswordSubsetDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "PasswordSubsetDlg.h"
#include "DboxMain.h"
#include "Fonts.h"
#include "winutils.h"

#include "core/StringX.h"
#include "core/PWSprefs.h"

#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------

CNumEdit::CNumEdit()
  : CEdit(), m_bLastMinus(false), m_bLastSeparator(false)
{
}

BEGIN_MESSAGE_MAP(CNumEdit, CEdit)
  //{{AFX_MSG_MAP(CNumEdit)
  ON_WM_CHAR()
  ON_WM_PASTE()
  ON_WM_RBUTTONDOWN()
  ON_WM_RBUTTONUP()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CNumEdit::OnPaste()
{
  // Do nothing as processing text pasted in the middle of current text
  // would be unnecessarily complicated (see processing in OnChar).
  // Not really a loss to the user in this function
}

void CNumEdit::OnRButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
  // Do nothing to prevent context menu and paste
}
void CNumEdit::OnRButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
  // Do nothing to prevent context menu and paste
}

void CNumEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  const bool bSeparator = nChar == L' ' || nChar == L';' || nChar == L',';

  const bool bWildcard = nChar == L'*';

  if (LineLength(LineIndex(0)) == 0) {
    // Initialise variables
    m_bLastMinus = m_bLastSeparator = false;

    // Ignore leading zeroes, backspaces, return or separator if edit field is empty
    if (bSeparator || nChar == L'0' || nChar == VK_BACK || nChar == VK_RETURN)
      return;
  }

  // Ensure character is a digit or a valid delimiter
  // Minus signs are allowed but must be followed by a digit
  // Otherwise just ignore it!

  // Ignore pressed character if last character was a minus sign and now another minus sign
  // or a zero or a separator is pressed
  if (m_bLastMinus && (nChar == L'-' || nChar == L'0' || bSeparator))
    return;

  // Ignore pressed character if last character was a separator and now another separator
  // or a zero is pressed
  if (m_bLastSeparator && (nChar == L'0' || bSeparator))
    return;

  // Ignore wildcard if string is not empty, or last char is not separator
  if (bWildcard && !m_bLastSeparator && LineLength(LineIndex(0)) != 0)
    return;
  
  // Do not pass on any character that is not a digit, minus sign, separator or backspace
  if (isdigit(nChar) || nChar == L'-' || bSeparator || bWildcard || nChar == VK_BACK) {
    // Send on to CEdit control
    CEdit::OnChar(nChar, nRepCnt, nFlags);

    // Set minus sign state if pressed
    if (nChar == L'-')
      m_bLastMinus = true;

    // Reset minus sign state after a digit or last character was a minus sign and
    // a backspace has been pressed
    if (isdigit(nChar) || (m_bLastMinus && nChar == VK_BACK))
      m_bLastMinus = false;

    // Set separator state if pressed
    if (bSeparator)
      m_bLastSeparator = true;

    // Reset separator state after a digit or a wildcard or a minus sign or last character was a separator and
    // a backspace has been pressed
    if (isdigit(nChar) || nChar == L'-' || bWildcard || (m_bLastSeparator && nChar == VK_BACK))
      m_bLastSeparator = false;
  }

  // If a separator or wildcard or backspace is pressed, update displayed password subset
  if (bSeparator || bWildcard || nChar == VK_RETURN || nChar == VK_BACK || LineLength(LineIndex(0)) == 0)
    GetParent()->SendMessage(PWS_MSG_DISPLAYPASSWORDSUBSET);
}

//-----------------------------------------------------------------------------
CPasswordSubsetDlg::CPasswordSubsetDlg(CWnd* pParent, const StringX &sxPasswordd)
  : CPWDialog(CPasswordSubsetDlg::IDD, pParent),
  m_pCopyBtn(nullptr),
  m_sxPassword(sxPasswordd), 
  m_csWarningMsg(L""),
  m_bShown(false), m_bCopyPasswordEnabled(false),
  m_bImageLoaded(FALSE), m_bDisabledImageLoaded(FALSE)
{
}

CPasswordSubsetDlg::~CPasswordSubsetDlg()
{
  if (m_bImageLoaded)
    m_CopyPswdBitmap.Detach();

  if (m_bDisabledImageLoaded)
    m_DisabledCopyPswdBitmap.Detach();
}

void CPasswordSubsetDlg::DoDataExchange(CDataExchange* pDX)
{
    CPWDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPasswordSubsetDlg)
    DDX_Text(pDX, IDC_SUBSETPOSITIONS, m_csSubsetPositions);
    DDX_Text(pDX, IDC_STATICSUBSETWARNING, m_csWarningMsg);
    DDX_Control(pDX, IDC_SUBSETRESULTS, m_edResults);
    DDX_Control(pDX, IDC_SUBSETPOSITIONS, m_neSubsetPositions);
    DDX_Control(pDX, IDC_STATICSUBSETWARNING, m_stcWarningMsg);

    DDX_Control(pDX, IDC_COPYPASSWORDHELP1, m_Help1);
    DDX_Control(pDX, IDC_COPYPASSWORDHELP2, m_Help2);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPasswordSubsetDlg, CPWDialog)
  //{{AFX_MSG_MAP(CPasswordSubsetDlg)
  ON_WM_CTLCOLOR()

  ON_MESSAGE(PWS_MSG_DISPLAYPASSWORDSUBSET, OnDisplayStatus)

  ON_BN_CLICKED(IDC_COPYPASSWORD, OnCopy)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPasswordSubsetDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();
  
  Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_SUBSETRESULTS));

  m_pCopyBtn = (CButton *)GetDlgItem(IDC_COPYPASSWORD);

  // Place dialog where user had it last
  CRect rect, dlgRect;
  GetWindowRect(&dlgRect);
  PWSprefs::GetInstance()->GetPrefPSSRect(rect.top, rect.bottom, 
                                          rect.left, rect.right);

  HRGN hrgnWork = WinUtil::GetWorkAreaRegion();
  // also check that window will be visible
  if ((rect.top == -1 && rect.bottom == -1 && rect.left == -1 && rect.right == -1) || !RectInRegion(hrgnWork, rect)){
    rect = dlgRect;
  }

  ::DeleteObject(hrgnWork);

  // Ignore size just set position
  SetWindowPos(NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

  if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
    m_Help1.Init(IDB_QUESTIONMARK);
    m_Help2.Init(IDB_QUESTIONMARK);

    AddTool(IDC_COPYPASSWORDHELP1, IDS_COPYPASSWORDHELP1);
    AddTool(IDC_COPYPASSWORDHELP2, IDS_COPYPASSWORDHELP2);
    ActivateToolTip();
  } else {
    m_Help1.EnableWindow(FALSE);
    m_Help1.ShowWindow(SW_HIDE);
    m_Help2.EnableWindow(FALSE);
    m_Help2.ShowWindow(SW_HIDE);
  }

  // Load bitmaps
  UINT nImageID = PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar) ?
                     IDB_COPYPASSWORD_NEW : IDB_COPYPASSWORD_CLASSIC;

  m_bImageLoaded = WinUtil::LoadScaledBitmap(m_CopyPswdBitmap, nImageID, true, m_hWnd);

  ASSERT(m_bImageLoaded);

  nImageID = PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar) ?
    IDB_COPYPASSWORD_NEW_D : IDB_COPYPASSWORD_CLASSIC_D;

  m_bDisabledImageLoaded = WinUtil::LoadScaledBitmap(m_DisabledCopyPswdBitmap, nImageID, true, m_hWnd);

  ASSERT(m_bDisabledImageLoaded);
  if (m_bDisabledImageLoaded) {
    m_pCopyBtn->SetBitmap(m_DisabledCopyPswdBitmap);
  }

  ShowWindow(SW_SHOW);
  return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CPasswordSubsetDlg::PreTranslateMessage(MSG *pMsg)
{
  RelayToolTipEvent(pMsg);

  // Don't even look like it was pressed if it should be disabled
  if (pMsg->message == WM_LBUTTONDOWN && pMsg->hwnd == m_pCopyBtn->GetSafeHwnd() &&
      !m_bCopyPasswordEnabled) {
    return TRUE;
  }

  // Don't even process double click - looks bad
  if (pMsg->message == WM_LBUTTONDBLCLK && pMsg->hwnd == m_pCopyBtn->GetSafeHwnd()) {
    return TRUE;
  }

  return CPWDialog::PreTranslateMessage(pMsg);
}

HBRUSH CPasswordSubsetDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CPWDialog::OnCtlColor(pDC, pWnd, nCtlColor);

  // Only deal with Static controls and then
  // Only with our special one - change colour of warning message
  if (nCtlColor == CTLCOLOR_STATIC && pWnd->GetDlgCtrlID() == IDC_STATICSUBSETWARNING) {
    if (((CStaticExtn *)pWnd)->GetColourState()) {
      COLORREF cfUser = ((CStaticExtn *)pWnd)->GetUserColour();
      pDC->SetTextColor(cfUser);
    }
  }

  // Let's get out of here
  return hbr;
}

void CPasswordSubsetDlg::OnCancel()
{
  CRect rect;
  GetWindowRect(&rect);
  PWSprefs::GetInstance()->SetPrefPSSRect(rect.top, rect.bottom,
                                          rect.left, rect.right);
  if (m_bShown)
    CPWDialog::EndDialog(4);
  else
    CPWDialog::OnCancel();
}

LRESULT CPasswordSubsetDlg::OnDisplayStatus(WPARAM /* wParam */, LPARAM /* lParam */)
{
  UpdateData(TRUE);
  m_stcWarningMsg.SetWindowText(L"");
  m_stcWarningMsg.ResetColour();

  SubsetInfo result = GetSubsetInfo(m_csSubsetPositions, true);

  if (result.err_id != 0) {
    m_csWarningMsg.Format(result.err_id, m_sxPassword.length());

    m_stcWarningMsg.SetWindowText(m_csWarningMsg);
    m_stcWarningMsg.SetColour(RGB(255, 0, 0));
    m_stcWarningMsg.Invalidate();

    m_neSubsetPositions.SetSel(result.err_start_pos, result.err_end_pos);
    m_neSubsetPositions.SetFocus();

    // Disable Copy to Clipboard
    m_pCopyBtn->SetBitmap(m_DisabledCopyPswdBitmap);
    m_bCopyPasswordEnabled = false;

    return 0L;
  }

  if (result.passwd_sub.empty()) {
    // Clear results
    m_edResults.SetWindowText(L"");

    // Disable Copy to Clipboard
    m_pCopyBtn->SetBitmap(m_DisabledCopyPswdBitmap);
    m_bCopyPasswordEnabled = false;
    return 0L;
  }

  m_edResults.SetWindowText(result.passwd_sub.c_str());
  m_bShown = true;

  // Enable Copy to Clipboard
  m_bCopyPasswordEnabled = !result.incomplete_string;
  m_pCopyBtn->SetBitmap(m_bCopyPasswordEnabled ? m_CopyPswdBitmap : m_DisabledCopyPswdBitmap);
  
  return 1L;
}

void CPasswordSubsetDlg::OnCopy()
{
  if (!m_bCopyPasswordEnabled)
    return;

  SubsetInfo result = GetSubsetInfo(m_csSubsetPositions, false);
  if (result.err_id != 0 || result.incomplete_string) {
    return;
  }

  GetMainDlg()->SetClipboardData(result.passwd_sub);
  GetMainDlg()->UpdateLastClipboardAction(CItemData::PASSWORD);

  m_csWarningMsg.LoadString(IDS_PASSWORDCOPIED);

  m_stcWarningMsg.SetWindowText(m_csWarningMsg);
  m_stcWarningMsg.SetColour(RGB(0, 0, 0));
  m_stcWarningMsg.Invalidate();
}

CPasswordSubsetDlg::SubsetInfo CPasswordSubsetDlg::GetSubsetInfo(const CString& subset, bool with_delims) const
{
  int icurpos(0);
  CString resToken(subset);
  const size_t ipwlengh = m_sxPassword.length();
  
  SubsetInfo result;
  result.err_id = 0;
  result.passwd_sub.clear();
  result.incomplete_string = false;

  StringX passwd_sub;

  while (!resToken.IsEmpty() && icurpos != -1) {
    int lastpos = icurpos;
    resToken = subset.Tokenize(L";, ", icurpos);

    if (resToken.IsEmpty()) {
      continue;
    }

    if (resToken == L'*') {
      passwd_sub += m_sxPassword;
      continue;
    }

    int ipos = _wtoi(resToken);
    // ipos can't be zero but is valid if:
    //   a. Positive: 1 <= ipos <= password_length
    //   b. Negative: -password_lengh < ipos <= -1
    if (abs(ipos) > (int)ipwlengh) {
      result.err_id = ipos > 0 ? IDS_SUBSETINDEXTOOBIG : IDS_SUBSETINDEXTOOSMALL;
      result.err_start_pos = lastpos;
      result.err_end_pos = icurpos;
      result.incomplete_string = true;
      return result;
    }

    if (lastpos + resToken.GetLength() == subset.GetLength()) {
      // skipping number: current token last in string, and there is no separator after it
      result.incomplete_string = true;
      break;
    }

    if (ipos < 0) {
      ipos = (int)ipwlengh + ipos + 1;
    }

    passwd_sub += m_sxPassword[ipos - 1];
  }

  if (with_delims) {
    result.passwd_sub.reserve(passwd_sub.size() * 2);
    for (size_t i = 0; i < passwd_sub.size(); ++i) {
      result.passwd_sub += passwd_sub[i];
      result.passwd_sub += L' ';
    }
  }
  else {
    result.passwd_sub = passwd_sub;
  }

  return result;
}
