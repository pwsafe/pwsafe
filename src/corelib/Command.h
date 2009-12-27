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
class CommandInterface;

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
  Command(CommandInterface *pcomInt);
  virtual ~Command();
  virtual int Execute() = 0;
  virtual int Redo() = 0;
  virtual void Undo() = 0;

  void SetNoGUINotify() {m_bNotifyGUI = false;}
  void ResetSavedState(bool bNewDBState) {m_bSaveDBChanged = bNewDBState;}

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

protected:
  void SaveState();
  void RestoreState();

protected:
  CommandInterface *m_pcomInt;
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
  MultiCommands(CommandInterface *pcomInt);
  ~MultiCommands();
  int Execute();
  int Redo();
  void Undo();

  void Add(Command *c);
  bool Remove(Command *c);
  bool Remove();
  bool GetRC(Command *c, int &rc);
  bool GetRC(const size_t ncmd, int &rc);
  std::size_t GetSize() const {return m_cmds.size();}
  void UpdateField(CItemData &ci, CItemData::FieldType ftype, StringX value);
  void AddEntry(CItemData &ci);
  void AddDependentEntry(const uuid_array_t &base_uuid, 
                         const uuid_array_t &entry_uuid,
                         const CItemData::EntryType type);
 private:
  std::vector<Command *> m_cmds;
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
  UpdateGUICommand(CommandInterface *pcomInt, const Command::ExecuteFn When,
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
  GUICommand(CommandInterface *pcomInt, PWSGUICmdIF *pGUICmdIF);
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
  DBPrefsCommand(CommandInterface *pcomInt, StringX &sxNewDBPrefs);
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
  AddEntryCommand(CommandInterface *pcomInt, CItemData &ci);
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
  DeleteEntryCommand(CommandInterface *pcomInt, CItemData &ci);
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
  EditEntryCommand(CommandInterface *pcomInt, CItemData &old_ci, CItemData &new_ci);
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
  UpdateEntryCommand(CommandInterface *pcomInt, CItemData &ci,
                     const CItemData::FieldType &ftype,
                     const StringX &value);
  int Execute();
  int Redo();
  void Undo();

private:
  void Doit(const uuid_array_t &entry_uuid,
            const CItemData::FieldType &ftype,
            const StringX &value,
            const CItemData::EntryStatus es);

  uuid_array_t m_entry_uuid;
  CItemData::FieldType m_ftype;
  StringX m_value, m_old_value;
  CItemData::EntryStatus m_old_status;
};

class UpdatePasswordCommand : public Command
{
public:
  UpdatePasswordCommand(CommandInterface *pcomInt, CItemData &ci, const StringX sxNewPassword);
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
  AddDependentEntryCommand(CommandInterface *pcomInt, const uuid_array_t &base_uuid,
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
  AddDependentEntriesCommand(CommandInterface *pcomInt, UUIDList &dependentslist, CReport *rpt,
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
  RemoveDependentEntryCommand(CommandInterface *pcomInt,
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
  RemoveDependentEntriesCommand(CommandInterface *pcomInt,
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
  RemoveAllDependentEntriesCommand(CommandInterface *pcomInt, const uuid_array_t &base_uuid,
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
  MoveDependentEntriesCommand(CommandInterface *pcomInt, const uuid_array_t &from_baseuuid,
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
  ResetAllAliasPasswordsCommand(CommandInterface *pcomInt, const uuid_array_t &base_uuid);
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
  UpdatePasswordHistoryCommand(CommandInterface *pcomInt, const int iAction,
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
