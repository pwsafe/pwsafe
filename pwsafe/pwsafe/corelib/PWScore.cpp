// file PWScore.cpp
//-----------------------------------------------------------------------------

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include "PWScore.h"
#include "global.h"

PWScore::PWScore() : m_currfile(_T("")), m_changed(false),
		     m_usedefuser(false), m_defusername(_T(""))
{
}

PWScore::~PWScore()
{
  // Nothing (for now)
}

void
PWScore::ClearData(void)
{
  global.m_passkey.Trash();

   //Composed of ciphertext, so doesn't need to be overwritten
   m_pwlist.RemoveAll();
	
}

void
PWScore::NewFile(const CMyString &passkey)
{
   ClearData();

   global.m_passkey = passkey;

   for (int x=0; x<8; x++)
      m_randstuff[x] = newrand();
   m_randstuff[8] = m_randstuff[9] = '\0';
   GenRandhash(global.m_passkey, m_randstuff, m_randhash);
   m_changed = false;
}

int
PWScore::WriteFile(const CMyString &filename)
{
  int out = _open((LPCTSTR)filename,
		  _O_BINARY|_O_WRONLY|_O_SEQUENTIAL|_O_TRUNC|_O_CREAT,
		  _S_IREAD | _S_IWRITE);

  if (out == -1)
    return CANT_OPEN_FILE;

  _write(out, m_randstuff, 8);
  _write(out, m_randhash, 20);

  /*
    I know salt is just salt, but randomness always makes me
    nervous - must check this out {jpr}
  */
  unsigned char* thesalt = new unsigned char[SaltLength];
  for (int x=0; x<SaltLength; x++)
    thesalt[x] = newrand();

  _write(out, thesalt, SaltLength);
	
  unsigned char ipthing[8];
  for (x=0; x<8; x++)
    ipthing[x] = newrand();
  _write(out, ipthing, 8);

  //Write out full names
  if (GetUseDefUser())
    MakeFullNames(GetDefUsername());

  CItemData temp;
  POSITION listPos = m_pwlist.GetHeadPosition();
  CMyString tempdata;
  while (listPos != NULL)
    {
      temp = m_pwlist.GetAt(listPos);
      temp.GetName(tempdata);
      WriteCBC(out, tempdata, thesalt, ipthing);
      temp.GetPassword(tempdata);
      WriteCBC(out, tempdata, thesalt, ipthing);
      temp.GetNotes(tempdata);
      WriteCBC(out, tempdata, thesalt, ipthing);
      m_pwlist.GetNext(listPos);
    }
  _close(out);

  delete [] thesalt;

  //Restore shortened names if necessary
  if (GetUseDefUser())
    DropDefUsernames(GetDefUsername());

  m_changed = FALSE;

  return SUCCESS;
}

bool PWScore::FileExists(const CMyString &filename)
{
  struct _stat statbuf;
  int status;

  status = ::_tstat(filename, &statbuf);
  return (status == 0);
}


int PWScore::CheckPassword(const CMyString &filename, CMyString& passkey)
{
  unsigned char randstuff[StuffSize];
  unsigned char randhash[20];

  int in = _open((LPCTSTR) filename,
		 _O_BINARY | _O_RDONLY | _O_SEQUENTIAL,
		 S_IREAD | _S_IWRITE);

  if (in == -1) {
    return CANT_OPEN_FILE;
  } else {
    /*
      The beginning of the database file is
      8 bytes of randomness and a SHA1 hash {jpr}
    */
    _read(in, randstuff, 8);
    _read(in, randhash, 20);
    _close(in);
  }

  randstuff[8] = randstuff[9] = '\0'; // Gross fugbix
  unsigned char temphash[20]; // HashSize
  GenRandhash(passkey, randstuff, temphash);

  if (0 != memcmp((char*)randhash,
		  (char*)temphash,
		  20)) {// HashSize
    return WRONG_PASSWORD;
  } else {
    // Side effect: If the match is successful, update our data members
    // ??? Do we need to do this ???
    memcpy(m_randstuff, randstuff, StuffSize);
    memcpy(m_randhash, randhash, 20);
    return SUCCESS;
  }
}

int PWScore::WriteCBC(int fp, const CString &data, const unsigned char *salt,
		      unsigned char *ipthing)
{
  // We do a double cast because the LPCSTR cast operator is overridden by the CString class
  // to access the pointer we need,
  // but we in fact need it as an unsigned char. Grrrr.
  LPCSTR passstr = LPCSTR(global.m_passkey);
  LPCSTR datastr = LPCSTR(data);

  return _writecbc(fp, (const unsigned char *)datastr, data.GetLength(),
		   (const unsigned char *)passstr, global.m_passkey.GetLength(),
		   salt, SaltLength, ipthing);
}


