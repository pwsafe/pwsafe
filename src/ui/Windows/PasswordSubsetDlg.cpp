/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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

#include "core/StringX.h"
#include "core/PWSprefs.h"

#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern HRGN GetWorkAreaRegion();

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

  // Do not pass on any character that is not a digit, minus sign, separator or backspace
  if (isdigit(nChar) || nChar == L'-' || bSeparator || nChar == VK_BACK) {
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

    // Reset separator state after a digit or a minus sign or last character was a separator and
    // a backspace has been pressed
    if (isdigit(nChar) || nChar == L'-' || (m_bLastSeparator && nChar == VK_BACK))
      m_bLastSeparator = false;
  }

  // If a separator or backspace is pressed, update displayed password subset
  if (bSeparator || nChar == VK_RETURN || nChar == VK_BACK || LineLength(LineIndex(0)) == 0)
    GetParent()->SendMessage(PWS_MSG_DISPLAYPASSWORDSUBSET);
}

//-----------------------------------------------------------------------------
CPasswordSubsetDlg::CPasswordSubsetDlg(CWnd* pParent, const StringX &sxPasswordd)
  : CPWDialog(CPasswordSubsetDlg::IDD, pParent),
  m_sxPassword(sxPasswordd), m_bShown(false), m_csWarningMsg(L""), m_pCopyBtn(NULL),
  m_bCopyPasswordEnabled(false), m_bImageLoaded(FALSE), m_bDisabledImageLoaded(FALSE)
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

  HRGN hrgnWork = GetWorkAreaRegion();
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

  m_bImageLoaded = m_CopyPswdBitmap.Attach(
                ::LoadImage(::AfxFindResourceHandle(MAKEINTRESOURCE(nImageID), RT_BITMAP),
                MAKEINTRESOURCE(nImageID), IMAGE_BITMAP, 0, 0,
                (LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_SHARED)));

  ASSERT(m_bImageLoaded);
  if (m_bImageLoaded) {
    FixBitmapBackground(m_CopyPswdBitmap);
  }

  nImageID = PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar) ?
    IDB_COPYPASSWORD_NEW_D : IDB_COPYPASSWORD_CLASSIC_D;

  m_bDisabledImageLoaded = m_DisabledCopyPswdBitmap.Attach(
    ::LoadImage(::AfxFindResourceHandle(MAKEINTRESOURCE(nImageID), RT_BITMAP),
      MAKEINTRESOURCE(nImageID), IMAGE_BITMAP, 0, 0,
      (LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_SHARED)));

  ASSERT(m_bDisabledImageLoaded);
  if (m_bDisabledImageLoaded) {
    FixBitmapBackground(m_DisabledCopyPswdBitmap);
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

  int icurpos(0);
  std::vector<int> vpos;
  CString resToken(m_csSubsetPositions);
  const size_t ipwlengh = m_sxPassword.length();

  while (!resToken.IsEmpty() && icurpos != -1) {
    int lastpos = icurpos;
    resToken = m_csSubsetPositions.Tokenize(L";, ", icurpos);
    if (resToken.IsEmpty())
      continue;

    int ipos = _wtoi(resToken);
    // ipos can't be zero but is valid if:
    //   a. Positive: 1 <= ipos <= password_length
    //   b. Negative: -password_lengh < ipos <= -1
    if (abs(ipos) > (int)ipwlengh) {
      m_csWarningMsg.Format(ipos > 0 ? IDS_SUBSETINDEXTOOBIG : IDS_SUBSETINDEXTOOSMALL, ipwlengh);

      m_stcWarningMsg.SetWindowText(m_csWarningMsg);
      m_stcWarningMsg.SetColour(RGB(255, 0, 0));
      m_stcWarningMsg.Invalidate();
      vpos.clear();
      m_neSubsetPositions.SetSel(lastpos, icurpos);
      m_neSubsetPositions.SetFocus();

      // Disable Copy to Clipboard
      m_pCopyBtn->SetBitmap(m_DisabledCopyPswdBitmap);
      m_bCopyPasswordEnabled = false;

      return 0L;
    }

    if (ipos < 0)
      ipos = (int)ipwlengh + ipos + 1;

    vpos.push_back(ipos - 1);
  };

  if (vpos.empty()) {
    // Clear results
    m_edResults.SetWindowText(L"");

    // Disable Copy to Clipboard
    m_pCopyBtn->SetBitmap(m_DisabledCopyPswdBitmap);
    m_bCopyPasswordEnabled = false;
    return 0L;
  }

  // Check that the last character was a separator
  bool bLastSeparator = m_csSubsetPositions.Right(1).FindOneOf(L" ,;") != -1;
  if (!bLastSeparator) {
    // No - so remove last entry
    vpos.pop_back();
  }

  std::vector<int>::const_iterator citer;
  StringX sSubset;
  for (citer = vpos.begin(); citer != vpos.end(); citer++) {
    sSubset += m_sxPassword[*citer];
    sSubset += L" ";
  }

  m_edResults.SetWindowText(sSubset.c_str());
  m_bShown = true;

  // Enable Copy to Clipboard
  m_pCopyBtn->SetBitmap(bLastSeparator ? m_CopyPswdBitmap : m_DisabledCopyPswdBitmap);
  m_bCopyPasswordEnabled = bLastSeparator;

  return 1L;
}

void CPasswordSubsetDlg::OnCopy()
{
  if (!m_bCopyPasswordEnabled)
    return;

  CSecString cs_data;

  int len = m_edResults.LineLength(m_edResults.LineIndex(0));
  m_edResults.GetLine(0, cs_data.GetBuffer(len), len);
  cs_data.ReleaseBuffer(len);

  // Remove blanks from between the characters
  // XXX - this breaks if a selected char happens to be a space...
  cs_data.Remove(L' ');
  GetMainDlg()->SetClipboardData(cs_data);
  GetMainDlg()->UpdateLastClipboardAction(CItemData::PASSWORD);

  m_csWarningMsg.LoadString(IDS_PASSWORDCOPIED);

  m_stcWarningMsg.SetWindowText(m_csWarningMsg);
  m_stcWarningMsg.SetColour(RGB(0, 0, 0));
  m_stcWarningMsg.Invalidate();
}
