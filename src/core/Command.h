/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "PWSfile.h"
#include "StringX.h"
#include "os/UUID.h"

#include "coredefs.h"

#include <map>
#include <vector>
#include <typeinfo>

/**
 * Command-derived classes are used to support undo/redo.
 * All Command based objects are created via their static Create()
 * member function, and deleted by the CommandInterface implementation
 * when appropriate. All constructors are non-public, to ensure that
 * no Command object can be created on the stack.
 */

class MultiCommands;

// Base Command class

class Command
{
public:
  enum CommandDBChange { NONE = -1, MULTICOMMAND, DB, DBPREFS, DBHEADER, DBEMPTYGROUP, DBPOLICYNAMES,
    DBFILTERS };

  virtual ~Command();
  virtual int Execute() = 0;
  virtual int Redo() {return Execute();} // common case
  virtual void Undo() = 0;

  void SetNoGUINotify() {m_bNotifyGUI = false;}
  bool GetGUINotify() const {return m_bNotifyGUI;}

  // This tells a MultiCommand that this command could change an entry
  // as opposed to a DB preference, header, empty group, password policy or filter
  bool IsEntryChangeType() const { return m_CommandChangeType == DB; }

  // This states if something was actually changed
  bool WasDBChanged() const { return m_CommandDBChange != NONE; }

protected:
  Command(CommandInterface *pcomInt); // protected constructor!

  void SaveDBInformation();
  void RestoreDBInformation();

  // If a command is within a MultiCommand, do not save DB information
  // other than that needed for the actual command.
  // This prevents multiple copies of similar data that is only needed once
  // in a MultiCommand i.e. at creation and during Undo
  bool InMultiCommand() const { return m_bInMultiCommand; }

  friend class MultiCommands; // Yes, a derived class needs to be a friend here...
  void SetInMultiCommand() { m_bInMultiCommand = true; }
  
  CommandInterface *m_pcomInt;
  bool m_bNotifyGUI, m_bInMultiCommand;

  // Command return code (not all commands set this to anything other than zero)
  int m_RC;

  // If needed, to be used by a Multicommand that changes the DB or
  // a single command that changes the DB outside a MultiCommand
  std::vector<StringX> m_vSavedModifiedNodes;

  // This is the potential change type if it there is anything to do
  // It is set so that MultiCommands can tell whether to save DB information
  CommandDBChange m_CommandChangeType;

  // The command change value is ONLY set during Execute
  CommandDBChange m_CommandDBChange;
};

// GUI related commands

// Used mainly in MultiCommands to delay the GUI update until all the
// Commands have been executed e.g. Import.
// Add one at the start for a GUI update during an Undo and one at the
// end for a GUI update during Execute or Redo

class UpdateGUICommand : public Command
{
public:
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
    GUI_REFRESH_TREE,
    GUI_REFRESH_ENTRY,
    GUI_REFRESH_GROUPS,
    GUI_REFRESH_BOTHVIEWS,
    GUI_DB_PREFERENCES_CHANGED,
    GUI_PWH_CHANGED_IN_DB
  };

  // Note: entryUUID only used in GUI_REFRESH_ENTRY
  static UpdateGUICommand *Create(CommandInterface *pcomInt,
                                  ExecuteFn When, GUI_Action ga,
                                  const pws_os::CUUID &entryUUID = pws_os::CUUID::NullUUID())
  { return new UpdateGUICommand(pcomInt, When, ga, entryUUID); }
  int Execute();
  void Undo();

private:
  UpdateGUICommand& operator=(const UpdateGUICommand&); // Do not implement
  UpdateGUICommand(CommandInterface *pcomInt, ExecuteFn When,
                   GUI_Action ga, const pws_os::CUUID &entryUUID);
  const ExecuteFn m_When;
  const GUI_Action m_ga;
  pws_os::CUUID m_entryUUID;
};

// PWS related commands

