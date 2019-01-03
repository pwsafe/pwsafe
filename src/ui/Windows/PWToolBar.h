/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CPWToolBar

#include "PWTouch.h"

#include <map>

typedef std::map<UINT, UINT> ID2ImageMap;
typedef ID2ImageMap::iterator ID2ImageMapIter;

class CPWToolBarX : public CToolBar
{
  DECLARE_DYNAMIC(CPWToolBarX)

public:
  CPWToolBarX();
  virtual ~CPWToolBarX();

  void Init(const int NumBits, const bool bRefresh = false);
  void LoadDefaultToolBar(const int toolbarMode);
  void CustomizeButtons(CString csButtonNames);
  void ChangeImages(const int toolbarMode);
  void Reset();

  CString GetButtonString() const;
  int GetBrowseURLImageIndex() const {return m_iBrowseURL_BM_offset;}
  int GetSendEmailImageIndex() {return m_iSendEmail_BM_offset;}
  void MapControlIDtoImage(ID2ImageMap &IDtoImages);
  void SetBitmapBackground(CBitmap &bm, const COLORREF newbkgrndColour);
  void RefreshImages();

protected:
  //{{AFX_MSG(CPWToolBarX)
  afx_msg void OnToolBarGetButtonInfo(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnToolBarQueryInsert(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnToolBarQueryDelete(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnToolBarQueryInfo(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnToolBarReset(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnDestroy();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  struct GuiRecord {
    CString name;
    UINT ID;
    UINT classicBM; // RESID of classic bitmap,
    UINT newBM;     // of New bitmap, and
    UINT disBM;     // disabled version of new bitmap
    UINT GetClassicBM() const {return classicBM;}
    UINT GetNewBM() const {return newBM;}
    UINT GetDisBM() const {return disBM;}
  };

  // member function pointer typedef for above getters
  typedef UINT (GuiRecord::*GuiRecordGetter)() const;
  
  // Following needed for std::find_if
  // Abandon with great joy when C++11 Lambda supported!
  struct GuiInfoFinder {
  GuiInfoFinder(const CString &s) : m_s(s) {}
    bool operator()(const GuiRecord &r) {return r.name == m_s;}
    const CString &m_s;
  };

  void SetupImageList(const GuiRecord *guiInfo,
                      GuiRecordGetter GetBM, GuiRecordGetter GetDisBM,
                      const int numBMs, const int nImageList);

  static const GuiRecord MainGuiInfo[];
  static const GuiRecord OtherGuiInfo[];

  // 1st = Classic; 2nd = New 8; 3rd = New 32;
  CImageList m_ImageLists[3];
  // 1st = New 8; 2nd = New 32;
  CImageList m_DisabledImageLists[2];

  CString m_csDefaultButtonString;
  TBBUTTON *m_pOriginalTBinfo;

  int m_iNum_Bitmaps, m_iNumDefaultButtons, m_NumBits;
  int m_toolbarMode, m_bitmode;
  bool m_bIsDefault;
  int m_iBrowseURL_BM_offset, m_iSendEmail_BM_offset;
};

/**
* typedef to hide the fact that CPWToolBar is really a mixin.
*/

typedef CPWTouch< CPWToolBarX > CPWToolBar;
