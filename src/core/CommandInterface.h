/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __COMMANDINTERFACE_H
#define __COMMANDINTERFACE_H
#include "coredefs.h"
#include "Command.h"
/**
 * An abstract base class representing all of the PWScore functionality
 * that Command needs to know about.
 * This is an alternative to making every Command subclass a friend of
 * PWScore.
 */

class CReport;

class CommandInterface {
 public:
  CommandInterface() {}
  // Methods used both by PWScore and Commands:
  virtual bool IsReadOnly() const = 0;

  virtual bool GetUniqueGTUValidated() const = 0;

  virtual ItemListIter Find(const pws_os::CUUID &entry_uuid) = 0;
  virtual ItemListConstIter Find(const pws_os::CUUID &entry_uuid) const = 0;
  virtual ItemListIter GetEntryIter() = 0;
  virtual ItemListConstIter GetEntryIter() const = 0;
  virtual ItemListIter GetEntryEndIter() = 0;
  virtual ItemListConstIter GetEntryEndIter() const = 0;

  // Command-specific methods
  virtual void DoAddEntry(const CItemData &item, const CItemAtt *att) = 0;
  virtual void DoDeleteEntry(const CItemData &item) = 0;
  virtual void DoReplaceEntry(const CItemData &old_ci, const CItemData &new_ci) = 0;

  // General routines for aliases and shortcuts
  virtual void DoAddDependentEntry(const pws_os::CUUID &base_uuid,
                                   const pws_os::CUUID &entry_uuid,
                                   const CItemData::EntryType type) = 0;
  virtual void DoRemoveDependentEntry(const pws_os::CUUID &base_uuid,
                                      const pws_os::CUUID &entry_uuid, 
                                      const CItemData::EntryType type) = 0;
  virtual void DoRemoveAllDependentEntries(const pws_os::CUUID &base_uuid, 
                                           const CItemData::EntryType type) = 0;
  virtual int DoAddDependentEntries(UUIDVector &dependentslist, CReport *rpt, 
                                    const CItemData::EntryType type, 
                                    const int &iVia,
                                    ItemList *pmapDeletedItems = nullptr,
                                    SaveTypePWMap *pmapSaveTypePW = nullptr) = 0;
  virtual void UndoAddDependentEntries(ItemList *pmapDeletedItems,
                                       SaveTypePWMap *pmapSaveTypePW) = 0;
  virtual bool DoMoveDependentEntries(const pws_os::CUUID &from_baseuuid, 
                                      const pws_os::CUUID &to_baseuuid, 
                                      const CItemData::EntryType type) = 0;

  virtual int DoUpdatePasswordHistory(int iAction, int new_default_max,
                                      SavePWHistoryMap &mapSavedHistory) = 0;
  virtual void UndoUpdatePasswordHistory(SavePWHistoryMap &mapSavedHistory) = 0;

  virtual int DoRenameGroup(const StringX &sxOldPath, const StringX &sxNewPath,
                            MultiCommands * &pmulticmds) = 0;
  virtual void UndoRenameGroup(MultiCommands *pmulticmds) = 0;

  virtual int DoChangeHeader(const StringX &sxNewValue, const PWSfile::HeaderType ht) = 0;
  virtual void UndoChangeHeader(const StringX &sxOldValue, const PWSfile::HeaderType ht) = 0;
  virtual StringX GetHeaderItem(PWSfile::HeaderType ht) = 0;

  virtual void AddChangedNodes(StringX path) = 0;
  
  virtual const CItemData *GetBaseEntry(const CItemData *pAliasOrSC) const = 0;
  virtual const ItemMMap &GetBase2AliasesMmap() const = 0;
  virtual void SetBase2AliasesMmap(ItemMMap &) = 0;
  virtual const ItemMMap &GetBase2ShortcutsMmap() const = 0;
  virtual void SetBase2ShortcutsMmap(ItemMMap &) = 0;

  virtual void NotifyGUINeedsUpdating(UpdateGUICommand::GUI_Action,
                                      const pws_os::CUUID &,
                                      CItemData::FieldType ft = CItemData::START) = 0;
  virtual void NotifyGUINeedsUpdating(UpdateGUICommand::GUI_Action ga,
                                      const std::vector<StringX> &vGroups) = 0;

  virtual void AddExpiryEntry(const CItemData &ci) = 0;
  virtual void UpdateExpiryEntry(const CItemData &ci) = 0;
  virtual void UpdateExpiryEntry(const pws_os::CUUID &uuid, const CItemData::FieldType ft,
                                 const StringX &value) = 0;
  virtual void RemoveExpiryEntry(const CItemData &ci) = 0;

  virtual const PSWDPolicyMap &GetPasswordPolicies() = 0;
  virtual bool SetPasswordPolicies(const PSWDPolicyMap &MapPSWDPLC) = 0;
  virtual bool AddPolicy(const StringX &sxPolicyName, const PWPolicy &st_pp,
                         const bool bAllowReplace = false) = 0;
  virtual bool GetPolicyFromName(const StringX &sxPolicyName, PWPolicy &st_pp) const = 0;

  virtual bool SetEmptyGroups(const std::vector<StringX> &vEmptyGroups) = 0;
  virtual bool AddEmptyGroup(const StringX &sxEmptyGroup) = 0;
  virtual bool RemoveEmptyGroup(const StringX &sxEmptyGroup) = 0;
  virtual bool RenameEmptyGroup(const StringX &sxOldGroup, const StringX &sxNewGroup) = 0;
  virtual bool RenameEmptyGroupPaths(const StringX &sxOldPath, const StringX &sxNewPath) = 0;
  virtual const std::vector<StringX> & GetEmptyGroups() const = 0;

  virtual const PWSFilters &GetDBFilters() = 0;
  virtual bool SetDBFilters(const PWSFilters &MapDBFilters) = 0;

  virtual uint32 GetHashIters() const = 0;
  virtual void SetHashIters(uint32 value) = 0;

  std::vector<StringX> &GetModifiedNodes() { return m_vModifiedNodes; }
  void SetModifiedNodes(const std::vector<StringX> &saved_vNodes_Modified)
  { m_vModifiedNodes = saved_vNodes_Modified; }

  
  virtual ~CommandInterface() {}

 protected:
  // Changed groups
  std::vector<StringX> m_vModifiedNodes;
};

#endif /* __COMMANDINTERFACE_H */
