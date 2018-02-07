/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// ViewReport.cpp : implementation file
//

#include "stdafx.h"
#include "ViewReport.h"
#include "core/report.h"
#include "core/util.h"
#include "DboxMain.h"

// CViewReport dialog

IMPLEMENT_DYNAMIC(CViewReport, CPWResizeDialog)

CViewReport::CViewReport(CWnd* pParent /*=NULL*/,
                         CReport *pRpt /*=NULL*/)
  : CPWResizeDialog(CViewReport::IDD, pParent),
  m_pRpt(pRpt), m_bMemoryAllocOK(false)
{
  // Convert LF to CRLF
  StringX sxCRLF(L"\r\n"), sxLF(L"\n");
  m_pString = m_pRpt->GetString(); 
  Replace(m_pString, sxCRLF, sxLF);
  Replace(m_pString, sxLF, sxCRLF);

  m_dwDatasize = (DWORD)(m_pString.length() * sizeof(wchar_t));

  m_backgroundcolour = RGB(255, 255, 255);
  m_backgroundbrush.CreateSolidBrush(m_backgroundcolour);
  m_textcolor = ::GetSysColor(COLOR_WINDOWTEXT);
}

CViewReport::~CViewReport()
{
  m_backgroundbrush.DeleteObject();
}

void CViewReport::DoDataExchange(CDataExchange* pDX)
{
  CPWResizeDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_EDITREPORT, m_editreport);
}

BEGIN_MESSAGE_MAP(CViewReport, CPWResizeDialog)
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED(IDOK, Save)
  ON_BN_CLICKED(IDCANCEL, Finish)
  ON_BN_CLICKED(IDC_REPORT2CLIPBOARD, SendToClipboard)
END_MESSAGE_MAP()

// CViewReport message handlers

BOOL CViewReport::OnInitDialog()
{
  std::vector<UINT> vibottombtns;
  vibottombtns.push_back(IDOK);
  vibottombtns.push_back(IDC_REPORT2CLIPBOARD);
  vibottombtns.push_back(IDCANCEL);

  AddMainCtrlID(IDC_EDITREPORT);
  AddBtnsCtrlIDs(vibottombtns, 2);

  UINT statustext[1] = {IDS_BLANK};
  SetStatusBar(&statustext[0], 1, false);

  CPWResizeDialog::OnInitDialog();

  if (GetParent() == NULL)
    GetDlgItem(IDC_REPORT2CLIPBOARD)->EnableWindow(FALSE);

  // Get new edit string (as per MS doc.)
  HLOCAL h = ::LocalAlloc(LHND, m_dwDatasize + sizeof(wchar_t));
  if (h == NULL) {
    pws_os::Trace(L"ViewReport: Unable to allocate memory.  Can't do this properly!\n");
    m_editreport.SetWindowText(m_pString.c_str());
    return FALSE;
  }
  m_bMemoryAllocOK = true;

  LPCWSTR lpszText = (LPCWSTR)::LocalLock(h);
  memcpy((void *)lpszText, m_pString.c_str(), m_dwDatasize);

  // Now work out maximum size of dialog
  CClientDC dc(GetDlgItem(IDC_EDITREPORT));

  //get the size of the text
  CRect textRect(0, 0, 32767, 32767);
  CFont *pOldFont = dc.SelectObject(m_editreport.GetFont());

  // Get Height
  dc.DrawText(lpszText, (int)m_pString.length(), &textRect, DT_CALCRECT | DT_NOCLIP);

  // Get width of longest line - ignores tabs - but no issue as edit control has
  // horizontal scroll bars
  wchar_t pSeps[] = L"\r\n";
  int iMaxWidth(-1);
  // Capture individual lines:
  wchar_t *next_token;
  wchar_t *token = wcstok_s((LPWSTR)lpszText, pSeps, &next_token);
  while(token) {
    CSize sz = dc.GetTextExtent(token, (int)wcslen(token));
    if (sz.cx > iMaxWidth)
      iMaxWidth = sz.cx;
    token = wcstok_s(NULL, pSeps, &next_token);
  }

  dc.SelectObject(pOldFont);

  //get the size of the edit control and the dialog
  CRect editRect, dlgRect;
  m_editreport.GetClientRect(&editRect);
  GetClientRect(&dlgRect);

  // Get height and width of characters
  TEXTMETRIC tm;
  dc.GetTextMetrics(&tm);

  // Set size based on current size (add spare in case)
  int iAdditionalHeight(0), iAdditionalWidth(0);
  if (iMaxWidth > editRect.Width())
    iAdditionalWidth = (iMaxWidth - editRect.Width()) + 2 * tm.tmMaxCharWidth;
  if (textRect.Height() > editRect.Height())
    iAdditionalHeight = (textRect.Height() - editRect.Height()) + 2 * tm.tmHeight;

  // Set it
  SetMaxHeightWidth(dlgRect.Height() + iAdditionalHeight, 
                    dlgRect.Width()  + iAdditionalWidth);

  // Refresh data as _wcstok trashes it!
  memcpy((void *)lpszText, m_pString.c_str(), m_dwDatasize);
  ::LocalUnlock(h);

  // Free original handle
  ::LocalFree(m_editreport.GetHandle());
  // Set ours
  m_editreport.SetHandle(h);

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CViewReport::Save()
{
  m_pRpt->SaveToDisk();
  GetDlgItem(IDOK)->EnableWindow(FALSE);
}

void CViewReport::SendToClipboard()
{
  GetMainDlg()->SetClipboardData(m_pString);
}

void CViewReport::Finish()
{
  if (m_bMemoryAllocOK) {
    HLOCAL h = m_editreport.GetHandle();
    LPCWSTR lpszText = (LPCWSTR)::LocalLock(h);

    if (m_dwDatasize > 0) {
      trashMemory((void *)lpszText, m_dwDatasize);
      m_dwDatasize = 0;
    }

    ::LocalUnlock(h);
  }

  CPWResizeDialog::OnCancel();
}

HBRUSH CViewReport::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CPWResizeDialog::OnCtlColor(pDC, pWnd, nCtlColor);

  if (pWnd->GetDlgCtrlID() == IDC_EDITREPORT) {
    pDC->SetBkColor(m_backgroundcolour);
    pDC->SetTextColor(m_textcolor);
    return (HBRUSH) m_backgroundbrush;
  }
  
  return hbr;
}