class DBPrefsCommand : public Command
{
public:
  // Call Create without newHashIters when this hasn't changed.
  static DBPrefsCommand *Create(CommandInterface *pcomInt,
                                StringX &sxNewDBPrefs, uint32 newHashIters = 0)
  { return new DBPrefsCommand(pcomInt, sxNewDBPrefs, newHashIters); }
  int Execute();
  void Undo();

private:
  DBPrefsCommand(CommandInterface *pcomInt, StringX &sxNewDBPrefs, uint32 newHashIters);
  const StringX m_sxOldDBPrefs, m_sxNewDBPrefs;
  const uint32 m_oldHashIters, m_newHashIters;
};

class DBPolicyNamesCommand : public Command
{
public:
  enum Function {NP_ADDNEW = 0, NP_REPLACEALL};

  static DBPolicyNamesCommand *Create(CommandInterface *pcomInt,
                                PSWDPolicyMap &MapPSWDPLC,
                                Function function)
  { return new DBPolicyNamesCommand(pcomInt, MapPSWDPLC, function); }
  static DBPolicyNamesCommand *Create(CommandInterface *pcomInt,
                                StringX &sxPolicyName, PWPolicy &st_pp)
  { return new DBPolicyNamesCommand(pcomInt, sxPolicyName, st_pp); }
  int Execute();
  void Undo();

private:
  DBPolicyNamesCommand(CommandInterface *pcomInt, PSWDPolicyMap &MapPSWDPLC,
                       Function function);
  DBPolicyNamesCommand(CommandInterface *pcomInt, StringX &sxPolicyName,
                       PWPolicy &st_pp);

  PSWDPolicyMap m_OldMapPSWDPLC;
  PSWDPolicyMap m_NewMapPSWDPLC;
  StringX m_sxPolicyName;
  PWPolicy m_st_ppp;
  Function m_function;
  bool m_bSingleAdd;
};

class DBEmptyGroupsCommand : public Command
{
public:
  enum Function {EG_ADD = 0, EG_DELETE, EG_RENAME, EG_ADDALL = 10, EG_REPLACEALL, EG_RENAMEPATH};

  static DBEmptyGroupsCommand *Create(CommandInterface *pcomInt,
                                const std::vector<StringX> &vEmptyGroups,
                                Function function)
  { return new DBEmptyGroupsCommand(pcomInt, vEmptyGroups, function); }
  static DBEmptyGroupsCommand *Create(CommandInterface *pcomInt,
                                const StringX &sxEmptyGroup, Function function)
  { return new DBEmptyGroupsCommand(pcomInt, sxEmptyGroup, function); }
  static DBEmptyGroupsCommand *Create(CommandInterface *pcomInt,
                                const StringX &sxOldGroup, const StringX &sxNewGroup, Function function)
  { return new DBEmptyGroupsCommand(pcomInt, sxOldGroup, sxNewGroup, function); }
  int Execute();
  void Undo();

private:
  DBEmptyGroupsCommand(CommandInterface *pcomInt, const std::vector<StringX> &vEmptyGroups,
                       Function function);
  DBEmptyGroupsCommand(CommandInterface *pcomInt, const StringX &sxEmptyGroup,
                       Function function);
  DBEmptyGroupsCommand(CommandInterface *pcomInt, const StringX &sxOldGroup, const StringX &sxNewGroup,
                       Function function);

  std::vector<StringX> m_vOldEmptyGroups;
  std::vector<StringX> m_vNewEmptyGroups;
  StringX m_sxEmptyGroup, m_sxOldGroup, m_sxNewGroup;
  Function m_function;
  bool m_bSingleGroup;
};

class DeleteEntryCommand;

class AddEntryCommand : public Command
{
public:
  static AddEntryCommand *Create(CommandInterface *pcomInt, const CItemData &ci,
                                 const pws_os::CUUID &baseUUID = pws_os::CUUID::NullUUID(),
                                 const CItemAtt *att = nullptr, const Command *pcmd = nullptr)
  { return new AddEntryCommand(pcomInt, ci, baseUUID, att, pcmd); }
  ~AddEntryCommand();
  int Execute();
  void Undo();

  friend class DeleteEntryCommand; // allow access to c'tor

private:
  AddEntryCommand& operator=(const AddEntryCommand&); // Do not implement
  AddEntryCommand(CommandInterface *pcomInt, const CItemData &ci,
                  const pws_os::CUUID &baseUUID, const CItemAtt *att,
                  const Command *pcmd = nullptr);
  CItemData m_ci;
  CItemAtt m_att;
  bool m_bExpired;
};

