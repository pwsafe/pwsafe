// ItemData.h
//-----------------------------------------------------------------------------

#if !defined ItemData_h
#define ItemData_h

#include "Util.h"
#include "ItemField.h"
#include "UUIDGen.h"
#include <time.h> // for time_t
//-----------------------------------------------------------------------------

/*
 * CItemData is a class that contains the data present in a password entry
 *
 * 'Name' is the pre-2.x field, that had both the entry title and the
 * username rolled-in together, separated by SPLTCHR (defined in util.h).
 * In 2.0 and later, this field is unused, and the title and username
 * are stored in separate fields.
 *
 * What makes this class interesting is that all fields are kept encrypted
 * from the moment of construction, and are decrypted by the appropriate
 * accessor (Get* member function).
 *
 * All this is to protect the data in memory, and has nothing to do with
 * how the records are written to disk.
 */

class BlowFish;

class CItemData
{
public:
  enum {NAME=0, UUID=0x1, GROUP = 0x2, TITLE = 0x3, USER = 0x4, NOTES = 0x5,
	PASSWORD = 0x6, CTIME = 0x7, PMTIME = 0x8, ATIME = 0x9, LTIME = 0xa,
	POLICY = 0xb, RMTIME = 0xc, URL = 0xd, AUTOTYPE = 0xe, PWHIST = 0xf,
    END = 0xff}; // field types, per formatV{2,3}.txt

  static void SetSessionKey(); // call exactly once per session
   //Construction
  CItemData();

   CItemData(const CItemData& stuffhere);

   //Data retrieval
   CMyString GetName() const; // V17 - deprecated - replaced by GetTitle & GetUser
   CMyString GetTitle() const; // V20
   CMyString GetUser() const; // V20
   CMyString GetPassword() const;
   CMyString GetNotes(TCHAR delimiter = 0) const;
   void GetUUID(uuid_array_t &) const; // V20
   CMyString GetGroup() const; // V20
   CMyString GetURL() const; // V30
   CMyString GetAutoType() const; // V30
   CMyString GetATime() const {return GetTime(ATIME, ASC_UNKNOWN);}  // V30
   CMyString GetCTime() const {return GetTime(CTIME, ASC_UNKNOWN);}  // V30
   CMyString GetLTime() const {return GetTime(LTIME, ASC_UNKNOWN);}  // V30
   CMyString GetPMTime() const {return GetTime(PMTIME, ASC_UNKNOWN);}  // V30
   CMyString GetRMTime() const {return GetTime(RMTIME, ASC_UNKNOWN);}  // V30
   CMyString GetATimeN() const {return GetTime(ATIME, ASC_NULL);}  // V30
   CMyString GetCTimeN() const {return GetTime(CTIME, ASC_NULL);}  // V30
   CMyString GetLTimeN() const {return GetTime(LTIME, ASC_NULL);}  // V30
   CMyString GetPMTimeN() const {return GetTime(PMTIME, ASC_NULL);}  // V30
   CMyString GetRMTimeN() const {return GetTime(RMTIME, ASC_NULL);}  // V30
   CMyString GetATimeExp() const {return GetTime(ATIME, EXPORT_IMPORT);}  // V30
   CMyString GetCTimeExp() const {return GetTime(CTIME, EXPORT_IMPORT);}  // V30
   CMyString GetLTimeExp() const {return GetTime(LTIME, EXPORT_IMPORT);}  // V30
   CMyString GetPMTimeExp() const {return GetTime(PMTIME, EXPORT_IMPORT);}  // V30
   CMyString GetRMTimeExp() const {return GetTime(RMTIME, EXPORT_IMPORT);}  // V30
   //  These populate the time structure instead of giving a character string
   void GetATime(time_t &t) const {return GetTime(ATIME, t);}  // V30
   void GetCTime(time_t &t) const {return GetTime(CTIME, t);}  // V30
   void GetLTime(time_t &t) const {return GetTime(LTIME, t);}  // V30
   void GetPMTime(time_t &t) const {return GetTime(PMTIME, t);}  // V30
   void GetRMTime(time_t &t) const {return GetTime(RMTIME, t);}  // V30
   CMyString GetPWHistory() const;  // V30
   // GetPlaintext returns all fields separated by separator, if delimiter is != 0, then
   // it's used for multi-line notes.
   CMyString GetPlaintext(TCHAR separator, TCHAR delimiter = 0) const;

