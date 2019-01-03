/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// SetDBID.cpp : implementation file
//

#include "stdafx.h"

#include "SetDBID.h"
#include "DboxMain.h"
#include "ThisMfcApp.h"
#include "SingleInstance.h"
#include "GeneralMsgBox.h"

#include "core/PWSprefs.h"

// CNPEdit Edit to prevent non numeric paste

BEGIN_MESSAGE_MAP(CNPEdit, CEdit)
  ON_MESSAGE(WM_PASTE, OnPaste)
END_MESSAGE_MAP()

LRESULT CNPEdit::OnPaste(WPARAM, LPARAM)
{
  // Only allow symbols to be pasted and stop duplicates
  if (!OpenClipboard() || !IsClipboardFormatAvailable(CF_UNICODETEXT))
    return 0L;

  std::wstring sClipData;
  HANDLE hData = GetClipboardData(CF_UNICODETEXT);
  if (hData != NULL) {
    wchar_t *buffer = (wchar_t *)GlobalLock(hData);
    if (buffer != NULL) {
      sClipData = buffer;
      GlobalUnlock(hData);
    }
  }
  CloseClipboard();

  // Get current text
  CString csData;
  GetWindowText(csData);

  // Check clipboard data is numeric and will fit
  if (sClipData.find_first_not_of(L"0123456789") != std::wstring::npos ||
      (csData.GetLength() + (int)sClipData.length()) > 2) {
    // There are non-numeric characters here or too long to fit - ignore paste
    return 0L;
  }

  // Do the paste function
  int nStart, nEnd;
  GetSel(nStart, nEnd);

  CString csNewText;
  csNewText = csData.Left(nStart) + sClipData.c_str() + csData.Mid(nEnd);
  SetWindowText(csNewText);
  
  return 1L;
}

// CSetDBID dialog

IMPLEMENT_DYNAMIC(CSetDBID, CPWDialog)

CSetDBID::CSetDBID(CWnd *pParent, int iDBIndex)
	: CPWDialog(IDD_SETDBID, pParent), m_pParent((DboxMain *)pParent),
  m_iInitialDBIndex(iDBIndex), m_iDBIndex(iDBIndex), m_iLockedTextColour(0),
  m_iUnLockedTextColour(0), m_bInitDone(false)
{
  // Locked
  m_clrLockedTextOptions[0] = RGB(255, 255,   0); // Yellow - default
  m_clrLockedTextOptions[1] = RGB(255,   0,   0); // Red
  m_clrLockedTextOptions[2] = RGB(254, 254, 254); // White-ish (if white then would come out transparent)
  m_clrLockedTextOptions[3] = RGB(  1,   1,   1); // Black-ish (if black then would come out transparent)

  // Unlocked
  m_clrUnlockedTextOptions[0] = RGB(255, 255,   0); // Yellow - default
  m_clrUnlockedTextOptions[1] = RGB(  0, 255,   0); // Green
  m_clrUnlockedTextOptions[2] = RGB(254, 254, 254); // White-ish (if white then would come out transparent)
  m_clrUnlockedTextOptions[3] = RGB(  1,   1,   1); // Black-ish (if black then would come out transparent)
}

CSetDBID::~CSetDBID()
{
}

void CSetDBID::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

  DDX_Text(pDX, IDC_DBINDEX, m_iDBIndex);
  DDX_Control(pDX, IDC_DBINDEX, m_edtSBIndex);
  DDV_MinMaxInt(pDX, m_iDBIndex, 0, 99);

  DDX_Radio(pDX, IDC_DBINDEX_TEXTCOLOURL0, m_iLockedTextColour);
  DDX_Radio(pDX, IDC_DBINDEX_TEXTCOLOURUL0, m_iUnLockedTextColour);

  DDX_Control(pDX, IDC_LOCKEDIMAGE, m_stLockedImage);
  DDX_Control(pDX, IDC_UNLOCKEDIMAGE, m_stUnlockedImage);
}

