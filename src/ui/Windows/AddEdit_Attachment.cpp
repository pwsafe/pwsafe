/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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

#include "PWFileDialog.h"

#include "GeneralMsgBox.h"
#include "Fonts.h"

#include "os/file.h"
#include "os/debug.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <Shellapi.h>
#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
// CAddEdit_Attachment property page

IMPLEMENT_DYNAMIC(CAddEdit_Attachment, CAddEdit_PropertyPage)

CAddEdit_Attachment::CAddEdit_Attachment(CWnd *pParent, st_AE_master_data *pAEMD)
  : CAddEdit_PropertyPage(pParent, 
                          CAddEdit_Attachment::IDD, CAddEdit_Attachment::IDD_SHORT,
                          pAEMD),
   m_bInitdone(false), m_AttName(L""), m_AttFileName(L""), m_attType(NO_ATTACHMENT)
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
    DDX_Control(pDX, IDC_ATT_IMAGE, m_stImgAttachment);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddEdit_Attachment, CAddEdit_PropertyPage)
  //{{AFX_MSG_MAP(CAddEdit_Attachment)
  //ON_WM_PAINT()

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

BOOL CAddEdit_Attachment::PreTranslateMessage(MSG *pMsg)
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

  // Get Add/Edit font
  Fonts *pFonts = Fonts::GetInstance();
  CFont *pFont = pFonts->GetAddEditFont();

  // Change font size of the attachment name and file name fields
  GetDlgItem(IDC_ATT_NAME)->SetFont(pFont);
  GetDlgItem(IDC_ATT_FILE)->SetFont(pFont);

  // Check initial state
  if (!M_pci()->HasAttRef()) {
    m_attType = NO_ATTACHMENT;
  } else {
    // m_attachment() set in CAddEdit_PropertySheet::SetupInitialValues()
    // If we have an attachment, load & preview
    m_AttName = M_attachment().GetTitle();
    m_AttFileName = M_attachment().GetFilePath() + M_attachment().GetFileName();

    // Get other properties
    m_csMediaType = M_attachment().GetMediaType().c_str();
    if (m_csMediaType == L"unknown") {
      m_csMediaType.LoadString(IDS_UNKNOWN);
    }

    wchar_t szFileSize[256];
    StrFormatByteSize(M_attachment().GetContentSize(), szFileSize, 256);
    m_csSize = szFileSize;

    m_csFileCTime = M_attachment().GetFileCTime().c_str();
    if (m_csFileCTime.IsEmpty())
      m_csFileCTime.LoadString(IDS_NA);

    m_csFileMTime = M_attachment().GetFileMTime().c_str();
    if (m_csFileMTime.IsEmpty())
      m_csFileMTime.LoadString(IDS_NA);

    ShowPreview();
  }

  UpdateData(FALSE);
  UpdateControls();

  m_bInitdone = true;

  return TRUE;  // return TRUE unless you set the focus to a control
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
      // copy data into the entry - we do it ourselves here first
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

  if (M_attachment().HasContent()) {
    // Only if still an attachment do the following!
    M_attachment().SetTitle(m_AttName);
  }

  return CAddEdit_PropertyPage::OnApply();
}

void CAddEdit_Attachment::OnHelp()
{
  ShowHelp(L"::/html/attachments.html");
}

void CAddEdit_Attachment::OnPaint()
{
  CAddEdit_PropertyPage::OnPaint();
}

