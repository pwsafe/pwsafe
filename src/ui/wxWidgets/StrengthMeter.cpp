/*
 * Initial version created as 'StrengthMeter.cpp'
 * by rafaelx on 2025-05-01.
 *
 * Copyright (c) 2019-2025 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "StrengthMeter.h"

wxBEGIN_EVENT_TABLE(StrengthMeter, wxControl)
  EVT_PAINT(StrengthMeter::OnPaint)
  EVT_SIZE(StrengthMeter::OnSize)
wxEND_EVENT_TABLE()

StrengthMeter::StrengthMeter(wxWindow* parent, wxWindowID id,
                             const wxPoint& pos, const wxSize& size)
  : wxControl(parent, id, pos, size, wxBORDER_NONE)
  , m_strength(STRENGTH_MIN)
  , m_weakColor(wxColor(255, 75, 75))    // Red
  , m_mediumColor(wxColor(255, 165, 0))  // Orange
  , m_strongColor(wxColor(0, 175, 0))    // Green
  , m_weakLabel(_("Weak Password"))
  , m_mediumLabel(_("Medium Password"))
  , m_strongLabel(_("Strong Password"))
{
  SetBackgroundStyle(wxBG_STYLE_PAINT);
  SetMinSize(wxSize(200, 10));
}

void StrengthMeter::SetStrength(int strength)
{
  if (strength < STRENGTH_MIN) {
    m_strength = STRENGTH_MIN;
  }
  else if (strength > STRENGTH_MAX) {
    m_strength = STRENGTH_MAX;
  }
  else {
    m_strength = strength;
  }
  Refresh();
  SetToolTip(GetLabelForStrength());
}

void StrengthMeter::SetColors(const wxColor& weak, const wxColor& medium, const wxColor& strong)
{
  m_weakColor = weak;
  m_mediumColor = medium;
  m_strongColor = strong;
  Refresh();
}

void StrengthMeter::SetLabels(const wxString& weak, const wxString& medium, const wxString& strong)
{
  m_weakLabel = weak;
  m_mediumLabel = medium;
  m_strongLabel = strong;
  Refresh();
}

wxColor StrengthMeter::GetColorForStrength() const
{
  if (IsWeakStrength()) {
    return m_weakColor;
  }
  else if (IsMediumStrength()) {
    return m_mediumColor;
  }
  else {
    return m_strongColor;
  }
}

wxString StrengthMeter::GetLabelForStrength() const
{
  if (!HasStrength()) {
    return wxEmptyString;
  }
  else if (IsWeakStrength()) {
    return m_weakLabel;
  }
  else if (IsMediumStrength()) {
    return m_mediumLabel;
  }
  else {
    return m_strongLabel;
  }
}

void StrengthMeter::OnPaint(wxPaintEvent& event)
{
  wxPaintDC dc(this);
  auto size = GetSize();
    
  // Draw background
  auto bgColor = GetBackgroundColour();
  dc.SetBrush(wxBrush(bgColor));
  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());

  // Calculate metrics
  int barX = 1, barY = 0, radius = 3;
  int barHeight = size.GetHeight();
  int barWidth = static_cast<int>((size.GetWidth() - 1) * (m_strength / 100.0));
    
  // Draw meter background
  if (wxSystemSettings::GetAppearance().IsDark()) {
#ifndef __WXMAC__
    // Use background color of text controls when dark theme is in use
    auto brushLightness = 104;
    auto bgListbox = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX);
    dc.SetBrush(wxBrush(bgListbox.ChangeLightness(brushLightness)));
#endif // __WXMAC__
  }
  else {
    // When using a light theme, reduce the brightness so that the background appears slightly darker
    auto brushLightness = 95;
    dc.SetBrush(wxBrush(bgColor.ChangeLightness(brushLightness)));
  }
  auto penLightness = wxSystemSettings::GetAppearance().IsDark() ? 125 : 80;
  dc.SetPen(wxPen(bgColor.ChangeLightness(penLightness)));
  dc.DrawRoundedRectangle(barX, barY, size.GetWidth() - 1, barHeight, radius);

  // Draw strength bar
  if (barWidth > 0) {
    auto barColor = GetColorForStrength();
    dc.SetBrush(wxBrush(barColor));
    dc.SetPen(wxPen(barColor.ChangeLightness(75)));
    dc.DrawRoundedRectangle(barX, barY, barWidth, barHeight, radius);
  }
}

void StrengthMeter::OnSize(wxSizeEvent& event)
{
  Refresh();
  event.Skip();
}
