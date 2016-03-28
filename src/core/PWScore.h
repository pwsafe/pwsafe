/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
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
#include "StringX.h"
#include "PWSfile.h"
#include "PWSFilters.h"
#include "Report.h"
#include "Proxy.h"
#include "UIinterface.h"
#include "Command.h"
#include "CommandInterface.h"
#include "DBCompareData.h"
#include "ExpiredList.h"

#include "coredefs.h"

// Parameter list for ParseBaseEntryPWD
struct BaseEntryParms {
  // All fields except "InputType" are 'output'.
  StringX csPwdGroup;
  StringX csPwdTitle;
  StringX csPwdUser;
  pws_os::CUUID base_uuid;
  CItemData::EntryType InputType;
  CItemData::EntryType TargetType;
  int ibasedata;
  bool bMultipleEntriesFound;
  BaseEntryParms() : base_uuid(pws_os::CUUID::NullUUID()) {}
};

// Formatted Database properties
struct st_DBProperties {
  StringX database;
  StringX databaseformat;
  StringX numgroups;
  StringX numentries;
  StringX numattachments;
  StringX whenlastsaved;
  StringX wholastsaved;
  StringX whatlastsaved;
  StringX file_uuid;
  StringX unknownfields;
  StringX db_name;
  StringX db_description;
};

struct st_ValidateResults;

class PWScore : public CommandInterface
{
public:
  enum {
    SUCCESS = 0,
    FAILURE = 1,
    USER_DECLINED_SAVE = 2,
    CANT_GET_LOCK = 3,
    DB_HAS_CHANGED = 4,
    CANT_OPEN_FILE = PWSfile::CANT_OPEN_FILE, // -10
    USER_CANCEL = -9,                         // -9
    WRONG_PASSWORD = PWSfile::WRONG_PASSWORD, //  5
    BAD_DIGEST = PWSfile::BAD_DIGEST,         //  6
    TRUNCATED_FILE = PWSfile::TRUNCATED_FILE, //  8 (or maybe corrupt?)
    READ_FAIL = PWSfile::READ_FAIL,           //  9
    WRITE_FAIL = PWSfile::WRITE_FAIL,         //  10
    UNKNOWN_VERSION,                          //  11
    NOT_SUCCESS,                              //  12
    ALREADY_OPEN,                             //  13
    INVALID_FORMAT,                           //  14
    USER_EXIT,                                //  15
    XML_FAILED_VALIDATION,                    //  16
    XML_FAILED_IMPORT,                        //  17
    LIMIT_REACHED,                            //  18 - OBSOLETE (for demo)
    UNIMPLEMENTED,                            //  19
    NO_ENTRIES_EXPORTED,                      //  20
    DB_HAS_DUPLICATES,                        //  21
    OK_WITH_ERRORS,                           //  22
    OK_WITH_VALIDATION_ERRORS,                //  23
    OPEN_NODB                                 //  24
  };

  PWScore();
  ~PWScore();

  bool SetUIInterFace(UIInterFace *pUIIF, size_t num_supported,
                      std::bitset<UIInterFace::NUM_SUPPORTED> bsSupportedFunctions);

  // Set following to a Reporter-derived object
  // so that we can inform user of events of interest
  static void SetReporter(Reporter *pReporter) {m_pReporter = pReporter;}
  static void SetAsker(Asker *pAsker) {m_pAsker = pAsker;}
  static bool IsAskerSet() {return m_pAsker != NULL;}
  static bool IsReporterSet() {return m_pReporter != NULL;}

  // Get/Set File UUIDs
  void ClearFileUUID() { m_hdr.m_file_uuid = pws_os::CUUID::NullUUID(); }
  void SetFileUUID(const pws_os::CUUID &fu) { m_hdr.m_file_uuid = fu; }
  const pws_os::CUUID &GetFileUUID() const { return m_hdr.m_file_uuid; }

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

