/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// Command.cpp
//-----------------------------------------------------------------------------

#include "CommandInterface.h"
#include "Command.h"
#include "PWSprefs.h"

#include <algorithm>

// ------------------------------------------------
// Base class: Command
// ------------------------------------------------

/*

The base class provides Save/restore state functions to enable Undo/Redo to be
correctly performed.

All GUI functions should ONLY use Command-derived classes to update any values in
the core.  None should be updated directly.

The MultiCommands derived class allows multiple commands to be lumped together
as one unit of work (in terms of Execute, Undo & Redo)

All other derived classes are aptly named to indicate their function.

There are 2 special derived classes:
1. UpdateGUICommand - which calls the GUI to allow it to update what the user sees
after any change.  For MultiCommands, it is normal to turn off GUI notification
during the execution of the individual commands and only notify the GUI at the 
end (e.g. after importing a lot of entries or after undoing the importing of these
entries).  Flags can be set on when to do this notification.

2. GUICommand - which allows the GUI to add GUI related commands to a MultiCommand
unit of work.

*/

Command::Command(CommandInterface *pcomInt)
:  m_pcomInt(pcomInt), m_bNotifyGUI(true), m_RC(0), m_bState(false)
{
}

Command::~Command()
{
}

// Save/restore state
void Command::SaveState()
{
  m_bSaveDBChanged = m_pcomInt->IsChanged();
  m_bUniqueGTUValidated = m_pcomInt->GetUniqueGTUValidated();
  m_saved_vnodes_modified = m_pcomInt->GetVnodesModified();
}

void Command::RestoreState()
{
  m_pcomInt->SetDBChanged(m_bSaveDBChanged);
  m_pcomInt->SetUniqueGTUValidated(m_bUniqueGTUValidated);
  m_pcomInt->SetVnodesModified(m_saved_vnodes_modified);
}

// ------------------------------------------------
// MultiCommands
// ------------------------------------------------

MultiCommands::MultiCommands(CommandInterface *pcomInt)
  : Command(pcomInt)
{
}

MultiCommands::~MultiCommands()
{
  std::vector<Command *>::iterator cmd_Iter;

  for (cmd_Iter = m_vpcmds.begin(); cmd_Iter != m_vpcmds.end(); cmd_Iter++) {
    delete (*cmd_Iter);
  }
}

int MultiCommands::Execute()
{
  std::vector<Command *>::iterator cmd_Iter;

  pws_os::Trace(_T("Multicommands Execute\n"));
  for (cmd_Iter = m_vpcmds.begin(); cmd_Iter != m_vpcmds.end(); cmd_Iter++) {
    int rc = (*cmd_Iter)->Execute();
    m_vRCs.push_back(rc);
  }
  m_bState = true;
  return 0;
}

int MultiCommands::Redo()
{
  pws_os::Trace(_T("Multicommands Redo\n"));
  return Execute();
}

void MultiCommands::Undo()
{
  std::vector<Command *>::reverse_iterator cmd_rIter;

  pws_os::Trace(_T("Multicommands Undo\n"));
  for (cmd_rIter = m_vpcmds.rbegin(); cmd_rIter != m_vpcmds.rend(); cmd_rIter++) {
    (*cmd_rIter)->Undo();
  }
  m_bState = false;
}

void MultiCommands::Add(Command *pcmd)
{
  pws_os::Trace(_T("Multicommands Add\n"));
  m_vpcmds.push_back(pcmd);
}

bool MultiCommands::Remove(Command *pcmd)
{
  std::vector<Command *>::iterator cmd_Iter;

  pws_os::Trace(_T("Multicommands Remove\n"));
  cmd_Iter = find(m_vpcmds.begin(), m_vpcmds.end(), pcmd);
  if (cmd_Iter != m_vpcmds.end()) {
    delete (*cmd_Iter);
    m_vpcmds.erase(cmd_Iter);
    return true;
  } else
    return false;
}

bool MultiCommands::Remove()
{
  pws_os::Trace(_T("Multicommands Remove\n"));
  if (!m_vpcmds.empty()) {
    delete m_vpcmds.back();
    m_vpcmds.pop_back();
    return true;
  } else
    return false;
}

