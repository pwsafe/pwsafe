/*
 * Initial version created as 'StrengthMeter.h'
 * by rafaelx on 2025-05-01.
 *
 * Copyright (c) 2019-2025 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef STRENGTH_METER_H
#define STRENGTH_METER_H

#include <wx/wx.h>

class StrengthMeter : public wxControl {
public:
  static const int STRENGTH_MIN;
  static const int STRENGTH_WEEK;
  static const int STRENGTH_MEDIUM;
  static const int STRENGTH_MAX;

public:
  StrengthMeter(wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize);

  // Set strength value (0-100)
  void SetStrength(int strength);
  int GetStrength() const { return m_strength; }

  // Set custom colors if needed
  void SetColors(const wxColor& weak, const wxColor& medium, const wxColor& strong);

  // Set custom labels
  void SetLabels(const wxString& weak, const wxString& medium, const wxString& strong);

private:
  void OnPaint(wxPaintEvent& event);
  void OnSize(wxSizeEvent& event);
  wxColor GetColorForStrength() const;
  wxString GetLabelForStrength() const;

  bool HasStrength() const { return m_strength > STRENGTH_MIN; }
  bool IsWeekStrength() const { return m_strength < STRENGTH_WEEK; }
  bool IsMediumStrength() const { return m_strength < STRENGTH_MEDIUM; }

  int m_strength;
  wxColor m_weakColor;
  wxColor m_mediumColor;
  wxColor m_strongColor;
  wxString m_weakLabel;
  wxString m_mediumLabel;
  wxString m_strongLabel;

  wxDECLARE_EVENT_TABLE();
};

#endif // STRENGTH_METER_H
