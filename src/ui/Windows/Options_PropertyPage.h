/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "PWPropertyPage.h"
#include "SecString.h"
#include "ControlExtns.h"

class COptions_PropertySheet;
class PWScore;

// Database option text colour (COLORREF) - equivalent to RGB(0, 0, 128)
#define CR_DATABASE_OPTIONS 0x800000

struct st_Opt_master_data {
  bool bLongPPs;   // Long or Wide PropertyPages

  CSecString CurrentFile;
  CSecString UserBackupPrefix;
  CSecString UserBackupOtherLocation;
  BOOL SaveImmediately;
  BOOL BackupBeforeSave;
  BOOL BackupPrefix;
  int BackupLocation;
  int BackupSuffix;
  int MaxNumIncBackups;
  // Preferences min/max values
  short prefminBackupIncrement;
  short prefmaxBackupIncrement;

  // Display Data
  BOOL AlwaysOnTop;
  BOOL ShowPasswordInEdit;
  BOOL ShowUsernameInTree;
  BOOL ShowPasswordInTree;
  BOOL ShowNotesAsTipsInViews;
  BOOL ExplorerTypeTree;
  BOOL EnableGrid;
  BOOL NotesShowInEdit;
  BOOL WordWrapNotes;
  BOOL PreExpiryWarn;
  BOOL HighlightChanges;
  BOOL EnableTransparency;
  int PreExpiryWarnDays;
  int TreeDisplayStatusAtOpen;
  int PercentTransparency;
  // Preferences min/max values
  short prefminExpiryDays;
  short prefmaxExpiryDays;
  short prefminPercentTransparency;
  short prefmaxPercentTransparency;
  
  // Misc Data
  BOOL ConfirmDelete;
  BOOL MaintainDatetimeStamps;
  BOOL EscExits;
  int DoubleClickAction, ShiftDoubleClickAction;

  CSecString DefUsername;
  CSecString OtherBrowserLocation;
  CSecString OtherBrowserCmdLineParms;
  CSecString OtherEditorLocation;
  CSecString OtherEditorCmdLineParms;
  CSecString AutotypeText;
  unsigned AutotypeDelay;
  BOOL UseDefuser;
  BOOL QuerySetDef;
  BOOL MinAuto;
  // Preferences min/max values
  int prefminAutotypeDelay;
  int prefmaxAutotypeDelay;

  // Password History Data
  BOOL SavePWHistory;
  int PWHistoryNumDefault;
  int PWHAction;
  int PWHDefExpDays; // not strictly password history, but in the same tab.
  // Preferences min/max values
  short prefminPWHNumber;
  short prefmaxPWHNumber;

  // Security Data
  BOOL ClearClipboardOnMinimize;
  BOOL ClearClipboardOnExit;
  BOOL LockOnMinimize;
  BOOL ConfirmCopy;
  BOOL LockOnWindowLock;
  BOOL LockOnIdleTimeout;
  BOOL CopyPswdBrowseURL;
  int IdleTimeOut;
  uint32 HashIters;
  // Preferences min/max values
  short prefminIdleTimeout;
  short prefmaxIdleTimeout;

  // Shortcut Data
  int32 AppHotKeyValue;
  BOOL AppHotKeyEnabled;
  int ColWidth;
  int DefColWidth;

  // System Data
  BOOL UseSystemTray;
  BOOL HideSystemTray;
  BOOL MRUOnFileMenu;
  BOOL Startup;
  BOOL DefaultOpenRO;
  BOOL MultipleInstances;
  int MaxREItems;
  int MaxMRUItems;
  // Preferences min/max values
  short prefminREItems;
  short prefmaxREItems;
  short prefminMRU;
  short prefmaxMRU;
};

class COptions_PropertyPage : public CPWPropertyPage
{
public:
  COptions_PropertyPage(CWnd *pParent, UINT nID, st_Opt_master_data *pOPTMD);
  COptions_PropertyPage(CWnd *pParent, UINT nID, UINT nID_Short, st_Opt_master_data *pOPTMD);
  virtual ~COptions_PropertyPage() {}

  virtual BOOL OnQueryCancel();

  // Retrieve DoubleClickAction or ClearClipboardOnMimimize
  // or if Hot Key set
  // Make sure no overlap with 'PP_' enum in CPWPropertyPage
  enum {PPOPT_GET_DCA = 10, PPOPT_GET_CCOM, PPOPT_HOTKEY_SET};