BEGIN_MESSAGE_MAP(CSetDBID, CDialog)
  ON_WM_DESTROY()
  ON_WM_CTLCOLOR()

  ON_BN_CLICKED(IDOK, OnOK)
  ON_BN_CLICKED(IDCANCEL, OnCancel)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_COMMAND_RANGE(IDC_DBINDEX_TEXTCOLOURL0, IDC_DBINDEX_TEXTCOLOURL3, OnSetLockedColour)
  ON_COMMAND_RANGE(IDC_DBINDEX_TEXTCOLOURUL0, IDC_DBINDEX_TEXTCOLOURUL3, OnSetUnlockedColour)
END_MESSAGE_MAP()

// CSetDBID message handlers

BOOL CSetDBID::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  // Trying to get our icon to get background colour (failed when System Tray
  // icon created in ThisMafApp as too early - no main dialog HWND).
  // Windows 10 default is black but Windows 7 seems to be white
  NOTIFYICONIDENTIFIER nii = { sizeof(nii) };
  nii.hWnd = m_pParent->GetSafeHwnd();
  nii.uID = PWS_MSG_ICON_NOTIFY;
  nii.guidItem = GUID_NULL;
  RECT rcIcon;
  HRESULT hr = Shell_NotifyIconGetRect(&nii, &rcIcon);

  COLORREF clr = (COLORREF)-1;
  if (SUCCEEDED(hr)) {
    CWnd *pWnd = CWnd::GetDesktopWindow();
    CDC *pDC = pWnd->GetDC();
    clr = pDC->GetPixel(rcIcon.right + 1, rcIcon.top + 1);
    pws_os::Trace(L"clr: %06x; R=%d; G=%d; B=%d\n", clr, GetRValue(clr), GetGValue(clr), GetBValue(clr));
    clr = RGB(GetRValue(clr), GetGValue(clr), GetBValue(clr));
    ReleaseDC(pDC);
  }

  m_clrBackground = (clr != (COLORREF)-1) ? clr : GetSysColor(COLOR_BACKGROUND);

  m_Brush.CreateSolidBrush(m_clrBackground);

  VERIFY(m_bmLocked.Attach(::LoadImage(
    ::AfxFindResourceHandle(MAKEINTRESOURCE(IDB_LOCKED_TRAY_INDEX), RT_BITMAP),
    MAKEINTRESOURCE(IDB_LOCKED_TRAY_INDEX), IMAGE_BITMAP, 0, 0,
                                    (LR_DEFAULTSIZE | LR_CREATEDIBSECTION))));

  VERIFY(m_bmUnlocked.Attach(::LoadImage(
    ::AfxFindResourceHandle(MAKEINTRESOURCE(IDB_UNLOCKED_TRAY_INDEX), RT_BITMAP),
    MAKEINTRESOURCE(IDB_UNLOCKED_TRAY_INDEX), IMAGE_BITMAP, 0, 0,
                                    (LR_DEFAULTSIZE | LR_CREATEDIBSECTION))));

  SetBitmapBackground(m_bmLocked);
  SetBitmapBackground(m_bmUnlocked);

  m_clrLockedTextColour = m_pParent->GetLockedIndexColour();
  m_clrUnlockedTextColour = m_pParent->GetUnlockedIndexColour();

  for (int i = 0; i < 4; i++) {
    if (m_clrLockedTextOptions[i] == m_clrLockedTextColour) {
      m_iLockedTextColour = i;
      break;
    }
  }

  for (int i = 0; i < 4; i++) {
    if (m_clrUnlockedTextOptions[i] == m_clrUnlockedTextColour) {
      m_iUnLockedTextColour = i;
      break;
    }
  }

  CreateIndexBitmap(55, m_clrLockedTextColour, true);
  CreateIndexBitmap(55, m_clrUnlockedTextColour, false);

  // Index if between 1 & 99
  m_edtSBIndex.SetLimitText(2);

  UpdateData(FALSE);

  GotoDlgCtrl((CWnd *)&m_edtSBIndex);

  m_bInitDone = true;

  return FALSE;  // return TRUE unless you set the focus to a control
}

