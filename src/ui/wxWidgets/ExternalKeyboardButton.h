/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ExternalKeyboardButton.h
 *
 */

#ifndef _ExternalKeyboardButton_
#define _ExternalKeyboardButton_

#include "SafeCombinationCtrl.h"

#include <wx/bmpbuttn.h> // Base class: wxBitmapButton

class ExternalKeyboardButton : public wxBitmapButton
{
public:
  ExternalKeyboardButton(wxWindow *parent, wxWindowID id = wxID_ANY,
                         const wxPoint &pos = wxDefaultPosition,
                         const wxSize &size = wxDefaultSize,
                         long style = wxBU_AUTODRAW,
                         const wxValidator &validator = wxDefaultValidator,
                         const wxString &name = wxT("button"));

  ExternalKeyboardButton(const ExternalKeyboardButton&) = delete;
  ExternalKeyboardButton& operator=(const ExternalKeyboardButton&) = delete;
  ~ExternalKeyboardButton();

  void SetFocusOnSafeCombinationCtrl(SafeCombinationCtrl *ctrl) { m_TargetSafeCombinationCtrl = ctrl; }

protected:
  void HandleCommandEvent(wxCommandEvent &evt);

private:
  SafeCombinationCtrl *m_TargetSafeCombinationCtrl = nullptr;
};

#endif // _ExternalKeyboardButton_
