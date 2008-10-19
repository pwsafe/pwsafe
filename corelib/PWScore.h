/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __PWSCORE_H
#define __PWSCORE_H

// PWScore.h
//-----------------------------------------------------------------------------

#include <map> // for CList
#include "ItemData.h"
#include "StringX.h"
#include "PWSfile.h"
#include "PWSFilters.h"
#include "UUIDGen.h"
#include "Report.h"
#include "Proxy.h"

#define MAXDEMO 10

typedef std::map<CUUIDGen, CItemData, CUUIDGen::ltuuid> ItemList;
typedef ItemList::iterator ItemListIter;
typedef ItemList::const_iterator ItemListConstIter;

typedef std::vector<CItemData> OrderedItemList;

typedef std::vector<CUUIDGen> UUIDList;
typedef UUIDList::iterator UUIDListIter;

typedef std::multimap<CUUIDGen, CUUIDGen, CUUIDGen::ltuuid> ItemMMap;

typedef std::map<CUUIDGen, CUUIDGen, CUUIDGen::ltuuid> ItemMap;

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
  };

  PWScore();
  ~PWScore();

  // Set following to a Reporter-derived object
  // so that we can inform user of events of interest
  static void SetReporter(Reporter *reporter) {m_Reporter = reporter;}

  // Following used to read/write databases
  StringX GetCurFile() const {return m_currfile;}
  void SetCurFile(const StringX &file) {m_currfile = file;}
  bool GetUseDefUser() const {return m_usedefuser;}
  void SetUseDefUser(bool v) {m_usedefuser = v;}
  StringX GetDefUsername() const {return m_defusername;}
  void SetDefUsername(const StringX &du) {m_defusername = du;}

  void ClearFileUUID();
  void SetFileUUID(uuid_array_t &file_uuid_array);
  void GetFileUUID(uuid_array_t &file_uuid_array) const;
  bool HasHeaderUnknownFields() const
  {return !m_UHFL.empty();}
  int GetNumRecordsWithUnknownFields() const
  {return m_nRecordsWithUnknownFields;}
  void SetNumRecordsWithUnknownFields(const int num)
  {m_nRecordsWithUnknownFields = num;}
  void DecrementNumRecordsWithUnknownFields()
  {m_nRecordsWithUnknownFields--;}
  void IncrementNumRecordsWithUnknownFields()
  {m_nRecordsWithUnknownFields++;}

  void ClearData();
  void ReInit(bool bNewfile = false);
  void NewFile(const StringX &passkey);
  int WriteCurFile() {return WriteFile(m_currfile);}
  int WriteFile(const StringX &filename, PWSfile::VERSION version = PWSfile::VCURRENT);
  int WriteV17File(const StringX &filename)
  {return WriteFile(filename, PWSfile::V17);}
  int WriteV2File(const StringX &filename)
  {return WriteFile(filename, PWSfile::V20);}
  int WritePlaintextFile(const StringX &filename,
                         const CItemData::FieldBits &bsExport,
                         const stringT &subgroup, const int &iObject,
                         const int &iFunction, TCHAR &delimiter,
                         const OrderedItemList *il = NULL);
  int WriteXMLFile(const StringX &filename,
                   const CItemData::FieldBits &bsExport,
                   const stringT &subgroup, const int &iObject,
                   const int &iFunction, const TCHAR delimiter,
                   const OrderedItemList *il = NULL);
  int ImportPlaintextFile(const StringX &ImportedPrefix,
                          const StringX &filename, stringT &strErrors,
                          TCHAR fieldSeparator, TCHAR delimiter,
                          int &numImported, int &numSkipped,
                          CReport &rpt);
  int ImportKeePassTextFile(const StringX &filename);
  int ImportXMLFile(const stringT &ImportedPrefix,
                    const stringT &strXMLFileName,
                    const stringT &strXSDFileName,
                    stringT &strErrors, int &numValidated, int &numImported,
                    bool &bBadUnknownFileFields,
                    bool &bBadUnknownRecordFields, CReport &rpt);
  bool FileExists(const stringT &filename) const;
  bool FileExists(const stringT &filename, bool &bReadOnly) const; 
  int ReadCurFile(const StringX &passkey)
  {return ReadFile(m_currfile, passkey);}
  int ReadFile(const StringX &filename, const StringX &passkey);
  PWSfile::VERSION GetReadFileVersion() const {return m_ReadFileVersion;}
  int RenameFile(const StringX &oldname, const StringX &newname);
  bool BackupCurFile(int maxNumIncBackups, int backupSuffix,
                     const stringT &userBackupPrefix,
                     const stringT &userBackupDir);
  int CheckPassword(const StringX &filename, const StringX &passkey);
  void ChangePassword(const StringX &newPassword);
  
  bool LockFile(const stringT &filename, stringT &locker);
  bool IsLockedFile(const stringT &filename) const;
  void UnlockFile(const stringT &filename);
  // Following 3 routines only for SaveAs to use a temporary lock handle
  // LockFile2, UnLockFile2 & MoveLock
  bool LockFile2(const stringT &filename, stringT &locker);
  void UnlockFile2(const stringT &filename);
  void MoveLock()
  {m_lockFileHandle = m_lockFileHandle2; m_lockFileHandle2 = INVALID_HANDLE_VALUE;}
  
  void SetApplicationNameAndVersion(const stringT &appName, DWORD dwMajorMinor);
  void SetReadOnly(bool state) { m_IsReadOnly = state;}
  bool IsReadOnly() const {return m_IsReadOnly;};

  // Return list of unique groups
  void GetUniqueGroups(std::vector<stringT> &ary) const;
  StringX GetUniqueTitle(const StringX &path, const StringX &title,
                         const StringX &user, const int IDS_MESSAGE);

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
  void AddEntry(const CItemData &item)
  {uuid_array_t entry_uuid; item.GetUUID(entry_uuid); AddEntry(entry_uuid, item);}
  void AddEntry(const uuid_array_t &entry_uuid, const CItemData &item);
  ItemList::size_type GetNumEntries() const {return m_pwlist.size();}
  void RemoveEntryAt(ItemListIter pos)
  {m_changed = true; NotifyListModified(); m_pwlist.erase(pos);}
  // Find in m_pwlist by title and user name, exact match
  ItemListIter Find(const StringX &a_group,
                    const StringX &a_title, const StringX &a_user);
  ItemListIter Find(const uuid_array_t &entry_uuid)
  {return m_pwlist.find(entry_uuid);}
  ItemListConstIter Find(const uuid_array_t &entry_uuid) const
  {return m_pwlist.find(entry_uuid);}

  // General routines for aliases and shortcuts
  void AddDependentEntry(const uuid_array_t &base_uuid, const uuid_array_t &entry_uuid,
                         const CItemData::EntryType type);
  void RemoveDependentEntry(const uuid_array_t &base_uuid, const uuid_array_t &entry_uuid, 
                            const CItemData::EntryType type);
  void RemoveAllDependentEntries(const uuid_array_t &base_uuid, 
                                 const CItemData::EntryType type);
  void GetAllDependentEntries(const uuid_array_t &base_uuid, UUIDList &dependentslist, 
                              const CItemData::EntryType type);
  void MoveDependentEntries(const uuid_array_t &from_baseuuid, 
                            const uuid_array_t &to_baseuuid, 
                            const CItemData::EntryType type);
  int  AddDependentEntries(UUIDList &dependentslist, CReport *rpt, 
                           const CItemData::EntryType type, 
                           const int &iVia);
  bool GetDependentEntryBaseUUID(const uuid_array_t &entry_uuid, uuid_array_t &base_uuid, 
                                 const CItemData::EntryType type);
  bool GetBaseEntry(const StringX &passwd, GetBaseEntryPL &pl);

  // Actions for Aliases only
  void ResetAllAliasPasswords(const uuid_array_t &base_uuid);

  // Actions relating to alias/base and shortcut/base maps
  void AddAliasBaseEntry(const uuid_array_t &alias_uuid, const uuid_array_t &base_uuid)
  {m_alias2base_map[alias_uuid] = base_uuid;}
  void AddShortcutBaseEntry(const uuid_array_t &shortcut_uuid, const uuid_array_t &base_uuid)
  {m_shortcut2base_map[shortcut_uuid] = base_uuid;}
  bool GetAliasBaseUUID(const uuid_array_t &alias_uuid, uuid_array_t &base_uuid)
  {return GetDependentEntryBaseUUID(alias_uuid, base_uuid, CItemData::ET_ALIAS);}
  bool GetShortcutBaseUUID(const uuid_array_t &shortcut_uuid, uuid_array_t &base_uuid)
  {return GetDependentEntryBaseUUID(shortcut_uuid, base_uuid, CItemData::ET_SHORTCUT);}
  void SetAliasBaseUUID(const uuid_array_t &alias_uuid, uuid_array_t &base_uuid)
  {m_alias2base_map[alias_uuid] = base_uuid;}
  void SetShortcutBaseUUID(const uuid_array_t &shortcut_uuid, uuid_array_t &base_uuid)
  {m_shortcut2base_map[shortcut_uuid] = base_uuid;}
  int NumAliases(const uuid_array_t &base_uuid)
  {return m_base2aliases_mmap.count(base_uuid);}
  int NumShortcuts(const uuid_array_t &base_uuid)
  {return m_base2shortcuts_mmap.count(base_uuid);}

  ItemListIter GetUniqueBase(const StringX &title, bool &bMultiple);
  ItemListIter GetUniqueBase(const StringX &grouptitle, 
                             const StringX &titleuser, bool &bMultiple);

  bool IsChanged() const {return m_changed;}
  void SetChanged(bool changed) {m_changed = changed;} // use sparingly...

  // (Un)Register to be notified if the password list changes
  bool RegisterOnListModified(void (*pfcn) (LPARAM), LPARAM);
  void UnRegisterOnListModified();
  void NotifyListModified();
  void SuspendOnListNotification()
  {m_bNotify = false;}
  void ResumeOnListNotification()
  {m_bNotify = true;}

  void SetPassKey(const StringX &new_passkey);

  void SetDisplayStatus(const std::vector<bool> &s);
  const std::vector<bool> &GetDisplayStatus() const;
  bool WasDisplayStatusChanged() const;
  void CopyPWList(const ItemList &in);
  // Validate() returns true if data modified, false if all OK
  bool Validate(stringT &status);
  const PWSfile::HeaderRecord &GetHeader() const {return m_hdr;}
  void SetAsker(Asker *asker) {m_asker = asker;}
  
  // Filters
  PWSFilters m_MapFilters;

