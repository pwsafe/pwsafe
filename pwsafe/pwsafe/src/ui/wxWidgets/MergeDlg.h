#ifndef __MERGEDLG_H__
#define __MERGEDLG_H__

#include <wx/dialog.h>

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
  wxString GetOtherSafeCombination() const;
  SelectionCriteria GetSelectionCriteria() const;
  
private:
  PWScore* m_core;
  SelectionCriteria* m_selection;
  DbSelectionPanel* m_dbPanel;
};

#endif // __MERGEDLG_H__
