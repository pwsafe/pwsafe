/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file ImgStatic.cpp
//-----------------------------------------------------------------------------

#include "StdAfx.h"

#include "Windowsdefs.h"
#include "ImgStatic.h"

#include "GeneralMsgBox.h"
#include "os/debug.h"
#include "resource3.h"

#pragma warning(push)
#pragma warning(disable:4458) // declaration of 'xxx' hides class member
#include <GdiPlus.h>
#pragma warning(pop)

using namespace Gdiplus;

CImgStatic::CImgStatic()
  : CStatic(), m_pStream(NULL), m_bImageLoaded(false), m_gdiplusToken(0)
{
  // Initialise Gdiplus graphics
  GdiplusStartupInput gdiplusStartupInput;
  GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}

CImgStatic::~CImgStatic()
{
  // Tidy up
  FreeStream();

  // Shutdown Gdiplus graphics
  GdiplusShutdown(m_gdiplusToken);
}

BEGIN_MESSAGE_MAP(CImgStatic, CStatic)
  ON_WM_ERASEBKGND()
  ON_WM_DROPFILES()
END_MESSAGE_MAP()

void CImgStatic::PreSubclassWindow()
{
  // Ensure that this is OWNERDRAW
  CStatic::PreSubclassWindow();
  ModifyStyle(0, SS_OWNERDRAW);
}

void CImgStatic::OnDropFiles(HDROP hDropInfo)
{
  CStatic::OnDropFiles(hDropInfo);

  UINT nCntFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, 0, 0);

  // Shouldn't have zero files if called!
  if (nCntFiles == 0)
    return;

  if (nCntFiles > 1) {
    const CString cs_errmsg(MAKEINTRESOURCE(IDS_IMAGE_LIMIT_1));
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

HRESULT CImgStatic::Load(CString &szFilePath)
{
  HRESULT hResult;

  // Delete any current image
  FreeStream();

  // Allocate stream
  hResult = CreateStreamOnHGlobal(NULL, TRUE, &m_pStream);
  if (FAILED(hResult)) {
    return hResult;
  }

  // Open file
  CFile cFile;
  CFileException cFileException;
  if (!cFile.Open(szFilePath, CStdioFile::modeRead | CStdioFile::typeBinary, &cFileException)) {
    if (m_pStream != NULL) {
      m_pStream->Release();
      m_pStream = NULL;
    }
    return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, cFileException.m_lOsError);
  }

  // Copy the specified file's content to the stream
  BYTE pBuffer[1024] = { 0 };
  while (UINT dwRead = cFile.Read(pBuffer, 1024)) {
    hResult = m_pStream->Write(pBuffer, dwRead, NULL);
    if (FAILED(hResult)) {
      if (m_pStream != NULL) {
        m_pStream->Release();
        m_pStream = NULL;
      }
      cFile.Close();
      return hResult;
    }
  }

  // Close the file
  cFile.Close();

  // Mark as Loaded
  m_bImageLoaded = true;

  Invalidate();
  RedrawWindow();

  return S_OK;
}

HRESULT CImgStatic::Load(IStream *piStream)
{
  HRESULT hResult;

  // Delete any current image
  FreeStream();

  // Check for validity of argument
  if (piStream == NULL) {
    return E_FAIL;
  }

  // Allocate stream
  hResult = CreateStreamOnHGlobal(NULL, TRUE, &m_pStream);
  if (FAILED(hResult)) {
    return hResult;
  }

  // Rewind the argument stream
  LARGE_INTEGER lInt;
  lInt.QuadPart = 0;
  piStream->Seek(lInt, STREAM_SEEK_SET, NULL);

  // Read the lenght of the argument stream
  STATSTG statSTG;
  hResult = piStream->Stat(&statSTG, STATFLAG_DEFAULT);
  if (FAILED(hResult)) {
    if (m_pStream != NULL) {
      m_pStream->Release();
      m_pStream = NULL;
    }
    return hResult;
  }

  // Copy the argument stream to the class stream
  piStream->CopyTo(m_pStream, statSTG.cbSize, NULL, NULL);

  // Mark as loaded
  m_bImageLoaded = true;

  Invalidate();
  RedrawWindow();

  return S_OK;
}

