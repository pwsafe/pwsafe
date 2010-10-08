/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "./PwsHelpProvider.h"
#include <wx/help.h>
#include <wx/fs_arc.h>

IMPLEMENT_CLASS( PWSHelpProvider, wxEvtHandler )

BEGIN_EVENT_TABLE( PWSHelpProvider, wxEvtHandler )
  EVT_BUTTON( wxID_HELP,  PWSHelpProvider::OnHelp )
END_EVENT_TABLE()


PWSHelpProvider::PWSHelpProvider() : m_controller(new wxHtmlHelpController)
{
  wxFileSystem::AddHandler(new wxArchiveFSHandler);
  if (!m_controller->Initialize(wxT("/home/sghosh/work/pwsafe/help/help.zip")))
    wxMessageBox(_("Could not initialize help subsystem.  Help would not be available"),
    _("Error initializing help"), wxOK | wxICON_ERROR);
}

PWSHelpProvider::~PWSHelpProvider()
{
  delete m_controller;
  m_controller = 0;
}

PWSHelpProvider& PWSHelpProvider::Instance()
{
  static PWSHelpProvider s_instance;
  return s_instance;
}

void PWSHelpProvider::OnHelp(wxCommandEvent& evt)
{
  wxWindow* win = wxDynamicCast(evt.GetEventObject(), wxWindow);
  if (win) {
    if (win->GetId() == wxID_HELP && !(win = win->GetParent()))
        return;
    HelpContextMap::iterator itr = m_map.find(win->GetId());
    if (itr != m_map.end())
      m_controller->DisplaySection(itr->second);
    else {
      win = win->GetParent();
    }
  }
}

void PWSHelpProvider::SetHelpContext(wxWindow* win, const wxString& helpSection)
{
  m_map[win->GetId()] = helpSection;
  win->PushEventHandler(this);
}
