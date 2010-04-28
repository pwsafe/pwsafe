/*
 * Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file version.cpp
 * 
 */

#include "./passwordsafeframe.h"
#include "./SystemTray.h"
#include "../../corelib/PWSprefs.h"

#include <wx/menu.h>

#include "../graphics/wxWidgets/tray.xpm"
#include "../graphics/wxWidgets/locked_tray.xpm"
#include "../graphics/wxWidgets/unlocked_tray.xpm"

BEGIN_EVENT_TABLE( SystemTray, wxTaskBarIcon )
  EVT_MENU( ID_SYSTRAY_RESTORE, SystemTray::OnSysTrayMenuItem )
  EVT_MENU( ID_SYSTRAY_LOCK,    SystemTray::OnSysTrayMenuItem )
  EVT_MENU( ID_SYSTRAY_UNLOCK,  SystemTray::OnSysTrayMenuItem )
  EVT_MENU( wxID_EXIT,          SystemTray::OnSysTrayMenuItem )
  EVT_MENU( wxID_ICONIZE_FRAME, SystemTray::OnSysTrayMenuItem )
  EVT_MENU( ID_CLEARCLIPBOARD,  SystemTray::OnSysTrayMenuItem )
  EVT_MENU( wxID_ABOUT,         SystemTray::OnSysTrayMenuItem )
  EVT_MENU( wxID_CLOSE,         SystemTray::OnSysTrayMenuItem )
  EVT_TASKBAR_LEFT_DCLICK( SystemTray::OnTaskBarLeftDoubleClick )
END_EVENT_TABLE()

SystemTray::SystemTray(PasswordSafeFrame* frame) : iconClosed(tray_xpm), 
                                                   iconUnlocked(unlocked_tray_xpm), 
                                                   iconLocked(locked_tray_xpm),
                                                   m_frame(frame),
                                                   m_status(TRAY_CLOSED)
{
}

void SystemTray::SetTrayStatus(TrayStatus status)
{
  m_status = status;
 
#if wxCHECK_VERSION(2,9,0)
  if (!wxTaskBarIcon::IsAvailable())
    return;
#endif

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::UseSystemTray)) {
     switch(status) {
       case TRAY_CLOSED:
         SetIcon(iconClosed);
         break;

       case TRAY_UNLOCKED:
         SetIcon(iconUnlocked);
         break;

       case TRAY_LOCKED:
         SetIcon(iconLocked);
         break;

       default:
         break;
     }
  }
}

//virtual 
wxMenu* SystemTray::CreatePopupMenu()
{
  wxMenu* menu = new wxMenu;

  switch (m_status) {
    case TRAY_UNLOCKED:
        menu->Append(ID_SYSTRAY_LOCK, wxT("&Lock Safe"));
      break;

    case TRAY_LOCKED:
        menu->Append(ID_SYSTRAY_UNLOCK, wxT("&Unlock Safe"));
        break;

    case TRAY_CLOSED:
        menu->Append(wxID_NONE, wxT("No Safe Open"));
        break;

    default:
        break;

  }

  if (m_status != TRAY_CLOSED) {
    menu->AppendSeparator();
    menu->Append(wxID_CLOSE, wxT("&Close"));
  }
  
  menu->AppendSeparator();
  menu->Append(wxID_ICONIZE_FRAME, wxT("&Minimize"));
  menu->Append(ID_SYSTRAY_RESTORE, wxT("&Restore"));
  menu->AppendSeparator();
  menu->Append(ID_CLEARCLIPBOARD,  wxT("&Clear Clipboard"));
  menu->Append(wxID_ABOUT,         wxT("&About Password Safe..."));
  menu->AppendSeparator();
  menu->Append(wxID_EXIT, wxT("&Exit"));
  
  //let the user iconize even if its already iconized
  if (!m_frame->IsShown())
    menu->Enable(wxID_ICONIZE_FRAME, false);

  return menu;
}

void SystemTray::OnSysTrayMenuItem(wxCommandEvent& evt)
{
  switch(evt.GetId()) {

    case ID_SYSTRAY_RESTORE:
      m_frame->UnlockSafe(true); // true => restore UI
      break;

    case ID_SYSTRAY_LOCK:
      m_frame->HideUI(true);
      break;

    case ID_SYSTRAY_UNLOCK:
      m_frame->UnlockSafe(false); // false => don't restore UI
      break;

    case wxID_EXIT:
    case wxID_ICONIZE_FRAME:
    case ID_CLEARCLIPBOARD:
    case wxID_ABOUT:
    case wxID_CLOSE:
      m_frame->ProcessEvent(evt);
      break;

    default:
      break;
  }
}

void SystemTray::OnTaskBarLeftDoubleClick(wxTaskBarIconEvent& /*evt*/)
{
  m_frame->UnlockSafe(true); //true => restore UI
}
