/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file pwsqrcodedlg.h
 * 
 */

/*
 * A dialog class to display a QR code. The dialog closes automatically for security reasons.
 */

#include <wx/dialog.h>
#include <wx/timer.h>
#include <wx/stattext.h>

#include "../../core/StringX.h"

class PWSQRCodeDlg : public wxDialog {

  DECLARE_CLASS(PWSQRCodeDlg)
  DECLARE_EVENT_TABLE()

  wxTimer timer;
  int secondsRemaining;
  wxStaticText *secondsText = nullptr;

  void UpdateTimeRemaining();

public:
  PWSQRCodeDlg(wxWindow* parent,
		  	   const StringX &data,
			   const wxString& title,
			   const int seconds = 15,
			   const wxPoint &pos=wxDefaultPosition,
			   const wxSize &size=wxDefaultSize,
			   long style=wxDEFAULT_DIALOG_STYLE,
			   const wxString &name=wxDialogNameStr);

  ~PWSQRCodeDlg() = default;
  
  //void OnInitDialog(wxInitDialogEvent& evt)  override;
  //void OnRelayoutDlg(wxCommandEvent& evt)  override;  
  void CreateControls(const StringX &data);
  void OnClose(wxCommandEvent & evt);
  void OnTimer(wxTimerEvent &evt);
  void OnInitDialog(wxInitDialogEvent &evt);
};

constexpr bool HasQRCode() {
#ifndef NO_QR
  return true;
#else
  return false;
#endif
}

