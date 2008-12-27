/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

class CStaticDropTarget : public COleDropTarget
{
public:
  CStaticDropTarget(CDDStatic *parent)
    : m_DDstatic(*parent) {}

  DROPEFFECT OnDragEnter(CWnd* /* pWnd */, COleDataObject* /* pDataObject */,
                         DWORD /* dwKeyState */, CPoint /* point */)
  {// No one allowed to Drag onto me!
   return DROPEFFECT_NONE;}

  DROPEFFECT OnDragOver(CWnd* /* pWnd */, COleDataObject* /* pDataObject */,
                        DWORD /* dwKeyState */, CPoint /* point */)
  {// No one allowed to Drag onto me!
   return DROPEFFECT_NONE;}

  void OnDragLeave(CWnd* pWnd)
  {COleDropTarget::OnDragLeave(pWnd);}

  BOOL OnDrop(CWnd* /* pWnd */, COleDataObject* /* pDataObject */,
              DROPEFFECT /* dropEffect */, CPoint /* point*/)
  {// No one allowed to Drop onto me!
   return FALSE;}

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
      TRACE(_T("CStaticDropSource::QueryContinueDrag - dropped\n"));
      m_DDstatic.EndDrop();
    }
    return sCode;
  }

  virtual SCODE GiveFeedback(DROPEFFECT /* dropEffect */)
  {return DRAGDROP_S_USEDEFAULTCURSORS;}

private:
  CDDStatic &m_DDstatic;
};

class CStaticDataSource : public COleDataSource
{
public:
  CStaticDataSource(CDDStatic *parent, COleDropSource *ds)
    : m_DDstatic(*parent), m_pDropSource(ds) {}

  DROPEFFECT StartDragging(RECT* rClient)
  {
    TRACE(_T("CStaticDataSource::StartDragging\n"));

    DelayRenderData(CF_UNICODETEXT);
    DelayRenderData(CF_TEXT);

    TRACE(_T("CStaticDataSource::StartDragging - calling DoDragDrop\n"));
    DROPEFFECT dropEffect = DoDragDrop(DROPEFFECT_COPY, rClient, m_pDropSource);

    TRACE(_T("CStaticDataSource::StartDragging - returned from DoDragDrop, dropEffect=%d\n"),
      dropEffect);

    if (m_DDstatic.m_hgDataTXT != NULL) {
      TRACE(_T("CStaticDataSource::StartDragging - Unlock/Free m_hgDataTXT\n"));
      GlobalUnlock(m_DDstatic.m_hgDataTXT);
      GlobalFree(m_DDstatic.m_hgDataTXT);
      m_DDstatic.m_hgDataTXT = NULL;
    }
    if (m_DDstatic.m_hgDataUTXT != NULL) {
      TRACE(_T("CStaticDataSource::StartDragging - Unlock/Free m_hgDataUTXT\n"));
      GlobalUnlock(m_DDstatic.m_hgDataUTXT);
      GlobalFree(m_DDstatic.m_hgDataUTXT);
      m_DDstatic.m_hgDataUTXT = NULL;
    }
    return dropEffect;
  }

  BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
  {return m_DDstatic.OnRenderGlobalData(lpFormatEtc, phGlobal);}

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

  // see comment in constructor re these member variables
  delete m_pDropTarget;
  delete m_pDropSource;
  delete m_pDataSource;
}

void CDDStatic::Init(const UINT nImageID, const UINT nDisabledImageID)
{
  m_pDropTarget->Register(this);

  // Save resource IDs (Static and required image)
  m_nID = GetDlgCtrlID();

  // Save pointer to DboxMain
  m_pDbx = static_cast<DboxMain *>(GetParent());

  // Load bitmap
  BOOL brc;
  brc = m_OKbitmap.Attach(::LoadImage(
                  ::AfxFindResourceHandle(MAKEINTRESOURCE(nImageID), RT_BITMAP),
                  MAKEINTRESOURCE(nImageID), IMAGE_BITMAP, 0, 0,
                  (LR_DEFAULTSIZE | LR_CREATEDIBSECTION)));
  ASSERT(brc);


  brc = m_NOTOKbitmap.Attach(::LoadImage(
                  ::AfxFindResourceHandle(MAKEINTRESOURCE(nDisabledImageID), RT_BITMAP),
                  MAKEINTRESOURCE(nDisabledImageID), IMAGE_BITMAP, 0, 0,
                  (LR_DEFAULTSIZE | LR_CREATEDIBSECTION)));
  ASSERT(brc);

  const COLORREF crCOLOR_3DFACE = GetSysColor(COLOR_3DFACE);
  SetBitmapBackground(m_OKbitmap, crCOLOR_3DFACE);
  SetBitmapBackground(m_NOTOKbitmap, crCOLOR_3DFACE);

  // Set bitmap in Static
  m_bState = false;
  SetBitmap((HBITMAP)m_NOTOKbitmap);
}

