/// \file ItemData.cpp
//-----------------------------------------------------------------------------

//in add, Ok is ok the first time

#include "PwsPlatform.h"
#include "../PasswordSafe.h"

#include <math.h>

#include "Util.h"
#include "Blowfish.h"
#include "sha1.h"

#include "../ThisMfcApp.h"

#include "ItemData.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
//More complex constructor
CItemData::CItemData(const CMyString &name,
                     const CMyString &password,
                     const CMyString &notes)
{
   InitStuff();
   SetName(name);
   SetPassword(password);
   SetNotes(notes);
}

CItemData::CItemData(const CItemData &stuffhere)
{
   m_nLength = stuffhere.m_nLength;
   m_pwLength = stuffhere.m_pwLength;
   m_notesLength = stuffhere.m_notesLength;

   m_name = new unsigned char[GetBlockSize(m_nLength)];
   m_nameValid = TRUE;
   memcpy(m_name, stuffhere.m_name, GetBlockSize(m_nLength));
	
   m_password = new unsigned char[GetBlockSize(m_pwLength)];
   m_pwValid = TRUE;
   memcpy(m_password, stuffhere.m_password, GetBlockSize(m_pwLength));

   m_notes = new unsigned char[GetBlockSize(m_notesLength)];
   m_notesValid = TRUE;
   memcpy(m_notes, stuffhere.m_notes, GetBlockSize(m_notesLength));

   memcpy((char*)m_salt, (char*)stuffhere.m_salt, SaltLength);
   m_saltValid = stuffhere.m_saltValid;
}

//Returns a plaintext name
BOOL
CItemData::GetName(CMyString &name) const
{
   return DecryptData(m_name, m_nLength, m_nameValid, &name);
}

CMyString
CItemData::GetName() const
{
   CMyString ret;
   (void) DecryptData(m_name, m_nLength, m_nameValid, &ret);
   return ret;
}


//Returns a plaintext password
BOOL
CItemData::GetPassword(CMyString &password) const
{
   return DecryptData(m_password, m_pwLength, m_pwValid, &password);
}

CMyString
CItemData::GetPassword() const
{
   CMyString ret;
   (void) DecryptData(m_password, m_pwLength, m_pwValid, &ret);
   return ret;
}


//Returns a plaintext notes
BOOL
CItemData::GetNotes(CMyString &notes) const
{
   return DecryptData(m_notes, m_notesLength, m_notesValid, &notes);
}

CMyString
CItemData::GetNotes() const
{
   CMyString ret;
   (void) DecryptData(m_notes, m_notesLength, m_notesValid, &ret);
   return ret;
}


//Encrypts a plaintext name and stores it in m_name
BOOL
CItemData::SetName(const CMyString &name)
{
   return EncryptData(name, &m_name, &m_nLength, m_nameValid);
}

//Encrypts a plaintext password and stores it in m_password
BOOL
CItemData::SetPassword(const CMyString &password)
{
   return EncryptData(password, &m_password, &m_pwLength, m_pwValid);
}

//Encrypts plaintext notes and stores them in m_notes
BOOL
CItemData::SetNotes(const CMyString &notes)
{
   return EncryptData(notes, &m_notes, &m_notesLength, m_notesValid);
}

//Deletes stuff
CItemData::~CItemData()
{
   if (m_nameValid == TRUE)
   {
      delete [] m_name;
      m_nameValid = FALSE;
   }
   if (m_pwValid == TRUE)
   {
      delete [] m_password;
      m_pwValid = FALSE;
   }
   if (m_notesValid == TRUE)
   {
      delete [] m_notes;
      m_notesValid = FALSE;
   }
}

//Encrypts the thing in plain to the variable cipher - alloc'd here
BOOL
CItemData::EncryptData(const CMyString &plain,
                       unsigned char **cipher,
                       int *cLength,
                       BOOL &valid)
{
  const LPCSTR plainstr = (const LPCSTR)plain; // use of CString::operator LPCSTR
  int result = EncryptData((const unsigned char*)plainstr,
                            plain.GetLength(),
                            cipher,
                            cLength,
                            valid);
   return result;
}


BlowFish *
CItemData::MakeBlowFish() const
{
  ASSERT(m_saltValid);
  LPCSTR passstr = LPCSTR(app.m_passkey);

  return ::MakeBlowFish((const unsigned char *)passstr, app.m_passkey.GetLength(),
			m_salt, SaltLength);
}



