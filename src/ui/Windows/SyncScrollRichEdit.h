/*
* Copyright (c) 2017 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// SyncScrollRichEdit.h : header file
//

// CSyncScrollRichEdit

#include "RichEditCtrlExtn.h"

class CSyncScrollRichEdit : public CRichEditCtrlExtn
{
public:
	CSyncScrollRichEdit();
	virtual ~CSyncScrollRichEdit();

  void SetPartner(CSyncScrollRichEdit *pPartner)
  { m_pPartner = pPartner; }

protected:
  BOOL PartnerOnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
  void PartnerOnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar, SCROLLINFO *psi);
  void PartnerOnKeyDown(int iFirstVisibleLine);

  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);

	DECLARE_MESSAGE_MAP()

private:
  CSyncScrollRichEdit *m_pPartner;

  int m_nSBPos, m_nSBTrackPos;
};
