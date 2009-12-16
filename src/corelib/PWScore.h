/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __PWSCORE_H
#define __PWSCORE_H

// PWScore.h
//-----------------------------------------------------------------------------

#include "os/pws_tchar.h"
#include "ItemData.h"
#include "StringX.h"
#include "PWSfile.h"
#include "PWSFilters.h"
#include "UUIDGen.h"
#include "Report.h"
#include "Proxy.h"
#include "Command.h"

#include "coredefs.h"

#define MAXDEMO 10

// Parameter list for GetBaseEntry
struct GetBaseEntryPL {
  // All fields except "InputType" are 'output'.
  StringX csPwdGroup;
  StringX csPwdTitle;
  StringX csPwdUser;
  uuid_array_t base_uuid;
  CItemData::EntryType InputType;
  CItemData::EntryType TargetType;
  int ibasedata;
  bool bMultipleEntriesFound;
};

class PWScore
{
  // Only class Command member functions can update the database.
  // Even within PWScore, changes should be via the Command class.
  // All public PWScore members only return information or entries
  // in PWScore structures
  friend class Command;

public:
  enum {
    SUCCESS = 0,
    FAILURE = 1,
    CANT_OPEN_FILE = -10,
    USER_CANCEL,                              // -9
    WRONG_PASSWORD = PWSfile::WRONG_PASSWORD, //  6 - ensure the same value
    BAD_DIGEST = PWSfile::BAD_DIGEST,         //  7 - ensure the same value
    UNKNOWN_VERSION,                          //  8
    NOT_SUCCESS,                              //  9
    ALREADY_OPEN,                             // 10
    INVALID_FORMAT,                           // 11
    USER_EXIT,                                // 12
    XML_FAILED_VALIDATION,                    // 13
    XML_FAILED_IMPORT,                        // 14
    LIMIT_REACHED,                            // 15
    UNIMPLEMENTED,                            // 16
    NO_ENTRIES_EXPORTED,                      // 17
  };

  PWScore();
  ~PWScore();

  // Set following to a Reporter-derived object
  // so that we can inform user of events of interest
  static void SetReporter(Reporter *pReporter) {m_pReporter = pReporter;}
  static void SetAsker(Asker *pAsker) {m_pAsker = pAsker;}

  // Get/Set Default User use/value
  bool GetUseDefUser() const {return m_usedefuser;}
  void SetUseDefUser(bool bUseDefUser) {m_usedefuser = bUseDefUser;}
  StringX GetDefUsername() const {return m_defusername;}
  void SetDefUsername(const StringX &sxdefuser) {m_defusername = sxdefuser;}

  // Get/Set File UUIDs
  void ClearFileUUID();
  void SetFileUUID(uuid_array_t &file_uuid_array);
  void GetFileUUID(uuid_array_t &file_uuid_array) const;

  // Get/Set Unknown Fields info
  bool HasHeaderUnknownFields() const
  {return !m_UHFL.empty();}
  int GetNumRecordsWithUnknownFields() const
  {return m_nRecordsWithUnknownFields;}
  void DecrementNumRecordsWithUnknownFields()
  {m_nRecordsWithUnknownFields--;}
  void IncrementNumRecordsWithUnknownFields()
  {m_nRecordsWithUnknownFields++;}

  // Clear out database structures and associated fields
  void ClearData();
  void ReInit(bool bNewfile = false);

  // Following used to read/write databases and Get/Set file name
  StringX GetCurFile() const {return m_currfile;}
  void SetCurFile(const StringX &file) {m_currfile = file;}

  int ReadCurFile(const StringX &passkey)
  {return ReadFile(m_currfile, passkey);}
  int ReadFile(const StringX &filename, const StringX &passkey);
  PWSfile::VERSION GetReadFileVersion() const {return m_ReadFileVersion;}
  bool BackupCurFile(int maxNumIncBackups, int backupSuffix,
                     const stringT &userBackupPrefix,
                     const stringT &userBackupDir);

  void NewFile(const StringX &passkey);
  int WriteCurFile() {return WriteFile(m_currfile);}
  int WriteFile(const StringX &filename, PWSfile::VERSION version = PWSfile::VCURRENT);
  int WriteV17File(const StringX &filename)
  {return WriteFile(filename, PWSfile::V17);}
  int WriteV2File(const StringX &filename)
  {return WriteFile(filename, PWSfile::V20);}

  // R/O file status
  void SetReadOnly(bool state) {m_IsReadOnly = state;}
  bool IsReadOnly() const {return m_IsReadOnly;};

