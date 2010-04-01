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

#include <wx/menu.h>

#include "../graphics/wxWidgets/tray.xpm"
#include "../graphics/wxWidgets/locked_tray.xpm"
#include "../graphics/wxWidgets/unlocked_tray.xpm"

BEGIN_EVENT_TABLE( SystemTray, wxTaskBarIcon )
  EVT_MENU( ID_SYSTRAY_RESTORE, SystemTray::OnSysTrayRestore )
END_EVENT_TABLE()

SystemTray::SystemTray(PasswordSafeFrame* frame) : iconClosed(tray_xpm), 
                                                   iconUnlocked(unlocked_tray_xpm), 
                                                   iconLocked(locked_tray_xpm),
                                                   m_frame(frame)
{
}

void SystemTray::SetTrayStatus(TrayStatus status)
{
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

//virtual 
wxMenu* SystemTray::CreatePopupMenu()
{
  wxMenu* menu = new wxMenu;
  menu->Append(ID_SYSTRAY_RESTORE, wxT("&Restore"));
  return menu;
}

void SystemTray::OnSysTrayRestore(wxCommandEvent& /*evt*/)
{
  m_frame->OnSysTrayRestore();
}

