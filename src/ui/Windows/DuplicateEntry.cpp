/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// DuplicateEntry.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "DuplicateEntry.h"
#include "fonts.h"

CDuplicateEntry::CDuplicateEntry(CWnd* pParent,
  CString csCaption,  CString csMessage,
  const StringX sxGroup, const StringX sxTitle,
  const StringX sxUser)
  :CPWDialog(CDuplicateEntry::IDD, pParent),
  m_csCaption(csCaption), m_csMessage(csMessage),
  m_sxGroup(sxGroup), m_sxTitle(sxTitle), m_sxUser(sxUser)
{
  ASSERT(csCaption.GetLength() > 0);
  ASSERT(csMessage.GetLength() > 0);
}

BEGIN_MESSAGE_MAP(CDuplicateEntry, CPWDialog)
END_MESSAGE_MAP()

BOOL CDuplicateEntry::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  SetWindowText(m_csCaption);
  GetDlgItem(IDC_STATIC_MESSAGE)->SetWindowText(m_csMessage);

  StringX sxEntry;
  sxEntry = L"\xab" + m_sxGroup + L"\xbb " +
            L"\xab" + m_sxTitle + L"\xbb " +
            L"\xab" + m_sxUser  + L"\xbb";
  GetDlgItem(IDC_ENTRY)->SetWindowText(sxEntry.c_str());

  // Get Add/Edit font
  CFont *pFont = Fonts::GetInstance()->GetAddEditFont();
  GetDlgItem(IDC_ENTRY)->SetFont(pFont);

  return TRUE;
}
