/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

class CWZPropertySheet;

class CWZPropertyPage : public CPropertyPage
{
public:
  DECLARE_DYNAMIC(CWZPropertyPage)

  CWZPropertyPage(UINT nID, UINT nIDCaption = 0, const int nType = INVALID);
  ~CWZPropertyPage() {}

  // Following override to reset idle timeout on any event
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

  enum {INVALID = -1, START, MIDDLE, PENULTIMATE, LAST};

protected:
  CWZPropertySheet *m_pWZPSH;

  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnSetActive();

  // Generated message map functions
  //{{AFX_MSG(CWZPropertyPage)
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  int m_nType;
  UINT m_nID;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
