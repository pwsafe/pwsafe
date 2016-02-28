/*
 * Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef __EXPORTTEXTWARNINGDLG_H__
#define __EXPORTTEXTWARNINGDLG_H__

#include <wx/dialog.h> // Base class: wxDialog

#include "./AdvancedSelectionDlg.h"

struct SelectionCriteria;

class CExportTextWarningDlgBase : public wxDialog {

  DECLARE_CLASS( CExportTextWarningDlgBase )
  DECLARE_EVENT_TABLE()

public:
  CExportTextWarningDlgBase(wxWindow* parent);
  ~CExportTextWarningDlgBase();

  void OnAdvancedSelection( wxCommandEvent& evt );

  virtual void DoAdvancedSelection() = 0;

  SelectionCriteria* selCriteria;
  StringX           passKey;
  wxString          delimiter;
private:
  void OnYubibtnClick( wxCommandEvent& event );
  void OnPollingTimer(wxTimerEvent& timerEvent);

  const wxString defDelim;
  CSafeCombinationCtrl* m_combinationEntry;
  wxBitmapButton* m_YubiBtn;
  wxStaticText* m_yubiStatusCtrl;
  wxTimer* m_pollingTimer; // for Yubi, but can't go into mixin :-(
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
    dlg.ShowModal();
  }
};

#endif // __EXPORTTEXTWARNINGDLG_H__
