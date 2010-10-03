/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// CAttProperties dialog

#include "PWDialog.h"

// Input parameter list for dialog
struct st_AttProp {
  CString name;
  CString path;
  CString desc;
  CString cdate;
  CString adate;
  CString mdate;
  CString ddate;
  CString usize;
  CString comp;
  CString crc;
  CString odigest;
};

class CAttProperties : public CPWDialog
{
  DECLARE_DYNAMIC(CAttProperties)

public:
  CAttProperties(const st_AttProp &st_prop, CWnd *pParent = NULL);
  virtual ~CAttProperties();

  afx_msg void OnOK();
  virtual BOOL OnInitDialog();

  // Dialog Data
  enum { IDD = IDD_ATT_PROPERTIES };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  DECLARE_MESSAGE_MAP()

private:
  CString m_name;
  CString m_path;
  CString m_desc;
  CString m_cdate;
  CString m_adate;
  CString m_mdate;
  CString m_ddate;
  CString m_usize;
  CString m_comp;
  CString m_crc;
  CString m_odigest;

  CFont *m_pcfont;
};
