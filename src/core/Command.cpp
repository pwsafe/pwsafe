/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
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

using pws_os::CUUID;

Command::Command(CommandInterface *pcomInt)
:  m_pcomInt(pcomInt), m_bNotifyGUI(true), m_RC(0), m_CommandDBChange(NONE)
{
  m_PreCommand.Clear();
  m_PostCommand.Clear();
  m_Command.Clear();
}

Command::~Command()
{
}

// The following should only be called from PWScore Execute/Undo/Redo
void Command::SaveChangedState(StateType st, st_DBChangeStatus &stDBCS)
{
  switch (st) {
    case COMMANDACTION:
      // Set what the command changes
      m_Command = stDBCS;
      break;
    case PREEXECUTE:
      // Save current pre-command execute state
      m_PreCommand = stDBCS;
      break;
    case POSTEXECUTE:
      // Here we generate new state from pre-execute state and command action states
      // Effectively post-execute state = pre-execute state + command action state
      // and, unlike the other calls, return the result
      stDBCS = m_PostCommand = (m_PreCommand + m_Command);
      break;
  }
}

// The following should only be called from PWScore Execute/Undo/Redo
void Command::RestoreChangedState(st_DBChangeStatus &stDBCS)
{
  stDBCS = m_PreCommand;
}

// ------------------------------------------------
// MultiCommands
// ------------------------------------------------

MultiCommands::MultiCommands(CommandInterface *pcomInt)
  : Command(pcomInt)
{
  m_CommandDBChange = MULTICOMMAND;
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

  for (cmd_Iter = m_vpcmds.begin(); cmd_Iter != m_vpcmds.end(); cmd_Iter++) {
    int rc(-1);
    if (*cmd_Iter != NULL) {
      rc = (*cmd_Iter)->Execute();
    }
    m_vRCs.push_back(rc);
  }

  return 0;
}

void MultiCommands::Undo()
{
  std::vector<Command *>::reverse_iterator cmd_rIter;

  for (cmd_rIter = m_vpcmds.rbegin(); cmd_rIter != m_vpcmds.rend(); cmd_rIter++) {
    if (*cmd_rIter != NULL)
      (*cmd_rIter)->Undo();
  }
}

void MultiCommands::Add(Command *pcmd)
{
  ASSERT(pcmd != NULL);
  m_vpcmds.push_back(pcmd);
}

void MultiCommands::Insert(Command *pcmd)
{
  // VERY INEFFICIENT - use sparingly to add commands at the front of the
  // multi-command vector
  ASSERT(pcmd != NULL);
  m_vpcmds.insert(m_vpcmds.begin(), pcmd);
}

bool MultiCommands::Remove(Command *pcmd)
{
  ASSERT(pcmd != NULL);
  std::vector<Command *>::iterator cmd_Iter;

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

// ------------------------------------------------
// UpdateGUICommand
// ------------------------------------------------

UpdateGUICommand::UpdateGUICommand(CommandInterface *pcomInt,
                                   ExecuteFn When, GUI_Action ga,
                                   const pws_os::CUUID &entryUUID)
 : Command(pcomInt), m_When(When), m_ga(ga), m_entryUUID(entryUUID)
{
}

int UpdateGUICommand::Execute()
{
  if (m_When == WN_EXECUTE || m_When == WN_EXECUTE_REDO ||
      m_When == WN_REDO || m_When == WN_ALL) {
    m_pcomInt->NotifyGUINeedsUpdating(m_ga, m_entryUUID);
  }
  return 0;
}

void UpdateGUICommand::Undo()
{
  if (m_When == WN_UNDO || m_When == WN_ALL) {
    m_pcomInt->NotifyGUINeedsUpdating(m_ga, m_entryUUID);
  }
}

// ------------------------------------------------
// DBPrefsCommand
// ------------------------------------------------

DBPrefsCommand::DBPrefsCommand(CommandInterface *pcomInt, StringX &sxDBPrefs)
  : Command(pcomInt), m_sxNewDBPrefs(sxDBPrefs)
{
  m_sxOldDBPrefs = PWSprefs::GetInstance()->Store();
}

int DBPrefsCommand::Execute()
{
  PWSprefs::GetInstance()->Load(m_sxNewDBPrefs);
  if (!m_pcomInt->IsReadOnly())
    m_pcomInt->SetDBPrefsChanged(m_pcomInt->HaveHeaderPreferencesChanged(m_sxNewDBPrefs)); //?????

  if (m_bNotifyGUI) {
    m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_DB_PREFERENCES_CHANGED,
                                      CUUID::NullUUID());
  }
  m_CommandDBChange = DBPREFS;
  return 0;
}

void DBPrefsCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DBPREFS) {
    PWSprefs::GetInstance()->Load(m_sxOldDBPrefs);

    if (m_bNotifyGUI) {
      m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_DB_PREFERENCES_CHANGED,
        CUUID::NullUUID());
    }
  }
}

// ------------------------------------------------
// DBPolicyNamesCommand
// ------------------------------------------------

DBPolicyNamesCommand::DBPolicyNamesCommand(CommandInterface *pcomInt,
                                           PSWDPolicyMap &MapPSWDPLC,
                                           Function function)
  : Command(pcomInt), m_NewMapPSWDPLC(MapPSWDPLC), m_function(function),
  m_bSingleAdd(false)
{
  m_OldMapPSWDPLC = pcomInt->GetPasswordPolicies();
}

DBPolicyNamesCommand::DBPolicyNamesCommand(CommandInterface *pcomInt,
                                           StringX &sxPolicyName,
                                           PWPolicy &st_pp)
  : Command(pcomInt), m_sxPolicyName(sxPolicyName), m_st_ppp(st_pp),
  m_bSingleAdd(true)
{
  m_OldMapPSWDPLC = pcomInt->GetPasswordPolicies();
}

int DBPolicyNamesCommand::Execute()
{
  if (!m_pcomInt->IsReadOnly()) {
    bool bChanged(false);
    if (m_bSingleAdd) {
      bChanged = m_pcomInt->AddPolicy(m_sxPolicyName, m_st_ppp);
    } else {
      switch (m_function) {
        case NP_ADDNEW:
        {
          PSWDPolicyMapIter iter;
          int count(0);
          for (iter = m_NewMapPSWDPLC.begin(); iter != m_NewMapPSWDPLC.end(); iter++) {
            if (m_pcomInt->AddPolicy(iter->first, iter->second))
              count++;
          }
          bChanged = count > 0;
          break;
        }
        case NP_REPLACEALL:
          if (m_OldMapPSWDPLC != m_NewMapPSWDPLC)
            bChanged = m_pcomInt->SetPasswordPolicies(m_NewMapPSWDPLC);
          break;
        default:
          // Unknown function
          ASSERT(0);
          break;
      }
    }

    if (m_bNotifyGUI) {
      m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_UPDATE_STATUSBAR,
                                        CUUID::NullUUID());
    }
    if (bChanged)
      m_CommandDBChange = DBPOLICYNAMES;
  }
  return 0;
}

void DBPolicyNamesCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DBPOLICYNAMES) {
    m_pcomInt->SetPasswordPolicies(m_OldMapPSWDPLC);

    if (m_bNotifyGUI) {
      m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_UPDATE_STATUSBAR,
                                        CUUID::NullUUID());
    }
  }
}

// ------------------------------------------------
// DBEmptyGroupsCommand
// ------------------------------------------------

DBEmptyGroupsCommand::DBEmptyGroupsCommand(CommandInterface *pcomInt,
                                           const std::vector<StringX> &vEmptyGroups,
                                           Function function)
  : Command(pcomInt), m_vNewEmptyGroups(vEmptyGroups),
  m_function(function), m_bSingleGroup(false)
{
  m_vOldEmptyGroups = pcomInt->GetEmptyGroups();
}

DBEmptyGroupsCommand::DBEmptyGroupsCommand(CommandInterface *pcomInt,
                                           const StringX &sxEmptyGroup,
                                           Function function)
  : Command(pcomInt), m_sxEmptyGroup(sxEmptyGroup), m_function(function),
  m_bSingleGroup(true)
{
  m_vOldEmptyGroups = pcomInt->GetEmptyGroups();
}