  DECLARE_DYNAMIC(COptions_PropertyPage)

  // Backup Data
  inline CSecString &M_CurrentFile() {return m_OPTMD.CurrentFile;}
  inline CString &M_UserBackupPrefix() {return m_OPTMD.UserBackupPrefix;}
  inline CString &M_UserBackupOtherLocation() {return m_OPTMD.UserBackupOtherLocation;}
  inline BOOL &M_SaveImmediately() {return m_OPTMD.SaveImmediately;}
  inline BOOL &M_BackupBeforeSave() {return m_OPTMD.BackupBeforeSave;}
  inline BOOL &M_BackupPrefix() {return m_OPTMD.BackupPrefix;}
  inline int &M_BackupLocation() {return m_OPTMD.BackupLocation;}
  inline int &M_BackupSuffix() {return m_OPTMD.BackupSuffix;}
  inline int &M_MaxNumIncBackups() {return m_OPTMD.MaxNumIncBackups;}
  // Preferences min/max values
  inline short &M_prefminBackupIncrement() { return m_OPTMD.prefminBackupIncrement; }
  inline short &M_prefmaxBackupIncrement() { return m_OPTMD.prefmaxBackupIncrement; }

  // Display Data
  inline BOOL &M_AlwaysOnTop() {return m_OPTMD.AlwaysOnTop;}
  inline BOOL &M_ShowPasswordInEdit() {return m_OPTMD.ShowPasswordInEdit;}
  inline BOOL &M_ShowUsernameInTree() {return m_OPTMD.ShowUsernameInTree;}
  inline BOOL &M_ShowPasswordInTree() {return m_OPTMD.ShowPasswordInTree;}
  inline BOOL &M_ShowNotesAsTipsInViews() {return m_OPTMD.ShowNotesAsTipsInViews;}
  inline BOOL &M_ExplorerTypeTree() {return m_OPTMD.ExplorerTypeTree;}
  inline BOOL &M_EnableGrid() {return m_OPTMD.EnableGrid;}
  inline BOOL &M_NotesShowInEdit() {return m_OPTMD.NotesShowInEdit;}
  inline BOOL &M_WordWrapNotes() {return m_OPTMD.WordWrapNotes;}
  inline BOOL &M_PreExpiryWarn() {return m_OPTMD.PreExpiryWarn;}
  inline BOOL &M_HighlightChanges() {return m_OPTMD.HighlightChanges;}
  inline BOOL &M_EnableTransparency() {return m_OPTMD.EnableTransparency;}
  inline int &M_PreExpiryWarnDays() {return m_OPTMD.PreExpiryWarnDays;}
  inline int &M_TreeDisplayStatusAtOpen() {return m_OPTMD.TreeDisplayStatusAtOpen;}
  inline int &M_PercentTransparency() { return m_OPTMD.PercentTransparency; }
  // Preferences min/max values
  inline short &M_prefminExpiryDays() { return m_OPTMD.prefminExpiryDays; }
  inline short &M_prefmaxExpiryDays() { return m_OPTMD.prefmaxExpiryDays; }
  
  // Misc Data
  inline BOOL &M_ConfirmDelete() {return m_OPTMD.ConfirmDelete;}
  inline BOOL &M_MaintainDatetimeStamps() {return m_OPTMD.MaintainDatetimeStamps;}
  inline BOOL &M_EscExits() {return m_OPTMD.EscExits;}
  inline int &M_DoubleClickAction() {return m_OPTMD.DoubleClickAction;}
  inline int &M_ShiftDoubleClickAction() {return m_OPTMD.ShiftDoubleClickAction;}
  inline short &M_prefminPercentTransparency() { return m_OPTMD.prefminPercentTransparency; }
  inline short &M_prefmaxPercentTransparency() { return m_OPTMD.prefmaxPercentTransparency; }