   void CreateUUID(); // V20 - generate UUID for new item
   void SetName(const CMyString &name,
		const CMyString &defaultUsername); // V17 - deprecated - replaced by SetTitle & SetUser
   void SetTitle(const CMyString &title, TCHAR delimiter = 0);
   void SetUser(const CMyString &user); // V20
   void SetPassword(const CMyString &password);
   void SetNotes(const CMyString &notes, TCHAR delimiter = 0);
   void SetUUID(const uuid_array_t &UUID); // V20
   void SetGroup(const CMyString &group); // V20
   void SetURL(const CMyString &URL); // V30
   void SetAutoType(const CMyString &autotype); // V30
   void SetATime() {return SetTime(ATIME);}  // V30
   void SetATime(time_t t) {return SetTime(ATIME, t);}  // V30
   void SetATime(const CString &time_str) {return SetTime(ATIME, time_str);}  // V30
   void SetCTime() {return SetTime(CTIME);}  // V30
   void SetCTime(time_t t) {return SetTime(CTIME, t);}  // V30
   void SetCTime(const CString &time_str) {return SetTime(CTIME, time_str);}  // V30
   void SetLTime() {return SetTime(LTIME);}  // V30
   void SetLTime(time_t t) {return SetTime(LTIME, t);}  // V30
   void SetLTime(const CString &time_str) {return SetTime(LTIME, time_str);}  // V30
   void SetPMTime() {return SetTime(PMTIME);}  // V30
   void SetPMTime(time_t t) {return SetTime(PMTIME, t);}  // V30
   void SetPMTime(const CString &time_str) {return SetTime(PMTIME, time_str);}  // V30
   void SetRMTime() {return SetTime(RMTIME);}  // V30
   void SetRMTime(time_t t) {return SetTime(RMTIME, t);}  // V30
   void SetRMTime(const CString &time_str) {return SetTime(RMTIME, time_str);}  // V30
   void SetPWHistory(const CMyString &PWHistory);  // V30
   CItemData& operator=(const CItemData& second);
  // Following used by display methods - we just keep it handy
  void *GetDisplayInfo() const {return m_display_info;}
  void SetDisplayInfo(void *di) {m_display_info = di;}
  void Clear();
private:
  CItemField m_Name;
  CItemField m_Title;
  CItemField m_User;
  CItemField m_Password;
  CItemField m_Notes;
  CItemField m_UUID;
  CItemField m_Group;
  CItemField m_URL;
  CItemField m_AutoType;
  CItemField m_tttATime;	// last 'A'ccess time
  CItemField m_tttCTime;	// 'C'reation time
  CItemField m_tttLTime;	// password 'L'ifetime
  CItemField m_tttPMTime;	// last 'P'assword 'M'odification time
  CItemField m_tttRMTime;	// last 'R'ecord 'M'odification time
  CItemField m_PWHistory;

  // random key for storing stuff in memory, just to remove dependence
  // on passphrase
  static bool IsSessionKeySet;
  static unsigned char SessionKey[64];
  //The salt value
  unsigned char m_salt[SaltLength];
  // Following used by display methods - we just keep it handy
  void *m_display_info;

  // move from pre-2.0 name to post-2.0 title+user
  void SplitName(const CMyString &name,
		 CMyString &title, CMyString &username);
  bool VerifyASCDateTimeString(const CString time_str);
  bool VerifyImportDateTimeString(const CString time_str);
  CMyString GetTime(const int whichtime, const int result_format) const; // V30
  void GetTime(const int whichtime, time_t &t) const; // V30
  void SetTime(const int whichtime); // V30
  void SetTime(const int whichtime, time_t t); // V30
  void SetTime(const int whichtime, const CString &time_str); // V30

  // Create local Encryption/Decryption object
  BlowFish *MakeBlowFish() const;
  // Laziness is a Virtue:
  void GetField(const CItemField &field, CMyString &value) const;
  void GetField(const CItemField &field, unsigned char *value, unsigned int &length) const;
  void SetField(CItemField &field, const CMyString &value);
  void SetField(CItemField &field, const unsigned char *value, unsigned int length);
};

#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
