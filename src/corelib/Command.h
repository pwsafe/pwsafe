/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// Command.h
//-----------------------------------------------------------------------------

#ifndef __COMMAND_H
#define __COMMAND_H

class CReport;
class PWScore;

#include "ItemData.h"
#include "StringX.h"
#include "UUIDGen.h"
#include "GUICommandInterface.h"

#include "coredefs.h"

#include <map>
#include <vector>
#include <algorithm>

// Base Command class

class Command
{
public:
  Command(PWScore *pcore);
  virtual ~Command();
  virtual int Execute() = 0;
  virtual int Redo() = 0;
  virtual void Undo() = 0;

  void SetNoGUINotify() {m_bNotifyGUI = false;}
  void ResetSavedState(const bool bNewDBState) {m_bSaveDBChanged = bNewDBState;}

  enum ExecuteFn {
    WN_INVALID = -1,
    WN_ALL = 0,
    WN_EXECUTE = 1,
    WN_REDO = 2,
    WN_EXECUTE_REDO = 3,
    WN_UNDO = 4
  };

  enum GUI_Action {
    GUI_UPDATE_STATUSBAR = 0,
    GUI_ADD_ENTRY,
    GUI_DELETE_ENTRY,
    GUI_REFRESH_ENTRYFIELD,
    GUI_REFRESH_ENTRYPASSWORD,
    GUI_REDO_IMPORT,
    GUI_UNDO_IMPORT,
    GUI_REDO_MERGESYNC,
    GUI_UNDO_MERGESYNC,
  };

  enum DependentsType {
    DT_BASE2ALIASES_MMAP = 0,
    DT_BASE2SHORTCUTS_MMAP,
    DT_ALIAS2BASE_MAP,
    DT_SHORTCUT2BASE_MAP
  };

protected:
  void SaveState();
  void RestoreState();

  void SaveDependentsState(const Command::DependentsType itype);
  void RestoreDependentsState(const Command::DependentsType itype);

  void DoUpdateGUI(const Command::GUI_Action ga);
  void DoGUICmd(const Command::ExecuteFn &When, PWSGUICmdIF *pGUICmdIF);

  void DoUpdateDBPrefs(const StringX &sxNewDBPrefs);
  void UndoUpdateDBPrefs(const StringX &sxOldDBPrefs, const bool m_bOldState);
 
  void DoAddEntry(CItemData &ci);
  void DoDeleteEntry(CItemData &ci);
  void DoEditEntry(CItemData &old_ci, CItemData &new_ci);
  void UndoEditEntry(CItemData &old_ci, CItemData &new_ci);

  void DoUpdateEntry(const uuid_array_t &entry_uuid,
                     const CItemData::FieldType &ftype,
                     const StringX &value,
                     const CItemData::EntryStatus es);

  void DoUpdatePassword(const uuid_array_t &entry_uuid,
                        const StringX sxNewPassword);
  void UndoUpdatePassword(const uuid_array_t &entry_uuid,
                          const StringX &sxOldPassword, const StringX &sxOldPWHistory,
                          const CItemData::EntryStatus es);

  void DoAddDependentEntry(const uuid_array_t &base_uuid,
                           const uuid_array_t &entry_uuid,
                           const CItemData::EntryType type);
  int DoAddDependentEntries(UUIDList &dependentslist, CReport *rpt,
                            const CItemData::EntryType type,
                            const int &iVia,
                            ItemList *pmapDeletedItems,
                            SaveTypePWMap *pmapSaveTypePW);
  void UndoAddDependentEntries(ItemList *pmapDeletedItems,
                               SaveTypePWMap *pmapSaveTypePW);
  void DoRemoveDependentEntry(const uuid_array_t &base_uuid,
                              const uuid_array_t &entry_uuid,
                              const CItemData::EntryType type);
  void DoRemoveAllDependentEntries(const uuid_array_t &base_uuid,
                                   const CItemData::EntryType type);
  void UndoRemoveAllDependentEntries(const uuid_array_t &base_uuid,
                                     const CItemData::EntryType type);
  void DoMoveDependentEntries(const uuid_array_t &from_baseuuid,
                              const uuid_array_t &to_baseuuid,
                              const CItemData::EntryType type);
  void DoResetAllAliasPasswords(const uuid_array_t &base_uuid,
                                std::vector<CUUIDGen> &vSavedAliases);
  void UndoResetAllAliasPasswords(const uuid_array_t &base_uuid,
                                  std::vector<CUUIDGen> &vSavedAliases);
  int DoUpdatePasswordHistory(const int iAction, const int new_default_max,
                              SavePWHistoryMap &mapSavedHistory);
  void UndoUpdatePasswordHistory(SavePWHistoryMap &mapSavedHistory);

protected:
  PWScore *m_pcore;
  bool m_bSaveDBChanged;
  bool m_bNotifyGUI;
  int m_RC;
  ExecuteFn m_When;
  bool m_bState;

