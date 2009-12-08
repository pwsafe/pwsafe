/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// Command.cpp
//-----------------------------------------------------------------------------

#include "PWScore.h"
#include "Command.h"
#include "PWSprefs.h"

// ------------------------------------------------
// Base class: Command
// ------------------------------------------------

Command::Command(PWScore *pcore)
:  m_pcore(pcore), m_bNotifyGUI(true), m_RC(0), m_When(WN_INVALID),
   m_bState(false)
{
}

Command::~Command()
{
}

// Save/restore state
void Command::SaveState()
{
  m_bSaveDBChanged = m_pcore->IsChanged();
  m_saved_vnodes_modified = m_pcore->m_vnodes_modified;
}

void Command::RestoreState()
{
  m_pcore->SetDBChanged(m_bSaveDBChanged);
  m_pcore->m_vnodes_modified = m_saved_vnodes_modified;
}

void Command::SaveDependentsState(const Command::DependentsType itype)
{
  switch (itype) {
    case DT_BASE2ALIASES_MMAP:
      m_saved_base2aliases_mmap = m_pcore->m_base2aliases_mmap;
      break;
    case DT_BASE2SHORTCUTS_MMAP:
      m_saved_base2shortcuts_mmap = m_pcore->m_base2shortcuts_mmap;
      break;
    case DT_ALIAS2BASE_MAP:
      m_saved_alias2base_map = m_pcore->m_alias2base_map;
      break;
    case DT_SHORTCUT2BASE_MAP:
      m_saved_shortcut2base_map = m_pcore->m_shortcut2base_map;
      break;
    default:
      ASSERT(0);
  }
}

void Command::RestoreDependentsState(const Command::DependentsType itype)
{
  switch (itype) {
    case DT_BASE2ALIASES_MMAP:
      m_pcore->m_base2aliases_mmap = m_saved_base2aliases_mmap;
      break;
    case DT_BASE2SHORTCUTS_MMAP:
      m_pcore->m_base2shortcuts_mmap = m_saved_base2shortcuts_mmap;
      break;
    case DT_ALIAS2BASE_MAP:
      m_pcore->m_alias2base_map = m_saved_alias2base_map;
      break;
    case DT_SHORTCUT2BASE_MAP:
      m_pcore->m_shortcut2base_map = m_saved_shortcut2base_map;
      break;
    default:
      ASSERT(0);
  }
}

void Command::DoUpdateGUI(const Command::ExecuteFn When, const Command::GUI_Action ga)
{
  if (m_When == When || When == Command::WN_ALL) {
    uuid_array_t entry_uuid = {'0'}; // Not needed but NotifyGUINeedsUpdating needs it!
    m_pcore->NotifyGUINeedsUpdating(ga, entry_uuid);
  }
}

void Command::DoUpdateDBPrefs(const StringX &sxNewDBPrefs)
{
  if (m_pcore->IsReadOnly())
    return;

  PWSprefs::GetInstance()->Load(sxNewDBPrefs);
  m_pcore->SetDBPrefsChanged(m_pcore->HaveHeaderPreferencesChanged(sxNewDBPrefs));
}

void Command::UndoUpdateDBPrefs(const StringX &sxOldDBPrefs, const bool bOldState)
{
  if (m_pcore->IsReadOnly())
    return;

  PWSprefs::GetInstance()->Load(sxOldDBPrefs);
  m_pcore->SetDBPrefsChanged(bOldState);
}

// Proxy routines to update pcore's data

void Command::DoAddEntry(CItemData &ci)
{
  TRACE(L"Command DoAddEntry\n");
  if (m_pcore->IsReadOnly())
    return;

  m_pcore->DoAddEntry(ci);
  m_pcore->AddChangedNodes(ci.GetGroup());

  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid;
    ci.GetUUID(entry_uuid);
    m_pcore->NotifyGUINeedsUpdating(Command::GUI_ADD_ENTRY, entry_uuid);
  }
}

void Command::DoDeleteEntry(CItemData &ci)
{
  TRACE(L"Command DoDeleteEntry\n");
  if (m_pcore->IsReadOnly())
    return;

  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid;
    ci.GetUUID(entry_uuid);
    m_pcore->NotifyGUINeedsUpdating(Command::GUI_DELETE_ENTRY, entry_uuid);
  }

  m_pcore->DoDeleteEntry(ci);
  m_pcore->AddChangedNodes(ci.GetGroup());
}