class DeleteEntryCommand : public Command
{
public:
  static DeleteEntryCommand *Create(CommandInterface *pcomInt,
                                    const CItemData &ci,
                                    const Command *pcmd = nullptr)
  { return new DeleteEntryCommand(pcomInt, ci, pcmd); }
  ~DeleteEntryCommand();
  int Execute();
  void Undo();

  friend class AddEntryCommand; // allow access to c'tor

private:
  DeleteEntryCommand& operator=(const DeleteEntryCommand&); // Do not implement
  DeleteEntryCommand(CommandInterface *pcomInt, const CItemData &ci,
                     const Command *pcmd = nullptr);
  const CItemData m_ci;
  CItemAtt m_att;
  pws_os::CUUID m_base_uuid; // for undo of shortcut or alias deletion
  std::vector<CItemData> m_vdependents; // for undo of base deletion
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
  void Undo();

private:
  UpdateEntryCommand(CommandInterface *pcomInt, const CItemData &ci,
                     const CItemData::FieldType ftype,
                     const StringX &value);
  void Doit(const pws_os::CUUID &entry_uuid,
            CItemData::FieldType ftype,
            const StringX &value,
            CItemData::EntryStatus es,
            UpdateGUICommand::ExecuteFn efn);

  CItemData m_old_ci;
  CItemData m_new_ci;
  const CItemData::FieldType m_ftype;
};

class UpdatePasswordCommand : public Command
{
public:
  static UpdatePasswordCommand *Create(CommandInterface *pcomInt,
                                       CItemData &ci,
                                       const StringX sxNewPassword)
  { return new UpdatePasswordCommand(pcomInt, ci, sxNewPassword); }
  int Execute();
  void Undo();

private:
  UpdatePasswordCommand(CommandInterface *pcomInt,
                        const CItemData &ci, const StringX &sxNewPassword);
  const CItemData m_old_ci; // for uuid, password, password history, XTime, entry status
  CItemData m_new_ci; // for new password
};

class AddDependentEntryCommand : public Command
{
public:
  static AddDependentEntryCommand *Create(CommandInterface *pcomInt,
                                          const pws_os::CUUID &base_uuid,
                                          const pws_os::CUUID &entry_uuid,
                                          const CItemData::EntryType type)
  { return new AddDependentEntryCommand(pcomInt, base_uuid, entry_uuid, type); }
  int Execute();
  void Undo();

private:
  AddDependentEntryCommand(CommandInterface *pcomInt,
                           const pws_os::CUUID &base_uuid,
                           const pws_os::CUUID &entry_uuid,
                           const CItemData::EntryType type);
  pws_os::CUUID m_base_uuid;
  pws_os::CUUID m_entry_uuid;
  CItemData::EntryType m_type;
};

class AddDependentEntriesCommand : public Command
{
public:
  static AddDependentEntriesCommand *Create(CommandInterface *pcomInt,
                                            UUIDVector &dependentslist,
                                            CReport *pRpt,
                                            CItemData::EntryType type,
                                            int iVia)
  { return new AddDependentEntriesCommand(pcomInt, dependentslist, pRpt,
                                          type, iVia); }
  ~AddDependentEntriesCommand();
  int Execute();
  void Undo();

private:
  AddDependentEntriesCommand(CommandInterface *pcomInt,
                             UUIDVector &dependentslist, CReport *pRpt,
                             CItemData::EntryType type, int iVia);
  UUIDVector m_dependentslist;
  ItemList m_mapDeletedItems;
  SaveTypePWMap m_mapSaveStatus;
  CReport *m_pRpt;
  CItemData::EntryType m_type;
  int m_iVia;

  // Alias/Shortcut structures
  // Permanent Multimap: since potentially more than one alias/shortcut per base
  //  Key = base uuid; Value = multiple alias/shortcut uuids
  ItemMMap m_saved_base2aliases_mmap;
  ItemMMap m_saved_base2shortcuts_mmap;
};

