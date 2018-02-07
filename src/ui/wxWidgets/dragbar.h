/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file dragbar.h
 *
 * Implements a generic toolbar-like class from which tool icons can be dragged and
 * dropped on targets accepting text drops.
 *
 * This class exists solely because I couldn't find a way to handle
 * left mouse-down events in wxToolBar class.  Also, finding the tool
 * on which a click happens (if it were possible) is a pain
 */

#ifndef __DRAGBAR_H__
#define __DRAGBAR_H__

#include <wx/control.h> // Base class: wxToolBar
#include <wx/vector.h>

struct DragBarItem {
  int id;
  wxBitmap bmp;
  wxBitmap bmpDisabled;
  wxString tooltip;
  bool     enabled;
};

WX_DECLARE_OBJARRAY(DragBarItem, DragBarItemsArray);

class CDragBar : public wxControl
{
  wxSize              m_margins;
  DragBarItemsArray   m_items;
  wxOrientation       m_orientation;
  int                 m_bmpWidth;
  int                 m_bmpHeight;

public:

  struct IDragSourceTextProvider {
    virtual wxString GetText(int id) const = 0;
    virtual bool IsEnabled(int id) const = 0;
    virtual ~IDragSourceTextProvider() {}
  };

  CDragBar(wxFrame* parent, IDragSourceTextProvider* provider, wxOrientation orient = wxHORIZONTAL);
  ~CDragBar();

  void AddTool(int id, const wxBitmap& bmp, const wxString& tooltip = wxEmptyString,
                                    const wxBitmap& bmpDisabled = wxNullBitmap);
  void ClearTools() { m_items.Empty(); }
  size_t GetToolsCount() const { return m_items.GetCount(); }
  void SetToolBitmaps(int id, const wxBitmap& bmp, const wxBitmap& bmpDisabled = wxNullBitmap);

  //overridden from wxWindow
  virtual wxSize DoGetBestSize() const ;

  void OnLeftDown(wxMouseEvent& evt);
  void OnPaint(wxPaintEvent& evt);
  void OnMouseMove(wxMouseEvent& evt);
  void OnMouseLeave(wxMouseEvent& evt);
  void OnUpdateUI(wxUpdateUIEvent& evt);

  DECLARE_CLASS(CDragBar)
  DECLARE_EVENT_TABLE()

private:
  IDragSourceTextProvider* m_provider;

  int FindToolFromCoords(const wxPoint& pt);
  wxSize GetInvalidatedIconRange(const wxRect& rect);

  int GetToolX(size_t idx) const {
    switch (m_orientation) {
      case wxHORIZONTAL:
        return m_margins.GetWidth() +
                    static_cast<int>(idx * (m_margins.GetWidth() + m_bmpWidth));
      case wxVERTICAL:
        return m_margins.GetWidth();

      default:
        wxASSERT(wxT("Invalid orientation")); return -1;
    }
  }

  int GetToolY(size_t idx) const {
    switch(m_orientation) {
      case wxHORIZONTAL:
        return m_margins.GetHeight();

      case wxVERTICAL:
        return m_margins.GetHeight() +
                    static_cast<int>(idx * (m_margins.GetHeight() + m_bmpHeight));

      default:
        wxASSERT(wxT("Invalid orientation")); return -1;
    }
  }

};

#endif // __DRAGBAR_H__
