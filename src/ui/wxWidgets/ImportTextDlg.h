/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ImportTextDlg.h
* 
*/

#ifndef _IMPORTTEXTDLG_H_
#define _IMPORTTEXTDLG_H_

#include <wx/dialog.h> // Base class: wxDialog
#include <wx/grid.h>
#include <wx/collpane.h>
#include "../../os/typedefs.h"

class wxBoxSizer;
class wxSizerFlags;
class wxRadioButton;

//Usage: Instantiate this class and if ShowModal() returns Ok, check
//the member variables which are automatically set by validators
class ImportTextDlg : public wxDialog
{
  DECLARE_CLASS( ImportTextDlg )
  DECLARE_EVENT_TABLE()

public:
  static ImportTextDlg* Create(wxWindow *parent, const wxString& filename = wxEmptyString);
  virtual ~ImportTextDlg() = default;
protected:
  ImportTextDlg(wxWindow *parent, const wxString& filename);
  void CreateControls();
public:
  wxString filepath;
  
  bool delimiterComma;
  bool delimiterSpace;
  bool delimiterTab;
  bool delimiterSemicolon;
  bool delimiterOther;
  wxString strDelimiterOther;
  wxString strDelimiterLine;
  
  bool importUnderGroup;
  wxString groupName;
  
  bool importPasswordsOnly;
  
  TCHAR FieldSeparator() const;
private:
  wxCollapsiblePane* CreateParsingOptionsPane(wxBoxSizer* dlgSizer);
  wxCollapsiblePane* CreateImportOptionsPane(wxBoxSizer* dlgSizer);
  wxBoxSizer* CreateVerticalButtonSizer(long flags);

  //convenience functions
  wxCheckBox* CheckBox(wxWindow* parent, const wxString& label, bool* validatorTarget);
  wxTextCtrl* TextCtrl(wxWindow* parent, wxString* validatorTarget);
  wxRadioButton* RadioButton(wxWindow* parent, const wxString& label, bool* validatorTarget, int flags = 0);
};

#endif // __IMPORTTEXTDLG_H__
