/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SystemTray.cpp
 *
 */
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "./passwordsafeframe.h"
#include "./SystemTray.h"
#include "../../core/PWSprefs.h"
#include "./wxutils.h"
#include "./SystemTrayMenuId.h"

#include <wx/menu.h>

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include "./graphics/tray.xpm"
#include "./graphics/locked_tray.xpm"
#include "./graphics/unlocked_tray.xpm"
#include "./graphics/about.xpm"
#include "./graphics/exit.xpm"
#include "./graphics/lock.xpm"
#include "./graphics/unlock.xpm"

#include "./graphics/toolbar/new/copypassword.xpm"
#include "./graphics/toolbar/new/copyuser.xpm"
#include "./graphics/toolbar/new/copynotes.xpm"
#include "./graphics/toolbar/new/clearclipboard.xpm"
#include "./graphics/toolbar/new/autotype.xpm"
#include "./graphics/toolbar/new/browseurl.xpm"
#include "./graphics/toolbar/new/browseurlplus.xpm"
#include "./graphics/toolbar/new/sendemail.xpm"
#include "./graphics/toolbar/new/delete.xpm"
#include "./graphics/toolbar/new/close.xpm"

// Classic icons currently not used
//#include "./graphics/toolbar/classic/copypassword.xpm"
//#include "./graphics/toolbar/classic/copyuser.xpm"
//#include "./graphics/toolbar/classic/copynotes.xpm"
//#include "./graphics/toolbar/classic/clearclipboard.xpm"
//#include "./graphics/toolbar/classic/autotype.xpm"
//#include "./graphics/toolbar/classic/browseurl.xpm"
//#include "./graphics/toolbar/classic/browseurlplus.xpm"
//#include "./graphics/toolbar/classic/sendemail.xpm"
//#include "./graphics/toolbar/classic/delete.xpm"
//#include "./graphics/toolbar/classic/close.xpm"

BEGIN_EVENT_TABLE( SystemTray, wxTaskBarIcon )
  EVT_MENU( ID_SYSTRAY_RESTORE, SystemTray::OnSysTrayMenuItem )
  EVT_MENU( ID_SYSTRAY_LOCK,    SystemTray::OnSysTrayMenuItem )
  EVT_MENU( ID_SYSTRAY_UNLOCK,  SystemTray::OnSysTrayMenuItem )
  EVT_MENU( wxID_EXIT,          SystemTray::OnSysTrayMenuItem )
  EVT_MENU( wxID_ICONIZE_FRAME, SystemTray::OnSysTrayMenuItem )
  EVT_MENU( ID_CLEARCLIPBOARD,  SystemTray::OnSysTrayMenuItem )
  EVT_MENU( wxID_ABOUT,         SystemTray::OnSysTrayMenuItem )
  EVT_MENU( wxID_CLOSE,         SystemTray::OnSysTrayMenuItem )
  EVT_MENU( ID_SYSTRAY_CLEAR_RUE,  SystemTray::OnSysTrayMenuItem )
  EVT_MENU_RANGE(MIN_RUE_COMMAND_ID, MAX_RUE_COMMAND_ID, SystemTray::OnSysTrayMenuItem )
  EVT_TASKBAR_LEFT_DCLICK( SystemTray::OnTaskBarLeftDoubleClick )
END_EVENT_TABLE()

SystemTray::SystemTray(PasswordSafeFrame* frame) : iconClosed(tray_xpm),
                                                   iconUnlocked(unlocked_tray_xpm),
                                                   iconLocked(locked_tray_xpm),
                                                   m_frame(frame),
                                                   m_status(TrayStatus::  CLOSED)
{
}

void SystemTray::SetTrayStatus(TrayStatus status)
{
  m_status = status;

  if (!IsTaskBarIconAvailable())
    return;

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::UseSystemTray)) {
     switch(status) {
       case TrayStatus::CLOSED:
         SetIcon(iconClosed, wxTheApp->GetAppName());
         break;

       case TrayStatus::UNLOCKED:
         SetIcon(iconUnlocked, m_frame->GetCurrentSafe());
         break;

       case TrayStatus::LOCKED:
         SetIcon(iconLocked, m_frame->GetCurrentSafe());
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
    case TrayStatus::UNLOCKED:
        menu->Append(ID_SYSTRAY_LOCK, _("&Lock Safe"))->SetBitmap(wxBitmap(lock_xpm));
      break;

    case TrayStatus::LOCKED:
        menu->Append(ID_SYSTRAY_UNLOCK, _("&Unlock Safe"))->SetBitmap(wxBitmap(unlock_xpm));
        break;

    case TrayStatus::CLOSED:
        menu->Append(wxID_NONE, _("No Safe Open"));
        break;

    default:
        break;

  }

  if (m_status != TrayStatus::CLOSED) {
    menu->AppendSeparator();
    menu->Append(wxID_CLOSE, _("&Close"))->SetBitmap(wxBitmap(close_xpm));
    menu->AppendSubMenu(GetRecentHistory(), _("&Recent Entries History"));
  }

  menu->AppendSeparator();
  menu->Append(wxID_ICONIZE_FRAME, _("&Minimize"));
  menu->Append(ID_SYSTRAY_RESTORE, _("&Restore"));
  menu->AppendSeparator();
  menu->Append(ID_CLEARCLIPBOARD,  _("&Clear Clipboard"))->SetBitmap(wxBitmap(clearclipboard_xpm));
  menu->Append(wxID_ABOUT,         _("&About Password Safe..."))->SetBitmap(wxBitmap(about_xpm));
  menu->AppendSeparator();
  menu->Append(wxID_EXIT, _("&Exit"))->SetBitmap(wxBitmap(exit_xpm));

  //let the user iconize even if its already iconized
  if (!m_frame->IsShown())
    menu->Enable(wxID_ICONIZE_FRAME, false);

  return menu;
}

