/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
* Contributed by Ashley R. Thomas, 29-Nov-2023
*/
/// \file DisplayAuthCodeDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "DisplayAuthCodeDlg.h"
#include "DboxMain.h"
#include "Fonts.h"
#include "winutils.h"

#include "core/StringX.h"
#include "core/PWSprefs.h"

#include <vector>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
CDisplayAuthCodeDlg::CDisplayAuthCodeDlg(CWnd* pParent, PWScore& core, const pws_os::CUUID& uuidEntry)
  :
  CPWDialog(CDisplayAuthCodeDlg::IDD, pParent),
  m_core(core),
  m_uuidItem(uuidEntry),
  m_bCopyToClipboard(false)
{
  ASSERT(&m_core != NULL);
}

CDisplayAuthCodeDlg::~CDisplayAuthCodeDlg()
{
}

void CDisplayAuthCodeDlg::DoDataExchange(CDataExchange* pDX)
{
    CPWDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDisplayAuthCodeDlg)
    DDX_Control(pDX, IDC_AC_STATIC_ENTRYNAME, m_stcEntryName);
    DDX_Control(pDX, IDC_AC_BUTTON_COPY_TWOFACTORCODE, m_btnCopyTwoFactorCode);
    DDX_Control(pDX, IDC_AC_STATIC_TWOFACTORCODE, m_stcTwoFactorCode);
    DDX_Control(pDX, IDOK, m_btnClose);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDisplayAuthCodeDlg, CPWDialog)
  //{{AFX_MSG_MAP(CDisplayAuthCodeDlg)
  ON_WM_SHOWWINDOW()
  ON_WM_GETMINMAXINFO()
  ON_WM_WINDOWPOSCHANGING()
  ON_WM_SIZE()
  ON_WM_TIMER()
  ON_BN_CLICKED(IDC_AC_BUTTON_COPY_TWOFACTORCODE, OnCopyTwoFactorCode)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CDisplayAuthCodeDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  // Place dialog where user had it last
  CRect rect, dlgRect;
  GetWindowRect(&dlgRect);
  PWSprefs::GetInstance()->GetPrefPSSRect(rect.top, rect.bottom, 
                                          rect.left, rect.right);
  HRGN hrgnWork = WinUtil::GetWorkAreaRegion();
  // Ensure window will be visible.
  if ((rect.top == -1 && rect.bottom == -1 && rect.left == -1 && rect.right == -1) || !RectInRegion(hrgnWork, rect)) {
    rect = dlgRect;
  }
  ::DeleteObject(hrgnWork);
  SetWindowPos(NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

  if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
    // The tooltips for this control look much better on 1080/4K
    // with max at 400 rather than default of 300.
    m_pToolTipCtrl->SetMaxTipWidth(400);
    AddTool(IDC_AC_BUTTON_COPY_TWOFACTORCODE, IDS_TWOFACTORCODEBUTTON_CONFIGURED);
    AddTool(IDC_AC_STATIC_TWOFACTORCODE, IDS_AC_STATIC_TWOFACTORCODE);
    ActivateToolTip();
  }

  GetClientRect(&m_rcInitial);
  GetWindowRect(&m_rwInitial);
  m_cxMinWidth = m_rwInitial.Width();

  CItemData* pci = GetItem();
  ASSERT(pci != NULL);
  if (!pci)
    return TRUE;

  // Initial entry name size and location.
  m_stcEntryName.GetWindowRect(&m_rectInitialEntryName);
  ScreenToClient(&m_rectInitialEntryName);

  // Create the actual entry name: «group» «title» «username»
  CString csEntryName;
  StringX sx_group(L""), sx_title, sx_user(L"");
  if (!pci->IsGroupEmpty())
    sx_group = pci->GetGroup();
  sx_title = pci->GetTitle();
  if (!pci->IsUserEmpty())
    sx_user = pci->GetUser();
  csEntryName.Format(IDS_DISPLAYAUTHCODE_TITLEFMT, sx_group.c_str(), sx_title.c_str(), sx_user.c_str());
  m_stcEntryName.SetWindowText(csEntryName);

  // Create a pseudo long entry name used in calculations that help put a limit
  // on the effects of dialog minimum width as affected by the entry name width.
  CString csMockLongEntryName;
  csMockLongEntryName.Format(IDS_DISPLAYAUTHCODE_TITLEFMT, L"Some Long Group Name", L"Some Long Title Here, Inc.", L"somelonguser@email.local");

  // Calc and use the minimum width of both m_stcEntryName and csMockLongEntryName...
  CDC* pDC = m_stcEntryName.GetDC();
  CFont* pfontEntryName = m_stcEntryName.GetFont();
  CFont* pofont = pDC->SelectObject(pfontEntryName);
  TEXTMETRIC tm;
  pDC->GetTextMetrics(&tm);
  int cxEntryNameText = pDC->GetTextExtent(csEntryName).cx + tm.tmAveCharWidth;
  int cxLongEntryNameWidth = pDC->GetTextExtent(csMockLongEntryName).cx + tm.tmAveCharWidth;
  // Pick the smallest to use for guiding dialog minimum width.
  int cxEntryNameWidthToUse = std::min(cxLongEntryNameWidth, cxEntryNameText);
  pDC->SelectObject(pofont);
  m_stcEntryName.ReleaseDC(pDC);
  // Adjust dialog min width if entry name min width has increased.
  if (cxEntryNameWidthToUse > m_rectInitialEntryName.Width())
    m_cxMinWidth += cxEntryNameWidthToUse - m_rectInitialEntryName.Width();

  // Calc initial auth code static control margin.
  LOGFONT lf;
  CFont* pFont = m_stcTwoFactorCode.GetFont();
  pFont->GetLogFont(&lf);
  CRect rcTwoFactorCode;
  m_stcTwoFactorCode.GetWindowRect(&rcTwoFactorCode);
  int cyTwoFactorCodeInitialInternalMargin = (rcTwoFactorCode.Height() - abs(lf.lfHeight) + 1) / 2;

  // Change auth code static font to password font.
  Fonts* pFonts = Fonts::GetInstance();
  pFonts->ApplyPasswordFont(&m_stcTwoFactorCode);
  pFont = m_stcTwoFactorCode.GetFont();
  LOGFONT lfAuthCode;
  pFont->GetLogFont(&lfAuthCode);
  int cyFontHeight = abs(lfAuthCode.lfHeight);

  m_btnCopyTwoFactorCode.GetWindowRect(&m_rectInitialAuthCodeButton);
  ScreenToClient(&m_rectInitialAuthCodeButton);
  m_cyAuthCodeButtonMarginBottom = m_rcInitial.bottom - m_rectInitialAuthCodeButton.bottom;

  // Get initial auth code static rect.
  m_stcTwoFactorCode.GetWindowRect(&m_rectInitialAuthCode);
  // Resize height to match password font.
  m_rectInitialAuthCode.bottom = m_rectInitialAuthCode.top + cyFontHeight + (cyTwoFactorCodeInitialInternalMargin * 2);
  ScreenToClient(&m_rectInitialAuthCode);
  // Center it vertically based on the button height.
  int authCodeTop = m_rectInitialAuthCodeButton.top + (m_rectInitialAuthCodeButton.Height() - m_rectInitialAuthCode.Height()) / 2;
  m_rectInitialAuthCode.MoveToY(authCodeTop);
  m_stcTwoFactorCode.MoveWindow(m_rectInitialAuthCode.left, m_rectInitialAuthCode.top, m_rectInitialAuthCode.Width(), m_rectInitialAuthCode.Height());

  // Get initial Close button rect.
  m_btnClose.GetWindowRect(&m_rectInitialCloseButton);
  ScreenToClient(&m_rectInitialCloseButton);

  SetupAuthenticationCodeUiElements();

  ShowWindow(SW_SHOW);

  SetWindowPos(NULL, 0, 0, m_cxMinWidth, m_rwInitial.Height(), SWP_NOMOVE | SWP_NOZORDER);

  return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CDisplayAuthCodeDlg::PreTranslateMessage(MSG* pMsg)
{
  RelayToolTipEvent(pMsg);
  return CPWDialog::PreTranslateMessage(pMsg);
}

void CDisplayAuthCodeDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
  CPWDialog::OnGetMinMaxInfo(lpMMI);
  if (!IsWindowVisible())
    return;
  lpMMI->ptMinTrackSize.x = std::max(lpMMI->ptMinTrackSize.x, static_cast<LONG>(m_cxMinWidth));
  lpMMI->ptMinTrackSize.y = std::max(lpMMI->ptMinTrackSize.y, static_cast<LONG>(m_rwInitial.Height()));
}

void CDisplayAuthCodeDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
  CPWDialog::OnWindowPosChanging(lpwndpos);
  if (!IsWindowVisible())
    return;
  if (lpwndpos->flags & SWP_NOSIZE)
    return;
  // Do not let the width to height ratio shrink below its initial value.
  double ro = m_cxMinWidth / static_cast<double>(m_rwInitial.Height());
  double rc = lpwndpos->cx / static_cast<double>(lpwndpos->cy);
  if (rc < ro)
    lpwndpos->cx = static_cast<int>(lpwndpos->cy * ro);
}

void CDisplayAuthCodeDlg::OnSize(UINT nType, int cx, int cy)
{
  CPWDialog::OnSize(nType, cx, cy);
  if (!IsWindowVisible())
    return;
  if (nType == SIZE_MINIMIZED)
    return;

  CRect rc;
  GetClientRect(&rc);

  int cxEntryNameMargin = m_rectInitialEntryName.left - rc.left;
  CRect rectEntryName;
  m_stcEntryName.GetWindowRect(&rectEntryName);
  ScreenToClient(&rectEntryName);
  rectEntryName.right = rc.right - cxEntryNameMargin;
  m_stcEntryName.MoveWindow(
    rectEntryName.left,
    rectEntryName.top,
    rectEntryName.Width(),
    rectEntryName.Height()
  );
  m_stcEntryName.Invalidate();

  // Calc new button size. It is a square so width == height as it changes size.
  CRect rectButtonAuthCode;
  m_btnCopyTwoFactorCode.GetWindowRect(&rectButtonAuthCode);
  ScreenToClient(&rectButtonAuthCode);
  rectButtonAuthCode.bottom = rc.bottom - m_cyAuthCodeButtonMarginBottom;
  rectButtonAuthCode.right = rectButtonAuthCode.left + rectButtonAuthCode.Height();
  m_btnCopyTwoFactorCode.MoveWindow(
    rectButtonAuthCode.left,
    rectButtonAuthCode.top,
    rectButtonAuthCode.Width(),
    rectButtonAuthCode.Height()
  );

  // The two factor code static is center vertically in relation to the auth code button height.
  // Its width stretches, but its height remains the same, matching the password font height.
  CRect rectTwoFactorCode;
  m_stcTwoFactorCode.GetWindowRect(&rectTwoFactorCode);
  ScreenToClient(&rectTwoFactorCode);
  rectTwoFactorCode.left = rectButtonAuthCode.right + (m_rectInitialAuthCode.left - m_rectInitialAuthCodeButton.right);
  rectTwoFactorCode.right = rc.right - (m_rcInitial.right - m_rectInitialAuthCode.right);
  int cyTwoFactorCodeHeight = rectTwoFactorCode.Height();
  rectTwoFactorCode.top = rectButtonAuthCode.top + ((rectButtonAuthCode.Height() - rectTwoFactorCode.Height()) / 2);
  rectTwoFactorCode.bottom = rectTwoFactorCode.top + cyTwoFactorCodeHeight;
  m_stcTwoFactorCode.MoveWindow(
    rectTwoFactorCode.left,
    rectTwoFactorCode.top,
    rectTwoFactorCode.Width(),
    rectTwoFactorCode.Height()
  );
  m_stcTwoFactorCode.Invalidate();

  // The Close button remains the same size, located in the lower/right.
  int xClose = rc.right - (m_rcInitial.right - m_rectInitialCloseButton.left);
  int yClose = rc.bottom - (m_rcInitial.bottom - m_rectInitialCloseButton.top);
  m_btnClose.MoveWindow(xClose, yClose, m_rectInitialCloseButton.Width(), m_rectInitialCloseButton.Height());
  m_btnClose.Invalidate();
}

void CDisplayAuthCodeDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
  if (bShow) {
    SetupAuthenticationCodeUiElements();
  } else {
    StopAuthenticationCodeUi();
  }
  CPWDialog::OnShowWindow(bShow, nStatus);
}

void CDisplayAuthCodeDlg::SetupAuthenticationCodeUiElements()
{
  m_btnCopyTwoFactorCode.SetPieColor(RGB(0, 192, 255));
  m_btnCopyTwoFactorCode.SetPercent(0);
  CItemData* pciCred = GetCredentialItem();
  if (!pciCred)
    return;
  bool isTwoFactorKey = pciCred->GetTwoFactorKeyLength() > 0;
  m_btnCopyTwoFactorCode.EnableWindow(isTwoFactorKey);
  m_btnCopyTwoFactorCode.ShowWindow(isTwoFactorKey ? SW_SHOW : SW_HIDE);
  if (isTwoFactorKey)
    SetTimer(TIMER_TWO_FACTOR_AUTH_CODE_COUNTDOWN, USER_TIMER_MINIMUM, NULL);
  else
    KillTimer(TIMER_TWO_FACTOR_AUTH_CODE_COUNTDOWN);
}

void CDisplayAuthCodeDlg::StopAuthenticationCodeUi()
{
  m_bCopyToClipboard = false;
  KillTimer(TIMER_TWO_FACTOR_AUTH_CODE_COUNTDOWN);
}

