// PWScore.h
//-----------------------------------------------------------------------------

#pragma once

#include <afxtempl.h> // for CList
#include "ItemData.h"
#include "MyString.h"
#include "PWSfile.h"

#include <vector>
#include <bitset>

typedef CList<CItemData,CItemData> ItemList;

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
    XML_FAILED_IMPORT							// 14
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

  const CString &GetCurrentUser() const {return m_user;}
  const CString &GetCurrentHost() const {return m_sysname;}
  const CString &GetWhoLastSaved() const {return m_wholastsaved;}
  const CString &GetWhenLastSaved() const {return m_whenlastsaved;}
  const CString &GetWhatLastSaved() const {return m_whatlastsaved;}

  void ClearData();
  void NewFile(const CMyString &passkey);
  int WriteCurFile() {return WriteFile(m_currfile);}
  int WriteFile(const CMyString &filename, PWSfile::VERSION version = PWSfile::VCURRENT);
  int WriteV17File(const CMyString &filename)
    {return WriteFile(filename, PWSfile::V17);}
  int WriteV2File(const CMyString &filename)
    {return WriteFile(filename, PWSfile::V20);}
  int WritePlaintextFile(const CMyString &filename,
                         const bool &bwrite_header,
                         const std::bitset<16> &bsExport,
                         const CString &subgroup, const int &iObject,
                         const int &iFunction, TCHAR &delimiter,
                         const ItemList *il = NULL);
  int WriteXMLFile(const CMyString &filename, const TCHAR delimiter,
                   const ItemList *il = NULL);
  int ImportPlaintextFile(const CMyString &ImportedPrefix, const CMyString &filename, CString &strErrors,
			TCHAR fieldSeparator, TCHAR delimiter, int &numImported, int &numSkipped, bool bimport_preV3);
  int ImportKeePassTextFile(const CMyString &filename);
  int ImportXMLFile(const CString &ImportedPrefix, const CString &strXMLFileName, const CString &strXSDFileName,
			CString &strErrors, int &numValidated, int &numImported);
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
  int BackupCurFile();
  int CheckPassword(const CMyString &filename, CMyString &passkey);
  void ChangePassword(const CMyString & newPassword);
  bool LockFile(const CMyString &filename, CMyString &locker, const bool bDB = true);
  bool IsLockedFile(const CMyString &filename, const bool bDB = true) const;
  void UnlockFile(const CMyString &filename, const bool bDB = true);

  // Return list of unique groups
  void GetUniqueGroups(CStringArray &ary);

  POSITION GetFirstEntryPosition() const
    {return m_pwlist.GetHeadPosition();}
  POSITION AddEntryToTail(const CItemData &item)
    {m_changed = true; return m_pwlist.AddTail(item);}
  CItemData GetEntryAt(POSITION pos) const
    {return m_pwlist.GetAt(pos);}
  CItemData &GetEntryAt(POSITION pos)
    {return m_pwlist.GetAt(pos);}
  CItemData GetNextEntry(POSITION &pos) const
    {return m_pwlist.GetNext(pos);}
  CItemData &GetNextEntry(POSITION &pos)
    {return m_pwlist.GetNext(pos);}
  CItemData &GetTailEntry()
    {return m_pwlist.GetTail();}
  int GetNumEntries() const {return m_pwlist.GetCount();}
  void RemoveEntryAt(POSITION pos)
    {m_changed = true; m_pwlist.RemoveAt(pos);}
 // Find in m_pwlist by title and user name, exact match
  POSITION Find(const CMyString &a_group,
		const CMyString &a_title, const CMyString &a_user) const;
  POSITION Find(const uuid_array_t &uuid) const;

  bool IsChanged() const {return m_changed;}
  void SetChanged(bool changed) {m_changed = changed;} // use sparingly...
  void SetPassKey(const CMyString &new_passkey);

  void SetDisplayStatus(char *p_char_displaystatus, const int length);
  CString GetDisplayStatus() {return m_displaystatus;}
  void CopyPWList(const ItemList &in);
  // Validate() returns true if data modified, false if all OK
  bool Validate(int &n, unsigned &num_PWH_fixed, 
                unsigned &num_uuid_fixed, unsigned &num_uuid_notunique);

 private:
  CMyString m_currfile; // current pw db filespec
  unsigned char *m_passkey; // encrypted by session key
  unsigned int m_passkey_len; // Length of cleartext passkey
  static unsigned char m_session_key[20];
  static unsigned char m_session_salt[20];
  static unsigned char m_session_initialized;
  static CString m_hdr;

  CMyString GetPassKey() const; // returns cleartext - USE WITH CARE
  // Following used by SetPassKey
  void EncryptPassword(const unsigned char *plaintext, int len,
		       unsigned char *ciphertext) const;
  bool m_usedefuser;
  CMyString m_defusername;
  PWSfile::VERSION m_ReadFileVersion;
  unsigned short m_nCurrentMajorVersion, m_nCurrentMinorVersion;

  // the password database
  ItemList m_pwlist;

  bool m_changed;
  HANDLE m_lockFileHandle;

  CString m_displaystatus;
  CString m_wholastsaved, m_whenlastsaved, m_whatlastsaved;
  CString m_user, m_sysname;
};