  int ReadCurFile(const StringX &passkey, const bool bValidate = false,
                  const size_t iMAXCHARS = 0, CReport *pRpt = NULL)
  {return ReadFile(m_currfile, passkey, bValidate, iMAXCHARS, pRpt);}
  int ReadFile(const StringX &filename, const StringX &passkey,
               const bool bValidate = false, const size_t iMAXCHARS = 0,
               CReport *pRpt = NULL);
  PWSfile::VERSION GetReadFileVersion() const {return m_ReadFileVersion;}
  bool BackupCurFile(int maxNumIncBackups, int backupSuffix,
                     const stringT &userBackupPrefix,
                     const stringT &userBackupDir, stringT &bu_fname);

  void NewFile(const StringX &passkey);
  int WriteCurFile() {return WriteFile(m_currfile, m_ReadFileVersion);}
  int WriteFile(const StringX &filename, PWSfile::VERSION version,
                bool bUpdateSig = true);
  int WriteExportFile(const StringX &filename, OrderedItemList *pOIL,
                      PWScore *pINcore, PWSfile::VERSION version,
                      CReport *pRpt = NULL);
  int WriteV17File(const StringX &filename)
  {return WriteFile(filename, PWSfile::V17, false);}
  int WriteV2File(const StringX &filename)
  {return WriteFile(filename, PWSfile::V20, false);}

  // R/O file status
  void SetReadOnly(bool state) {m_IsReadOnly = state;}
  bool IsReadOnly() const {return m_IsReadOnly;};

  // Check/Change master passphrase
  int CheckPasskey(const StringX &filename, const StringX &passkey);
  void ChangePasskey(const StringX &newPasskey);
  void SetPassKey(const StringX &new_passkey);

  // Database functions
  int TestSelection(const bool bAdvanced,
                    const stringT &subgroup_name,
                    const int &subgroup_object,
                    const int &subgroup_function,
                    const OrderedItemList *pOIL);

  void Compare(PWScore *pothercore,
               const CItemData::FieldBits &bsFields, const bool &subgroup_bset,
               const bool &bTreatWhiteSpaceasEmpty, const stringT &subgroup_name,
               const int &subgroup_object, const int &subgroup_function,
               CompareData &list_OnlyInCurrent, CompareData &list_OnlyInComp,
               CompareData &list_Conflicts, CompareData &list_Identical,
               bool *pbCancel = NULL);

  stringT Merge(PWScore *pothercore,
                const bool &subgroup_bset,
                const stringT &subgroup_name,
                const int &subgroup_object, const int &subgroup_function,
                CReport *pRpt, bool *pbCancel = NULL);

  void Synchronize(PWScore *pothercore, 
                   const CItemData::FieldBits &bsFields, const bool &subgroup_bset,
                   const stringT &subgroup_name,
                   const int &subgroup_object, const int &subgroup_function,
                   int &numUpdated, CReport *pRpt, bool *pbCancel = NULL);

  // Export databases
  int WritePlaintextFile(const StringX &filename,
                         const CItemData::FieldBits &bsExport,
                         const stringT &subgroup, const int &iObject,
                         const int &iFunction, const TCHAR &delimiter,
                         int &numExported, const OrderedItemList *pOIL = NULL,
                         CReport *pRpt = NULL);

  int WriteXMLFile(const StringX &filename,
                   const CItemData::FieldBits &bsExport,
                   const stringT &subgroup, const int &iObject,
                   const int &iFunction, const TCHAR &delimiter,
                   int &numExported, const OrderedItemList *pOIL = NULL,
                   const bool &bFilterActive = false,
                   CReport *pRpt = NULL);

