// file PWScore.cpp
//-----------------------------------------------------------------------------

#if (defined(_MSC_VER) && (_MSC_VER >= 13))
#include <fstream>
using namespace std;
#else
#include <fstream.h> // for WritePlaintextFile
#endif

#include "PWScore.h"
#include "BlowFish.h"

PWScore::PWScore() : m_currfile(_T("")), m_changed(false),
		     m_usedefuser(false), m_defusername(_T("")),
		     m_ReadFileVersion(PWSfile::UNKNOWN_VERSION),
		     m_passkey(NULL), m_passkey_len(0)
{
  int i;

  srand((unsigned)time(NULL));
  CItemData::SetSessionKey(); // per-session initialization
  for (i = 0; i < sizeof(m_session_key); i++)
    m_session_key[i] = newrand();
  for (i = 0; i < sizeof(m_session_salt); i++)
    m_session_salt[i] = newrand();

}

PWScore::~PWScore()
{
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + 7)/8)*8);
    delete[] m_passkey;
  }
}

void
PWScore::ClearData(void)
{
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + 7)/8)*8);
    delete[] m_passkey;
    m_passkey_len = 0;
  }
  //Composed of ciphertext, so doesn't need to be overwritten
  m_pwlist.RemoveAll();
}

void
PWScore::NewFile(const CMyString &passkey)
{
   ClearData();
   SetPassKey(passkey);
   m_changed = false;
}

int
PWScore::WriteFile(const CMyString &filename, PWSfile::VERSION version)
{
  PWSfile out(filename, GetPassKey());

  int status;

  status = out.OpenWriteFile(version);

  if (status != PWSfile::SUCCESS)
    return CANT_OPEN_FILE;

  CItemData temp;
  POSITION listPos = m_pwlist.GetHeadPosition();
  while (listPos != NULL)
    {
      temp = m_pwlist.GetAt(listPos);
      out.WriteRecord(temp);
      m_pwlist.GetNext(listPos);
    }
  out.CloseFile();

  m_changed = FALSE;
  m_ReadFileVersion = version; // needed when saving a V17 as V20 1st time [871893]

  return SUCCESS;
}

int
PWScore::WritePlaintextFile(const CMyString &filename)
{
  ofstream of(filename);

  if (!of)
    return CANT_OPEN_FILE;

  CItemData temp;
  POSITION listPos = m_pwlist.GetHeadPosition();
  while (listPos != NULL)
    {
      temp = m_pwlist.GetAt(listPos);
      of << temp.GetPlaintext('\t') << endl;
      m_pwlist.GetNext(listPos);
    }
  of.close();

  return SUCCESS;
}

int PWScore::CheckPassword(const CMyString &filename, CMyString& passkey)
{
  PWSfile f(filename, passkey);

  int status = f.CheckPassword();

  switch (status) {
  case PWSfile::SUCCESS:
    return SUCCESS;
  case PWSfile::CANT_OPEN_FILE:
    return CANT_OPEN_FILE;
  case PWSfile::WRONG_PASSWORD:
    return WRONG_PASSWORD;
  default:
    ASSERT(0);
    return status; // should never happen
  }
}


int
PWScore::ReadFile(const CMyString &a_filename,
                   const CMyString &a_passkey)
{	
   //That passkey had better be the same one that came from CheckPassword(...)

   PWSfile in(a_filename, a_passkey);

  int status;
  
  m_ReadFileVersion = in.GetFileVersion();

  if (m_ReadFileVersion == PWSfile::UNKNOWN_VERSION)
    return UNKNOWN_VERSION;

  status = in.OpenReadFile(m_ReadFileVersion);

  if (status != PWSfile::SUCCESS)
    return CANT_OPEN_FILE;
  
  // prepare handling of pre-2.0 DEFUSERCHR conversion
  if (m_ReadFileVersion == PWSfile::V17)
    in.SetDefUsername(m_defusername);

   ClearData(); //Before overwriting old data, but after opening the file... 

   SetPassKey(a_passkey);

   CItemData temp;

   status = in.ReadRecord(temp);

   while (status == PWSfile::SUCCESS)
   {
      m_pwlist.AddTail(temp);
      status = in.ReadRecord(temp);
   }

   in.CloseFile();

   return SUCCESS;
}

int PWScore::RenameFile(const CMyString &oldname, const CMyString &newname)
{
  return PWSfile::RenameFile(oldname, newname);
}


void PWScore::ChangePassword(const CMyString &newPassword)
{
 
  SetPassKey(newPassword);
  m_changed = TRUE;
}


// Finds stuff based on title & user fields only
POSITION
PWScore::Find(const CMyString &a_title, const CMyString &a_user)
{
   POSITION listPos = m_pwlist.GetHeadPosition();
   CMyString title, user;

   while (listPos != NULL)
   {
      title = m_pwlist.GetAt(listPos).GetTitle();
      user = m_pwlist.GetAt(listPos).GetUser();
      if (title == a_title && user == a_user)
         break;
      else
         m_pwlist.GetNext(listPos);
   }

   return listPos;
}

void PWScore::EncryptPassword(const unsigned char *plaintext, int len,
			      unsigned char *ciphertext) const
{
  // ciphertext is ((len +7)/8)*8 bytes long
  BlowFish *Algorithm = MakeBlowFish(m_session_key, sizeof(m_session_key),
				     m_session_salt, sizeof(m_session_salt));
  int BlockLength = ((len + 7)/8)*8;
  unsigned char curblock[8];

  for (int x=0;x<BlockLength;x+=8) {
    int i;
    if ((len == 0) ||
	((len%8 != 0) && (len - x < 8))) {
      //This is for an uneven last block
      memset(curblock, 0, 8);
      for (i = 0; i < len %8; i++)
	curblock[i] = plaintext[x + i];
      } else
	for (i = 0; i < 8; i++)
	  curblock[i] = plaintext[x + i];
      Algorithm->Encrypt(curblock, curblock);
      memcpy(ciphertext + x, curblock, 8);
   }
   trashMemory(curblock, 8);
  delete Algorithm;
}

void PWScore::SetPassKey(const CMyString &new_passkey)
{
  // if changing, clear old
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + 7)/8)*8);
    delete[] m_passkey;
  }

  m_passkey_len = new_passkey.GetLength();

  int BlockLength = ((m_passkey_len + 7)/8)*8;
  m_passkey = new unsigned char[BlockLength];
  LPCTSTR plaintext = LPCTSTR(new_passkey);
  EncryptPassword((const unsigned char *)plaintext, m_passkey_len,
		  m_passkey);
}

CMyString PWScore::GetPassKey() const
{
  CMyString retval(_T(""));
  if (m_passkey_len > 0) {
    unsigned int BlockLength = ((m_passkey_len + 7)/8)*8;
    BlowFish *Algorithm = MakeBlowFish(m_session_key, sizeof(m_session_key),
				       m_session_salt, sizeof(m_session_salt));
    unsigned char curblock[8];

    for (unsigned int x = 0; x < BlockLength; x += 8) {
      unsigned int i;
      for (i = 0; i < 8; i++)
	curblock[i] = m_passkey[x + i];
      Algorithm->Decrypt(curblock, curblock);
      for (i = 0; i < 8; i++)
	if (x + i < m_passkey_len)
	  retval += curblock[i];
    }
    trashMemory(curblock, 8);
    delete Algorithm;
  }
  return retval;
}
