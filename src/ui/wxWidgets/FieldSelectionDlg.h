/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file FieldSelectionDlg.h
 * 
 */

/*
 * A dialog class to let the user select multiple CItemData::FieldType items
 */

#ifndef _FIELDSELECTIONDLG_H_
#define _FIELDSELECTIONDLG_H_

#include <wx/dialog.h>

#include "./FieldSelectionPanel.h"
#include "../../core/ItemData.h"

class FieldSelectionDlg : public wxDialog
{
  DECLARE_EVENT_TABLE()
public:
  static FieldSelectionDlg* Create(wxWindow *parent, const CItemData::FieldType* available, size_t navail,
                    const CItemData::FieldType* mandatory, size_t nmandatory,
                    FieldSet& userSelection,
                    const wxString& operation,  //something like "search", or "merge" or "compare"
                    // These would be constructed from 'operation' if not provided explicitly
                    const wxString& validationMessage = wxEmptyString, 
                    const wxString& validationTitle = wxEmptyString,
                    const wxString& title = wxEmptyString,
                    const wxString& staticText = wxEmptyString);

protected:
  FieldSelectionDlg(wxWindow *parent, const CItemData::FieldType* available, size_t navail,
                    const CItemData::FieldType* mandatory, size_t nmandatory,
                    FieldSet& userSelection,
                    const wxString& operation,  //something like "search", or "merge" or "compare"
                    // These would be constructed from 'operation' if not provided explicitly
                    const wxString& validationMessage, 
                    const wxString& validationTitle,
                    const wxString& title,
                    const wxString& staticText);
  
  void OnInitDialog(wxInitDialogEvent& evt);
  void OnRelayoutDlg(wxCommandEvent& evt);  
};

#endif // _FIELDSELECTIONDLG_H_
