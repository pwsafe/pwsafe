/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "PWDialog.h"
#include <vector>

// CPWResizeDialog dialog

class CPWResizeDialog : public CPWDialog
{
  DECLARE_DYNAMIC(CPWResizeDialog)

public:
  CPWResizeDialog(UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor
  virtual ~CPWResizeDialog();

  void AddMainCtrlID(UINT iMainCtrl) {m_iMainCtrl = iMainCtrl;}
  void AddBtnsCtrlIDs(std::vector<UINT> viBottomButtons, int iFocus = 0);
  void SetStatusBar(const UINT *pstatustext, int nIDCount, bool bTextVisible = true);
  bool IsStatusBarOK() {return m_bStatusBarOK;}
  void SetMaxHeightWidth(int maxHeight, int maxWidth);

// Dialog Data

protected:
  virtual BOOL OnInitDialog();

  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

  DECLARE_MESSAGE_MAP()

  CStatusBar m_RSDStatusBar;

private:
  void SetControls(int cx, int cy);

  bool m_bInitDone, m_bStatusBarOK;
  int m_DialogMinWidth, m_DialogMinHeight;
  int m_DialogMaxWidth, m_DialogMaxHeight;
  int m_cxBSpace, m_cyBSpace, m_cySBar, m_ybuttondiff;

  CWnd *m_pMainCtrl;
  std::vector<UINT> m_viBottomButtons;
  UINT m_iMainCtrl;
  UINT *m_pstatustext;
  int m_numbtns, m_numsbpanes, m_iFocus;
  bool m_bTextVisible;
};
