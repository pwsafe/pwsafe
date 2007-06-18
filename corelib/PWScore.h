/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
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

#define MAXDEMO 10

typedef std::map<CUUIDGen, CItemData, CUUIDGen::ltuuid> ItemList;
typedef ItemList::iterator ItemListIter;
typedef ItemList::const_iterator ItemListConstIter;

typedef std::vector<CItemData> OrderedItemList;

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

  const CString &GetWhoLastSaved() const {return m_wholastsaved;}
  const CString &GetWhenLastSaved() const {return m_whenlastsaved;}
  const CString &GetWhatLastSaved() const {return m_whatlastsaved;}
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
  void SetFileHashIterations(const int &nITER)
  {m_nITER = nITER;}

  void ClearData();
  void ReInit();
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
  int ImportPlaintextFile(const CMyString &ImportedPrefix, const CMyString &filename, CString &strErrors,
                          TCHAR fieldSeparator, TCHAR delimiter, int &numImported, int &numSkipped);
  int ImportKeePassTextFile(const CMyString &filename);
  int ImportXMLFile(const CString &ImportedPrefix, const CString &strXMLFileName, const CString &strXSDFileName,
                    CString &strErrors, int &numValidated, int &numImported,
                    bool &bBadUnknownFileFields, bool &bBadUnknownRecordFields);
  bool FileExists(const CMyString &filename) const {return PWSfile::FileExists(filename);}
  bool FileExists(const CMyString &filename, bool &bReadOnly) const 
  {return PWSfile::FileExists(filename, bReadOnly);}
  int ReadCurFile(const CMyString &passkey)
  {return ReadFile(m_currfile, passkey);}
  int ReadFile(const CMyString &filename, const CMyString &passkey);
  PWSfile::VERSION GetReadFileVersion() const {return m_ReadFileVersion;}
  unsigned short GetCurrentMajorVersion() const {return m_nCurrentMajorVersion;}
  unsigned short GetCurrentMinorVersion() const {return m_nCurrentMinorVersion;}
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
  void SetApplicationMajorMinor(DWORD dwMajorMinor) {m_dwMajorMinor = dwMajorMinor;}
  void SetReadOnly(bool state) { m_IsReadOnly = state;}
  bool IsReadOnly() const {return m_IsReadOnly;};

  // Return list of unique groups
  void GetUniqueGroups(CStringArray &ary);

  ItemListIter GetEntryIter()
  {return m_pwlist.begin();}
  ItemListConstIter GetEntryIter() const
  {return m_pwlist.begin();}
  ItemListIter GetEntryEndIter()
  {return m_pwlist.end();}
  ItemListConstIter GetEntryEndIter() const
  {return m_pwlist.end();}
  CItemData &GetEntry(ItemListIter iter)
    {return iter->second; }
  const CItemData &GetEntry(ItemListConstIter iter) const
  {return iter->second;}
  void AddEntry(const CItemData &item)
  {uuid_array_t uuid; item.GetUUID(uuid); AddEntry(uuid, item);}
  void AddEntry(const uuid_array_t &uuid, const CItemData &item);
  size_t GetNumEntries() const {return m_pwlist.size();}
  void RemoveEntryAt(ItemListIter pos)
  {m_changed = true; m_pwlist.erase(pos);}
  // Find in m_pwlist by title and user name, exact match
  ItemListIter Find(const CMyString &a_group,
                    const CMyString &a_title, const CMyString &a_user);
  ItemListIter Find(const uuid_array_t &uuid)
  {return m_pwlist.find(uuid);}
  ItemListConstIter Find(const uuid_array_t &uuid) const
  {return m_pwlist.find(uuid);}

  bool IsChanged() const {return m_changed;}
  void SetChanged(bool changed) {m_changed = changed;} // use sparingly...
  void SetPassKey(const CMyString &new_passkey);

  void SetDisplayStatus(const std::vector<bool> &s) { m_displaystatus = s;}
  const std::vector<bool> &GetDisplayStatus() const {return m_displaystatus;}
  void CopyPWList(const ItemList &in);
  // Validate() returns true if data modified, false if all OK
  bool Validate(CString &status);

 private:
  CMyString m_currfile; // current pw db filespec
  unsigned char *m_passkey; // encrypted by session key
  unsigned int m_passkey_len; // Length of cleartext passkey
  static unsigned char m_session_key[20];
  static unsigned char m_session_salt[20];
  static unsigned char m_session_initialized;
  static CString m_hdr;
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
  PWSfile::VERSION m_ReadFileVersion;
  unsigned short m_nCurrentMajorVersion, m_nCurrentMinorVersion;
  DWORD m_dwMajorMinor;

  // the password database
  ItemList m_pwlist;

  bool m_changed;
  bool m_IsReadOnly;

  std::vector<bool> m_displaystatus;
  CString m_wholastsaved, m_whenlastsaved, m_whatlastsaved;
  uuid_array_t m_file_uuid_array;
  int m_nITER;
  UnknownFieldList m_UHFL;
  int m_nRecordsWithUnknownFields;
};
#endif /* __PWSCORE_H */
