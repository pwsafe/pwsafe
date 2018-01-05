/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

////////////////////////////////////////////////////////////////
// 1997 Microsoft Systems Journal.
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//

// Modified for PWS toolbar
// Remove automatic accelerators
// Updated for VS2005 + UNICODE
// Change from MS classes to STL equivalents where possible
// Understand that RUE menu also uses MENUITEMINFO dwItemData field for icons

#include "Subclass.h"
#include "PWToolbar.h"
#include <vector>
#include <map>
#include <algorithm>

//////////////////
// CCoolMenuManager implements "cool" menus with buttons in them. To use:
//
//  * Instantiate in your CMainFrame.
//  * Call Install to install it
//

// identifies owner-draw data as mine
const long COOLMENUITEMID = MAKELONG(MAKEWORD('C', 'M'),MAKEWORD('I', 'D'));

// private struct: one of these for each owner-draw menu item
struct CMenuItemData {
  long     magicNum;      // magic number identifying me
  CString  text;          // item text
  UINT     fType;         // original item type flags
  int      iButton;       // index of button image in image list
  CMenuItemData()         { magicNum = COOLMENUITEMID; iButton = -1; fType = 0; }
  BOOL     IsCMID()       { return magicNum == COOLMENUITEMID; }
};

typedef std::vector<HMENU> MenuVector;
typedef MenuVector::iterator MenuVectorIter;

typedef std::vector<CMenuItemData *> PMDVector;
typedef PMDVector::iterator PMDVectorIter;

class CCoolMenuManager : private CSubclassWnd
{
public:
  DECLARE_DYNAMIC(CCoolMenuManager)
  CCoolMenuManager();
  ~CCoolMenuManager();
  void Cleanup();

  // You can set these any time
  BOOL m_bShowButtons;      // use to control whether buttons are shown

  // public functions to use
  void Install(CWnd* pWnd) { HookWindow(pWnd); }
  void SetImageList(CPWToolBar *pwtoolbar);
  void SetMapping(CPWToolBar *pwtoolbar)
  {pwtoolbar->MapControlIDtoImage(m_IDtoImages);}

  // should never need to call:
  static  HBITMAP GetMFCDotBitmap();  // get..
  static  void    FixMFCDotBitmap();  // and fix MFC's dot bitmap

protected:
  CImageList     m_ImageList;   // image list for all buttons
  CImageList     m_DisabledImageList; // disabled image list for all buttons
  MenuVector     m_menuList;    // list of HMENU's initialized
  PMDVector      m_pmdList;     // Vector of pointers to CMenuItemData
  CSize          m_szBitmap;    // size of button bitmap
  CSize          m_szButton;    // size of button (including shadow)
  CFont          m_fontMenu;    // menu font
  ID2ImageMap    m_IDtoImages;  // To obtain image ID from Control ID
  bool           m_bNoDIL;      // "true" if No Disabled Image List i.e. Classic toolbar

  // helpers
  void DrawMenuText(CDC& dc, CRect rc, CString text, COLORREF color);
  BOOL Draw3DCheckmark(CDC& dc, const CRect& rc, BOOL bSelected,
    HBITMAP hbmCheck = NULL);
  void ConvertMenu(CMenu* pMenu,UINT nIndex,BOOL bSysMenu,BOOL bShowButtons);
  CFont * GetMenuFont();

  void FillRect(CDC& dc, const CRect& rc, COLORREF color);
  void DrawEmbossed(CDC& dc, CImageList &il, int iBtn, CPoint p);

  // Get button index for given command ID, or -1 if not found
  int  GetButtonIndex(UINT nID) {
    ID2ImageMapIter iter = m_IDtoImages.find(nID);
    return iter != m_IDtoImages.end() ? iter->second : -1;
  }

  // window proc to hook frame using CSubclassWnd implementation
  virtual LRESULT WindowProc(UINT msg, WPARAM wp, LPARAM lp);

  // CSubclassWnd message handlers
  virtual void CMOnInitMenuPopup(CMenu* pMenu, UINT nIndex, BOOL bSysMenu);
  virtual BOOL CMOnMeasureItem(LPMEASUREITEMSTRUCT lpms);
  virtual BOOL CMOnDrawItem(LPDRAWITEMSTRUCT lpds);
  virtual LRESULT CMOnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
  virtual void CMOnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
};

//////////////////
// Friendly version of MENUITEMINFO initializes itself
//
struct CMenuItemInfo : public MENUITEMINFO {
  CMenuItemInfo()
  {
    SecureZeroMemory(this, sizeof(MENUITEMINFO));
    cbSize = sizeof(MENUITEMINFO);
  }
};
