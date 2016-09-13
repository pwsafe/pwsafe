/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
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
#include "SelectAttachment.h"
#include "Reuse_Attachment.h"

#include "GeneralMsgBox.h"
#include "Fonts.h"

#include "os/dir.h"
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
  m_bInitdone(false), m_iAttachmentRefcount(0),
  m_AttName(L""), m_AttFileName(L""), m_attType(NO_ATTACHMENT)
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
    DDX_Control(pDX, IDC_STATIC_REFERENCEDBY, m_stReferences);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddEdit_Attachment, CAddEdit_PropertyPage)
  //{{AFX_MSG_MAP(CAddEdit_Attachment)
  // Common
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  ON_BN_CLICKED(ID_HELP, OnHelp)
  ON_STN_CLICKED(IDC_STATIC_REFERENCEDBY, OnListEntries)

  // Our message handlers
  ON_BN_CLICKED(IDC_ATT_IMPORT, OnAttImport)
  ON_BN_CLICKED(IDC_ATT_EXPORT, OnAttExport)
  ON_BN_CLICKED(IDC_ATT_REMOVE, OnAttRemove)
  ON_BN_CLICKED(IDC_ATT_ATTACH, OnAttLinkExisting)

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

  // Do tooltips 
  if (pMsg->message == WM_MOUSEMOVE) {
    if (m_pToolTipCtrl != NULL) {
      // Change to allow tooltip on disabled controls
      MSG msg = *pMsg;
      msg.hwnd = (HWND)m_pToolTipCtrl->SendMessage(TTM_WINDOWFROMPOINT, 0,
                         (LPARAM)&msg.pt);
      CPoint pt = pMsg->pt;
      ::ScreenToClient(msg.hwnd, &pt);

      msg.lParam = MAKELONG(pt.x, pt.y);

      // Now let the ToolTip process this message.
      m_pToolTipCtrl->RelayEvent(&msg);
    }
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

  if (InitToolTip()) {
    AddTool(IDC_ATT_IMPORT, IDS_ATT_IMPORT);
    AddTool(IDC_ATT_EXPORT, IDS_ATT_EXPORT);
    AddTool(IDC_ATT_REMOVE, IDS_ATT_REMOVE);
    AddTool(IDC_ATT_ATTACH, IDS_ATT_ATTACH);

    ActivateToolTip();
  }

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

    wchar_t szFileSize[256];
    StrFormatByteSize(M_attachment().GetContentSize(), szFileSize, 256);
    m_csSize = szFileSize;

    m_csFileCTime = M_attachment().GetFileCTime().c_str();
    if (m_csFileCTime.IsEmpty())
      m_csFileCTime.LoadString(IDS_NA);

    m_csFileMTime = M_attachment().GetFileMTime().c_str();
    if (m_csFileMTime.IsEmpty())
      m_csFileMTime.LoadString(IDS_NA);

    m_iAttachmentRefcount = M_attachment().GetRefcount();

    ShowPreview();
  }

  UpdateData(FALSE);
  UpdateControls();

  m_bInitdone = true;

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

