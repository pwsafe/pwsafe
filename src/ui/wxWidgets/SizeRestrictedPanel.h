/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SizeRestrictedPanel.h
* 
*/

#ifndef _SIZERESTRICTEDPANEL_H_
#define _SIZERESTRICTEDPANEL_H_

#include <wx/panel.h>

class SizeRestrictedPanel : public wxPanel
{
  //this is the parent whose size we must use for size calculations if we are ourselves hidden
  wxWindow* m_sizingParent;

  public:
    SizeRestrictedPanel(wxWindow* parent, wxWindow* sizingParent, wxWindowID id = wxID_ANY);
    wxSize GetWindowSizeForVirtualSize(const wxSize& size) const;
};

#endif // _SIZERESTRICTEDPANEL_H_