BOOL CAddEdit_Attachment::OnEraseBkgnd(CDC * /*pDC */)
{
  if (m_stImgAttachment.IsImageLoaded())
    m_stImgAttachment.RedrawWindow();

  return 1;
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
  HRESULT hr;

  UpdateData(TRUE);

  if (m_AttFileName.IsEmpty()) {
    // Ask user for file name - annoyingly - returned string is all in upper case!
    // Remove last separator
    const CString cs_allimages(MAKEINTRESOURCE(IDS_ALL_IMAGE_FILES));
    const DWORD dwExclude = CImage::excludeOther;
    hr = m_AttImage.GetImporterFilterString(filter, aguidFileTypes, cs_allimages, dwExclude);
    ASSERT(hr >= 0);

    // Make better visually
    filter = filter.Right(filter.GetLength() - cs_allimages.GetLength());
    filter.MakeLower();
    filter = cs_allimages + filter;

    // Remove last separator to add the all files
    filter = filter.Left(filter.GetLength() - 1);

    // Add "All files"
    const CString cs_allfiles(MAKEINTRESOURCE(IDS_FDF_ALL));
    filter.Append(cs_allfiles);

    CFileDialog fileDlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, filter, this);
    if (fileDlg.DoModal() == IDCANCEL)
      return;

    m_AttFileName = CSecString(fileDlg.GetPathName());
  } else {
    if (!pws_os::FileExists(LPCWSTR(m_AttFileName))) {
      CGeneralMsgBox gmb;
      gmb.AfxMessageBox(IDS_ATTACHMENT_NOTFOUND);
      return;
    }
  }

  // Get file information
  struct _stati64 info;
  VERIFY(_wstati64(m_AttFileName, &info) == 0);

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
  HRESULT hr;
  std::wstring soutputfile;

  wchar_t fname[_MAX_FNAME], ext[_MAX_EXT], new_ext[_MAX_EXT];
  _wsplitpath_s(m_AttFileName, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

  // Default suffix should be the same as the original file (skip over leading ".")
  CString cs_ext = ext[0] == '.' ? ext + 1 : ext;

  switch (m_attType) {
    case NO_ATTACHMENT:
      return;
    case ATTACHMENT_IS_IMAGE:
    {
      if (m_AttImage.IsNull())
        return;

      hr = m_AttImage.GetExporterFilterString(filter, aguidFileTypes);
      if (FAILED(hr))
        return;

      // Extensions look much nicer in lower case
      filter.MakeLower();

      // Get index of current extension in filter string - note in pairs so need to skip every other one
      int cPos = 0;
      int iIndex = 1;  // Unusually, the filter index starts at 1 not 0
      CString cs_ext_nocase(ext); cs_ext_nocase.MakeLower();
      CString cs_filter_nocase(filter);
      CString cs_token;
      cs_token = cs_filter_nocase.Tokenize(L"|", cPos);  // Descriptions
      cs_token = cs_filter_nocase.Tokenize(L"|", cPos);  // Extensions
      if (cs_token.Find(cs_ext_nocase) == -1) {
        while (!cs_token.IsEmpty()) {
          cs_token = cs_filter_nocase.Tokenize(L"|", cPos);  // Descriptions
          cs_token = cs_filter_nocase.Tokenize(L"|", cPos);  // Extensions
          if (cs_token.Find(cs_ext_nocase) != -1)
            break;
          iIndex++;  // Documentation says index is per pair of file types
        };
      }

      CPWFileDialog fileDlg(FALSE, cs_ext, fname, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, this);
      fileDlg.m_pOFN->nFilterIndex = iIndex + 1;  // Not sure why need to add 1 but it seems to work!

      if (fileDlg.DoModal() == IDOK) {
        soutputfile = fileDlg.GetPathName();

        // Get new extension
        _wsplitpath_s(m_AttFileName, NULL, 0, NULL, 0, NULL, 0, new_ext, _MAX_EXT);
        
        // If new extension is the same as old, export the file rather than use
        // CImage to save it (which may well change its size)
        if (_wcsicmp(ext, new_ext) == 0) {
          int rc = M_attachment().Export(soutputfile);
          hr = (rc == PWScore::SUCCESS) ? S_OK : E_FAIL;
        } else {
          hr = m_AttImage.Save(soutputfile.c_str());
        }

        if (FAILED(hr)) {
          CGeneralMsgBox gmb;
          const CString cs_errmsg(MAKEINTRESOURCE(IDS_IMAGE_SAVE_FAILED));
          gmb.AfxMessageBox(cs_errmsg);
          return;
        }
      } else {
        // User cancelled save
        return;
      }
      break;
    }
    case ATTACHMENT_NOT_IMAGE:
    {
      // Set filter "??? files (*.???)|*.???||"
      SHFILEINFO sfi = {0};
      DWORD_PTR dw = SHGetFileInfo(m_AttFileName, 0, &sfi, sizeof(sfi), SHGFI_TYPENAME);
      if (dw != 0) {
        filter.Format(IDS_FDF_FILES, static_cast<LPCWSTR>(sfi.szTypeName), ext, ext);
      } else {
        // Use All files!
        filter.LoadString(IDS_FDF_ALL);
      }
      CPWFileDialog fileDlg(FALSE, cs_ext, fname, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, this);
      if (fileDlg.DoModal() == IDOK) {
        soutputfile = fileDlg.GetPathName();
        M_attachment().Export(soutputfile);
      } else {
        // User cancelled save
        return;
      }
      break;
    }
  }

  // We have saved/exported the file - now reset its file times to the original
  // when it was first imported/added
  if (soutputfile.empty()) {
    // Shouldn't get here with an empty export file name!
    ASSERT(0);
    return;
  }

  // Get old file times
  time_t ctime, mtime, atime;
  M_attachment().GetFileCTime(ctime);
  M_attachment().GetFileMTime(mtime);
  M_attachment().GetFileATime(atime);
  
  bool bUpdateFileTimes = pws_os::SetFileTimes(soutputfile, ctime, mtime, atime);
  if (!bUpdateFileTimes) {
    pws_os::Trace(L"Unable to open newly exported file to set file times.");
  }
}

