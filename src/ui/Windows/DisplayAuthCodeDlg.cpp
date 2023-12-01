/*
* Copyright (c) 2003-2023 Rony Shapiro <ronys@pwsafe.org>.
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
    DDX_Control(pDX, IDC_AC_BUTTON_COPY_TWOFACTORCODE, m_btnCopyTwoFactorCode);
    DDX_Control(pDX, IDC_AC_STATIC_TWOFACTORCODE, m_stcTwoFactorCode);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDisplayAuthCodeDlg, CPWDialog)
  //{{AFX_MSG_MAP(CDisplayAuthCodeDlg)
  ON_WM_SHOWWINDOW()
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
  if ((rect.top == -1 && rect.bottom == -1 && rect.left == -1 && rect.right == -1) || !RectInRegion(hrgnWork, rect)){
    rect = dlgRect;
  }
  ::DeleteObject(hrgnWork);

  // Ignore size just set position
  SetWindowPos(NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

  if (InitToolTip(TTS_BALLOON | TTS_NOPREFIX, 0)) {
    AddTool(IDC_AC_BUTTON_COPY_TWOFACTORCODE, IDS_TWOFACTORCODEBUTTON);
    AddTool(IDC_AC_STATIC_TWOFACTORCODE, IDS_AC_STATIC_TWOFACTORCODE);
    ActivateToolTip();
  }

  CItemData* pci = GetItem();
  ASSERT(pci != NULL);
  if (!pci)
    return TRUE;

  // Each of Group, Title, Username is on a separate "row."
  // If a value has text, set the control, else mark the "row" for removal.
  const std::vector<StringX> vRowData = { pci->GetGroup(), pci->GetTitle(), pci->GetUser() };
  const int nColumns = 2;
  const int nRows = static_cast<int>(vRowData.size());
  std::vector<int> vRowsToRemove;
  int row = 0;
  for (auto& v : vRowData) {
    if (v.empty())
      vRowsToRemove.push_back(row);
    else
      GetDlgItem(IDC_AC_EDIT_GROUP + (row * nColumns))->SetWindowText(v.c_str());
    row++;
  }

  // Remove the rows without values.
  int cyVertDistanceTotal;
  if (WinUtil::RemoveControlRows(*this, IDC_AC_LABEL_GROUP, nColumns, nRows, vRowsToRemove, cyVertDistanceTotal)) {
    // Move everything else up.
    WinUtil::MoveControlDelta(*GetDlgItem(IDC_AC_BUTTON_COPY_TWOFACTORCODE), 0, -cyVertDistanceTotal);
    WinUtil::MoveControlDelta(*GetDlgItem(IDC_AC_STATIC_TWOFACTORCODE), 0, -cyVertDistanceTotal);
    WinUtil::MoveControlDelta(*GetDlgItem(IDOK), 0, -cyVertDistanceTotal);
    CRect rc;
    GetWindowRect(&rc);
    rc.bottom -= cyVertDistanceTotal;
    MoveWindow(&rc);
  }

  if (Fonts::CreateFontMatchingWindowHeight(m_stcTwoFactorCode, m_fontTwoFactorCode))
    m_stcTwoFactorCode.SetFont(&m_fontTwoFactorCode);

  SetupAuthenticationCodeUiElements();

  ShowWindow(SW_SHOW);

  return TRUE;  // return TRUE unless you set the focus to a control
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
    if (GetMainDlg()->IsLastSensitiveClipboardItemPresent())
      CopyAuthCodeToClipboard();
    else
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