void CAddEdit_Attachment::OnListEntries()
{
  PWScore *pcore = (PWScore *)GetMainDlg()->GetCore();

  pws_os::CUUID att_uuid = M_attachment().GetUUID();

  ItemMMap_Range eq = pcore->GetAttRange(att_uuid);

  std::vector<st_gtui> vgtui;

  for (ItemMMapConstIter it = eq.first; it != eq.second; ++it) {
    pws_os::CUUID itemUUID = it->second;
    ItemListIter iter = pcore->Find(itemUUID);

    st_gtui stgtui;

    CItemData &item = pcore->GetEntry(iter);
    stgtui.sxGroup = item.GetGroup();
    stgtui.sxTitle = item.GetTitle();
    stgtui.sxUser = item.GetUser();
    stgtui.image = GetMainDlg()->GetEntryImage(item);

    vgtui.push_back(stgtui);
  }

  CViewAttachmentEntriesDlg dlg(this, &vgtui);

  dlg.DoModal();
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
  int rc = _wstati64(m_AttFileName, &info);
  ASSERT(rc == 0);

  m_csFileCTime = PWSUtil::ConvertToDateTimeString(info.st_ctime, PWSUtil::TMC_LOCALE).c_str();
  m_csFileMTime = PWSUtil::ConvertToDateTimeString(info.st_mtime, PWSUtil::TMC_LOCALE).c_str();

  // Before we import it - check we don't already have it
  std::vector<pws_os::CUUID> vSimilarFiles;
  for (auto attPos = M_pcore()->GetAttIter(); attPos != M_pcore()->GetAttEndIter();
            attPos++) {
    if (attPos->second.GetContentLength() == info.st_size) {
      vSimilarFiles.push_back(attPos->first);
    }
  }

  CItemAtt &att_new = M_attachment();
  hr = S_OK;

  // Now import it
  int status = att_new.Import(LPCWSTR(m_AttFileName));
  if (status != PWScore::SUCCESS) {
    // most likely file error - TBD better error reporting
    rc = 1;
    goto load_error;
  }

  if (vSimilarFiles.size() > 0) {
    unsigned char newfile_digest[SHA1::HASHLEN];
    att_new.GetContentDigest(newfile_digest);
    std::wstring spath, sdrive, sdir, sfname, sextn;

    pws_os::splitpath(m_AttFileName, sdrive, sdir, sfname, sextn);
    spath = pws_os::makepath(sdrive, sdir, L"", L"");

    for (size_t i = 0; i < vSimilarFiles.size(); i++) {
      unsigned char digest[SHA1::HASHLEN];
      CItemAtt &att_sim = M_pcore()->GetAtt(vSimilarFiles[i]);
      att_sim.GetContentDigest(digest);
      if (memcmp(digest, newfile_digest, SHA1::HASHLEN) == 0) {
        CReuse_Attachment dlg;
        dlg.SetFileDetails((sfname + sextn).c_str(), spath.c_str(),
                            att_sim.GetFileName(), att_sim.GetFilePath(), att_sim.GetTitle());
        INT_PTR dlgrc = dlg.DoModal();
        if (dlgrc == IDOK) {
          // Use orginal
          // By setting the new attachments UUID to the existing one - it will be detected
          // later and a link added instead of a whole new attachment
          M_attachment().SetUUID(vSimilarFiles[i]);
          att_new.SetUUID(vSimilarFiles[i]);

          // Update fields in attachment and this dialog
          m_AttName = att_sim.GetTitle();
          M_attachment().SetTitle(att_sim.GetTitle());
          m_AttFileName = att_sim.GetFilePath() + att_sim.GetFileName();

          // Prevents user changing anything for this linked attachment
          m_iAttachmentRefcount = att_sim.GetRefcount() + 1;
          break;
        }
      }
    }
  }

  // Get media type before we find we can't load it
  m_csMediaType = att_new.GetMediaType().c_str();

  // Get other properties
  wchar_t szFileSize[256];
  StrFormatByteSize(att_new.GetContentSize(), szFileSize, 256);
  m_csSize = szFileSize;
  m_csFileCTime = att_new.GetFileCTime().c_str();
  if (m_csFileCTime.IsEmpty())
    m_csFileCTime.LoadString(IDS_NA);
  m_csFileMTime = att_new.GetFileMTime().c_str();
  if (m_csFileMTime.IsEmpty())
    m_csFileMTime.LoadString(IDS_NA);

  //if (m_csMediaType.Left(5) == L"image") {
  //  // Should be an image file - but may not be supported by CImage - try..
  //  hr = m_AttImage.Load(m_AttFileName);
  //  if (SUCCEEDED(hr)) {
  //    hr = m_stImgAttachment.Load(m_AttFileName);
  //  }

  //  if (SUCCEEDED(hr)) {
  //    // Success - was an image
  //    m_attType = ATTACHMENT_IS_IMAGE;
  //  }
  //}

  // Create UUID if not already present
  if (!att_new.HasUUID())
    att_new.CreateUUID();

  ShowPreview();

  m_ae_psh->SetChanged(true);
  Invalidate();
  UpdateControls();
  UpdateData(FALSE);
  UpdateWindow();

  return;

load_error:
  // Ooops???
  m_stImgAttachment.IssueError(rc, hr);
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
  CString cs_ext = ext + 1;

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

      CPWFileDialog fileDlg(FALSE, cs_ext, fname,
                            OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, this);
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
  if (m_stImgAttachment.IsImageLoaded()) {
    m_stImgAttachment.ClearImage();
  }

  if (!m_AttImage.IsNull())
    m_AttImage.Destroy();

  m_AttFileName = m_AttName = L"";
  m_csSize = m_csFileCTime = m_csFileMTime = m_csMediaType = L"";
  M_attachment().Clear();
  m_iAttachmentRefcount = 0;

  m_ae_psh->SetChanged(true);
  m_stImgAttachment.SetWindowText(L"");

  m_attType = NO_ATTACHMENT;

  // Re-enable user input in case disabled during Attach
  m_iAttachmentRefcount = 0;

  UpdateControls();
  UpdateData(FALSE);
  UpdateWindow();
}

