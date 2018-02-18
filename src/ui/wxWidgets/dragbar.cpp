/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file dragbar.cpp
*
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

////@begin includes
#include "dragbar.h"
#include "../../os/pws_tchar.h"
#include "../../core/PwsPlatform.h"
#include "../../core/ItemData.h"
#include "./wxutils.h"
#include <wx/dnd.h>
////@end includes

#include <algorithm>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

BEGIN_EVENT_TABLE( CDragBar, wxControl )
  EVT_LEFT_DOWN(CDragBar::OnLeftDown)
  EVT_PAINT(CDragBar::OnPaint)
  EVT_MOTION(CDragBar::OnMouseMove)
  EVT_LEAVE_WINDOW(CDragBar::OnMouseLeave)
END_EVENT_TABLE()

IMPLEMENT_CLASS( CDragBar, wxControl )

CDragBar::CDragBar(wxFrame* parent, IDragSourceTextProvider* provider,
                                wxOrientation orient /*= wxHORIZONTAL*/) :
                                                wxControl(parent, wxID_ANY),
                                                m_margins(5, 3),
                                                m_orientation(orient),
                                                m_bmpWidth(0),
                                                m_bmpHeight(0),
                                                m_provider(provider)
{
  Connect(GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CDragBar::OnUpdateUI));
}

CDragBar::~CDragBar()
{
}

void CDragBar::OnLeftDown(wxMouseEvent& evt)
{
  const int idx = FindToolFromCoords(evt.GetPosition());
  if (idx == -1)
    return;

  wxASSERT(idx >= 0 && size_t(idx) < m_items.size());

  wxString text = m_provider->GetText(m_items[idx].id);
  if (!text.IsEmpty()) {
    wxTextDataObjectEx dataObj(text);
    wxDropSource source(dataObj, this);//, wxDROP_ICON(DragbarElements[idx].name));
    switch (source.DoDragDrop()) {
      case wxDragError:
        wxLogDebug(_("Error dragging"));
        break;
      case wxDragNone:
        wxLogDebug(_("Nothing happened dragging"));
        break;
      case wxDragCopy:
        wxLogDebug(_("Copied successfully"));
        break;
      case wxDragMove:
        wxLogDebug(_("Moved successfully"));
        break;
      case wxDragCancel:
        wxLogDebug(_("Dragging cancelled"));
        break;
      default:
        wxLogDebug(_("Unexpected result dragging"));
        break;
    }
  }
}

void CDragBar::AddTool(int id, const wxBitmap& bmp, const wxString& tooltip /*= wxEmptyString*/,
                                    const wxBitmap& bmpDisabled /*= wxNullBitmap*/)
{
  //all bitmaps must be same size
  if (m_bmpWidth == 0)
    m_bmpWidth = bmp.GetWidth();
  else {
    wxASSERT(m_bmpWidth == bmp.GetWidth());
  }

  if (m_bmpHeight == 0)
    m_bmpHeight = bmp.GetHeight();
  else {
    wxASSERT(m_bmpHeight == bmp.GetHeight());
  }

  if (bmpDisabled.IsOk()) {
    wxASSERT(m_bmpWidth == bmpDisabled.GetWidth());
    wxASSERT(m_bmpHeight == bmpDisabled.GetHeight());
  }

  DragBarItem item;
  item.id = id;
  item.bmp = bmp;
  item.bmpDisabled = bmpDisabled;
  item.tooltip = tooltip;
  item.enabled = true;

  m_items.push_back( item );
}

int CDragBar::FindToolFromCoords(const wxPoint& pt)
{
  int idx = -1;

  switch(m_orientation) {
    case wxHORIZONTAL:
    {
      int w = m_bmpWidth + m_margins.GetWidth();
      if ((pt.x % w) < m_margins.GetWidth())
        return -1;
      else
        idx = pt.x/w;
      break;

    }
    case wxVERTICAL:
    {
      int h = m_bmpHeight + m_margins.GetHeight();
      if ((pt.y % h) < m_margins.GetHeight())
        return -1;
      else
        idx = pt.y/h;
      break;
    }
    default:
      wxFAIL_MSG(wxT("Unknown type of dragbar orientation"));
      return -1;
  }

  return (idx < 0 || size_t(idx) >= m_items.size()) ? -1 : idx;
}

