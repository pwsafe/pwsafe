/*
 * Initial version created as 'SetDatabaseIdDlg.h'
 * by rafaelx on 2023-02-21.
 *
 * Copyright (c) 2019-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef SETDATABASEIDMAIN_H
#define SETDATABASEIDMAIN_H

//(*Headers(SetDatabaseIdDlg)
#include <wx/clrpicker.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
//*)

#define SYMBOL_SETDATABASEIDDLG_STYLE wxCAPTION|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_SETDATABASEIDDLG_TITLE _("Set Database ID")
#define SYMBOL_SETDATABASEIDDLG_IDNAME wxID_ANY
#define SYMBOL_SETDATABASEIDDLG_SIZE wxSize(-1, -1)
#define SYMBOL_SETDATABASEIDDLG_POSITION wxDefaultPosition

class SetDatabaseIdDlg: public wxDialog
{
public:
  /// Creation
  static SetDatabaseIdDlg* Create(wxWindow *parent, wxWindowID id = SYMBOL_SETDATABASEIDDLG_IDNAME, const wxString& caption = SYMBOL_SETDATABASEIDDLG_TITLE, const wxPoint& pos = SYMBOL_SETDATABASEIDDLG_POSITION, const wxSize& size = SYMBOL_SETDATABASEIDDLG_SIZE, long style = SYMBOL_SETDATABASEIDDLG_STYLE);

  /// Destructor
  ~SetDatabaseIdDlg() = default;

  /// Getters / Setters
  int GetDatabaseID() const { return m_DatabaseID; };
  wxColor GetLockedDatabaseIdColor() const { return m_LockedDatabaseIDColor; };
  wxColor GetUnlockedDatabaseIdColor() const { return m_UnlockedDatabaseIDColor; };

  void SetDatabaseID(int id);
  void SetLockedDatabaseIdColor(const wxColor& color);
  void SetUnlockedDatabaseIdColor(const wxColor& color);
  void UpdateSampleBitmaps();

protected:
  /// Constructors
  SetDatabaseIdDlg(wxWindow *parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style);

  /// Creates the controls and sizers
  void CreateControls();

private:
    
  //(*Handlers(SetDatabaseIdDlg)
  void OnOkClick(wxCommandEvent& event);
  void OnCancelClick(wxCommandEvent& event);
  void OnClose(wxCloseEvent& event);
  void OnDatabaseIdChange(wxSpinEvent& event);
  void OnLockedTextColorChanged(wxColourPickerEvent& event);
  void OnUnlockedTextColorChanged(wxColourPickerEvent& event);
  //*)

  //(*Identifiers(SetDatabaseIdDlg)
  static const wxWindowID ID_SPINCTRL_DATABASEID;
  static const wxWindowID ID_COLORPICKERCTRL_LOCKEDTEXTCOLOR;
  static const wxWindowID ID_STATICBITMAP_LOCKEDICON;
  static const wxWindowID ID_COLORPICKERCTRL_UNLOCKEDTEXTCOLOR;
  static const wxWindowID ID_STATICBITMAP_UNLOCKEDICON;
  //*)

  //(*Declarations(SetDatabaseIdDlg)
  wxBoxSizer* BoxSizer1;
  wxColourPickerCtrl* m_ColorPickerCtrl_LockedTextColor;
  wxColourPickerCtrl* m_ColorPickerCtrl_UnlockedTextColor;
  wxSpinCtrl* m_SpinCtrl_DatabaseId;
  wxStaticBitmap* m_StaticBitmap_LockedIcon;
  wxStaticBitmap* m_StaticBitmap_UnlockedIcon;
  //*)

  int m_DatabaseID;
  wxColor m_LockedDatabaseIDColor;
  wxColor m_UnlockedDatabaseIDColor;
  wxIcon m_LockedIcon;
  wxIcon m_UnlockedIcon;

  DECLARE_EVENT_TABLE()
};

#endif // SETDATABASEIDMAIN_H
