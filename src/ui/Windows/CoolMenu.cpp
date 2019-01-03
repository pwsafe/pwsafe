/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

////////////////////////////////////////////////////////////////
// CoolMenu 1997 Microsoft Systems Journal.
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//

// Modified for PWS toolbar
// Remove automatic accelerators
// Updated for VS2005 + UNICODE
// Change from MS classes to STL equivalents where possible
// Understand that RUE menu also uses MENUITEMINFO dwItemData field for icons

#include "StdAfx.h"
#include "CoolMenu.h"
#include "PWToolbar.h"
#include "resource2.h"
#include "resource3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// constants used for drawing
const int CXGAP = 1;           // num pixels between button and text
const int CXTEXTMARGIN = 2;    // num pixels after highlight to start text
const int CXBUTTONMARGIN = 2;  // num pixels wider button is than bitmap
const int CYBUTTONMARGIN = 2;  // ditto for height

// DrawText flags
const int DT_MYSTANDARD = DT_SINGLELINE | DT_LEFT | DT_VCENTER;

IMPLEMENT_DYNAMIC(CCoolMenuManager, CSubclassWnd)

CCoolMenuManager::CCoolMenuManager()
{
  m_bShowButtons = TRUE;                 // show buttons by default
  FixMFCDotBitmap();

  // These are PWS specific.
  m_szBitmap = CSize(16, 16);
  m_szButton = CSize(16, 16) + CSize(CXBUTTONMARGIN << 1, CYBUTTONMARGIN << 1);
}

CCoolMenuManager::~CCoolMenuManager()
{
}

void CCoolMenuManager::Cleanup()
{
  // For some reason - the destructor doesn't get called!
  m_ImageList.DeleteImageList();
  if (!m_bNoDIL)
    m_DisabledImageList.DeleteImageList();
  m_IDtoImages.clear(); 
  m_fontMenu.DeleteObject();

  // Somehow, for Dialog applications, if the user exits using the accelerator, 
  // it doesn't tidy up by calling OnMenuSelect(0, 0xFFFF, NULL), so do it now
  // the hard way!
  while (!m_pmdList.empty()) {
    CMenuItemData * &pmd = m_pmdList.back();
    if (pmd && pmd->IsCMID())
      delete pmd;
    m_pmdList.pop_back();
  }
}

//////////////////
// Virtual CSubclassWnd window proc. All messages come here before frame
// window. Isn't it cool? Just like in the old days!
//
LRESULT CCoolMenuManager::WindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
  switch(msg) {
    case WM_MEASUREITEM:
      if (CMOnMeasureItem((MEASUREITEMSTRUCT*)lp))
        return TRUE; // handled
      break;
    case WM_DRAWITEM:
      if (CMOnDrawItem((DRAWITEMSTRUCT*)lp))
        return TRUE; // handled
      break;
    case WM_INITMENUPOPUP:
      // Very important: must let dialog window handle it first!
      // Because if someone calls CCmdUI::SetText, MFC will change item to
      // MFT_STRING, so I must change back to MFT_OWNERDRAW.
      //
      CSubclassWnd::WindowProc(msg, wp, lp);
      CMOnInitMenuPopup(CMenu::FromHandle((HMENU)wp), (UINT)LOWORD(lp), (BOOL)HIWORD(lp));
      return FALSE;
    case WM_MENUSELECT:
      CMOnMenuSelect((UINT)LOWORD(wp), (UINT)HIWORD(wp), (HMENU)lp);
      break;
    case WM_MENUCHAR:
      LRESULT lr = CMOnMenuChar((wchar_t)LOWORD(wp), (UINT)HIWORD(wp), 
                                CMenu::FromHandle((HMENU)lp));
      if (lr != 0)
        return lr;
      break;
  }
  return CSubclassWnd::WindowProc(msg, wp, lp);
}