BEGIN_MESSAGE_MAP(CDDStatic, CStaticExtn)
  //{{AFX_MSG_MAP(CDDStatic)
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
  ON_WM_LBUTTONUP()
  ON_WM_LBUTTONDOWN()
  ON_WM_TIMER()
  ON_WM_MOUSEMOVE()
  ON_WM_DESTROY()
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

void CDDStatic::OnDestroy()
{
  m_OKbitmap.DeleteObject();
  m_NOTOKbitmap.DeleteObject();

  CStaticExtn::OnDestroy();
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

void CDDStatic::OnTimer(UINT nIDEvent)
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

    m_pci = m_pDbx->GetLastSelected();
    TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, CStatic::GetSafeHwnd(), 0};
    TrackMouseEvent(&tme);
  }

  if (m_TimerID == 0 || m_pci == NULL)
    goto StandardProcessing;

  // Check if we really moved enough
  int iX = m_StartPoint.x - point.x;
  int iY = m_StartPoint.y - point.y;
  if ((iX * iX + iY * iY) > MINIMUM_MOVE_SQUARE) {
    m_pci = m_pDbx->GetLastSelected();

    // Get client rectangle
    RECT rClient;
    GetClientRect(&rClient);

    // Copy data to Clipboard anyway
    SendToClipboard();

    // Start dragging
    m_bDropped = false;
    TRACE(_T("CDDStatic::OnMouseMove: call m_pDataSource->StartDragging\n"));
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
      TRACE(_T("m_pDataSource->StartDragging() failed\n"));
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
  if (m_pDbx == NULL || m_pci == NULL)
    return;

  CItemData *pci(m_pci);

  const CItemData::EntryType entrytype = pci->GetEntryType();
  if (m_nID == IDC_STATIC_DRAGPASSWORD && entrytype == CItemData::ET_ALIAS) {
    // This is an alias
    uuid_array_t entry_uuid, base_uuid;
    pci->GetUUID(entry_uuid);
    m_pDbx->GetAliasBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_pDbx->Find(base_uuid);
    if (iter != m_pDbx->End()) {
      pci = &(iter->second);
    }
  }

  if (entrytype == CItemData::ET_SHORTCUT) {
    // This is an shortcut
    if (m_nID != IDC_STATIC_DRAGGROUP &&
        m_nID != IDC_STATIC_DRAGTITLE &&
        m_nID != IDC_STATIC_DRAGUSER) {
      uuid_array_t entry_uuid, base_uuid;
      pci->GetUUID(entry_uuid);
      m_pDbx->GetShortcutBaseUUID(entry_uuid, base_uuid);

      ItemListIter iter = m_pDbx->Find(base_uuid);
      if (iter != m_pDbx->End()) {
        pci = &(iter->second);
      }
    }
  }

  StringX cs_dragdata;
  StringX::size_type ipos;
  switch (m_nID) {
    case IDC_STATIC_DRAGGROUP:
      cs_dragdata = pci->GetGroup();
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
      ipos = cs_dragdata.find(_T("[alt]"));
      if (ipos != StringX::npos)
        cs_dragdata.replace(ipos, 5, _T(""));
      ipos = cs_dragdata.find(_T("[ssh]"));
      if (ipos != StringX::npos)
        cs_dragdata.replace(ipos, 5, _T(""));
      ipos = cs_dragdata.find(_T("{alt}"));
      if (ipos != StringX::npos)
        cs_dragdata.replace(ipos, 5, _T(""));
      break;
    default:
      return;
  }

  m_pDbx->SetClipboardData(cs_dragdata);
}

