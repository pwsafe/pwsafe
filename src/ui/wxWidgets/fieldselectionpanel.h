/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/** \file fieldselectionpanel.h
* 
*/

#ifndef __FIELDSELECTIONPANEL__
#define __FIELDSELECTIONPANEL__

#include <wx/panel.h>

#include "../../core/ItemData.h"
#include <set>

typedef std::set<CItemData::FieldType> FieldSet;

/*
 * This class implements a re-usable panel which lets user select
 * multiple item fields (CItemData::FieldType). Its meant to be
 * embedded in other dialogs and should be used with the custom
 * validator defined below the class
 */
 
class FieldSelectionPanel : public wxPanel {

  DECLARE_EVENT_TABLE()

  //returns wxNOT_FOUND if the field is not found in the listbox
  int FindField(CItemData::FieldType ft, wxListBox* lb) const;
  void MoveItem(int index, wxListBox* from, wxListBox* to);
  bool ItemIsMandatory(size_t index) const;

public:
  FieldSelectionPanel(wxWindow* parent);
  ~FieldSelectionPanel();

  void AddField(CItemData::FieldType ft, bool selected, bool mandatory);
  size_t GetNumSelectedFields() const;
  CItemData::FieldType GetSelectedFieldAt(size_t index) const;

  void OnSelectSome( wxCommandEvent& evt );
  void OnSelectAll( wxCommandEvent& evt );
  void OnRemoveSome( wxCommandEvent& evt );
  void OnRemoveAll( wxCommandEvent& evt );

private:
  wxListBox *m_lbSelected, *m_lbAvailable;
};

/*
 * This class should be used to transfer data to/from
 * the FieldSelectionPanel class declared above
 */
class FieldSelectionPanelValidator: public wxValidator
{
  //private: used internally by Clone
  FieldSelectionPanelValidator(const FieldSet& available,
                               const FieldSet& mandatory,
                               FieldSet& userSelection,
                               const wxString& validationMessage,
                               const wxString& validationTitle);

public:
  //userSelection must include all fields that are automatically selected
  FieldSelectionPanelValidator(const CItemData::FieldType* available, size_t navail,
                               const CItemData::FieldType* mandatory, size_t nmandatory,
                               FieldSet& userSelection,
                               const wxString& validationMessage,
                               const wxString& validationTitle);

  virtual wxObject* Clone() const {
    return new FieldSelectionPanelValidator(m_availableFields,
                                            m_mandatoryFields,
                                            m_userSelectedFields,
                                            m_validationMessage,
                                            m_validationTitle);
  }

  virtual bool TransferFromWindow();
  virtual bool TransferToWindow();
  virtual bool Validate (wxWindow* parent);

private:
  //For internal use
  FieldSet m_availableFields, m_mandatoryFields;

  //This is where the user's selections are transferred to/from
  FieldSet& m_userSelectedFields;

  //used in msgbox during validation
  wxString m_validationMessage, m_validationTitle;

  void SanitizeSelections();

  //we have reference members.  Don't let the class get copied inadvertently
  DECLARE_NO_COPY_CLASS(FieldSelectionPanelValidator)
};

#endif // __FIELDSELECTIONPANEL__