void CAddEdit_Attachment::OnAttLinkExisting()
{
  pws_os::CUUID att_uuid = pws_os::CUUID::NullUUID();
  bool bOrphaned(false);
  CSelectAttachment dlg(this, &att_uuid, &bOrphaned);

  INT_PTR rc = dlg.DoModal();

  if (rc >= 0 && att_uuid != pws_os::CUUID::NullUUID()) {
    // Attach this to current entry
    M_attachment() = M_pcore()->GetAtt(att_uuid);

    m_AttName = M_attachment().GetTitle();
    m_AttFileName = M_attachment().GetFilePath() + M_attachment().GetFileName();

    // Get other properties
    m_csMediaType = M_attachment().GetMediaType().c_str();

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

    m_ae_psh->SetChanged(true);

    // Need to prevent user changing any fields otherwise won't be the same attachment
    M_attachment().IncRefcount();
    m_iAttachmentRefcount = M_attachment().GetRefcount();

    Invalidate();
    UpdateControls();
    UpdateData(FALSE);
    UpdateWindow();
  }
}

void CAddEdit_Attachment::UpdateControls()
{
  bool bHasAttachment = M_attachment().HasContent();
  bool bIsRO = (M_uicaller() == IDS_VIEWENTRY ||
    (M_uicaller() == IDS_EDITENTRY && M_protected() != 0));
  BOOL bAttachmentsPresent = (M_pcore()->GetNumAtts() != 0);

  // Other entries reference this attachment - if user changes these fields, then
  // won't be the same - it does mean that the user can't rename the attachment until
  // only one entry has it.
  ((CEdit *)GetDlgItem(IDC_ATT_NAME))->SetReadOnly(bIsRO || m_iAttachmentRefcount > 1);
  ((CEdit *)GetDlgItem(IDC_ATT_FILE))->SetReadOnly(bIsRO || m_iAttachmentRefcount > 1);

  if (bIsRO || m_iAttachmentRefcount > 1) {
    // If fields now read-only, make dialog the focus rather than the attachment's name
    SetFocus();
  }

  m_stReferences.EnableWindow(m_iAttachmentRefcount > 1 ? TRUE : FALSE);
  m_stReferences.ShowWindow(m_iAttachmentRefcount > 1 ? SW_SHOW : SW_HIDE);

  // Currently only accept one attachment per entry
  // If already have one, don't allow drop of any more
  m_stImgAttachment.DragAcceptFiles(bHasAttachment || bIsRO ? FALSE : TRUE);

  // Set up buttons
  GetDlgItem(IDC_ATT_IMPORT)->EnableWindow(!bHasAttachment && !bIsRO);
  GetDlgItem(IDC_ATT_EXPORT)->EnableWindow(bHasAttachment);
  GetDlgItem(IDC_ATT_REMOVE)->EnableWindow(bHasAttachment && !bIsRO);
  GetDlgItem(IDC_ATT_ATTACH)->EnableWindow(!bHasAttachment && !bIsRO && bAttachmentsPresent);

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
    ASSERT(0);
    // Shouldn't ask for preview if no attachment!
  } else {// att.HasContent()
    if (m_csMediaType.Left(5) == L"image") {
      // Should be an image file - but may not be supported by CImage - try..
      // Allocate attachment buffer
      UINT imagesize = att.GetContentSize();
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
