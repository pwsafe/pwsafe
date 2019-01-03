/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// SetFiltersDlg.cpp : implementation file
//

#include "../stdafx.h"

#include "PWFiltersDlg.h"
#include "SetFiltersDlg.h"
#include "SetHistoryFiltersDlg.h"
#include "SetPolicyFiltersDlg.h"
#include "SetAttachmentFiltersDlg.h"

#include "../GeneralMsgBox.h"
#include "../resource3.h"

// CSetFiltersDlg dialog

IMPLEMENT_DYNAMIC(CSetFiltersDlg, CPWFiltersDlg)

CSetFiltersDlg::CSetFiltersDlg(CWnd* pParent,
                               st_filters *pfilters,
                               const int &iWMSGID,
                               const bool bCanHaveAttachments,
                               const std::set<StringX> *psMediaTypes,
                               const bool bAllowSet)
 : CPWFiltersDlg(pParent, DFTYPE_MAIN, pfilters->fname.c_str(), bCanHaveAttachments, psMediaTypes),
  m_iWMSGID(iWMSGID)
{
  ASSERT(pParent != NULL);
  ASSERT(pfilters != NULL);
  ASSERT(iWMSGID != 0);

  m_pParent = pParent;
  m_pfilters = pfilters;
  m_bAllowSet = bAllowSet;

  m_cstitle.LoadString(IDS_SETDISPLAYFILTERS);
}

CSetFiltersDlg::~CSetFiltersDlg()
{
}

BEGIN_MESSAGE_MAP(CSetFiltersDlg, CPWFiltersDlg)
  ON_BN_CLICKED(IDC_APPLY, OnApply)
END_MESSAGE_MAP()

void CSetFiltersDlg::EnableDisableApply()
{
  GetDlgItem(IDC_APPLY)->EnableWindow(m_pfilters->num_Mactive == 0 ? FALSE : TRUE);
}

void CSetFiltersDlg::OnApply()
{
  if (UpdateData(TRUE) == FALSE)
    return;

  if (m_filtername.IsEmpty()) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(IDS_FILTERNAMEEMPTY);
    return;
  }

  // Send message to parent to update the display by applying the filters
  // as they are today (if OK of course)
  if (!CPWFiltersDlg::VerifyFilters())
    return;

  m_pfilters->fname = m_filtername;

  // Tell parent (DboxMain) to execute the current filters
  m_pParent->SendMessage(m_iWMSGID, (WPARAM)m_pfilters);
}