void SetAspectRatio(Image &image, RECT &rc)
{
  double dWidth = (rc.right - rc.left);
  double dHeight = (rc.bottom - rc.top);
  double dAspectRatio = dWidth / dHeight;
  double dPictureWidth = image.GetWidth();
  double dPictureHeight = image.GetHeight();
  double dPictureAspectRatio = dPictureWidth / dPictureHeight;
  if (dPictureAspectRatio > dAspectRatio) {
    double nNewHeight = (dWidth / dPictureWidth*dPictureHeight);
    double nCenteringFactor = (dHeight - nNewHeight) / 2;
    SetRect(&rc, 0, (int)nCenteringFactor, (int)dWidth, (int)(nNewHeight + nCenteringFactor));
  } else if (dPictureAspectRatio < dAspectRatio) {
    double nNewWidth = (dHeight / dPictureHeight*dPictureWidth);
    double nCenteringFactor = (dWidth - nNewWidth) / 2;
    SetRect(&rc, (int)nCenteringFactor, 0, (int)(nNewWidth + nCenteringFactor), (int)(dHeight));
  }
}

void CImgStatic::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
  // Check if pic data is loaded
  if (m_bImageLoaded) {
    RECT rc;
    GetClientRect(&rc);

    // Get Gdiplus graphics object
    Graphics grp(lpDrawItemStruct->hDC);

    // Get image
    Image image(m_pStream);

    // Adjust for aspect ratio
    SetAspectRatio(image, rc);

    // Draw it
    grp.DrawImage(&image, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
  }
}

BOOL CImgStatic::OnEraseBkgnd(CDC *pDC)
{
  if (m_bImageLoaded) {
    RECT rc;
    GetClientRect(&rc);

    // Get Gdiplus graphics object
    Graphics grp(pDC->GetSafeHdc());

    // Clear rectangle
    Color gdipColor;
    gdipColor.SetFromCOLORREF(GetSysColor(COLOR_3DFACE));

    SolidBrush brush(gdipColor);
    grp.FillRectangle(&brush, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
    grp.Flush();

    return TRUE;
  } else {
    return CStatic::OnEraseBkgnd(pDC);
  }
}

void CImgStatic::FreeStream()
{
  // Free up stream
  if (m_bImageLoaded) {
    m_bImageLoaded = false;
    // Free resources
    if (m_pStream != NULL) {
      m_pStream->Release();
      m_pStream = NULL;
    }
  }
}

void CImgStatic::ClearImage()
{
  // Get rid of image and return it back to empty CStatic control
  FreeStream();

  Graphics grp(GetDC()->GetSafeHdc());

  COLORREF clrCOLOR_3DFACE = GetSysColor(COLOR_3DFACE);
  grp.Clear(clrCOLOR_3DFACE);
}

// Used by AddEdit_Attachment & ViewAttachment - so single place for this
void CImgStatic::IssueError(int rc, HRESULT hr)
{
  CGeneralMsgBox gmb;
  CString cs_errmsg, cs_title(MAKEINTRESOURCE(IDS_IMAGE_LOAD_FAILED));
  switch (rc) {
  case 0:
    // Use hr value - but need to convert to Windows error code
    if (HRESULT_FACILITY(hr) == FACILITY_WIN32 ||
        HRESULT_FACILITY(hr) == FACILITY_WINDOWS) {
      DWORD dwlasterror = HRESULT_CODE(hr);
      SetLastError(dwlasterror);
      pws_os::IssueError((LPCWSTR)cs_title, true);
      return;
    } else {
      cs_errmsg.Format(L"Unknown HRESULT error 0x%08x.", hr);
    }
    break;
  case 1:
    cs_errmsg.LoadString(IDS_IMAGE_IMPORT_FAILED);
    break;
  case 2:
    cs_errmsg.LoadString(IDS_MEM_ALLOC_FAILED);
    break;
  case 3:
    cs_errmsg.LoadString(IDS_MEM_LOCK_FAILED);
    break;
  }

  gmb.MessageBox(cs_errmsg, cs_title, MB_OK);
}