wxMenu* SystemTray::GetRecentHistory()
{
  //Must be on the heap.  wxWidgets will delete it
  wxMenu* menu = new wxMenu;

  menu->Append(ID_SYSTRAY_CLEAR_RUE, _("&Clear Recent History"));
  menu->Append(ID_TRAYRECENT_ENTRY_HELP1, _("Note: Entry format is «Group» «Title» «Username»"));
  menu->Append(ID_TRAYRECENT_ENTRY_HELP2, _("Note: Empty fields are shown as « »"));
  menu->AppendSeparator();

  menu->Enable(ID_TRAYRECENT_ENTRY_HELP1, false);
  menu->Enable(ID_TRAYRECENT_ENTRY_HELP2, false);

  std::vector<RUEntryData> menulist;
  m_frame->GetAllMenuItemStrings(menulist);

  for (size_t idx = 0; idx < menulist.size(); ++idx) {
    if (menulist[idx].pci && !menulist[idx].string.empty()) {
      menu->AppendSubMenu(SetupRecentEntryMenu(menulist[idx].pci, idx), towxstring(menulist[idx].string));
    }
  }

  return menu;
}

wxMenu* SystemTray::SetupRecentEntryMenu(const CItemData* pci, size_t idx)
{
  wxASSERT(pci);

  wxMenu* menu = new wxMenu;

  menu->Append(MakeCommandId(RUE_COPYPASSWORD, idx), _("&Copy Password to clipboard"))->SetBitmap(wxBitmap(copypassword_xpm));

  if (!pci->IsUserEmpty())
    menu->Append(MakeCommandId(RUE_COPYUSERNAME, idx), _("Copy &Username to clipboard"))->SetBitmap(wxBitmap(copyuser_xpm));

  if (!pci->IsNotesEmpty())
    menu->Append(MakeCommandId(RUE_COPYNOTES, idx), _("Copy &Notes to clipboard"))->SetBitmap(wxBitmap(copynotes_xpm));

  menu->Append(MakeCommandId(RUE_AUTOTYPE, idx), _("Perform Auto&Type"))->SetBitmap(wxBitmap(autotype_xpm));

  if (!pci->IsURLEmpty() && !pci->IsURLEmail())
    menu->Append(MakeCommandId(RUE_COPYURL, idx), _("Copy URL to clipboard"));

  if (!pci->IsEmailEmpty() || (!pci->IsURLEmpty() && pci->IsURLEmail()))
    menu->Append(MakeCommandId(RUE_COPYEMAIL, idx), _("Copy email to clipboard"));

  if (!pci->IsURLEmpty() && !pci->IsURLEmail()) {
    menu->Append(MakeCommandId(RUE_BROWSE, idx), _("&Browse to URL"))->SetBitmap(wxBitmap(browseurl_xpm));
    menu->Append(MakeCommandId(RUE_BROWSEAUTOTYPE, idx), _("Browse to URL + &Autotype"))->SetBitmap(wxBitmap(browseurlplus_xpm));
  }

  if (!pci->IsEmailEmpty() || (!pci->IsURLEmpty() && pci->IsURLEmail()))
    menu->Append(MakeCommandId(RUE_SENDEMAIL, idx), _("&Send email"))->SetBitmap(wxBitmap(sendemail_xpm));

  if (!pci->IsRunCommandEmpty())
    menu->Append(MakeCommandId(RUE_RUNCOMMAND, idx), _("&Run Command"));

  menu->Append(MakeCommandId(RUE_DELETERUEENTRY, idx), _("&Delete from Recent Entry List"))->SetBitmap(wxBitmap(delete_xpm));

  return menu;
}

void SystemTray::OnSysTrayMenuItem(wxCommandEvent& evt)
{
  EventHandlerDisabler ehd(this);

  const int id = evt.GetId();
  if (IsRUECommand(id)) {
    RUEOperation opn = GetRUEOperation(id);
    if (opn == RUE_DELETERUEENTRY) {
      m_frame->DeleteRUEntry(GetRUEIndex(id));
    }
    else {
      wxCommandEvent cmd(evt.GetEventType(), GetFrameCommandId(opn));
      cmd.SetExtraLong(id);
#if wxCHECK_VERSION(2,9,0)
      m_frame->GetEventHandler()->ProcessEvent(cmd);
#else
      m_frame->ProcessEvent(cmd);
#endif
    }
  }
  else {
    switch(id) {

      case ID_SYSTRAY_RESTORE:
        m_frame->UnlockSafe(true, false); // true => restore UI
        break;

      case ID_SYSTRAY_LOCK:
        m_frame->HideUI(true);
        break;

      case ID_SYSTRAY_UNLOCK:
        m_frame->UnlockSafe(false, false); // false => don't restore UI
        break;

      case ID_SYSTRAY_CLEAR_RUE:
        m_frame->ClearRUEList();
        break;

      case wxID_EXIT:
      case ID_CLEARCLIPBOARD:
      case wxID_ABOUT:
      case wxID_CLOSE:
        m_frame->GetEventHandler()->ProcessEvent(evt);
        break;

      case wxID_ICONIZE_FRAME:
        m_frame->Iconize();
        break;

      default:
        break;
    }
  }
}

void SystemTray::OnTaskBarLeftDoubleClick(wxTaskBarIconEvent& /*evt*/)
{
  EventHandlerDisabler ehd(this);
  m_frame->UnlockSafe(true, false); //true => restore UI
}
