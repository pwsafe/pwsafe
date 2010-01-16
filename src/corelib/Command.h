/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "coredefs.h"

#include <map>
#include <vector>

/**
 * Command-derived classes are used to support undo/redo.
 * All Command based objects are created via their static Create()
 * member function, and deleted by the CommandInterface implementation
 * when appropriate. All constructors are non-public, to ensure that
 * no Command object can be created on the stack.
 */

// Base Command class

class Command
{
public:
  virtual ~Command();
  virtual int Execute() = 0;
  virtual int Redo() = 0;
  virtual void Undo() = 0;

  void SetNoGUINotify() {m_bNotifyGUI = false;}
  void ResetSavedState(bool bNewDBState) {m_bSaveDBChanged = bNewDBState;}

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
    GUI_REFRESH_TREE,
    GUI_DB_PREFERENCES_CHANGED,
  };

  enum ExecuteFn {
    WN_INVALID = -1,
    WN_ALL = 0,
    WN_EXECUTE = 1,
    WN_REDO = 2,
    WN_EXECUTE_REDO = 3,
    WN_UNDO = 4
  };

protected:
  Command(CommandInterface *pcomInt); // protected constructor!
  void SaveState();
  void RestoreState();

protected:
  CommandInterface *m_pcomInt;
  bool m_bSaveDBChanged;
  bool m_bNotifyGUI;
  int m_RC;
  bool m_bState;

  // Changed groups
  std::vector<StringX> m_saved_vnodes_modified;
};

// Derived MultiCommands class
class AddEntryCommand; // silly fwd decl - move  MultiCommands to last?

class MultiCommands : public Command
{
public:
  static MultiCommands *Create(CommandInterface *pcomInt)
  { return new MultiCommands(pcomInt); }
  ~MultiCommands();
  int Execute();
  int Redo();
  void Undo();

  void Add(Command *pcmd);
  bool Remove(Command *pcmd);
  bool Remove();
  bool GetRC(Command *pcmd, int &rc);
  bool GetRC(const size_t ncmd, int &rc);
  std::size_t GetSize() const {return m_vpcmds.size();}

  // Some convenience routines:
  void AddEntry(const CItemData &ci);
  void UpdateField(CItemData &ci, CItemData::FieldType ftype, StringX value);
  // Following is used to change a simple AddEntryCommand command into
  // AddEntryCommand + AddDependentEntryCommand
  static MultiCommands *MakeAddDependentCommand(CommandInterface *pcomInt,
                                                Command *paec,
                                                const uuid_array_t &base_uuid, 
                                                const uuid_array_t &entry_uuid,
                                                CItemData::EntryType type);
 private:
  MultiCommands(CommandInterface *pcomInt);
  std::vector<Command *> m_vpcmds;
  std::vector<int> m_vRCs;
};

// GUI related commands

// Used mainly in MultiCommands to delay the GUI update until all the
// Commands have been executed e.g. Import.
// Add one at the start for a GUI update during an Undo and one at the
// end for a GUI update during Execute or Redo

class UpdateGUICommand : public Command
{
public:
  static UpdateGUICommand *Create(CommandInterface *pcomInt,
                                  Command::ExecuteFn When,
                                  GUI_Action ga)
  { return new UpdateGUICommand(pcomInt, When, ga); }
  int Execute();
  int Redo();
  void Undo();

private:
  UpdateGUICommand(CommandInterface *pcomInt, Command::ExecuteFn When,
                   GUI_Action ga);
  ExecuteFn m_When;
  GUI_Action m_ga;
};

// PWS related commands

class DBPrefsCommand : public Command
{
public:
  static DBPrefsCommand *Create(CommandInterface *pcomInt,
                                StringX &sxNewDBPrefs)
  { return new DBPrefsCommand(pcomInt, sxNewDBPrefs); }
  int Execute();
  int Redo();
  void Undo();

private:
  DBPrefsCommand(CommandInterface *pcomInt, StringX &sxNewDBPrefs);
  StringX m_sxOldDBPrefs;
  StringX m_sxNewDBPrefs;
  bool m_bOldState;
};

