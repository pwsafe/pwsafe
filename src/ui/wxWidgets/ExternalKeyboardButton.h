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
