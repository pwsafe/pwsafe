/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "DboxMain.h"
#include "AddEdit_PropertyPage.h"

IMPLEMENT_DYNAMIC(CAddEdit_PropertyPage, CPWPropertyPage)

COLORREF CAddEdit_PropertyPage::crefGreen = (RGB(222, 255, 222));
COLORREF CAddEdit_PropertyPage::crefWhite = (RGB(255, 255, 255));

CAddEdit_PropertyPage::CAddEdit_PropertyPage(CWnd *pParent, UINT nID,
                                             st_AE_master_data *pAEMD)
 : CPWPropertyPage(nID), m_AEMD(*pAEMD)
{
  // Save pointer to my PropertySheet
  m_ae_psh = (CAddEdit_PropertySheet *)pParent;
}

BOOL CAddEdit_PropertyPage::OnQueryCancel()
{
  // Check whether there have been any changes in order to ask the user
  // if they really want to cancel
  // QuerySiblings is only sent to loaded PropertyPages (i.e. user has
  // selected to view them as ones not yet loaded cannot have changed fields)
  if (QuerySiblings(PP_DATA_CHANGED, 0L) != 0L) {
    if (AfxMessageBox(IDS_AREYOUSURE,
                      MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDNO)
      return FALSE;
  }
  return CPWPropertyPage::OnQueryCancel();
}
