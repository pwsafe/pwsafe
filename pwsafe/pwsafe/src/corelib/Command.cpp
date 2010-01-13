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
  m_saved_vnodes_modified = m_pcomInt->GetVnodesModified();
}

void Command::RestoreState()
{
  m_pcomInt->SetDBChanged(m_bSaveDBChanged);
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

  TRACE(L"Multicommands Execute\n");
  for (cmd_Iter = m_vpcmds.begin(); cmd_Iter != m_vpcmds.end(); cmd_Iter++) {
    int rc = (*cmd_Iter)->Execute();
    m_vRCs.push_back(rc);
  }
  m_bState = true;
  return 0;
}

int MultiCommands::Redo()
{
  TRACE(L"Multicommands Redo\n");
  return Execute();
}

void MultiCommands::Undo()
{
  std::vector<Command *>::reverse_iterator cmd_rIter;

  TRACE(L"Multicommands Undo\n");
  for (cmd_rIter = m_vpcmds.rbegin(); cmd_rIter != m_vpcmds.rend(); cmd_rIter++) {
    (*cmd_rIter)->Undo();
  }
  m_bState = false;
}

void MultiCommands::Add(Command *pcmd)
{
  TRACE(L"Multicommands Add\n");
  m_vpcmds.push_back(pcmd);
}

bool MultiCommands::Remove(Command *pcmd)
{
  std::vector<Command *>::iterator cmd_Iter;

  TRACE(L"Multicommands Remove\n");
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
  TRACE(L"Multicommands Remove\n");
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

  TRACE(L"Multicommands GetRC\n");
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

void MultiCommands::AddEntry(const CItemData &ci)
{
  Add(AddEntryCommand::Create(m_pcomInt, ci));
}

void MultiCommands::UpdateField(CItemData &ci, CItemData::FieldType ftype, StringX value)
{
  Add(UpdateEntryCommand::Create(m_pcomInt, ci, ftype, value));
}

MultiCommands *MultiCommands::MakeAddDependentCommand(CommandInterface *pcomInt,
                                                      Command *paec,
                                                      const uuid_array_t &base_uuid, 
                                                      const uuid_array_t &entry_uuid,
                                                      CItemData::EntryType type)
{
  MultiCommands *pmc = new MultiCommands(pcomInt);
  pmc->Add(paec);
  pmc->Add(AddDependentEntryCommand::Create(pmc->m_pcomInt, base_uuid,
                                            entry_uuid, type));
  return pmc;
}

// ------------------------------------------------
// UpdateGUICommand
// ------------------------------------------------

UpdateGUICommand::UpdateGUICommand(CommandInterface *pcomInt,
                                   Command::ExecuteFn When,
                                   Command::GUI_Action ga)
  : Command(pcomInt), m_When(When), m_ga(ga)
{
}

int UpdateGUICommand::Execute()
{
  TRACE(L"UpdateGUICommand Execute\n");
  if (m_When == Command::WN_EXECUTE || m_When == Command::WN_EXECUTE_REDO || 
      m_When == Command::WN_ALL) {
    uuid_array_t entry_uuid = {'0'}; // dummy
    m_pcomInt->NotifyGUINeedsUpdating(m_ga, entry_uuid);
  }
  return 0;
}

int UpdateGUICommand::Redo()
{
  TRACE(L"UpdateGUICommand Redo\n");
  if (m_When == Command::WN_REDO || m_When == Command::WN_EXECUTE_REDO || 
      m_When == Command::WN_ALL) {
    uuid_array_t entry_uuid = {'0'}; // dummy
    m_pcomInt->NotifyGUINeedsUpdating(m_ga, entry_uuid);
  }
  return 0;
}

void UpdateGUICommand::Undo()
{
  TRACE(L"UpdateGUICommand Undo\n");
  if (m_When == Command::WN_UNDO || m_When == Command::WN_ALL) {
    uuid_array_t entry_uuid = {'0'}; // dummy
    m_pcomInt->NotifyGUINeedsUpdating(m_ga, entry_uuid);
  }
}

// ------------------------------------------------
// GUICommand
// ------------------------------------------------

GUICommand::GUICommand(CommandInterface *pcomInt, PWSGUICmdIF *pGUICmdIF)
  : Command(pcomInt), m_pGUICmdIF(pGUICmdIF)
{
}

GUICommand::~GUICommand()
{
  delete m_pGUICmdIF;
}

int GUICommand::Execute()
{
  m_pcomInt->CallGUICommandInterface(WN_EXECUTE, m_pGUICmdIF);
  return 0;
}

int GUICommand::Redo()
{
  m_pcomInt->CallGUICommandInterface(WN_REDO, m_pGUICmdIF);
  return 0;
}

void GUICommand::Undo()
{
  m_pcomInt->CallGUICommandInterface(WN_UNDO, m_pGUICmdIF);
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
    m_pcomInt->NotifyGUINeedsUpdating(Command::GUI_DB_PREFERENCES_CHANGED, entry_uuid);
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
    m_pcomInt->NotifyGUINeedsUpdating(Command::GUI_DB_PREFERENCES_CHANGED, entry_uuid);
  }

  m_bState = false;
}