//////////////////
// Get menu font, creating if needed
//
CFont * CCoolMenuManager::GetMenuFont()
{
  if (!(HFONT)m_fontMenu) {
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    VERIFY(m_fontMenu.CreateFontIndirect(&ncm.lfMenuFont));
  }
  return &m_fontMenu;
}

//////////////////
// Handle WM_MEASUREITEM on behalf of frame: compute menu item size.
//
BOOL CCoolMenuManager::CMOnMeasureItem(LPMEASUREITEMSTRUCT lpmis)
{
  ASSERT(lpmis);
  CMenuItemData *pmd = (CMenuItemData *)lpmis->itemData;
  if (lpmis->CtlType != ODT_MENU || pmd == NULL || !pmd->IsCMID())
    return FALSE; // not handled by me

  if (pmd->fType & MFT_SEPARATOR) {
    // separator: use half system height and zero width
    lpmis->itemHeight = GetSystemMetrics(SM_CYMENU) >> 1;
    lpmis->itemWidth  = 0;
  } else {
    // compute size of text: use DrawText with DT_CALCRECT

    CWindowDC dc(NULL);  // screen DC--I won't actually draw on it
    CRect rcText(0, 0, 0, 0);
    CFont *pOldFont = dc.SelectObject(GetMenuFont());
    dc.DrawText(pmd->text, rcText, DT_MYSTANDARD|DT_CALCRECT);
    dc.SelectObject(pOldFont);

    // height of item is just height of a standard menu item
    lpmis->itemHeight = std::max(GetSystemMetrics(SM_CYMENU), rcText.Height());

    // width is width of text plus a bunch of stuff
    int cx = rcText.Width();    // text width
    cx += CXTEXTMARGIN << 1;    // L/R margin for readability
    cx += CXGAP;                // space between button and menu text
    cx += m_szButton.cx << 1;   // button width (L=button; R=empty margin)

    // whatever value I return in lpmis->itemWidth, Windows will add the
    // width of a menu checkmark, so I must subtract to defeat Windows. Argh.
    //
    cx -= GetSystemMetrics(SM_CXMENUCHECK)-1;
    lpmis->itemWidth = cx;    // done deal
  }
  return TRUE; // handled
}

