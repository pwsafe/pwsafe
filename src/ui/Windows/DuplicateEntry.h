/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// DuplicateEntry.h

#include "PWDialog.h"

#include "core/StringX.h"

class CDuplicateEntry : public CPWDialog
{
public:
  CDuplicateEntry(CWnd* pParent = NULL, CString csCaption = L"",
      CString csMessage = L"",
      const StringX sxGroup = L"", const StringX sxTitle = L"",
      const StringX sxUser = L"");

private:
  // Dialog Data
  //{{AFX_DATA(CConfirmDeleteDlg)
  enum { IDD = IDD_DUPLICATE_ENTRY_DIALOG};
  //}}AFX_DATA

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CConfirmDeleteDlg)
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CConfirmDeleteDlg)
  virtual BOOL OnInitDialog();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  StringX m_sxGroup, m_sxTitle, m_sxUser;
  CString m_csCaption, m_csMessage;
};
