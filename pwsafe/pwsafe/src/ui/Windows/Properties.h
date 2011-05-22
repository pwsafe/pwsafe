/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// CProperties dialog - this is what's displayed when user selects File->Properties

#include "PWDialog.h"
#include "core/PWScore.h"

class CProperties : public CPWDialog
{
  DECLARE_DYNAMIC(CProperties)

public:
  CProperties(const st_DBProperties &st_dbp, CWnd* pParent = NULL)
    : CPWDialog(CProperties::IDD, pParent), m_dbp(st_dbp) {}

  virtual BOOL OnInitDialog();

  // Dialog Data
  enum { IDD = IDD_PROPERTIES };

protected:
DECLARE_MESSAGE_MAP()

private:
  const st_DBProperties m_dbp;
};