/////////////////
// Handle WM_DRAWITEM on behalf of frame. Note: in all that goes
// below, can't assume rcItem.left=0 because of multi-column menus!
//
BOOL CCoolMenuManager::CMOnDrawItem(LPDRAWITEMSTRUCT lpdis)
{
  ASSERT(lpdis);
  CMenuItemData *pmd = (CMenuItemData *)lpdis->itemData;
  if (lpdis->CtlType != ODT_MENU || pmd == NULL || !pmd->IsCMID())
    return FALSE; // not handled by me

  ASSERT(lpdis->itemAction != ODA_FOCUS);
  ASSERT(lpdis->hDC);
  CDC dc;
  dc.Attach(lpdis->hDC);

  const CRect& rcItem = lpdis->rcItem;
  if (pmd->fType & MFT_SEPARATOR) {
    // draw separator
    CRect rc = rcItem;                // copy rect
    rc.top += rc.Height() >> 1;        // vertical center
    dc.DrawEdge(&rc, EDGE_ETCHED, BF_TOP);    // draw separator line
  } else {                          // not a separator
    BOOL bDisabled = lpdis->itemState & ODS_GRAYED;
    BOOL bSelected = lpdis->itemState & ODS_SELECTED;
    BOOL bChecked  = lpdis->itemState & ODS_CHECKED;
    BOOL bHaveButn = FALSE;

    // Paint button, or blank if none
    CRect rcButn(rcItem.TopLeft(), m_szButton);  // button rect
    rcButn += CPoint(0,                  // center vertically
                     (rcItem.Height() - rcButn.Height()) >> 1);

    int iButton = pmd->iButton;
    if (iButton >= 0) {
      // this item has a button!
      bHaveButn = TRUE;

      // compute point to start drawing
      CSize sz = rcButn.Size() - m_szBitmap;
      sz.cx >>= 1;
      sz.cy >>= 1;
      CPoint p(rcButn.TopLeft() + sz);

      // draw disabled or normal
      if (!bDisabled) {
        // normal: fill BG depending on state
        FillRect(dc, rcButn, GetSysColor(
          (bChecked && !bSelected) ? COLOR_3DLIGHT : COLOR_MENU));

        // draw pushed-in or popped-out edge
        if (bSelected || bChecked) {
          CRect rc2 = rcButn;
          dc.DrawEdge(rc2, bChecked ? BDR_SUNKENOUTER : BDR_RAISEDINNER,
                      BF_RECT);
        }
        // draw the button!
        m_ImageList.Draw(&dc, iButton, p, ILD_TRANSPARENT);

      } else {
        // use DrawEmbossed to draw disabled button in colour for Classic menu
        if (m_bNoDIL)
          DrawEmbossed(dc, m_ImageList, iButton, p);
        else
          m_DisabledImageList.Draw(&dc, iButton, p, ILD_TRANSPARENT);
      }
    } else {
      // no button: look for custom checked/unchecked bitmaps
      CMenuItemInfo miinfo;
      miinfo.fMask = MIIM_CHECKMARKS;
      GetMenuItemInfo((HMENU)lpdis->hwndItem,
                      lpdis->itemID, MF_BYCOMMAND, &miinfo);
      if (bChecked || miinfo.hbmpUnchecked) {
        bHaveButn = Draw3DCheckmark(dc, rcButn, bSelected,
                                    bChecked ? miinfo.hbmpChecked : miinfo.hbmpUnchecked);
      }
    }

    // Done with button, now paint text. First do background if needed.
    int cxButn = m_szButton.cx;        // width of button
    COLORREF colorBG = GetSysColor(bSelected ? COLOR_HIGHLIGHT : COLOR_MENU);
    if (bSelected || lpdis->itemAction == ODA_SELECT) {
      // selected or selection state changed: paint text background
      CRect rcBG = rcItem;              // whole rectangle
      if (bHaveButn)                    // if there's a button:
        rcBG.left += cxButn + CXGAP;    //  don't paint over it!
      FillRect(dc, rcBG, colorBG);      // paint it!
    }

    // compute text rectangle and colors
    CRect rcText = rcItem;         // start w/whole item
    rcText.left += cxButn + CXGAP + CXTEXTMARGIN; // left margin
    rcText.right -= cxButn;        // right margin
    dc.SetBkMode(TRANSPARENT);     // paint transparent text
    COLORREF colorText = GetSysColor(bDisabled ?  COLOR_GRAYTEXT :
      bSelected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT);

    // Now paint menu item text.  No need to select font,
    // because windows sets it up before sending WM_DRAWITEM
    //
    if (bDisabled && (!bSelected || colorText == colorBG)) {
      // disabled: draw highlight text shifted southeast 1 pixel for embossed
      // look. Don't do it if item is selected, tho--unless text color same
      // as menu highlight color. Got it?
      //
      DrawMenuText(dc, rcText + CPoint(1,1), pmd->text,
        GetSysColor(COLOR_3DHILIGHT));
    }
    DrawMenuText(dc, rcText, pmd->text, colorText); // finally!
  }
  dc.Detach();

  return TRUE; // handled
}

/////////////////
// Helper function to draw justified menu text. If the text contains a TAB,
// draw everything after the tab right-aligned
//
void CCoolMenuManager::DrawMenuText(CDC& dc, CRect rc, CString text,
                                    COLORREF color)
{
  CString left = text;
  CString right;
  int iTabPos = left.Find('\t');
  if (iTabPos >= 0) {
    right = left.Right(left.GetLength() - iTabPos - 1);
    left  = left.Left(iTabPos);
  }
  dc.SetTextColor(color);
  dc.DrawText(left, &rc, DT_MYSTANDARD);
  if (iTabPos > 0)
    dc.DrawText(right, &rc, DT_MYSTANDARD|DT_RIGHT);
}