int
PWScore::ReadCBC(int fp, CMyString &data, const unsigned char *salt,
		 unsigned char *ipthing)
{
  // We do a double cast because the LPCSTR cast operator is overridden by the CString class
  // to access the pointer we need,
  // but we in fact need it as an unsigned char. Grrrr.
  LPCSTR passstr = LPCSTR(global.m_passkey);

  unsigned char *buffer = NULL;
  unsigned int buffer_len = 0;
  int retval;

  retval = _readcbc(fp, buffer, buffer_len,
		   (const unsigned char *)passstr, global.m_passkey.GetLength(),
		   salt, SaltLength, ipthing);
  if (buffer_len > 0) {
    CMyString str(LPCSTR(buffer), buffer_len);
    data = str;
    trashMemory(buffer, buffer_len);
    delete[] buffer;
  } else {
    data = "";
  }
  return retval;
}


int
PWScore::ReadFile(const CMyString &a_filename,
                   const CMyString &a_passkey)
{	
   //That passkey had better be the same one that came from CheckPassword(...)

   int in = _open((LPCTSTR) a_filename,
                  _O_BINARY |_O_RDONLY | _O_SEQUENTIAL,
                  S_IREAD | _S_IWRITE);

   if (in == -1)
      return CANT_OPEN_FILE;

   ClearData(); //Before overwriting old data, but after opening the file... 

   _read(in, m_randstuff, 8);
   _read(in, m_randhash, 20);

   unsigned char* salt = new unsigned char[SaltLength];
   unsigned char ipthing[8];
   _read(in, salt, SaltLength);
   _read(in, ipthing, 8);

   global.m_passkey = a_passkey;

   CItemData temp;
   CMyString tempdata;

   int numread = 0;
   numread += ReadCBC(in, tempdata, salt, ipthing);
   temp.SetName(tempdata);
   numread += ReadCBC(in, tempdata, salt, ipthing);
   temp.SetPassword(tempdata);
   numread += ReadCBC(in, tempdata, salt, ipthing);
   temp.SetNotes(tempdata);
   while (numread > 0)
   {
      m_pwlist.AddTail(temp);
      numread = 0;
      numread += ReadCBC(in, tempdata, salt, ipthing);
      temp.SetName(tempdata);
      numread += ReadCBC(in, tempdata, salt, ipthing);
      temp.SetPassword(tempdata);
      numread += ReadCBC(in, tempdata, salt, ipthing);
      temp.SetNotes(tempdata);
   }

   delete [] salt;
   _close(in);

   //Shorten names if necessary
   if (GetUseDefUser())
   {
      DropDefUsernames(GetDefUsername());
   }

   return SUCCESS;
}

void PWScore::ChangePassword(const CMyString &newPassword)
{
  /*
   * To change passkeys, the data is copied into a list of CMyStrings
   * and then re-put into the list with the new passkey
   */

  /*
   * CItemData should have a ChangePasskey method instead
   */

  /*
   * Here is my latest thought on this: It is definately possible to give
   * CItemData a ChangePasskey method. However, that would involve either
   * keeping two copies of the key schedule in memory at once, which would
   * then require a lot more overhead and variables than we currently have,
   * or recreating first the current and then the new schedule for each
   * item, which would be really slow. Which is why I think that we should
   * leave well enough alone. I mean, this function does work in the end.
   */
	
  //Copies the list into a plaintext list of CMyStrings
  CList<CMyString, CMyString> tempList;
  tempList.RemoveAll();
  POSITION listPos = m_pwlist.GetHeadPosition();
  while (listPos != NULL)
    {
      CItemData temp;
      temp = m_pwlist.GetAt(listPos);
      CMyString str;
      temp.GetName(str);
      tempList.AddTail(str);
      temp.GetPassword(str);
      tempList.AddTail(str);
      temp.GetNotes(str);
      tempList.AddTail(str);
      m_pwlist.GetNext(listPos);
    }
  m_pwlist.RemoveAll();
  listPos = tempList.GetHeadPosition();

  //Changes the global password. Eck.
  global.m_passkey = newPassword;
		
  //Gets a new random value used for password authentication
  for (int x=0; x < 8; x++)
    m_randstuff[x] = newrand();
  /*
   * We generate 8 bytes of randomness, but m_randstuff
   * is larger: StuffSize bytes. This appears to be a bug,
   * let's at least explicitly zero the extra 2 bytes, since redefining
   * StuffSize to 8 would break every existing database...
   */
  m_randstuff[8] = m_randstuff[9] = '\0';

  GenRandhash(newPassword, m_randstuff, m_randhash);

  //Puts the list of CMyStrings back into CItemData
  while (listPos != NULL)
    {
      CItemData temp;
			
      temp.SetName(tempList.GetAt(listPos));
      tempList.GetNext(listPos);
			
      temp.SetPassword(tempList.GetAt(listPos));
      tempList.GetNext(listPos);

      temp.SetNotes(tempList.GetAt(listPos));
      tempList.GetNext(listPos);

      m_pwlist.AddTail(temp);
    }
  m_changed = TRUE;
}


