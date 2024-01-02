/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ExportTextWarningDlg.h
* 
*/

#ifndef _EXPORTTEXTWARNINGDLG_H_
#define _EXPORTTEXTWARNINGDLG_H_

#include <wx/dialog.h> // Base class: wxDialog

#include "AdvancedSelectionDlg.h"
#include "SafeCombinationCtrl.h"
#ifndef NO_YUBI
#include "YubiMixin.h"
#endif

struct SelectionCriteria;

#ifndef NO_YUBI
class ExportTextWarningDlgBase : public wxDialog, private YubiMixin
#else
class ExportTextWarningDlgBase : public wxDialog
#endif
{

  DECLARE_CLASS( ExportTextWarningDlgBase )
  DECLARE_EVENT_TABLE()
public:
  SelectionCriteria* selCriteria;
  StringX           passKey;
  wxString          delimiter;
protected:
  explicit ExportTextWarningDlgBase(wxWindow *parent);
  ~ExportTextWarningDlgBase();

  void OnAdvancedSelection( wxCommandEvent& evt );

  virtual void DoAdvancedSelection() = 0;
private:
#ifndef NO_YUBI
  void OnYubibtnClick( wxCommandEvent& event );
  void OnPollingTimer(wxTimerEvent& timerEvent);
#endif
  const wxString defDelim;
  SafeCombinationCtrl* m_combinationEntry;
};

template <class DlgType>
class ExportTextWarningDlg : public ExportTextWarningDlgBase
{
public:
  static ExportTextWarningDlg* Create(wxWindow *parent) {
    return new ExportTextWarningDlg(parent);
  }
protected:
  explicit ExportTextWarningDlg(wxWindow *parent) : ExportTextWarningDlgBase(parent)
  {
    SetTitle(DlgType::GetTitle());
  }

  virtual void DoAdvancedSelection() {
    ShowModalAndGetResult<AdvancedSelectionDlg<DlgType>>(this, selCriteria);
  }
};

#endif // _EXPORTTEXTWARNINGDLG_H_