DBEmptyGroupsCommand::DBEmptyGroupsCommand(CommandInterface *pcomInt,
                                           const StringX &sxOldGroup,
                                           const StringX &sxNewGroup,
                                           Function function)
  : Command(pcomInt), m_sxOldGroup(sxOldGroup), m_sxNewGroup(sxNewGroup),
  m_function(function)
{
  // This function call is used to rename a single empty group (EG_RENAME) or
  // to rename all empty groups that are sub-groups of the current group
  // (EG_RENAMEPATH).

  m_bSingleGroup = (function == EG_ADD || function == EG_DELETE || function == EG_RENAME);
}

int DBEmptyGroupsCommand::Execute()
{
  if (!m_pcomInt->IsReadOnly()) {
    bool bChanged(false);
    if (m_bSingleGroup) {
      // Single Empty Group functions
      switch (m_function) {
        case EG_ADD:
          bChanged = m_pcomInt->AddEmptyGroup(m_sxEmptyGroup);
          break;
        case EG_DELETE:
          bChanged = m_pcomInt->RemoveEmptyGroup(m_sxEmptyGroup);
          break;
        case EG_RENAME:
          bChanged = m_pcomInt->RenameEmptyGroup(m_sxOldGroup, m_sxNewGroup);
          break;
        default:
          // Ignore multi-group functions
          ASSERT(0);
          break;
      }
    } else {
      // Multi-Empty Group functions
      switch (m_function) {
        case EG_ADDALL:
        {
          int count(0);
          for (size_t n = 0; n < m_vNewEmptyGroups.size(); n++) {
            if (m_pcomInt->AddEmptyGroup(m_vNewEmptyGroups[n]))
              count++;
          }
          bChanged = count > 0;
          break;
        }
        case EG_REPLACEALL:
          if (m_vOldEmptyGroups != m_vNewEmptyGroups) {
            bChanged = m_pcomInt->SetEmptyGroups(m_vNewEmptyGroups);
          }
          break;
        case EG_RENAMEPATH:
          if (m_sxOldGroup != m_sxNewGroup) {
            bChanged = m_pcomInt->RenameEmptyGroupPaths(m_sxOldGroup, m_sxNewGroup);
          }
          break;
        default:
          // Ignore single group functions
          ASSERT(0);
          break;
      }
    }
    if (bChanged) {
      if (m_bNotifyGUI) {
        m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_REFRESH_TREE,
          CUUID::NullUUID());

        m_CommandDBChange = DBEMPTYGROUP;
      }
    }
  }
  return 0;
}

void DBEmptyGroupsCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DBEMPTYGROUP) {
    if (m_bSingleGroup) {
      // Single Empty Group functions
      switch (m_function) {
        case EG_ADD:
          m_pcomInt->RemoveEmptyGroup(m_sxEmptyGroup);
          break;
        case EG_DELETE:
          m_pcomInt->AddEmptyGroup(m_sxEmptyGroup);
          break;
        case EG_RENAME:
          m_pcomInt->RenameEmptyGroup(m_sxNewGroup, m_sxOldGroup);
          break;
        default:
          // Ignore multi-group functions
          ASSERT(0);
          break;
      }
    } else {
      // Multi-Empty Group functions
      switch (m_function) {
        case EG_ADDALL:
        case EG_REPLACEALL:
          m_pcomInt->SetEmptyGroups(m_vOldEmptyGroups);
          break;
        case EG_RENAMEPATH:
          m_pcomInt->RenameEmptyGroupPaths(m_sxNewGroup, m_sxOldGroup);
          break;
        default:
          // Ignore single group functions
          ASSERT(0);
          break;
      }
    }
    if (m_bNotifyGUI) {
      m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_REFRESH_TREE,
        CUUID::NullUUID());
    }
  }
}

// ------------------------------------------------
// AddEntryCommand
// ------------------------------------------------

