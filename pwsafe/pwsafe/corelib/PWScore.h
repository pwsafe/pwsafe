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
#include "MyString.h"
#include "PWSfile.h"
#include "UUIDGen.h"
#include "Report.h"

#define MAXDEMO 10

typedef std::map<CUUIDGen, CItemData, CUUIDGen::ltuuid> ItemList;
typedef ItemList::iterator ItemListIter;
typedef ItemList::const_iterator ItemListConstIter;

typedef std::vector<CItemData> OrderedItemList;

typedef std::vector<CUUIDGen> UUIDList;
typedef UUIDList::iterator UUIDListIter;

typedef std::multimap<CUUIDGen, CUUIDGen, CUUIDGen::ltuuid> ItemMMap;
typedef ItemMMap::iterator ItemMMapIter;
typedef ItemMMap::const_iterator ItemMMapConstIter;
typedef std::pair <CUUIDGen, CUUIDGen> ItemMMap_Pair;

typedef std::map<CUUIDGen, CUUIDGen, CUUIDGen::ltuuid> ItemMap;
typedef ItemMap::iterator ItemMapIter;
typedef ItemMap::const_iterator ItemMapConstIter;
typedef std::pair <CUUIDGen, CUUIDGen> ItemMap_Pair;

// Parameter list for GetBaseEntry
struct GetBaseEntryPL {
  // All fields except "InputType" are 'output'.
  CMyString csPwdGroup;
  CMyString csPwdTitle;
  CMyString csPwdUser;
  uuid_array_t base_uuid;
  CItemData::EntryType InputType;
  CItemData::EntryType TargetType;
  int ibasedata;
  bool bMultipleEntriesFound;
};

class PWScore {
public:

  enum {
    SUCCESS = 0,
    FAILURE = 1,
    CANT_OPEN_FILE = -10,
    USER_CANCEL,								// -9
    WRONG_PASSWORD = PWSfile::WRONG_PASSWORD,	//  6 - ensure the same value
    BAD_DIGEST = PWSfile::BAD_DIGEST,			//  7 - ensure the same value
    UNKNOWN_VERSION,							//  8
    NOT_SUCCESS,								//  9
    ALREADY_OPEN,								// 10
    INVALID_FORMAT,								// 11
    USER_EXIT,									// 12
    XML_FAILED_VALIDATION,						// 13
    XML_FAILED_IMPORT,							// 14
    LIMIT_REACHED                               // 15
  };


  PWScore();
  ~PWScore();

  // Following used to read/write databases
  CMyString GetCurFile() const {return m_currfile;}
  void SetCurFile(const CMyString &file) {m_currfile = file;}
  bool GetUseDefUser() const {return m_usedefuser;}
  void SetUseDefUser(bool v) {m_usedefuser = v;}
  CMyString GetDefUsername() const {return m_defusername;}
  void SetDefUsername(const CMyString &du) {m_defusername = du;}

  void ClearFileUUID();
  void SetFileUUID(uuid_array_t &file_uuid_array);
  void GetFileUUID(uuid_array_t &file_uuid_array);
  bool HasHeaderUnknownFields()
  {return !m_UHFL.empty();}
  int GetNumRecordsWithUnknownFields()
  {return m_nRecordsWithUnknownFields;}
  void SetNumRecordsWithUnknownFields(const int num)
  {m_nRecordsWithUnknownFields = num;}
  void DecrementNumRecordsWithUnknownFields()
  {m_nRecordsWithUnknownFields--;}
  void IncrementNumRecordsWithUnknownFields()
  {m_nRecordsWithUnknownFields++;}

  void ClearData();
  void ReInit(bool bNewfile = false);
  void NewFile(const CMyString &passkey);
  int WriteCurFile() {return WriteFile(m_currfile);}
  int WriteFile(const CMyString &filename, PWSfile::VERSION version = PWSfile::VCURRENT);
  int WriteV17File(const CMyString &filename)
  {return WriteFile(filename, PWSfile::V17);}
  int WriteV2File(const CMyString &filename)
  {return WriteFile(filename, PWSfile::V20);}
  int WritePlaintextFile(const CMyString &filename,
    const CItemData::FieldBits &bsExport,
    const CString &subgroup, const int &iObject,
    const int &iFunction, TCHAR &delimiter,
    const OrderedItemList *il = NULL);
  int WriteXMLFile(const CMyString &filename,
    const CItemData::FieldBits &bsExport,
    const CString &subgroup, const int &iObject,
    const int &iFunction, const TCHAR delimiter,
    const OrderedItemList *il = NULL);
  int ImportPlaintextFile(const CMyString &ImportedPrefix,
    const CMyString &filename, CString &strErrors,
    TCHAR fieldSeparator, TCHAR delimiter,
    int &numImported, int &numSkipped,
    CReport &rpt);
  int ImportKeePassTextFile(const CMyString &filename);
  int ImportXMLFile(const CString &ImportedPrefix,
    const CString &strXMLFileName,
    const CString &strXSDFileName,
    CString &strErrors, int &numValidated, int &numImported,
    bool &bBadUnknownFileFields,
    bool &bBadUnknownRecordFields, CReport &rpt);
  bool FileExists(const CMyString &filename) const {return PWSfile::FileExists(filename);}
  bool FileExists(const CMyString &filename, bool &bReadOnly) const 
  {return PWSfile::FileExists(filename, bReadOnly);}
  int ReadCurFile(const CMyString &passkey)
  {return ReadFile(m_currfile, passkey);}
  int ReadFile(const CMyString &filename, const CMyString &passkey);
  PWSfile::VERSION GetReadFileVersion() const {return m_ReadFileVersion;}
  int RenameFile(const CMyString &oldname, const CMyString &newname);
  bool BackupCurFile(int maxNumIncBackups, int backupSuffix,
    const CString &userBackupPrefix, const CString &userBackupDir);
  int CheckPassword(const CMyString &filename, CMyString &passkey);
  void ChangePassword(const CMyString & newPassword);
  bool LockFile(const CMyString &filename, CMyString &locker)
  {return PWSfile::LockFile(filename, locker,
  m_lockFileHandle, m_LockCount);}
  bool IsLockedFile(const CMyString &filename) const
  {return PWSfile::IsLockedFile(filename);}
  void UnlockFile(const CMyString &filename)
  {return PWSfile::UnlockFile(filename, 
  m_lockFileHandle, m_LockCount);}
  void SetApplicationNameAndVersion(const CString &appName, DWORD dwMajorMinor);
  void SetReadOnly(bool state) { m_IsReadOnly = state;}
  bool IsReadOnly() const {return m_IsReadOnly;};