class DeleteEntryCommand;

class AddEntryCommand : public Command
{
public:
  static AddEntryCommand *Create(CommandInterface *pcomInt, const CItemData &ci)
  { return new AddEntryCommand(pcomInt, ci); }
  ~AddEntryCommand();
  int Execute();
  int Redo();
  void Undo();
  friend class DeleteEntryCommand; // allow access to c'tor
private:
  AddEntryCommand(CommandInterface *pcomInt, const CItemData &ci);
  const CItemData m_ci;
};

class DeleteEntryCommand : public Command
{
public:
  static DeleteEntryCommand *Create(CommandInterface *pcomInt,
                                    const CItemData &ci)
  { return new DeleteEntryCommand(pcomInt, ci); }
  ~DeleteEntryCommand();
  int Execute();
  int Redo();
  void Undo();
  friend class AddEntryCommand; // allow access to c'tor

private:
  DeleteEntryCommand(CommandInterface *pcomInt, const CItemData &ci);
  const CItemData m_ci;
  uuid_array_t m_base_uuid; // for undo of shortcut or alias deletion
  std::vector<CItemData> m_dependents; // for undo of base deletion
};

class EditEntryCommand : public Command
{
public:
  static EditEntryCommand *Create(CommandInterface *pcomInt,
                                  const CItemData &old_ci,
                                  const CItemData &new_ci)
  { return new EditEntryCommand(pcomInt, old_ci, new_ci); }
  ~EditEntryCommand();
  int Execute();
  int Redo();
  void Undo();

private:
  EditEntryCommand(CommandInterface *pcomInt, const CItemData &old_ci,
                   const CItemData &new_ci);
  CItemData m_old_ci;
  CItemData m_new_ci;
};

class UpdateEntryCommand : public Command
{
public:
  static UpdateEntryCommand *Create(CommandInterface *pcomInt,
                                    const CItemData &ci,
                                    CItemData::FieldType ftype,
                                    const StringX &value)
  { return new UpdateEntryCommand(pcomInt, ci, ftype, value); }
  int Execute();
  int Redo();
  void Undo();

private:
  UpdateEntryCommand(CommandInterface *pcomInt, const CItemData &ci,
                     const CItemData::FieldType ftype,
                     const StringX &value);
  void Doit(const uuid_array_t &entry_uuid,
            CItemData::FieldType ftype,
            const StringX &value,
            CItemData::EntryStatus es);

  uuid_array_t m_entry_uuid;
  CItemData::FieldType m_ftype;
  StringX m_value, m_old_value;
  CItemData::EntryStatus m_old_status;
};

class UpdatePasswordCommand : public Command
{
public:
  static UpdatePasswordCommand *Create(CommandInterface *pcomInt,
                                       CItemData &ci,
                                       const StringX sxNewPassword)
  { return new UpdatePasswordCommand(pcomInt, ci, sxNewPassword); }
  int Execute();
  int Redo();
  void Undo();

private:
  UpdatePasswordCommand(CommandInterface *pcomInt,
                        CItemData &ci, const StringX sxNewPassword);
  uuid_array_t m_entry_uuid;
  StringX m_sxNewPassword, m_sxOldPassword, m_sxOldPWHistory;
  CItemData::EntryStatus m_old_status;
};

class AddDependentEntryCommand : public Command
{
public:
  static AddDependentEntryCommand *Create(CommandInterface *pcomInt,
                                          const uuid_array_t &base_uuid,
                                          const uuid_array_t &entry_uuid,
                                          const CItemData::EntryType type)
  { return new AddDependentEntryCommand(pcomInt, base_uuid, entry_uuid, type); }
  int Execute();
  int Redo();
  void Undo();

private:
  AddDependentEntryCommand(CommandInterface *pcomInt,
                           const uuid_array_t &base_uuid,
                           const uuid_array_t &entry_uuid,
                           const CItemData::EntryType type);
  uuid_array_t m_base_uuid;
  uuid_array_t m_entry_uuid;
  CItemData::EntryType m_type;
};

