#ifndef __EXPORTTEXTWARNINGDLG_H__
#define __EXPORTTEXTWARNINGDLG_H__

#include <wx/dialog.h> // Base class: wxDialog

#include "./AdvancedSelectionDlg.h"

class CExportTextWarningDlgBase : public wxDialog {

  DECLARE_CLASS( CExportTextWarningDlgBase )
  DECLARE_EVENT_TABLE()

public:
  CExportTextWarningDlgBase(wxWindow* parent);
  ~CExportTextWarningDlgBase() {}

  void OnAdvancedSelection( wxCommandEvent& evt );

  virtual void DoAdvancedSelection() = 0;

  SelectionCriteria selCriteria;
  StringX           passKey;
  wxString          delimiter;
};

template <class DlgType>
class CExportTextWarningDlg : public CExportTextWarningDlgBase
{
public:
  CExportTextWarningDlg(wxWindow* parent) : CExportTextWarningDlgBase(parent)
  {
    SetTitle(DlgType::GetTitle());
  }

  virtual void DoAdvancedSelection() {
    AdvancedSelectionDlg<DlgType> dlg(this, selCriteria);
    if (dlg.ShowModal() == wxID_OK)
      dlg.GetSelectionCriteria(selCriteria);
  }
};

#endif // __EXPORTTEXTWARNINGDLG_H__
