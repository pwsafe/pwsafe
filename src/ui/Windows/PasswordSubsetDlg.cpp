/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
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
BEGIN_MESSAGE_MAP(CNumEdit, CEdit)
  //{{AFX_MSG_MAP(CNumEdit)
  ON_WM_CHAR()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

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

  // Do not pass on any character that is not a digit, minus sign or separator
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

  // If a separator is pressed, update displayed password subset
  if (bSeparator || nChar == VK_RETURN || LineLength(LineIndex(0)) == 0)
    GetParent()->SendMessage(WM_DISPLAYPASSWORDSUBSET);
}

//-----------------------------------------------------------------------------
CPasswordSubsetDlg::CPasswordSubsetDlg(CWnd* pParent, const StringX &passwd)
  : CPWDialog(CPasswordSubsetDlg::IDD, pParent),
    m_passwd(passwd), m_bshown(false), m_warningmsg(L"")
{
}

CPasswordSubsetDlg::~CPasswordSubsetDlg()
{
  m_CopyPswdBitmap.Detach();
}


void CPasswordSubsetDlg::DoDataExchange(CDataExchange* pDX)
{
    CPWDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPasswordSubsetDlg)
    DDX_Text(pDX, IDC_SUBSET, m_subset);
    DDX_Text(pDX, IDC_STATICSUBSETWARNING, m_warningmsg);
    DDX_Control(pDX, IDC_SUBSETRESULTS, m_results);
    DDX_Control(pDX, IDC_SUBSET, m_ne_subset);
    DDX_Control(pDX, IDC_STATICSUBSETWARNING, m_stcwarningmsg);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPasswordSubsetDlg, CPWDialog)
  //{{AFX_MSG_MAP(CPasswordSubsetDlg)
  ON_WM_CTLCOLOR()
  ON_MESSAGE(WM_DISPLAYPASSWORDSUBSET, OnDisplayStatus)
  ON_BN_CLICKED(IDC_COPYPASSWORD, OnCopy)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPasswordSubsetDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();
  
  Fonts::GetInstance()->ApplyPasswordFont(GetDlgItem(IDC_SUBSETRESULTS));

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

  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, TTS_BALLOON | TTS_NOPREFIX)) {
    pws_os::Trace(L"Unable To create CManagePSWDPols Dialog ToolTip\n");
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
  } else {
    EnableToolTips(TRUE);

    // Delay initial show & reshow
    int iTime = m_pToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
    m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, iTime * 4);
    m_pToolTipCtrl->Activate(TRUE);
    m_pToolTipCtrl->SetMaxTipWidth(500);
    AddTool(IDC_COPYPASSWORD, IDS_CLICKTOCOPYGENPSWD);
  }

  // Load bitmap
  UINT nImageID = PWSprefs::GetInstance()->GetPref(PWSprefs::UseNewToolbar) ?
                     IDB_COPYPASSWORD_NEW : IDB_COPYPASSWORD_CLASSIC;

  BOOL brc = m_CopyPswdBitmap.Attach(::LoadImage(
                                                 ::AfxFindResourceHandle(MAKEINTRESOURCE(nImageID), RT_BITMAP),
                                                 MAKEINTRESOURCE(nImageID), IMAGE_BITMAP, 0, 0,
                                                 (LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_SHARED)));

  ASSERT(brc);
  if (brc) {
    FixBitmapBackground(m_CopyPswdBitmap);

    CButton *pBtn = (CButton *)GetDlgItem(IDC_COPYPASSWORD);
    ASSERT(pBtn != NULL);
    if (pBtn != NULL)
      pBtn->SetBitmap(m_CopyPswdBitmap);
  }
  ShowWindow(SW_SHOW);
  return TRUE;
}

BOOL CPasswordSubsetDlg::PreTranslateMessage(MSG* pMsg)
{
  RelayToolTipEvent(pMsg);

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
  if (m_bshown)
    CPWDialog::EndDialog(4);
  else
    CPWDialog::OnCancel();
}

LRESULT CPasswordSubsetDlg::OnDisplayStatus(WPARAM /* wParam */, LPARAM /* lParam */)
{
  UpdateData(TRUE);
  m_stcwarningmsg.SetWindowText(L"");
  m_stcwarningmsg.ResetColour();
  m_subset.Trim();

  int icurpos(0);
  std::vector<int> vpos;
  CString resToken(m_subset);
  const size_t ipwlengh = m_passwd.length();

  while (resToken != L"" && icurpos != -1) {
    int lastpos = icurpos;
    resToken = m_subset.Tokenize(L";, ", icurpos);
    if (resToken == L"")
      continue;

    int ipos = _wtoi(resToken);
    // ipos can't be zero but is valid if:
    //   a. Positive: 1 <= ipos <= password_length
    //   b. Negative: -password_lengh < ipos <= -1
    if (abs(ipos) > (int)ipwlengh) {
      m_warningmsg.Format(ipos > 0 ? IDS_SUBSETINDEXTOOBIG : IDS_SUBSETINDEXTOOSMALL, ipwlengh);

      m_stcwarningmsg.SetWindowText(m_warningmsg);
      m_stcwarningmsg.SetColour(RGB(255, 0, 0));
      m_stcwarningmsg.Invalidate();
      vpos.clear();
      m_ne_subset.SetSel(lastpos, icurpos);
      m_ne_subset.SetFocus();

      // Disable Copy to Clipboard
      GetDlgItem(IDC_COPYPASSWORD)->EnableWindow(FALSE);

      return 0L;
    }
    if (ipos < 0)
      ipos = (int)ipwlengh + ipos + 1;

    vpos.push_back(ipos - 1);
  };

  std::vector<int>::const_iterator citer;
  StringX sSubset;
  for (citer = vpos.begin(); citer != vpos.end(); citer++) {
    sSubset += m_passwd[*citer];
    sSubset += L" ";
  }
  m_results.SetWindowText(sSubset.c_str());
  m_bshown = true;

  // Enable Copy to Clipboard
  GetDlgItem(IDC_COPYPASSWORD)->EnableWindow(TRUE);
  return 1L;
}

void CPasswordSubsetDlg::OnCopy()
{
  CSecString cs_data;

  int len = m_results.LineLength(m_results.LineIndex(0));
  m_results.GetLine(0, cs_data.GetBuffer(len), len);
  cs_data.ReleaseBuffer(len);

  // Remove blanks from between the characters
  cs_data.Remove(_T(' '));
  GetMainDlg()->SetClipboardData(cs_data);
  GetMainDlg()->UpdateLastClipboardAction(CItemData::PASSWORD);
}
