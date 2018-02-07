/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// DDStatic.cpp : implementation file
//

#include "stdafx.h"
#include "afxole.h"

#include "DDStatic.h"
#include "DboxMain.h"
#include "ThisMfcApp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

class CStaticDropTarget : public COleDropTarget
{
public:
  CStaticDropTarget(CDDStatic *parent)
    : m_DDstatic(*parent) {}

  DROPEFFECT OnDragEnter(CWnd * /* pWnd */, COleDataObject * /* pDataObject */,
                         DWORD /* dwKeyState */, CPoint /* point */)
  {
    // No one allowed to Drag onto me!
    return DROPEFFECT_NONE;
  }

  DROPEFFECT OnDragOver(CWnd * /* pWnd */, COleDataObject * /* pDataObject */,
                        DWORD /* dwKeyState */, CPoint /* point */)
  {
    // No one allowed to Drag onto me!
    return DROPEFFECT_NONE;
  }

  void OnDragLeave(CWnd* pWnd)
  {COleDropTarget::OnDragLeave(pWnd);}

  BOOL OnDrop(CWnd * /* pWnd */, COleDataObject * /* pDataObject */,
              DROPEFFECT /* dropEffect */, CPoint /* point*/)
  {
    // No one allowed to Drop onto me!
    return FALSE;
  }

private:
  CDDStatic &m_DDstatic; // Not used as yet
};

class CStaticDropSource : public COleDropSource
{
public:
  CStaticDropSource(CDDStatic *parent)
    : m_DDstatic(*parent) {}

  virtual SCODE QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState)
  {
    // To prevent processing in multiple calls to CStaticDataSource::OnRenderGlobalData
    //  Only process the request if data has been dropped.
    SCODE sCode = COleDropSource::QueryContinueDrag(bEscapePressed, dwKeyState);
    if (sCode == DRAGDROP_S_DROP) {
      m_DDstatic.EndDrop();
    }
    return sCode;
  }

  virtual SCODE GiveFeedback(DROPEFFECT /* dropEffect */)
  {
    return DRAGDROP_S_USEDEFAULTCURSORS;
  }

private:
  CDDStatic &m_DDstatic;
};

class CStaticDataSource : public COleDataSource
{
public:
  CStaticDataSource(CDDStatic *parent, COleDropSource *ds)
    : m_DDstatic(*parent), m_pDropSource(ds) {}

  DROPEFFECT StartDragging(RECT *rClient)
  {
    DelayRenderData(CF_UNICODETEXT);
    DelayRenderData(CF_TEXT);

    DROPEFFECT dropEffect = DoDragDrop(DROPEFFECT_COPY, rClient, m_pDropSource);

    if (m_DDstatic.m_hgDataTXT != NULL) {
      GlobalUnlock(m_DDstatic.m_hgDataTXT);
      GlobalFree(m_DDstatic.m_hgDataTXT);
      m_DDstatic.m_hgDataTXT = NULL;
    }

    if (m_DDstatic.m_hgDataUTXT != NULL) {
      GlobalUnlock(m_DDstatic.m_hgDataUTXT);
      GlobalFree(m_DDstatic.m_hgDataUTXT);
      m_DDstatic.m_hgDataUTXT = NULL;
    }
    return dropEffect;
  }

  BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL *phGlobal)
  {
    return m_DDstatic.OnRenderGlobalData(lpFormatEtc, phGlobal);
  }

private:
  CDDStatic &m_DDstatic;
  COleDropSource *m_pDropSource;
};

/*
* Implementation of CDDStatic begins here
*/

CDDStatic::CDDStatic()
  : m_pci(NULL), m_hgDataTXT(NULL), m_hgDataUTXT(NULL),
  m_TimerID(0), m_bMouseInClient(false)
{
  m_pDropTarget = new CStaticDropTarget(this);
  m_pDropSource = new CStaticDropSource(this);
  m_pDataSource = new CStaticDataSource(this, m_pDropSource);
}

CDDStatic::~CDDStatic()
{
  m_pDropTarget->Revoke();

  // Don't delete m_pDataSource but first release all references and
  // this routine will delete it when the references get to 0.
  m_pDataSource->InternalRelease();

  // delete the Drop Target & Source
  delete m_pDropTarget;
  delete m_pDropSource;
}