  // Alias/Shortcut structures
  // Permanent Multimap: since potentially more than one alias/shortcut per base
  //  Key = base uuid; Value = multiple alias/shortcut uuids
  std::multimap<CUUIDGen, CUUIDGen, CUUIDGen::ltuuid> m_saved_base2aliases_mmap;
  std::multimap<CUUIDGen, CUUIDGen, CUUIDGen::ltuuid> m_saved_base2shortcuts_mmap;

  // Permanent Map: since an alias only has one base
  //  Key = alias/shortcut uuid; Value = base uuid
  std::map<CUUIDGen, CUUIDGen, CUUIDGen::ltuuid> m_saved_alias2base_map;
  std::map<CUUIDGen, CUUIDGen, CUUIDGen::ltuuid> m_saved_shortcut2base_map;

  // Changed groups
  std::vector<StringX> m_saved_vnodes_modified;
};

// Derived MultiCommands class

class MultiCommands : public Command
{
public:
  MultiCommands(PWScore *pcore);
  ~MultiCommands();
  int Execute();
  int Redo();
  void Undo();

  void Add(Command *c);
  bool Remove(Command *c);
  bool Remove();
  bool GetRC(Command *c, int &rc);
  bool GetRC(const size_t ncmd, int &rc);
  PWScore *GetCore()
  {return m_pcore;}
  std::size_t GetSize()
  {return m_pcmds == NULL ? (std::size_t)(-1) : m_pcmds->size();}

private:
  std::vector<Command *> *m_pcmds;
  std::vector<int> m_RCs;
};

// GUI related commands

// Used mainly in MultiCommands to delay the GUI update until all the
// Commands have been executed e.g. Import.
// Add one at the start for a GUI update during an Undo and one at the
// end for a GUI update during Execute or Redo

class UpdateGUICommand : public Command
{
public:
  UpdateGUICommand(PWScore *pcore, const Command::ExecuteFn When,
                   const Command::GUI_Action ga);
  int Execute();
  int Redo();
  void Undo();

private:
  Command::GUI_Action m_ga;
};

class GUICommand : public Command
{
public:
  GUICommand(PWScore *pcore, PWSGUICmdIF *pGUICmdIF);
  ~GUICommand();
  int Execute();
  int Redo();
  void Undo();

private:
  PWSGUICmdIF *m_pGUICmdIF;
};

// PWS related commands

class DBPrefsCommand : public Command
{
public:
  DBPrefsCommand(PWScore *pcore, StringX &sxNewDBPrefs);
  int Execute();
  int Redo();
  void Undo();

private:
  StringX m_sxOldDBPrefs;
  StringX m_sxNewDBPrefs;
  bool m_bOldState;
};

class AddEntryCommand : public Command
{
public:
  AddEntryCommand(PWScore *pcore, CItemData &ci);
  ~AddEntryCommand();
  int Execute();
  int Redo();
  void Undo();

private:
  CItemData m_ci;
};

class DeleteEntryCommand : public Command
{
public:
  DeleteEntryCommand(PWScore *pcore, CItemData &ci);
  ~DeleteEntryCommand();
  int Execute();
  int Redo();
  void Undo();

private:
  CItemData m_ci;
};

class EditEntryCommand : public Command
{
public:
  EditEntryCommand(PWScore *pcore, CItemData &old_ci, CItemData &new_ci);
  ~EditEntryCommand();
  int Execute();
  int Redo();
  void Undo();

private:
  CItemData m_old_ci;
  CItemData m_new_ci;
};

