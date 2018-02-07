/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* Courtesy of Zafir Anjum from www.codeguru.com 6 August 1998
* "Drawing horizontal and vertical gridlines" but now made
* a separate class.
* Updated due to comments (pen colour) and adjustable columnns.
* Also support our non-standard Header control ID
*/

#pragma once

// CGridListCtrl

class CGridListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CGridListCtrl)

public:
	CGridListCtrl();
	virtual ~CGridListCtrl();
  virtual void PreSubclassWindow();

  void SetLineColour(int RGBLineColour) { m_RGBLineColour = RGBLineColour; }
  void SetHeaderCtrlID(UINT nID) { m_HeaderCtrlID = nID; }

protected:
  afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()

private:
  UINT m_HeaderCtrlID;
  int m_RGBLineColour;
};