AddEntryCommand::AddEntryCommand(CommandInterface *pcomInt, const CItemData &ci,
                                 const CUUID &baseUUID,
                                 const CItemAtt *att, const Command *pcmd)
  : Command(pcomInt), m_ci(ci)
{
  if (att != NULL)
    m_att = *att;

  if (m_ci.IsDependent()) {
    ASSERT(baseUUID != CUUID::NullUUID());
    m_ci.SetBaseUUID(baseUUID);
  }

  if (pcmd != NULL)
    m_bNotifyGUI = pcmd->GetGUINotify();
}

AddEntryCommand::~AddEntryCommand()
{
}

int AddEntryCommand::Execute()
{
  if (!m_pcomInt->IsReadOnly()) {
    m_pcomInt->DoAddEntry(m_ci, &m_att);
    m_pcomInt->AddChangedNodes(m_ci.GetGroup());

    if (m_ci.IsDependent()) {
      m_pcomInt->DoAddDependentEntry(m_ci.GetBaseUUID(), m_ci.GetUUID(),
        m_ci.GetEntryType());
    }

    if (m_bNotifyGUI) {
      m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_ADD_ENTRY,
        m_ci.GetUUID());
    }

    time_t tttXTime;
    m_ci.GetXTime(tttXTime);
    if (tttXTime != time_t(0)) {
      m_pcomInt->AddExpiryEntry(m_ci);
    }
    m_CommandDBChange = DB;
  }
  return 0;
}

void AddEntryCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DB) {
    // Do actions in reverse order to Execute
    if (m_ci.IsDependent()) {
      m_pcomInt->DoRemoveDependentEntry(m_ci.GetBaseUUID(), m_ci.GetUUID(),
        m_ci.GetEntryType());
    }

    DeleteEntryCommand delete_entry_cmd(m_pcomInt, m_ci, this);
    delete_entry_cmd.Execute();
  }
}

// ------------------------------------------------
// DeleteEntryCommand
// ------------------------------------------------

DeleteEntryCommand::DeleteEntryCommand(CommandInterface *pcomInt,
                                       const CItemData &ci, const Command *pcmd)
  : Command(pcomInt), m_ci(ci), m_dependents(0)
{
  if (pcmd != NULL) {
    m_bNotifyGUI = pcmd->GetGUINotify();
  }

  if (ci.IsNormal())
    m_base_uuid = CUUID::NullUUID();
  else {
    const CUUID uuid = ci.GetUUID();
    // If ci is not a normal entry, gather the related entry
    // info for undo
    if (ci.IsDependent()) {
      // For aliases or shortcuts, we just need the uuid of the base entry
      m_base_uuid = ci.GetBaseUUID();
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
      for (iter = immap.lower_bound(uuid);
           iter != immap.upper_bound(uuid); iter++) {
        const CUUID dep_uuid(iter->second);
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
  // Get out quick if R-O
  if (m_pcomInt->IsReadOnly())
    return 0;

  // There's at least one case where we may get here with an entry
  // that's already been deleted: Consider a base and its shortcut
  // both in the same group, and the group's being deleted:
  // Each has its own delete command in the group-generated
  // multicommand, but the shortcut will have been deleted as
  // part of the base's dependency deletion...
  // Easiest solution is to check here that we don't delete
  // something that's no longer in the system. Should be OK for
  // undo's.

  if (m_pcomInt->Find(m_ci.GetUUID()) == m_pcomInt->GetEntryEndIter())
    return 1;

  if (m_bNotifyGUI) {
    m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_DELETE_ENTRY,
                                      m_ci.GetUUID());
  }
  // XXX if entry has an attachment, find and store it in m_att for undo.
  // XXX as well as removing it / decrementing its refcount
  m_pcomInt->DoDeleteEntry(m_ci);
  m_pcomInt->AddChangedNodes(m_ci.GetGroup());
  m_pcomInt->RemoveExpiryEntry(m_ci);

  m_CommandDBChange = DB;
  return 0;
}

void DeleteEntryCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DB) {
    MultiCommands *pmulticmds = MultiCommands::Create(m_pcomInt);

    if (m_ci.IsDependent()) {
      // Check if dep entry hasn't already been added - can happen if
      // base and dep in group that's being undeleted.
      if (m_pcomInt->Find(m_ci.GetUUID()) == m_pcomInt->GetEntryEndIter()) {
        pmulticmds->Add(AddEntryCommand::Create(m_pcomInt, m_ci, m_ci.GetBaseUUID(), &m_att, this));
      }
    } else {
      pmulticmds->Add(AddEntryCommand::Create(m_pcomInt, m_ci, m_ci.GetBaseUUID(), &m_att, this));

      if (m_ci.IsShortcutBase()) { // restore dependents
        for (std::vector<CItemData>::iterator iter = m_dependents.begin();
             iter != m_dependents.end(); iter++) {
          pmulticmds->Add(AddEntryCommand::Create(m_pcomInt, *iter, iter->GetBaseUUID(), NULL));
        }
      } else if (m_ci.IsAliasBase()) {
        // Undeleting an alias base means making all the dependents refer to the alias
        // again. Perhaps the easiest approach is to delete the existing entries
        // and create new aliases.
        for (std::vector<CItemData>::iterator iter = m_dependents.begin();
             iter != m_dependents.end(); iter++) {
          // Need to check that alias still exists - could have been deleted in group along with item
          // being undone, in which case it will be added separately
          if (m_pcomInt->Find(iter->GetUUID()) == m_pcomInt->GetEntryEndIter())
            continue;

          // out with the old...
          pmulticmds->Add(DeleteEntryCommand::Create(m_pcomInt, *iter, this));
          // in with the new!
          pmulticmds->Add(AddEntryCommand::Create(m_pcomInt, *iter, iter->GetBaseUUID(), NULL, this));
        }
      } // IsAliasBase
    } // !IsDependent

    // Now do everything, if there is anything to do.
    if (pmulticmds->GetSize() > 0)
      pmulticmds->Execute();

    // Since not needed for Undo/Redo again - delete it
    delete pmulticmds;
  } // R/W & change to undo
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
  ASSERT(m_old_ci.GetUUID() == m_new_ci.GetUUID());
}

