/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// Fieldchanges.cpp : implementation file
//

#include "stdafx.h"

#include "FieldchangesDlg.h"
#include "Fonts.h"

#include "core/ItemData.h"

CFieldchangesDlg::CFieldchangesDlg(CWnd *pParent, CItemData *pci,
  const StringX &sxOriginal_Text, const StringX &sxNew_Text, const bool bIsNotes)
  : CPWDialog(CFieldchangesDlg::IDD, pParent), m_pci(pci), m_bIsNotes(bIsNotes),
  m_sxOriginal_Text(sxOriginal_Text), m_sxNew_Text(sxNew_Text)
{
}

CFieldchangesDlg::~CFieldchangesDlg()
{
}

BEGIN_MESSAGE_MAP(CFieldchangesDlg, CPWDialog)
END_MESSAGE_MAP()

void CFieldchangesDlg::DoDataExchange(CDataExchange *pDX)
{
  CPWDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_GROUP, m_ex_group);
  DDX_Control(pDX, IDC_TITLE, m_ex_title);
  DDX_Control(pDX, IDC_USERNAME, m_ex_username);

  DDX_Control(pDX, IDC_ORIGINAL_TEXT, m_SSRE_Original_Text);
  DDX_Control(pDX, IDC_NEW_TEXT, m_SSRE_New_Text);
}

BOOL CFieldchangesDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  // Get Add/Edit font
  Fonts *pFonts = Fonts::GetInstance();
  CFont *pFont = pFonts->GetAddEditFont();

  // Change font size of the group, title, username
  m_ex_group.SetFont(pFont);
  m_ex_title.SetFont(pFont);
  m_ex_username.SetFont(pFont);

  if (m_bIsNotes) {
    pFont = pFonts->GetNotesFont();
    m_SSRE_Original_Text.SetFont(pFont);
    m_SSRE_New_Text.SetFont(pFont);
  } else {
    m_SSRE_Original_Text.SetFont(pFont);
    m_SSRE_New_Text.SetFont(pFont);
  }

  m_ex_group.SetWindowText(m_pci->GetGroup().c_str());
  m_ex_title.SetWindowText(m_pci->GetTitle().c_str());
  m_ex_username.SetWindowText(m_pci->GetUser().c_str());

  // CRichEditCtrlExtn doesn't understand \r\n in terms of character counting
  Replace(m_sxOriginal_Text, StringX(L"\r\n"), StringX(L"\n"));
  Replace(m_sxNew_Text, StringX(L"\r\n"), StringX(L"\n"));

  // Set up scrolling partners
  m_SSRE_Original_Text.SetPartner(&m_SSRE_New_Text);
  m_SSRE_Original_Text.SetWindowText(m_sxOriginal_Text.c_str());
  m_SSRE_Original_Text.SetTargetDevice(NULL, 0);
  m_SSRE_Original_Text.SetSel(0, 0);

  m_SSRE_New_Text.SetPartner(&m_SSRE_Original_Text);
  m_SSRE_New_Text.SetWindowText(m_sxNew_Text.c_str());  
  m_SSRE_New_Text.SetTargetDevice(NULL, 0);
  m_SSRE_New_Text.SetSel(0, 0);

  GotoDlgCtrl(GetDlgItem(IDOK));

  return FALSE;  // return TRUE unless you set the focus to a control
}