void CDDStatic::Init(const UINT nImageID, const UINT nDisabledImageID)
{
  m_pDropTarget->Register(this);

  // Save resource IDs (Static and required image)
  m_nID = GetDlgCtrlID();

  // Load bitmap
  VERIFY(m_OKbitmap.Attach(::LoadImage(
                  ::AfxFindResourceHandle(MAKEINTRESOURCE(nImageID), RT_BITMAP),
                  MAKEINTRESOURCE(nImageID), IMAGE_BITMAP, 0, 0,
                  (LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_SHARED))));

  VERIFY(m_NOTOKbitmap.Attach(::LoadImage(
                  ::AfxFindResourceHandle(MAKEINTRESOURCE(nDisabledImageID), RT_BITMAP),
                  MAKEINTRESOURCE(nDisabledImageID), IMAGE_BITMAP, 0, 0,
                  (LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_SHARED))));

  const COLORREF crCOLOR_3DFACE = GetSysColor(COLOR_3DFACE);
  SetBitmapBackground(m_OKbitmap, crCOLOR_3DFACE);
  SetBitmapBackground(m_NOTOKbitmap, crCOLOR_3DFACE);

  // Set bitmap in Static
  m_bState = false;
  SetBitmap((HBITMAP)m_NOTOKbitmap);
}

void CDDStatic::ReInit(const UINT nImageID, const UINT nDisabledImageID)
{
  // Detach old bitmaps, attach new ones
  m_OKbitmap.Detach();
  m_NOTOKbitmap.Detach();

  VERIFY(m_OKbitmap.Attach(::LoadImage(
                  ::AfxFindResourceHandle(MAKEINTRESOURCE(nImageID), RT_BITMAP),
                  MAKEINTRESOURCE(nImageID), IMAGE_BITMAP, 0, 0,
                  (LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_SHARED))));
  
  VERIFY(m_NOTOKbitmap.Attach(::LoadImage(
                  ::AfxFindResourceHandle(MAKEINTRESOURCE(nDisabledImageID), RT_BITMAP),
                  MAKEINTRESOURCE(nDisabledImageID), IMAGE_BITMAP, 0, 0,
                  (LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_SHARED))));

  const COLORREF crCOLOR_3DFACE = GetSysColor(COLOR_3DFACE);
  SetBitmapBackground(m_OKbitmap, crCOLOR_3DFACE);
  SetBitmapBackground(m_NOTOKbitmap, crCOLOR_3DFACE);

  if (m_bState) {
    SetBitmap((HBITMAP)m_OKbitmap);
  } else {
    SetBitmap((HBITMAP)m_NOTOKbitmap);
  }
}

BEGIN_MESSAGE_MAP(CDDStatic, CStaticExtn)
  //{{AFX_MSG_MAP(CDDStatic)
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
  ON_WM_LBUTTONUP()
  ON_WM_LBUTTONDOWN()
  ON_WM_TIMER()
  ON_WM_MOUSEMOVE()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CDDStatic::SetStaticState(const bool state)
{
  if (m_bState == state) // no change
    return;

  m_bState = state;
  if (m_bState) {
    SetBitmap((HBITMAP)m_OKbitmap);
  } else {
    SetBitmap((HBITMAP)m_NOTOKbitmap);
  }
}

void CDDStatic::OnLButtonDown(UINT nFlags, CPoint point)
{
  // Do not allow D&D if the field is empty
  if (!m_bState)
    return;

  // Keep start point
  m_StartPoint = point;

  CStaticExtn::OnLButtonDown(nFlags, point);

  // Start a timer for 0.1 seconds
  m_TimerID = SetTimer(TIMER_DRAGBAR, TIMER_DRAGBAR_TIME, NULL);
}

void CDDStatic::OnLButtonUp(UINT nFlags, CPoint point)
{
  m_StartPoint.x = -100;
  m_StartPoint.y = -100;

  if (m_TimerID) {
    KillTimer(m_TimerID);
    m_TimerID = 0;
  }

  CStatic::OnLButtonUp(nFlags, point);
}

void CDDStatic::OnTimer(UINT_PTR nIDEvent)
{
  if (nIDEvent == TIMER_DRAGBAR)  {
    POINT pt;
    ::GetCursorPos(&pt);
    CRect iRect;
    GetWindowRect(iRect);
    if (!(iRect.PtInRect(pt))) {
      KillTimer(nIDEvent);
      m_TimerID = 0;
    }
  }

  CStatic::OnTimer(nIDEvent);
}

