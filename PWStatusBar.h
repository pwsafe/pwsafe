/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CPWStatusBar

class CPWStatusBar : public CStatusBar
{
  DECLARE_DYNAMIC(CPWStatusBar)

  enum {SB_DBLCLICK = 0, SB_CLIPBOARDACTION,
#ifdef DEBUG
        SB_CONFIG,
#endif
        SB_MODIFIED, SB_READONLY, SB_NUM_ENT, SB_FILTER,
        SB_TOTAL /* this must be the last entry */};

public:
  CPWStatusBar();
  virtual ~CPWStatusBar();
  virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

  void SetFilterStatus(bool bStatus)
  {m_bFilterStatus = bStatus;}

  int GetBitmapWidth()
  {return m_bmWidth;}

protected:
  //{{AFX_MSG(CPWStatusBar)
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  bool m_bFilterStatus;
  int m_bmHeight, m_bmWidth;
  CBitmap m_FilterBitmap;
};