#ifndef OBM_CHECK
#define OBM_CHECK 32760 // from winuser.h
#endif

//////////////////
// Draw 3D checkmark
//
//    dc         device context to draw in
//    rc         rectangle to center bitmap in
//    bSelected  TRUE if button is also selected
//    hbmCheck   Checkmark bitmap to use, or NULL for default
//
BOOL CCoolMenuManager::Draw3DCheckmark(CDC& dc, const CRect& rc, BOOL bSelected, 
                                       HBITMAP hbm_Check)
{
  // get checkmark bitmap if none, use Windows standard
  HBITMAP hbmCheck(hbm_Check);
  if (!hbmCheck) {
    CBitmap bm;
    VERIFY(bm.LoadOEMBitmap(OBM_CHECK));
    hbmCheck = (HBITMAP)bm.Detach();
    ASSERT(hbmCheck);
  }

  // center bitmap in caller's rectangle
  BITMAP bm;
  ::GetObject(hbmCheck, sizeof(bm), &bm);
  int cx = bm.bmWidth;
  int cy = bm.bmHeight;
  CRect rcDest = rc;
  CPoint p(0, 0);
  CSize delta(CPoint((rc.Width() - cx)/2, (rc.Height() - cy)/2));
  if (rc.Width() > cx)
    rcDest = CRect(rc.TopLeft() + delta, CSize(cx, cy));
  else
    p -= delta;

  // select checkmark into memory DC
  CDC memdc;
  memdc.CreateCompatibleDC(&dc);
  HBITMAP hOldBM = (HBITMAP)::SelectObject(memdc, hbmCheck);

  // set BG color based on selected state
  COLORREF colorOld =
    dc.SetBkColor(GetSysColor(bSelected ? COLOR_MENU : COLOR_3DLIGHT));
  dc.BitBlt(rcDest.left, rcDest.top, rcDest.Width(), rcDest.Height(),
            &memdc, p.x, p.y, SRCCOPY);
  dc.SetBkColor(colorOld);

  ::SelectObject(memdc, hOldBM); // restore

  // draw pushed-in highlight.
  if (rc.Width() > cx)        // if room:
    rcDest.InflateRect(1, 1); // inflate checkmark by one pixel all around
  dc.DrawEdge(&rcDest, BDR_SUNKENOUTER, BF_RECT);

  memdc.DeleteDC();
  ::DeleteObject(hbmCheck);
  ::DeleteObject(hOldBM);
  return TRUE;
}

//////////////////
// Handle WM_INITMENUPOPUP on behalf of frame.
//
void CCoolMenuManager::CMOnInitMenuPopup(CMenu* pMenu, UINT nIndex, BOOL bSysMenu)
{
  ConvertMenu(pMenu, nIndex, bSysMenu, m_bShowButtons);
}