  // Check/Change master passphrase
  int CheckPasskey(const StringX &filename, const StringX &passkey);
  void ChangePasskey(const StringX &newPasskey);
  void SetPassKey(const StringX &new_passkey);

  // Export databases
  int TestForExport(const stringT &subgroup_name, const int &subgroup_object,
                           const int &subgroup_function, const OrderedItemList *il);
  int WritePlaintextFile(const StringX &filename,
                         const CItemData::FieldBits &bsExport,
                         const stringT &subgroup, const int &iObject,
                         const int &iFunction, const TCHAR &delimiter,
                         const OrderedItemList *il = NULL);
  int WriteXMLFile(const StringX &filename,
                   const CItemData::FieldBits &bsExport,
                   const stringT &subgroup, const int &iObject,
                   const int &iFunction, const TCHAR &delimiter,
                   const OrderedItemList *il = NULL,
                   const bool &bFilterActive = false);

  // Import databases
  int ImportPlaintextFile(const StringX &ImportedPrefix,
                          const StringX &filename,
                          const TCHAR &fieldSeparator, const TCHAR &delimiter,
                          const bool &bImportPSWDsOnly,
                          stringT &strErrors,
                          int &numImported, int &numSkipped,
                          CReport &rpt);
  int ImportXMLFile(const stringT &ImportedPrefix,
                    const stringT &strXMLFileName,
                    const stringT &strXSDFileName,
                    const bool &bImportPSWDsOnly,
                    stringT &strErrors, int &numValidated, int &numImported,
                    bool &bBadUnknownFileFields,
                    bool &bBadUnknownRecordFields,
                    CReport &rpt);
  int ImportKeePassTextFile(const StringX &filename);

  // Locking files open in R/W mode
  bool LockFile(const stringT &filename, stringT &locker);
  bool IsLockedFile(const stringT &filename) const;
  void UnlockFile(const stringT &filename);

  // Following 3 routines only for SaveAs to use a temporary lock handle
  // LockFile2, UnLockFile2 & MoveLock
  bool LockFile2(const stringT &filename, stringT &locker);
  void UnlockFile2(const stringT &filename);
  void MoveLock()
  {m_lockFileHandle = m_lockFileHandle2; m_lockFileHandle2 = INVALID_HANDLE_VALUE;
   m_LockCount = m_LockCount2; m_LockCount2 = 0;}

  // Set application data
  void SetApplicationNameAndVersion(const stringT &appName, DWORD dwMajorMinor);

  // Return list of unique groups
  void GetUniqueGroups(std::vector<stringT> &ary) const;
  StringX GetUniqueTitle(const StringX &path, const StringX &title,
                         const StringX &user, const int IDS_MESSAGE);

  // Access to individual entries in database
  ItemListIter GetEntryIter()
  {return m_pwlist.begin();}
  ItemListConstIter GetEntryIter() const
  {return m_pwlist.begin();}
  ItemListIter GetEntryEndIter()
  {return m_pwlist.end();}
  ItemListConstIter GetEntryEndIter() const
  {return m_pwlist.end();}
  CItemData &GetEntry(ItemListIter iter)
  {return iter->second;}
  const CItemData &GetEntry(ItemListConstIter iter) const
  {return iter->second;}
  ItemList::size_type GetNumEntries() const {return m_pwlist.size();}
 
  // Command functions
  int Execute(Command *pcmd);
  void Undo();
  void Redo();
  void ClearCommands();
  void ResetStateAfterSave();
  bool AnyToUndo();
  bool AnyToRedo();

  // Find in m_pwlist by title and user name, exact match
  ItemListIter Find(const StringX &a_group,
                    const StringX &a_title, const StringX &a_user);
  ItemListIter Find(const uuid_array_t &entry_uuid)
  {return m_pwlist.find(entry_uuid);}
  ItemListConstIter Find(const uuid_array_t &entry_uuid) const
  {return m_pwlist.find(entry_uuid);}

  // General routines for aliases and shortcuts
  void GetAllDependentEntries(const uuid_array_t &base_uuid, UUIDList &dependentslist, 
                              const CItemData::EntryType type);
  bool GetDependentEntryBaseUUID(const uuid_array_t &entry_uuid, uuid_array_t &base_uuid, 
                                 const CItemData::EntryType type);
  bool GetBaseEntry(const StringX &passwd, GetBaseEntryPL &pl);