class UpdateEntryCommand : public Command
{
public:
  UpdateEntryCommand(PWScore *pcore, CItemData &ci, const CItemData::FieldType &ftype,
                     const StringX &value);
  int Execute();
  int Redo();
  void Undo();

private:
  uuid_array_t m_entry_uuid;
  CItemData::FieldType m_ftype;
  StringX m_value, m_old_value;
  CItemData::EntryStatus m_old_status;
};

class UpdatePasswordCommand : public Command
{
public:
  UpdatePasswordCommand(PWScore *pcore, CItemData &ci, const StringX sxNewPassword);
  int Execute();
  int Redo();
  void Undo();

private:
  uuid_array_t m_entry_uuid;
  StringX m_sxNewPassword, m_sxOldPassword, m_sxOldPWHistory;
  CItemData::EntryStatus m_old_status;
};

class AddDependentEntryCommand : public Command
{
public:
  AddDependentEntryCommand(PWScore *pcore, const uuid_array_t &base_uuid,
                           const uuid_array_t &entry_uuid,
                           const CItemData::EntryType type);
  int Execute();
  int Redo();
  void Undo();

private:
  uuid_array_t m_base_uuid;
  uuid_array_t m_entry_uuid;
  CItemData::EntryType m_type;
};

class AddDependentEntriesCommand : public Command
{
public:
  AddDependentEntriesCommand(PWScore *pcore, UUIDList &dependentslist, CReport *rpt,
                             const CItemData::EntryType type,
                             const int &iVia);
  ~AddDependentEntriesCommand();
  int Execute();
  int Redo();
  void Undo();

private:
  UUIDList m_dependentslist;
  ItemList *m_pmapDeletedItems;
  SaveTypePWMap *m_pmapSaveStatus;
  CReport *m_rpt;
  CItemData::EntryType m_type;
  int m_iVia;
};

class RemoveDependentEntryCommand : public Command
{
public:
  RemoveDependentEntryCommand(PWScore *pcore,
                              const uuid_array_t &base_uuid,
                              const uuid_array_t &entry_uuid,
                              const CItemData::EntryType type);
  int Execute();
  int Redo();
  void Undo();

private:
  uuid_array_t m_base_uuid;
  uuid_array_t m_entry_uuid;
  CItemData::EntryType m_type;
};

class RemoveDependentEntriesCommand : public Command
{
public:
  RemoveDependentEntriesCommand(PWScore *pcore,
                                const uuid_array_t &base_uuid,
                                const uuid_array_t &entry_uuid,
                                const CItemData::EntryType type);
  int Execute();
  int Redo();
  void Undo();

private:
  uuid_array_t m_base_uuid;
  uuid_array_t m_entry_uuid;
  CItemData::EntryType m_type;
};

class RemoveAllDependentEntriesCommand : public Command
{
public:
  RemoveAllDependentEntriesCommand(PWScore *pcore, const uuid_array_t &base_uuid,
                                   const CItemData::EntryType type);
  int Execute();
  int Redo();
  void Undo();

private:
  uuid_array_t m_base_uuid;
  uuid_array_t m_entry_uuid;
  CItemData::EntryType m_type;
};

class MoveDependentEntriesCommand : public Command
{
public:
  MoveDependentEntriesCommand(PWScore *pcore, const uuid_array_t &from_baseuuid,
                              const uuid_array_t &to_baseuuid,
                              const CItemData::EntryType type);
  int Execute();
  int Redo();
  void Undo();

private:
  uuid_array_t m_from_baseuuid;
  uuid_array_t m_to_baseuuid;
  CItemData::EntryType m_type;
};

class ResetAllAliasPasswordsCommand : public Command
{
public:
  ResetAllAliasPasswordsCommand(PWScore *pcore, const uuid_array_t &base_uuid);
  int Execute();
  int Redo();
  void Undo();

private:
  uuid_array_t m_base_uuid;
  std::vector<CUUIDGen> m_vSavedAliases;
};

class UpdatePasswordHistoryCommand : public Command
{
public:
  UpdatePasswordHistoryCommand(PWScore *pcore, const int iAction,
                               const int new_default_max);
  int Execute();
  int Redo();
  void Undo();

private:
  int m_iAction;
  int m_new_default_max;
  SavePWHistoryMap m_mapSavedHistory;
};

#endif