//////////////////
// This rather gnarly function is used both to convert the menu from strings to
// owner-draw and vice versa.
//
void CCoolMenuManager::ConvertMenu(CMenu* pMenu, UINT /* nIndex */, 
                                   BOOL bSysMenu, BOOL bShowButtons)
{
  ASSERT_VALID(pMenu);

  CString sItemName, cs_text;

  UINT nItem = pMenu->GetMenuItemCount();
  for (UINT i = 0; i < nItem; i++) {  // loop over each item in menu
    // get menu item info
    wchar_t itemname[256] = {L'\0'};
    CMenuItemInfo miinfo;
    miinfo.fMask = MIIM_SUBMENU | MIIM_DATA | MIIM_FTYPE | MIIM_ID | 
                   MIIM_STRING;
    miinfo.dwTypeData = itemname;
    miinfo.cch = sizeof(itemname);
    pMenu->GetMenuItemInfo(i, &miinfo, TRUE);
    CMenuItemData* pmd = (CMenuItemData*)miinfo.dwItemData;

    if (pmd != NULL && !pmd->IsCMID()) {
      continue; // owner-draw menu item isn't mine--leave it alone
    }

    if (bSysMenu && miinfo.wID >= 0xF000) {
      continue; // don't do for system menu commands
    }

    // now that I have the info, I will modify it
    miinfo.fMask = 0;  // assume nothing to change

    if (bShowButtons) {
      // I'm showing buttons: convert to owner-draw
      if (!(miinfo.fType & MFT_OWNERDRAW)) {
        // If not already owner-draw, make it so. NOTE: If app calls
        // pCmdUI->SetText to change the text of a menu item, MFC will
        // turn the item to MFT_STRING. So I must set it back to
        // MFT_OWNERDRAW again. In this case, the menu item data (pmd)
        // will still be there.
        //
        miinfo.fType |= MFT_OWNERDRAW;
        miinfo.fMask |= MIIM_FTYPE | MIIM_STRING;
        if (!pmd) {                           // if no item data:
          pmd = new CMenuItemData;            //   create one
          ASSERT(pmd);                        //   (I hope)
          m_pmdList.push_back(pmd);           // Save it for deleting later
          pmd->fType = miinfo.fType;          //   handy when drawing
          UINT iCtrlID = miinfo.wID;          // Get Control ID
          if (iCtrlID >= ID_MENUITEM_TRAYAUTOTYPE1 &&
            iCtrlID <= ID_MENUITEM_TRAYAUTOTYPEMAX)
            iCtrlID = ID_MENUITEM_AUTOTYPE;
          else
          if (iCtrlID >= ID_MENUITEM_TRAYBROWSE1 &&
              iCtrlID <= ID_MENUITEM_TRAYBROWSEMAX) {
            cs_text.LoadString(IDS_TRAYBROWSE);
            if (cs_text.Compare(miinfo.dwTypeData) == 0)
              iCtrlID = ID_MENUITEM_BROWSEURL;
            else
              iCtrlID = ID_MENUITEM_SENDEMAIL;
          } else
          if (iCtrlID >= ID_MENUITEM_TRAYDELETE1 &&
              iCtrlID <= ID_MENUITEM_TRAYDELETEMAX)
            iCtrlID = ID_MENUITEM_DELETEENTRY;
          else
          if (iCtrlID >= ID_MENUITEM_TRAYCOPYNOTES1 &&
              iCtrlID <= ID_MENUITEM_TRAYCOPYNOTESMAX)
            iCtrlID = ID_MENUITEM_COPYNOTESFLD;
          else
          if (iCtrlID >= ID_MENUITEM_TRAYCOPYPASSWORD1 &&
              iCtrlID <= ID_MENUITEM_TRAYCOPYPASSWORDMAX)
            iCtrlID = ID_MENUITEM_COPYPASSWORD;
          else
          if (iCtrlID >= ID_MENUITEM_TRAYCOPYUSERNAME1 &&
              iCtrlID <= ID_MENUITEM_TRAYCOPYUSERNAMEMAX)
            iCtrlID = ID_MENUITEM_COPYUSERNAME;
          else
          if (iCtrlID >= ID_MENUITEM_TRAYRUNCMD1 &&
              iCtrlID <= ID_MENUITEM_TRAYRUNCMDMAX)
            iCtrlID = ID_MENUITEM_RUNCOMMAND;
          else
          if (iCtrlID >= ID_MENUITEM_TRAYBROWSEPLUS1 &&
              iCtrlID <= ID_MENUITEM_TRAYBROWSEPLUSMAX)
            iCtrlID = ID_MENUITEM_BROWSEURLPLUS;
          else
          if (iCtrlID >= ID_MENUITEM_TRAYVIEWEDIT1 &&
              iCtrlID <= ID_MENUITEM_TRAYVIEWEDITMAX)
            iCtrlID = ID_MENUITEM_EDITENTRY;
          else
          if (iCtrlID >= ID_MENUITEM_TRAYSENDEMAIL1 &&
              iCtrlID <= ID_MENUITEM_TRAYSENDEMAILMAX)
            iCtrlID = ID_MENUITEM_SENDEMAIL;
          else
          if (iCtrlID >= ID_FILE_MRU_ENTRY1 &&
              iCtrlID <= ID_FILE_MRU_ENTRYMAX)
            iCtrlID = ID_MENUITEM_MRUENTRY;
          else
          if (iCtrlID >= ID_MENUITEM_TRAYSELECT1 &&
              iCtrlID <= ID_MENUITEM_TRAYSELECTMAX)
            iCtrlID = ID_MENUITEM_TRAYSELECT;

            pmd->iButton = GetButtonIndex(iCtrlID);
            miinfo.dwItemData = (ULONG_PTR)pmd; //   set in menu item data
            miinfo.fMask |= MIIM_DATA;          //   set item data
        }
        pmd->text = miinfo.dwTypeData;          // copy menu item string
      }

      // now add the menu to list of "converted" menus
      HMENU hmenu = pMenu->GetSafeHmenu();
      ASSERT(hmenu);
      MenuVectorIter iter = std::find(m_menuList.begin(), m_menuList.end(),
                                      hmenu);
      if (iter == m_menuList.end())
        m_menuList.push_back(hmenu);

    } else {
      // no buttons -- I'm converting to strings
      if (miinfo.fType & MFT_OWNERDRAW) {     // if ownerdraw:
        miinfo.fType &= ~MFT_OWNERDRAW;       //   turn it off
        miinfo.fMask |= MIIM_TYPE;            //   change item type
        ASSERT(pmd);                          //   sanity check
        sItemName = pmd->text;                //   save name before deleting pmd
      } else                                  // otherwise:
        sItemName = miinfo.dwTypeData;        //   use name from MENUITEMINFO

      // NOTE: pmd (item data) could still be left hanging around even
      // if MFT_OWNERDRAW is not set, in case mentioned above where app
      // calls pCmdUI->SetText to set text of item and MFC sets the type
      // to MFT_STRING.
      //
      miinfo.dwItemData = NULL;               // item data is NULL
      miinfo.fMask |= MIIM_DATA;              // change it
      delete pmd;                             // and item data too
      PMDVectorIter iter = std::find(m_pmdList.begin(), m_pmdList.end(), pmd);
      if (iter != m_pmdList.end())
        m_pmdList.erase(iter);

      if (miinfo.fMask & MIIM_TYPE) {
        // if setting name, copy name from CString to buffer and set cch
        wcsncpy_s(itemname, _countof(itemname), sItemName, _countof(itemname));
        miinfo.dwTypeData = itemname;
        miinfo.cch = sItemName.GetLength();
      }
    }

    // if after all the above, there is anything to change, change it
    if (miinfo.fMask) {
      pMenu->SetMenuItemInfo(i, &miinfo, TRUE);
    }
  }
}