BOOL CDDStatic::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
  TRACE(_T("CDDStatic::OnRenderGlobalData: %s; ci == %p\n"),
          lpFormatEtc->cfFormat == CF_UNICODETEXT ? _T("CF_UNICODETEXT") : _T("CF_TEXT"),
          m_pci);

  if (lpFormatEtc->cfFormat != CF_UNICODETEXT &&
      lpFormatEtc->cfFormat != CF_TEXT)
    return FALSE;

  if (m_hgDataTXT != NULL) {
    TRACE(_T("CDDStatic::OnRenderGlobalData - Unlock/Free m_hgDataTXT\n"));
    GlobalUnlock(m_hgDataTXT);
    GlobalFree(m_hgDataTXT);
    m_hgDataTXT = NULL;
  }
  if (m_hgDataUTXT != NULL) {
    TRACE(_T("CDDStatic::OnRenderGlobalData - Unlock/Free m_hgDataUTXT\n"));
    GlobalUnlock(m_hgDataUTXT);
    GlobalFree(m_hgDataUTXT);
    m_hgDataUTXT = NULL;
  }

  if (m_pci == NULL) {
    TRACE(_T("CDDStatic::OnRenderGlobalData - mpci == NULL\n"));
    return FALSE;
  }

  CItemData *pci(m_pci);

  const CItemData::EntryType entrytype = pci->GetEntryType();
  if (m_nID == IDC_STATIC_DRAGPASSWORD && entrytype == CItemData::ET_ALIAS) {
    // This is an alias
    uuid_array_t entry_uuid, base_uuid;
    pci->GetUUID(entry_uuid);
    m_pDbx->GetAliasBaseUUID(entry_uuid, base_uuid);

    ItemListIter iter = m_pDbx->Find(base_uuid);
    if (iter != m_pDbx->End()) {
      pci = &(iter->second);
    }
  }

  if (entrytype == CItemData::ET_SHORTCUT) {
    // This is an shortcut
    if (m_nID != IDC_STATIC_DRAGGROUP &&
        m_nID != IDC_STATIC_DRAGTITLE &&
        m_nID != IDC_STATIC_DRAGUSER) {
      uuid_array_t entry_uuid, base_uuid;
      pci->GetUUID(entry_uuid);
      m_pDbx->GetShortcutBaseUUID(entry_uuid, base_uuid);

      ItemListIter iter = m_pDbx->Find(base_uuid);
      if (iter != m_pDbx->End()) {
        pci = &(iter->second);
      }
    }
  }

  StringX cs_dragdata;
  StringX::size_type ipos;
  switch (m_nID) {
    case IDC_STATIC_DRAGGROUP:
      cs_dragdata = pci->GetGroup();
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
      ipos = cs_dragdata.find(_T("[alt]"));
      if (ipos != StringX::npos)
        cs_dragdata.replace(ipos, 5, _T(""));
      ipos = cs_dragdata.find(_T("[ssh]"));
      if (ipos != StringX::npos)
        cs_dragdata.replace(ipos, 5, _T(""));
      ipos = cs_dragdata.find(_T("{alt}"));
      if (ipos != StringX::npos)
        cs_dragdata.replace(ipos, 5, _T(""));
      break;
    default:
      return FALSE;
  }

  const int ilen = cs_dragdata.length();
  if (ilen == 0) {
    // Nothing to do - why were we even called???
    return FALSE;
  }

  DWORD dwBufLen;
  LPSTR lpszA(NULL);
  LPWSTR lpszW(NULL);