  // Import databases
  // If returned status is SUCCESS, then returned Command * can be executed.
  int ImportPlaintextFile(const StringX &ImportedPrefix,
                          const StringX &filename,
                          const TCHAR &fieldSeparator, const TCHAR &delimiter,
                          const bool &bImportPSWDsOnly,
                          stringT &strErrors,
                          int &numImported, int &numSkipped,
                          int &numPWHErrors, int &numRenamed, int &numNoPolicy,
                          CReport &rpt, Command *&pcommand);
  int ImportXMLFile(const stringT &ImportedPrefix,
                    const stringT &strXMLFileName,
                    const stringT &strXSDFileName,
                    const bool &bImportPSWDsOnly,
                    stringT &strXMLErrors, stringT &strSkippedList,
                    stringT &strPWHErrorList, stringT &strRenameList,
                    int &numValidated, int &numImported, int &numSkipped,
                    int &numPWHErrors, int &numRenamed,
                    int &numNoPolicy,  int &numRenamedPolicies,
                    int &numShortcutsRemoved,
                    CReport &rpt, Command *&pcommand);
  int ImportKeePassV1TXTFile(const StringX &filename,
                             int &numImported, int &numSkipped, int &numRenamed,
                             UINT &uiReasonCode, CReport &rpt, Command *&pcommand);
  int ImportKeePassV1CSVFile(const StringX &filename,
                             int &numImported, int &numSkipped, int &numRenamed,
                             UINT &uiReasonCode, CReport &rpt, Command *&pcommand);

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
  void GetUniqueGroups(std::vector<stringT> &vUniqueGroups) const;
  // Construct unique title
  StringX GetUniqueTitle(const StringX &group, const StringX &title,
                         const StringX &user, const int IDS_MESSAGE);
  // Get all password policy names
  void GetPolicyNames(std::vector<stringT> &vNames) const;
  bool GetPolicyFromName(const StringX &sxPolicyName, PWPolicy &st_pp) const;
  Command *ProcessPolicyName(PWScore *pothercore, CItemData &updtEntry,
                             std::map<StringX, StringX> &mapRenamedPolicies,
                             std::vector<StringX> &vs_PoliciesAdded,
                             StringX &sxOtherPolicyName, bool &bUpdated,
                             const StringX &sxDateTime, const UINT &IDS_MESSAGE);

  // This routine should only be directly called from XML import
  void MakePolicyUnique(std::map<StringX, StringX> &mapRenamedPolicies,
                        StringX &sxPolicyName, const StringX &sxDateTime,
                        const UINT IDS_MESSAGE);

  bool GetEntriesUsingNamedPasswordPolicy(const StringX sxPolicyName,
                                          std::vector<st_GroupTitleUser> &ventries);

  // Populate setGTU & setUUID from m_pwlist. Returns false & empty set if
  // m_pwlist had one or more entries with same GTU/UUID respectively.
  bool InitialiseGTU(GTUSet &setGTU);
  bool InitialiseGTU(GTUSet &setGTU, const StringX &sxPolicyName);
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
  bool AnyToUndo() const;
  bool AnyToRedo() const;

  // Find in m_pwlist by group, title and user name, exact match
  ItemListIter Find(const StringX &a_group,
                    const StringX &a_title, const StringX &a_user);
  ItemListIter Find(const pws_os::CUUID &entry_uuid)
  {return m_pwlist.find(entry_uuid);}
  ItemListConstIter Find(const pws_os::CUUID &entry_uuid) const
  {return m_pwlist.find(entry_uuid);}

  bool ConfirmDelete(const CItemData *pci); // ask user when about to delete a base,
  //                                           otherwise just return true

  // General routines for aliases and shortcuts
  void GetAllDependentEntries(const pws_os::CUUID &base_uuid,
                              UUIDVector &dependentslist, 
                              const CItemData::EntryType type);
  // Takes apart a 'special' password into its components:
  bool ParseBaseEntryPWD(const StringX &passwd, BaseEntryParms &pl);

  const CItemData *GetBaseEntry(const CItemData *pAliasOrSC) const;
  CItemData *GetBaseEntry(const CItemData *pAliasOrSC);

