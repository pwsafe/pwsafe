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

#include "GeneralMsgBox.h"

#include "os/file.h"

#include <Shellapi.h>
#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_DateTimes property page

IMPLEMENT_DYNAMIC(CAddEdit_Attachment, CAddEdit_PropertyPage)

CAddEdit_Attachment::CAddEdit_Attachment(CWnd *pParent, st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent, 
                          CAddEdit_Attachment::IDD, CAddEdit_Attachment::IDD_SHORT,
                          pAEMD),
   m_bInitdone(false), m_AttName(_T("")), m_AttFile(_T("")), m_attType(NO_ATTACHMENT)
{
}

CAddEdit_Attachment::~CAddEdit_Attachment()
{
}

void CAddEdit_Attachment::DoDataExchange(CDataExchange* pDX)
{
    CAddEdit_PropertyPage::DoDataExchange(pDX);

    //{{AFX_DATA_MAP(CAddEdit_Attachment)
    DDX_Text(pDX, IDC_ATT_NAME, m_AttName);
    DDX_Text(pDX, IDC_ATT_FILE, m_AttFile);
    DDX_Control(pDX, IDC_STATIC_NOPREVIEW, m_stcNoPreview);

    if (pDX->m_bSaveAndValidate == 0)
      DDX_Control(pDX, IDC_ATT_IMAGE, m_AttStatic);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddEdit_Attachment, CAddEdit_PropertyPage)
  //{{AFX_MSG_MAP(CAddEdit_Attachment)
  ON_WM_PAINT()

  // Common
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  ON_BN_CLICKED(ID_HELP, OnHelp)

  // Our message handlers
  ON_BN_CLICKED(IDC_ATT_IMPORT, OnAttImport)
  ON_BN_CLICKED(IDC_ATT_EXPORT, OnAttExport)
  ON_BN_CLICKED(IDC_ATT_REMOVE, OnAttRemove)

  // For dropped files
  ON_MESSAGE(PWS_MSG_DROPPED_FILE, OnDroppedFile)
  //}}AFX_MSG_MAP
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
  CAddEdit_PropertyPage::OnInitDialog();

  // TBD  load from attachment record

  // Keep initial size and position of static image control
  m_AttStatic.GetClientRect(m_initial_clientrect);
  m_AttStatic.GetWindowRect(m_initial_windowrect);
  ScreenToClient(&m_initial_windowrect);

  m_xoffset = m_initial_windowrect.left - m_initial_clientrect.left;
  m_yoffset = m_initial_windowrect.top - m_initial_clientrect.top;

  // Get Image filters
  CSimpleArray<GUID> aguidFileTypes;
  HRESULT hResult;

  const CString cs_allimages(MAKEINTRESOURCE(IDS_ALL_IMAGE_FILES));
  const DWORD dwExclude = CImage::excludeOther;
  hResult = m_AttImage.GetImporterFilterString(m_csImageFilter, aguidFileTypes, cs_allimages, dwExclude);
  ASSERT(hResult >= 0);

  std::wstring wsfilter = m_csImageFilter;

  std::wstring delimiters = L"*|;()";
  
  // Remove cs_allimages from front of string
  wsfilter.erase(0, cs_allimages.GetLength());
  // Skip delimiters at beginning.
  std::wstring::size_type lastPos = wsfilter.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  std::wstring::size_type pos = wsfilter.find_first_of(delimiters, lastPos);

  while (std::wstring::npos != pos || std::wstring::npos != lastPos) {
    // Found a token, add it to the vector ONLY if not empty, not already there and
    // beginning with a '.' (as per the extension retrieved by splitpath)
    std::wstring token = wsfilter.substr(lastPos, pos - lastPos);
    if (!token.empty() && token.substr(0, 1) == L"." && 
      std::find(m_image_extns.begin(), m_image_extns.end(), token) == m_image_extns.end()) {
      m_image_extns.push_back(token);
    }

    // Skip delimiters
    lastPos = wsfilter.find_first_not_of(delimiters, pos);

    // Find next "non-delimiter"
    pos = wsfilter.find_first_of(delimiters, lastPos);
  }

  m_stcNoPreview.ShowWindow(SW_HIDE);

  UpdateControls();
  return TRUE;
}

BOOL CAddEdit_Attachment::OnKillActive()
{
  if (UpdateData(TRUE) == FALSE)
    return FALSE;

  return CAddEdit_PropertyPage::OnKillActive();
}