void Command::DoEditEntry(CItemData &old_ci, CItemData &new_ci)
{
  TRACE(L"Command DoEditEntry\n");
  if (m_pcore->IsReadOnly())
    return;

  m_pcore->DoDeleteEntry(old_ci);
  m_pcore->DoAddEntry(new_ci);
  m_pcore->AddChangedNodes(old_ci.GetGroup());
  m_pcore->AddChangedNodes(new_ci.GetGroup());

  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid;
    old_ci.GetUUID(entry_uuid);
    m_pcore->NotifyGUINeedsUpdating(Command::GUI_DELETE_ENTRY, entry_uuid);
    m_pcore->NotifyGUINeedsUpdating(Command::GUI_ADD_ENTRY, entry_uuid);
  }
}

void Command::UndoEditEntry(CItemData &old_ci, CItemData &new_ci)
{
  TRACE(L"Command UndoEditEntry\n");
  if (m_pcore->IsReadOnly())
    return;

  m_pcore->DoDeleteEntry(new_ci);
  m_pcore->DoAddEntry(old_ci);

  if (m_bNotifyGUI) {
    uuid_array_t entry_uuid;
    old_ci.GetUUID(entry_uuid);
    m_pcore->NotifyGUINeedsUpdating(Command::GUI_DELETE_ENTRY, entry_uuid);
    m_pcore->NotifyGUINeedsUpdating(Command::GUI_ADD_ENTRY, entry_uuid);
  }
}

void Command::DoUpdateEntry(const uuid_array_t &entry_uuid,
                            const CItemData::FieldType &ftype,
                            const StringX &value,
                            const CItemData::EntryStatus es)
{
  TRACE(L"Command DoUpdateEntry\n");
  if (m_pcore->IsReadOnly())
    return;

  ItemListIter pos = m_pcore->m_pwlist.find(entry_uuid);
  if (pos != m_pcore->m_pwlist.end()) {
    pos->second.SetFieldValue(ftype, value);
    pos->second.SetStatus(es);
    m_pcore->m_bDBChanged = true;
    m_pcore->AddChangedNodes(pos->second.GetGroup());
  }
}

void Command::DoUpdatePassword(const uuid_array_t &entry_uuid,
                               const StringX sxNewPassword)
{
  TRACE(L"Command DoUpdatePassword\n");
  if (m_pcore->IsReadOnly())
    return;

  ItemListIter pos = m_pcore->m_pwlist.find(entry_uuid);
  if (pos != m_pcore->m_pwlist.end()) {
    pos->second.UpdatePassword(sxNewPassword);
    pos->second.SetStatus(CItemData::ES_MODIFIED);
    m_pcore->m_bDBChanged = true;
    m_pcore->AddChangedNodes(pos->second.GetGroup());
  }
}

void Command::UndoUpdatePassword(const uuid_array_t &entry_uuid,
                                 const StringX &sxOldPassword,
                                 const StringX &sxOldPWHistory,
                                 const CItemData::EntryStatus es)
{
  TRACE(L"Command UndoUpdatePassword\n");
  if (m_pcore->IsReadOnly())
    return;

  ItemListIter pos = m_pcore->m_pwlist.find(entry_uuid);
  if (pos != m_pcore->m_pwlist.end()) {
    pos->second.SetPassword(sxOldPassword);
    pos->second.SetPWHistory(sxOldPWHistory);
    pos->second.SetStatus(es);
  }
}

void Command::DoAddDependentEntry(const uuid_array_t &base_uuid,
                                  const uuid_array_t &entry_uuid,
                                  const CItemData::EntryType type)
{
  TRACE(L"Command DoAddDependentEntry\n");
  if (m_pcore->IsReadOnly())
    return;

  m_pcore->DoAddDependentEntry(base_uuid, entry_uuid, type);
}

