/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// SaveFilter dialog

#include "../PWDialog.h"

class CSaveFilterDlg : public CPWDialog
{
  DECLARE_DYNAMIC(CSaveFilterDlg)

public:
  CSaveFilterDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CSaveFilterDlg();

  int GetSelectStore() {return m_selectedstore;}

// Dialog Data
  enum { IDD = IDD_SAVEFILTER };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  BOOL OnInitDialog();

  afx_msg void OnSave();

  DECLARE_MESSAGE_MAP()

private:
  int m_selectedstore;
};
