/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// ViewAttahmentDlg.cpp : implementation file
//

#include "stdafx.h"

#include "ViewAttachmentDlg.h"
#include "GeneralMsgBox.h"

#include "os/debug.h"

// CViewAttachmentDlg dialog

IMPLEMENT_DYNAMIC(CViewAttachmentDlg, CPWResizeDialog)

CViewAttachmentDlg::CViewAttachmentDlg(CWnd *pParent, CItemAtt *patt)
	: CPWResizeDialog(IDD_VIEWATTACHMENT, pParent), m_pattachment(patt)
{
}

CViewAttachmentDlg::~CViewAttachmentDlg()
{
}

void CViewAttachmentDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_ATT_IMAGE, m_stImgAttachment);
}

BEGIN_MESSAGE_MAP(CViewAttachmentDlg, CPWResizeDialog)
  ON_WM_SIZE()
  ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP()

// CViewAttachmentDlg message handlers

BOOL CViewAttachmentDlg::OnInitDialog()
{
  std::vector<UINT> vibottombtns;
  vibottombtns.push_back(IDOK);

  AddMainCtrlID(IDC_ATT_IMAGE);
  AddBtnsCtrlIDs(vibottombtns, 0);

  UINT statustext[1] = { IDS_BLANK };
  SetStatusBar(&statustext[0], 1, false);

  CPWResizeDialog::OnInitDialog();

  // Get current dialog dimensions
  CRect dlgRect;
  GetClientRect(&dlgRect);

  // Set it to allow to get 5 times as big (for those with big screens!)
  SetMaxHeightWidth(5 * dlgRect.Height(), 5 * dlgRect.Width());

  // Should be an image file - but may not be supported by CImage - try..
  // Allocate attachment buffer
  int rc(0);
  HRESULT hr(S_OK);

  UINT imagesize = m_pattachment->GetContentSize();
  HGLOBAL gMemory = GlobalAlloc(GMEM_MOVEABLE, imagesize);
  ASSERT(gMemory);

  if (gMemory == NULL) {
    // Ooops???
    rc = 2;  // Document this!
    goto load_error;
  }

  BYTE *pBuffer = (BYTE *)GlobalLock(gMemory);
  ASSERT(pBuffer);

  if (pBuffer == NULL) {
    rc = 3;  // Document this!
    GlobalFree(gMemory);
    goto load_error;
  }

  // Load image into buffer
  m_pattachment->GetContent(pBuffer, imagesize);

  // Put it into a IStream
  IStream *pStream = NULL;
  hr = CreateStreamOnHGlobal(gMemory, FALSE, &pStream);
  if (SUCCEEDED(hr)) {
    // Load it
    hr = m_stImgAttachment.Load(pStream);
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

  return TRUE;

load_error:
  // Ooops???
  m_stImgAttachment.IssueError(rc, hr);

  // Stop dialog opening at all
  EndDialog(IDCANCEL);
  return TRUE;
}

void CViewAttachmentDlg::OnOK()
{
  CPWResizeDialog::OnOK();
}

void CViewAttachmentDlg::OnSize(UINT nType, int cx, int cy)
{
  CPWResizeDialog::OnSize(nType, cx, cy);

  if (m_stImgAttachment.IsImageLoaded()) {
    m_stImgAttachment.RedrawWindow();
  }
}