EditEntryCommand::~EditEntryCommand()
{
}

int EditEntryCommand::Execute()
{
  if (!m_pcomInt->IsReadOnly()) {
    m_pcomInt->DoReplaceEntry(m_old_ci, m_new_ci);

    m_pcomInt->AddChangedNodes(m_old_ci.GetGroup());
    m_pcomInt->AddChangedNodes(m_new_ci.GetGroup());

    if (m_bNotifyGUI) {
      const CUUID entry_uuid = m_old_ci.GetUUID();
      // If the entry's group has changed, refresh the entire tree, otherwise, just the entry
      // in the tree and list views
      UpdateGUICommand::GUI_Action gac = (m_old_ci.GetGroup() != m_new_ci.GetGroup()) ?
        UpdateGUICommand::GUI_REFRESH_TREE : UpdateGUICommand::GUI_REFRESH_ENTRY;
      m_pcomInt->NotifyGUINeedsUpdating(gac, entry_uuid);
    }

    m_CommandDBChange = DB;
  }
  return 0;
}

void EditEntryCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DB) {
    m_pcomInt->DoReplaceEntry(m_new_ci, m_old_ci);

    if (m_bNotifyGUI) {
      const CUUID entry_uuid = m_old_ci.GetUUID();
      // If the entry's group has changed, refresh the entire tree, otherwise, just the entry
      // in the tree and list views
      UpdateGUICommand::GUI_Action gac = (m_old_ci.GetGroup() != m_new_ci.GetGroup()) ?
        UpdateGUICommand::GUI_REFRESH_TREE : UpdateGUICommand::GUI_REFRESH_ENTRY;
      m_pcomInt->NotifyGUINeedsUpdating(gac, entry_uuid);
    }
  }
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
  m_entry_uuid = ci.GetUUID();
  m_old_status = ci.GetStatus();
  m_old_value = ci.GetFieldValue(m_ftype);
}

