/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef __ExternalKeyboardButton__
#define __ExternalKeyboardButton__

#include <wx/bmpbuttn.h> // Base class: wxBitmapButton

class ExternalKeyboardButton : public wxBitmapButton {

public:
  ExternalKeyboardButton(wxWindow* parent, wxWindowID id = wxID_ANY,
                                            const wxPoint& pos = wxDefaultPosition,
                                            const wxSize& size = wxDefaultSize, 
                                            long style = wxBU_AUTODRAW, 
                                            const wxValidator& validator = wxDefaultValidator, 
                                            const wxString& name = wxT("button"));
  ~ExternalKeyboardButton();

  void HandleCommandEvent(wxCommandEvent& evt);

};

#endif // __ExternalKeyboardButton__
