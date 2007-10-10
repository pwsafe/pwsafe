/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#pragma once

// ExportXML.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportXML dialog

#include "PWDialog.h"
#include "corelib/ItemData.h"

class CExportXMLDlg : public CPWDialog
{
// Construction
public:
	CExportXMLDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportXMLDlg)
	enum { IDD = IDD_EXPORT_XML };
	CString	m_ExportXMLPassword;
	CString m_defexpdelim;
	//}}AFX_DATA

	CItemData::FieldBits m_bsExport;
	CString m_subgroup_name;
	int m_subgroup_set, m_subgroup_object, m_subgroup_function;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportXMLDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual BOOL OnInitDialog();
	// Generated message map functions
	//{{AFX_MSG(CExportXMLDlg)
	afx_msg void OnAdvanced();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void AFXAPI DDV_CheckExpDelimiter(CDataExchange* pDX,
                                    const CString &delimiter);
};
