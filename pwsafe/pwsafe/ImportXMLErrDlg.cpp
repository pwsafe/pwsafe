/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// ImportXMLErrDlg.cpp : implementation file
//

#include "ImportXMLErrDlg.h"
#include "corelib/PWSclipboard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportXMLErrDlg dialog

CImportXMLErrDlg::CImportXMLErrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImportXMLErrDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportXMLErrDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
}

void CImportXMLErrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportXMLErrDlg)
	DDX_Text(pDX, IDC_XML_IMPORT_ACTION, m_strActionText);
	DDX_Text(pDX, IDC_XML_IMPORT_RESULTS, m_strResultText);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CImportXMLErrDlg, CDialog)
	//{{AFX_MSG_MAP(CImportXMLErrDlg)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDC_COPY_ERRORS_TO_CLIPBOARD, OnBnClickedCopyToClipboard)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportXMLErrDlg message handlers

//	---------------------------------------------------------------------------
void CImportXMLErrDlg::OnOK()
{
	CDialog::OnOK();
}

void
CImportXMLErrDlg::OnBnClickedCopyToClipboard()
{
  PWSclipboard cb;
	cb.SetData(m_strResultText, false);
}
