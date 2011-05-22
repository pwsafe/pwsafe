/*
 * Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/** \file
* 
*/

#ifndef _COMPAREDLG_H_
#define _COMPAREDLG_H_

#include <wx/dialog.h>
#include <wx/collpane.h>

class PWScore;
struct SelectionCriteria;

class CompareDlg: public wxDialog
{
  void CreateControls();
  wxCollapsiblePane* CreateDBSelectionPanel(wxSizer* sizer);
  void OnPaneCollapse(wxCollapsiblePaneEvent& evt);
  void OnTopPaneCollapse(wxCollapsiblePaneEvent& evt);
  void OnReLayout(wxCommandEvent& evt);
  void OnCompare(wxCommandEvent& );
  void DoCompare();

public:
  CompareDlg(wxWindow* parent, PWScore* core);
  ~CompareDlg();

private:
  PWScore* m_currentCore;
  SelectionCriteria* m_selCriteria;

  //DECLARE_CLASS( MergeDlg )
  DECLARE_EVENT_TABLE()
};

#endif

