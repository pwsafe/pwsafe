////////////////////////////////////////////////////////////////////
// wxutils.h - file for various wxWidgets related utility functions, 
// macros, classes, etc


#ifndef __WXUTILS_H__
#define __WXUTILS_H__

#include "../../corelib/StringX.h"
#include "../../corelib/PWSprefs.h"

inline wxString& operator << ( wxString& str, const wxPoint& pt) {
  return str << wxT('[') << pt.x << wxT(',') << pt.y << wxT(']');
}

inline wxString& operator << ( wxString& str, const wxSize& sz) {
  return str << wxT('[') << sz.GetWidth() << wxT(',') << sz.GetHeight() << wxT(']');
}

inline wxString& operator << ( wxString& str, const StringX& s) {
  return str << s.c_str();
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
    wxClassInfo name<templatename>::ms_classInfo(wxT(#name "<" #templatename ">"),      \
            &basename::ms_classInfo,                                                    \
            NULL,                                                                       \
            (int) sizeof(name),                                                         \
            (wxObjectConstructorFn) NULL);                                              \
                                                                                        \
    template <>                                                                         \
    wxClassInfo* name<templatename>::GetClassInfo() const                               \
        { return &name<templatename>::ms_classInfo; }

#endif

