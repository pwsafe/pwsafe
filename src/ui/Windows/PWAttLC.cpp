/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "PWAttLC.h"
#include "AttProperties.h"
#include "DboxMain.h"
#include "AddDescription.h"
#include "NumUtilities.h"

#include "resource.h"
#include "resource3.h"
#include "corelib/StringX.h"

#include <algorithm>
#include <iomanip>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPWAttLC::CPWAttLC()
  : m_pParent(NULL), m_fwidth(-1), m_lwidth(-1), m_rowheight(-1),
   m_numattachments(0), m_iItem(-1), m_pCheckImageList(NULL),
   m_pToolTipCtrl(NULL), m_pchTip(NULL), m_pwchTip(NULL), m_pbColTT(NULL)
{
  m_crWindowText = ::GetSysColor(COLOR_WINDOWTEXT);
}

CPWAttLC::~CPWAttLC()
{
  // Do not delete filters, as they need to passed back to the caller

  if (m_pCheckImageList != NULL) {
    m_pCheckImageList->DeleteImageList();
    delete m_pCheckImageList;
  }

  m_menuManager.Cleanup();

  delete m_pToolTipCtrl;
  delete [] m_pbColTT;

  delete m_pchTip;
  delete m_pwchTip;
}

BEGIN_MESSAGE_MAP(CPWAttLC, CListCtrl)
  //{{AFX_MSG_MAP(CPWAttLC)
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  ON_NOTIFY(HDN_ITEMCHANGED, 0, OnHdnItemchanged)
  ON_NOTIFY(HDN_ENDDRAG, 0, OnHdnEndDrag)
  ON_NOTIFY(HDN_ENDTRACK, 0, OnHdnEndTrack)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPWAttLC::Init(const LCType lct, CWnd *pWnd, const bool bReadOnly)
{
  m_lct = lct;
  m_pParent = GetParent();
  m_pWnd = pWnd;
  m_bReadOnly = bReadOnly;
  EnableToolTips(TRUE);

  m_menuManager.Install(this);
  m_menuManager.SetImageList(&((DboxMain *)m_pWnd)->m_MainToolBar);
  m_menuManager.SetMapping(&((DboxMain *)m_pWnd)->m_MainToolBar);

  const COLORREF crTransparent = RGB(192, 192, 192);
  CBitmap bitmap;
  BITMAP bm;
  bitmap.LoadBitmap(IDB_UNCHECKEDA);
  bitmap.GetBitmap(&bm); // should be 13 x 13

  m_pCheckImageList = new CImageList;
  BOOL status = m_pCheckImageList->Create(bm.bmWidth, bm.bmHeight,
                                     ILC_MASK | ILC_COLOR, 4, 0);
  ASSERT(status != 0);

  m_pCheckImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_CHECKED);
  m_pCheckImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_UNCHECKEDA_D);
  m_pCheckImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_CHECKED_D);
  m_pCheckImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();

  // Set CHeaderDtrl control ID
  m_pHeaderCtrl = GetHeaderCtrl();
  UINT uiID(0);
  switch (m_lct) {
    case NEW:
      uiID = IDC_NEW_ATTLC_HEADER;
      break;
    case EXISTING:
      uiID = IDC_EXISTING_ATTLC_HEADER;
      break;
    case EXTRACT:
      uiID = IDC_EXTRACT_ATTLC_HEADER;
      break;
    case VIEW:
      uiID = IDC_VIEW_ATTLC_HEADER;
      break;
  }
  m_pHeaderCtrl->SetDlgCtrlID(uiID);
  CString cs_text;

  int ncol = 0;
  if (m_lct == EXISTING) {
    cs_text.LoadString(IDS_DEL);
    InsertColumn(ncol, cs_text); ncol++;
  } else
  if (m_lct == VIEW) {
    cs_text.LoadStringW(IDS_GROUP);
    InsertColumn(ncol, cs_text); ncol++;
    cs_text.LoadStringW(IDS_TITLE);
    InsertColumn(ncol, cs_text); ncol++;
    cs_text.LoadStringW(IDS_USERNAME);
    InsertColumn(ncol, cs_text); ncol++;
  }
  cs_text.LoadString(IDS_ERASE);
  InsertColumn(ncol, cs_text); ncol++;
  cs_text.LoadString(IDS_REMOVEABLE);
  InsertColumn(ncol, cs_text); ncol++;
  cs_text.LoadStringW(IDS_FILENAME);
  InsertColumn(ncol, cs_text); ncol++;
  cs_text.LoadStringW(IDS_DESCRIPTION);
  InsertColumn(ncol, cs_text); ncol++;
  cs_text.LoadStringW(IDS_PATH);
  InsertColumn(ncol, cs_text); ncol++;

  int numcols = GetHeaderCtrl()->GetItemCount();
  m_pbColTT = new bool[numcols];
  for (int i = 0; i < numcols; i++) {
    m_pbColTT[i] = false;
    SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
  }

  // Make button headers centered (and so their contents in the ListCtrl)
  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT;
  lvc.fmt = LVCFMT_CENTER;
  switch (m_lct) {
    case NEW:
      SetColumn(0, &lvc);
      SetColumn(1, &lvc);
      break;
    case EXTRACT:
      SetColumn(0, &lvc);
      SetColumn(1, &lvc);
      break;
    case EXISTING:
      SetColumn(0, &lvc);
      SetColumn(1, &lvc);
      SetColumn(2, &lvc);
      break;
    case VIEW:
      SetColumn(3, &lvc);
      SetColumn(4, &lvc);
      break;
  }

  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, TTS_BALLOON | TTS_NOPREFIX)) {
    pws_os::Trace(L"Unable To create CPWAttLC ListCtrl ToolTip\n");
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
  } else {
    m_pToolTipCtrl->SetMaxTipWidth(300);

    RECT rcTT;
    CString csTT;
    switch (m_lct) {
      case EXISTING:
        csTT.LoadString(IDS_ATT_TOOLTIP0);
        m_pbColTT[0] = true;
        m_pHeaderCtrl->GetItemRect(0, &rcTT);
        m_pToolTipCtrl->AddTool(m_pHeaderCtrl, csTT, &rcTT, 1);

        csTT.LoadString(IDS_ATT_TOOLTIP1);
        m_pbColTT[1] = true;
        m_pHeaderCtrl->GetItemRect(1, &rcTT);
        m_pToolTipCtrl->AddTool(m_pHeaderCtrl, csTT, &rcTT, 2);

        csTT.LoadString(IDS_ATT_TOOLTIP2);
        m_pbColTT[2] = true;
        m_pHeaderCtrl->GetItemRect(2, &rcTT);
        m_pToolTipCtrl->AddTool(m_pHeaderCtrl, csTT, &rcTT, 3);
        break;
      case NEW:
        csTT.LoadString(IDS_ATT_TOOLTIP1);
        m_pbColTT[0] = true;
        m_pHeaderCtrl->GetItemRect(0, &rcTT);
        m_pToolTipCtrl->AddTool(m_pHeaderCtrl, csTT, &rcTT, 1);

        csTT.LoadString(IDS_ATT_TOOLTIP2);
        m_pbColTT[1] = true;
        m_pHeaderCtrl->GetItemRect(1, &rcTT);
        m_pToolTipCtrl->AddTool(m_pHeaderCtrl, csTT, &rcTT, 2);
        break;
      case EXTRACT:
        csTT.LoadString(IDS_ATT_TOOLTIP1);
        m_pbColTT[0] = true;
        m_pHeaderCtrl->GetItemRect(0, &rcTT);
        m_pToolTipCtrl->AddTool(m_pHeaderCtrl, csTT, &rcTT, 1);

        csTT.LoadString(IDS_ATT_TOOLTIP2);
        m_pbColTT[1] = true;
        m_pHeaderCtrl->GetItemRect(1, &rcTT);
        m_pToolTipCtrl->AddTool(m_pHeaderCtrl, csTT, &rcTT, 2);
        break;
      case VIEW:
        csTT.LoadString(IDS_ATT_TOOLTIP1);
        m_pbColTT[0] = true;
        m_pHeaderCtrl->GetItemRect(3, &rcTT);
        m_pToolTipCtrl->AddTool(m_pHeaderCtrl, csTT, &rcTT, 4);

        csTT.LoadString(IDS_ATT_TOOLTIP2);
        m_pbColTT[1] = true;
        m_pHeaderCtrl->GetItemRect(4, &rcTT);
        m_pToolTipCtrl->AddTool(m_pHeaderCtrl, csTT, &rcTT, 5);
        break;
    }
    EnableToolTips();
    m_pToolTipCtrl->Activate(TRUE);
  }

  m_iItem = 0;
}

