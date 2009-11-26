////////////////////////////////////////////////////////////////////
// wxutils.h - file for various wxWidgets related utility functions, 
// macros, classes, etc


#ifndef __WXUTILS_H__
#define __WXUTILS_H__

wxString& operator << ( wxString& str, const wxPoint& pt) {
  return str << wxT('[') << pt.x << wxT(',') << pt.y << wxT(']');
}

wxString& operator << ( wxString& str, const wxSize& sz) {
  return str << wxT('[') << sz.GetWidth() << wxT(',') << sz.GetHeight() << wxT(']');
}

#endif