class RemoveDependentEntryCommand : public Command
{
public:
  static RemoveDependentEntryCommand *Create(CommandInterface *pcomInt,
                                             const pws_os::CUUID &base_uuid,
                                             const pws_os::CUUID &entry_uuid,
                                             CItemData::EntryType type)
  { return new RemoveDependentEntryCommand(pcomInt, base_uuid, entry_uuid,
                                           type); }
  int Execute();
  void Undo();

private:
  RemoveDependentEntryCommand(CommandInterface *pcomInt,
                              const pws_os::CUUID &base_uuid,
                              const pws_os::CUUID &entry_uuid,
                              CItemData::EntryType type);
  pws_os::CUUID m_base_uuid;
  pws_os::CUUID m_entry_uuid;
  CItemData::EntryType m_type;
};

class MoveDependentEntriesCommand : public Command
{
public:
  static MoveDependentEntriesCommand *Create(CommandInterface *pcomInt,
                                             const pws_os::CUUID &from_baseuuid,
                                             const pws_os::CUUID &to_baseuuid,
                                             CItemData::EntryType type)
  { return new MoveDependentEntriesCommand(pcomInt, from_baseuuid, to_baseuuid,
                                           type); }
  int Execute();
  void Undo();

private:
  MoveDependentEntriesCommand(CommandInterface *pcomInt,
                              const pws_os::CUUID &from_baseuuid,
                              const pws_os::CUUID &to_baseuuid,
                              CItemData::EntryType type);
  pws_os::CUUID m_from_baseuuid;
  pws_os::CUUID m_to_baseuuid;
  CItemData::EntryType m_type;

  ItemMMap m_saved_base2aliases_mmap;
  ItemMMap m_saved_base2shortcuts_mmap;
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
  void Undo();

private:
  UpdatePasswordHistoryCommand(CommandInterface *pcomInt, int iAction,
                               int new_default_max);
  int m_iAction;
  int m_new_default_max;
  SavePWHistoryMap m_mapSavedHistory;
};

class RenameGroupCommand : public Command
{
public:
  static RenameGroupCommand *Create(CommandInterface *pcomInt,
                                    const StringX sxOldPath, const StringX sxNewPath)
  { return new RenameGroupCommand(pcomInt, sxOldPath, sxNewPath); }
  ~RenameGroupCommand();
  int Execute();
  void Undo();

  void GetPaths(StringX &sxOldPath, StringX &sxNewPath) const
  { sxOldPath = m_sxOldPath; sxNewPath = m_sxNewPath; }

private:
  RenameGroupCommand(CommandInterface *pcomInt,
                     StringX sxOldPath, StringX sxNewPath);

   StringX m_sxOldPath, m_sxNewPath;
   MultiCommands *m_pmulticmds;
};

class ChangeDBHeaderCommand : public Command {
public:
  static ChangeDBHeaderCommand *Create(CommandInterface *pcomInt,
    const StringX sxNewValue, const PWSfile::HeaderType ht)
  { return new ChangeDBHeaderCommand(pcomInt, sxNewValue, ht); }
  int Execute();
  void Undo();

private:
  ChangeDBHeaderCommand(CommandInterface *pcomInt,
    StringX sxOldValue, const PWSfile::HeaderType ht);

  StringX m_sxOldValue, m_sxNewValue;
  PWSfile::HeaderType m_ht;
};

class DBFiltersCommand : public Command {
public:
  static DBFiltersCommand *Create(CommandInterface *pcomInt,
    PWSFilters &MapFilters)
  { return new DBFiltersCommand(pcomInt, MapFilters); }

  int Execute();
  void Undo();

private:
  DBFiltersCommand(CommandInterface *pcomInt, PWSFilters &MapFilters);

  PWSFilters m_NewMapFilters;
  PWSFilters m_OldMapFilters;
};

// Derived MultiCommands class
class MultiCommands : public Command
{
public:
  static MultiCommands *Create(CommandInterface *pcomInt)
  { return new MultiCommands(pcomInt); }
  ~MultiCommands();
  int Execute();
  void Undo();

  void Add(Command *pcmd);
  void Insert(Command *pcmd, size_t ioffset = 0); // VERY INEFFICIENT - use sparingly
  bool GetRC(Command *pcmd, int &rc);
  bool GetRC(const size_t ncmd, int &rc);
  std::size_t GetSize() const {return m_vpcmds.size();}
  bool IsEmpty() const { return m_vpcmds.empty(); }
  void SetNested() { SetInMultiCommand(); }