BOOL CPWAttLC::PreTranslateMessage(MSG* pMsg)
{
  // Do tooltips
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);

  return CListCtrl::PreTranslateMessage(pMsg);
}

void CPWAttLC::OnHdnItemchanged(NMHDR *, LRESULT *pResult)
{
  RecalcHeaderTips();
  *pResult = 0;
}

void CPWAttLC::OnHdnEndDrag(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMHEADER pNMHeader = reinterpret_cast<LPNMHEADER>(pNMHDR);
  int nWidth = GetColumnWidth(pNMHeader->iItem);
  PostMessage(LVM_SETCOLUMNWIDTH, pNMHeader->iItem, MAKELPARAM(nWidth, 0));
  *pResult = 0;
}

void CPWAttLC::OnHdnEndTrack(NMHDR *, LRESULT *pResult)
{
  RecalcHeaderTips();
  *pResult = 0;
}

void CPWAttLC::RecalcHeaderTips()
{
  RECT rcTT;
  // Update all tools' rect
  int numcols = m_pHeaderCtrl->GetItemCount();
  for (int i = 0; i < numcols; i++) {
    if (m_pbColTT[i]) {
      m_pHeaderCtrl->GetItemRect(i, &rcTT);
      m_pToolTipCtrl->SetToolRect(m_pHeaderCtrl, i + 1, &rcTT);
    }
  }
}

