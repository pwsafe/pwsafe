/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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
  virtual bool IsChanged() const = 0;
  virtual bool IsReadOnly() const = 0;
  virtual void SetDBChanged(bool bDBChanged, bool bNotify = true) = 0;
  virtual void SetDBPrefsChanged(bool bDBprefschanged) = 0;
  virtual bool HaveHeaderPreferencesChanged(const StringX &prefString) = 0;

  virtual void SetUniqueGTUValidated(bool bState) = 0;
  virtual bool GetUniqueGTUValidated() const = 0;

  virtual ItemListIter Find(const uuid_array_t &entry_uuid) = 0;
  virtual ItemListConstIter Find(const uuid_array_t &entry_uuid) const = 0;
  virtual ItemListIter GetEntryIter() = 0;
  virtual ItemListConstIter GetEntryIter() const = 0;
  virtual ItemListIter GetEntryEndIter() = 0;
  virtual ItemListConstIter GetEntryEndIter() const = 0;

  // Command-specific methods
  virtual void DoAddEntry(const CItemData &item) = 0;
  virtual void DoDeleteEntry(const CItemData &item) = 0;
  virtual void DoReplaceEntry(const CItemData &old_ci, const CItemData &new_ci) = 0;

  // General routines for aliases and shortcuts
  virtual void DoAddDependentEntry(const uuid_array_t &base_uuid,
                                   const uuid_array_t &entry_uuid,
                                   const CItemData::EntryType type) = 0;
  virtual void DoRemoveDependentEntry(const uuid_array_t &base_uuid,
                                      const uuid_array_t &entry_uuid, 
                                      const CItemData::EntryType type) = 0;
  virtual void DoRemoveAllDependentEntries(const uuid_array_t &base_uuid, 
                                           const CItemData::EntryType type) = 0;
  virtual int DoAddDependentEntries(UUIDList &dependentslist, CReport *rpt, 
                                    const CItemData::EntryType type, 
                                    const int &iVia,
                                    ItemList *pmapDeletedItems = NULL,
                                    SaveTypePWMap *pmapSaveTypePW = NULL) = 0;
  virtual void UndoAddDependentEntries(ItemList *pmapDeletedItems,
                                       SaveTypePWMap *pmapSaveTypePW) = 0;
  virtual void DoMoveDependentEntries(const uuid_array_t &from_baseuuid, 
                                      const uuid_array_t &to_baseuuid, 
                                      const CItemData::EntryType type) = 0;

  virtual int DoUpdatePasswordHistory(int iAction, int new_default_max,
                                      SavePWHistoryMap &mapSavedHistory) = 0;
  virtual void UndoUpdatePasswordHistory(SavePWHistoryMap &mapSavedHistory) = 0;

  virtual const std::vector<StringX> &GetVnodesModified() const = 0;
  virtual void SetVnodesModified(const std::vector<StringX> &) = 0;
  virtual void AddChangedNodes(StringX path) = 0;
  
  virtual CItemData *GetBaseEntry(const CItemData *pAliasOrSC) = 0;
  virtual const ItemMMap &GetBase2AliasesMmap() const = 0;
  virtual void SetBase2AliasesMmap(ItemMMap &) = 0;
  virtual const ItemMMap &GetBase2ShortcutsMmap() const = 0;
  virtual void SetBase2ShortcutsMmap(ItemMMap &) = 0;
  virtual const ItemMap &GetAlias2BaseMap() const = 0;
  virtual void SetAlias2BaseMap(const ItemMap &) = 0;
  virtual const ItemMap &GetShortcuts2BaseMap() const = 0;
  virtual void SetShortcuts2BaseMap(const ItemMap &) = 0;

  virtual void NotifyGUINeedsUpdating(UpdateGUICommand::GUI_Action,
                                      uuid_array_t &,
                                      CItemData::FieldType ft = CItemData::START,
                                      bool bUpdateGUI = true) = 0;

  virtual ~CommandInterface() {}
};

#endif /* __COMMANDINTERFACE_H */