int Command::DoAddDependentEntries(UUIDList &dependentslist, CReport *rpt,
                                   const CItemData::EntryType type,
                                   const int &iVia,
                                   ItemList *pmapDeletedItems,
                                   SaveTypePWMap *pmapSaveTypePW)
{
  TRACE(L"Command DoAddDependentEntries\n");
  if (m_pcore->IsReadOnly())
    return 0;

  return m_pcore->DoAddDependentEntries(dependentslist, rpt, type, iVia,
                                        pmapDeletedItems, pmapSaveTypePW);
}

void Command::UndoAddDependentEntries(ItemList *pmapDeletedItems,
                                      SaveTypePWMap *pmapSaveTypePW)
{
  TRACE(L"Command UndoAddDependentEntries\n");
  if (m_pcore->IsReadOnly())
    return;

  m_pcore->UndoAddDependentEntries(pmapDeletedItems, pmapSaveTypePW);
}

void Command::DoRemoveDependentEntry(const uuid_array_t &base_uuid,
                                     const uuid_array_t &entry_uuid,
                                     const CItemData::EntryType type)
{
  TRACE(L"Command DoRemoveDependentEntry\n");
  if (m_pcore->IsReadOnly())
    return;

  m_pcore->DoRemoveDependentEntry(base_uuid, entry_uuid, type);
}

void Command::DoRemoveAllDependentEntries(const uuid_array_t &base_uuid,
                                          const CItemData::EntryType type)
{
  TRACE(L"Command DoRemoveAllDependentEntries\n");
  if (m_pcore->IsReadOnly())
    return;

  m_pcore->DoRemoveAllDependentEntries(base_uuid, type);
}

void Command::UndoRemoveAllDependentEntries(const uuid_array_t &base_uuid,
                                            const CItemData::EntryType type)
{
  TRACE(L"Command UndoRemoveAllDependentEntries\n");
  if (m_pcore->IsReadOnly())
    return;

  ItemListIter iter = m_pcore->m_pwlist.find(base_uuid);
  if (iter != m_pcore->m_pwlist.end()) {
    iter->second.SetEntryType(type == CItemData::ET_ALIAS ?
                                           CItemData::ET_ALIASBASE : CItemData::ET_SHORTCUTBASE);
  }
}

void Command::DoMoveDependentEntries(const uuid_array_t &from_baseuuid,
                                     const uuid_array_t &to_baseuuid,
                                     const CItemData::EntryType type)
{
  TRACE(L"Command MoveDependentEntries\n");
  if (m_pcore->IsReadOnly())
    return;

  m_pcore->DoMoveDependentEntries(from_baseuuid, to_baseuuid, type);
}

void Command::DoResetAllAliasPasswords(const uuid_array_t &base_uuid,
                                       std::vector<CUUIDGen> &vSavedAliases)
{
  TRACE(L"Command DoResetAllAliasPasswords\n");
  if (m_pcore->IsReadOnly())
    return;

  m_pcore->DoResetAllAliasPasswords(base_uuid, vSavedAliases);
}

void Command::UndoResetAllAliasPasswords(const uuid_array_t &base_uuid,
                                         std::vector<CUUIDGen> &vSavedAliases)
{
  TRACE(L"Command UndoResetAllAliasPasswords\n");
  if (m_pcore->IsReadOnly())
    return;

  m_pcore->UndoResetAllAliasPasswords(base_uuid, vSavedAliases);
}

int Command::DoUpdatePasswordHistory(const int iAction, const int new_default_max,
                                     SavePWHistoryMap &mapSavedHistory)
{
  TRACE(L"Command DoUpdatePasswordHistory\n");
  if (m_pcore->IsReadOnly())
    return 0;

  return m_pcore->DoUpdatePasswordHistory(iAction, new_default_max, mapSavedHistory);
}

void Command::UndoUpdatePasswordHistory(SavePWHistoryMap &mapSavedHistory)
{
  TRACE(L"Command UndoUpdatePasswordHistory\n");
  if (m_pcore->IsReadOnly())
    return;

  m_pcore->UndoUpdatePasswordHistory(mapSavedHistory);
}

// ------------------------------------------------
// MultiCommands
// ------------------------------------------------

MultiCommands::MultiCommands(PWScore *pcore)
: Command(pcore)
{
  m_pcmds = new std::vector<Command *>;
}