private:
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
  bool m_usedefuser;
  StringX m_defusername;
  stringT m_AppNameAndVersion;
  PWSfile::VERSION m_ReadFileVersion;
  bool m_changed;
  bool m_IsReadOnly;
  PWSfile::HeaderRecord m_hdr;
  std::vector<bool> m_OrigDisplayStatus;
  static Reporter *m_Reporter; // set as soon as possible to show errors

  // THE password database
  //  Key = entry's uuid; Value = entry's CItemData
  ItemList m_pwlist;

  // Alias structures
  // Permanent Multimap: since potentially more than one alias/shortcut per base
  //  Key = base uuid; Value = multiple alias/shortcut uuids
  ItemMMap m_base2aliases_mmap;
  ItemMMap m_base2shortcuts_mmap;

  // Permanent Map: since an alias only has one base
  //  Key = alias/shortcut uuid; Value = base uuid
  ItemMap m_alias2base_map;
  ItemMap m_shortcut2base_map;
  
  UnknownFieldList m_UHFL;
  int m_nRecordsWithUnknownFields;

  // Callback if password list has been modified
  void (*m_pfcnNotifyListModified) (LPARAM);
  LPARAM m_NotifyInstance;
  bool m_bNotify;
  Asker *m_asker;
};
#endif /* __PWSCORE_H */
