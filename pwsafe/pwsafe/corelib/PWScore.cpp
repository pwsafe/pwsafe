// file PWScore.cpp
//-----------------------------------------------------------------------------

#include <fstream.h> // for WritePlaintextFile

#include "PWScore.h"

PWScore::PWScore() : m_currfile(_T("")), m_changed(false),
		     m_usedefuser(false), m_defusername(_T("")),
		     m_ReadFileVersion(PWSfile::UNKNOWN_VERSION)
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
  // Nothing (for now)
}

void
PWScore::ClearData(void)
{
  m_passkey.Trash();

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

void PWScore::SetPassKey(const CMyString &new_passkey)
{
  m_passkey = new_passkey; // XXX tmp!!!
}

bool PWScore::IsPassKey(const CMyString &new_passkey) const
{
  return new_passkey == m_passkey; // XXX tmp
  // Lazy way: Get cleartext m_passkey and compare
  // Right way: encrypt new_passkey & compare to m_passkey
}

CMyString PWScore::GetPassKey() const
{
  return m_passkey; // XXX tmp
}