MultiCommands::~MultiCommands()
{
  std::vector<Command *>::iterator cmd_Iter;

  for (cmd_Iter = m_pcmds->begin(); cmd_Iter != m_pcmds->end(); cmd_Iter++) {
    delete (*cmd_Iter);
  }
  delete m_pcmds;
}

int MultiCommands::Execute()
{
  std::vector<Command *>::iterator cmd_Iter;

  TRACE(L"Multicommands Execute\n");
  for (cmd_Iter = m_pcmds->begin(); cmd_Iter != m_pcmds->end(); cmd_Iter++) {
    int rc = (*cmd_Iter)->Execute();
    m_RCs.push_back(rc);
  }
  m_bState = true;
  return 0;
}

void MultiCommands::Undo()
{
  std::vector<Command *>::reverse_iterator cmd_rIter;

  TRACE(L"Multicommands Undo\n");
  for (cmd_rIter = m_pcmds->rbegin(); cmd_rIter != m_pcmds->rend(); cmd_rIter++) {
    (*cmd_rIter)->Undo();
  }
  m_bState = false;
}

void MultiCommands::Add(Command *c)
{
  TRACE(L"Multicommands Add\n");
  m_pcmds->push_back(c);
}

bool MultiCommands::Remove(Command *c)
{
  std::vector<Command *>::iterator cmd_Iter;

  TRACE(L"Multicommands Remove\n");
  cmd_Iter = find(m_pcmds->begin(), m_pcmds->end(), c);
  if (cmd_Iter != m_pcmds->end()) {
    m_pcmds->erase(cmd_Iter);
    return true;
  } else
    return false;
}

bool MultiCommands::Remove()
{
  TRACE(L"Multicommands Remove\n");
  if (m_pcmds->size() > 0) {
    m_pcmds->pop_back();
    return true;
  } else
    return false;
}

bool MultiCommands::GetRC(Command *c, int &rc)
{
  std::vector<Command *>::iterator cmd_Iter;

  TRACE(L"Multicommands GetRC\n");
  cmd_Iter = find(m_pcmds->begin(), m_pcmds->end(), c);
  if (cmd_Iter != m_pcmds->end()) {
    rc = m_RCs[cmd_Iter - m_pcmds->begin()];
    return true;
  } else {
    rc = 0;
    return false;
  }
}

bool MultiCommands::GetRC(const size_t ncmd, int &rc)
{
  if (ncmd <= 0 || ncmd > m_RCs.size()) {
    rc = 0;
    return false;
  } else {
    rc = m_RCs[ncmd - 1];
    return true;
  }
}

// ------------------------------------------------
// UpdateGUICommand
// ------------------------------------------------

UpdateGUICommand::UpdateGUICommand(PWScore *pcore, const Command::ExecuteFn When,
                                   const Command::GUI_Action ga)
  : Command(pcore), m_ga(ga)
{
  m_When = When;
}

int UpdateGUICommand::Execute()
{
  if (m_When == WN_EXECUTE)
    Command::DoUpdateGUI(m_When, m_ga);
  return 0;
}

void UpdateGUICommand::Undo()
{
  if (m_When == WN_UNDO)
    Command::DoUpdateGUI(m_When, m_ga);
}

// ------------------------------------------------
// DBPrefsCommand
// ------------------------------------------------

DBPrefsCommand::DBPrefsCommand(PWScore *pcore, StringX &sxDBPrefs)
  : Command(pcore), m_sxNewDBPrefs(sxDBPrefs)
{
  m_bOldState = PWSprefs::GetInstance()->IsDBprefsChanged();
  m_sxOldDBPrefs = PWSprefs::GetInstance()->Store();
}

int DBPrefsCommand::Execute()
{
  Command::DoUpdateDBPrefs(m_sxNewDBPrefs);
  m_bState = true;
  return 0;
}

void DBPrefsCommand::Undo()
{
  Command::UndoUpdateDBPrefs(m_sxOldDBPrefs, m_bOldState);
  m_bState = false;
}

// ------------------------------------------------
// AddEntryCommand
// ------------------------------------------------

AddEntryCommand::AddEntryCommand(PWScore *pcore, CItemData &ci)
  : Command(pcore), m_ci(ci)
{
}

