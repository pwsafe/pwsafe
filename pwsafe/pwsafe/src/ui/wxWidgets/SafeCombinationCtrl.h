/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef __SAFECOMBINATIONCTRL_H__
#define __SAFECOMBINATIONCTRL_H__

#include <wx/sizer.h> // Base class: wxBoxSizer
#include <wx/event.h> // Base class: wxEvtHandler
#include "../../core/StringX.h"

//without this class, we get 'pointer to member conversion via virtual base' error
class CommandEventHandler : public wxEvtHandler {
public:
  void HandleCommandEvent(wxCommandEvent& evt);
};

class CSafeCombinationCtrl : virtual public wxBoxSizer, virtual CommandEventHandler {

public:
  CSafeCombinationCtrl(wxWindow* parent, wxWindowID textCtrlID = wxID_ANY, StringX* valPtr = 0,
                        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
  ~CSafeCombinationCtrl();
  
  StringX GetCombination() const;
  void SetValidatorTarget(StringX* str);
  void SelectCombinationText() const;

private:
  wxTextCtrl* textCtrl;

};

#endif // __SAFECOMBINATIONCTRL_H__