  inline CSecString &M_DefUsername() {return m_OPTMD.DefUsername;}
  inline CString &M_OtherBrowserLocation() {return m_OPTMD.OtherBrowserLocation;}
  inline CString &M_OtherBrowserCmdLineParms() {return m_OPTMD.OtherBrowserCmdLineParms;}
  inline CString &M_OtherEditorLocation() {return m_OPTMD.OtherEditorLocation;}
  inline CString &M_OtherEditorCmdLineParms() {return m_OPTMD.OtherEditorCmdLineParms;}
  inline CString &M_AutotypeText() {return m_OPTMD.AutotypeText;}
  inline unsigned &M_AutotypeDelay() {return m_OPTMD.AutotypeDelay;}
  inline BOOL &M_UseDefUsername() {return m_OPTMD.UseDefuser;}
  inline BOOL &M_QuerySetDefUsername() {return m_OPTMD.QuerySetDef;}
  inline BOOL &M_AutotypeMinimize() {return m_OPTMD.MinAuto;}
  // Preferences min/max values
  inline int &M_prefminAutotypeDelay() { return m_OPTMD.prefminAutotypeDelay; }
  inline int &M_prefmaxAutotypeDelay() { return m_OPTMD.prefmaxAutotypeDelay; }

  // Password History Data
  inline BOOL &M_SavePWHistory() {return m_OPTMD.SavePWHistory;}
  inline int &M_PWHistoryNumDefault() {return m_OPTMD.PWHistoryNumDefault;}
  inline int &M_PWHDefExpDays() {return m_OPTMD.PWHDefExpDays;}
  inline int &M_PWHAction() {return m_OPTMD.PWHAction;}
  // Preferences min/max values
  inline short &M_prefminPWHNumber() { return m_OPTMD.prefminPWHNumber; }
  inline short &M_prefmaxPWHNumber() { return m_OPTMD.prefmaxPWHNumber; }

  // Security Data
  inline BOOL &M_ClearClipboardOnMinimize() {return m_OPTMD.ClearClipboardOnMinimize;}
  inline BOOL &M_ClearClipboardOnExit() {return m_OPTMD.ClearClipboardOnExit;}
  inline BOOL &M_LockOnMinimize() {return m_OPTMD.LockOnMinimize;}
  inline BOOL &M_ConfirmCopy() {return m_OPTMD.ConfirmCopy;}
  inline BOOL &M_LockOnWindowLock() {return m_OPTMD.LockOnWindowLock;}
  inline BOOL &M_LockOnIdleTimeout() {return m_OPTMD.LockOnIdleTimeout;}
  inline BOOL &M_CopyPswdBrowseURL() {return m_OPTMD.CopyPswdBrowseURL;}
  inline int &M_IdleTimeOut() {return m_OPTMD.IdleTimeOut;}
  inline uint32 &M_HashIters() {return m_OPTMD.HashIters;}
  // Preferences min/max values
  inline short &M_prefminIdleTimeout() { return m_OPTMD.prefminIdleTimeout; }
  inline short &M_prefmaxIdleTimeout() { return m_OPTMD.prefmaxIdleTimeout; }

  // Shortcut Data
  inline int32 &M_AppHotKey_Value() {return m_OPTMD.AppHotKeyValue;}
  inline BOOL &M_AppHotKeyEnabled() {return m_OPTMD.AppHotKeyEnabled;}
  inline int &M_ColWidth() {return m_OPTMD.ColWidth;}
  inline int &M_DefColWidth() {return m_OPTMD.DefColWidth;}

  // System Data
  inline BOOL &M_UseSystemTray() {return m_OPTMD.UseSystemTray;}
  inline BOOL &M_HideSystemTray() {return m_OPTMD.HideSystemTray;}
  inline BOOL &M_MRUOnFileMenu() {return m_OPTMD.MRUOnFileMenu;}
  inline BOOL &M_Startup() {return m_OPTMD.Startup;}
  inline BOOL &M_DefaultOpenRO() {return m_OPTMD.DefaultOpenRO;}
  inline BOOL &M_MultipleInstances() {return m_OPTMD.MultipleInstances;}
  inline int &M_MaxREItems() {return m_OPTMD.MaxREItems;}
  inline int &M_MaxMRUItems() {return m_OPTMD.MaxMRUItems;}
  // Preferences min/max values
  inline short &M_prefminREItems() { return m_OPTMD.prefminREItems; }
  inline short &M_prefmaxREItems() { return m_OPTMD.prefmaxREItems; }
  inline short &M_prefminMRU() { return m_OPTMD.prefminMRU; }
  inline short &M_prefmaxMRU() { return m_OPTMD.prefmaxMRU; }

protected:
  COptions_PropertySheet *m_options_psh;
  st_Opt_master_data &m_OPTMD;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