bool MultiCommands::GetRC(Command *pcmd, int &rc)
{
  std::vector<Command *>::iterator cmd_Iter;

  pws_os::Trace(_T("Multicommands GetRC\n"));
  cmd_Iter = find(m_vpcmds.begin(), m_vpcmds.end(), pcmd);
  if (cmd_Iter != m_vpcmds.end()) {
    rc = m_vRCs[cmd_Iter - m_vpcmds.begin()];
    return true;
  } else {
    rc = 0;
    return false;
  }
}

bool MultiCommands::GetRC(const size_t ncmd, int &rc)
{
  if (ncmd <= 0 || ncmd > m_vRCs.size()) {
    rc = 0;
    return false;
  } else {
    rc = m_vRCs[ncmd - 1];
    return true;
  }
}

void MultiCommands::ResetSavedState(bool bNewDBState)
{
  Command::ResetSavedState(bNewDBState);
  std::vector<Command *>::iterator cmd_Iter;
  for (cmd_Iter = m_vpcmds.begin(); cmd_Iter != m_vpcmds.end(); cmd_Iter++) {
    (*cmd_Iter)->ResetSavedState(bNewDBState);
  }
}

// ------------------------------------------------
// UpdateGUICommand
// ------------------------------------------------

UpdateGUICommand::UpdateGUICommand(CommandInterface *pcomInt,
                                   ExecuteFn When, GUI_Action ga)
  : Command(pcomInt), m_When(When), m_ga(ga)
{
}

int UpdateGUICommand::Execute()
{
  pws_os::Trace(_T("UpdateGUICommand Execute\n"));
  if (m_When == WN_EXECUTE || m_When == WN_EXECUTE_REDO || 
      m_When == WN_ALL) {
    uuid_array_t entry_uuid = {'0'}; // dummy
    m_pcomInt->NotifyGUINeedsUpdating(m_ga, entry_uuid);
  }
  return 0;
}

int UpdateGUICommand::Redo()
{
  pws_os::Trace(_T("UpdateGUICommand Redo\n"));
  if (m_When == WN_REDO || m_When == WN_EXECUTE_REDO || 
      m_When == WN_ALL) {
    uuid_array_t entry_uuid = {'0'}; // dummy
    m_pcomInt->NotifyGUINeedsUpdating(m_ga, entry_uuid);
  }
  return 0;
}

void UpdateGUICommand::Undo()
{
  pws_os::Trace(_T("UpdateGUICommand Undo\n"));
  if (m_When == WN_UNDO || m_When == WN_ALL) {
    uuid_array_t entry_uuid = {'0'}; // dummy
    m_pcomInt->NotifyGUINeedsUpdating(m_ga, entry_uuid);
  }
}

// ------------------------------------------------
// DBPrefsCommand
// ------------------------------------------------

DBPrefsCommand::DBPrefsCommand(CommandInterface *pcomInt, StringX &sxDBPrefs)
  : Command(pcomInt), m_sxNewDBPrefs(sxDBPrefs)
{
  m_bOldState = PWSprefs::GetInstance()->IsDBprefsChanged();
  m_sxOldDBPrefs = PWSprefs::GetInstance()->Store();
}

int DBPrefsCommand::Execute()
{
  if (m_pcomInt->IsReadOnly())
    return 0;

  PWSprefs::GetInstance()->Load(m_sxNewDBPrefs);
  m_pcomInt->SetDBPrefsChanged(m_pcomInt->HaveHeaderPreferencesChanged(m_sxNewDBPrefs));

  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid = {'0'}; // dummy
    m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_DB_PREFERENCES_CHANGED,
                                      entry_uuid);
  }

  m_bState = true;
  return 0;
}

int DBPrefsCommand::Redo()
{
  return Execute();
}

void DBPrefsCommand::Undo()
{
  if (m_pcomInt->IsReadOnly())
    return;

  PWSprefs::GetInstance()->Load(m_sxOldDBPrefs);
  m_pcomInt->SetDBPrefsChanged(m_bOldState);

  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid = {'0'}; // dummy
    m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_DB_PREFERENCES_CHANGED,
                                      entry_uuid);
  }

  m_bState = false;
}

// ------------------------------------------------
// AddEntryCommand
// ------------------------------------------------

