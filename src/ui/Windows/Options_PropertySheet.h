/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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

#include "ThisMfcApp.h"

#include "core/coredefs.h"

class COptions_PropertySheet : public CPWPropertySheet
{
public:
  COptions_PropertySheet(UINT nID, CWnd *pParent, const bool bLongPPs);
  ~COptions_PropertySheet();

  DECLARE_DYNAMIC(COptions_PropertySheet)

  MapMenuShortcuts GetMaps() const {return m_pp_shortcuts->GetMaps();}
  int32 GetHotKeyValue() const {return m_OPTMD.AppHotKeyValue;}
  int GetDCA() const {return m_OPTMD.DoubleClickAction;}
  int GetMaxMRUItems() const {return m_OPTMD.MaxMRUItems;}
  int GetMaxREItems() const {return m_OPTMD.MaxREItems;}
  int GetPWHAction() const {return m_OPTMD.PWHAction;}
  uint32 GetHashIters() const {return m_OPTMD.HashIters;}
  int GetPWHistoryMax() const {return m_OPTMD.PWHistoryNumDefault;}
  bool GetHotKeyState() const {return m_OPTMD.AppHotKeyEnabled == TRUE;}
  bool GetEnableGrid() const {return m_OPTMD.EnableGrid == TRUE;}
  bool GetNotesAsTips() const {return m_OPTMD.ShowNotesAsTipsInViews == TRUE;}
  bool RefreshViews() {return m_bRefreshViews;}
  bool SaveGroupDisplayState() const {return m_bSaveGroupDisplayState;}
  bool UpdateShortcuts() const {return m_bUpdateShortcuts;}
  bool CheckExpired() const {return m_bCheckExpired;}
  bool UserDisplayChanged() const {return m_save_bShowUsernameInTree != 
                                          m_OPTMD.ShowUsernameInTree;}
  bool PswdDisplayChanged() const {return m_save_bShowPasswordInTree != 
                                        m_OPTMD.ShowPasswordInTree;}
  bool ShowUsernameInTree() const {return m_OPTMD.ShowUsernameInTree == TRUE;}
  bool HighlightChanges() const {return m_OPTMD.HighlightChanges == TRUE;}
  bool SaveImmediately() const { return m_OPTMD.SaveImmediately == TRUE; }
  bool LockOnWindowLock() const {return m_OPTMD.LockOnWindowLock == TRUE;}
  bool LockOnWindowLockChanged() const {return m_OPTMD.LockOnWindowLock !=
                                               m_save_bLockOnWindowLock;}
  bool StartupShortcut() const {return m_OPTMD.Startup == TRUE;}
  bool StartupShortcutChanged() const {return m_OPTMD.Startup !=
                                              m_bStartupShortcutExists;}
  bool EnableTransparencyChanged() const {return m_save_bEnableTransparency !=
                                          m_OPTMD.EnableTransparency;}
  bool WindowTransparencyChanged() const {return m_save_iPercentTransparency !=
                                          m_OPTMD.PercentTransparency;}
  bool UpdateTransparency(const int value)
  { return app.GetMainDlg()->SetLayered((CWnd *)this, value); }

protected:
  virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  st_Opt_master_data m_OPTMD;

private:
  void SetupInitialValues();
  void UpdateCopyPreferences();

  CString m_save_bSymbols;
  int m_save_iPreExpiryWarnDays, m_save_iUseOwnSymbols, m_save_DisplayPreference;
  int m_save_iPercentTransparency;
  bool m_bIsModified, m_bChanged;
  bool m_bRefreshViews, m_bSaveGroupDisplayState, m_bUpdateShortcuts, m_bCheckExpired;
  BOOL m_save_bSaveImmediately, m_save_bHighlightChanges, m_save_bPreExpiryWarn;
  BOOL m_save_bShowUsernameInTree, m_save_bShowPasswordInTree, m_save_bExplorerTypeTree;
  BOOL m_save_bLockOnWindowLock, m_bStartupShortcutExists;
  BOOL m_save_bEnableTransparency;

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
