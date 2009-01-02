/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// FilterActionsDlg dialog

#include "../PWDialog.h"

#include <vector>

enum {FA_DELETE = 0, FA_SELECT};

class CFilterActionsDlg : public CPWDialog
{
  DECLARE_DYNAMIC(CFilterActionsDlg)

public:
  CFilterActionsDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CFilterActionsDlg();

  void SetFunction(int function) {m_function = function;}
  void SetList(const std::vector<CString> &vcs_db)
  {m_vcs_db = vcs_db;}
  const CString &GetSelected() const
  {return m_selected;}

// Dialog Data
  enum { IDD = IDD_FILTERACTIONS };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  BOOL OnInitDialog();
  afx_msg void OnExecute();

  DECLARE_MESSAGE_MAP()

private:
  std::vector<CString> m_vcs_db;
  CComboBox m_combo;
  CString m_selected;
  int m_function;
};