AddEntryCommand::AddEntryCommand(CommandInterface *pcomInt, const CItemData &ci)
  : Command(pcomInt), m_ci(ci)
{
  ASSERT(!ci.IsDependent()); // use other c'tor for dependent entries!
}

AddEntryCommand::AddEntryCommand(CommandInterface *pcomInt, const CItemData &ci
                                 , const uuid_array_t base_uuid)
  : Command(pcomInt), m_ci(ci)
{
  memcpy(m_base_uuid, base_uuid, sizeof(uuid_array_t));
}

AddEntryCommand::~AddEntryCommand()
{
}

int AddEntryCommand::Execute()
{
  SaveState();
  pws_os::Trace(_T("Command DoAddEntry\n"));
  if (m_pcomInt->IsReadOnly())
    return 0;

  m_pcomInt->DoAddEntry(m_ci);
  m_pcomInt->AddChangedNodes(m_ci.GetGroup());

  if (m_ci.IsDependent()) {
    uuid_array_t entry_uuid;
    m_ci.GetUUID(entry_uuid);
    m_pcomInt->DoAddDependentEntry(m_base_uuid, entry_uuid, m_ci.GetEntryType());
  }
  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid;
    m_ci.GetUUID(entry_uuid);
    m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_ADD_ENTRY,
                                      entry_uuid);
  }
  m_bState = true;
  return 0;
}

int AddEntryCommand::Redo()
{
  return Execute();
}

void AddEntryCommand::Undo()
{
  DeleteEntryCommand dec(m_pcomInt, m_ci);
  dec.Execute();
  if (m_ci.IsDependent()) {
    uuid_array_t entry_uuid;
    m_ci.GetUUID(entry_uuid);
    m_pcomInt->DoRemoveDependentEntry(m_base_uuid, entry_uuid, m_ci.GetEntryType());
  }
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// DeleteEntryCommand
// ------------------------------------------------

DeleteEntryCommand::DeleteEntryCommand(CommandInterface *pcomInt,
                                       const CItemData &ci)
  : Command(pcomInt), m_ci(ci), m_dependents(0)
{
  if (ci.IsNormal())
    memset(m_base_uuid, 0, sizeof(uuid_array_t));
  else {
    uuid_array_t uuid;
    ci.GetUUID(uuid);
    // If ci is not a normal entry, gather the related entry
    // info for undo
    if (ci.IsDependent()) {
      // For aliases or shortcuts, we just need the uuid of the base entry
      const ItemMap &imap = (ci.IsAlias() ? pcomInt->GetAlias2BaseMap() :
                             pcomInt->GetShortcuts2BaseMap());
      imap.find(CUUIDGen(uuid))->second.GetUUID(m_base_uuid);
    } else if (ci.IsBase()) {
      /**
       * When a shortcut base is deleted, we need to save all
       * the shortcuts referencing it, as they too are deleted.
       * When an alias base is deleted, we need the uuids of all its
       * dependents, to change their passwords back upon undo
       * To save code, we just keep the entire entry, same as shortcuts
      */
      const ItemMMap &immap = 
        ci.IsShortcutBase() ? pcomInt->GetBase2ShortcutsMmap() : pcomInt->GetBase2AliasesMmap();
      ItemMMapConstIter iter;
      for (iter = immap.lower_bound(CUUIDGen(uuid));
           iter != immap.upper_bound(CUUIDGen(uuid)); iter++) {
        uuid_array_t dep_uuid;
        iter->second.GetUUID(dep_uuid);
        ItemListIter itemIter = pcomInt->Find(dep_uuid);
        ASSERT(itemIter != pcomInt->GetEntryEndIter());
        if (itemIter != pcomInt->GetEntryEndIter())
          m_dependents.push_back(itemIter->second);
      } // for all dependents
    } // IsBase
  } // !IsNormal
}

DeleteEntryCommand::~DeleteEntryCommand()
{
}

int DeleteEntryCommand::Execute()
{
  SaveState();
  pws_os::Trace(_T("DeleteEntryCommand::Execute()\n"));
  if (m_pcomInt->IsReadOnly())
    return 0;

  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid;
    m_ci.GetUUID(entry_uuid);
    m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_DELETE_ENTRY,
                                      entry_uuid);
  }

  m_pcomInt->DoDeleteEntry(m_ci);
  m_pcomInt->AddChangedNodes(m_ci.GetGroup());
  m_bState = true;
  return 0;
}