  // Actions relating to alias/base and shortcut/base maps
  bool GetAliasBaseUUID(const uuid_array_t &alias_uuid, uuid_array_t &base_uuid)
  {return GetDependentEntryBaseUUID(alias_uuid, base_uuid, CItemData::ET_ALIAS);}
  bool GetShortcutBaseUUID(const uuid_array_t &shortcut_uuid, uuid_array_t &base_uuid)
  {return GetDependentEntryBaseUUID(shortcut_uuid, base_uuid, CItemData::ET_SHORTCUT);}

  int NumAliases(const uuid_array_t &base_uuid)
  {return m_base2aliases_mmap.count(base_uuid);}
  int NumShortcuts(const uuid_array_t &base_uuid)
  {return m_base2shortcuts_mmap.count(base_uuid);}

  ItemListIter GetUniqueBase(const StringX &title, bool &bMultiple);
  ItemListIter GetUniqueBase(const StringX &grouptitle, 
                             const StringX &titleuser, bool &bMultiple);

  // Use following calls to 'SetChanged' & 'SetDBChanged' sparingly
  // outside of corelib
  void SetChanged(const bool bDBChanged, const bool bDBprefschanged)
  {m_bDBChanged = bDBChanged; 
   m_bDBPrefsChanged = bDBprefschanged;
   NotifyDBModified();}
  void SetDBChanged(const bool bDBChanged)
  {m_bDBChanged = bDBChanged;
   NotifyDBModified();}
  void SetDBPrefsChanged(const bool bDBprefschanged)
  {m_bDBPrefsChanged = bDBprefschanged;
   NotifyDBModified();}

  bool IsChanged() const {return m_bDBChanged;}
  bool HaveDBPrefsChanged() const {return m_bDBPrefsChanged;}
  bool HaveHeaderPreferencesChanged(const StringX prefString)
  {return _tcscmp(prefString.c_str(), m_hdr.m_prefString.c_str()) != 0;}

  // (Un)Register callback to be notified if the database changes
  bool RegisterOnDBModified(void (*pfcn) (LPARAM, bool), LPARAM);
  void UnRegisterOnDBModified();
  void NotifyDBModified();
  void SuspendOnDBNotification()
  {m_bNotifyDB = false;}
  void ResumeOnDBNotification()
  {m_bNotifyDB = m_pfcnNotifyDBModified != NULL && m_NotifyDBInstance != 0;}

  // (Un)Register callback to let GUI know that DB has changed
  bool RegisterGUINotify(void (*pfcn) (LPARAM , const Command::GUI_Action &,
                                       uuid_array_t &, LPARAM ), LPARAM);
  void UnRegisterGUINotify();
  void NotifyGUINeedsUpdating(const Command::GUI_Action &, uuid_array_t &,
                              LPARAM lparam = NULL);

  // (Un)Register callback to let GUI populate its field in an entry
  bool RegisterGUIUpdateEntry(void (*pfcn) (CItemData &));
  void UnRegisterGUIUpdateEntry();
  void GUIUpdateEntry(CItemData &ci);

  // (Un)Register callback to perform a GUI command
  bool RegisterGUICommandInterface(void (*pfcn) (LPARAM, const Command::ExecuteFn &,
                                   PWSGUICmdIF *), LPARAM instance);
  void UnRegisterGUICommandInterface();
  void CallGUICommandInterface(const Command::ExecuteFn &, PWSGUICmdIF *);

  // Get/Set Display information from/to database
  void SetDisplayStatus(const std::vector<bool> &s);
  const std::vector<bool> &GetDisplayStatus() const;
  bool WasDisplayStatusChanged() const;

  void CopyPWList(const ItemList &in);

  // Validate() returns true if data modified, false if all OK
  bool Validate(stringT &status);
  const PWSfile::HeaderRecord &GetHeader() const {return m_hdr;}
  
  // Filters
  PWSFilters m_MapFilters;

  // Changed nodes
  void ClearChangedNodes()
  {m_vnodes_modified.clear();}
  void AddChangedNodes(StringX path);
  bool IsNodeModified(StringX &path)
  {return std::find(m_vnodes_modified.begin(), m_vnodes_modified.end(), path) != 
                    m_vnodes_modified.end();}

  // Should be private but XML Import can't then reach it
  void (*m_pfcnGUIUpdateEntry) (CItemData &);

private:
  // Database update routines

  // NOTE: Member functions startig with 'Do' or 'Undo' are meant to
  // be executed via the Command process ONLY
  void DoAddEntry(const CItemData &item);
  void DoDeleteEntry(const CItemData &item);

