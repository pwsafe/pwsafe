/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef __IMPORTXMLDLG_H__
#define __IMPORTXMLDLG_H__

#include <wx/dialog.h> // Base class: wxDialog

class wxCheckBox;
class wxTextCtrl;

class CImportXMLDlg : public wxDialog {

  DECLARE_CLASS( CImportXMLDlg )
  
public:
  CImportXMLDlg(wxWindow* parent);

public:
  bool importUnderGroup;
  wxString groupName;
  
  bool importPasswordsOnly;
  
  wxString filepath;
  
private:
  wxCheckBox* CheckBox(const wxString& label, bool* validatorTarget);
  wxTextCtrl* TextCtrl(wxString* validatorTarget);

};

#endif // __IMPORTXMLDLG_H__