int DeleteEntryCommand::Redo()
{
  return Execute();
}

void DeleteEntryCommand::Undo()
{
  uuid_array_t uuid;
  m_ci.GetUUID(uuid);
  if (m_ci.IsDependent()) {
    Command *pcmd = AddEntryCommand::Create(m_pcomInt, m_ci, m_base_uuid);
    pcmd->Execute();
    delete pcmd;
  } else {
    AddEntryCommand undo(m_pcomInt, m_ci);
    undo.Execute();
    if (m_ci.IsShortcutBase()) { // restore dependents
      for (std::vector<CItemData>::iterator iter = m_dependents.begin();
           iter != m_dependents.end(); iter++) {
        Command *pcmd = AddEntryCommand::Create(m_pcomInt, *iter, uuid);
        pcmd->Execute();
        delete pcmd;
      }
    } else if (m_ci.IsAliasBase()) {
      // Undeleting an alias base means making all the dependents refer to the alias
      // again. Perhaps the easiest approach is to delete the existing entries
      // and create new aliases.
      for (std::vector<CItemData>::iterator iter = m_dependents.begin();
           iter != m_dependents.end(); iter++) {
        DeleteEntryCommand delExAlias(m_pcomInt, *iter);
        delExAlias.Execute(); // out with the old...
        uuid_array_t alias_uuid;
        iter->GetUUID(alias_uuid);
        Command *pcmd = AddEntryCommand::Create(m_pcomInt, *iter, uuid);
        pcmd->Execute(); // in with the new!
        delete pcmd;
      }
    }
  }
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// EditEntryCommand
// ------------------------------------------------

EditEntryCommand::EditEntryCommand(CommandInterface *pcomInt,
                                   const CItemData &old_ci,
                                   const CItemData &new_ci)
  : Command(pcomInt), m_old_ci(old_ci), m_new_ci(new_ci)
{
  // We're only supposed to operate on entries
  // with same uuids, and possibly different fields
  uuid_array_t old_uuid, new_uuid;
  m_old_ci.GetUUID(old_uuid);
  m_new_ci.GetUUID(new_uuid);
  ASSERT(CUUIDGen(old_uuid) == CUUIDGen(new_uuid));
}

EditEntryCommand::~EditEntryCommand()
{
}

int EditEntryCommand::Execute()
{
  SaveState();
  pws_os::Trace(_T("EditEntry::Execute\n"));
  if (m_pcomInt->IsReadOnly())
    return 0;

  m_pcomInt->DoReplaceEntry(m_old_ci, m_new_ci);

  m_pcomInt->AddChangedNodes(m_old_ci.GetGroup());
  m_pcomInt->AddChangedNodes(m_new_ci.GetGroup());
  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid;
    m_old_ci.GetUUID(entry_uuid);
    // if the group's changed, refresh the entire tree, otherwise, just the field
    UpdateGUICommand::GUI_Action gac = (m_old_ci.GetGroup() != m_new_ci.GetGroup()) ?
      UpdateGUICommand::GUI_REFRESH_TREE : UpdateGUICommand::GUI_REFRESH_ENTRYFIELD;
    m_pcomInt->NotifyGUINeedsUpdating(gac, entry_uuid);
  }
  m_bState = true;
  return 0;
}

int EditEntryCommand::Redo()
{
  return Execute();
}

void EditEntryCommand::Undo()
{
  pws_os::Trace(_T("EditEntry::Undo\n"));
  if (m_pcomInt->IsReadOnly())
    return;

  m_pcomInt->DoReplaceEntry(m_new_ci, m_old_ci);

  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid;
    m_old_ci.GetUUID(entry_uuid);
    // if the group's changed, refresh the entire tree, otherwise, just the field
    UpdateGUICommand::GUI_Action gac = (m_old_ci.GetGroup() != m_new_ci.GetGroup()) ?
      UpdateGUICommand::GUI_REFRESH_TREE : UpdateGUICommand::GUI_REFRESH_ENTRYFIELD;
    m_pcomInt->NotifyGUINeedsUpdating(gac, entry_uuid);
  }
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// UpdateEntryCommand
// ------------------------------------------------