//Finds stuff based on the .GetName() part not the entire object
POSITION
PWScore::Find(const CMyString &a_title, const CMyString &a_user)
{
   POSITION listPos = m_pwlist.GetHeadPosition();
   CMyString curthing;

   while (listPos != NULL)
   {
      m_pwlist.GetAt(listPos).GetName(curthing);
	  CMyString title, user;
	  SplitName(curthing, title, user);
      if (title == a_title && user == a_user)
         break;
      else
         m_pwlist.GetNext(listPos);
   }

   return listPos;
}


/*
  The following two functions are for use when switching default
  username states.

  Should be run only if m_usedefuser == TRUE
*/
void
PWScore::MakeFullNames(const CMyString &defusername)
{
  POSITION listPos = m_pwlist.GetHeadPosition();
  while (listPos != NULL)
    {
      CMyString temp;
      m_pwlist.GetAt(listPos).GetName(temp);
      //Start MakeFullName
      int pos = temp.FindByte(SPLTCHR);
      int pos2 = temp.FindByte(DEFUSERCHR);
      if (pos==-1 && pos2!=-1)
	{
	  //Insert defusername if string contains defchr but not splitchr
	  m_pwlist.GetAt(listPos).SetName((CMyString)temp.Left(pos2)
					  + SPLTSTR + defusername);
	}
      // End MakeFullName
      m_pwlist.GetNext(listPos);
    }
}

//Should only be run on full names...
void
PWScore::DropDefUsernames(const CMyString &defusername)
{
  POSITION listPos = m_pwlist.GetHeadPosition();
  while (listPos != NULL)
    {
      CMyString temp;
      m_pwlist.GetAt(listPos).GetName(temp);
      //Start DropDefUsername
      CMyString temptitle, tempusername;
      int pos = SplitName(temp, temptitle, tempusername);
      if ((pos!=-1) && (tempusername == defusername))
	{
	  //If name is splitable and username is default
	  m_pwlist.GetAt(listPos).SetName(temptitle + DEFUSERCHR);
	}
      //End DropDefUsername
      m_pwlist.GetNext(listPos);
    }
}




int
PWScore::SplitName(const CMyString &name,
		   CMyString &title, CMyString &username)
//Returns split position for a name that was split and -1 for non-split name
{
   int pos = name.FindByte(SPLTCHR);
   if (pos==-1) //Not a split name
   {
      int pos2 = name.FindByte(DEFUSERCHR);
      if (pos2 == -1)  //Make certain that you remove the DEFUSERCHR 
      {
         title = name;
      }
      else
      {
         title = CMyString(name.Left(pos2));
      }

      if ((pos2 != -1)
          && GetUseDefUser())
      {
         username = GetDefUsername();
      }
      else
      {
         username = _T("");
      }
   }
   else
   {
      /*
       * There should never ever be both a SPLITCHR and a DEFUSERCHR in
       * the same string
       */
      CMyString temp;
      temp = CMyString(name.Left(pos));
      temp.TrimRight();
      title = temp;
      temp = CMyString(name.Right(name.GetLength() - (pos+1))); // Zero-index string
      temp.TrimLeft();
      username = temp;
   }
   return pos;
}


void
PWScore::MakeName(CMyString& name,
		  const CMyString &title, const CMyString &username) const
{
   if (username == "")
      name = title;
   else if (GetUseDefUser()
	    && (username == GetDefUsername()))
   {
      name = title + DEFUSERCHR;
   }
   else 
   {
      name = title + SPLTSTR + username;
   }
}
