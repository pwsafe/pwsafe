/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
  CProperties(st_DBProperties *pdbp, const bool bReadonly, CWnd *pParent = NULL);
  virtual ~CProperties();

  bool HasDataChanged() {return m_bChanged;}

  // Dialog Data
  enum { IDD = IDD_PROPERTIES };

protected:
  CStatic m_stc_name;
  CStatic m_stc_description;

  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  afx_msg void OnEditName();
  afx_msg void OnEditDescription();
  
  DECLARE_MESSAGE_MAP()

private:
  void SetChangedStatus();

  st_DBProperties *m_pdbp;
  StringX m_old_name, m_old_description;
  bool m_bReadOnly, m_bChanged;
};