void CAddEdit_Attachment::OnAttRemove()
{
  if (m_stImgAttachment.IsImageLoaded())
    m_stImgAttachment.ClearImage();

  if (!m_AttImage.IsNull())
    m_AttImage.Destroy();

  m_AttFileName = m_AttName = L"";
  m_csSize = m_csFileCTime = m_csFileMTime = m_csMediaType = L"";
  M_attachment().Clear();

  m_ae_psh->SetChanged(true);
  m_stImgAttachment.SetWindowText(L"");

  m_attType = NO_ATTACHMENT;

  UpdateControls();
  UpdateData(FALSE);
  UpdateWindow();
}

 void CAddEdit_Attachment::UpdateControls()
 {
   bool bHasAttachment = M_attachment().HasContent();
   bool bIsRO = (M_uicaller() == IDS_VIEWENTRY ||
                 (M_uicaller() == IDS_EDITENTRY && M_protected() != 0));

   ((CEdit *)GetDlgItem(IDC_ATT_NAME))->SetReadOnly(bIsRO);
   
   // Currently only accept one attachment per entry
   // If already have one, don't allow drop of any more
   m_stImgAttachment.DragAcceptFiles(bHasAttachment || bIsRO ? FALSE : TRUE);

   // Set up buttons
   GetDlgItem(IDC_ATT_IMPORT)->EnableWindow(!bHasAttachment && !bIsRO);
   GetDlgItem(IDC_ATT_EXPORT)->EnableWindow(bHasAttachment);
   GetDlgItem(IDC_ATT_REMOVE)->EnableWindow(bHasAttachment && !bIsRO);

   // Don't allow user to change file name if attachment is present
   ((CEdit *)GetDlgItem(IDC_ATT_FILE))->SetReadOnly(bHasAttachment || bIsRO);

   switch (m_attType) {
     case ATTACHMENT_NOT_IMAGE:
       m_stImgAttachment.ShowWindow(SW_HIDE);
       m_stcNoPreview.ShowWindow(SW_SHOW);
       break;
     case NO_ATTACHMENT:
     case ATTACHMENT_IS_IMAGE:
       m_stImgAttachment.ShowWindow(SW_SHOW);
       m_stcNoPreview.ShowWindow(SW_HIDE);
       break;
   }
 }
 
void CAddEdit_Attachment::ShowPreview()
{
  CItemAtt &att = M_attachment();
  HRESULT hr(S_OK);

  int rc(0);

  // Assume not an image
  m_attType = ATTACHMENT_NOT_IMAGE;

  if (!att.HasContent()) {
    // No content so filename must not be empty
    if (m_AttFileName.IsEmpty())
      return;

    int status = att.Import(LPCWSTR(m_AttFileName));
    if (status != PWScore::SUCCESS) {
      // most likely file error - TBD better error reporting
      rc = 1;
      goto load_error;
    }

    // Get media type before we find we can't load it
    m_csMediaType = att.GetMediaType().c_str();

    // Get other properties
    wchar_t szFileSize[256];
    StrFormatByteSize(att.GetContentSize(), szFileSize, 256);
    m_csSize = szFileSize;
    m_csFileCTime = att.GetFileCTime().c_str();
    if (m_csFileCTime.IsEmpty())
      m_csFileCTime.LoadString(IDS_NA);
    m_csFileMTime = att.GetFileMTime().c_str();
    if (m_csFileMTime.IsEmpty())
      m_csFileMTime.LoadString(IDS_NA);

    if (m_csMediaType.Left(5) == L"image") {
      // Should be an image file - but may not be supported by CImage - try..
      hr = m_AttImage.Load(m_AttFileName);
      if (SUCCEEDED(hr)) {
        hr = m_stImgAttachment.Load(m_AttFileName);
      } 

      if (SUCCEEDED(hr)) {
        // Success - was an image
        m_attType = ATTACHMENT_IS_IMAGE;
      }
    }

    // Create UUID if not already present
    if (!att.HasUUID())
      att.CreateUUID();
  } else {// att.HasContent()
    // This should only be the case during the InitDialog - maybe m_bInitDone
    // in the logic for this processing rather than att.HasContent
    ASSERT(!m_bInitdone);

    if (m_csMediaType.Left(5) == L"image") {
      // Should be an image file - but may not be supported by CImage - try..
      // Allocate attachment buffer
      UINT imagesize = (UINT)att.GetContentSize();
      HGLOBAL gMemory = GlobalAlloc(GMEM_MOVEABLE, imagesize);
      ASSERT(gMemory);

      if (gMemory == NULL) {
        rc = 2;
        goto load_error;
      }

      BYTE *pBuffer = (BYTE *)GlobalLock(gMemory);
      ASSERT(pBuffer);

      if (pBuffer == NULL) {
        rc = 3;
        GlobalFree(gMemory);
        goto load_error;
      }

      // Load image into buffer
      att.GetContent(pBuffer, imagesize);

      // Put it into a IStream
      IStream *pStream = NULL;
      hr = CreateStreamOnHGlobal(gMemory, FALSE, &pStream);
      if (SUCCEEDED(hr)) {
        // Load it
        hr = m_AttImage.Load(pStream);
        if (SUCCEEDED(hr)) {
          hr = m_stImgAttachment.Load(pStream);
        }
      }

      // Clean up - no real need to trash the buffer
      if (pStream != NULL)
        pStream->Release();

      GlobalUnlock(gMemory);
      GlobalFree(gMemory);

      // Check image load (or previous error of putting content into an IStream)
      if (FAILED(hr)) {
        goto load_error;
      }

      // Success - was an image
      m_attType = ATTACHMENT_IS_IMAGE;
    }
  }

  return;

load_error:
  // Ooops???
  m_stImgAttachment.IssueError(rc, hr);
}