void UpdateEntryCommand::Doit(const CUUID &entry_uuid,
                              CItemData::FieldType ftype,
                              const StringX &value,
                              CItemData::EntryStatus es,
                              UpdateGUICommand::ExecuteFn efn)
{
  ItemListIter pos = m_pcomInt->Find(entry_uuid);
  if (pos != m_pcomInt->GetEntryEndIter()) {
    if (ftype != CItemData::PASSWORD)
      pos->second.SetFieldValue(ftype, value);
    else {
      if (efn == UpdateGUICommand::WN_EXECUTE_REDO) {
        m_oldpwhistory = pos->second.GetPWHistory();
        pos->second.GetXTime(m_tttoldXtime);
        pos->second.UpdatePassword(value);
      } else {
        pos->second.SetPassword(value);
        pos->second.SetXTime(m_tttoldXtime);
        pos->second.SetPWHistory(m_oldpwhistory);
      }
    }
    if (ftype == CItemData::PASSWORD ||
        ftype == CItemData::XTIME)
      m_pcomInt->UpdateExpiryEntry(pos->second);

    pos->second.SetStatus(es);
    m_pcomInt->AddChangedNodes(pos->second.GetGroup());
  }
}

int UpdateEntryCommand::Execute()
{
  if (!m_pcomInt->IsReadOnly()) {
    Doit(m_entry_uuid, m_ftype, m_value, CItemData::ES_MODIFIED, UpdateGUICommand::WN_EXECUTE_REDO);

    if (m_bNotifyGUI)
      m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_REFRESH_ENTRYFIELD,
        m_entry_uuid, m_ftype);

    if (m_ftype == CItemData::XTIME)
      m_pcomInt->UpdateExpiryEntry(m_entry_uuid, m_ftype, m_value);

    m_CommandDBChange = DB;
  }
  return 0;
}

void UpdateEntryCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DB) {
    Doit(m_entry_uuid, m_ftype, m_old_value, m_old_status, UpdateGUICommand::WN_UNDO);

    if (m_bNotifyGUI)
      m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_REFRESH_ENTRYFIELD,
        m_entry_uuid, m_ftype);
  }
}

// ------------------------------------------------
// UpdatePasswordCommand
// ------------------------------------------------

UpdatePasswordCommand::UpdatePasswordCommand(CommandInterface *pcomInt,
                                             CItemData &ci,
                                             const StringX sxNewPassword)
  : Command(pcomInt), m_sxNewPassword(sxNewPassword)
{
  m_entry_uuid = ci.GetUUID();
  m_old_status = ci.GetStatus();
  m_sxOldPassword = ci.GetPassword();
  m_sxOldPWHistory = ci.GetPWHistory();
  ci.GetXTime(m_tttOldXTime);
}

int UpdatePasswordCommand::Execute()
{
  if (!m_pcomInt->IsReadOnly() && m_sxOldPassword != m_sxNewPassword) {
    ItemListIter pos = m_pcomInt->Find(m_entry_uuid);
    if (pos != m_pcomInt->GetEntryEndIter()) {
      pos->second.UpdatePassword(m_sxNewPassword);
      time_t tttNewXTime;
      pos->second.GetXTime(tttNewXTime);
      if (m_tttOldXTime != tttNewXTime) {
        m_pcomInt->UpdateExpiryEntry(pos->second);
      }
      pos->second.SetStatus(CItemData::ES_MODIFIED);
      m_pcomInt->AddChangedNodes(pos->second.GetGroup());
    }

    m_CommandDBChange = DB;

    if (m_bNotifyGUI)
      m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_REFRESH_ENTRYPASSWORD,
        m_entry_uuid);

  }
  return 0;
}

void UpdatePasswordCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DB) {
    ItemListIter pos = m_pcomInt->Find(m_entry_uuid);
    if (pos != m_pcomInt->GetEntryEndIter()) {
      pos->second.SetPassword(m_sxOldPassword);
      pos->second.SetPWHistory(m_sxOldPWHistory);
      pos->second.SetStatus(m_old_status);
      pos->second.SetXTime(m_tttOldXTime);
    }

    if (m_bNotifyGUI)
      m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_REFRESH_ENTRYPASSWORD,
        m_entry_uuid);
  }
}

