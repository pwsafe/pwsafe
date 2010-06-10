/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "UIinterface.h"
#include "Command.h"
#include "CommandInterface.h"

#include "coredefs.h"

#define MAXDEMO 10

// Parameter list for ParseBaseEntryPWD
struct BaseEntryParms {
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

// Formatted Database properties
struct st_DBProperties {
  StringX database;
  StringX databaseformat;
  StringX numgroups;
  StringX numentries;
  StringX whenlastsaved;
  StringX wholastsaved;
  StringX whatlastsaved;
  StringX file_uuid;
  StringX unknownfields;
};

class PWScore : public CommandInterface
{
public:
  enum {
    SUCCESS = 0,
    FAILURE = 1,
    CANT_OPEN_FILE = PWSfile::CANT_OPEN_FILE, // -10 - ensure the same value
    USER_CANCEL,                              // -9
    WRONG_PASSWORD = PWSfile::WRONG_PASSWORD, //  5 - ensure the same value
    BAD_DIGEST = PWSfile::BAD_DIGEST,         //  6 - ensure the same value
    UNKNOWN_VERSION,                          //  7
    NOT_SUCCESS,                              //  8
    ALREADY_OPEN,                             //  9
    INVALID_FORMAT,                           // 10
    USER_EXIT,                                // 11
    XML_FAILED_VALIDATION,                    // 12
    XML_FAILED_IMPORT,                        // 13
    LIMIT_REACHED,                            // 14
    UNIMPLEMENTED,                            // 15
    NO_ENTRIES_EXPORTED,                      // 16
    DB_HAS_DUPLICATES,                        // 17
    OK_WITH_ERRORS,                           // 18
  };

  PWScore();
  ~PWScore();

  static uuid_array_t NULL_UUID;

  void SetUIInterFace(UIInterFace *pUIIF) {m_pUIIF = pUIIF;}
  // Set following to a Reporter-derived object
  // so that we can inform user of events of interest
  static void SetReporter(Reporter *pReporter) {m_pReporter = pReporter;}
  static void SetAsker(Asker *pAsker) {m_pAsker = pAsker;}
  static bool IsAskerSet() {return m_pAsker != NULL;}
  static bool IsReporterSet() {return m_pReporter != NULL;}

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

  int ReadCurFile(const StringX &passkey, const size_t iMAXCHARS = 0)
  {return ReadFile(m_currfile, passkey, iMAXCHARS);}
  int ReadFile(const StringX &filename, const StringX &passkey, const size_t iMAXCHARS = 0);
  PWSfile::VERSION GetReadFileVersion() const {return m_ReadFileVersion;}
  bool BackupCurFile(int maxNumIncBackups, int backupSuffix,
                     const stringT &userBackupPrefix,
                     const stringT &userBackupDir, stringT &bu_fname);

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
  // If returned status is SUCCESS, then returned Command * can be executed.
  int ImportPlaintextFile(const StringX &ImportedPrefix,
                          const StringX &filename,
                          const TCHAR &fieldSeparator, const TCHAR &delimiter,
                          const bool &bImportPSWDsOnly,
                          stringT &strErrors,
                          int &numImported, int &numSkipped,
                          int &numPWHErrors, int &numRenamed,
                          CReport &rpt, Command *&pcommand);
  int ImportXMLFile(const stringT &ImportedPrefix,
                    const stringT &strXMLFileName,
                    const stringT &strXSDFileName,
                    const bool &bImportPSWDsOnly,
                    stringT &strXMLErrors, stringT &strSkippedList,
                    stringT &strPWHErrorList, stringT &strRenameList, 
                    int &numValidated, int &numImported, int &numSkipped,
                    int &numPWHErrors, int &numRenamed, 
                    bool &bBadUnknownFileFields,
                    bool &bBadUnknownRecordFields,
                    CReport &rpt, Command *&pcommand);
  int ImportKeePassTextFile(const StringX &filename, Command *&pcommand);

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
  StringX GetUniqueTitle(const StringX &group, const StringX &title,
                         const StringX &user, const int IDS_MESSAGE);

  // Populate setGTU & setUUID from m_pwlist. Returns false & empty set if
  // m_pwlist had one or more entries with same GTU/UUID respectively.
  bool InitialiseGTU(GTUSet &setGTU);
  bool InitialiseUUID(UUIDSet &setUUID);
  // Adds an st_GroupTitleUser to setGTU, possible modifying title
  // to ensure uniqueness. Returns false if title was modified.
  bool MakeEntryUnique(GTUSet &setGTU, const StringX &group, StringX &title,
                       const StringX &user, const int IDS_MESSAGE);
  void SetUniqueGTUValidated(bool bState)
  {m_bUniqueGTUValidated = bState;}
  bool GetUniqueGTUValidated() const
  {return m_bUniqueGTUValidated;}

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

