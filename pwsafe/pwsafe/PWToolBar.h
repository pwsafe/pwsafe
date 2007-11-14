/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#pragma once

// CPWToolBar

class CPWToolBar : public CToolBar
{
  DECLARE_DYNAMIC(CPWToolBar)

public:
  CPWToolBar();
  virtual ~CPWToolBar();

  void Init(const int NumBits);
  void LoadDefaultToolBar(const int toolbarMode);
  void CustomizeButtons(CString csButtonNames);
  void ChangeImages(const int toolbarMode);
  void Reset();

  CString GetButtonString();
  int GetBrowseURLImageIndex() {return m_iBrowseURL_BM_offset;}
  int GetSendEmailImageIndex() {return m_iSendEmail_BM_offset;}

protected:
  //{{AFX_MSG(CPWToolBar)
  //}}AFX_MSG
  afx_msg void OnToolBarGetButtonInfo(NMHDR *pNotifyStruct, LRESULT* pResult);
  afx_msg void OnToolBarQueryInsert(NMHDR* pNotifyStruct, LRESULT* pResult);
  afx_msg void OnToolBarQueryDelete(NMHDR* pNotifyStruct, LRESULT* pResult);
  afx_msg void OnToolBarQueryInfo(NMHDR* pNotifyStruct, LRESULT* pResult);
  afx_msg void OnToolBarReset(NMHDR* pNotifyStruct, LRESULT* pResult);

  DECLARE_MESSAGE_MAP()

private:
  static const CString m_csMainButtons[];
  static const UINT m_MainToolBarIDs[];
  static const UINT m_MainToolBarClassicBMs[];
  static const UINT m_MainToolBarNew8BMs[];
  static const UINT m_MainToolBarNew32BMs[];

  CString m_csDefaultButtonString;
  TBBUTTON *m_pOriginalTBinfo;
  CImageList m_ImageList;
  UINT m_ClassicFlags, m_NewFlags;
  int m_iMaxNumButtons, m_iNum_Bitmaps, m_iNumDefaultButtons;
  int m_toolbarMode, m_bitmode;
  COLORREF m_ClassicBackground, m_NewBackground;
  bool m_bIsDefault;

  int m_iBrowseURL_BM_offset, m_iSendEmail_BM_offset;
};