UpdateEntryCommand::UpdateEntryCommand(CommandInterface *pcomInt,
                                       const CItemData &ci,
                                       CItemData::FieldType ftype,
                                       const StringX &value)
  : Command(pcomInt), m_ftype(ftype), m_value(value)
{
  ci.GetUUID(m_entry_uuid);
  m_old_status = ci.GetStatus();
  m_old_value = ci.GetFieldValue(m_ftype);
}

void UpdateEntryCommand::Doit(const uuid_array_t &entry_uuid,
                              CItemData::FieldType ftype,
                              const StringX &value,
                              CItemData::EntryStatus es)
{
  pws_os::Trace(_T("UpdateEntryCommand::Doit\n"));
  if (m_pcomInt->IsReadOnly())
    return;

  ItemListIter pos = m_pcomInt->Find(entry_uuid);
  if (pos != m_pcomInt->GetEntryEndIter()) {
    pos->second.SetFieldValue(ftype, value);
    pos->second.SetStatus(es);
    m_pcomInt->SetDBChanged(true, false);
    m_pcomInt->AddChangedNodes(pos->second.GetGroup());
  }
}

int UpdateEntryCommand::Execute()
{
  SaveState();

  pws_os::Trace(_T("Command UpdateEntry: Field=0x%02x; Old Value=%s; NewValue=%s\n"),
    m_ftype, m_old_value.c_str(), m_value.c_str());

  Doit(m_entry_uuid, m_ftype, m_value, CItemData::ES_MODIFIED);

  if (m_bNotifyGUI)
    m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_REFRESH_ENTRYFIELD,
                                      m_entry_uuid, m_ftype);

  m_bState = true;
  return 0;
}

int UpdateEntryCommand::Redo()
{
  return Execute();
}

void UpdateEntryCommand::Undo()
{
  pws_os::Trace(_T("Command UndoUpdateEntry: Field=0x%02x; Old Value=%s;\n"),
    m_ftype, m_old_value.c_str());

  Doit(m_entry_uuid, m_ftype, m_old_value, m_old_status);
  RestoreState();

  if (m_bNotifyGUI)
    m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_REFRESH_ENTRYFIELD,
                                      m_entry_uuid, m_ftype);
  m_bState = false;
}

// ------------------------------------------------
// UpdatePasswordCommand
// ------------------------------------------------

UpdatePasswordCommand::UpdatePasswordCommand(CommandInterface *pcomInt,
                                             CItemData &ci,
                                             const StringX sxNewPassword)
  : Command(pcomInt), m_sxNewPassword(sxNewPassword)
{
  ci.GetUUID(m_entry_uuid);
  m_old_status = ci.GetStatus();
  m_sxOldPassword = ci.GetPassword();
  m_sxOldPWHistory = ci.GetPWHistory();
}

int UpdatePasswordCommand::Execute()
{
  SaveState();

  pws_os::Trace(_T("UpdatePassword::Execute\n"));
  if (m_pcomInt->IsReadOnly())
    return 0;

  ItemListIter pos = m_pcomInt->Find(m_entry_uuid);
  if (pos != m_pcomInt->GetEntryEndIter()) {
    pos->second.UpdatePassword(m_sxNewPassword);
    pos->second.SetStatus(CItemData::ES_MODIFIED);
    m_pcomInt->SetDBChanged(true, false);
    m_pcomInt->AddChangedNodes(pos->second.GetGroup());
  }

  if (m_bNotifyGUI)
    m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_REFRESH_ENTRYPASSWORD,
                                      m_entry_uuid);

  m_bState = true;
  return 0;
}

int UpdatePasswordCommand::Redo()
{
  return Execute();
}

void UpdatePasswordCommand::Undo()
{
  pws_os::Trace(_T("UpdatePasswordCommand::Undo\n"));
  if (m_pcomInt->IsReadOnly())
    return;

  ItemListIter pos = m_pcomInt->Find(m_entry_uuid);
  if (pos != m_pcomInt->GetEntryEndIter()) {
    pos->second.SetPassword(m_sxOldPassword);
    pos->second.SetPWHistory(m_sxOldPWHistory);
    pos->second.SetStatus(m_old_status);
  }
  RestoreState();

  if (m_bNotifyGUI)
    m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_REFRESH_ENTRYPASSWORD,
                                      m_entry_uuid);
  m_bState = false;
}

