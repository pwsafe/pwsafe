////////////////////////////////////////////////////////////////////
// wxutils.h - file for various wxWidgets related utility functions, 
// macros, classes, etc


#ifndef __WXUTILS_H__
#define __WXUTILS_H__

#include "../../core/StringX.h"
#include "../../core/PWSprefs.h"

inline wxString& operator << ( wxString& str, const wxPoint& pt) {
  return str << wxT('[') << pt.x << wxT(',') << pt.y << wxT(']');
}

inline wxString& operator << ( wxString& str, const wxSize& sz) {
  return str << wxT('[') << sz.GetWidth() << wxT(',') << sz.GetHeight() << wxT(']');
}

inline wxString towxstring(const StringX& str) {
  return wxString(str.data(), str.size());
}

inline wxString towxstring(const stringT& str) {
	return wxString(str.data(), str.size());
}

inline stringT tostdstring(const wxString& str) {
#if wxCHECK_VERSION(2,9,1)
  return str.ToStdWstring();
#else
  return stringT(str.data(), str.size());
#endif
}

inline StringX tostringx(const wxString& str) {
  return StringX(str.data(), str.size());
}

inline wxString& operator<<(wxString& w, const StringX& s) {
  return w << towxstring(s);
}

inline wxString& operator<<(wxString& w, const stringT& s) {
  return w << towxstring(s);
}

inline void ApplyPasswordFont(wxWindow* win)
{
  wxFont passwordFont(towxstring(PWSprefs::GetInstance()->GetPref(PWSprefs::PasswordFont)));
  if (passwordFont.IsOk()) {
    win->SetFont(passwordFont);
  }
}

//
// Macro to add RTTI to class templates that use DECLARE_CLASS macro
//
//This is required because these are really just specializations of templates
//and therefore we need to specify the template parameters (the "template <>" part)
//
// Used by help subsystem to figure out class names from the template specializations
//
//Copied (and modified) from <wx/object.h>
//
#define IMPLEMENT_CLASS_TEMPLATE(name, basename, templatename)                                \
    template <>                                                                         \
    wxClassInfo name<templatename>::ms_classInfo(wxSTRINGIZE_T(name) wxSTRINGIZE_T(<) wxSTRINGIZE_T(templatename) wxSTRINGIZE_T(>),      \
            &basename::ms_classInfo,                                                    \
            NULL,                                                                       \
            (int) sizeof(name),                                                         \
            (wxObjectConstructorFn) NULL);                                              \
                                                                                        \
    template <>                                                                         \
    wxClassInfo* name<templatename>::GetClassInfo() const                               \
        { return &name<templatename>::ms_classInfo; }

enum {
      TopMargin     = 20,
      BottomMargin  = 20,
      SideMargin    = 30,
      RowSeparation = 10,
      ColSeparation = 20
};

extern bool MergeSyncGTUCompare(const StringX &elem1, const StringX &elem2);

class EventHandlerDisabler {
  wxEvtHandler* m_handler;
  bool m_enabled;
public:
  EventHandlerDisabler(wxEvtHandler* h): m_handler(h), m_enabled(h->GetEvtHandlerEnabled()){
    if (m_enabled)
      h->SetEvtHandlerEnabled(false);
  }
  ~EventHandlerDisabler() {
    if (m_enabled)
      m_handler->SetEvtHandlerEnabled(true);
  }
};

class PWScore;
int ReadCore(PWScore& othercore, const wxString& file, const StringX& combination, 
                bool showMsgbox = true, wxWindow* msgboxParent = NULL);

inline const wxChar* ToStr(const wxString& s) {
  if (s == wxEmptyString) {
    return wxT("wxEmptyString");
  }
  else {
    return s.GetData();
  }
}

inline const wxChar* ToStr(bool b) {
  return b? wxT("True"): wxT("False");
}

void HideWindowRecursively(wxTopLevelWindow* win, wxWindowList& hiddenWindows);
void ShowWindowRecursively(wxWindowList& hiddenWindows);

//ensures at least one of the checkboxes are selected
class MultiCheckboxValidator: public wxValidator
{
  int* m_ids;
  size_t m_count;
  wxString m_msg, m_title;

public:
  MultiCheckboxValidator(int ids[], size_t num, const wxString& msg, const wxString& title);
  MultiCheckboxValidator(const MultiCheckboxValidator&);
  ~MultiCheckboxValidator();

  virtual wxObject* Clone() const;
  virtual bool Validate(wxWindow* parent);
  //note that you need to derive from this class if
  //the following two functions ought to do anything useful
  virtual bool TransferToWindow() { return true; }
  virtual bool TransferFromWindow() { return true; }

};

/*
 * This class injects context data from the time a popup menu
 * is created, into the wxCommandEvent objects arising out of menu
 * clicks
 */
class MenuEventModifier: public wxEvtHandler
{
  void* m_clientData;
public:
  MenuEventModifier(wxMenu* menu, void* clientData): m_clientData(clientData)
  {    
    menu->Connect(wxID_ANY,
                  wxEVT_COMMAND_MENU_SELECTED,
                  wxCommandEventHandler(MenuEventModifier::OnCommandEvent),
                  NULL, //this is for wxWidgets' private use only
                  this);
  }

  void OnCommandEvent(wxCommandEvent& evt) {
    evt.Skip();
    wxCHECK_RET(!evt.GetClientData(), wxT("Command event already has client data"));
    evt.SetClientData(m_clientData);
  }
};

#endif

