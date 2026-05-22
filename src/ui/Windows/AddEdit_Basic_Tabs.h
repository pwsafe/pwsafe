/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "AddEdit_Basic_CustomFieldsPage.h"
#include "AddEdit_Basic_NotesPage.h"

class CAddEdit_Basic_Tabs : public CPropertySheet
{
public:
  CAddEdit_Basic_Tabs(CWnd *pPropertySheetParent, st_AE_master_data *pAEMD);

  void CancelThreadWait();
  BOOL Create(CWnd *pParentWnd, const CRect &rect);
  bool IsExternalEditorActive() const;

protected:
  virtual BOOL OnInitDialog();

  afx_msg void OnSize(UINT nType, int cx, int cy);

  DECLARE_MESSAGE_MAP()

private:
  void LayoutPages();

  CAddEdit_Basic_NotesPage m_pp_notes;
  CAddEdit_Basic_CustomFieldsPage m_pp_customFields;
};
