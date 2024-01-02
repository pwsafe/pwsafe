/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ImportXmlDlg.h
* 
*/

#ifndef _IMPORTXMLDLG_H_
#define _IMPORTXMLDLG_H_

#include <wx/dialog.h> // Base class: wxDialog

class wxCheckBox;
class wxTextCtrl;

class ImportXmlDlg : public wxDialog
{
  DECLARE_CLASS( ImportXmlDlg )
  
protected:
  ImportXmlDlg(wxWindow *parent, const wxString& filename);

public:
  static ImportXmlDlg* Create(wxWindow *parent, const wxString& filename);

  bool importUnderGroup;
  wxString groupName;
  
  bool importPasswordsOnly;
  
  wxString filepath;
  
private:
  wxCheckBox* CheckBox(const wxString& label, bool* validatorTarget);
  wxTextCtrl* TextCtrl(wxString* validatorTarget);

};

#endif // _IMPORTXMLDLG_H_
