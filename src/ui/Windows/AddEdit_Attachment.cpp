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

#include <sys/types.h>
#include <sys/stat.h>

#include <Shellapi.h>
#include <algorithm>

BEGIN_MESSAGE_MAP(CDragDropAttachment, CStatic)
  ON_WM_DROPFILES()
END_MESSAGE_MAP()

void CDragDropAttachment::OnDropFiles(HDROP hDropInfo)
{
  CStatic::OnDropFiles(hDropInfo);

  UINT nCntFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, 0, 0);

  // Shouldn't have zero files if called!
  if (nCntFiles == 0)
    return;

  if (nCntFiles > 1) {
    const CString cs_errmsg = L"Sorry, currently there is a limit of only one attachment per entry";
    ::AfxMessageBox(cs_errmsg);
    return;
  }

  wchar_t szBuf[MAX_PATH];
  ::DragQueryFile(hDropInfo, 0, szBuf, sizeof(szBuf));

  // Get parent to process this file
  CWnd *pWnd = GetParent();
  ASSERT(pWnd);

  // Use SendMessage rather than PastMessage so that szBuf doesn't go out of scope
  // Send nCntFiles even though only one attachment is supported at the moment
  pWnd->SendMessage(PWS_MSG_DROPPED_FILE, (WPARAM)szBuf, nCntFiles);
}

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_Attachment property page

IMPLEMENT_DYNAMIC(CAddEdit_Attachment, CAddEdit_PropertyPage)

CAddEdit_Attachment::CAddEdit_Attachment(CWnd *pParent, st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent, 
                          CAddEdit_Attachment::IDD, CAddEdit_Attachment::IDD_SHORT,
                          pAEMD),
   m_bInitdone(false), m_AttName(_T("")), m_AttFileName(_T("")), m_attType(NO_ATTACHMENT)
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
    DDX_Text(pDX, IDC_ATT_FILE, m_AttFileName);
    DDX_Text(pDX, IDC_FILEMTYPE, m_csMediaType);
    DDX_Text(pDX, IDC_FILESIZE, m_csSize);
    DDX_Text(pDX, IDC_FILECTIME, m_csFileCTime);
    DDX_Text(pDX, IDC_FILEMTIME, m_csFileMTime);

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

  // Check initial state
  if (!M_pci()->HasAttRef()) {
    m_attType = NO_ATTACHMENT;
  } else {
    // If we have an attachment - preview it
    ShowPreview();
  }

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
#ifdef DEBUG
  ASSERT(lParam == 1);
#else
  UNREFERENCED_PARAMETER(lParam);
#endif

  wchar_t *sxFileName = reinterpret_cast<wchar_t *>(wParam);
  m_AttFileName = sxFileName;

  // Update dialog with filename so that Import uses it
  UpdateData(FALSE);

  // Import file
  OnAttImport();
  return 0;
}

