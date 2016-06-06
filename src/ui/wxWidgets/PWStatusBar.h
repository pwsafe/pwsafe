/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
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
  CPWStatusBar(wxWindow *parent, wxWindowID id = wxID_ANY, long 	style = wxSTB_DEFAULT_STYLE)
    : wxStatusBar(parent, id, style)
    {}
};
#endif /* __PWSTATUSBAR_H */