void CPWAttLC::OnLButtonDown(UINT nFlags, CPoint point)
{
  int iItem = -1;
  int iSubItem = -1;
  m_iItem = -1;
  size_t num;
  LVHITTESTINFO lvhti;

  if (m_bReadOnly)
    goto exit;

  if (m_lct == VIEW)
    goto exit;

  lvhti.pt = point;
  SubItemHitTest(&lvhti);

  if (lvhti.flags & LVHT_ONITEM) {
    iItem = lvhti.iItem;
    iSubItem = lvhti.iSubItem;
    m_iItem = iItem;
  }

  if ((iItem >= 0)    && (iItem < m_numattachments) &&
      (iSubItem >= 0) && (iSubItem < m_pHeaderCtrl->GetItemCount())) {
    num = GetItemData(m_iItem);
    switch (m_lct) {
      case EXISTING:
        switch (iSubItem) {
          case 0:
            m_vATRecords[num].uiflags ^= ATT_ATTACHMENT_DELETED;
            break;
          case 1:
            m_vATRecords[num].flags ^= ATT_ERASURE_REQUIRED;
            m_vATRecords[num].uiflags |= ATT_ATTACHMENT_FLGCHGD;
            break;
          case 2:
            m_vATRecords[num].flags ^= ATT_EXTRACTTOREMOVEABLE;
            m_vATRecords[num].uiflags |= ATT_ATTACHMENT_FLGCHGD;
            break;
          default:
            break;
        }
        if (iSubItem < 3) {
          m_pParent->SendMessage(PWS_MSG_ATTACHMENT_FLAG_CHANGED, EXISTING, m_iItem);
          RedrawItems(m_iItem, m_iItem);
        }
        break;
      case NEW:
        switch (iSubItem) {
          case 0:
            m_vATRecords[num].flags ^= ATT_ERASURE_REQUIRED;
            break;
          case 1:
            m_vATRecords[num].flags ^= ATT_EXTRACTTOREMOVEABLE;
            break;
          default:
            break;
        }
        if (iSubItem < 2) {
          m_pParent->SendMessage(PWS_MSG_ATTACHMENT_FLAG_CHANGED, NEW, m_iItem);
          RedrawItems(m_iItem, m_iItem);
        }
        break;
      case EXTRACT:
      case VIEW:
      default:
        break;
    }
  }

exit:
  CListCtrl::OnLButtonDown(nFlags, point);
}

