#ifndef __SAFECOMBINATIONCTRL_H__
#define __SAFECOMBINATIONCTRL_H__

#include <wx/sizer.h> // Base class: wxBoxSizer
#include <wx/event.h> // Base class: wxEvtHandler

//without this class, we get 'pointer to member conversion via virtual base' error
class CommandEventHandler : public wxEvtHandler {
public:
  void HandleCommandEvent(wxCommandEvent& evt);
};

class CSafeCombinationCtrl : virtual public wxBoxSizer, virtual CommandEventHandler {

public:
  CSafeCombinationCtrl(wxWindow* parent, wxWindowID textCtrlID = wxID_ANY, wxString* valPtr = 0,
                        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
  ~CSafeCombinationCtrl();
  
  //so that other have easy access to it
  wxTextCtrl* textCtrl;

};

#endif // __SAFECOMBINATIONCTRL_H__