  // alias/base and shortcut/base handling
  void SortDependents(UUIDVector &dlist, StringX &csDependents);
  size_t NumAliases(const pws_os::CUUID &base_uuid) const
  {return m_base2aliases_mmap.count(base_uuid);}
  size_t NumShortcuts(const pws_os::CUUID &base_uuid) const
  {return m_base2shortcuts_mmap.count(base_uuid);}

  ItemListIter GetUniqueBase(const StringX &title, bool &bMultiple);
  ItemListIter GetUniqueBase(const StringX &grouptitle, 
                             const StringX &titleuser, bool &bMultiple);

  // Use following calls to 'SetChanged' & 'SetDBChanged' sparingly
  // outside of core
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

  bool ChangeMode(stringT &locker, int &iErrorCode);
  PWSFileSig& GetCurrentFileSig() {return *m_pFileSig;}

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
  void UpdateWizard(const stringT &s);

  // Get/Set Display information from/to database
  void SetDisplayStatus(const std::vector<bool> &s);
  const std::vector<bool> &GetDisplayStatus() const;
  bool WasDisplayStatusChanged() const;

  const PWSfileHeader &GetHeader() const {return m_hdr;}
  void SetHeader(const PWSfileHeader &hdr) { m_hdr = hdr; }

  void GetDBProperties(st_DBProperties &st_dbp);
  void SetHeaderUserFields(st_DBProperties &st_dbp);

  StringX &GetDBPreferences() {return m_hdr.m_prefString;}

  // Filters
  PWSFilters m_MapFilters;

  // Changed nodes
  void ClearChangedNodes()
  {m_vnodes_modified.clear();}
  bool IsNodeModified(StringX &path) const;

  void GetRUEList(UUIDList &RUElist)
  {RUElist = m_RUEList;}
  void SetRUEList(const UUIDList &RUElist)
  {m_RUEList = RUElist;}

  size_t GetExpirySize() {return m_ExpireCandidates.size();}
  ExpiredList GetExpired(int idays) {return m_ExpireCandidates.GetExpired(idays);}

  // Yubi support:
  const unsigned char *GetYubiSK() const;
  void SetYubiSK(const unsigned char *);
  
  // Password Policies
  bool IncrementPasswordPolicy(const StringX &sxPolicyName);
  bool DecrementPasswordPolicy(const StringX &sxPolicyName);

  const PSWDPolicyMap &GetPasswordPolicies()
  {return m_MapPSWDPLC;}
  void SetPasswordPolicies(const PSWDPolicyMap &MapPSWDPLC)
  {m_MapPSWDPLC = MapPSWDPLC; SetDBChanged(true);}

  void AddPolicy(const StringX &sxPolicyName, const PWPolicy &st_pp,
                 const bool bAllowReplace = false);

  // Empty Groups
  void SetEmptyGroups(const std::vector<StringX> &vEmptyGroups)
  {m_vEmptyGroups = vEmptyGroups; SetDBChanged(true);}
  const std::vector<StringX> &GetEmptyGroups() {return m_vEmptyGroups;}
  bool IsEmptyGroup(const StringX &sxEmptyGroup);

  // Keyboard shortcuts
  bool AddKBShortcut(const int32 &iKBShortcut, const pws_os::CUUID &uuid);
  bool DelKBShortcut(const int32 &iKBShortcut, const pws_os::CUUID &uuid);
  const pws_os::CUUID & GetKBShortcut(const int32 &iKBShortcut);
  const KBShortcutMap &GetAllKBShortcuts() {return m_KBShortcutMap;}
  void SetAppHotKey(const int32 &iAppHotKey) {m_iAppHotKey = iAppHotKey;}
  int32 GetAppHotKey() const {return m_iAppHotKey;}

  uint32 GetHashIters() const;
  void SetHashIters(uint32 value);