void CPWAttLC::OnRButtonDown(UINT nFlags, CPoint point)
{
  CMenu PopupMenu;
  CPoint pt;
  int iItem = -1;
  int iSubItem = -1;
  m_iItem = -1;
  size_t num;
  LVHITTESTINFO lvhti;

  lvhti.pt = point;
  SubItemHitTest(&lvhti);

  if (lvhti.flags & LVHT_ONITEM) {
    iItem = lvhti.iItem;
    iSubItem = lvhti.iSubItem;
    m_iItem = iItem;
  }

  if ((iItem >= 0)    && (iItem < m_numattachments) &&
      (iSubItem >= 0) && (iSubItem < m_pHeaderCtrl->GetItemCount())) {
    num = GetItemData(m_iItem);

    ATRecord *pATR(NULL);

    switch (m_lct) {
      case EXISTING:
      case NEW:
      case EXTRACT:
        pATR = &m_vATRecords[num];
        break;
      case VIEW:
        pATR = &m_vATRecordsEx[num].atr;
        break;
      default:
        goto exit;
    }

    if (pATR == NULL)
      goto exit;

    PopupMenu.LoadMenu(IDR_POPATTACHMENTS);
    CMenu* pContextMenu = PopupMenu.GetSubMenu(0);
  
    MENUINFO minfo ={0};
    minfo.cbSize = sizeof(MENUINFO);
    minfo.fMask = MIM_MENUDATA;
    minfo.dwMenuData = IDR_POPATTACHMENTS;

    if (m_bReadOnly) {
      pContextMenu->EnableMenuItem(ID_MENUITEM_EDITDESCRIPTION, MF_BYCOMMAND | MF_GRAYED);
    }

    BOOL brc;
    brc = PopupMenu.SetMenuInfo(&minfo);
    ASSERT(brc != 0);

    if (m_lct == NEW)
      pContextMenu->RemoveMenu(ID_MENUITEM_EXTRACT_ATTACHMENT, MF_BYCOMMAND);

    pt = point;
    ClientToScreen(&pt);

    int nID = pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
                                           pt.x, pt.y, this);

    if (nID == ID_MENUITEM_EXTRACT_ATTACHMENT) {
      ASSERT(m_pWnd != NULL);
      m_pWnd->SendMessage(PWS_MSG_EXTRACT_ATTACHMENT, (WPARAM)pATR, 0);
      goto exit;
    }

    if (nID == ID_MENUITEM_EXPORTATT2XML) {
      ASSERT(m_pWnd != NULL);
      ATRecordEx *pATREx;
      ATRecordEx atrex;
      if (m_lct != VIEW) {
        // Need an extended record
        atrex.atr = m_vATRecords[num];
        atrex.sxGroup = atrex.sxTitle = atrex.sxUser = L"";
        pATREx = &atrex;
      } else {
        pATREx = &m_vATRecordsEx[num];
      }
      m_pWnd->SendMessage(PWS_MSG_EXPORT_ATTACHMENT, (WPARAM)pATREx, 0);
      goto exit;
    }

    if (nID == ID_MENUITEM_EDITDESCRIPTION) {
      CAddDescription dlg(this, pATR->filename.c_str(), pATR->description.c_str());

      INT_PTR rc =  dlg.DoModal();

      if (rc == IDOK && pATR->description.c_str() != dlg.GetDescription()) {
        pATR->description = dlg.GetDescription();
        m_pWnd->SendMessage(PWS_MSG_CHANGE_ATTACHMENT, (WPARAM)pATR, 0);
        int nDescCol(3);
        if (m_lct == EXISTING) {
          nDescCol = 4;
        } else
        if (m_lct == VIEW) {
          nDescCol = 6;
        }

        SetItemText(iItem, nDescCol, pATR->description.c_str());
        Update(iItem);
      }
      goto exit;
    }

    if (nID != ID_MENUITEM_PROPERTIES)
      goto exit;

    double dblVar;
    wchar_t wcbuffer[40];
    StringX sx_text;
    CString cs_text;
    st_AttProp st_prop;

    st_prop.name = pATR->filename.c_str();
    st_prop.path = pATR->path.c_str();
    if (!pATR->description.empty())
      st_prop.desc = pATR->description.c_str();
    else
      st_prop.desc = L"n/a";

    dblVar = (pATR->uncsize + 1023.0) / 1024.0;
    PWSNumUtil::DoubleToLocalizedString(::GetThreadLocale(), dblVar, 0,
                                        wcbuffer, sizeof(wcbuffer) / sizeof(wchar_t));
    wcscat_s(wcbuffer, sizeof(wcbuffer) / sizeof(wchar_t), L" KB");
    st_prop.usize =  wcbuffer;
    if (m_lct != NEW) {
      sx_text = PWSUtil::ConvertToDateTimeString(pATR->dtime, TMC_LOCALE);
      st_prop.ddate = sx_text.c_str();
    } else {
      st_prop.ddate = L"n/a";
    }
    sx_text = PWSUtil::ConvertToDateTimeString(pATR->mtime, TMC_LOCALE);
    st_prop.mdate = sx_text.c_str();
    sx_text = PWSUtil::ConvertToDateTimeString(pATR->atime, TMC_LOCALE);
    st_prop.adate = sx_text.c_str();
    sx_text = PWSUtil::ConvertToDateTimeString(pATR->ctime, TMC_LOCALE);
    st_prop.cdate = sx_text.c_str();
    if (m_lct != NEW) {
      cs_text.Format(L"%08x", pATR->CRC);
      st_prop.crc =  cs_text;

      wostringstream os;
      for (size_t i = 0; i < sizeof(pATR->odigest); i++) {
        os << setw(2) << setfill(wchar_t('0')) << hex << int(pATR->odigest[i]);
        if ((i % 4) == 3)
          os << L" ";
      }
      st_prop.odigest = os.str().c_str();

      if (pATR->uncsize != 0) {
        dblVar = (pATR->cmpsize * 1.0E02) / pATR->uncsize;
        PWSNumUtil::DoubleToLocalizedString(::GetThreadLocale(), dblVar, 1,
                                            wcbuffer, sizeof(wcbuffer) / sizeof(wchar_t));
        wcscat_s(wcbuffer, sizeof(wcbuffer) / sizeof(wchar_t), L" %");
        st_prop.comp = wcbuffer;
      } else {
        st_prop.comp = L"n/a";
      }
    } else {
      st_prop.crc = L"n/a";
      st_prop.comp = L"n/a";
      st_prop.odigest = L"n/a";
    }
    CAttProperties dlg(st_prop, this);
    dlg.DoModal();
  }