void CDDStatic::OnMouseMove(UINT nFlags, CPoint point)
{
  if (!m_bMouseInClient) {
    m_bMouseInClient = true;

    m_pci = app.GetMainDlg()->GetLastSelected();
    TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, CStatic::GetSafeHwnd(), 0};
    TrackMouseEvent(&tme);
  }

  if (m_TimerID == 0)
    goto StandardProcessing;

  // Check if we really moved enough
  int iX = m_StartPoint.x - point.x;
  int iY = m_StartPoint.y - point.y;
  if ((iX * iX + iY * iY) > MINIMUM_MOVE_SQUARE) {
    m_pci = app.GetMainDlg()->GetLastSelected();
    if (m_pci == NULL) {
      m_groupname = app.GetMainDlg()->GetGroupName();
    } else {
      m_groupname = L"";
    }

    // Get client rectangle
    RECT rClient;
    GetClientRect(&rClient);

    // Copy data to Clipboard if Control pressed
    if (nFlags & MK_CONTROL)
      SendToClipboard();

    // Start dragging
    m_bDropped = false;
    DROPEFFECT de = m_pDataSource->StartDragging(&rClient);

    if (de == DROPEFFECT_NONE) {
      // Do cleanup - otherwise this is the responsibility of the recipient!
      if (m_hgDataTXT != NULL) {
        LPVOID lpData = GlobalLock(m_hgDataTXT);
        SIZE_T memsize = GlobalSize(m_hgDataTXT);
        if (lpData != NULL) {
          trashMemory(lpData, memsize);
          GlobalUnlock(m_hgDataTXT);
        }
        GlobalFree(m_hgDataTXT);
        m_hgDataTXT = NULL;
      }
      if (m_hgDataUTXT != NULL) {
        LPVOID lpData = GlobalLock(m_hgDataUTXT);
        SIZE_T memsize = GlobalSize(m_hgDataUTXT);
        if (lpData != NULL) {
          trashMemory(lpData, memsize);
          GlobalUnlock(m_hgDataUTXT);
        }
        GlobalFree(m_hgDataUTXT);
        m_hgDataUTXT = NULL;
      }
      pws_os::Trace(L"m_pDataSource->StartDragging() failed\n");
    } else {
      while (ShowCursor(TRUE) < 0)
        ;
    }

    LPARAM lparam = (LPARAM(point.y) << 16) | LPARAM(point.x);
    ::SendMessage(GetActiveWindow()->GetSafeHwnd(), WM_LBUTTONUP, 0, lparam);
  }

StandardProcessing:
  CStatic::OnMouseMove(nFlags, point);
}

LRESULT CDDStatic::OnMouseLeave(WPARAM, LPARAM)
{
  m_bMouseInClient = false;
  return 0L;
}

void CDDStatic::SetBitmapBackground(CBitmap &bm, const COLORREF newbkgrndColour)
{
  // Get how many pixels in the bitmap
  BITMAP bmInfo;
  bm.GetBitmap(&bmInfo);

  const UINT numPixels(bmInfo.bmHeight * bmInfo.bmWidth);

  // get a pointer to the pixels
  DIBSECTION ds;
  VERIFY(bm.GetObject(sizeof(DIBSECTION), &ds) == sizeof(DIBSECTION));

  RGBTRIPLE *pixels = reinterpret_cast<RGBTRIPLE*>(ds.dsBm.bmBits);
  ASSERT(pixels != NULL);

  const RGBTRIPLE newbkgrndColourRGB = {GetBValue(newbkgrndColour),
                                        GetGValue(newbkgrndColour),
                                        GetRValue(newbkgrndColour)};

  for (UINT i = 0; i < numPixels; ++i) {
    if (pixels[i].rgbtBlue  == 192 &&
        pixels[i].rgbtGreen == 192 &&
        pixels[i].rgbtRed   == 192) {
      pixels[i] = newbkgrndColourRGB;
    }
  }
}

void CDDStatic::SendToClipboard()
{
  if (m_nID == IDC_STATIC_DRAGAUTO)
    return;

  if (m_pci == NULL) {
    if (!m_groupname.empty()) {
      app.GetMainDlg()->SetClipboardData(m_groupname);
      app.GetMainDlg()->UpdateLastClipboardAction(CItemData::GROUP);
    }
    return;
  }

  const CItemData *pci(m_pci);

  // Handle shortcut or alias
  if ((m_nID == IDC_STATIC_DRAGPASSWORD && pci->IsAlias()) ||
      (pci->IsShortcut() && (m_nID != IDC_STATIC_DRAGGROUP &&
                             m_nID != IDC_STATIC_DRAGTITLE &&
                             m_nID != IDC_STATIC_DRAGUSER))) {
    pci = app.GetMainDlg()->GetBaseEntry(pci);
  }

  StringX cs_dragdata = GetData(pci);
  app.GetMainDlg()->SetClipboardData(cs_dragdata);
}