  // Return list of unique groups
  void GetUniqueGroups(CStringArray &ary);
  CMyString GetUniqueTitle(const CMyString &path, const CMyString &title,
    const CMyString &user, const int IDS_MESSAGE);

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
  ItemListIter Find(const CMyString &a_group,
    const CMyString &a_title, const CMyString &a_user);
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
  bool GetBaseEntry(const CMyString &Password, GetBaseEntryPL &pl);

  // Actions for Aliases only
  void ResetAllAliasPasswords(const uuid_array_t &base_uuid);

  // Actions relating to alias/base and shortcut/base maps
  void AddAliasBaseEntry(const uuid_array_t &alias_uuid, const uuid_array_t &base_uuid)
  {m_alias2base_map[alias_uuid] = base_uuid;}
  void AddShortcutBaseEntry(const uuid_array_t &shortcut_uuid, const uuid_array_t &base_uuid)
  {m_shortcut2base_map[shortcut_uuid] = base_uuid;}
  bool GetAliasBaseUUID(const uuid_array_t &alias_uuid, uuid_array_t &base_uuid)
  {return GetDependentEntryBaseUUID(alias_uuid, base_uuid, CItemData::Alias);}
  bool GetShortcutBaseUUID(const uuid_array_t &shortcut_uuid, uuid_array_t &base_uuid)
  {return GetDependentEntryBaseUUID(shortcut_uuid, base_uuid, CItemData::Shortcut);}
  void SetAliasBaseUUID(const uuid_array_t &alias_uuid, uuid_array_t &base_uuid)
  {m_alias2base_map[alias_uuid] = base_uuid;}
  void SetShortcutBaseUUID(const uuid_array_t &shortcut_uuid, uuid_array_t &base_uuid)
  {m_shortcut2base_map[shortcut_uuid] = base_uuid;}
  int NumAliases(const uuid_array_t &base_uuid)
  {return m_base2aliases_mmap.count(base_uuid);}
  int NumShortcuts(const uuid_array_t &base_uuid)
  {return m_base2shortcuts_mmap.count(base_uuid);}

  ItemListIter GetUniqueBase(const CMyString &title, bool &bMultiple);
  ItemListIter GetUniqueBase(const CMyString &grouptitle, 
    const CMyString &titleuser, bool &bMultiple);

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

  void SetPassKey(const CMyString &new_passkey);

  void SetDisplayStatus(const std::vector<bool> &s);
  const std::vector<bool> &GetDisplayStatus() const;
  bool WasDisplayStatusChanged() const;
  void CopyPWList(const ItemList &in);
  // Validate() returns true if data modified, false if all OK
  bool Validate(CString &status);
  const PWSfile::HeaderRecord &GetHeader() const {return m_hdr;}

private:
  CMyString m_currfile; // current pw db filespec
  unsigned char *m_passkey; // encrypted by session key
  unsigned int m_passkey_len; // Length of cleartext passkey
  static unsigned char m_session_key[20];
  static unsigned char m_session_salt[20];
  static unsigned char m_session_initialized;
  static CString m_impexphdr;
  HANDLE m_lockFileHandle;
  int m_LockCount;

  CMyString GetPassKey() const; // returns cleartext - USE WITH CARE
  // Following used by SetPassKey
  void EncryptPassword(const unsigned char *plaintext, int len,
    unsigned char *ciphertext) const;
  BOOL GetIncBackupFileName(const CString &cs_filenamebase,
    int i_maxnumincbackups, CString &cs_newname);

  bool m_usedefuser;
  CMyString m_defusername;
  CString m_AppNameAndVersion;

  PWSfile::VERSION m_ReadFileVersion;
  PWSfile::HeaderRecord m_hdr;
  std::vector<bool> m_OrigDisplayStatus;

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

  // List of possible aliases/shortcuts created during reading a database, 
  // importing text or XML, or OnDrop during D&D - needs to be confirmed 
  // that base exists after operation complete - then cleared.
  // UUIDList possible_aliases; - NOW a local variable

  bool m_changed;
  bool m_IsReadOnly;

  UnknownFieldList m_UHFL;
  int m_nRecordsWithUnknownFields;

  // Call back if password list has been modified
  void (*m_pfcnNotifyListModified) (LPARAM);
  LPARAM m_NotifyInstance;
  bool m_bNotify;
};
#endif /* __PWSCORE_H */
