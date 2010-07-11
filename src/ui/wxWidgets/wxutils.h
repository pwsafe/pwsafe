////////////////////////////////////////////////////////////////////
// wxutils.h - file for various wxWidgets related utility functions, 
// macros, classes, etc


#ifndef __WXUTILS_H__
#define __WXUTILS_H__

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

#endif

