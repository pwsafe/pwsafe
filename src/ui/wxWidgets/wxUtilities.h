/*
 * Copyright (c) 2003-2022 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file wxUtilities.h
* file for various wxWidgets related utility functions, macros, classes, etc
*/

#ifndef _WXUTILITIES_H_
#define _WXUTILITIES_H_

#include <wx/eventfilter.h>
#include <wx/dataobj.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/stream.h>
#include <wx/textctrl.h>
#include <wx/toplevel.h>

#include "core/ItemAtt.h"
#include "core/StringX.h"
#include "core/PWSprefs.h"

#include <set>

#if !wxCHECK_VERSION(3,1,0)
#define wxOVERRIDE
#endif

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

inline void ApplyFontPreference(wxWindow* window, const PWSprefs::StringPrefs fontPreference)
{
  switch (fontPreference)
  {
    case PWSprefs::StringPrefs::AddEditFont:
    case PWSprefs::StringPrefs::NotesFont:
    case PWSprefs::StringPrefs::PasswordFont:
    case PWSprefs::StringPrefs::TreeFont:
    {
      wxFont font(towxstring(PWSprefs::GetInstance()->GetPref(fontPreference)));
      if (font.IsOk()) {
        window->SetFont(font);
      }
    }
    break;
    default:
    wxASSERT_MSG(false, wxT("Wrong enumeration value."));
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
            nullptr,                                                                       \
            (int) sizeof(name),                                                         \
            (wxObjectConstructorFn) nullptr);                                              \
                                                                                        \
    template <>                                                                         \
    wxClassInfo* name<templatename>::GetClassInfo() const                               \
        { return &name<templatename>::ms_classInfo; }

enum {
      TopMargin     = 20,
      BottomMargin  = 20,
      SideMargin    = 20,
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

/* Skip events passed to constructor while object exists
*/
#if wxCHECK_VERSION(2,9,3)
class EventSkipper: public wxEventFilter {
  std::set<wxEventType> m_events;
public:
  EventSkipper(std::set<wxEventType> events): m_events(events) {
    wxEvtHandler::AddFilter(this);
  }

  ~EventSkipper() {
    wxEvtHandler::RemoveFilter(this);
  }

  virtual int FilterEvent(wxEvent& evt) {
    if (m_events.find(evt.GetEventType()) != m_events.end()) {
      pws_os::Trace(L"Event %d was ignored", evt.GetEventType());
      return Event_Ignore;
    }
    else {
      return Event_Skip; // here skip means normal processing
    }
  }
};
#endif

class PWScore;
int ReadCore(PWScore& othercore, const wxString& file, const StringX& combination,
                bool showMsgbox = true, wxWindow* msgboxParent = nullptr, bool setupCopy = false);

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
class EventDataInjector {
  wxEvtHandler* m_evtSource;
  void* m_clientData;
  wxEventType m_eventType;
  bool  m_oneShot;
public:
  EventDataInjector(wxEvtHandler* evtSource, void* clientData,
                    wxEventType evtType,
                    bool oneShot = true): m_evtSource(evtSource),
                                          m_clientData(clientData),
                                          m_eventType(evtType),
                                          m_oneShot(oneShot)
  {
    // no need for id, we bind directly to source
    evtSource->Bind(evtType,
                  &EventDataInjector::OnHookedEvent,
                  this);
  }

  void OnHookedEvent(wxEvent& event) {
    event.Skip();
    wxCommandEvent *cmdEvent = dynamic_cast<wxCommandEvent*>(&event);
    if (cmdEvent) {
      wxCHECK_RET(!cmdEvent->GetClientData(), wxT("Command event already has client data"));
      cmdEvent->SetClientData(m_clientData);
    }
    else {
      wxASSERT_MSG(false, wxT("Non-command event in EventDataInjector"));
    }
    if (m_oneShot) {
      wxCHECK_RET(m_evtSource->Unbind(m_eventType,
                              &EventDataInjector::OnHookedEvent,
                              this), wxT("Could not remove dynamic event parameter injection hook"));
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

/* Try to acquire critical section lock.
  If it's not possible, return immediately. Status could be checked by calling
  IsAcquired() */
#if wxCHECK_VERSION(2,9,3)
class wxCriticalSectionTryLocker {
public:
  wxCriticalSectionTryLocker(wxCriticalSection& cs): m_critsect(cs) {
    m_entered = m_critsect.TryEnter();
  }
  ~wxCriticalSectionTryLocker() {
    if (m_entered)
      m_critsect.Leave();
  }

  bool IsEntered() const {
    return m_entered;
  }
private:
  bool m_entered;
  wxCriticalSection& m_critsect;
  // no assignment operator nor copy ctor
  wxCriticalSectionTryLocker(const wxCriticalSectionTryLocker&);
  wxCriticalSectionTryLocker& operator=(const wxCriticalSectionTryLocker&);
};
#endif

/* Try to acquire mutex lock.
  If it's not possible, return immediately. Status could be checked by calling
  IsAcquired() */
class wxMutexTryLocker {
public:
  wxMutexTryLocker(wxMutex& mutex): m_mutex(mutex) {
    m_acquired = ( m_mutex.TryLock() == wxMUTEX_NO_ERROR );
  }

  bool IsAcquired() const {
    return m_acquired;
  }

  ~wxMutexTryLocker() {
    if (m_acquired)
      m_mutex.Unlock();
  }

private:
  bool m_acquired;
  wxMutex& m_mutex;
  // no assignment operator nor copy ctor
  wxMutexTryLocker(const wxMutexTryLocker&);
  wxMutexTryLocker& operator=(const wxMutexTryLocker&);
};

#ifdef __WXGTK20__
/* We need to add one more format to support DnD into Firefox, that wants
 "text/plain" and Firefox's clipboard with Wayland backend wants "text/plain;charset=utf-8",
(X11 backend works fine with STRING/UTF8_STRING in clipboard)
 https://developer.mozilla.org/en-US/docs/Web/Guide/HTML/Recommended_Drag_Types
 https://hg.mozilla.org/mozilla-central/file/tip/widget/gtk/nsDragService.cpp
 https://hg.mozilla.org/mozilla-central/file/tip/widget/gtk/nsClipboardWayland.cpp
 */

class wxTextDataObjectEx : public wxTextDataObject {
public:
  wxTextDataObjectEx(const wxString &text = wxEmptyString) : wxTextDataObject(text) {}

  virtual size_t GetFormatCount(Direction dir = Get) const wxOVERRIDE {
    // add one more format
    return wxTextDataObject::GetFormatCount(dir) + 2;
  }

  virtual void GetAllFormats(wxDataFormat *formats, Direction dir = Get) const wxOVERRIDE {
    wxTextDataObject::GetAllFormats(formats, dir);
    // set types for new format
    // for some reason "text/plain;charset=utf-8" don't work for DnD, but works for Wayland's clipboard,
    // and reverse
    size_t formatCount = wxTextDataObject::GetFormatCount(dir);
    formats[formatCount++].SetId("text/plain");
    formats[formatCount].SetId("text/plain;charset=utf-8");
  }
  // No need to override SetData, wxTextDataObject put "preferred format" text
  // into all formats
};
#else
typedef wxTextDataObject wxTextDataObjectEx;
#endif // __WXGTK20__

// Wrapper for wxTaskBarIcon::IsAvailable() that doesn't crash
// on Fedora or Ubuntu
bool IsTaskBarIconAvailable();

/**
 * Fixes a spinners initial, resp. minimum required size that is needed to fully show the control.
 */
void FixInitialSpinnerSize(wxSpinCtrl* control);

/**
 * Returns 'true' if the mime type description begins with 'image'.
 */
bool IsMimeTypeImage(const stringT &mimeTypeDescription);

/**
 * A panel in which the image resizes.
 *
 * @see https://wiki.wxwidgets.org/An_image_panel
 */
class ImagePanel : public wxPanel
{
public:
  ImagePanel(wxPanel *parent, const wxSize &size = wxDefaultSize);
  ~ImagePanel();

  void OnPaint(wxPaintEvent &event);
  void OnSize(wxSizeEvent &event);

  bool LoadFromAttachment(const CItemAtt& itemAttachment, wxWindow* parent, const wxString& messageBoxTitle);
  bool LoadFromFile(const wxString &file, wxBitmapType format = wxBITMAP_TYPE_ANY);
  bool LoadFromMemory(wxInputStream &stream);

  void Paint();

  void Clear();

protected:
  void Render(wxDC &dc);
  void DrawBitmapCentered(wxDC &dc, const wxSize &drawAreaSize, const wxSize &imageSize);
  void DetermineImageProperties(const wxImage &image);

private:
  int m_ImageWidth;           // The non-scaled width of the image.
  int m_ImageHeight;          // The non-scaled height of the image.
  double m_ImageAspectRatio;  // The image's aspect ratio.
  wxImage m_Image;            // The non-scaled image.
  wxBitmap m_Bitmap;          // The possibly scaled bitmap representation of the image.
};

bool IsCloseInProgress();

template <class Dlg, typename... Args> int ShowModalAndGetResult(Args&&... args) {
  Dlg* ptr = Dlg::Create(std::forward<Args>(args)...);
  int result = ptr->ShowModal();
  ptr->Destroy();
  return result;
}

template<class T> class DestroyWrapper {
public:
  template<typename... Args>
  DestroyWrapper(Args&&... args) : m_ptr(T::Create(std::forward<Args>(args)...)) {
  }

  ~DestroyWrapper() {
    if (m_ptr) {
      m_ptr->Destroy();
    }
  }

  T* Get() {
    return m_ptr;
  }
private:
  T *m_ptr;
};

#endif // _WXUTILITIES_H_