// ------------------------------------------------
// AddEntryCommand
// ------------------------------------------------

AddEntryCommand::AddEntryCommand(CommandInterface *pcomInt, const CItemData &ci)
  : Command(pcomInt), m_ci(ci)
{
}

AddEntryCommand::~AddEntryCommand()
{
}

int AddEntryCommand::Execute()
{
  SaveState();
  TRACE(L"Command DoAddEntry\n");
  if (m_pcomInt->IsReadOnly())
    return 0;

  m_pcomInt->DoAddEntry(m_ci);
  m_pcomInt->AddChangedNodes(m_ci.GetGroup());

  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid;
    m_ci.GetUUID(entry_uuid);
    m_pcomInt->NotifyGUINeedsUpdating(Command::GUI_ADD_ENTRY, entry_uuid);
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
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// DeleteEntryCommand
// ------------------------------------------------

DeleteEntryCommand::DeleteEntryCommand(CommandInterface *pcomInt,
                                       const CItemData &ci)
  : Command(pcomInt), m_ci(ci), m_related(0)
{
  if (ci.IsAlias() || ci.IsShortcut()) {
    uuid_array_t uuid;
    ci.GetUUID(uuid);
    const ItemMap &imap = (ci.IsAlias() ? pcomInt->GetAlias2BaseMap() :
                           pcomInt->GetShortcuts2BaseMap());
    m_related.push_back(imap.find(CUUIDGen(uuid))->second);
  }
}

DeleteEntryCommand::~DeleteEntryCommand()
{
}

int DeleteEntryCommand::Execute()
{
  SaveState();
  TRACE(L"DeleteEntryCommand::Execute()\n");
  if (m_pcomInt->IsReadOnly())
    return 0;

  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid;
    m_ci.GetUUID(entry_uuid);
    m_pcomInt->NotifyGUINeedsUpdating(Command::GUI_DELETE_ENTRY, entry_uuid);
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
  if (m_ci.IsShortcut() || m_ci.IsAlias()) {
    ASSERT(m_related.size() == 1); // m_related contains the base uuid
    uuid_array_t uuid, base_uuid;
    m_ci.GetUUID(uuid);
    m_related[0].GetUUID(base_uuid);

    Command *pcmd = 
      MultiCommands::MakeAddDependentCommand(m_pcomInt, AddEntryCommand::Create(m_pcomInt, m_ci),
                                             base_uuid, uuid, m_ci.GetEntryType());
    pcmd->Execute();
    delete pcmd;
  } else { // XXX TBD - add support for alias/shortcut base undelete
    AddEntryCommand undo(m_pcomInt, m_ci);
    undo.Execute();
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
}

EditEntryCommand::~EditEntryCommand()
{
}

int EditEntryCommand::Execute()
{
  SaveState();
  TRACE(L"EditEntry::Execute\n");
  if (m_pcomInt->IsReadOnly())
    return 0;

  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid;
    m_old_ci.GetUUID(entry_uuid);
    m_pcomInt->NotifyGUINeedsUpdating(Command::GUI_DELETE_ENTRY, entry_uuid,
                                      CItemData::END, false);
  }
  m_pcomInt->DoDeleteEntry(m_old_ci);

  m_pcomInt->DoAddEntry(m_new_ci);
  m_pcomInt->AddChangedNodes(m_old_ci.GetGroup());
  m_pcomInt->AddChangedNodes(m_new_ci.GetGroup());
  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid;
    m_old_ci.GetUUID(entry_uuid);
    m_pcomInt->NotifyGUINeedsUpdating(Command::GUI_ADD_ENTRY, entry_uuid);
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
  TRACE(L"EditEntry::Undo\n");
  if (m_pcomInt->IsReadOnly())
    return;

  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid;
    m_new_ci.GetUUID(entry_uuid);
    m_pcomInt->NotifyGUINeedsUpdating(Command::GUI_DELETE_ENTRY, entry_uuid,
                                      CItemData::END, false);
  }
  m_pcomInt->DoDeleteEntry(m_new_ci);

  m_pcomInt->DoAddEntry(m_old_ci);
  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid;
    m_old_ci.GetUUID(entry_uuid);
    m_pcomInt->NotifyGUINeedsUpdating(Command::GUI_ADD_ENTRY, entry_uuid);
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
  TRACE(L"UpdateEntryCommand::Doit\n");
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

  TRACE(L"Command UpdateEntry: Field=0x%02x; Old Value=%s; NewValue=%s\n",
    m_ftype, m_old_value.c_str(), m_value.c_str());

  Doit(m_entry_uuid, m_ftype, m_value, CItemData::ES_MODIFIED);

  if (m_bNotifyGUI)
    m_pcomInt->NotifyGUINeedsUpdating(Command::GUI_REFRESH_ENTRYFIELD,
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
  TRACE(L"Command UndoUpdateEntry: Field=0x%02x; Old Value=%s;\n",
    m_ftype, m_old_value.c_str());

  Doit(m_entry_uuid, m_ftype, m_old_value, m_old_status);
  RestoreState();

  if (m_bNotifyGUI)
    m_pcomInt->NotifyGUINeedsUpdating(Command::GUI_REFRESH_ENTRYFIELD,
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

  TRACE(L"UpdatePassword::Execute\n");
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
    m_pcomInt->NotifyGUINeedsUpdating(Command::GUI_REFRESH_ENTRYPASSWORD,
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
  TRACE(L"UpdatePasswordCommand::Undo\n");
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
    m_pcomInt->NotifyGUINeedsUpdating(Command::GUI_REFRESH_ENTRYPASSWORD,
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
  memcpy((void *)m_base_uuid, (void *)base_uuid, sizeof(uuid_array_t));
  memcpy((void *)m_entry_uuid, (void *)entry_uuid, sizeof(uuid_array_t));
}

int AddDependentEntryCommand::Execute()
{
  TRACE(L"AddDependentEntryCommand::Execute\n");
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
  TRACE(L"AddDependentEntryCommand::Undo\n");
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
                                                       UUIDList &dependentslist,
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
  TRACE(L"AddDependentEntriesCommand::Execute\n");
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
  TRACE(L"AddDependentEntriesCommand::Undo\n");
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
  memcpy((void *)m_base_uuid, (void *)base_uuid, sizeof(uuid_array_t));
  memcpy((void *)m_entry_uuid, (void *)entry_uuid, sizeof(uuid_array_t));
}

int RemoveDependentEntryCommand::Execute()
{
  TRACE(L"RemoveDependentEntryCommand::Execute\n");
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
  TRACE(L"RemoveDependentEntryCommand::Undo\n");
  if (m_pcomInt->IsReadOnly())
    return;

  m_pcomInt->DoAddDependentEntry(m_base_uuid, m_entry_uuid, m_type);
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// RemoveAllDependentEntriesCommand
// ------------------------------------------------

RemoveAllDependentEntriesCommand::RemoveAllDependentEntriesCommand(CommandInterface *pcomInt,
                                                                   const uuid_array_t &base_uuid,
                                                                   const CItemData::EntryType type)
  : Command(pcomInt), m_type(type)
{
  memcpy((void *)m_base_uuid, (void *)base_uuid, sizeof(uuid_array_t));
}

int RemoveAllDependentEntriesCommand::Execute()
{
  TRACE(L"RemoveAllDependentEntriesCommand::Execute\n");
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

  m_pcomInt->DoRemoveAllDependentEntries(m_base_uuid, m_type);
  m_bState = true;
  return 0;
}

int RemoveAllDependentEntriesCommand::Redo()
{
  return Execute();
}

void RemoveAllDependentEntriesCommand::Undo()
{
  TRACE(L"RemoveAllDependentEntriesCommand::Undo\n");
  if (m_pcomInt->IsReadOnly())
    return;

  ItemListIter iter = m_pcomInt->Find(m_base_uuid);
  if (iter != m_pcomInt->GetEntryEndIter()) {
    iter->second.SetEntryType(m_type == CItemData::ET_ALIAS ?
                              CItemData::ET_ALIASBASE : CItemData::ET_SHORTCUTBASE);
  }
  if (m_type ==  CItemData::ET_ALIAS) {
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
// MoveDependentEntriesCommand
// ------------------------------------------------

MoveDependentEntriesCommand::MoveDependentEntriesCommand(CommandInterface *pcomInt,
                                                         const uuid_array_t &from_baseuuid,
                                                         const uuid_array_t &to_baseuuid,
                                                         const CItemData::EntryType type)
  : Command(pcomInt), m_type(type)
{
  memcpy((void *)m_from_baseuuid, (void *)from_baseuuid, sizeof(uuid_array_t));
  memcpy((void *)m_to_baseuuid, (void *)to_baseuuid, sizeof(uuid_array_t));
}

int MoveDependentEntriesCommand::Execute()
{
  TRACE(L"MoveDependentEntriesCommand::Execute\n");
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
  TRACE(L"MoveDependentEntriesCommand::Undo\n");
  if (m_pcomInt->IsReadOnly())
    return;

  m_pcomInt->DoMoveDependentEntries(m_to_baseuuid, m_from_baseuuid, m_type);
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// ResetAllAliasPasswordsCommand
// ------------------------------------------------

ResetAllAliasPasswordsCommand::ResetAllAliasPasswordsCommand(CommandInterface *pcomInt,
                                                             const uuid_array_t &base_uuid)
  : Command(pcomInt)
{
  memcpy((void *)m_base_uuid, (void *)base_uuid, sizeof(uuid_array_t));
}

int ResetAllAliasPasswordsCommand::Execute()
{
  TRACE(L"ResetAllAliasPasswordsCommand::Execute\n");
  SaveState();
  m_saved_base2aliases_mmap = m_pcomInt->GetBase2AliasesMmap();
  if (m_pcomInt->IsReadOnly())
    return 0;

  m_pcomInt->DoResetAllAliasPasswords(m_base_uuid, m_vSavedAliases);
  m_bState = true;
  return 0;
}

int ResetAllAliasPasswordsCommand::Redo()
{
  return Execute();
}

void ResetAllAliasPasswordsCommand::Undo()
{
  TRACE(L"ResetAllAliasPasswordsCommand::Undo\n");
  if (m_pcomInt->IsReadOnly())
    return;

  m_pcomInt->UndoResetAllAliasPasswords(m_base_uuid, m_vSavedAliases);
  m_pcomInt->SetBase2AliasesMmap(m_saved_base2aliases_mmap);
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
  TRACE(L"UpdatePasswordHistoryCommand::Execute\n");
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
  TRACE(L"UpdatePasswordHistoryCommand::Undo\n");
  if (m_pcomInt->IsReadOnly())
    return;

  m_pcomInt->UndoUpdatePasswordHistory(m_mapSavedHistory);
  RestoreState();
  m_bState = false;
}