LRESULT CAddEdit_Attachment::OnQuerySiblings(WPARAM wParam, LPARAM)
{
  UpdateData(TRUE);

  switch (wParam) {
    case PP_DATA_CHANGED:
      if (M_attachment() != M_oldattachment())
        return 1L;
      break;
    case PP_UPDATE_VARIABLES:
    case PP_UPDATE_TIMES:
      // Since OnOK calls OnApply after we need to verify and/or
      // copy data into the entry - we do it ourselfs here first
      if (OnApply() == FALSE)
        return 1L;
      break;
  }
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
    m_AttImage.StretchBlt(m_AttStatic.GetDC()->GetSafeHdc(), 0, 0,
      m_clientrect.Width(), m_clientrect.Height(), SRCCOPY);
  }
}

LRESULT CAddEdit_Attachment::OnDroppedFile(WPARAM wParam, LPARAM lParam)
{
  // Currently only support one attachment per entry
  ASSERT(lParam == 1);
  wchar_t *sxFileName = reinterpret_cast<LPWSTR>(wParam);
  m_AttFile = sxFileName;

  // Update dialog with filename so that Import uses it
  UpdateData(FALSE);

  // Import file
  OnAttImport();
  return 0;
}

void CAddEdit_Attachment::OnAttImport()
{
  CString filter;
  HRESULT hResult;

  UpdateData(TRUE);

  if (m_AttFile.IsEmpty()) {
    // Ask user for file name
    // Remove last separator
    filter = m_csImageFilter.Left(m_csImageFilter.GetLength() - 1);

    // Add "All files"
    const CString cs_allfiles(MAKEINTRESOURCE(IDS_FDF_ALL));
    filter.Append(cs_allfiles);

    CFileDialog fileDlg(TRUE, NULL, NULL, 0, filter, this);
    if (fileDlg.DoModal() == IDCANCEL)
      return;

    m_AttFile = fileDlg.GetPathName();
  } else {
    if (!pws_os::FileExists(std::wstring(m_AttFile))) {
      CGeneralMsgBox gmb;
      gmb.AfxMessageBox(IDS_ATTACHMENT_NOTFOUND);
      return;
    }
  }

  hResult = m_AttImage.Load(m_AttFile);
  if (FAILED(hResult)) {
    // Probably not an image!  But how do we make sure???
    // Let's check if it is one of the image extensions we know about
    wchar_t ext[_MAX_EXT];
    _wsplitpath_s(m_AttFile, NULL, 0, NULL, 0, NULL, 0, ext, _MAX_EXT);
    if (std::find(m_image_extns.begin(), m_image_extns.end(), std::wstring(ext)) != m_image_extns.end()) {
      // It should be an image!
      CGeneralMsgBox gmb;
      const CString cs_errmsg = L"Failed to load image";
      gmb.AfxMessageBox(cs_errmsg);
    }

    m_AttStatic.ShowWindow(SW_HIDE);
    m_stcNoPreview.ShowWindow(SW_SHOW);
    m_attType = ATTACHMENT_NOT_IMAGE;
  } else {
    // Success - was an image
    m_AttStatic.ShowWindow(SW_SHOW);
    m_stcNoPreview.ShowWindow(SW_HIDE);
    m_attType = ATTACHMENT_IS_IMAGE;
  }

  CItemAtt &att = M_attachment();
  int status = att.Import(LPCWSTR(m_AttFile));
  ASSERT(status == PWScore::SUCCESS); // CImage loaded it, how can we fail??
  if (!att.HasUUID())
    att.CreateUUID();

  // Now draw image
  // Use original size if image is bigger otherwise resize and centre control to fit
  if (!m_AttImage.IsNull()) {
    int image_h = m_AttImage.GetHeight();
    int image_w = m_AttImage.GetWidth();
    CPoint centre_point = m_initial_clientrect.CenterPoint();

    if (image_h < m_initial_clientrect.Height() && image_w < m_initial_clientrect.Width()) {
      // Centre image
      int iNewLeft = centre_point.x - image_w / 2;
      int iNewTop = centre_point.y - image_h / 2;
      m_AttStatic.MoveWindow(iNewLeft + m_xoffset, iNewTop + m_yoffset, image_w, image_h, TRUE);

      // Get new client rectangle
      m_AttStatic.GetClientRect(m_clientrect);
    } else {
      // Use intial (maximum size) client rectangle
      // But might need to resize if the image aspect ratio is different to the control
      m_clientrect = m_initial_clientrect;

      double dWidth = m_initial_clientrect.Width();
      double dHeight = m_initial_clientrect.Height();
      double dAspectRatio = dWidth / dHeight;

      double dImageWidth = image_w;
      double dImageHeight = image_h;
      double dImageAspectRatio = dImageWidth / dImageHeight;

      // If the aspect ratios are the same then the control rectangle
      // will do, otherwise we need to calculate the new rectangle
      if (dImageAspectRatio > dAspectRatio) {
        int nNewHeight = (int)(dWidth / dImageWidth * dImageHeight);
        //int nCenteringFactor = (m_initial_clientrect.Height() - nNewHeight) / 2;
        m_clientrect.SetRect(0, 0, (int)dWidth, nNewHeight);

      } else if (dImageAspectRatio < dAspectRatio) {
        int nNewWidth = (int)(dHeight / dImageHeight * dImageWidth);
        //int nCenteringFactor = (m_initial_clientrect.Width() - nNewWidth) / 2;
        m_clientrect.SetRect(0, 0, nNewWidth, (int)(dHeight));
      }

      int iNewLeft = centre_point.x - m_clientrect.Width() / 2;
      int iNewTop = centre_point.y - m_clientrect.Height() / 2;
      m_AttStatic.MoveWindow(iNewLeft + m_xoffset, iNewTop + m_yoffset, 
        m_clientrect.Width(), m_clientrect.Height(), TRUE);
    }

    // Now paint it
    m_AttImage.StretchBlt(m_AttStatic.GetDC()->GetSafeHdc(), 0, 0,
      m_clientrect.Width(), m_clientrect.Height(), SRCCOPY);
  }

  m_ae_psh->SetChanged(true);
  Invalidate();
  UpdateControls();
  UpdateData(FALSE);
  UpdateWindow();
}