//////////////////
// User typed a char into menu. Look for item with & preceeding the char typed.
//
LRESULT CCoolMenuManager::CMOnMenuChar(UINT nChar, UINT /* nFlags */, CMenu* pMenu)
{
  ASSERT_VALID(pMenu);

  UINT iCurrentItem = (UINT)-1; // guaranteed higher than any command ID
  CUIntArray arItemsMatched;    // items that match the character typed

  UINT nItem = pMenu->GetMenuItemCount();
  for (UINT i = 0; i < nItem; i++) {
    // get menu info
    CMenuItemInfo miinfo;
    miinfo.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
    pMenu->GetMenuItemInfo(i, &miinfo, TRUE);

    CMenuItemData* pmd = (CMenuItemData*)miinfo.dwItemData;
    if ((miinfo.fType & MFT_OWNERDRAW) && pmd && pmd->IsCMID()) {
      CString& text = pmd->text;
      int iAmpersand = text.Find(L'&');
      if (iAmpersand >= 0 && toupper(nChar) == toupper(text[iAmpersand + 1]))
        arItemsMatched.Add(i);
    }
    if (miinfo.fState & MFS_HILITE)
      iCurrentItem = i; // note index of current item
  }

  // arItemsMatched now contains indexes of items that match the char typed.
  //
  //   * if none: beep
  //   * if one:  execute it
  //   * if more than one: highlight next
  //
  size_t nFound = arItemsMatched.GetSize();
  if (nFound == 0)
    return 0;

  else
  if (nFound == 1)
    return MAKELONG(arItemsMatched[0], MNC_EXECUTE);

  // more than one found--return 1st one past current selected item;
  UINT iSelect = 0;
  for (UINT i = 0; i < nFound; i++) {
    if (arItemsMatched[i] > iCurrentItem) {
      iSelect = i;
      break;
    }
  }
  return MAKELONG(arItemsMatched[iSelect], MNC_SELECT);
}

