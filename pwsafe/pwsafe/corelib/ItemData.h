// ItemData.h
//-----------------------------------------------------------------------------

#ifndef _ITEMDATA_H_
#define _ITEMDATA_H_

#include "util.h"

//-----------------------------------------------------------------------------
class CItemData
{
public:
   //Construction
   CItemData();
   CItemData(CMyString name, CMyString password, CMyString notes);
   CItemData(CItemData& stuffhere);

   //Data retrieval
   BOOL GetName(CMyString& name);
   BOOL GetPassword(CMyString& password);
   BOOL GetNotes(CMyString& notes);

   // jpr - new data retrieval
   CMyString GetName();
   CMyString GetPassword();
   CMyString GetNotes();

   BOOL SetName(CMyString name);
   BOOL SetPassword(CMyString password);
   BOOL SetNotes(CMyString notes);

   //Copies contents of pointers too
   CItemData& operator=(const CItemData& second);

   //Their respective pointers are valid
   BOOL m_nameValid;
   BOOL m_pwValid;
   BOOL m_notesValid;
   //for why public: explanation, check InitInstance

   //Destructor
   ~CItemData();

protected:
   //Actual encryption/decryption
   BOOL EncryptData(CMyString plain,
                    unsigned char** cipher, int* cLength,
                    BOOL* valid);
   BOOL EncryptData(unsigned char* plain, int plainlength,
                    unsigned char** cipher, int* cLength,
                    BOOL* valid);
   BOOL DecryptData(unsigned char* cipher, int cLength,
                    BOOL valid,
                    unsigned char* plain, int plainlength);
   BOOL DecryptData(unsigned char* cipher, int cLength,
                    BOOL valid,
                    CMyString* plain);

   //Number of 8 byte blocks needed for size
   int GetBlockSize(int size);
	
   //Length (real, not block) of m_name and m_password
   int m_nLength;
   int m_pwLength;
   int m_notesLength;

   //Pointers to ciphertext
   unsigned char* m_name;
   unsigned char* m_password;
   unsigned char* m_notes;

   //The salt value
   unsigned char m_salt[SaltLength];
	
   //Determine if the salt is valid
   BOOL m_saltValid;

private:
   //Local initialization
   void InitStuff();
};

#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
