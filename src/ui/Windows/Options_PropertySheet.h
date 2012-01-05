/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "PWPropertySheet.h"
#include "Options_PropertyPage.h"
#include "OptionsBackup.h"
#include "OptionsDisplay.h"
#include "OptionsMisc.h"
#include "OptionsPasswordHistory.h"
#include "OptionsSecurity.h"
#include "OptionsShortcuts.h"
#include "OptionsSystem.h"

#include "core/coredefs.h"

class DboxMain;
class PWScore;

class COptions_PropertySheet : public CPWPropertySheet
{
public:
  COptions_PropertySheet(UINT nID, CWnd *pDbx, const bool bLongPPs);
  ~COptions_PropertySheet();

  virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
  BOOL PreTranslateMessage(MSG* pMsg);

  DECLARE_DYNAMIC(COptions_PropertySheet)

  const MapMenuShortcuts GetMaps() {return m_pp_shortcuts->GetMaps();}
  const DWORD GetHotKeyValue() {return m_OPTMD.Hotkey_Value;}
  const int GetDCA() {return m_OPTMD.DoubleClickAction;}
  const int GetMaxMRUItems() {return m_OPTMD.MaxMRUItems;}
  const int GetMaxREItems() {return m_OPTMD.MaxREItems;}
  const int GetTrayIconColour() {return m_OPTMD.TrayIconColour;}
  const int GetPWHAction() {return m_OPTMD.PWHAction;}
  const int GetPWHistoryMax() {return m_OPTMD.PWHistoryNumDefault;}
  const bool GetHotKeyState() {return m_OPTMD.Hotkey_Enabled == TRUE;}
  const bool GetEnableGrid() {return m_OPTMD.EnableGrid == TRUE;}
  const bool GetNotesAsTips() { return m_OPTMD.ShowNotesAsTipsInViews == TRUE;}
  const bool RefreshViews() {return m_bRefreshViews;}
  const bool SaveGroupDisplayState() {return m_bSaveGroupDisplayState;}
  const bool UpdateShortcuts() {return m_bUpdateShortcuts;}
  const bool CheckExpired() {return m_bCheckExpired;}
  const bool UserDisplayChanged() {return m_save_bShowUsernameInTree != 
                                          m_OPTMD.ShowUsernameInTree;}
  const bool PswdDisplayChanged() {return m_save_bShowPasswordInTree != 
                                        m_OPTMD.ShowPasswordInTree;}
  const bool ShowUsernameInTree() {return m_OPTMD.ShowUsernameInTree == TRUE;}
  const bool HighlightChanges() {return m_OPTMD.HighlightChanges == TRUE;}
  const bool LockOnWindowLock() {return m_OPTMD.LockOnWindowLock == TRUE;}
  const bool LockOnWindowLockChanged() {return m_OPTMD.LockOnWindowLock !=
                                               m_save_bLockOnWindowLock;}
  const bool StartupShortcut() {return m_OPTMD.Startup == TRUE;}
  const bool StartupShortcutChanged() {return m_OPTMD.Startup !=
                                              m_bStartupShortcutExists;}
  
protected:
  st_Opt_master_data m_OPTMD;

private:
  void SetupInitialValues();
  void UpdateCopyPreferences();

  CString m_save_bSymbols;
  int m_save_iPreExpiryWarnDays, m_save_iUseOwnSymbols;
  bool m_bIsModified, m_bChanged;
  bool m_bRefreshViews, m_bSaveGroupDisplayState, m_bUpdateShortcuts, m_bCheckExpired;
  BOOL m_save_bHighlightChanges, m_save_bPreExpiryWarn;
  BOOL m_save_bShowUsernameInTree, m_save_bShowPasswordInTree, m_save_bExplorerTypeTree;
  BOOL m_save_bLockOnWindowLock, m_bStartupShortcutExists;

  COptionsBackup          *m_pp_backup;
  COptionsDisplay         *m_pp_display;
  COptionsMisc            *m_pp_misc;
  COptionsPasswordHistory *m_pp_passwordhistory;
  COptionsSecurity        *m_pp_security;
  COptionsShortcuts       *m_pp_shortcuts;
  COptionsSystem          *m_pp_system;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
