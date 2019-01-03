/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef __DBSELECTIONPANEL_H__
#define __DBSELECTIONPANEL_H__

#include <wx/panel.h>
#include "../../core/StringX.h"

class wxFilePickerCtrl;
class CSafeCombinationCtrl;
class PWScore;
class wxFileDirPickerEvent;

/*
 * This is a re-usable class for having the user select a db and
 * enter its combination.  The file picker ctrl is shown in the top
 * row and a masked textctrl + virtual keyboard button for entering
 * the combination in the second row.  It is meant to be used like
 * a child control by embedding in a wxSizer  See MergeDlg.cpp
 * and PwsSync.cpp for its usage
 * 
 * filePrompt - the static text displayed just above the file picker ctrl
 * 
 * filePickerCtrlTitle - the window title of the file picker dialog
 * 
 * rowsep - the multiplying factor for the separation between the first and second
 * rows.  A small dialog might pass a value of 2, while a wizard page might pass 5
 */
class DbSelectionPanel : public wxPanel {

  wxFilePickerCtrl* m_filepicker;
  CSafeCombinationCtrl* m_sc;
  bool m_bAutoValidate;
  PWScore* m_core;
  
public:
  DbSelectionPanel(wxWindow* parent, const wxString& filePrompt,
                    const wxString& filePickerCtrlTitle, bool autoValidate,
                    PWScore* core, unsigned rowsep); 
  ~DbSelectionPanel();

  //Set the keyboard focus on combination entry box and select-all
  void SelectCombinationText();

  //wxWindow override to not validate in cases like going back in a wizard page
  virtual bool Validate() {
    return !m_bAutoValidate || DoValidation();
  }

  bool DoValidation();

  void OnFilePicked(wxFileDirPickerEvent &evt);

  wxString m_filepath;
  StringX m_combination;
};

#endif // __DBSELECTIONPANEL_H__