// ------------------------------------------------
// AddDependentEntryCommand
// ------------------------------------------------

AddDependentEntryCommand::AddDependentEntryCommand(CommandInterface *pcomInt,
                                                   const CUUID &base_uuid,
                                                   const CUUID &entry_uuid,
                                                   const CItemData::EntryType type)
  : Command(pcomInt), m_base_uuid(base_uuid),
    m_entry_uuid(entry_uuid), m_type(type)
{
}

int AddDependentEntryCommand::Execute()
{
  if (!m_pcomInt->IsReadOnly()) {
    m_pcomInt->DoAddDependentEntry(m_base_uuid, m_entry_uuid, m_type);

    m_CommandDBChange = DB;
  }
  return 0;
}

void AddDependentEntryCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DB) {
    m_pcomInt->DoRemoveDependentEntry(m_base_uuid, m_entry_uuid, m_type);
  }
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
  int rc(0);
  if (!m_pcomInt->IsReadOnly()) {
    if (m_type == CItemData::ET_ALIAS) {
      m_saved_base2aliases_mmap = m_pcomInt->GetBase2AliasesMmap();
    } else { // if !alias, assume shortcut
      m_saved_base2shortcuts_mmap = m_pcomInt->GetBase2ShortcutsMmap();
    }

    rc = m_pcomInt->DoAddDependentEntries(m_dependentslist, m_pRpt,
      m_type, m_iVia,
      m_pmapDeletedItems, m_pmapSaveStatus);

    m_CommandDBChange = DB;
  }
  return rc;
}

void AddDependentEntriesCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DB) {
    m_pcomInt->UndoAddDependentEntries(m_pmapDeletedItems, m_pmapSaveStatus);

    if (m_type == CItemData::ET_ALIAS) {
      m_pcomInt->SetBase2AliasesMmap(m_saved_base2aliases_mmap);
    } else { // if !alias, assume shortcut
      m_pcomInt->SetBase2ShortcutsMmap(m_saved_base2shortcuts_mmap);
    }
  }
}

// ------------------------------------------------
// RemoveDependentEntryCommand
// ------------------------------------------------

RemoveDependentEntryCommand::RemoveDependentEntryCommand(CommandInterface *pcomInt,
                                                         const CUUID &base_uuid,
                                                         const CUUID &entry_uuid,
                                                         const CItemData::EntryType type)
  : Command(pcomInt), m_base_uuid(base_uuid),
    m_entry_uuid(entry_uuid), m_type(type)
{
}

int RemoveDependentEntryCommand::Execute()
{
  if (!m_pcomInt->IsReadOnly()) {
    m_pcomInt->DoRemoveDependentEntry(m_base_uuid, m_entry_uuid, m_type);

    m_CommandDBChange = DB;
  }
  return 0;
}

void RemoveDependentEntryCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DB) {
    m_pcomInt->DoAddDependentEntry(m_base_uuid, m_entry_uuid, m_type);
  }
}

// ------------------------------------------------
// MoveDependentEntriesCommand
// ------------------------------------------------

MoveDependentEntriesCommand::MoveDependentEntriesCommand(CommandInterface *pcomInt,
                                                         const CUUID &from_baseuuid,
                                                         const CUUID &to_baseuuid,
                                                         const CItemData::EntryType type)
  : Command(pcomInt), m_from_baseuuid(from_baseuuid),
    m_to_baseuuid(to_baseuuid), m_type(type)
{
}

int MoveDependentEntriesCommand::Execute()
{
  if (!m_pcomInt->IsReadOnly()) {
    if (m_type == CItemData::ET_ALIAS) {
      m_saved_base2aliases_mmap = m_pcomInt->GetBase2AliasesMmap();
    } else { // if !alias, assume shortcut
      m_saved_base2shortcuts_mmap = m_pcomInt->GetBase2ShortcutsMmap();
    }

    if (m_pcomInt->DoMoveDependentEntries(m_from_baseuuid, m_to_baseuuid, m_type))
      m_CommandDBChange = DB;
  }
  return 0;
}

