/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file QRCodeDlg.h
 * 
 */

/*
 * A dialog class to display a QR code. The dialog closes automatically for security reasons.
 */

#ifndef _QRCODEDLG_H_
#define _QRCODEDLG_H_

#include <wx/dialog.h>
#include <wx/timer.h>
#include <wx/stattext.h>

#include "../../core/StringX.h"

class QRCodeDlg : public wxDialog
{
  DECLARE_CLASS(QRCodeDlg)
  DECLARE_EVENT_TABLE()

  wxTimer timer;
  int secondsRemaining;
  wxStaticText *secondsText = nullptr;

  void UpdateTimeRemaining();

public:
  static QRCodeDlg* Create(wxWindow *parent, 
            const StringX &data,
            const wxString& title,
            const int seconds = 15,
            const wxPoint &pos=wxDefaultPosition,
            const wxSize &size=wxDefaultSize,
            long style=wxDEFAULT_DIALOG_STYLE,
            const wxString &name=wxDialogNameStr);

  ~QRCodeDlg() = default;
protected:
  QRCodeDlg(wxWindow *parent, const StringX &data, const wxString& title,
            const int seconds, 
            const wxPoint &pos, const wxSize &size, long style,
            const wxString &name);
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

#endif // _QRCODEDLG_H_
