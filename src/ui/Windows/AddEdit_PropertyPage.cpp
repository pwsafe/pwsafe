/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "passwordsafe.h"
#include "GeneralMsgBox.h"
#include "DboxMain.h"
#include "AddEdit_PropertyPage.h"

using pws_os::CUUID;

IMPLEMENT_DYNAMIC(CAddEdit_PropertyPage, CPWPropertyPage)

COLORREF CAddEdit_PropertyPage::crefGreen = (RGB(222, 255, 222));
COLORREF CAddEdit_PropertyPage::crefWhite = (RGB(255, 255, 255));

CAddEdit_PropertyPage::CAddEdit_PropertyPage(CWnd *pParent,
                                             UINT nID,
                                             st_AE_master_data *pAEMD)
  : CPWPropertyPage(nID), m_AEMD(*pAEMD),
    m_ae_psh((CAddEdit_PropertySheet *)pParent)
{
}

CAddEdit_PropertyPage::CAddEdit_PropertyPage(CWnd *pParent,
                                             UINT nID, UINT nID_Short,
                                             st_AE_master_data *pAEMD)
  : CPWPropertyPage(pAEMD->bLongPPs ? nID : nID_Short), m_AEMD(*pAEMD),
    m_ae_psh((CAddEdit_PropertySheet *)pParent)
{
}

BOOL CAddEdit_PropertyPage::OnQueryCancel()
{
  // Check whether there have been any changes in order to ask the user
  // if they really want to cancel
  // QuerySiblings is only sent to loaded PropertyPages (i.e., user has
  // selected to view them as ones not yet loaded cannot have changed fields)
  if (QuerySiblings(PP_DATA_CHANGED, 0L) != 0L) {
    CGeneralMsgBox gmb;
    std::vector<std::tuple<int, int>> tuples = {
      std::make_tuple(IDYES, IDS_DISCARD),
      std::make_tuple(IDCANCEL, IDS_CANCEL)
    };
    if (gmb.AfxMessageBox(IDS_AREYOUSURE, nullptr, tuples, 1, MB_ICONEXCLAMATION) == IDCANCEL)
      return FALSE;
  }
  return CPWPropertyPage::OnQueryCancel();
}

BOOL CAddEdit_PropertyPage::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  ASSERT(pResult != NULL);
  NMHDR* pNMHDR = (NMHDR*)lParam;

  if (pNMHDR->hwndFrom != m_hWnd && pNMHDR->hwndFrom != ::GetParent(m_hWnd))
    return FALSE;

  if (pNMHDR->code == PSN_QUERYINITIALFOCUS) {
    CWnd* pCtl = GetDlgItem(IDC_TITLE);
    if (pCtl) {
      *pResult = (LRESULT)pCtl->m_hWnd;
      return TRUE;
    }
  }
  return CPropertyPage::OnNotify(wParam, lParam, pResult);
}

CItemData* CAddEdit_PropertyPage::M_pci_credential()
{
  if (!M_pci())
    return nullptr;
  if (!M_pci()->IsAlias())
    return M_pci();
  return M_pcore()->GetBaseEntry(M_pci());
}
