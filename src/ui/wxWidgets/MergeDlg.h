/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __MERGEDLG_H__
#define __MERGEDLG_H__

#include <wx/dialog.h>
#include "../../core/StringX.h"

class PWScore;
struct SelectionCriteria;
class DbSelectionPanel;

class MergeDlg : public wxDialog {
  
  DECLARE_CLASS( MergeDlg )
  DECLARE_EVENT_TABLE()
  
public:
  MergeDlg(wxWindow* parent, PWScore* core);
  ~MergeDlg();

  void OnAdvancedSelection( wxCommandEvent& evt );

  wxString GetOtherSafePath() const;
  StringX GetOtherSafeCombination() const;
  SelectionCriteria GetSelectionCriteria() const;
  
private:
  PWScore* m_core;
  SelectionCriteria* m_selection;
  DbSelectionPanel* m_dbPanel;
};

#endif // __MERGEDLG_H__