void CAddEdit_Attachment::OnAttImport()
{
  CString filter;
  CSimpleArray<GUID> aguidFileTypes;
  HRESULT hResult;

  UpdateData(TRUE);

  if (m_AttFileName.IsEmpty()) {
    // Ask user for file name
    // Remove last separator
    const CString cs_allimages(MAKEINTRESOURCE(IDS_ALL_IMAGE_FILES));
    const DWORD dwExclude = CImage::excludeOther;
    hResult = m_AttImage.GetImporterFilterString(filter, aguidFileTypes, cs_allimages, dwExclude);
    ASSERT(hResult >= 0);

    filter = filter.Left(filter.GetLength() - 1);

    // Add "All files"
    const CString cs_allfiles(MAKEINTRESOURCE(IDS_FDF_ALL));
    filter.Append(cs_allfiles);

    CFileDialog fileDlg(TRUE, NULL, NULL, 0, filter, this);
    if (fileDlg.DoModal() == IDCANCEL)
      return;

    m_AttFileName = fileDlg.GetPathName();
  } else {
    if (!pws_os::FileExists(std::wstring(m_AttFileName))) {
      CGeneralMsgBox gmb;
      gmb.AfxMessageBox(IDS_ATTACHMENT_NOTFOUND);
      return;
    }
  }

  // Get file information
  struct _stati64 info;
  int rc = _wstati64(m_AttFileName, &info);
  ASSERT(rc == 0);

  m_csFileCTime = PWSUtil::ConvertToDateTimeString(info.st_ctime, PWSUtil::TMC_LOCALE).c_str();
  m_csFileMTime = PWSUtil::ConvertToDateTimeString(info.st_mtime, PWSUtil::TMC_LOCALE).c_str();

  ShowPreview();

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
  _wsplitpath_s(m_AttFileName, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

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
      DWORD_PTR dw = SHGetFileInfo(m_AttFileName, 0, &sfi, sizeof(sfi), SHGFI_TYPENAME);
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

  m_AttFileName = m_AttName = L"";
  m_csSize = m_csFileCTime = m_csFileMTime = m_csMediaType = L"";
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

   // Currently only accept one attachment per entry
   // If already have one, don't allow drop of any more
   m_AttStatic.DragAcceptFiles(bHasAttachment ? FALSE : TRUE);

   // Set up buttons
   GetDlgItem(IDC_ATT_IMPORT)->EnableWindow(!bHasAttachment);
   GetDlgItem(IDC_ATT_EXPORT)->EnableWindow(bHasAttachment);
   GetDlgItem(IDC_ATT_REMOVE)->EnableWindow(bHasAttachment);

   // Don't allow user to change file name if attachment is present
   ((CEdit *)GetDlgItem(IDC_ATT_FILE))->SetReadOnly(bHasAttachment);

   switch (m_attType) {
     case ATTACHMENT_NOT_IMAGE:
       m_AttStatic.ShowWindow(SW_HIDE);
       m_stcNoPreview.ShowWindow(SW_SHOW);
       break;
     case NO_ATTACHMENT:
     case ATTACHMENT_IS_IMAGE:
       m_AttStatic.ShowWindow(SW_SHOW);
       m_stcNoPreview.ShowWindow(SW_HIDE);
       break;
   }
 }
 
 void CAddEdit_Attachment::ShowPreview()
 {
   HRESULT hResult;

   wchar_t ext[_MAX_EXT];
   _wsplitpath_s(m_AttFileName, NULL, 0, NULL, 0, NULL, 0, ext, _MAX_EXT);

   // Get Media type - RFC 6838
   m_csMediaType.Empty();

   LPWSTR pwzMimeOut = NULL;
   // Note 1: FMFD_IGNOREMIMETEXTPLAIN not defined (UrlMon.h) if still supporting Windows XP
   // Note 2: FMFD_RETURNUPDATEDIMGMIMES not defined (UrlMon.h) in SDK 7.1A - need SDK 8.1 or later
   // Hardcode values for now
   DWORD dwMimeFlags = FMFD_URLASFILENAME | 0x4 /*FMFD_IGNOREMIMETEXTPLAIN*/ | 0x20 /*FMFD_RETURNUPDATEDIMGMIMES*/;
   hResult = FindMimeFromData(NULL, ext, NULL, 0, NULL, dwMimeFlags, &pwzMimeOut, 0);

   if (SUCCEEDED(hResult)) {
     m_csMediaType = pwzMimeOut;
     CoTaskMemFree(pwzMimeOut);
   }

   // Assume not an image
   m_attType = ATTACHMENT_NOT_IMAGE;

   if (m_csMediaType.Find(L"image") != -1) {
     // Should be an image file - but may not be supported by CImage - try..
     hResult = m_AttImage.Load(m_AttFileName);
     if (FAILED(hResult)) {
       // Ooops???
       CGeneralMsgBox gmb;
       const CString cs_errmsg = L"Failed to load image";
       gmb.AfxMessageBox(cs_errmsg);
     } else {
       // Success - was an image
       m_attType = ATTACHMENT_IS_IMAGE;
     }
   }

   CItemAtt &att = M_attachment();
   int status = att.Import(LPCWSTR(m_AttFileName));
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

   // Get properties
   wchar_t szFileSize[256];
   StrFormatByteSize(M_attachment().GetContentSize(), szFileSize, 256);
   m_csSize = szFileSize;

   // m_csFileCTime = M_attachment().GetFileCTime();
   // m_csFileMTime = M_attachment().GetFileMTime();
 }