exit:
  CListCtrl::OnRButtonDown(nFlags, point);
}

void CPWAttLC::OnCustomDraw(NMHDR* pNotifyStruct, LRESULT* pResult)
{
  NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNotifyStruct);

  *pResult = CDRF_DODEFAULT;
  const int iItem = pLVCD->nmcd.dwItemSpec;
  const int iSubItem = pLVCD->iSubItem;
  int ix, iy;
  size_t num;

  // Min & Max subitem that is a check bocx for NEW, EXTRACT, EXISTING & VIEW
  const static int imin_checkboxes[4] = {0, 0, 0, 3};
  const static int imax_checkboxes[4] = {1, 1, 2, 4};

  switch(pLVCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      *pResult = CDRF_NOTIFYITEMDRAW;
      break;
    case CDDS_ITEMPREPAINT:
      *pResult = CDRF_NOTIFYSUBITEMDRAW;
      break;
    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      {
        CRect rect;
        GetSubItemRect(iItem, iSubItem, LVIR_BOUNDS, rect);
        if (rect.top < 0) {
          *pResult = CDRF_SKIPDEFAULT;
          break;
        }
        if (iItem > m_numattachments - 1) {
          *pResult = CDRF_SKIPDEFAULT;
          return;
        }
        if (iSubItem == 0) {
          CRect rect1;
          GetSubItemRect(iItem, 1, LVIR_BOUNDS, rect1);
          rect.right = rect1.left;
        }
        pLVCD->clrText = m_crWindowText;
        pLVCD->clrTextBk = GetTextBkColor();
        CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
        CRect inner_rect(rect), first_rect(rect);
        inner_rect.DeflateRect(2, 2);

        if (iSubItem >= imin_checkboxes[m_lct] &&
            iSubItem <= imax_checkboxes[m_lct]) {
          num = pLVCD->nmcd.lItemlParam;
          int iCheck(-1);
          switch (m_lct) {
            case NEW:
               switch (iSubItem) {
                case 0:
                 iCheck = (m_vATRecords[num].flags & ATT_ERASURE_REQUIRED) ==
                             ATT_ERASURE_REQUIRED ? 1 : 0;
                 break;
                case 1:
                  iCheck = (m_vATRecords[num].flags & ATT_EXTRACTTOREMOVEABLE) ==
                              ATT_EXTRACTTOREMOVEABLE ? 1 : 0;
                  break;
              }
              break;
            case EXTRACT:
              switch (iSubItem) {
                case 0:
                  iCheck = (m_vATRecords[num].flags & ATT_ERASURE_REQUIRED) ==
                              ATT_ERASURE_REQUIRED ? 1 : 0;
                  break;
                case 1:
                  iCheck = (m_vATRecords[num].flags & ATT_EXTRACTTOREMOVEABLE) ==
                              ATT_EXTRACTTOREMOVEABLE ? 1 : 0;
                  break;
              }
              break;
            case EXISTING:
              switch (iSubItem) {
                case 0:
                  iCheck = (m_vATRecords[num].uiflags & ATT_ATTACHMENT_DELETED) ==
                              ATT_ATTACHMENT_DELETED ? 1 : 0;
                  break;
                case 1:
                  iCheck = (m_vATRecords[num].flags & ATT_ERASURE_REQUIRED) ==
                              ATT_ERASURE_REQUIRED ? 1 : 0;
                  break;
                case 2:
                  iCheck = (m_vATRecords[num].flags & ATT_EXTRACTTOREMOVEABLE) ==
                              ATT_EXTRACTTOREMOVEABLE ? 1 : 0;
                  break;
              }
              break;
            case VIEW:
              switch (iSubItem) {
                case 3:
                  iCheck = (m_vATRecordsEx[num].atr.flags & ATT_ERASURE_REQUIRED) ==
                              ATT_ERASURE_REQUIRED ? 1 : 0;
                  break;
                case 4:
                  iCheck = (m_vATRecordsEx[num].atr.flags & ATT_EXTRACTTOREMOVEABLE) ==
                              ATT_EXTRACTTOREMOVEABLE ? 1 : 0;
                  break;
              }
              break;
          }
          if (iCheck >= 0) {
            // Draw checked/unchecked image
            ix = inner_rect.CenterPoint().x;
            iy = inner_rect.CenterPoint().y;
            // The '7' below is ~ half the bitmap size of 13.
            inner_rect.SetRect(ix - 7, iy - 7, ix + 7, iy + 7);
            if (m_bReadOnly)
              iCheck += 2;
            DrawImage(pDC, inner_rect, iCheck);
            *pResult = CDRF_SKIPDEFAULT;
          }
        }
      }
      break;
    default:
      break;
  }
}

