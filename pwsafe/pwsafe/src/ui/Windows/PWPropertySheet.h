/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

class CPWPropertySheet : public CPropertySheet
{
public:
  CPWPropertySheet(UINT nID, CWnd* pParent)
  : CPropertySheet(nID, pParent) {}

  CPWPropertySheet(LPCTSTR pszCaption, CWnd* pParent)
  : CPropertySheet(pszCaption, pParent) {}

  // Following override to reset idle timeout on any event
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
  // Following override to stop accelerators interfering
  virtual INT_PTR DoModal();

  DECLARE_DYNAMIC(CPWPropertySheet)
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