wxSize CDragBar::GetInvalidatedIconRange(const wxRect& rect)
{
  switch(m_orientation) {
    case wxHORIZONTAL:
    {
      int first = FindToolFromCoords(rect.GetTopLeft());
      if (first == -1)
        first = FindToolFromCoords(rect.GetTopLeft() + wxSize(m_margins.GetWidth(), 0));

      int last = FindToolFromCoords(rect.GetTopRight());
      if (last == -1) {
        last = FindToolFromCoords(rect.GetTopRight() - wxSize(m_margins.GetWidth(), 0));
        if (last == -1)
          last = static_cast<int>(m_items.size() - 1);
      }

      return wxSize(first, last);
    }
    case wxVERTICAL:
    {
      int first = FindToolFromCoords(rect.GetTopLeft());
      if (first == -1)
        first = FindToolFromCoords(rect.GetTopLeft() + wxSize(0, m_margins.GetHeight()));

      int last = FindToolFromCoords(rect.GetBottomLeft());
      if (last == -1) {
        last = FindToolFromCoords(rect.GetBottomLeft() - wxSize(0, m_margins.GetHeight()));
        if (last == -1)
          last = static_cast<int>(m_items.size() - 1);
      }

      return wxSize(first, last);
    }
    default:
      wxFAIL_MSG(wxT("m_orientation not initialized correctly"));
      return wxSize(0, 0);
  }
}
void CDragBar::OnPaint(wxPaintEvent& /*evt*/)
{
  wxRect rcWin = GetRect(); //draw along the entire window rect, since clipping rect is always (0, 0, -1, -1)

  wxPaintDC dc(this);
  dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));

  //draw the dragbar background, which some platforms don't handle automatically
  dc.DrawRectangle(rcWin);

  wxPen shadow(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW));
  dc.SetPen(shadow);

  //draw a shadow along the bottom or the right edge, depending on orientation
  if (m_orientation == wxHORIZONTAL)
    dc.DrawLine(rcWin.GetBottomLeft(), rcWin.GetBottomRight());
  else
    dc.DrawLine(rcWin.GetTopRight(), rcWin.GetBottomRight());

  //remove the shadown pen
  dc.SetPen(wxNullPen);

  wxRect rc;
  dc.GetClippingBox(rc);
  //wxLogDebug(wxT("Clipping box = [(%d, %d) (%d %d)]"), rc.GetLeft(), rc.GetTop(), rc.GetRight(), rc.GetBottom());

  wxSize range = GetInvalidatedIconRange(rc);

  wxASSERT(range.x == -1 || (range.x >= 0 && size_t(range.x) < m_items.size()));
  wxASSERT(range.y == -1 || (range.y >= range.x && size_t(range.y) < m_items.size()));

  if (range.x >= 0) {
    //wxLogDebug(wxT("Painting dragbar icons %d - %d"), range.x + 1, range.y + 1);
    for (int idx = range.x; idx <= range.y; ++idx) {
      dc.DrawBitmap(m_items[idx].enabled? m_items[idx].bmp : m_items[idx].bmpDisabled,
                          GetToolX(idx), GetToolY(idx), true);
    }
  }
  else {
    //wxLogDebug(wxT("all dragbar icons valid"));
  }

}

wxSize CDragBar::DoGetBestSize() const
{
  switch(m_orientation) {
    case wxHORIZONTAL:
      return wxSize(std::max(GetToolX(m_items.size()), GetParent()->GetSize().GetWidth()), 2*m_margins.GetHeight() + m_bmpHeight + 10);
    case wxVERTICAL:
      return wxSize(2*m_margins.GetWidth() + m_bmpWidth + 1, std::max(GetParent()->GetSize().GetHeight(), GetToolY(m_items.size())));
    default:
      wxFAIL_MSG(wxT("m_orientation not initialized correctly"));
      return wxDefaultSize;
  }
}

void RemoveToolTip(wxWindow* win)
{
  //none of these work
  win->SetToolTip(wxEmptyString);
  win->SetToolTip(nullptr); // == UnsetToolTip()
}

void CDragBar::OnMouseMove(wxMouseEvent& evt)
{
  if (!evt.Dragging()) {
    int idx = FindToolFromCoords(evt.GetPosition());
    if (idx >= 0 && size_t(idx) < m_items.size())
      SetToolTip(m_items[idx].tooltip);
    else {
      RemoveToolTip(this);
      //wxLogDebug(wxT("Removed tooltip, idx was %d"), idx);
    }
  }
  else {
    RemoveToolTip(this);
    //wxLogDebug(wxT("Removed tooltip for dragging"));
  }
}

void CDragBar::OnMouseLeave(wxMouseEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  RemoveToolTip(this);
  //wxLogDebug(wxT("Removed tooltip since mouse left the window"));
}

void CDragBar::OnUpdateUI(wxUpdateUIEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
  for (size_t idx = 0; idx < m_items.size(); ++idx) {
    const bool status = m_provider->IsEnabled(m_items[idx].id);
    if (status != m_items[idx].enabled) {
      m_items[idx].enabled = status;
      RefreshRect( wxRect( wxPoint(GetToolX(idx), GetToolY(idx)), wxSize(m_bmpWidth, m_bmpHeight)), false );
    }
  }
}

void CDragBar::SetToolBitmaps(int id, const wxBitmap& bmp, const wxBitmap& bmpDisabled /*= wxNullBitmap*/)
{
  for (size_t idx = 0; idx < m_items.size(); ++idx) {
    if (m_items[idx].id == id) {
      m_items[idx].bmp = bmp;
      m_items[idx].bmpDisabled = bmpDisabled;
      RefreshRect( wxRect( wxPoint(GetToolX(idx), GetToolY(idx)), wxSize(m_bmpWidth, m_bmpHeight)), true );
      break;
    }
  }
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DragBarItemsArray)
