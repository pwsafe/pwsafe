/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "passwordsafe.h"
#include "GeneralMsgBox.h"
#include "DboxMain.h"
#include "Options_PropertyPage.h"
#include "Options_PropertySheet.h"

IMPLEMENT_DYNAMIC(COptions_PropertyPage, CPWPropertyPage)

COptions_PropertyPage::COptions_PropertyPage(CWnd *pParent,
                                             UINT nID,
                                             st_Opt_master_data *pOPTMD)
: CPWPropertyPage(nID),
  m_options_psh((COptions_PropertySheet *)pParent), m_OPTMD(*pOPTMD)
{
}

COptions_PropertyPage::COptions_PropertyPage(CWnd *pParent,
                                             UINT nID, UINT nID_Short,
                                             st_Opt_master_data *pOPTMD)
  : CPWPropertyPage(pOPTMD->bLongPPs ? nID : nID_Short),
    m_options_psh((COptions_PropertySheet *)pParent), m_OPTMD(*pOPTMD)
{
}

BOOL COptions_PropertyPage::OnQueryCancel()
{
  // Check whether there have been any changes in order to ask the user
  // if they really want to cancel
  // QuerySiblings is only sent to loaded PropertyPages (i.e. user has
  // selected to view them as ones not yet loaded cannot have changed fields)
  if (QuerySiblings(PP_DATA_CHANGED, 0L) != 0L) {
    CGeneralMsgBox gmb;
    if (gmb.AfxMessageBox(IDS_AREYOUSURE_OPT,
                          MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDNO)
      return FALSE;
  }
  return CPWPropertyPage::OnQueryCancel();
}
