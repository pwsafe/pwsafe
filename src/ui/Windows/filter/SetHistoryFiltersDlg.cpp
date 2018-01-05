/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// SetHistoryFiltersDlg.cpp : implementation file
//

#include "../stdafx.h"

#include "PWFiltersDlg.h"
#include "SetHistoryFiltersDlg.h"

#include "../resource3.h"

// CSetHistoryFiltersDlg dialog

IMPLEMENT_DYNAMIC(CSetHistoryFiltersDlg, CPWFiltersDlg)

CSetHistoryFiltersDlg::CSetHistoryFiltersDlg(CWnd* pParent, st_filters *pfilters,
                                             CString filtername)
  : CPWFiltersDlg(pParent, DFTYPE_PWHISTORY, filtername)
{
  ASSERT(pParent != NULL);
  ASSERT(pfilters != NULL);

  m_bAllowSet = false;
  m_pParent = pParent;
  m_pfilters = pfilters;

  m_cstitle.LoadString(IDS_SETPWHISTFILTERS);
}

CSetHistoryFiltersDlg::~CSetHistoryFiltersDlg()
{
}

BEGIN_MESSAGE_MAP(CSetHistoryFiltersDlg, CPWFiltersDlg)
END_MESSAGE_MAP()