  Command *FindCommand(const std::type_info &ti);

 private:
  MultiCommands(CommandInterface *pcomInt);

  std::vector<Command *> m_vpcmds;
  std::vector<int> m_vRCs;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Policies Management Commands
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Implements the management of policies within a collection.
 * 
 * Provides the ability for the individual policy specific commands 
 * to easily manage policies within a collection.
 */
class MultiPolicyCollector
{
private:
  PSWDPolicyMap& m_Policies;
  
protected:
  MultiPolicyCollector(PSWDPolicyMap& policies);
  virtual ~MultiPolicyCollector();
  
  void AddPolicy(const StringX& name, const PWPolicy& policy);
  void RemovePolicy(const StringX& name);
};

/**
 * Implements the management of a single policy, the default policy.
 * 
 * Needed by the policy specific command <code>PolicyCommandModify</code>, only.
 */
class SinglePolicyCollector
{
private:
  StringX   m_Name;
  PWPolicy& m_DefaultPolicy;
  
protected:
  SinglePolicyCollector(PWPolicy& defaultPolicy);
  virtual ~SinglePolicyCollector();
  
  void AddPolicy(const StringX& name, const PWPolicy& policy);
};

/**
 * This class implements the command for adding policies into a collection.
 */
class PolicyCommandAdd : public Command, public MultiPolicyCollector
{
private:
  StringX  m_Name;
  PWPolicy m_Policy;
  
protected:
public:
  PolicyCommandAdd(CommandInterface& commandInterface, PSWDPolicyMap& policies, const stringT& name, const PWPolicy& policy);
  ~PolicyCommandAdd();
  
  int Execute() override;
  void Undo() override;
};

/**
 * This class implements the command for removing policies from a collection.
 */
class PolicyCommandRemove : public Command, public MultiPolicyCollector
{
private:
  StringX  m_Name;
  PWPolicy m_Policy;
  
protected:
public:
  PolicyCommandRemove(CommandInterface& commandInterface, PSWDPolicyMap& policies, const stringT& name, const PWPolicy& policy);
  ~PolicyCommandRemove();
  
  int Execute() override;
  void Undo() override;
};

/**
 * Provides the command template for modifying of existing policy rules.
 * 
 * The Collector is responsible for handling the policy data of type T in a type depending manner.
 */
template <typename Collector, typename T>
class PolicyCommandModify : public Command, public Collector
{
private:
  StringX  m_Name;
  PWPolicy m_OriginalPolicy;
  PWPolicy m_ModifiedPolicy;

protected:
public:
  PolicyCommandModify(CommandInterface& commandInterface, T& data, const stringT& name, const PWPolicy& original, const PWPolicy& modified);
  ~PolicyCommandModify();
  
  int Execute() override;
  void Undo() override;
};

/**
 * This class implements the command for renaming of existing policies in a collection.
 * 
 * There are two possible types of changes that could have been applied to a policy.
 * Case 1) Policy name was changed, only.
 * Case 2) Policy name but also some policy attributes were changed
 * 
 * This class assums always the second case, so that even when only the name was changed 
 * a backup of the policy is created. That's why beside old and new name also two 
 * policies are expected.
 * 
 * To just fulfill the first case only the old and new name is necessary and the 
 * affected policy, that got renamed.
 * 
 * Assuming always case two, prevents additional checks what exactly has changed.
 * Only the name, only the policy rules or both?!
 */
class PolicyCommandRename : public Command, public MultiPolicyCollector
{
private:
  StringX  m_OldName;
  StringX  m_NewName;
  PWPolicy m_OriginalPolicy;
  PWPolicy m_ModifiedPolicy;

protected:
public:
  PolicyCommandRename(CommandInterface& commandInterface, PSWDPolicyMap& policies, const stringT& oldName, const stringT& newName, const PWPolicy& original, const PWPolicy& modified);
  ~PolicyCommandRename();
  
  int Execute() override;
  void Undo() override;
};

#endif /*  __COMMAND_H */
