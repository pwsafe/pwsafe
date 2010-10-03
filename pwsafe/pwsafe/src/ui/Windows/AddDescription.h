/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file AddDescription.h
//-----------------------------------------------------------------------------

#pragma once

#include "PWDialog.h"
#include "resource.h"

// CAddDescription dialog

class CAddDescription : public CPWDialog
{
  DECLARE_DYNAMIC(CAddDescription)

public:
  CAddDescription(CWnd* pParent, const CString filename, 
    const CString description = L"");   // standard constructor

// Dialog Data
  enum { IDD = IDD_ADD_DESCRIPTION };
  CString GetDescription() {return m_description;}

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  DECLARE_MESSAGE_MAP()

private:
  CString m_filename, m_description;
};