void MoveDependentEntriesCommand::Undo()
{
  // Can't move all back by calling DoMoveDependentEntries with reversed arguments
  // as there may have been some originally linked that should not be moved.
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DB) {
    if (m_type == CItemData::ET_ALIAS) {
      m_pcomInt->SetBase2AliasesMmap(m_saved_base2aliases_mmap);
    } else { // if !alias, assume shortcut
      m_pcomInt->SetBase2ShortcutsMmap(m_saved_base2shortcuts_mmap);
    }
  }
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
  int rc(0);
  if (!m_pcomInt->IsReadOnly()) {
    rc = m_pcomInt->DoUpdatePasswordHistory(m_iAction, m_new_default_max,
                               m_mapSavedHistory);

    m_CommandDBChange = DB;
  }
  return rc;
}

void UpdatePasswordHistoryCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DB) {
    m_pcomInt->UndoUpdatePasswordHistory(m_mapSavedHistory);
  }
}

// ------------------------------------------------
// RenameGroupCommand
// ------------------------------------------------

RenameGroupCommand::RenameGroupCommand(CommandInterface *pcomInt,
                                       const StringX sxOldPath, const StringX sxNewPath)
 : Command(pcomInt), m_sxOldPath(sxOldPath), m_sxNewPath(sxNewPath)
{}

int RenameGroupCommand::Execute()
{
  int rc(0);
  if (!m_pcomInt->IsReadOnly() && m_sxOldPath != m_sxNewPath) {
    rc = m_pcomInt->DoRenameGroup(m_sxOldPath, m_sxNewPath);

    m_CommandDBChange = DB;
  }
  return rc;
}

void RenameGroupCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DB) {
    m_pcomInt->UndoRenameGroup(m_sxOldPath, m_sxNewPath);
  }
}

// ------------------------------------------------
// ChangeDBHeaderCommand
// ------------------------------------------------

ChangeDBHeaderCommand::ChangeDBHeaderCommand(CommandInterface *pcomInt,
  const StringX sxNewValue, const PWSfile::HeaderType ht)
  : Command(pcomInt), m_sxNewValue(sxNewValue), m_ht(ht)
{
  m_sxOldValue = m_pcomInt->GetHeaderItem(m_ht);
}

int ChangeDBHeaderCommand::Execute()
{
  int rc(0);
  if (!m_pcomInt->IsReadOnly() && m_sxOldValue != m_sxNewValue) {
    rc = m_pcomInt->DoChangeHeader(m_sxNewValue, m_ht);

    m_CommandDBChange = DBHEADER;
  }
  return rc;
}

void ChangeDBHeaderCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DBHEADER) {
    m_pcomInt->UndoChangeHeader(m_sxOldValue, m_ht);
  }
}

// ------------------------------------------------
// DBFiltersCommand
// ------------------------------------------------

DBFiltersCommand::DBFiltersCommand(CommandInterface *pcomInt,
  PWSFilters &MapFilters)
  : Command(pcomInt), m_NewMapFilters(MapFilters)
{
  m_OldMapFilters = pcomInt->GetDBFilters();
}

int DBFiltersCommand::Execute()
{
  if (!m_pcomInt->IsReadOnly() && m_OldMapFilters != m_NewMapFilters) {
    bool bChanged(false);
    bChanged = m_pcomInt->SetDBFilters(m_NewMapFilters);

    if (bChanged) {
      m_CommandDBChange = DBFILTERS;

      if (m_bNotifyGUI) {
        m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_UPDATE_STATUSBAR,
          CUUID::NullUUID());
      }
    }
  }
  return 0;
}

void DBFiltersCommand::Undo()
{
  if (!m_pcomInt->IsReadOnly() && m_CommandDBChange == DBFILTERS) {
    m_pcomInt->SetDBFilters(m_OldMapFilters);

    if (m_bNotifyGUI) {
      m_pcomInt->NotifyGUINeedsUpdating(UpdateGUICommand::GUI_UPDATE_STATUSBAR,
        CUUID::NullUUID());
    }
  }
}