  // Find in m_pwlist by group, title and user name, exact match
  ItemListIter Find(const StringX &a_group,
                    const StringX &a_title, const StringX &a_user);
  ItemListIter Find(const uuid_array_t &entry_uuid)
  {return m_pwlist.find(entry_uuid);}
  ItemListConstIter Find(const uuid_array_t &entry_uuid) const
  {return m_pwlist.find(entry_uuid);}

  bool ConfirmDelete(const CItemData *pci); // ask user when about to delete a base,
  //                                           otherwise just return true

  // General routines for aliases and shortcuts
  void GetAllDependentEntries(const uuid_array_t &base_uuid,
                              UUIDVector &dependentslist, 
                              const CItemData::EntryType type);
  bool GetDependentEntryBaseUUID(const uuid_array_t &entry_uuid,
                                 uuid_array_t &base_uuid, 
                                 const CItemData::EntryType type) const;
  // Takes apart a 'special' password into its components:
  bool ParseBaseEntryPWD(const StringX &passwd, BaseEntryParms &pl);

  const CItemData *GetBaseEntry(const CItemData *pAliasOrSC) const;
  CItemData *GetBaseEntry(const CItemData *pAliasOrSC);

  // alias/base and shortcut/base handling
  void SortDependents(UUIDVector &dlist, StringX &csDependents);
  int NumAliases(const uuid_array_t &base_uuid) const
  {return (int)m_base2aliases_mmap.count(base_uuid);}
  int NumShortcuts(const uuid_array_t &base_uuid) const
  {return (int)m_base2shortcuts_mmap.count(base_uuid);}

  ItemListIter GetUniqueBase(const StringX &title, bool &bMultiple);
  ItemListIter GetUniqueBase(const StringX &grouptitle, 
                             const StringX &titleuser, bool &bMultiple);

  // Use following calls to 'SetChanged' & 'SetDBChanged' sparingly
  // outside of corelib
  void SetChanged(const bool bDBChanged, const bool bDBprefschanged)
  {m_bDBChanged = bDBChanged; 
   m_bDBPrefsChanged = bDBprefschanged;
   NotifyDBModified();}
  void SetDBChanged(bool bDBChanged, bool bNotify = true)
  {m_bDBChanged = bDBChanged;
    if (bNotify) NotifyDBModified();}
  void SetDBPrefsChanged(bool bDBprefschanged)
  {m_bDBPrefsChanged = bDBprefschanged;
   NotifyDBModified();}

  bool IsChanged() const {return m_bDBChanged;}
  bool HaveDBPrefsChanged() const {return m_bDBPrefsChanged;}
  bool HaveHeaderPreferencesChanged(const StringX &prefString)
  {return _tcscmp(prefString.c_str(), m_hdr.m_prefString.c_str()) != 0;}

  // Callback to be notified if the database changes
  void NotifyDBModified();
  void SuspendOnDBNotification()
  {m_bNotifyDB = false;}
  void ResumeOnDBNotification()
  {m_bNotifyDB = true;}

  void GUISetupDisplayInfo(CItemData &ci);
  void GUIRefreshEntry(const CItemData &ci);

  // Get/Set Display information from/to database
  void SetDisplayStatus(const std::vector<bool> &s);
  const std::vector<bool> &GetDisplayStatus() const;
  bool WasDisplayStatusChanged() const;

  void CopyPWList(const ItemList &in);

  // Validate() returns true if data modified, false if all OK
  bool Validate(stringT &status, CReport &rpt, const size_t iMAXCHARS = 0);
  const PWSfile::HeaderRecord &GetHeader() const {return m_hdr;}
  void GetDBProperties(st_DBProperties &st_dbp);
  StringX &GetDBPreferences() {return m_hdr.m_prefString;}

  // Filters
  PWSFilters m_MapFilters;

  // Changed nodes
  void ClearChangedNodes()
  {m_vnodes_modified.clear();}
  bool IsNodeModified(StringX &path) const;

  void GetRUEList(UUIDList &RUElist)
  {RUElist = m_RUEList;}
  void SetRUEList(const UUIDList RUElist)
  {m_RUEList = RUElist;}

private:
  // Database update routines