  CItemAtt &GetAtt(const pws_os::CUUID &attuuid) {return m_attlist[attuuid];}
  void PutAtt(const CItemAtt &att) {m_attlist[att.GetUUID()] = att;}
  void RemoveAtt(const pws_os::CUUID &attuuid);
  bool HasAtt(const pws_os::CUUID &attuuid) const {return m_attlist.find(attuuid) != m_attlist.end();}
  AttList::size_type GetNumAtts() const {return m_attlist.size();}
  std::set<StringX> GetAllMediaTypes() const;
  
protected:
  bool m_isAuxCore; // set in c'tor, if true, never update prefs from DB.  

private:
  // Database update routines

  // NOTE: Member functions starting with 'Do' or 'Undo' are meant to
  // be executed ONLY via Command subclasses. These are implementations of
  // the CommandInterface mixin, where they're declared public.
  virtual void DoAddEntry(const CItemData &item, const CItemAtt *att);
  virtual void DoDeleteEntry(const CItemData &item);
  virtual void DoReplaceEntry(const CItemData &old_ci, const CItemData &new_ci);

  // General routines for aliases and shortcuts
  virtual void DoAddDependentEntry(const pws_os::CUUID &base_uuid,
                                   const pws_os::CUUID &entry_uuid,
                                   const CItemData::EntryType type);
  virtual void DoRemoveDependentEntry(const pws_os::CUUID &base_uuid,
                                      const pws_os::CUUID &entry_uuid, 
                                      const CItemData::EntryType type);
  virtual void DoRemoveAllDependentEntries(const pws_os::CUUID &base_uuid, 
                                           const CItemData::EntryType type);
  virtual int DoAddDependentEntries(UUIDVector &dependentslist, CReport *pRpt, 
                                    const CItemData::EntryType type, 
                                    const int &iVia,
                                    ItemList *pmapDeletedItems = NULL,
                                    SaveTypePWMap *pmapSaveTypePW = NULL);
  virtual void UndoAddDependentEntries(ItemList *pmapDeletedItems,
                                       SaveTypePWMap *pmapSaveTypePW);
  virtual void DoMoveDependentEntries(const pws_os::CUUID &from_baseuuid, 
                                      const pws_os::CUUID &to_baseuuid, 
                                      const CItemData::EntryType type);

  virtual int DoUpdatePasswordHistory(int iAction, int new_default_max,
                                      SavePWHistoryMap &mapSavedHistory);
  virtual void UndoUpdatePasswordHistory(SavePWHistoryMap &mapSavedHistory);

  virtual int DoRenameGroup(const StringX &sxOldPath, const StringX &sxNewPath);
  virtual void UndoRenameGroup(const StringX &sxOldPath, const StringX &sxNewPath);

  virtual bool AddEmptyGroup(const StringX &sxEmptyGroup);
  virtual bool RemoveEmptyGroup(const StringX &sxEmptyGroup);
  virtual void RenameEmptyGroup(const StringX &sxOldGroup, const StringX &sxNewGroup);
  virtual void RenameEmptyGroupPaths(const StringX &sxOldPath, const StringX &sxNewPath);

  // End of Command Interface implementations

  void ProcessReadEntry(CItemData &ci_temp,
                        std::vector<st_GroupTitleUser> &vGTU_INVALID_UUID,
                        std::vector<st_GroupTitleUser> &vGTU_DUPLICATE_UUID,
                        st_ValidateResults &st_vr);
  // Validate() returns true if data modified, false if all OK
  bool Validate(const size_t iMAXCHARS, CReport *pRpt, st_ValidateResults &st_vr);

  void ParseDependants(); // populate data structures as needed - called in ReadFile()
  void ResetAllAliasPasswords(const pws_os::CUUID &base_uuid);
  
  StringX GetPassKey() const; // returns cleartext - USE WITH CARE
  // Following used by SetPassKey
  void EncryptPassword(const unsigned char *plaintext, size_t len,
                       unsigned char *ciphertext) const;