void CSetDBID::OnDestroy()
{
  if (m_bInitDone) {
    m_bmLocked.Detach();
    m_bmUnlocked.Detach();

    m_Brush.DeleteObject();
  }
}

void CSetDBID::OnHelp()
{
  ShowHelp(L"::/html/system_tray.html");
}

HBRUSH CSetDBID::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CPWDialog::OnCtlColor(pDC, pWnd, nCtlColor);

  // Only deal with Static controls and then
  // Only with our special ones
  if (nCtlColor == CTLCOLOR_STATIC && m_clrBackground != (COLORREF)-1) {
    switch (pWnd->GetDlgCtrlID()) {
    case IDC_LOCKEDIMAGE:
    case IDC_UNLOCKEDIMAGE:
      pDC->SetBkMode(TRANSPARENT);
      pDC->SetBkColor(m_clrBackground);
      pDC->SetTextColor(m_clrBackground);
      return (HBRUSH)m_Brush.GetSafeHandle();
    }
  }

  // Let's get out of here
  return hbr;
}

void CSetDBID::OnOK()
{
  UpdateData(TRUE);

  // Has user changed the index?
  if (m_iInitialDBIndex != m_iDBIndex) {
    // Has user disabled index feature?
    if (m_iDBIndex != 0) {
      // Try to get this index
      wchar_t szName[MAX_PATH];
      CreateUniqueName(UNIQUE_PWS_GUID, szName, MAX_PATH, SI_TRUSTEE_UNIQUE);

      CString csUserSBIndex;
      csUserSBIndex.Format(L"%s:DBI:%02d", static_cast<LPCWSTR>(szName), m_iDBIndex);

      m_hMutexDBIndex = CreateMutex(NULL, FALSE, csUserSBIndex);

      DWORD dwerr = ::GetLastError();
      if (dwerr == ERROR_ALREADY_EXISTS || dwerr == ERROR_ACCESS_DENIED) {
        CGeneralMsgBox gmb;
        gmb.AfxMessageBox(IDS_DBIDINUSE, MB_OK | MB_ICONEXCLAMATION);
        m_edtSBIndex.SetFocus();
        return;
      }
    }
  }

  CPWDialog::EndDialog(m_iDBIndex);
}

void CSetDBID::OnCancel()
{
  CPWDialog::EndDialog(-1);
}

void CSetDBID::OnSetLockedColour(UINT /*nID*/)
{
  UpdateData(TRUE);

  COLORREF clrLockedText = m_clrLockedTextOptions[m_iLockedTextColour];

  if (m_clrLockedTextColour != clrLockedText) {
    CreateIndexBitmap(55, clrLockedText, true);

    m_stLockedImage.UpdateWindow();
    m_clrLockedTextColour = clrLockedText;
  }
}

void CSetDBID::OnSetUnlockedColour(UINT /*nID*/)
{
  UpdateData(TRUE);

  COLORREF clrUnlockedText = m_clrUnlockedTextOptions[m_iUnLockedTextColour];

  if (m_clrUnlockedTextColour != clrUnlockedText) {
    CreateIndexBitmap(55, clrUnlockedText, false);

    m_stUnlockedImage.UpdateWindow();
    m_clrUnlockedTextColour = clrUnlockedText;
  }
}

