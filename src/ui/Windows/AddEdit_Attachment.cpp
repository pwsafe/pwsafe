/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AddEdit_Attachment.cpp : implementation file
//

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"    // For Help
#include "DboxMain.h"

#include "AddEdit_Attachment.h"
#include "AddEdit_PropertySheet.h"

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_DateTimes property page

IMPLEMENT_DYNAMIC(CAddEdit_Attachment, CAddEdit_PropertyPage)

CAddEdit_Attachment::CAddEdit_Attachment(CWnd *pParent, st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent, 
                          CAddEdit_Attachment::IDD, CAddEdit_Attachment::IDD_SHORT,
                          pAEMD),
  m_bInitdone(false), m_AttName(_T("")), m_AttFile(_T(""))
{
}

CAddEdit_Attachment::~CAddEdit_Attachment()
{
}

void CAddEdit_Attachment::DoDataExchange(CDataExchange* pDX)
{
    CAddEdit_PropertyPage::DoDataExchange(pDX);

    //{{AFX_DATA_MAP(CAddEdit_Attachment)
    //}}AFX_DATA_MAP
    DDX_Text(pDX, IDC_ATT_NAME, m_AttName);
    DDX_Text(pDX, IDC_ATT_FILE, m_AttFile);
    if (pDX->m_bSaveAndValidate == 0)
      DDX_Control(pDX, IDC_ATT_IMAGE, m_AttStatic);
}

BEGIN_MESSAGE_MAP(CAddEdit_Attachment, CAddEdit_PropertyPage)
  //{{AFX_MSG_MAP(CAddEdit_Attachment)
  ON_BN_CLICKED(ID_HELP, OnHelp)

  // Common
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
  ON_WM_PAINT()
  ON_BN_CLICKED(IDC_ATT_IMPORT, &CAddEdit_Attachment::OnBnClickedAttImport)
  ON_BN_CLICKED(IDC_ATT_EXPORT, &CAddEdit_Attachment::OnBnClickedAttExport)
  ON_BN_CLICKED(IDC_ATT_REMOVE, &CAddEdit_Attachment::OnBnClickedAttRemove)
END_MESSAGE_MAP()

BOOL CAddEdit_Attachment::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
    PostMessage(WM_COMMAND, MAKELONG(ID_HELP, BN_CLICKED), NULL);
    return TRUE;
  }

  return CAddEdit_PropertyPage::PreTranslateMessage(pMsg);
}

BOOL CAddEdit_Attachment::OnInitDialog()
{
  // TBD  load from attachment record
  UpdateControls();
  return TRUE;
}

BOOL CAddEdit_Attachment::OnKillActive()
{
  if (UpdateData(TRUE) == FALSE)
    return FALSE;

  return CAddEdit_PropertyPage::OnKillActive();
}


LRESULT CAddEdit_Attachment::OnQuerySiblings(WPARAM , LPARAM )
{
  UpdateData(TRUE);

  // Have any of my fields been changed?
  return 0L;
}

BOOL CAddEdit_Attachment::OnApply()
{
  if (M_uicaller() == IDS_VIEWENTRY || M_protected() != 0)
    return FALSE;

  return CAddEdit_PropertyPage::OnApply();
}

void CAddEdit_Attachment::OnHelp()
{
  ShowHelp(L"::/html/attachments.html");
}

void CAddEdit_Attachment::OnPaint()
{
  CAddEdit_PropertyPage::OnPaint();
  if (!m_AttImage.IsNull()) {
    CRect rect;
    m_AttStatic.GetClientRect(rect);
    m_AttImage.StretchBlt(m_AttStatic.GetDC()->GetSafeHdc(), 0, 0,
                          rect.Width(), rect.Height(), SRCCOPY);
  }
}

void CAddEdit_Attachment::OnBnClickedAttImport()
{
  CString filter;
	CSimpleArray<GUID> aguidFileTypes;
	HRESULT hResult;

	hResult = m_AttImage.GetImporterFilterString(filter,aguidFileTypes);
	if (FAILED(hResult))
		return;

  CFileDialog fileDlg(TRUE, NULL, NULL, 0, filter, this);
  if (fileDlg.DoModal() == IDOK) {
    m_AttFile = fileDlg.GetPathName();
    hResult = m_AttImage.Load(m_AttFile);
    if (FAILED(hResult)) {
      const CString errmess(L"Failed to load image");
      ::AfxMessageBox(errmess);
      return;
    }

    CItemAtt &att = M_attachment();
    int status = att.Import(LPCWSTR(m_AttFile));
    ASSERT(status == PWScore::SUCCESS); // CImage loaded it, how can we fail??
    if (!att.HasUUID())
      att.CreateUUID();

    m_ae_psh->SetChanged(true);
    Invalidate();
    UpdateControls();
    UpdateData(FALSE);
    UpdateWindow();
  }
}

void CAddEdit_Attachment::OnBnClickedAttExport()
{
  CString filter;
	CSimpleArray<GUID> aguidFileTypes;
	HRESULT hResult;

  if (m_AttImage.IsNull())
    return;

	hResult = m_AttImage.GetExporterFilterString(filter,aguidFileTypes);
	if (FAILED(hResult))
		return;

  CFileDialog fileDlg(FALSE, NULL, NULL, 0, filter, this);
  if (fileDlg.DoModal() == IDOK) {
    const CString sfile = fileDlg.GetPathName();
    hResult = m_AttImage.Save(sfile);
    if (FAILED(hResult)) {
      const CString errmess(L"Failed to save image");
      ::AfxMessageBox(errmess);
      return;
    }
  }
}

void CAddEdit_Attachment::OnBnClickedAttRemove()
{
  if (!m_AttImage.IsNull()) {
    CRect rect;
    m_AttStatic.GetClientRect(rect);
    m_AttImage.StretchBlt(m_AttStatic.GetDC()->GetSafeHdc(), 0, 0,
                          rect.Width(), rect.Height(), SRCERASE);
    m_AttImage.Destroy();
  }
  m_AttFile = m_AttName = L"";
  M_attachment().Clear();
  m_ae_psh->SetChanged(true);
  UpdateControls();
  UpdateData(FALSE);
  UpdateWindow();
}

 void CAddEdit_Attachment::UpdateControls()
 {
   bool hasImage = !m_AttImage.IsNull();
   GetDlgItem(IDC_ATT_IMPORT)->EnableWindow(!hasImage);
   GetDlgItem(IDC_ATT_EXPORT)->EnableWindow(hasImage);
   GetDlgItem(IDC_ATT_REMOVE)->EnableWindow(hasImage);
 }
