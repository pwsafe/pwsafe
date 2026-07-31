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

  // Public so CAddEdit_Basic can funnel Alt+<letter> here directly when one of
  // its own fields (rather than a subtab) has focus; see CAddEdit_Basic_SubPage
  // for the subtab side of the same delegation. Matches WM_SYSCHAR against each
  // subtab's own caption mnemonic (read directly off the tab control, so this
  // works for any I18N translation), switches to the matching subtab, and lets
  // it focus whatever control makes sense for it.
  virtual BOOL PreTranslateMessage(MSG *pMsg);

protected:
  virtual BOOL OnInitDialog();

  afx_msg void OnSize(UINT nType, int cx, int cy);

  DECLARE_MESSAGE_MAP()

private:
  void LayoutPages();

  // Switch to pPage (if not already active) and let it focus its own default
  // control; the single home for every subtab's mnemonic accelerator.
  void ActivatePage(CAddEdit_Basic_SubPage *pPage);

  CAddEdit_Basic_NotesPage m_pp_notes;
  CAddEdit_Basic_CustomFieldsPage m_pp_customFields;
};