BOOL
CItemData::EncryptData(const unsigned char *plain,
                       int plainlength,
                       unsigned char **cipher,
                       int *cLength,
                       BOOL &valid)
{
  // Note that the m_salt member is set here, and read in DecryptData,
  // hence this can't be const, but DecryptData can

   if (valid == TRUE)
   {
      delete [] *cipher;
      valid = FALSE;
   }
	
   //Figure out the length of the ciphertext (round for blocks)
   *cLength = plainlength;
   int BlockLength = GetBlockSize(*cLength);

   *cipher = new unsigned char[BlockLength];
   if (*cipher == NULL)
      return FALSE;
   valid = TRUE;
   int x;

   if (m_saltValid == FALSE)
   {
      for (x=0;x<SaltLength;x++)
         m_salt[x] = newrand();
      m_saltValid = TRUE;
   }

   BlowFish *Algorithm = MakeBlowFish();

   unsigned char *tempmem = new unsigned char[BlockLength];
   // invariant: BlockLength >= plainlength
   memcpy((char*)tempmem, (char*)plain, plainlength);

   //Fill the unused characters in with random stuff
   for (x=plainlength; x<BlockLength; x++)
      tempmem[x] = newrand();

   //Do the actual encryption
   for (x=0; x<BlockLength; x+=8)
      Algorithm->Encrypt(tempmem+x, *cipher+x);

   delete Algorithm;
   delete [] tempmem;

   return TRUE;
}

//This is always used for preallocated data - not elegant, but who cares
BOOL
CItemData::DecryptData(const unsigned char *cipher,
                       int cLength,
                       BOOL valid,
                       unsigned char *plain,
                       int plainlength) const
{
  if (valid == FALSE) // check here once instead of in each caller to DecryptData
    return FALSE;

   int BlockLength = GetBlockSize(cLength);

   BlowFish *Algorithm = MakeBlowFish();
	
   unsigned char *tempmem = new unsigned char[BlockLength];

   int x;
   for (x=0;x<BlockLength;x+=8)
      Algorithm->Decrypt(cipher+x, tempmem+x);

   delete Algorithm;

   for (x=0;x<cLength;x++)
      if (x<plainlength)
         plain[x] = tempmem[x];

   delete [] tempmem;

   return TRUE;
}

//Decrypts the thing pointed to by cipher into plain
BOOL
CItemData::DecryptData(const unsigned char *cipher,
                       int cLength,
                       BOOL valid,
                       CMyString *plain) const
{
  if (valid == FALSE) // check here once instead of in each caller to DecryptData
    return FALSE;

   int BlockLength = GetBlockSize(cLength);
	
   unsigned char *plaintxt = new unsigned char [ BlockLength + 1 ];

   BlowFish *Algorithm = MakeBlowFish();
   int x;
   for (x=0;x<BlockLength;x+=8)
      Algorithm->Decrypt(cipher+x, plaintxt+x);

   delete Algorithm;

   //ReleaseBuffer does a strlen, so 0s will be truncated
   for (x=cLength;x<BlockLength;x++)
      plaintxt[x] = 0;
   plaintxt[BlockLength] = 0;

   // {kjp} Assign the decrypted string and delete the temp buffer
   *plain = plaintxt;
   trashMemory( plaintxt, BlockLength + 1 );
   delete [] plaintxt;

   return TRUE;
}


//Called by the constructors
void CItemData::InitStuff()
{
   m_nLength = 0;
   m_pwLength = 0;
   m_notesLength = 0;
	
   m_name = NULL;
   m_nameValid = FALSE;

   m_password = NULL;	
   m_pwValid = FALSE;

   m_notes = NULL;
   m_notesValid = FALSE;

   m_saltValid = FALSE;
}


//Returns the number of bytes of 8 byte blocks needed to store 'size' bytes
int
CItemData::GetBlockSize(int size) const
{
   return (int)ceil((double)size/8.0) * 8;
}

CItemData&
CItemData::operator=(const CItemData &second)
{
   //Check for self-assignment
   if (&second != this)
   {
      if (m_nameValid == TRUE)
      {
         delete [] m_name;
         m_nameValid = FALSE;
      }
      if (m_pwValid == TRUE)
      {
         delete [] m_password;
         m_pwValid = FALSE;
      }
      if (m_notesValid == TRUE)
      {
         delete [] m_notes;
         m_notesValid = FALSE;
      }
		
      m_nLength = second.m_nLength;
      m_pwLength = second.m_pwLength;
      m_notesLength = second.m_notesLength;

      m_name = new unsigned char[GetBlockSize(m_nLength)];
      m_nameValid = TRUE;
      memcpy(m_name, second.m_name, GetBlockSize(m_nLength));
		
      m_password = new unsigned char[GetBlockSize(m_pwLength)];
      m_pwValid = TRUE;
      memcpy(m_password, second.m_password, GetBlockSize(m_pwLength));

      m_notes = new unsigned char[GetBlockSize(m_notesLength)];
      m_notesValid = TRUE;
      memcpy(m_notes, second.m_notes, GetBlockSize(m_notesLength));

      memcpy((char*)m_salt, (char*)second.m_salt, SaltLength);
      m_saltValid = second.m_saltValid;
   }

   return *this;
}

//TODO: "General System Fault. Please sacrifice a goat 
//and two chickens to continue."

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
