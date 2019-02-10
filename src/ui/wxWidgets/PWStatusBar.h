/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
  enum class Field { 
    DOUBLECLICK = 0, CLIPBOARDACTION,
    MODIFIED, READONLY, NUM_ENT, FILTER,
    COUNT
  };

  CPWStatusBar(wxWindow *parent, wxWindowID id = wxID_ANY, long style = wxSTB_DEFAULT_STYLE)
    : wxStatusBar(parent, id, style)
    {}

  virtual ~CPWStatusBar() {}

  void Setup()
  {
    const int FIELDS = static_cast<std::underlying_type<Field>::type>(Field::COUNT);
    const int widths[FIELDS] = { -6, -3, -1, -1, -2, -1 };
    SetFieldsCount(FIELDS, widths);
    SetFont(GetFont().MakeSmaller());
  }

  void SetStatusText(const wxString &text, Field field)
  {
    wxStatusBar::SetStatusText(text, static_cast<std::underlying_type<Field>::type>(field));
  }
};

#endif /* __PWSTATUSBAR_H */