  // NOTE: Member functions starting with 'Do' or 'Undo' are meant to
  // be executed ONLY via Command subclasses. These are implementations of
  // the CommandInterface mixin, where they're declared public.
  virtual void DoAddEntry(const CItemData &item);
  virtual void DoDeleteEntry(const CItemData &item);
  virtual void DoReplaceEntry(const CItemData &old_ci, const CItemData &new_ci);

  // General routines for aliases and shortcuts
  virtual void DoAddDependentEntry(const uuid_array_t &base_uuid,
                                   const uuid_array_t &entry_uuid,
                                   const CItemData::EntryType type);
  virtual void DoRemoveDependentEntry(const uuid_array_t &base_uuid,
                                      const uuid_array_t &entry_uuid, 
                                      const CItemData::EntryType type);
  virtual void DoRemoveAllDependentEntries(const uuid_array_t &base_uuid, 
                                           const CItemData::EntryType type);
  virtual int DoAddDependentEntries(UUIDVector &dependentslist, CReport *pRpt, 
                                    const CItemData::EntryType type, 
                                    const int &iVia,
                                    ItemList *pmapDeletedItems = NULL,
                                    SaveTypePWMap *pmapSaveTypePW = NULL);
  virtual void UndoAddDependentEntries(ItemList *pmapDeletedItems,
                                       SaveTypePWMap *pmapSaveTypePW);
  virtual void DoMoveDependentEntries(const uuid_array_t &from_baseuuid, 
                                      const uuid_array_t &to_baseuuid, 
                                      const CItemData::EntryType type);

  virtual int DoUpdatePasswordHistory(int iAction, int new_default_max,
                                      SavePWHistoryMap &mapSavedHistory);
  virtual void UndoUpdatePasswordHistory(SavePWHistoryMap &mapSavedHistory);

  // End of Command Interface implementations
  void ResetAllAliasPasswords(const uuid_array_t &base_uuid);
  
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

  stringT m_AppNameAndVersion;
  PWSfile::VERSION m_ReadFileVersion;

  bool m_bDBChanged;
  bool m_bDBPrefsChanged;
  bool m_IsReadOnly;
  bool m_bUniqueGTUValidated;

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
  // Following are private in PWScore, public in CommandInterface:
  const ItemMMap &GetBase2AliasesMmap() const {return m_base2aliases_mmap;}
  void SetBase2AliasesMmap(ItemMMap &b2amm) {m_base2aliases_mmap = b2amm;}
  const ItemMMap &GetBase2ShortcutsMmap() const {return m_base2shortcuts_mmap;}
  void SetBase2ShortcutsMmap(ItemMMap &b2smm) {m_base2shortcuts_mmap = b2smm;}
  const ItemMap &GetAlias2BaseMap() const {return m_alias2base_map;}
  void SetAlias2BaseMap(const ItemMap &a2bm) {m_alias2base_map = a2bm;}
  const ItemMap &GetShortcuts2BaseMap() const {return m_shortcut2base_map;}
  void SetShortcuts2BaseMap(const ItemMap &s2bm) {m_shortcut2base_map = s2bm;}
  
  // Changed groups
  std::vector<StringX> m_vnodes_modified;
  // Following are private in PWScore, public in CommandInterface:
  virtual const std::vector<StringX> &GetVnodesModified() const
  {return m_vnodes_modified;}
  virtual void SetVnodesModified(const std::vector<StringX> &mvm)
  {m_vnodes_modified = mvm;}
  void AddChangedNodes(StringX path);

  UnknownFieldList m_UHFL;
  int m_nRecordsWithUnknownFields;

  UUIDList m_RUEList;

  bool m_bNotifyDB;

  UIInterFace *m_pUIIF; // pointer to UI interface abtraction
  
  void NotifyGUINeedsUpdating(UpdateGUICommand::GUI_Action, uuid_array_t &,
                              CItemData::FieldType ft = CItemData::START,
                              bool bUpdateGUI = true);

  // Create header for included(Text) and excluded(XML) exports
  StringX BuildHeader(const CItemData::FieldBits &bsFields, const bool bIncluded);

  // Command list for Undo/Redo
  std::vector<Command *> m_vpcommands;
  std::vector<Command *>::iterator m_undo_iter;
  std::vector<Command *>::iterator m_redo_iter;

  static Reporter *m_pReporter; // set as soon as possible to show errors
  static Asker *m_pAsker;
  PWSFileSig *m_fileSig;
};

#endif /* __PWSCORE_H */