bool CDisplayAuthCodeDlg::UpdateAuthCode(CItemData* pciCred)
{
  StringX sxAuthCode;
  time_t time_now;
  double ratio;
  if (PWSTotp::GetNextTotpAuthCodeString(*pciCred, sxAuthCode, &time_now, &ratio) != PWSTotp::Success)
    return false;

  bool bNewCode = false;
  if (sxAuthCode != m_sxLastAuthCode) {
    bNewCode = true;
    m_stcTwoFactorCode.SetWindowText(sxAuthCode.c_str());
    m_sxLastAuthCode = sxAuthCode;
  }

  m_btnCopyTwoFactorCode.SetPercent(100.0 * ratio);

  return bNewCode;
}

void CDisplayAuthCodeDlg::OnTimer(UINT_PTR nIDEvent)
{
  if (nIDEvent != TIMER_TWO_FACTOR_AUTH_CODE_COUNTDOWN) {
    CPWDialog::OnTimer(nIDEvent);
    return;
  }

  CItemData* pciCred = GetCredentialItem();
  if (!pciCred) {
    StopAuthenticationCodeUi();
    return;
  }

  if (pciCred->GetTwoFactorKeyLength() == 0) {
    StopAuthenticationCodeUi();
    return;
  }

  bool bNewCode = UpdateAuthCode(pciCred);

  if (bNewCode && m_bCopyToClipboard) {
    ClipboardStatus clipboardStatus = GetMainDlg()->GetLastSensitiveClipboardItemStatus();
    if (clipboardStatus == ClipboardStatus::SuccessSensitivePresent)
      CopyAuthCodeToClipboard();
    else if (clipboardStatus != ClipboardStatus::ClipboardNotAvailable)
      m_bCopyToClipboard = false;
  }
}

void CDisplayAuthCodeDlg::CopyAuthCodeToClipboard()
{
  bool bDataSet = GetMainDlg()->SetClipboardData(m_sxLastAuthCode);
  ASSERT(bDataSet);
  if (bDataSet)
    GetMainDlg()->UpdateLastClipboardAction(ClipboardDataSource::AuthCode);
}

void CDisplayAuthCodeDlg::OnCopyTwoFactorCode()
{
  CItemData* pciCred = GetCredentialItem();
  if (!pciCred) {
    return;
  }
  m_bCopyToClipboard = true;
  CopyAuthCodeToClipboard();
  UpdateAuthCode(pciCred);
}