class AddDependentEntriesCommand : public Command
{
public:
  static AddDependentEntriesCommand *Create(CommandInterface *pcomInt,
                                            UUIDList &dependentslist,
                                            CReport *pRpt,
                                            CItemData::EntryType type,
                                            int iVia)
  { return new AddDependentEntriesCommand(pcomInt, dependentslist, pRpt,
                                          type, iVia); }
  ~AddDependentEntriesCommand();
  int Execute();
  int Redo();
  void Undo();

private:
  AddDependentEntriesCommand(CommandInterface *pcomInt,
                             UUIDList &dependentslist, CReport *pRpt,
                             CItemData::EntryType type, int iVia);
  UUIDList m_dependentslist;
  ItemList *m_pmapDeletedItems;
  SaveTypePWMap *m_pmapSaveStatus;
  CReport *m_pRpt;
  CItemData::EntryType m_type;
  int m_iVia;
  // Alias/Shortcut structures
  // Permanent Multimap: since potentially more than one alias/shortcut per base
  //  Key = base uuid; Value = multiple alias/shortcut uuids
  ItemMMap m_saved_base2aliases_mmap;
  ItemMMap m_saved_base2shortcuts_mmap;

  // Permanent Map: since an alias only has one base
  //  Key = alias/shortcut uuid; Value = base uuid
  ItemMap m_saved_alias2base_map;
  ItemMap m_saved_shortcut2base_map;
};

class RemoveDependentEntryCommand : public Command
{
public:
  static RemoveDependentEntryCommand *Create(CommandInterface *pcomInt,
                                             const uuid_array_t &base_uuid,
                                             const uuid_array_t &entry_uuid,
                                             CItemData::EntryType type)
  { return new RemoveDependentEntryCommand(pcomInt, base_uuid, entry_uuid,
                                           type); }
  int Execute();
  int Redo();
  void Undo();

private:
  RemoveDependentEntryCommand(CommandInterface *pcomInt,
                              const uuid_array_t &base_uuid,
                              const uuid_array_t &entry_uuid,
                              CItemData::EntryType type);
  uuid_array_t m_base_uuid;
  uuid_array_t m_entry_uuid;
  CItemData::EntryType m_type;
};

class MoveDependentEntriesCommand : public Command
{
public:
  static MoveDependentEntriesCommand *Create(CommandInterface *pcomInt,
                                             const uuid_array_t &from_baseuuid,
                                             const uuid_array_t &to_baseuuid,
                                             CItemData::EntryType type)
  { return new MoveDependentEntriesCommand(pcomInt, from_baseuuid, to_baseuuid,
                                           type); }
  int Execute();
  int Redo();
  void Undo();

private:
  MoveDependentEntriesCommand(CommandInterface *pcomInt,
                              const uuid_array_t &from_baseuuid,
                              const uuid_array_t &to_baseuuid,
                              CItemData::EntryType type);
  uuid_array_t m_from_baseuuid;
  uuid_array_t m_to_baseuuid;
  CItemData::EntryType m_type;
};

class UpdatePasswordHistoryCommand : public Command
{
public:
  static UpdatePasswordHistoryCommand *Create(CommandInterface *pcomInt,
                                              int iAction,
                                              int new_default_max)
  { return new UpdatePasswordHistoryCommand(pcomInt, iAction,
                                            new_default_max); }
  int Execute();
  int Redo();
  void Undo();

private:
  UpdatePasswordHistoryCommand(CommandInterface *pcomInt, int iAction,
                               int new_default_max);
  int m_iAction;
  int m_new_default_max;
  SavePWHistoryMap m_mapSavedHistory;
};

#endif
