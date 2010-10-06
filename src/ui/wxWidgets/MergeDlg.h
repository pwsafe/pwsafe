#ifndef __MERGEDLG_H__
#define __MERGEDLG_H__

#include <wx/dialog.h>

class PWScore;
struct SelectionCriteria;

class MergeDlg : public wxDialog {
  
  DECLARE_CLASS( MergeDlg )
  DECLARE_EVENT_TABLE()
  
public:
  MergeDlg(wxWindow* parent, PWScore* core);
  ~MergeDlg();

  void OnAdvancedSelection( wxCommandEvent& evt );
  void OnOk( wxCommandEvent& );

  wxString GetOtherSafePath() const { return m_filepath; }
  wxString GetOtherSafeCombination() const { return m_combination; }
  SelectionCriteria GetSelectionCriteria() const ;
  
private:
  wxString m_filepath;
  wxString m_combination;

  
  PWScore* m_core;
  SelectionCriteria* m_selection;
};

#endif // __MERGEDLG_H__
