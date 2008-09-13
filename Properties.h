/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// CProperties dialog

#include "PWDialog.h"
#include "corelib/PWScore.h"

class CProperties : public CPWDialog
{
  DECLARE_DYNAMIC(CProperties)

public:
  CProperties(const PWScore &core,
              CWnd* pParent = NULL);
  virtual ~CProperties();

  // Dialog Data
  enum { IDD = IDD_PROPERTIES };
 private:
  CString m_database;
  CString m_databaseformat;
  CString m_numgroups;
  CString m_numentries;
  CString m_whenlastsaved;
  CString m_wholastsaved;
  CString m_whatlastsaved;
  CString m_file_uuid;
  CString m_unknownfields;
  CString m_whenmpwset;
  CString m_mpwexp;
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnOK();
  virtual BOOL OnInitDialog();
};
