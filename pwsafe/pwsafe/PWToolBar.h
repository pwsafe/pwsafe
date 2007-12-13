/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#pragma once

// CPWToolBar

#include <map>

typedef std::map<UINT, UINT> ID2ImageMap;
typedef ID2ImageMap::iterator ID2ImageMapIter;

class CPWToolBar : public CToolBar
{
  DECLARE_DYNAMIC(CPWToolBar)

public:
  CPWToolBar();
  virtual ~CPWToolBar();

  void Init(const int NumBits, const bool bRefresh = false);
  void LoadDefaultToolBar(const int toolbarMode);
  void CustomizeButtons(CString csButtonNames);
  void ChangeImages(const int toolbarMode);
  void Reset();

  CString GetButtonString();
  int GetBrowseURLImageIndex() {return m_iBrowseURL_BM_offset;}
  int GetSendEmailImageIndex() {return m_iSendEmail_BM_offset;}
  void MapControlIDtoImage(ID2ImageMap &IDtoImages);
  void SetupImageList(const UINT *pBM_IDs, const UINT *pDisBM_IDs, 
                      const int numBMs, const int nImageList);
  void SetBitmapBackground(CBitmap &bm, const COLORREF newbkgrndColour);
  void RefreshImages();

protected:
  //{{AFX_MSG(CPWToolBar)
  afx_msg void OnToolBarGetButtonInfo(NMHDR *pNotifyStruct, LRESULT* pResult);
  afx_msg void OnToolBarQueryInsert(NMHDR* pNotifyStruct, LRESULT* pResult);
  afx_msg void OnToolBarQueryDelete(NMHDR* pNotifyStruct, LRESULT* pResult);
  afx_msg void OnToolBarQueryInfo(NMHDR* pNotifyStruct, LRESULT* pResult);
  afx_msg void OnToolBarReset(NMHDR* pNotifyStruct, LRESULT* pResult);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  static const CString m_csMainButtons[];
  static const UINT m_MainToolBarIDs[];
  static const UINT m_MainToolBarClassicBMs[];
  static const UINT m_MainToolBarNewBMs[];
  static const UINT m_MainToolBarNewDisBMs[];

  static const UINT m_OtherIDs[];
  static const UINT m_OtherClassicBMs[];
  static const UINT m_OtherNewBMs[];
  static const UINT m_OtherNewDisBMs[];

  // 1st = Classic; 2nd = New 8; 3rd = New 32;
  CImageList m_ImageLists[3];
  // 1st = New 8; 2nd = New 32;
  CImageList m_DisabledImageLists[2];

  CString m_csDefaultButtonString;
  TBBUTTON *m_pOriginalTBinfo;

  int m_iMaxNumButtons, m_iNum_Bitmaps, m_iNumDefaultButtons, m_NumBits;
  int m_toolbarMode, m_bitmode;
  bool m_bIsDefault;
  int m_iBrowseURL_BM_offset, m_iSendEmail_BM_offset;
};
