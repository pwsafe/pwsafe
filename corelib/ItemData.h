// ItemData.h
//-----------------------------------------------------------------------------

#if !defined ItemData_h
#define ItemData_h

#include "Util.h"

//-----------------------------------------------------------------------------

/*
 * CItemData is a class that contains the data present in a password entry
 *
 * 'Name' is the pre-2.x field, that had both the entry title and the username rolled-in
 * together, separated by SPLTCHR (defined in util.h). In 2.0 and later, this filed is unused,
 * and the tile and username are stored in separate fields.
 *
 * What makes this class interesting is that all fields are kept encrypted from the moment
 * of construction, and are decrypted by the appropriate accessor (Get* member function).
 *
 * Of course, all this is to protect the data in memory, and has nothing to do with how the
 * records are written to disk.
 */

class BlowFish;

class CItemData
{
public:
   //Construction
   CItemData()
     : m_nLength(0), m_tLength(0), m_uLength(0), m_pwLength(0), m_notesLength(0),
        m_name(NULL), m_nameValid(FALSE),
        m_title(NULL), m_titleValid(FALSE),
        m_user(NULL), m_userValid(FALSE),
        m_password(NULL), m_pwValid(FALSE),
        m_notes(NULL), m_notesValid(FALSE),
        m_saltValid(FALSE)
   {}

   CItemData(const CMyString &name, const CMyString &password, const CMyString &notes);
   CItemData(const CItemData& stuffhere);

   //Data retrieval
   CMyString GetName() const; // V17 - deprecated - replaced by GetTitle & GetUser
   CMyString GetTitle() const; // V20
   CMyString GetUser() const; // V20
   CMyString GetPassword() const;
   CMyString GetNotes() const;

   BOOL SetName(const CMyString &name); // V17 - deprecated - replaced by GetTitle & GetUser
   BOOL SetTitle(const CMyString &title); // V20
   BOOL SetUser(const CMyString &user); // V20
   BOOL SetPassword(const CMyString &password);
   BOOL SetNotes(const CMyString &notes);

   //Copies contents of pointers too
   CItemData& operator=(const CItemData& second);

   //Destructor
   ~CItemData();

private:
   //Their respective pointers are valid
   BOOL m_nameValid;
   BOOL m_titleValid;
   BOOL m_userValid;
   BOOL m_pwValid;
   BOOL m_notesValid;

   //Actual encryption/decryption
   BOOL EncryptData(const CMyString &plain,
                    unsigned char** cipher, int* cLength,
                    BOOL &valid);
   BOOL EncryptData(const unsigned char* plain, int plainlength,
                    unsigned char** cipher, int* cLength,
                    BOOL &valid);
   BOOL DecryptData(const unsigned char* cipher, int cLength,
                    BOOL valid,
                    unsigned char* plain, int plainlength) const;
   BOOL DecryptData(const unsigned char* cipher, int cLength,
                    BOOL valid,
                    CMyString* plain) const;

   //Number of 8 byte blocks needed for size
   int GetBlockSize(int size) const;
	
   //Length (real, not block) of data fields
   int m_nLength;
   int m_tLength;
   int m_uLength;
   int m_pwLength;
   int m_notesLength;

   //Pointers to ciphertext
   unsigned char* m_name;
   unsigned char* m_title;
   unsigned char* m_user;
   unsigned char* m_password;
   unsigned char* m_notes;

   //The salt value
   unsigned char m_salt[SaltLength];
	
   //Determine if the salt is valid
   BOOL m_saltValid;

   //Local initialization
   void InitStuff();

  // Create local Encryption/Decryption object
  BlowFish *MakeBlowFish() const;
};

#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
