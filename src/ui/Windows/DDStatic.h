/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#if !defined _DDStatic_H
#define _DDStatic_H

#include "ControlExtns.h"
#include "corelib/ItemData.h"

class DboxMain;

class CStaticDropTarget;
class CStaticDropSource;
class CStaticDataSource;

#define MINIMUM_MOVE_SQUARE  4  // 2 x 2 pixels

class CDDStatic : public CStaticExtn
{
public:
  CDDStatic();
  ~CDDStatic();

// Attributes

// Operations
public:
  void Init(const UINT nImageID, const UINT nDisabledImageID);
  void SetStaticState(const bool state);
  BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
  void EndDrop() {m_bDropped = true;}

protected:
  //{{AFX_MSG(CDDStatic)
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnTimer(UINT nIDEvent);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
  afx_msg void OnDestroy();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

private:
  void SendToClipboard();
  void SetBitmapBackground(CBitmap &bm, const COLORREF newbkgrndColour);
  StringX GetData(const CItemData *pci);

  CStaticDropTarget *m_pDropTarget;
  CStaticDropSource *m_pDropSource;
  CStaticDataSource *m_pDataSource;
  friend class CStaticDataSource;

  DboxMain *m_pDbx;
  CItemData *m_pci;
  HGLOBAL m_hgDataUTXT, m_hgDataTXT;
  StringX m_groupname;

  bool m_bState;  // Control active/inactive
  UINT m_nID;
  CBitmap m_OKbitmap, m_NOTOKbitmap;

  CPoint m_StartPoint;
  bool m_bDropped;
  UINT m_TimerID;
  bool m_bMouseInClient;
};

#endif  // _DDStatic_H