void CPWAttLC::DrawImage(CDC *pDC, CRect &rect, int nImage)
{
  // Draw check image in given rectangle
  if (rect.IsRectEmpty() || nImage < 0)
    return;

  if (m_pCheckImageList) {
    SIZE sizeImage = {0, 0};
    IMAGEINFO info;

    if (m_pCheckImageList->GetImageInfo(nImage, &info)) {
      sizeImage.cx = info.rcImage.right - info.rcImage.left;
      sizeImage.cy = info.rcImage.bottom - info.rcImage.top;
    }

    if (nImage >= 0) {
      if (rect.Width() > 0) {
        POINT point;

        point.y = rect.CenterPoint().y - (sizeImage.cy >> 1);
        point.x = rect.left;

        SIZE size;
        size.cx = rect.Width() < sizeImage.cx ? rect.Width() : sizeImage.cx;
        size.cy = rect.Height() < sizeImage.cy ? rect.Height() : sizeImage.cy;

        // Due to a bug in VS2010 MFC the following does not work!
        //    m_pCheckImageList->DrawIndirect(pDC, nImage, point, size, CPoint(0, 0));
        // So do it the hard way!
        IMAGELISTDRAWPARAMS imldp = {0};
        imldp.cbSize = sizeof(imldp);
        imldp.i = nImage;
        imldp.hdcDst = pDC->m_hDC;
        imldp.x = point.x;
        imldp.y = point.y;
        imldp.xBitmap = imldp.yBitmap = 0;
        imldp.cx = size.cx;
        imldp.cy = size.cy;
        imldp.fStyle = ILD_NORMAL;
        imldp.dwRop = SRCCOPY;
        imldp.rgbBk = CLR_DEFAULT;
        imldp.rgbFg = CLR_DEFAULT;
        imldp.fState = ILS_NORMAL;
        imldp.Frame = 0;
        imldp.crEffect = CLR_DEFAULT;

        m_pCheckImageList->DrawIndirect(&imldp);
      }
    }
  }
}