//////////////////
// Handle WM_MENUSELECT: check for menu closed
//
void CCoolMenuManager::CMOnMenuSelect(UINT /* nItemID */, UINT nFlags, HMENU hSysMenu)
{
  if (hSysMenu == NULL && nFlags == 0xFFFF) {
    // Windows has closed the menu: restore all menus to original state
    while (!m_menuList.empty()) {
      HMENU &hmenu = m_menuList.back();
      ConvertMenu(CMenu::FromHandle(hmenu), 0, FALSE, FALSE);
      m_menuList.pop_back();
    }
  }
}

//////////////////
// This function fixes MFC's diseased dot bitmap used for
// "radio-style" menu items (CCmdUI->SetRadio), which is completely
// wrong if the menu font is large.
//
void CCoolMenuManager::FixMFCDotBitmap()
{
  HBITMAP hbmDot = GetMFCDotBitmap();
  if (hbmDot) {
    // Draw a centered dot of appropriate size
    BITMAP bm;
    ::GetObject(hbmDot, sizeof(bm), &bm);
    CRect rcDot(0, 0, bm.bmWidth, bm.bmHeight);
    rcDot.DeflateRect((bm.bmWidth>>1) - 2, (bm.bmHeight>>1) - 2);

    CWindowDC dcScreen(NULL);
    CDC memdc;
    memdc.CreateCompatibleDC(&dcScreen);
    int nSave = memdc.SaveDC();
    memdc.SelectStockObject(BLACK_PEN);
    memdc.SelectStockObject(BLACK_BRUSH);
    memdc.SelectObject((HGDIOBJ)hbmDot);
    memdc.PatBlt(0, 0, bm.bmWidth, bm.bmHeight, WHITENESS);
    memdc.Ellipse(&rcDot);
    memdc.RestoreDC(nSave);
    memdc.DeleteDC();
    ::DeleteObject(hbmDot);
  }
}

//////////////////
// This function gets MFC's dot bitmap.
//
HBITMAP CCoolMenuManager::GetMFCDotBitmap()
{
  // The bitmap is stored in afxData.hbmMenuDot, but afxData is MFC-private,
  // so the only way to get it is create a menu, set the radio check,
  // and then see what bitmap MFC set in the menu item.
  CMenu menu;
  VERIFY(menu.CreateMenu());
  VERIFY(menu.AppendMenu(MFT_STRING, 0, (LPCWSTR)NULL));
  CCmdUI cui;
  cui.m_pMenu = &menu;
  cui.m_nIndex = 0;
  cui.m_nIndexMax = 1;
  cui.SetRadio(TRUE);
  CMenuItemInfo miinfo;
  miinfo.fMask = MIIM_CHECKMARKS;
  menu.GetMenuItemInfo(0, &miinfo, TRUE);
  HBITMAP hbmDot = miinfo.hbmpChecked;
  menu.DestroyMenu();
  return hbmDot;
}

