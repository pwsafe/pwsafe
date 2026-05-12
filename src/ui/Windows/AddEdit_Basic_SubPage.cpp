/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "StdAfx.h"
#include "AddEdit_Basic_SubPage.h"

IMPLEMENT_DYNAMIC(CAddEdit_Basic_SubPage, CPWPropertyPage)

CAddEdit_Basic_SubPage::CAddEdit_Basic_SubPage(CWnd *pParent, UINT nID,
                                               UINT nID_Short,
                                               st_AE_master_data *pAEMD)
  : CPWPropertyPage(pAEMD->bLongPPs ? nID : nID_Short),
    m_AEMD(*pAEMD),
    m_ae_psh((CAddEdit_PropertySheet *)pParent)
{
}