#ifdef UNICODE
  // We are Unicode!
  if (lpFormatEtc->cfFormat == CF_UNICODETEXT) {
    // So is requested data!
    dwBufLen = (ilen + 1) * sizeof(wchar_t);
    lpszW = new WCHAR[ilen + 1];
    TRACE(_T("lpszW allocated %p, size %d\n"), lpszW, dwBufLen);
#if (_MSC_VER >= 1400)
    (void) wcsncpy_s(lpszW, ilen + 1, cs_dragdata.c_str(), ilen);
#else
    (void)wcsncpy(lpszW, cs_dragdata, ilen);
    lpszW[ilen] = L'\0';
#endif
  } else {
    // They want it in ASCII - use lpszW temporarily
    lpszW = const_cast<LPWSTR>(cs_dragdata.c_str());
    dwBufLen = WideCharToMultiByte(CP_ACP, 0, lpszW, -1, NULL, 0, NULL, NULL);
    ASSERT(dwBufLen != 0);
    lpszA = new char[dwBufLen];
    TRACE(_T("lpszA allocated %p, size %d\n"), lpszA, dwBufLen);
    WideCharToMultiByte(CP_ACP, 0, lpszW, -1, lpszA, dwBufLen, NULL, NULL);
    lpszW = NULL;
  }
#else
  // We are Ascii!
  if (lpFormatEtc->cfFormat == CF_TEXT) {
    // So is requested data!
    dwBufLen = ilen + 1;
    lpszA = new char[ilen + 1];
    TRACE(_T("lpszA allocated %p, size %d\n"), lpszA, dwBufLen);
#if (_MSC_VER >= 1400)
    (void) strncpy_s(lpszA, ilen + 1, cs_dragdata.c_str(), ilen);
#else
    (void)strncpy(lpszA, cs_dragdata.c_str(), ilen);
    lpszA[ilen] = '\0';
#endif
  } else {
    // They want it in UNICODE - use lpszA temporarily
    lpszA = const_cast<LPSTR>(cs_dragdata.c_str());
    dwBufLen = MultiByteToWideChar(CP_ACP, 0, lpszA, -1, NULL, NULL);
    lpszW = new WCHAR[dwBufLen];
    TRACE(_T("lpszW allocated %p, size %d\n"), lpszW, dwBufLen);
    MultiByteToWideChar(CP_ACP, 0, lpszA, -1, lpszW, dwBufLen);
    lpszA = NULL;
  }
#endif

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
    TRACE(_T("CDDStatic::OnRenderGlobalData - Alloc global memory\n"));
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
    TRACE(_T("CDDStatic::OnRenderGlobalData - *phGlobal NOT NULL!\n"));
    SIZE_T inSize = GlobalSize(*phGlobal);
    SIZE_T ourSize = GlobalSize(*phgData);
    if (inSize < ourSize) {
      // Pre-allocated space too small.  Not allowed to increase it - FAIL
      TRACE(_T("CDDStatic::OnRenderGlobalData - NOT enough room - FAIL\n"));
    } else {
      // Enough room - copy our data into supplied area
      TRACE(_T("CDDStatic::OnRenderGlobalData - enough room - copy our data\n"));
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
  TRACE(_T("lpszA freed %p\n"), lpszA);
  delete[] lpszA;
  TRACE(_T("lpszW freed %p\n"), lpszW);
  delete[] lpszW;
  // Since lpDataBuffer pointed to one of the above - just zero the pointer
  lpDataBuffer = NULL;

  // If retval == TRUE, recipient is responsible for freeing the global memory
  // if D&D succeeds (see after StartDragging in OnMouseMove)
  if (retval == FALSE) {
    TRACE(_T("CDDStatic::OnRenderGlobalData - returning FALSE!\n"));
    if (lpData != NULL) {
      GlobalFree(*phgData);
      *phgData = NULL;
    }
  } else {
    TRACE(_T("CDDStatic::OnRenderGlobalData - D&D Data:"));
    if (lpFormatEtc->cfFormat == CF_UNICODETEXT) {
#ifdef UNICODE
      TRACE(_T("\"%s\"\n"), (LPWSTR)lpData);  // we are Unicode, data is Unicode
#else
      TRACE(_T("\"%S\"\n"), (LPSTR)lpData);   // we are NOT Unicode, data is Unicode
#endif
    } else {
#ifdef UNICODE
      TRACE(_T("\"%S\"\n"), (LPSTR)lpData);  // we are Unicode, data is NOT Unicode
#else
      TRACE(_T("\"%s\"\n"), (LPWSTR)lpData);  // we are NOT Unicode, data is NOT Unicode
#endif
    }
  }
  // Unlock our buffer
  if (lpData != NULL)
    GlobalUnlock(*phgData);

  return retval;
}