AddEntryCommand::~AddEntryCommand()
{
  if (!m_bState && m_ci.GetDisplayInfo() != NULL)
    delete m_ci.GetDisplayInfo();
}

int AddEntryCommand::Execute()
{
  SaveState();
  Command::DoAddEntry(m_ci);
  m_bState = true;
  return 0;
}

void AddEntryCommand::Undo()
{
  Command::DoDeleteEntry(m_ci);
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// DeleteEntryCommand
// ------------------------------------------------

DeleteEntryCommand::DeleteEntryCommand(PWScore *pcore, CItemData &ci)
  : Command(pcore), m_ci(ci)
{
}

DeleteEntryCommand::~DeleteEntryCommand()
{
  if (m_bState && m_ci.GetDisplayInfo() != NULL)
    delete m_ci.GetDisplayInfo();
}

int DeleteEntryCommand::Execute()
{
  SaveState();
  Command::DoDeleteEntry(m_ci);
  m_bState = true;
  return 0;
}

void DeleteEntryCommand::Undo()
{
  Command::DoAddEntry(m_ci);
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// EditEntryCommand
// ------------------------------------------------

EditEntryCommand::EditEntryCommand(PWScore *pcore,
                                   CItemData &old_ci,
                                   CItemData &new_ci)
  : Command(pcore), m_old_ci(old_ci), m_new_ci(new_ci)
{
}

EditEntryCommand::~EditEntryCommand()
{
  if (m_bState) {
    if (m_old_ci.GetDisplayInfo() != NULL)
      delete m_old_ci.GetDisplayInfo();
  } else {
    if (m_new_ci.GetDisplayInfo() != NULL)
      delete m_new_ci.GetDisplayInfo();
  }
}

int EditEntryCommand::Execute()
{
  SaveState();
  Command::DoEditEntry(m_old_ci, m_new_ci);
  m_bState = true;
  return 0;
}

void EditEntryCommand::Undo()
{
  Command::UndoEditEntry(m_old_ci, m_new_ci);
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// UpdateEntryCommand
// ------------------------------------------------

UpdateEntryCommand::UpdateEntryCommand(PWScore *pcore, CItemData &ci,
                                       const CItemData::FieldType &ftype,
                                       const StringX &value)
  : Command(pcore), m_ftype(ftype), m_value(value)
{
  ci.GetUUID(m_entry_uuid);
  m_old_status = ci.GetStatus();
  m_old_value = ci.GetFieldValue(m_ftype);
}

int UpdateEntryCommand::Execute()
{
  SaveState();

  TRACE(L"Command UpdateEntry: Field=0x%02x; Old Value=%s; NewValue=%s\n",
    m_ftype, m_old_value.c_str(), m_value.c_str());

  Command::DoUpdateEntry(m_entry_uuid, m_ftype, m_value, CItemData::ES_MODIFIED);

  if (m_bNotifyGUI)
    m_pcore->NotifyGUINeedsUpdating(Command::GUI_REFRESH_ENTRYFIELD, m_entry_uuid,
                                    (LPARAM)m_ftype);

  m_bState = true;
  return 0;
}

void UpdateEntryCommand::Undo()
{
  TRACE(L"Command UndoUpdateEntry: Field=0x%02x; Old Value=%s;\n",
    m_ftype, m_old_value.c_str());

  Command::DoUpdateEntry(m_entry_uuid, m_ftype, m_old_value, m_old_status);

  RestoreState();

  if (m_bNotifyGUI)
    m_pcore->NotifyGUINeedsUpdating(Command::GUI_REFRESH_ENTRYFIELD, m_entry_uuid,
                                    (LPARAM)m_ftype);
  m_bState = false;
}

// ------------------------------------------------
// UpdatePasswordCommand
// ------------------------------------------------

UpdatePasswordCommand::UpdatePasswordCommand(PWScore *pcore,
                                             CItemData &ci,
                                             const StringX sxNewPassword)
  : Command(pcore), m_sxNewPassword(sxNewPassword)
{
  ci.GetUUID(m_entry_uuid);
  m_old_status = ci.GetStatus();
  m_sxOldPassword = ci.GetPassword();
  m_sxOldPWHistory = ci.GetPWHistory();
}

int UpdatePasswordCommand::Execute()
{
  SaveState();

  Command::DoUpdatePassword(m_entry_uuid, m_sxNewPassword);

  if (m_bNotifyGUI)
    m_pcore->NotifyGUINeedsUpdating(Command::GUI_REFRESH_ENTRYPASSWORD, m_entry_uuid);

  m_bState = true;
  return 0;
}

void UpdatePasswordCommand::Undo()
{
  Command::UndoUpdatePassword(m_entry_uuid, m_sxOldPassword,
                              m_sxOldPWHistory, m_old_status);

  RestoreState();

  if (m_bNotifyGUI)
    m_pcore->NotifyGUINeedsUpdating(Command::GUI_REFRESH_ENTRYPASSWORD, m_entry_uuid);

  m_bState = false;
}

// ------------------------------------------------
// AddDependentEntryCommand
// ------------------------------------------------

AddDependentEntryCommand::AddDependentEntryCommand(PWScore *pcore,
                                                   const uuid_array_t &base_uuid,
                                                   const uuid_array_t &entry_uuid,
                                                   const CItemData::EntryType type)
  : Command(pcore), m_type(type)
{
  memcpy((void *)m_base_uuid, (void *)base_uuid, sizeof(uuid_array_t));
  memcpy((void *)m_entry_uuid, (void *)entry_uuid, sizeof(uuid_array_t));
}

int AddDependentEntryCommand::Execute()
{
  SaveState();
  Command::DoAddDependentEntry(m_base_uuid, m_entry_uuid, m_type);
  m_bState = true;
  return 0;
}

void AddDependentEntryCommand::Undo()
{
  Command::DoRemoveDependentEntry(m_base_uuid, m_entry_uuid, m_type);
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// AddDependentEntriesCommand
// ------------------------------------------------

AddDependentEntriesCommand::AddDependentEntriesCommand(PWScore *pcore,
                                                       UUIDList &dependentslist, CReport *rpt,
                                                       const CItemData::EntryType type,
                                                       const int &iVia)
  : Command(pcore), m_dependentslist(dependentslist), m_rpt(rpt),
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
  SaveState();
  SaveDependentsState(m_type == CItemData::ET_ALIAS ?
                                     DT_BASE2ALIASES_MMAP : DT_BASE2SHORTCUTS_MMAP);
  SaveDependentsState(m_type == CItemData::ET_ALIAS ?
                                     DT_ALIAS2BASE_MAP : DT_SHORTCUT2BASE_MAP);
  int rc = Command::DoAddDependentEntries(m_dependentslist, m_rpt, m_type, m_iVia,
                                          m_pmapDeletedItems, m_pmapSaveStatus);
  m_bState = true;
  return rc;
}

void AddDependentEntriesCommand::Undo()
{
  Command::UndoAddDependentEntries(m_pmapDeletedItems, m_pmapSaveStatus);
  RestoreDependentsState(m_type == CItemData::ET_ALIAS ?
                                        DT_BASE2ALIASES_MMAP : DT_BASE2SHORTCUTS_MMAP);
  RestoreDependentsState(m_type == CItemData::ET_ALIAS ?
                                        DT_ALIAS2BASE_MAP : DT_SHORTCUT2BASE_MAP);
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// RemoveDependentEntryCommand
// ------------------------------------------------

RemoveDependentEntryCommand::RemoveDependentEntryCommand(PWScore *pcore,
                                                         const uuid_array_t &base_uuid,
                                                         const uuid_array_t &entry_uuid,
                                                         const CItemData::EntryType type)
  : Command(pcore), m_type(type)
{
  memcpy((void *)m_base_uuid, (void *)base_uuid, sizeof(uuid_array_t));
  memcpy((void *)m_entry_uuid, (void *)entry_uuid, sizeof(uuid_array_t));
}

int RemoveDependentEntryCommand::Execute()
{
  SaveState();
  Command::DoRemoveDependentEntry(m_base_uuid, m_entry_uuid, m_type);
  m_bState = true;
  return 0;
}

void RemoveDependentEntryCommand::Undo()
{
  Command::DoAddDependentEntry(m_base_uuid, m_entry_uuid, m_type);
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// RemoveAllDependentEntriesCommand
// ------------------------------------------------

RemoveAllDependentEntriesCommand::RemoveAllDependentEntriesCommand(PWScore *pcore,
                                                                   const uuid_array_t &base_uuid,
                                                                   const CItemData::EntryType type)
  : Command(pcore), m_type(type)
{
  memcpy((void *)m_base_uuid, (void *)base_uuid, sizeof(uuid_array_t));
}

int RemoveAllDependentEntriesCommand::Execute()
{
  SaveState();
  SaveDependentsState(m_type == CItemData::ET_ALIAS ?
                                     DT_BASE2ALIASES_MMAP : DT_BASE2SHORTCUTS_MMAP);
  SaveDependentsState(m_type == CItemData::ET_ALIAS ?
                                     DT_ALIAS2BASE_MAP : DT_SHORTCUT2BASE_MAP);
  Command::DoRemoveAllDependentEntries(m_base_uuid, m_type);
  m_bState = true;
  return 0;
}

void RemoveAllDependentEntriesCommand::Undo()
{
  Command::UndoRemoveAllDependentEntries(m_base_uuid, m_type);
  RestoreDependentsState(m_type == CItemData::ET_ALIAS ?
                                        DT_BASE2ALIASES_MMAP : DT_BASE2SHORTCUTS_MMAP);
  RestoreDependentsState(m_type == CItemData::ET_ALIAS ?
                                        DT_ALIAS2BASE_MAP : DT_SHORTCUT2BASE_MAP);
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// MoveDependentEntriesCommand
// ------------------------------------------------

MoveDependentEntriesCommand::MoveDependentEntriesCommand(PWScore *pcore,
                                                         const uuid_array_t &from_baseuuid,
                                                         const uuid_array_t &to_baseuuid,
                                                         const CItemData::EntryType type)
  : Command(pcore), m_type(type)
{
  memcpy((void *)m_from_baseuuid, (void *)from_baseuuid, sizeof(uuid_array_t));
  memcpy((void *)m_to_baseuuid, (void *)to_baseuuid, sizeof(uuid_array_t));
}

int MoveDependentEntriesCommand::Execute()
{
  SaveState();
  Command::DoMoveDependentEntries(m_from_baseuuid, m_to_baseuuid, m_type);
  m_bState = true;
  return 0;
}

void MoveDependentEntriesCommand::Undo()
{
  Command::DoMoveDependentEntries(m_to_baseuuid, m_from_baseuuid, m_type);
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// ResetAllAliasPasswordsCommand
// ------------------------------------------------

ResetAllAliasPasswordsCommand::ResetAllAliasPasswordsCommand(PWScore *pcore,
                                                             const uuid_array_t &base_uuid)
  : Command(pcore)
{
  memcpy((void *)m_base_uuid, (void *)base_uuid, sizeof(uuid_array_t));
}

int ResetAllAliasPasswordsCommand::Execute()
{
  SaveState();
  SaveDependentsState(DT_BASE2ALIASES_MMAP);
  Command::DoResetAllAliasPasswords(m_base_uuid, m_vSavedAliases);
  m_bState = true;
  return 0;
}

void ResetAllAliasPasswordsCommand::Undo()
{
  Command::UndoResetAllAliasPasswords(m_base_uuid, m_vSavedAliases);
  RestoreDependentsState(DT_BASE2ALIASES_MMAP);
  RestoreState();
  m_bState = false;
}

// ------------------------------------------------
// UpdatePasswordHistoryCommand
// ------------------------------------------------

UpdatePasswordHistoryCommand::UpdatePasswordHistoryCommand(PWScore *pcore,
                                                           const int iAction,
                                                           const int new_default_max)
 : Command(pcore), m_iAction(iAction), m_new_default_max(new_default_max)
{}

int UpdatePasswordHistoryCommand::Execute()
{
  SaveState();
  int rc = Command::DoUpdatePasswordHistory(m_iAction, m_new_default_max,
                                            m_mapSavedHistory);
  m_bState = true;
  return rc;
}

void UpdatePasswordHistoryCommand::Undo()
{
  Command::UndoUpdatePasswordHistory(m_mapSavedHistory);
  RestoreState();
  m_bState = false;
}
