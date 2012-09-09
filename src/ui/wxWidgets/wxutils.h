/*
 * Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

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

// Workaround for wxTE_PASSWORD being immutable:
void ShowHideText(wxTextCtrl *&txtCtrl, const wxString &text,
                  wxSizer *sizer, bool show);

//ensures at least one of the checkboxes are selected
class MultiCheckboxValidator: public wxValidator
{
 protected:
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
 * This class injects context data into an event, to avoid having
 * to "remember" that data in member variables.  It does that by
 * hooking itself in as a dynamic event handler entry into the
 * event source, which are called before other types of handlers
 */
template <class evtClass>
class EventDataInjector: public wxEvtHandler
{
  wxEvtHandler* m_evtSource;
  void* m_clientData;
  wxEventType m_eventType;
  int m_windowId;
  bool  m_oneShot;
public:
  EventDataInjector(wxEvtHandler* evtSource, void* clientData,
                    wxEventType evtType, int winid = wxID_ANY,
                    bool oneShot = true): m_evtSource(evtSource),
                                          m_clientData(clientData),
                                          m_eventType(evtType),
                                          m_windowId(winid),
                                          m_oneShot(oneShot)
  {    
    evtSource->Connect(winid, evtType, 
                  evtType,
                  (wxObjectEventFunction)&EventDataInjector::OnHookedEvent,
                  NULL, //this is for wxWidgets' private use only
                  this);
  }

  void OnHookedEvent(evtClass& evt) {
    evt.Skip();
    wxCHECK_RET(!evt.GetClientData(), wxT("Command event already has client data"));
    evt.SetClientData(m_clientData);
    if (m_oneShot) {
      wxCHECK_RET(m_evtSource->Disconnect(m_windowId, m_eventType,
                              (wxObjectEventFunction)&EventDataInjector::OnHookedEvent,
                              NULL, this), wxT("Could not remove dynamic event parameter injection hook"));
    }
  }
};

int pless(int* first, int* second);

template <typename T>
class AutoRestore
{
  T& m_ref;
  T  m_oldVal;
public:
  AutoRestore(T& var, T newVal): m_ref(var), m_oldVal(var)
  {
    var = newVal;
  }
  ~AutoRestore()
  {
    m_ref = m_oldVal;
  }
};

#endif

