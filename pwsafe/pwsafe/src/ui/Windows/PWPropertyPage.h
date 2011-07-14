/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

class CPWPropertyPage : public CPropertyPage
{
public:
  CPWPropertyPage(UINT nID);
  // accepts two resids, choose which one to display based
  // on screen dimensions @ invoke time
  CPWPropertyPage(UINT nID, UINT shortID);
  virtual ~CPWPropertyPage() {}

  // Following override to reset idle timeout on any event
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

  enum {PP_DATA_CHANGED = 0,
        PP_UPDATE_VARIABLES,
        PP_UPDATE_PWPOLICY,
        PP_UPDATE_TIMES};

  DECLARE_DYNAMIC(CPWPropertyPage)
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