void CSetDBID::SetBitmapBackground(CBitmap &bm)
{
  // Get how many pixels in the bitmap
  BITMAP bmInfo;
  bm.GetBitmap(&bmInfo);

  const UINT numPixels(bmInfo.bmHeight * bmInfo.bmWidth);

  // get a pointer to the pixels
  DIBSECTION ds;
  VERIFY(bm.GetObject(sizeof(DIBSECTION), &ds) == sizeof(DIBSECTION));

  RGBTRIPLE *pixels = reinterpret_cast<RGBTRIPLE*>(ds.dsBm.bmBits);
  ASSERT(pixels != NULL);

  const RGBTRIPLE newbkgrndColourRGB = { GetBValue(m_clrBackground),
                                         GetGValue(m_clrBackground),
                                         GetRValue(m_clrBackground) };

  for (UINT i = 0; i < numPixels; ++i) {
    if (pixels[i].rgbtBlue == 255 &&
      pixels[i].rgbtGreen == 0 &&
      pixels[i].rgbtRed == 255) {
      pixels[i] = newbkgrndColourRGB;
    }
  }
}

void CSetDBID::CreateIndexBitmap(const int iIndex, const COLORREF clrText, const bool bLocked)
{
  CString csValue;
  csValue.Format(L"%2d", iIndex);

  // Get bitmap size
  BITMAP bm;
  GetObject(bLocked ? m_bmLocked : m_bmUnlocked, sizeof(BITMAP), &bm);

  // Create bitmap attributes
  BITMAPINFO bmInfo;
  memset(&bmInfo.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
  bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmInfo.bmiHeader.biWidth = bm.bmWidth;
  bmInfo.bmiHeader.biHeight = bm.bmHeight;
  bmInfo.bmiHeader.biPlanes = 1;
  bmInfo.bmiHeader.biBitCount = 24;

  HDC hDC = ::GetDC(NULL);
  HDC hMemDC = ::CreateCompatibleDC(hDC);

  // Create new bitmap
  BYTE *pbase(NULL);
  HBITMAP hNewBmp = CreateDIBSection(hDC, &bmInfo, DIB_RGB_COLORS, (void **)&pbase, 0, 0);
  HGDIOBJ hOldBmp = ::SelectObject(hMemDC, hNewBmp);

  // Copy background bitmap
  HDC hdcBmp = CreateCompatibleDC(hMemDC);
  HGDIOBJ hOldBackground = SelectObject(hdcBmp, bLocked ? m_bmLocked : m_bmUnlocked);
  BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hdcBmp, 0, 0, SRCCOPY);
  ::SelectObject(hdcBmp, hOldBackground);
  ::DeleteDC(hdcBmp);

  // Create font
  LOGFONT lf = { 0 };
  lf.lfHeight = -22;
  lf.lfWeight = FW_NORMAL;
  lf.lfOutPrecision = PROOF_QUALITY;
  lf.lfQuality = ANTIALIASED_QUALITY;
  wmemset(lf.lfFaceName, 0, LF_FACESIZE);
  lstrcpy(lf.lfFaceName, L"Arial Black");

  HFONT hFont = ::CreateFontIndirect(&lf);
  HGDIOBJ hOldFont = ::SelectObject(hMemDC, hFont);

  // Write text - Do NOT use SetTextAlign
  ::SetBkMode(hMemDC, TRANSPARENT);
  ::SetTextColor(hMemDC, clrText);
  ::TextOut(hMemDC, 0, 0, (LPCWSTR)csValue, 2);

  // Cleanup font
  ::SelectObject(hMemDC, hOldFont);
  ::DeleteObject(hFont);

  // Release background bitmap
  ::SelectObject(hMemDC, hOldBmp);

  ::DeleteDC(hMemDC);
  ::ReleaseDC(NULL, hDC);

  // Set new image in picture control, whcih takes ownership
  HBITMAP hbmOldIndex;
  if (bLocked) {
    hbmOldIndex = m_stLockedImage.SetBitmap(hNewBmp);
  } else {
    hbmOldIndex = m_stUnlockedImage.SetBitmap(hNewBmp);
  }

  ::DeleteObject(hbmOldIndex);
}