// ------------------------------------------------
// AddDependentEntryCommand
// ------------------------------------------------

AddDependentEntryCommand::AddDependentEntryCommand(CommandInterface *pcomInt,
                                                   const uuid_array_t &base_uuid,
                                                   const uuid_array_t &entry_uuid,
                                                   const CItemData::EntryType type)
  : Command(pcomInt), m_type(type)
{
  memcpy(static_cast<void *>(m_base_uuid), static_cast<const void *>(base_uuid), sizeof(uuid_array_t));
  memcpy(static_cast<void *>(m_entry_uuid), static_cast<const void *>(entry_uuid), sizeof(uuid_array_t));
}

int AddDependentEntryCommand::Execute()
{
  pws_os::Trace(_T("AddDependentEntryCommand::Execute\n"));
  SaveState();
  if (m_pcomInt->IsReadOnly())
    return 0;

  m_pcomInt->DoAddDependentEntry(m_base_uuid, m_entry_uuid, m_type);
  m_bState = true;
  return 0;
}

int AddDependentEntryCommand::Redo()
{
  return Execute();
}

void AddDependentEntryCommand::Undo()
{
  pws_os::Trace(_T("AddDependentEntryCommand::Undo\n"));
  if (m_pcomInt->IsReadOnly())
    return;

  m_pcomInt->DoRemoveDependentEntry(m_base_uuid, m_entry_uuid, m_type);
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// AddDependentEntriesCommand
// ------------------------------------------------

AddDependentEntriesCommand::AddDependentEntriesCommand(CommandInterface *pcomInt,
                                                       UUIDVector &dependentslist,
                                                       CReport *pRpt,
                                                       CItemData::EntryType type,
                                                       int iVia)
  : Command(pcomInt), m_dependentslist(dependentslist), m_pRpt(pRpt),
    m_type(type), m_iVia(iVia)
{
  m_pmapDeletedItems = new ItemList;
  m_pmapSaveStatus = new SaveTypePWMap;
}

AddDependentEntriesCommand::~AddDependentEntriesCommand()
{
  delete m_pmapDeletedItems;
  delete m_pmapSaveStatus;
}

int AddDependentEntriesCommand::Execute()
{
  pws_os::Trace(_T("AddDependentEntriesCommand::Execute\n"));
  SaveState();
  if (m_type == CItemData::ET_ALIAS) {
    m_saved_base2aliases_mmap = m_pcomInt->GetBase2AliasesMmap();
    m_saved_alias2base_map = m_pcomInt->GetAlias2BaseMap();
  } else { // if !alias, assume shortcut
    m_saved_base2shortcuts_mmap = m_pcomInt->GetBase2ShortcutsMmap();
    m_saved_shortcut2base_map = m_pcomInt->GetShortcuts2BaseMap();
  }
  if (m_pcomInt->IsReadOnly())
    return 0;

  int rc =  m_pcomInt->DoAddDependentEntries(m_dependentslist, m_pRpt,
                                             m_type, m_iVia,
                                             m_pmapDeletedItems, m_pmapSaveStatus);
  m_bState = true;
  return rc;
}

int AddDependentEntriesCommand::Redo()
{
  return Execute();
}

void AddDependentEntriesCommand::Undo()
{
  pws_os::Trace(_T("AddDependentEntriesCommand::Undo\n"));
  if (m_pcomInt->IsReadOnly())
    return;

  m_pcomInt->UndoAddDependentEntries(m_pmapDeletedItems, m_pmapSaveStatus);
  if (m_type == CItemData::ET_ALIAS) {
    m_pcomInt->SetBase2AliasesMmap(m_saved_base2aliases_mmap);
    m_pcomInt->SetAlias2BaseMap(m_saved_alias2base_map);
  } else { // if !alias, assume shortcut
    m_pcomInt->SetBase2ShortcutsMmap(m_saved_base2shortcuts_mmap);
    m_pcomInt->SetShortcuts2BaseMap(m_saved_shortcut2base_map);
  }
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// RemoveDependentEntryCommand
// ------------------------------------------------

RemoveDependentEntryCommand::RemoveDependentEntryCommand(CommandInterface *pcomInt,
                                                         const uuid_array_t &base_uuid,
                                                         const uuid_array_t &entry_uuid,
                                                         const CItemData::EntryType type)
  : Command(pcomInt), m_type(type)
{
  memcpy(static_cast<void *>(m_base_uuid), static_cast<const void *>(base_uuid), sizeof(uuid_array_t));
  memcpy(static_cast<void *>(m_entry_uuid), static_cast<const void *>(entry_uuid), sizeof(uuid_array_t));
}

int RemoveDependentEntryCommand::Execute()
{
  pws_os::Trace(_T("RemoveDependentEntryCommand::Execute\n"));
  SaveState();
  if (m_pcomInt->IsReadOnly())
    return 0;

  m_pcomInt->DoRemoveDependentEntry(m_base_uuid, m_entry_uuid, m_type);
  m_bState = true;
  return 0;
}

int RemoveDependentEntryCommand::Redo()
{
  return Execute();
}

void RemoveDependentEntryCommand::Undo()
{
  pws_os::Trace(_T("RemoveDependentEntryCommand::Undo\n"));
  if (m_pcomInt->IsReadOnly())
    return;

  m_pcomInt->DoAddDependentEntry(m_base_uuid, m_entry_uuid, m_type);
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// MoveDependentEntriesCommand
// ------------------------------------------------

MoveDependentEntriesCommand::MoveDependentEntriesCommand(CommandInterface *pcomInt,
                                                         const uuid_array_t &from_baseuuid,
                                                         const uuid_array_t &to_baseuuid,
                                                         const CItemData::EntryType type)
  : Command(pcomInt), m_type(type)
{
  memcpy(static_cast<void *>(m_from_baseuuid), static_cast<const void *>(from_baseuuid), sizeof(uuid_array_t));
  memcpy(static_cast<void *>(m_to_baseuuid), static_cast<const void *>(to_baseuuid), sizeof(uuid_array_t));
}

int MoveDependentEntriesCommand::Execute()
{
  pws_os::Trace(_T("MoveDependentEntriesCommand::Execute\n"));
  SaveState();
  if (m_pcomInt->IsReadOnly())
    return 0;

  m_pcomInt->DoMoveDependentEntries(m_from_baseuuid, m_to_baseuuid, m_type);
  m_bState = true;
  return 0;
}

int MoveDependentEntriesCommand::Redo()
{
  return Execute();
}

void MoveDependentEntriesCommand::Undo()
{
  pws_os::Trace(_T("MoveDependentEntriesCommand::Undo\n"));
  if (m_pcomInt->IsReadOnly())
    return;

  m_pcomInt->DoMoveDependentEntries(m_to_baseuuid, m_from_baseuuid, m_type);
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// UpdatePasswordHistoryCommand
// ------------------------------------------------

UpdatePasswordHistoryCommand::UpdatePasswordHistoryCommand(CommandInterface *pcomInt,
                                                           const int iAction,
                                                           const int new_default_max)
 : Command(pcomInt), m_iAction(iAction), m_new_default_max(new_default_max)
{}

int UpdatePasswordHistoryCommand::Execute()
{
  SaveState();
  pws_os::Trace(_T("UpdatePasswordHistoryCommand::Execute\n"));
  if (m_pcomInt->IsReadOnly())
    return 0;

  int rc = m_pcomInt->DoUpdatePasswordHistory(m_iAction, m_new_default_max,
                                              m_mapSavedHistory);
  m_bState = true;
  return rc;
}

int UpdatePasswordHistoryCommand::Redo()
{
  return Execute();
}

void UpdatePasswordHistoryCommand::Undo()
{
  pws_os::Trace(_T("UpdatePasswordHistoryCommand::Undo\n"));
  if (m_pcomInt->IsReadOnly())
    return;

  m_pcomInt->UndoUpdatePasswordHistory(m_mapSavedHistory);
  RestoreState();
  m_bState = false;
}
