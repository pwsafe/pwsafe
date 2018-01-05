/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file ImgStatic.cpp
//-----------------------------------------------------------------------------

#include "StdAfx.h"

#include "ImgStatic.h"

#include "GeneralMsgBox.h"
#include "os/debug.h"
#include "resource3.h"

#pragma warning(push)
#pragma warning(disable:4458) // declaration of 'xxx' hides class member
#include <GdiPlus.h>
#pragma warning(pop)

CImgStatic::CImgStatic()
  : CStatic(), m_pStream(NULL), m_bInitDone(false), m_bImageLoaded(false),
  m_bUseScrollBars(false), m_bZooming(false), 
  m_iZoomFactor(10), m_iHPos(0), m_iVPos(0), m_gdiplusToken(0)
{
  // Initialise Gdiplus graphics
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}

CImgStatic::~CImgStatic()
{
  // Tidy up
  FreeStream();

  // Shutdown Gdiplus graphics
  Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

BEGIN_MESSAGE_MAP(CImgStatic, CStatic)
  ON_WM_ERASEBKGND()
  ON_WM_DROPFILES()
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
END_MESSAGE_MAP()

void CImgStatic::PreSubclassWindow()
{
  // Ensure that this is OWNERDRAW
  CStatic::PreSubclassWindow();
  ModifyStyle(0, SS_OWNERDRAW);

  if (m_bUseScrollBars) {
    UINT uiHHeight = GetSystemMetrics(SM_CYHSCROLL);
    UINT uiVWidth = GetSystemMetrics(SM_CXVSCROLL);
    CRect rectClient, rectH, rectV;
    GetClientRect(rectClient);
    rectH = rectClient;
    rectH.top = rectH.bottom - uiHHeight;
    rectH.right -= uiVWidth;

    rectV = rectClient;
    rectV.left = rectV.right - uiVWidth;
    rectV.bottom -= uiHHeight;

    m_HScroll.Create(SBS_HORZ | SBS_BOTTOMALIGN | WS_CHILD | WS_VISIBLE, rectH, this, IDC_IMAGE_HSCROLL);
    m_VScroll.Create(SBS_VERT | SBS_RIGHTALIGN | WS_CHILD | WS_VISIBLE, rectV, this, IDC_IMAGE_VSCROLL);

    SCROLLINFO sci;
    memset(&sci, 0, sizeof(sci));

    sci.cbSize = sizeof(SCROLLINFO);
    sci.fMask = SIF_ALL;
    sci.nMin = 0;
    sci.nMax = 100;
    sci.nPage = 20;

    m_HScroll.SetScrollInfo(&sci, TRUE);
    m_VScroll.SetScrollInfo(&sci, TRUE);

    if (m_iZoomFactor <= 10) {
      m_HScroll.EnableWindow(FALSE);
      m_HScroll.ShowWindow(SW_HIDE);
      m_VScroll.EnableWindow(FALSE);
      m_VScroll.ShowWindow(SW_HIDE);
    }
  }

  m_bInitDone = true;
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

void CImgStatic::SetZoomFactor(int iZoom)
{
  if (m_bInitDone && m_iZoomFactor != iZoom) {
    m_iZoomFactor = iZoom;

    if (m_HScroll.GetSafeHwnd() != NULL) {
      m_HScroll.EnableWindow(m_iZoomFactor <= 10 ? FALSE : TRUE);
      m_HScroll.ShowWindow(m_iZoomFactor <= 10 ? SW_HIDE : SW_SHOW);
      m_VScroll.EnableWindow(m_iZoomFactor <= 10 ? FALSE : TRUE);
      m_VScroll.ShowWindow(m_iZoomFactor <= 10 ? SW_HIDE : SW_SHOW);
    }

    m_bZooming = true;
    RedrawWindow();
    m_bZooming = false;
  }
}

void SetAspectRatio(Gdiplus::Image &image, RECT &rc)
{
  double dWidth = (rc.right - rc.left);
  double dHeight = (rc.bottom - rc.top);
  double dAspectRatio = dWidth / dHeight;
  double dImageWidth = image.GetWidth();
  double dImageHeight = image.GetHeight();
  double dImageAspectRatio = dImageWidth / dImageHeight;
  if (dImageAspectRatio > dAspectRatio) {
    double nNewHeight = (dWidth / dImageWidth*dImageHeight);
    double nCenteringFactor = (dHeight - nNewHeight) / 2;
    SetRect(&rc, 0, (int)nCenteringFactor, (int)dWidth, (int)(nNewHeight + nCenteringFactor));
  } else if (dImageAspectRatio < dAspectRatio) {
    double nNewWidth = (dHeight / dImageHeight*dImageWidth);
    double nCenteringFactor = (dWidth - nNewWidth) / 2;
    SetRect(&rc, (int)nCenteringFactor, 0, (int)(nNewWidth + nCenteringFactor), (int)(dHeight));
  }
}

void CImgStatic::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
  // Check if image data is loaded
  if (m_bImageLoaded) {
    RECT rc;
    GetClientRect(&rc);

    if (m_bUseScrollBars) {
      UINT uiHHeight = GetSystemMetrics(SM_CYHSCROLL);
      UINT uiVWidth = GetSystemMetrics(SM_CXVSCROLL);
      CRect rectH, rectV;
      rectH = rc;
      rectH.top = rectH.bottom - uiHHeight;
      rectH.right -= uiVWidth;

      rectV = rc;
      rectV.left = rectV.right - uiVWidth;
      rectV.bottom -= uiHHeight;

      m_HScroll.MoveWindow(rectH);
      m_VScroll.MoveWindow(rectV);
    }

    // Get Gdiplus graphics object
    Gdiplus::Graphics grp(lpDrawItemStruct->hDC);

    // Get image
    Gdiplus::Image image(m_pStream);

    // Adjust for aspect ratio
    SetAspectRatio(image, rc);

    if (m_iZoomFactor == 10) {
      // Draw it
      grp.DrawImage(&image, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
    } else {
      Gdiplus::Rect rcDest(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
      int srcx, srcy, srcwidth, srcheight;

      srcx = (int)(image.GetWidth() * ((float)m_iHPos / 100.0));
      srcy = (int)(image.GetHeight() * ((float)m_iVPos / 100.0));
      srcwidth = (int)(image.GetWidth() / ((float)m_iZoomFactor / 10.0));
      srcheight = (int)(image.GetHeight() / ((float)m_iZoomFactor / 10.0));

      grp.DrawImage(&image, rcDest, srcx, srcy, srcwidth, srcheight, Gdiplus::UnitPixel);
    }
  }
}

BOOL CImgStatic::OnEraseBkgnd(CDC *pDC)
{
  if (m_bZooming)
    return TRUE;
  
  if (m_bImageLoaded) {
    RECT rc;
    GetClientRect(&rc);

    // Get Gdiplus graphics object
    Gdiplus::Graphics grp(pDC->GetSafeHdc());

    // Clear rectangle
    Gdiplus::Color gdipColor;
    gdipColor.SetFromCOLORREF(GetSysColor(COLOR_3DFACE));

    Gdiplus::SolidBrush brush(gdipColor);
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

  RECT rc;
  GetClientRect(&rc);
  Gdiplus::Graphics grp(GetDC()->GetSafeHdc());

  // Clear rectangle
  Gdiplus::Color gdipColor;
  gdipColor.SetFromCOLORREF(GetSysColor(COLOR_3DFACE));

  Gdiplus::SolidBrush brush(gdipColor);
  grp.FillRectangle(&brush, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
  grp.Flush();
}

void CImgStatic::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
  if (pScrollBar->GetDlgCtrlID() == IDC_IMAGE_HSCROLL) {
    switch (nSBCode) {
      break;
    case SB_LINELEFT:
      nPos -= 1;
      break;
    case SB_LINERIGHT:
      nPos += 1;
      break;
    case SB_PAGELEFT:
      nPos -= 10;
      break;
    case SB_PAGERIGHT:
      nPos += 10;
      break;
    case SB_LEFT:
      nPos = 0;
      break;
    case SB_RIGHT:
      nPos = 100;
      break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      break;
    default:
      return;
    }

    if (nPos < 0) {
      nPos = 0;
    } else if (nPos > 100) {
      nPos = 100;
    }

    m_iHPos = nPos;

    pScrollBar->SetScrollPos(nPos, TRUE);

    m_bZooming = true;
    RedrawWindow();
    m_bZooming = false;
  }
}

void CImgStatic::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
  if (pScrollBar->GetDlgCtrlID() == IDC_IMAGE_VSCROLL) {
    switch (nSBCode) {
      break;
    case SB_LINEUP:
      nPos += 1;
      break;
    case SB_LINEDOWN:
      nPos -= 1;
      break;
    case SB_PAGEUP:
      nPos += 10;
      break;
    case SB_PAGEDOWN:
      nPos -= 10;
      break;
    case SB_TOP:
      nPos = 0;
      break;
    case SB_BOTTOM:
      nPos = 100;
      break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      break;
    default:
      return;
    }

    if (nPos < 0) {
      nPos = 0;
    } else if (nPos > 100) {
      nPos = 100;
    }

    m_iVPos = nPos;

    pScrollBar->SetScrollPos(nPos, TRUE);

    m_bZooming = true;
    RedrawWindow();
    m_bZooming = false;
  }
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
