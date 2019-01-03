/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

/** \file
 * This is the base class for Filter-related dialog boxes
 */

#include "../PWDialog.h" // base class
#include "core/Match.h" // for PWSMatch::MatchRule

class CFilterBaseDlg : public CPWDialog
{
protected:
  CFilterBaseDlg(UINT nIDTemplate, CWnd* pParentWnd = NULL);
  virtual BOOL OnInitDialog();

public:
  PWSMatch::MatchRule m_rule;
  PWSMatch::MatchType m_type;
  CString m_title;
  CString m_oldtitle;
  bool m_bFirst;

  DECLARE_DYNAMIC(CFilterBaseDlg)

protected:
  int m_rule2selection[PWSMatch::MR_LAST];
};
