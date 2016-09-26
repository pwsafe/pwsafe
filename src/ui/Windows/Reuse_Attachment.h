/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// Reuse_Attachment.h
//-----------------------------------------------------------------------------

#pragma once

#include "PWDialog.h"

#include "core/StringX.h"

// CReuse_Attachment dialog

class CReuse_Attachment : public CPWDialog
{
	DECLARE_DYNAMIC(CReuse_Attachment)

public:
	CReuse_Attachment(CWnd* pParent = NULL);   // standard constructor
	virtual ~CReuse_Attachment();

  BOOL OnInitDialog();

  void SetFileDetails(StringX sxImportFN, StringX sxImportFP,
    StringX sxExistingFN, StringX sxExistingFP, StringX sxExistingFT)
  { m_sxImportFN = sxImportFN; m_sxImportFP = sxImportFP;
    m_sxExistingFN = sxExistingFN; m_sxExistingFP = sxExistingFP;
    m_sxExistingFT = sxExistingFN;}

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REUSEATTACHMENT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
  StringX m_sxImportFN, m_sxImportFP,m_sxExistingFN, m_sxExistingFP, m_sxExistingFT;
};
