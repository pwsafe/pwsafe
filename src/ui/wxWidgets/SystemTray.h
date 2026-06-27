/*
 * Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
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

class SystemTray : public wxTaskBarIcon
{
  public:
    enum class TrayStatus { CLOSED, UNLOCKED, LOCKED };

    SystemTray(PasswordSafeFrame* frame);

    void SetTrayStatus(TrayStatus status);
    TrayStatus GetTrayStatus() const { return m_status; }
    void ShowIcon(void) { SetTrayStatus(m_status); }
    bool IsLocked(void) const { return m_status == TrayStatus::LOCKED; }
    
    /// event handler for a wxEVT_TASKBAR_LEFT_DCLICK
    void OnTaskBarLeftDoubleClick( wxTaskBarIconEvent& evt );

    /// event handler for all menu item events sent from system tray menu
    void OnSysTrayMenuItem( wxCommandEvent& evt );

    using wxTaskBarIcon::RemoveIcon;

  protected:
    //overridden from wxTaskBarIcon, called by framework on r-click
    virtual wxMenu* CreatePopupMenu();
#if wxUSE_APPINDICATOR
    //overridden from wxTaskBarIcon: supplies the persistent menu the
    //AppIndicator/SNI backend keeps attached to the indicator; unlike
    //CreatePopupMenu() this must not pop up a menu as a side effect,
    //since it's invoked on every SetTrayStatus() call, not just clicks
    virtual wxMenu* GetPopupMenu();
#endif
    void ProcessSysTrayMenuItem(int itemId);

  private:
    wxMenu* BuildMenu();
    void PopulateMenu(wxMenu* menu);
    wxMenu* GetRecentHistory();
    wxMenu* SetupRecentEntryMenu(const CItemData* pci, size_t idx);
    void ShowSetDatabaseIdDialog();

    bool m_TrayIconWithOverlay;
    int m_DatabaseID;
    wxColor m_LockedDatabaseIdColor, m_UnlockedDatabaseIdColor;
    wxIcon m_IconClosed;
    wxIcon m_IconUnlocked, m_IconLocked;
    wxIcon m_IconUnlockedWithID, m_IconLockedWithID;
    PasswordSafeFrame* m_frame;
    TrayStatus m_status;
#if wxUSE_APPINDICATOR
    // Kept alive for wx's AppIndicator backend (GetPopupMenu()'s contract:
    // the returned menu is never destroyed by wx). Deliberately not deleted
    // in a destructor: wxTaskBarIcon's own dtor drops the AppIndicator's
    // GtkMenu ref first, and this instance must outlive that; freed by the
    // OS at process exit instead.
    wxMenu* m_sniMenu = nullptr;
#endif

    DECLARE_EVENT_TABLE()
};

#endif /* _SYSTEMTRAY_H_ */
