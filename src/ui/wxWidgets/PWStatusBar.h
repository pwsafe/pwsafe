/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/** \file
 * Wrapper for wxStatusBar, same name/functionality as MFC version
 */

#ifndef __PWSTATUSBAR_H
#define __PWSTATUSBAR_H
#include <wx/statusbr.h>
class CPWStatusBar : public wxStatusBar
{
 public:
  enum {SB_DBLCLICK = 0, SB_CLIPBOARDACTION,
        SB_MODIFIED, SB_READONLY, SB_NUM_ENT, SB_FILTER,
        SB_LAST};
  CPWStatusBar(wxWindow *parent, wxWindowID id = wxID_ANY, long 	style = wxSTB_DEFAULT_STYLE)
    : wxStatusBar(parent, id, style)
    {}
  virtual ~CPWStatusBar() {}

  void Setup()
  {
    const int widths[SB_LAST] = {-6, -3, -1, -3, -2, -1};
    SetFieldsCount(SB_LAST, widths);
  }
};
#endif /* __PWSTATUSBAR_H */