INT_PTR CPWAttLC::OnToolHitTest(CPoint point, TOOLINFO *pTI) const
{
  LVHITTESTINFO lvhti;
  lvhti.pt = point;

  ListView_SubItemHitTest(this->m_hWnd, &lvhti);
  int nSubItem = lvhti.iSubItem;

  // nFlags is 0 if the SubItemHitTest fails
  // Therefore, 0 & <anything> will equal false
  if (lvhti.flags & LVHT_ONITEMLABEL) {
    // get the client (area occupied by this control
    RECT rcClient;
    GetClientRect(&rcClient);

    // fill in the TOOLINFO structure
    pTI->hwnd = m_hWnd;
    pTI->uId = (UINT) (nSubItem + 1);
    pTI->lpszText = LPSTR_TEXTCALLBACK;
    pTI->rect = rcClient;

    return pTI->uId;  // By returning a unique value per listItem,
              // we ensure that when the mouse moves over another
              // list item, the tooltip will change
  } else {
    //Otherwise, we aren't interested, so let the message propagate
    return -1;
  }
}

BOOL CPWAttLC::OnToolTipText(UINT /*id*/, NMHDR * pNMHDR, LRESULT * pResult)
{
  UINT nID = pNMHDR->idFrom;

  // check if this is the automatic tooltip of the control
  if (nID == 0)
    return TRUE;  // do not allow display of automatic tooltip,
                  // or our tooltip will disappear

  // handle both ANSI and UNICODE versions of the message
  TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
  TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

  *pResult = 0;

  // get the mouse position
  const MSG* pMessage;
  pMessage = GetCurrentMessage();
  ASSERT(pMessage);
  CPoint pt;
  pt = pMessage->pt;    // get the point from the message
  ScreenToClient(&pt);  // convert the point's coords to be relative to this control

  // see if the point falls onto a list item
  LVHITTESTINFO lvhti;
  lvhti.pt = pt;

  SubItemHitTest(&lvhti);
  int nSubItem = lvhti.iSubItem;

  // nFlags is 0 if the SubItemHitTest fails
  // Therefore, 0 & <anything> will equal false
  if (lvhti.flags & LVHT_ONITEMLABEL) {
    // If it did fall on a list item,
    // and it was also hit one of the
    // item specific subitems we wish to show tooltips for
    switch (m_lct) {
      case EXISTING:
        switch (nSubItem) {
          case 0:
            nID = IDS_ATT_TOOLTIP0;
            break;
          case 1:
            nID = IDS_ATT_TOOLTIP1;
            break;
          case 2:
            nID = IDS_ATT_TOOLTIP2;
            break;
          default:
            return FALSE;
        }
        break;
      case NEW:
      case EXTRACT:
        switch (nSubItem) {
          case 0:
            nID = IDS_ATT_TOOLTIP1;
            break;
          case 1:
            nID = IDS_ATT_TOOLTIP2;
            break;
          default:
            return FALSE;
        }
      case VIEW:
        switch (nSubItem) {
          case 3:
            nID = IDS_ATT_TOOLTIP1;
            break;
          case 4:
            nID = IDS_ATT_TOOLTIP2;
            break;
          default:
            return FALSE;
        }
        break;
    }
    // If there was a CString associated with the list item,
    // copy it's text (up to 80 characters worth, limitation
    // of the TOOLTIPTEXT structure) into the TOOLTIPTEXT
    // structure's szText member
    CString cs_TipText(MAKEINTRESOURCE(nID));

#define LONG_TOOLTIPS

#ifdef LONG_TOOLTIPS
    if (pNMHDR->code == TTN_NEEDTEXTA) {
      delete m_pchTip;

      m_pchTip = new char[cs_TipText.GetLength() + 1];
#if (_MSC_VER >= 1400)
      size_t num_converted;
      wcstombs_s(&num_converted, m_pchTip, cs_TipText.GetLength() + 1, cs_TipText,
                 cs_TipText.GetLength() + 1);
#else
      wcstombs(m_pchTip, cs_TipText, cs_TipText.GetLength() + 1);
#endif
      pTTTA->lpszText = (LPSTR)m_pchTip;
    } else {
      delete m_pwchTip;

      m_pwchTip = new WCHAR[cs_TipText.GetLength() + 1];
#if (_MSC_VER >= 1400)
      wcsncpy_s(m_pwchTip, cs_TipText.GetLength() + 1,
                cs_TipText, _TRUNCATE);
#else
      wcsncpy(m_pwchTip, cs_TipText, cs_TipText.GetLength() + 1);
#endif
      pTTTW->lpszText = (LPWSTR)m_pwchTip;
    }
#else // Short Tooltips!
    if (pNMHDR->code == TTN_NEEDTEXTA) {
      int n = WideCharToMultiByte(CP_ACP, 0, cs_TipText, -1,
                                  pTTTA->szText, _countof(pTTTA->szText),
                                  NULL, NULL);
      if (n > 0)
        pTTTA->szText[n - 1] = 0;
    } else {
#if (_MSC_VER >= 1400)
      wcsncpy_s(pTTTW->szText, _countof(pTTTW->szText),
               cs_TipText, _TRUNCATE);
#else
      wcsncpy(pTTTW->szText, cs_TipText, _countof(pTTTW->szText));
#endif
    }
#endif // Long/short tooltips

    return TRUE;   // we found a tool tip,
  }

  return FALSE;  // we didn't handle the message, let the
                 // framework continue propagating the message
}

