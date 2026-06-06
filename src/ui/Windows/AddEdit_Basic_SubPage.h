/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "PWPropertyPage.h"
#include "AddEdit_PropertyPage.h"

class CAddEdit_PropertySheet;

class CAddEdit_Basic_SubPage : public CPWPropertyPage
{
public:
  CAddEdit_Basic_SubPage(CWnd *pParent, UINT nID, UINT nID_Short,
                         st_AE_master_data *pAEMD);
  virtual ~CAddEdit_Basic_SubPage() {}

  DECLARE_DYNAMIC(CAddEdit_Basic_SubPage)

protected:
  UINT &M_uicaller() { return m_AEMD.uicaller; }
  PWScore *&M_pcore() { return m_AEMD.pcore; }
  CSecString &M_notes() { return m_AEMD.notes; }
  CustomFieldList &M_customfields() { return m_AEMD.customfields; }
  unsigned char &M_protected() { return m_AEMD.ucprotected; }

  st_AE_master_data &m_AEMD;
  CAddEdit_PropertySheet *m_ae_psh;
};
