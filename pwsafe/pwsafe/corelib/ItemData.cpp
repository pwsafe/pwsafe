/// \file ItemData.cpp
//-----------------------------------------------------------------------------

//in add, Ok is ok the first time

#include "stdafx.h"
#include "PasswordSafe.h"
#include "ItemData.h"
#include "util.h"
#include "blowfish.h"
#include "sha1.h"

#include <io.h>
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Normal constructor
CItemData::CItemData()
{
   InitStuff();
}

//More complex constructor
CItemData::CItemData(CMyString name,
                     CMyString password,
                     CMyString notes)
{
   InitStuff();
   SetName(name);
   SetPassword(password);
   SetNotes(notes);
}

CItemData::CItemData(CItemData &stuffhere)
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
BOOL CItemData::GetName(CMyString &name)
{
   return DecryptData(m_name, m_nLength, m_nameValid, &name);
}
CMyString
CItemData::GetName()
{
   CMyString ret;
   (void) DecryptData(m_name, m_nLength, m_nameValid, &ret);
   return ret;
}


//Returns a plaintext password
BOOL CItemData::GetPassword(CMyString &password)
{
   return DecryptData(m_password, m_pwLength, m_pwValid, &password);
}
CMyString
CItemData::GetPassword()
{
   CMyString ret;
   (void) DecryptData(m_password, m_pwLength, m_pwValid, &ret);
   return ret;
}


//Returns a plaintext notes
BOOL CItemData::GetNotes(CMyString &notes)
{
   return DecryptData(m_notes, m_notesLength, m_notesValid, &notes);
}
CMyString
CItemData::GetNotes()
{
   CMyString ret;
   (void) DecryptData(m_notes, m_notesLength, m_notesValid, &ret);
   return ret;
}


//Encrypts a plaintext name and stores it in m_name
BOOL CItemData::SetName(CMyString name)
{
   return EncryptData(name, &m_name, &m_nLength, (BOOL*)&m_nameValid);
}

//Encrypts a plaintext password and stores it in m_password
BOOL CItemData::SetPassword(CMyString password)
{
   return EncryptData(password, &m_password, &m_pwLength, (BOOL*)&m_pwValid);
}

//Encrypts plaintext notes and stores them in m_notes
BOOL CItemData::SetNotes(CMyString notes)
{
   return EncryptData(notes, &m_notes, &m_notesLength, (BOOL*)&m_notesValid);
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
BOOL CItemData::EncryptData(CMyString plain,
                            unsigned char **cipher,
                            int *cLength,
                            BOOL *valid)
{
   int result = EncryptData((unsigned char*)plain.GetBuffer(plain.GetLength()),
                            plain.GetLength(),
                            cipher,
                            cLength,
                            valid);
   plain.ReleaseBuffer();
   return result;
}

BOOL CItemData::EncryptData(unsigned char *plain,
                            int plainlength,
                            unsigned char **cipher,
                            int *cLength,
                            BOOL *valid)
{
   if (*valid == TRUE)
   {
      delete [] *cipher;
      *valid = FALSE;
   }
	
   //Figure out the length of the ciphertext (round for blocks)
   *cLength = plainlength;
   int BlockLength = GetBlockSize(*cLength);

   *cipher = new unsigned char[BlockLength];
   if (*cipher == NULL)
      return FALSE;
   *valid = TRUE;
   int x;

   if (m_saltValid == FALSE)
   {
      for (x=0;x<SaltLength;x++)
         m_salt[x] = newrand();
      m_saltValid = TRUE;
   }

   unsigned char passkey[20];
   VirtualLock(passkey, 20);
   CMyString stringPasskey = app.m_passkey;

   SHA1_CTX context;
   SHA1Init(&context);
   SHA1Update(&context,
              (unsigned char*)stringPasskey.GetBuffer(stringPasskey.GetLength()),
              stringPasskey.GetLength());
   stringPasskey.ReleaseBuffer();
   SHA1Update(&context, m_salt, SaltLength);
   SHA1Final(passkey, &context);
	
   BlowFish Algorithm(passkey, 20);

   unsigned char *tempmem = new unsigned char[BlockLength];
   memcpy((char*)tempmem, (char*)plain, plainlength);

   //Fill the unused characters in with random stuff
   for (x=plainlength;x<BlockLength;x++)
      tempmem[x] = newrand();

   //Do the actual encryption
   for (x=0;x<BlockLength;x+=8)
      Algorithm.Encrypt(tempmem+x, *cipher+x);

   delete [] tempmem;
   trashMemory(passkey, 20);
   trashMemory(context);
   VirtualUnlock(passkey, 20);

   return TRUE;
}

//This is always used for preallocated data - not elegant, but who cares
BOOL CItemData::DecryptData(unsigned char *cipher,
                            int cLength,
                            BOOL valid,
                            unsigned char *plain,
                            int plainlength)
{
   int BlockLength = GetBlockSize(cLength);
	
   unsigned char passkey[20];
   VirtualLock((char*)passkey, 20);
   CMyString stringPasskey = app.m_passkey;

   SHA1_CTX context;
   SHA1Init(&context);
   SHA1Update(&context,
              (unsigned char*)stringPasskey.GetBuffer(stringPasskey.GetLength()),
              stringPasskey.GetLength());
   stringPasskey.ReleaseBuffer();
   SHA1Update(&context, m_salt, SaltLength);
   SHA1Final(passkey, &context);

   BlowFish Algorithm(passkey, 20);
	
   unsigned char *tempmem = new unsigned char[BlockLength];

   int x;
   for (x=0;x<BlockLength;x+=8)
      Algorithm.Decrypt(cipher+x, tempmem+x);

   for (x=0;x<cLength;x++)
      if (x<plainlength)
         plain[x] = tempmem[x];

   trashMemory(passkey, 20);
   trashMemory(context);
   VirtualUnlock((char*)passkey, 20);
   delete [] tempmem;

   return TRUE;
}

//Decrypts the thing pointed to by cipher into plain
BOOL CItemData::DecryptData(unsigned char *cipher,
                            int cLength,
                            BOOL valid,
                            CMyString *plain)
{
   int BlockLength = GetBlockSize(cLength);
	
   unsigned char *plaintxt = (unsigned char*)plain->GetBuffer(BlockLength+1);
   unsigned char passkey[20];
   VirtualLock((char*)passkey, 20);
   CMyString stringPasskey = app.m_passkey;

   SHA1_CTX context;
   SHA1Init(&context);
   SHA1Update(&context,
              (unsigned char*)stringPasskey.GetBuffer(stringPasskey.GetLength()),
              stringPasskey.GetLength());
   stringPasskey.ReleaseBuffer();
   SHA1Update(&context, m_salt, SaltLength);
   SHA1Final(passkey, &context);

   BlowFish Algorithm(passkey, 20);
   int x;
   for (x=0;x<BlockLength;x+=8)
      Algorithm.Decrypt(cipher+x, plaintxt+x);

   //ReleaseBuffer does a strlen, so 0s will be truncated
   for (x=cLength;x<BlockLength;x++)
      plaintxt[x] = 0;
   plaintxt[BlockLength] = 0;

   plain->ReleaseBuffer();
   trashMemory(passkey, 20);
   trashMemory(context);
   VirtualUnlock((char*)passkey, 20);

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

//Returns the number of 8 byte blocks needed to store 'size' bytes
int CItemData::GetBlockSize(int size)
{
   return (int)ceil((double)size/8.0) * 8;
}

CItemData& CItemData::operator=(const CItemData &second)
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