BOOL CDDStatic::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
  if (lpFormatEtc->cfFormat != CF_UNICODETEXT &&
      lpFormatEtc->cfFormat != CF_TEXT)
    return FALSE;

  if (m_hgDataTXT != NULL) {
    GlobalUnlock(m_hgDataTXT);
    GlobalFree(m_hgDataTXT);
    m_hgDataTXT = NULL;
  }

  if (m_hgDataUTXT != NULL) {
    GlobalUnlock(m_hgDataUTXT);
    GlobalFree(m_hgDataUTXT);
    m_hgDataUTXT = NULL;
  }

  StringX cs_dragdata;
  if (m_pci == NULL) {
    if (m_groupname.empty()) {
      return FALSE;
    } else {
      cs_dragdata = m_groupname;
    }
  } else { // m_pci != NULL
    const CItemData *pci(m_pci);

    // Handle shortcut or alias
    if ((m_nID == IDC_STATIC_DRAGPASSWORD && pci->IsAlias()) ||
        (pci->IsShortcut() && (m_nID != IDC_STATIC_DRAGGROUP &&
                               m_nID != IDC_STATIC_DRAGTITLE &&
                               m_nID != IDC_STATIC_DRAGUSER))) {
      pci = app.GetMainDlg()->GetBaseEntry(pci);
    }
    cs_dragdata = GetData(pci);
    if (cs_dragdata.empty() && m_nID != IDC_STATIC_DRAGAUTO)
      return FALSE;
  }

  const size_t ilen = cs_dragdata.length();
  if (ilen == 0 && m_nID != IDC_STATIC_DRAGAUTO) {
    // Nothing to do - why were we even called???
    return FALSE;
  }

  DWORD dwBufLen;
  LPSTR lpszA(NULL);
  LPWSTR lpszW(NULL);

  if (lpFormatEtc->cfFormat == CF_UNICODETEXT) {
    // So is requested data!
    dwBufLen = (DWORD)((ilen + 1) * sizeof(wchar_t));
    lpszW = new WCHAR[ilen + 1];
    if (ilen == 0) {
      lpszW[ilen] = L'\0';
    } else {
      (void) wcsncpy_s(lpszW, ilen + 1, cs_dragdata.c_str(), ilen);
    }
  } else {
    // They want it in ASCII - use lpszW temporarily
    if (ilen == 0) {
      dwBufLen = 1;
      lpszA = new char[dwBufLen];
      lpszA = '\0';
    } else {
      lpszW = const_cast<LPWSTR>(cs_dragdata.c_str());
      dwBufLen = WideCharToMultiByte(CP_ACP, 0, lpszW, -1, NULL, 0, NULL, NULL);
      ASSERT(dwBufLen != 0);
      lpszA = new char[dwBufLen];
      WideCharToMultiByte(CP_ACP, 0, lpszW, -1, lpszA, dwBufLen, NULL, NULL);
      lpszW = NULL;
    }
  }

  LPVOID lpData(NULL);
  LPVOID lpDataBuffer;
  HGLOBAL *phgData;
  if (lpFormatEtc->cfFormat == CF_UNICODETEXT) {
    lpDataBuffer = (LPVOID)lpszW;
    phgData = &m_hgDataUTXT;
  } else {
    lpDataBuffer = (LPVOID)lpszA;
    phgData = &m_hgDataTXT;
  }

  BOOL retval(FALSE);
  if (*phGlobal == NULL) {
    *phgData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwBufLen);
    ASSERT(*phgData != NULL);
    if (*phgData == NULL)
      goto bad_return;

    lpData = GlobalLock(*phgData);
    ASSERT(lpData != NULL);
    if (lpData == NULL)
      goto bad_return;

    // Copy data
    memcpy(lpData, lpDataBuffer, dwBufLen);
    *phGlobal = *phgData;
    retval = TRUE;
  } else {
    pws_os::Trace(L"CDDStatic::OnRenderGlobalData - *phGlobal NOT NULL!\n");
    SIZE_T inSize = GlobalSize(*phGlobal);
    SIZE_T ourSize = GlobalSize(*phgData);
    if (inSize < ourSize) {
      // Pre-allocated space too small.  Not allowed to increase it - FAIL
      pws_os::Trace(L"CDDStatic::OnRenderGlobalData - NOT enough room - FAIL\n");
    } else {
      // Enough room - copy our data into supplied area
      LPVOID pInGlobalLock = GlobalLock(*phGlobal);
      ASSERT(pInGlobalLock != NULL);
      if (pInGlobalLock == NULL)
        goto bad_return;

      memcpy(pInGlobalLock, lpDataBuffer, ourSize);
      GlobalUnlock(*phGlobal);
      retval = TRUE;
    }
  }

