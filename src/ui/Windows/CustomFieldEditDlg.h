/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

#include "core/CustomFields.h"
#include "resource.h"
#include "PWDialog.h"


class CCustomFieldEditDlg : public CPWDialog
{
public:
  CCustomFieldEditDlg(CWnd* pParent, const CustomFieldList& fields); // For adding
  CCustomFieldEditDlg(CWnd* pParent, const CustomFieldList& fields, const CustomField& cf); // For editing


  enum { IDD = IDD_CUSTOMFIELD_EDIT };

  CString m_name;
  CString m_value;
  BOOL m_sensitive;

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();
  virtual void OnOK();

  DECLARE_MESSAGE_MAP()

private:
  CustomFieldList m_fields;
  const CString m_orig_name; // for editing, to skip uniqueness check
};
