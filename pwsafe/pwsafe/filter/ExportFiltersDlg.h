/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// ExportFilters dialog

#include "../PWDialog.h"

class CExportFiltersDlg : public CPWDialog
{
  DECLARE_DYNAMIC(CExportFiltersDlg)

public:
  CExportFiltersDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CExportFiltersDlg();

  void SetAvailableStores(bool bDB, bool bGlobal)
  {m_bDB = bDB; m_bGlobal = bGlobal;}
  int GetSelected() {return m_selectedstore;}

// Dialog Data
  enum { IDD = IDD_EXPORTFILTERS };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  BOOL OnInitDialog();

  afx_msg void OnExport();

  DECLARE_MESSAGE_MAP()

private:
  bool m_bDB, m_bGlobal;
  int m_selectedstore;
};