void CAddEdit_Attachment::OnAttExport()
{
  CString filter;
  CSimpleArray<GUID> aguidFileTypes;
  HRESULT hResult;

  wchar_t fname[_MAX_FNAME],  ext[_MAX_EXT];
  _wsplitpath_s(m_AttFile, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

  switch (m_attType) {
    case NO_ATTACHMENT:
      return;
    case ATTACHMENT_IS_IMAGE:
    {
      if (m_AttImage.IsNull())
        return;

      hResult = m_AttImage.GetExporterFilterString(filter, aguidFileTypes);
      if (FAILED(hResult))
        return;

      CFileDialog fileDlg(FALSE, ext, fname, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, this);
      if (fileDlg.DoModal() == IDOK) {
        const CString sfile = fileDlg.GetPathName();
        hResult = m_AttImage.Save(sfile);
        if (FAILED(hResult)) {
          CGeneralMsgBox gmb;
          const CString cs_errmsg = L"Failed to save image";
          gmb.AfxMessageBox(cs_errmsg);
          return;
        }
      } else {
        // User cancelled save
        return;
      }
    }
    case ATTACHMENT_NOT_IMAGE:
    {
      // Set filter "??? files (*.???)|*.???||"
      SHFILEINFO sfi = {0};
      DWORD_PTR dw = SHGetFileInfo(m_AttFile, 0, &sfi, sizeof(sfi), SHGFI_TYPENAME);
      if (dw != 0) {
        filter.Format(IDS_FDF_FILES, sfi.szTypeName, ext, ext);
      } else {
        // Use All files!
        filter.LoadString(IDS_FDF_ALL);
      }
      CFileDialog fileDlg(FALSE, ext, fname, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, this);
      if (fileDlg.DoModal() == IDOK) {
        const CString sfile = fileDlg.GetPathName();
        // TODO - Need to write out non image file
        return;
      } else {
        // User cancelled save
        return;
      }
    }
  }
}

void CAddEdit_Attachment::OnAttRemove()
{
  if (!m_AttImage.IsNull()) {
    CBrush brDialog, brBlack;
    CRect rect;
    CDC *dc = m_AttStatic.GetDC();

    // Redraw current interior (using StretchBlt with SRCERASE leaves a black box)
    m_AttStatic.GetClientRect(rect);
    brDialog.CreateSolidBrush(::GetSysColor(COLOR_3DFACE));
    dc->FillRect(&rect, &brDialog);
    brDialog.DeleteObject();

    // Reset static control size and repaint
    m_AttStatic.MoveWindow(&m_initial_windowrect, TRUE);

    // Redraw black frame
    brBlack.CreateSolidBrush(RGB(0, 0, 0));
    dc->FrameRect(&m_initial_clientrect, &brBlack);
    brBlack.DeleteObject();

    // Destory image
    m_AttImage.Destroy();
  }
  
  m_AttStatic.ShowWindow(SW_SHOW);
  m_stcNoPreview.ShowWindow(SW_HIDE);

  m_AttFile = m_AttName = L"";
  M_attachment().Clear();
  m_ae_psh->SetChanged(true);
  m_AttStatic.SetWindowText(L"");

  m_attType = NO_ATTACHMENT;

  UpdateControls();
  UpdateData(FALSE);
  UpdateWindow();
}

 void CAddEdit_Attachment::UpdateControls()
 {
   bool bHasAttachment = (m_attType != NO_ATTACHMENT);
   GetDlgItem(IDC_ATT_IMPORT)->EnableWindow(!bHasAttachment);
   GetDlgItem(IDC_ATT_EXPORT)->EnableWindow(bHasAttachment);
   GetDlgItem(IDC_ATT_REMOVE)->EnableWindow(bHasAttachment);
 }