  // General routines for aliases and shortcuts
  void DoAddDependentEntry(const uuid_array_t &base_uuid,
                           const uuid_array_t &entry_uuid,
                           const CItemData::EntryType type);
  void DoRemoveDependentEntry(const uuid_array_t &base_uuid,
                              const uuid_array_t &entry_uuid, 
                              const CItemData::EntryType type);
  void DoRemoveAllDependentEntries(const uuid_array_t &base_uuid, 
                                   const CItemData::EntryType type);
  int DoAddDependentEntries(UUIDList &dependentslist, CReport *rpt, 
                            const CItemData::EntryType type, 
                            const int &iVia,
                            ItemList *pmapDeletedItems = NULL,
                            SaveTypePWMap *pmapSaveTypePW = NULL);
  void UndoAddDependentEntries(ItemList *pmapDeletedItems,
                               SaveTypePWMap *pmapSaveTypePW);
  void DoMoveDependentEntries(const uuid_array_t &from_baseuuid, 
                              const uuid_array_t &to_baseuuid, 
                              const CItemData::EntryType type);

  // Actions for Aliases only
  void DoResetAllAliasPasswords(const uuid_array_t &base_uuid,
                                std::vector<CUUIDGen> &vSavedAliases);
  void UndoResetAllAliasPasswords(const uuid_array_t &base_uuid,
                                  std::vector<CUUIDGen> &vSavedAliases);

  int DoUpdatePasswordHistory(int iAction, int new_default_max,
                              SavePWHistoryMap &mapSavedHistory);
  void UndoUpdatePasswordHistory(SavePWHistoryMap &mapSavedHistory);

  StringX GetPassKey() const; // returns cleartext - USE WITH CARE
  // Following used by SetPassKey
  void EncryptPassword(const unsigned char *plaintext, int len,
                       unsigned char *ciphertext) const;

  StringX m_currfile; // current pw db filespec

  unsigned char *m_passkey; // encrypted by session key
  unsigned int m_passkey_len; // Length of cleartext passkey

  static unsigned char m_session_key[20];
  static unsigned char m_session_salt[20];
  static unsigned char m_session_initialized;

  HANDLE m_lockFileHandle;
  HANDLE m_lockFileHandle2;
  int m_LockCount;
  int m_LockCount2;

  bool m_usedefuser;
  StringX m_defusername;

  stringT m_AppNameAndVersion;
  PWSfile::VERSION m_ReadFileVersion;

  bool m_bDBChanged;
  bool m_bDBPrefsChanged;
  bool m_IsReadOnly;

  PWSfile::HeaderRecord m_hdr;
  std::vector<bool> m_OrigDisplayStatus;

  // THE password database
  //  Key = entry's uuid; Value = entry's CItemData
  ItemList m_pwlist;

  // Alias/Shortcut structures
  // Permanent Multimap: since potentially more than one alias/shortcut per base
  //  Key = base uuid; Value = multiple alias/shortcut uuids
  ItemMMap m_base2aliases_mmap;
  ItemMMap m_base2shortcuts_mmap;

  // Permanent Map: since an alias only has one base
  //  Key = alias/shortcut uuid; Value = base uuid
  ItemMap m_alias2base_map;
  ItemMap m_shortcut2base_map;

  // Changed groups
  std::vector<StringX> m_vnodes_modified;
  
  UnknownFieldList m_UHFL;
  int m_nRecordsWithUnknownFields;

  // Callback if database has been modified
  void (*m_pfcnNotifyDBModified) (LPARAM, bool);
  LPARAM m_NotifyDBInstance;
  bool m_bNotifyDB;

  // Create header for included(Text) and excluded(XML) exports
  StringX BuildHeader(const CItemData::FieldBits &bsFields, const bool bIncluded);

  // Command list for Undo/Redo
  std::vector<Command *> m_vpcommands;
  std::vector<Command *>::iterator m_undo_iter;
  std::vector<Command *>::iterator m_redo_iter;

  // To notify GUI of an update
  void (*m_pfcnNotifyUpdateGUI) (LPARAM , const Command::GUI_Action &, 
                                 uuid_array_t &, LPARAM );
  LPARAM m_NotifyUpdateGUIInstance;

  // To perform a GUI dependent Command
  void (*m_pfcnGUICommandInterface) (LPARAM , const Command::ExecuteFn &, PWSGUICmdIF *);
  LPARAM m_GUICommandInterfaceInstance;

  static Reporter *m_pReporter; // set as soon as possible to show errors
  static Asker *m_pAsker;
};

#endif /* __PWSCORE_H */
