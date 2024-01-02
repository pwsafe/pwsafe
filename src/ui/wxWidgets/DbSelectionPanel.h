/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file DbSelectionPanel.h
* 
*/

#ifndef _DBSELECTIONPANEL_H_
#define _DBSELECTIONPANEL_H_

#include <wx/panel.h>
#include "../../core/StringX.h"
#ifndef NO_YUBI
#include "YubiMixin.h"
#endif

class wxFilePickerCtrl;
class SafeCombinationCtrl;
class PWScore;
class wxFileDirPickerEvent;

/*
 * This is a re-usable class for having the user select a db and
 * enter its combination.  The file picker ctrl is shown in the top
 * row and a masked textctrl + virtual keyboard button for entering
 * the combination in the second row.  It is meant to be used like
 * a child control by embedding in a wxSizer  See MergeDlg.cpp
 * and SyncWizard.cpp for its usage
 * 
 * filePrompt - the static text displayed just above the file picker ctrl
 * 
 * filePickerCtrlTitle - the window title of the file picker dialog
 * 
 * rowsep - the multiplying factor for the separation between the first and second
 * rows.  A small dialog might pass a value of 2, while a wizard page might pass 5
 */
#ifndef NO_YUBI
class DbSelectionPanel : public wxPanel, private YubiMixin
#else
  class DbSelectionPanel : public wxPanel
#endif
{
public:
  DbSelectionPanel(wxWindow* parent, const wxString& filePrompt,
                    const wxString& filePickerCtrlTitle, bool autoValidate,
                    PWScore* core, unsigned rowsep, int buttonConfirmationId = wxID_OK, const wxString filename = "");
  ~DbSelectionPanel() {};

  //Set the keyboard focus on combination entry box and select-all
  void SelectCombinationText();

  //wxWindow override to not validate in cases like going back in a wizard page
  virtual bool Validate() {
    return !m_bAutoValidate || DoValidation();
  }

  // Need to override this for Yubikey
  virtual bool TransferDataFromWindow();

  bool DoValidation();

  void OnFilePicked(wxFileDirPickerEvent &evt);

  wxString m_filepath;
  StringX m_combination;

private:
#ifndef NO_YUBI
  void OnYubibtnClick(wxCommandEvent& event);
  void OnPollingTimer(wxTimerEvent& event);
#endif

  wxFilePickerCtrl* m_filepicker;
  SafeCombinationCtrl* m_sc;
  bool m_bAutoValidate;
  PWScore* m_core;
  int m_confirmationButtonId;
  StringX m_yubiCombination; // needed to adjust TransferDataFromWindow()
};

#endif // _DBSELECTIONPANEL_H_
