/*
 * Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
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

#include "../../core/StringX.h"

class PWSQRCodeDlg : public wxDialog {

  DECLARE_CLASS(PWSQRCodeDlg)
  DECLARE_EVENT_TABLE()

public:
  PWSQRCodeDlg(wxWindow* parent,
		  	   const StringX &data,
			   const wxString& description,
			   const int seconds = 15,
			   const wxString &dlgTitle = wxT("Scan the QR code quickly"),
			   const wxPoint &pos=wxDefaultPosition,
			   const wxSize &size=wxDefaultSize,
			   long style=wxDEFAULT_DIALOG_STYLE,
			   const wxString &name=wxDialogNameStr);

  ~PWSQRCodeDlg() = default;
  
  //void OnInitDialog(wxInitDialogEvent& evt)  override;
  //void OnRelayoutDlg(wxCommandEvent& evt)  override;  
  void CreateControls(const StringX &data, const wxString &description);
  void OnClose(wxCommandEvent & evt);
};

