// ItemData.h
//-----------------------------------------------------------------------------

#if !defined ItemData_h
#define ItemData_h

#include "Util.h"
#include "ItemField.h"

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
  enum {NAME=0, GUID=0x1, GROUP = 0x2, TITLE = 0x3, USER = 0x4, NOTES = 0x5,
	PASSWORD = 0x6, CTIME = 0x7, MTIME = 0x8, ATIME = 0x9, LTIME = 0xa,
	POLICY = 0xb}; // field types, per formatV2.txt
   //Construction
  CItemData();

   CItemData(const CItemData& stuffhere);

   //Data retrieval
   CMyString GetName() const; // V17 - deprecated - replaced by GetTitle & GetUser
   CMyString GetTitle() const; // V20
   CMyString GetUser() const; // V20
   CMyString GetPassword() const;
   CMyString GetNotes() const;

   void SetName(const CMyString &name); // V17 - deprecated - replaced by GetTitle & GetUser
   void SetTitle(const CMyString &title); // V20
   void SetUser(const CMyString &user); // V20
   void SetPassword(const CMyString &password);
   void SetNotes(const CMyString &notes);

   CItemData& operator=(const CItemData& second);

private:
  CItemField m_Name;
  CItemField m_Title;
  CItemField m_User;
  CItemField m_Password;
  CItemField m_Notes;

  //The salt value
  unsigned char m_salt[SaltLength];
	
  // move from pre-2.0 name to post-2.0 title+user
  void SplitName(const CMyString &name,
		 CMyString &title, CMyString &username);

  // Create local Encryption/Decryption object
  BlowFish *MakeBlowFish() const;
  // Laziness is a Virtue:
  void GetField(const CItemField &field, CMyString &value) const;
  void SetField(CItemField &field, const CMyString &value);
};

#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
