/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "FilterBaseDlg.h"

IMPLEMENT_DYNAMIC(CFilterBaseDlg, CPWDialog)

CFilterBaseDlg::CFilterBaseDlg(UINT nIDTemplate, CWnd* pParentWnd)
: CPWDialog(nIDTemplate, pParentWnd),
  m_bFirst(true), m_rule(PWSMatch::MR_INVALID)
{
  for (int i = (int)PWSMatch::MR_INVALID; i < (int)PWSMatch::MR_LAST; i++) {
    m_rule2selection[i] = -1;
  }
}

BOOL CFilterBaseDlg::OnInitDialog()
{
  BOOL retval = CPWDialog::OnInitDialog();

  if (retval == TRUE) {
    if (m_bFirst) {
      GetWindowText(m_oldtitle);
      m_bFirst = false;
    }
    SetWindowText(m_oldtitle + m_title);
  }
  return retval;
}