bad_return:
  // Finished with buffer - trash it
  trashMemory(lpDataBuffer, dwBufLen);
  // Free the strings (only one is actually in use)
  delete[] lpszA;
  delete[] lpszW;
  // Since lpDataBuffer pointed to one of the above - just zero the pointer
  lpDataBuffer = NULL;

  // If retval == TRUE, recipient is responsible for freeing the global memory
  // if D&D succeeds (see after StartDragging in OnMouseMove)
  if (retval == FALSE) {
    pws_os::Trace(L"CDDStatic::OnRenderGlobalData - returning FALSE!\n");
    if (lpData != NULL) {
      GlobalFree(*phgData);
      *phgData = NULL;
    }
  } else {
    /*
    pws_os::Trace(L"CDDStatic::OnRenderGlobalData - D&D Data:");
    if (lpFormatEtc->cfFormat == CF_UNICODETEXT) {
      pws_os::Trace(L"\"%ls\"\n", (LPWSTR)lpData);  // data is Unicode
    } else {
      pws_os::Trace(L"\"%hs\"\n", (LPSTR)lpData);   // data is NOT Unicode
    }
    */
  }

  // Unlock our buffer
  if (lpData != NULL)
    GlobalUnlock(*phgData);

  return retval;
}

StringX CDDStatic::GetData(const CItemData *pci)
{
  StringX cs_dragdata(L"");
  StringX::size_type ipos;

  switch (m_nID) {
    case IDC_STATIC_DRAGGROUP:
      cs_dragdata = pci->GetGroup();
      if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
        StringX::size_type index;
        index = cs_dragdata.rfind(L".");
        if (index != StringX::npos) {
          cs_dragdata = cs_dragdata.substr(index + 1);
        }
      }
      break;
    case IDC_STATIC_DRAGTITLE:
      cs_dragdata = pci->GetTitle();
      break;
    case IDC_STATIC_DRAGPASSWORD:
      cs_dragdata = pci->GetPassword();
      break;
    case IDC_STATIC_DRAGUSER:
      cs_dragdata = pci->GetUser();
      break;
    case IDC_STATIC_DRAGNOTES:
      cs_dragdata = pci->GetNotes();
      break;
    case IDC_STATIC_DRAGURL:
      cs_dragdata = pci->GetURL();
      ipos = cs_dragdata.find(L"[alt]");
      if (ipos != StringX::npos)
        cs_dragdata.replace(ipos, 5, L"");
      ipos = cs_dragdata.find(L"[ssh]");
      if (ipos != StringX::npos)
        cs_dragdata.replace(ipos, 5, L"");
      ipos = cs_dragdata.find(L"{alt}");
      if (ipos != StringX::npos)
        cs_dragdata.replace(ipos, 5, L"");
      break;
    case IDC_STATIC_DRAGEMAIL:
      cs_dragdata = pci->GetEmail();
      break;
    case IDC_STATIC_DRAGAUTO:
      cs_dragdata = L"";
      break;
    default:
      break;
  }
  return cs_dragdata;
}

void CDDStatic::EndDrop()
{
  m_bDropped = true;

  if (m_nID == IDC_STATIC_DRAGAUTO) {
    if (m_pci != NULL) {
      // Get handle of window where the user wants Autotype to start
      // Then make it the foreground window
      POINT screenpoint;
      ::GetCursorPos(&screenpoint);
      HWND hwndFoundWindow = ::WindowFromPoint(screenpoint);
      if (hwndFoundWindow != NULL) {
        ::SetForegroundWindow(hwndFoundWindow);
      }
      app.GetMainDlg()->PostMessage(PWS_MSG_DRAGAUTOTYPE, (WPARAM)m_pci);
    }
  }
}
