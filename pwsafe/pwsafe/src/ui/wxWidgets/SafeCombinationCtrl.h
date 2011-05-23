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
