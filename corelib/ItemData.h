// ItemData.h
//-----------------------------------------------------------------------------

#if !defined ItemData_h
#define ItemData_h

#include "Util.h"
#include "ItemField.h"
#include "UUIDGen.h"

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
	PASSWORD = 0x6, CTIME = 0x7, MTIME = 0x8, ATIME = 0x9, LTIME = 0xa,
	POLICY = 0xb, END = 0xff}; // field types, per formatV2.txt
   //Construction
  CItemData();

   CItemData(const CItemData& stuffhere);

   //Data retrieval
   CMyString GetName() const; // V17 - deprecated - replaced by GetTitle & GetUser
   CMyString GetTitle() const; // V20
   CMyString GetUser() const; // V20
   CMyString GetPassword() const;
   CMyString GetNotes() const;
   void GetUUID(uuid_array_t &) const; // V20
   CMyString GetGroup() const; // V20
   CMyString GetPlaintext(char separator) const; // returns all fields separated by separator

   void CreateUUID(); // V20 - generate UUID for new item
   void SetName(const CMyString &name); // V17 - deprecated - replaced by SetTitle & SetUser
   void SetTitle(const CMyString &title); // V20
   void SetUser(const CMyString &user); // V20
   void SetPassword(const CMyString &password);
   void SetNotes(const CMyString &notes);
   void SetUUID(const uuid_array_t &UUID); // V20
   void SetGroup(const CMyString &group); // V20
   CItemData& operator=(const CItemData& second);
  // Following used by display methods - we just keep it handy
  void *GetDisplayInfo() const {return m_display_info;}
  void SetDisplayInfo(void *di) {m_display_info = di;}

private:
  CItemField m_Name;
  CItemField m_Title;
  CItemField m_User;
  CItemField m_Password;
  CItemField m_Notes;
  CItemField m_UUID;
  CItemField m_Group;
  //The salt value
  unsigned char m_salt[SaltLength];
  // Following used by display methods - we just keep it handy
  void *m_display_info;

  // move from pre-2.0 name to post-2.0 title+user
  void SplitName(const CMyString &name,
		 CMyString &title, CMyString &username);

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
