// InputBox.cpp : implementation file
//
/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "InputBox.h"

#include "resource3.h"

// CInputBox dialog

IMPLEMENT_DYNAMIC(CInputBox, CPWDialog)

CInputBox::CInputBox(UINT nIDCaption, CString csInitalText, int maxlen,
    const bool bReadOnly, CWnd *pParent)
  : CPWDialog(CInputBox::IDD, pParent), m_nIDCaption(nIDCaption), m_maxlen(maxlen),
  m_csText(csInitalText), m_bReadOnly(bReadOnly), m_bInitDone(false)
{ 
}

CInputBox::~CInputBox()
{
}

void CInputBox::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_INPUTBOX_TEXT, m_edText);
}

BEGIN_MESSAGE_MAP(CInputBox, CPWDialog)
  ON_WM_ACTIVATE()
  ON_BN_CLICKED(IDOK, OnOK)
  ON_EN_CHANGE(IDC_INPUTBOX_TEXT, OnInputChanged)
END_MESSAGE_MAP()

// CInputBox message handlers

BOOL CInputBox::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  CString csCaption(MAKEINTRESOURCE(m_nIDCaption));
  SetWindowText(csCaption);
  m_edText.SetWindowText(m_csText);

  if (m_maxlen > 0)
    m_edText.SetLimitText(m_maxlen);
  else
    GetDlgItem(IDC_INPUTBOX_CHAR_COUNT)->ShowWindow(SW_HIDE);

  if (m_bReadOnly) {
    // Hide the Cancel button and centre the OK button
    GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
    GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

    CString csClose(MAKEINTRESOURCE(IDS_CLOSE));
    GetDlgItem(IDOK)->SetWindowText(csClose);

    CRect dlgRect, btnRect;
    GetClientRect(&dlgRect);

    GetDlgItem(IDOK)->GetWindowRect(&btnRect);
    ScreenToClient(&btnRect);

    int ytop = btnRect.top;
    int xleft = (dlgRect.Width() / 2) - (btnRect.Width() / 2);
    GetDlgItem(IDOK)->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);

    // Make Edit control R-O
    m_edText.SetReadOnly(TRUE);
  }

  if (m_maxlen > 0) {
    CString csNumChars;
    csNumChars.Format(IDS_INPUTBOX_CHAR_COUNT, m_csText.GetLength());
    GetDlgItem(IDC_INPUTBOX_CHAR_COUNT)->SetWindowText(csNumChars);
  }

  GotoDlgCtrl(GetDlgItem(IDC_INPUTBOX_TEXT));

  return FALSE;
}

BOOL CInputBox::PreTranslateMessage(MSG *pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
    PostMessage(WM_COMMAND, MAKELONG(IDCANCEL, BN_CLICKED), NULL);
    return TRUE;
  }

  return CPWDialog::PreTranslateMessage(pMsg);
}

void CInputBox::OnActivate(UINT , CWnd *, BOOL )
{
  if (!m_bInitDone) {
    m_edText.SetSel(-1, 0);
    m_bInitDone = true;
  }
}

void CInputBox::OnOK() 
{
  UpdateData(TRUE);

  m_edText.GetWindowText(m_csText);

  CPWDialog::OnOK();
}

void CInputBox::OnInputChanged()
{
  if (m_maxlen > 0) {
    CString csText, csNumChars;
    m_edText.GetWindowText(csText);
    csNumChars.Format(IDS_INPUTBOX_CHAR_COUNT, csText.GetLength());
    GetDlgItem(IDC_INPUTBOX_CHAR_COUNT)->SetWindowText(csNumChars);
  }
}