void CPWAttLC::RemoveAttachment()
{
  if (m_lct == EXISTING) {
    // User presses Delete key whilst an entry in Existing Attachment ListCtrl is selected
    DWORD_PTR dwData = GetItemData(m_iItem);
    m_vATRecords[dwData].uiflags |= ATT_ATTACHMENT_DELETED;
  } else {
    // User presses Delete key whilst an entry in New Attachment ListCtrl is selected
    DeleteItem(m_iItem);
  }
}

void CPWAttLC::AddAttachments(ATRExVector &vatrex)
{
  // Only called by View Attachments
  m_vATRecordsEx = vatrex;
  for (size_t i = 0; i < m_vATRecordsEx.size(); i++) {
    AddAttachment(i, m_vATRecordsEx[i]);
  }
}
    
void CPWAttLC::AddAttachments(ATRVector &vatr)
{
  // Called by Add/Edit Attachments and Extract Attachments
  m_vATRecords = vatr;
  for (size_t i = 0; i < m_vATRecords.size(); i++) {
    AddAttachment(i, m_vATRecords[i]);
  }
}

void CPWAttLC::AddAttachment(const size_t &num, ATRecordEx &atrex)
{
  // Only called by View Attachments
  AddAttachment(num, atrex.atr, atrex.sxGroup.c_str(), 
                atrex.sxTitle.c_str(), atrex.sxUser.c_str());
}

void CPWAttLC::AddNewAttachment(const size_t &num, ATRecord &atr)
{
  // Add new attachment
  m_vATRecords.push_back(atr);
  ASSERT((m_vATRecords.size() - 1) == num);
  AddAttachment(num, atr);
}

void CPWAttLC::AddAttachment(const size_t &num, ATRecord &atr, 
                             const wchar_t * szGroup, const wchar_t * szTitle, const wchar_t * szUser)
{
  // Title is a mandatory field so must always not be NULL.
  // This is used to determine if called from View or Add/Edit or Extract
  int newID = m_numattachments;
  CString cs_text;
  StringX sx_text;

  m_numattachments++;
  int ncol(0);

  if (m_lct == EXISTING) {
    InsertItem(newID, L""); ncol++;                          // ATT_DELETE_BUTTON
    SetItemText(newID, ncol, L""); ncol++;                   // ATT_ERSRQD_BUTTON
  } else
  if (m_lct == VIEW) {
    ASSERT(szTitle != NULL);
    InsertItem(newID, szGroup); ncol++;                      // ENTRY - group
    SetItemText(newID, ncol, szTitle); ncol++;               // ENTRY - title
    SetItemText(newID, ncol, szUser); ncol++;                // ENTRY - user
    SetItemText(newID, ncol, L""); ncol++;                   // ATT_ERSRQD_BUTTON
  } else {
    InsertItem(newID, L""); ncol++;                          // ATT_ERSRQD_BUTTON
  }
  SetItemText(newID, ncol, L""); ncol++;                     // ATT_REMDRV_BUTTON
  SetItemText(newID, ncol, atr.filename.c_str()); ncol++;    // ATT_FILENAME
  SetItemText(newID, ncol, atr.description.c_str()); ncol++; // ATT_DESCRIPTION
  SetItemText(newID, ncol, atr.path.c_str()); ncol++;        // ATT_PATH

  SetItemData(newID, num);

  int numcols = GetHeaderCtrl()->GetItemCount();
  for (int i = 0; i < numcols; i++) {
    SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
  }
}
