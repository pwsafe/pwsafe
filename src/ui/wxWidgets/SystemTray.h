/*
 * Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SystemTray.h
 * 
 */

#ifndef _SYSTEMTRAY_H_
#define _SYSTEMTRAY_H_

#include <wx/taskbar.h>
#include <wx/icon.h>

class PasswordSafeFrame;

class SystemTray : protected wxTaskBarIcon
{
  public:
    typedef enum { TRAY_CLOSED, TRAY_UNLOCKED, TRAY_LOCKED } TrayStatus;

    SystemTray(PasswordSafeFrame* frame);

    void SetTrayStatus(TrayStatus st);
    TrayStatus GetTrayStatus() const {return m_status;}
    void ShowIcon(void) { SetTrayStatus(m_status); }
    bool IsLocked(void) const { return m_status == TRAY_LOCKED; }
    
    /// event handler for a wxEVT_TASKBAR_LEFT_DCLICK
    void OnTaskBarLeftDoubleClick( wxTaskBarIconEvent& evt );

    /// event handler for all menu item events sent from system tray menu
    void OnSysTrayMenuItem( wxCommandEvent& evt );

    using wxTaskBarIcon::RemoveIcon;

  protected:
    //overridden from wxTaskBarIcon, called by framework on r-click
    virtual wxMenu* CreatePopupMenu();

  private:
    wxMenu* GetRecentHistory();
    wxMenu* SetupRecentEntryMenu(const CItemData* pci, size_t idx);

	wxIcon iconClosedBlack, iconClosedBlue, iconClosedYellow, iconClosedWhite;
    wxIcon iconUnlocked, iconLocked;
    PasswordSafeFrame* m_frame;
    TrayStatus m_status;

    DECLARE_EVENT_TABLE()
};

#endif /* _SYSTEMTRAY_H_ */

