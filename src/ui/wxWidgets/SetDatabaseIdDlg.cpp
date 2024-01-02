/*
 * Initial version created as 'SetDatabaseIdDlg.cpp'
 * by rafaelx on 2023-02-21.
 *
 * Copyright (c) 2019-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "SetDatabaseIdDlg.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(SetDatabaseIdDlg)
#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxUtilities.h"
#include "graphics/locked_tray.xpm"
#include "graphics/unlocked_tray.xpm"

//(*IdInit(SetDatabaseIdDlg)
const wxWindowID SetDatabaseIdDlg::ID_SPINCTRL_DATABASEID = wxNewId();
const wxWindowID SetDatabaseIdDlg::ID_COLORPICKERCTRL_LOCKEDTEXTCOLOR = wxNewId();
const wxWindowID SetDatabaseIdDlg::ID_STATICBITMAP_LOCKEDICON = wxNewId();
const wxWindowID SetDatabaseIdDlg::ID_COLORPICKERCTRL_UNLOCKEDTEXTCOLOR = wxNewId();
const wxWindowID SetDatabaseIdDlg::ID_STATICBITMAP_UNLOCKEDICON = wxNewId();
//*)

BEGIN_EVENT_TABLE(SetDatabaseIdDlg,wxDialog)
    //(*EventTable(SetDatabaseIdDlg)
    EVT_BUTTON( wxID_OK, SetDatabaseIdDlg::OnOkClick )
    EVT_BUTTON( wxID_CANCEL, SetDatabaseIdDlg::OnCancelClick )
    EVT_CLOSE( SetDatabaseIdDlg::OnClose )
    //*)
END_EVENT_TABLE()

SetDatabaseIdDlg* SetDatabaseIdDlg::Create(wxWindow *parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
  return new SetDatabaseIdDlg(parent, id, caption, pos, size, style);
}

SetDatabaseIdDlg::SetDatabaseIdDlg(wxWindow *parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
: m_DatabaseID(0), m_LockedDatabaseIDColor(*wxYELLOW), m_UnlockedDatabaseIDColor(*wxYELLOW), m_LockedIcon(locked_tray_xpm), m_UnlockedIcon(unlocked_tray_xpm)
{
  wxASSERT(!parent || parent->IsTopLevel());

  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, id, caption, pos, size, style);

  CreateControls();
  if (GetSizer()) {
    GetSizer()->SetSizeHints(this);
  }
  Centre();
}

void SetDatabaseIdDlg::CreateControls()
{
  //(*Initialize(SetDatabaseIdDlg)
  wxFlexGridSizer* FlexGridSizer1;
  wxStaticText* StaticText1;
  wxStaticText* StaticText2;
  wxStaticText* StaticText3;
  wxStaticText* StaticText4;
  wxStdDialogButtonSizer* StdDialogButtonSizer1;

  BoxSizer1 = new wxBoxSizer(wxVERTICAL);
  StaticText1 = new wxStaticText(this, wxID_ANY, _("Enter a value between 0 and 99 to overlay on system tray icon.\nA value of zero disables the overlay."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  BoxSizer1->Add(StaticText1, 0, wxALL, 10);
  FlexGridSizer1 = new wxFlexGridSizer(3, 3, 0, 0);
  StaticText4 = new wxStaticText(this, wxID_ANY, _("Database Id:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_SpinCtrl_DatabaseId = new wxSpinCtrl(this, ID_SPINCTRL_DATABASEID, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 99, 0, _T("ID_SPINCTRL_DATABASEID"));
  FlexGridSizer1->Add(m_SpinCtrl_DatabaseId, 1, wxALL|wxEXPAND, 5);
  FlexGridSizer1->Add(68,24,1, wxALL|wxEXPAND, 5);
  StaticText2 = new wxStaticText(this, wxID_ANY, _("Text color in locked state:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_ColorPickerCtrl_LockedTextColor = new wxColourPickerCtrl(this, ID_COLORPICKERCTRL_LOCKEDTEXTCOLOR, wxColour(0,0,0), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_COLORPICKERCTRL_LOCKEDTEXTCOLOR"));
  
  FlexGridSizer1->Add(m_ColorPickerCtrl_LockedTextColor, 1, wxALL|wxEXPAND, 5);
  m_StaticBitmap_LockedIcon = new wxStaticBitmap(this, ID_STATICBITMAP_LOCKEDICON, wxBitmap(locked_tray_xpm), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICBITMAP_LOCKEDICON"));
  FlexGridSizer1->Add(m_StaticBitmap_LockedIcon, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
  StaticText3 = new wxStaticText(this, wxID_ANY, _("Text color in unlocked state:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
  FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  m_ColorPickerCtrl_UnlockedTextColor = new wxColourPickerCtrl(this, ID_COLORPICKERCTRL_UNLOCKEDTEXTCOLOR, wxColour(0,0,0), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_COLORPICKERCTRL_UNLOCKEDTEXTCOLOR"));
  
  FlexGridSizer1->Add(m_ColorPickerCtrl_UnlockedTextColor, 1, wxALL|wxEXPAND, 5);
  m_StaticBitmap_UnlockedIcon = new wxStaticBitmap(this, ID_STATICBITMAP_UNLOCKEDICON, wxBitmap(unlocked_tray_xpm), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICBITMAP_UNLOCKEDICON"));
  FlexGridSizer1->Add(m_StaticBitmap_UnlockedIcon, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
  BoxSizer1->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 5);
  StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
  StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
  StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
  StdDialogButtonSizer1->Realize();
  BoxSizer1->Add(StdDialogButtonSizer1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
  SetSizer(BoxSizer1);
  BoxSizer1->Fit(this);
  BoxSizer1->SetSizeHints(this);

  Connect(ID_SPINCTRL_DATABASEID,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&SetDatabaseIdDlg::OnDatabaseIdChange);
  Connect(ID_COLORPICKERCTRL_LOCKEDTEXTCOLOR,wxEVT_COMMAND_COLOURPICKER_CHANGED,(wxObjectEventFunction)&SetDatabaseIdDlg::OnLockedTextColorChanged);
  Connect(ID_COLORPICKERCTRL_UNLOCKEDTEXTCOLOR,wxEVT_COMMAND_COLOURPICKER_CHANGED,(wxObjectEventFunction)&SetDatabaseIdDlg::OnUnlockedTextColorChanged);
  //*)
}

void SetDatabaseIdDlg::SetDatabaseID(int id) {
  if (id < 0)
    m_DatabaseID = 0;
  else if (id > 99)
    m_DatabaseID = 99;
  else
    m_DatabaseID = id;

  m_SpinCtrl_DatabaseId->SetValue(m_DatabaseID);
}

void SetDatabaseIdDlg::SetLockedDatabaseIdColor(const wxColor& color) {
  m_LockedDatabaseIDColor = color;
  m_ColorPickerCtrl_LockedTextColor->SetColour(m_LockedDatabaseIDColor);
}

void SetDatabaseIdDlg::SetUnlockedDatabaseIdColor(const wxColor& color) {
  m_UnlockedDatabaseIDColor = color;
  m_ColorPickerCtrl_UnlockedTextColor->SetColour(m_UnlockedDatabaseIDColor);
}

void SetDatabaseIdDlg::UpdateSampleBitmaps()
{
  if ((m_DatabaseID > 0) && (m_DatabaseID < 100)) {
    auto iconOverlayText = wxString::Format(wxT("%i"), m_DatabaseID);
    auto lockedIconWithOverlay = CreateIconWithOverlay(m_LockedIcon, m_LockedDatabaseIDColor, iconOverlayText);
    auto unlockedIconWithOverlay = CreateIconWithOverlay(m_UnlockedIcon, m_UnlockedDatabaseIDColor, iconOverlayText);
    m_StaticBitmap_LockedIcon->SetIcon(lockedIconWithOverlay);
    m_StaticBitmap_UnlockedIcon->SetIcon(unlockedIconWithOverlay);
  }
  else {
    m_StaticBitmap_LockedIcon->SetIcon(locked_tray_xpm);
    m_StaticBitmap_UnlockedIcon->SetIcon(unlocked_tray_xpm);
  }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void SetDatabaseIdDlg::OnOkClick(wxCommandEvent& WXUNUSED(event))
{
  EndModal(wxID_OK);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void SetDatabaseIdDlg::OnCancelClick(wxCommandEvent& WXUNUSED(event))
{
  EndModal(wxID_CANCEL);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void SetDatabaseIdDlg::OnClose(wxCloseEvent& WXUNUSED(event))
{
  EndModal(wxID_CANCEL);
}

void SetDatabaseIdDlg::OnDatabaseIdChange(wxSpinEvent& event)
{
  m_DatabaseID = event.GetValue();
  UpdateSampleBitmaps();
}

void SetDatabaseIdDlg::OnLockedTextColorChanged(wxColourPickerEvent& event)
{
  m_LockedDatabaseIDColor = event.GetColour();
  UpdateSampleBitmaps();
}

void SetDatabaseIdDlg::OnUnlockedTextColorChanged(wxColourPickerEvent& event)
{
  m_UnlockedDatabaseIDColor = event.GetColour();
  UpdateSampleBitmaps();
}
