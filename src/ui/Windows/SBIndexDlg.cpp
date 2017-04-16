/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// SBIndexDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SBIndexDlg.h"
#include "SingleInstance.h"
#include "GeneralMsgBox.h"

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

// CSBIndexDlg dialog

IMPLEMENT_DYNAMIC(CSBIndexDlg, CPWDialog)

CSBIndexDlg::CSBIndexDlg(CWnd* pParent, int iDBIndex)
	: CPWDialog(IDD_SETDBINDEX, pParent), m_iInitialDBIndex(iDBIndex), m_iDBIndex(iDBIndex)
{
}

CSBIndexDlg::~CSBIndexDlg()
{
}

void CSBIndexDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

  DDX_Text(pDX, IDC_DBINDEX, m_iDBIndex);
  DDX_Control(pDX, IDC_DBINDEX, m_edtSBIndex);
  DDV_MinMaxInt(pDX, m_iDBIndex, 0, 99);
}

BEGIN_MESSAGE_MAP(CSBIndexDlg, CDialog)
  ON_BN_CLICKED(IDOK, OnOK)
  ON_BN_CLICKED(IDCANCEL, OnCancel)
  ON_BN_CLICKED(ID_HELP, OnHelp)
END_MESSAGE_MAP()

// CSBIndexDlg message handlers

BOOL CSBIndexDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  m_edtSBIndex.SetLimitText(2);

  UpdateData(FALSE);

  GotoDlgCtrl((CWnd *)&m_edtSBIndex);

  return FALSE;  // return TRUE unless you set the focus to a control
}

void CSBIndexDlg::OnHelp()
{
  ShowHelp(L"::/html/manage_menu.html#SetDBIndex");
}

void CSBIndexDlg::OnOK()
{
  UpdateData(TRUE);

  if (m_iInitialDBIndex != m_iDBIndex) {
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
        gmb.AfxMessageBox(IDS_DBINDEXINUSE, MB_OK | MB_ICONEXCLAMATION);
        m_edtSBIndex.SetFocus();
        return;
      }
    }
  }

  CPWDialog::EndDialog(m_iDBIndex);
}

void CSBIndexDlg::OnCancel()
{
  CPWDialog::EndDialog(-1);
}