  int MergeDependents(PWScore *pothercore, MultiCommands *pmulticmds,
                      uuid_array_t &base_uuid, uuid_array_t &new_base_uuid, 
                      const bool bTitleRenamed, stringT &timeStr, 
                      const CItemData::EntryType et, std::vector<StringX> &vs_added);

  StringX m_currfile; // current pw db filespec

  unsigned char *m_passkey; // encrypted by session key
  size_t m_passkey_len; // Length of cleartext passkey

  uint32 m_hashIters; // for new or currently open db.

  static unsigned char m_session_key[32];
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

  PWSfileHeader m_hdr;
  std::vector<bool> m_OrigDisplayStatus;

  // THE password database
  //  Key = entry's uuid; Value = entry's CItemData
  ItemList m_pwlist;

  // Attachments, if any
  AttList m_attlist;
  
  // Alias/Shortcut structures
  // Permanent Multimap: since potentially more than one alias/shortcut per base
  //  Key = base uuid; Value = multiple alias/shortcut uuids
  ItemMMap m_base2aliases_mmap;
  ItemMMap m_base2shortcuts_mmap;

  // Following are private in PWScore, public in CommandInterface:
  const ItemMMap &GetBase2AliasesMmap() const {return m_base2aliases_mmap;}
  void SetBase2AliasesMmap(ItemMMap &b2amm) {m_base2aliases_mmap = b2amm;}
  const ItemMMap &GetBase2ShortcutsMmap() const {return m_base2shortcuts_mmap;}
  void SetBase2ShortcutsMmap(ItemMMap &b2smm) {m_base2shortcuts_mmap = b2smm;}
  
  // Changed groups
  std::vector<StringX> m_vnodes_modified;
  // Following are private in PWScore, public in CommandInterface:
  virtual const std::vector<StringX> &GetVnodesModified() const
  {return m_vnodes_modified;}
  virtual void SetVnodesModified(const std::vector<StringX> &mvm)
  {m_vnodes_modified = mvm;}
  void AddChangedNodes(StringX path);

  // EmptyGroups
  std::vector<StringX> m_vEmptyGroups;

  UnknownFieldList m_UHFL;
  int m_nRecordsWithUnknownFields;

  UUIDList m_RUEList;

  bool m_bNotifyDB;

  UIInterFace *m_pUIIF; // pointer to UI interface abtraction
  std::bitset<UIInterFace::NUM_SUPPORTED> m_bsSupportedFunctions;
  
  void NotifyGUINeedsUpdating(UpdateGUICommand::GUI_Action, const pws_os::CUUID &,
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
  PWSFileSig *m_pFileSig;

  // Entries with an expiry date
  ExpiredList m_ExpireCandidates;
  void AddExpiryEntry(const CItemData &ci)
  {m_ExpireCandidates.Add(ci);}
  void UpdateExpiryEntry(const CItemData &ci)
  {m_ExpireCandidates.Update(ci);}
  void UpdateExpiryEntry(const pws_os::CUUID &uuid, const CItemData::FieldType ft,
                         const StringX &value);
  void RemoveExpiryEntry(const CItemData &ci)
  {m_ExpireCandidates.Remove(ci);}

  stringT GetXMLPWPolicies(const OrderedItemList *pOIL = NULL);
  PSWDPolicyMap m_MapPSWDPLC;

  KBShortcutMap m_KBShortcutMap;
  int32 m_iAppHotKey;

  // ValidateKBShortcut() returns true if data modified, false if all OK
  bool ValidateKBShortcut(int32 &iKBShortcut);
};

/**
 * Use this for "other" cores that we want concurrently.
 * This class does not update the database preferences
 * when reading a database, so there's no need to save/restore
 * the "main" db prefs.
 */
class PWSAuxCore : public PWScore {
 public:
 PWSAuxCore() : PWScore() {m_isAuxCore = true;}
};

#endif /* __PWSCORE_H */