void CCoolMenuManager::SetImageList(CPWToolBar *pwtoolbar)
{
  m_ImageList.DeleteImageList();
  if (!m_bNoDIL)
    m_DisabledImageList.DeleteImageList();

  CToolBarCtrl &tbCtrl = pwtoolbar->GetToolBarCtrl();

  CImageList *pil = tbCtrl.GetImageList();
  m_ImageList.Create(pil);

  CImageList *pdil = tbCtrl.GetDisabledImageList();
  if (pdil != NULL) {
    m_DisabledImageList.Create(pdil);
    m_bNoDIL = false;
  } else
    m_bNoDIL = true;
}

////////////////////////////////////////////////////////////////
// Helper functions
// PixieLib(TM) Copyright 1997 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.

//////////////////
// Shorthand to fill a rectangle with a solid color.
//
void CCoolMenuManager::FillRect(CDC& dc, const CRect& rc, COLORREF color)
{
  CBrush brush(color);
  CBrush* pOldBrush = dc.SelectObject(&brush);
  dc.PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), PATCOPY);
  dc.SelectObject(pOldBrush);
}

// Function to draw embossed (disabled) bitmaps
// modified to create toolbar disabled images also used by the menu!

const COLORREF CWHITE  = RGB(255, 255, 255);

// This is the "magic" ROP code used to generate the embossed look for
// a disabled button. It's listed in Appendix F of the Win32 Programmer's
// Reference as PSDPxax (!) which is a cryptic reverse-polish notation for
//
// ((Destination XOR Pattern) AND Source) XOR Pattern
//
// which I leave to you to figure out. In the case where I apply it,
// Source is a monochrome bitmap which I want to draw in such a way that
// the black pixels get transformed to the brush color and the white pixels
// draw transparently--i.e. leave the Destination alone.
//
// black ==> Pattern
// white ==> Destination (ie, transparent)
//
// 0xb8074a is the ROP code that does this. For more info, see Charles
// Petzold, _Programming Windows_, 2nd Edition, p 622-624.
//
const DWORD    MAGICROP    = 0xb8074a;

//////////////////
// Draw an image with the embossed (disabled) look.
//
//    dc      device context to draw in
//    il      image list containing image
//    i       index of image to draw
//    p       point in dc to draw image at
//
void CCoolMenuManager::DrawEmbossed(CDC& dc, CImageList &il, int iBtn, CPoint p)
{
  IMAGEINFO ImageInfo;
  VERIFY(il.GetImageInfo(0, &ImageInfo));
  CRect rc = ImageInfo.rcImage;
  int cx = rc.Width();
  int cy = rc.Height();

  // create memory dc
  CDC memdc;
  memdc.CreateCompatibleDC(&dc);

  // create color bitmap
  CBitmap bm;
  //bm.CreateCompatibleBitmap(&dc, cx, cy);
  bm.CreateBitmap(cx, cy, 1, 1, NULL);

  // draw image into memory DC--fill BG white first
  CBitmap* pOldBitmap = memdc.SelectObject(&bm);
  memdc.PatBlt(0, 0, cx, cy, WHITENESS);
  il.Draw(&memdc, iBtn, CPoint(0, 0), ILD_TRANSPARENT);

  // This seems to be required. Why, I don't know. ???
  COLORREF colorOldBG = dc.SetBkColor(CWHITE);

  // Draw using highlight offset by (1,1), then shadow
  CBrush brShadow(GetSysColor(COLOR_3DSHADOW));
  CBrush brHilite(GetSysColor(COLOR_3DHIGHLIGHT));
  CBrush* pOldBrush = dc.SelectObject(&brHilite);
  dc.BitBlt(p.x + 1, p.y + 1, cx, cy, &memdc, 0, 0, MAGICROP);
  dc.SelectObject(&brShadow);
  dc.BitBlt(p.x, p.y, cx, cy, &memdc, 0, 0, MAGICROP);
  dc.SelectObject(pOldBrush);         // restore
  dc.SetBkColor(colorOldBG);          // ...
  memdc.SelectObject(pOldBitmap);     // ...
  memdc.DeleteDC();
  bm.DeleteObject();
}
