#ifndef __DBSELECTIONPANEL_H__
#define __DBSELECTIONPANEL_H__

#include <wx/panel.h>

class CSafeCombinationCtrl;

/*
 * This is a re-usable class for having the user select a db and
 * enter its combination.  The file picker ctrl is shown in the top
 * row and a masked textctrl + virtual keyboard button for entering
 * the combination in the second row.  It is meant to be used like
 * a child control by embedding in a wxSizer  See MergeDlg.cpp
 * and PwsSync.cpp for its usage
 * 
 * filePrompt - the static text displayed just above the file picker ctrl
 * 
 * filePickerCtrlTitle - the window title of the file picker dialog
 * 
 * rowsep - the multiplying factor for the separation between the first and second
 * rows.  A small dialog might pass a value of 2, while a wizard page might pass 5
 */
class DbSelectionPanel : public wxPanel {

  CSafeCombinationCtrl* m_sc;

public:
  DbSelectionPanel(wxWindow* parent, const wxString& filePrompt,
                    const wxString& filePickerCtrlTitle, unsigned rowsep); 
  ~DbSelectionPanel();

  //Set the keyboard focus on combination entry box and select-all
  void SelectCombinationText();

  wxString m_filepath;
  wxString m_combination;
};

#endif // __DBSELECTIONPANEL_H__
