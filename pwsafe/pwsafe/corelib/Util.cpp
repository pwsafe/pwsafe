/// \file Util.cpp
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

/*
  so far, this 'util' module is heavily dependent on the app object.
  this is bad, and I will attempt to fix that - jpr
*/
#include "ThisMfcApp.h"

#include "sha1.h"
#include "resource.h"
#include "BlowFish.h"

#include <fcntl.h>
#include <errno.h>
#include <io.h>
#include <sys\stat.h>

#include "Util.h"

// for now
#define String CString
#define SecString CMyString


static void ErrorMessages(String fn, int fp);
static void xormem(unsigned char* mem1, unsigned char* mem2, int length);

//-----------------------------------------------------------------------------
/*
  Note: A bunch of the encryption-related routines may not be Unicode
  compliant.  Which really isn't a huge problem, since they are actually
  using MFC routines anyway
*/

//-----------------------------------------------------------------------------
//Overwrite the memory

void
trashMemory(SHA1_CTX& context)
{
   trashMemory((unsigned char*)context.state, sizeof context.state);
   trashMemory((unsigned char*)context.count, sizeof context.count);
   trashMemory((unsigned char*)context.buffer, sizeof context.buffer);
}


void
trashMemory(unsigned char* buffer,
            long length,
            int numiter) // default 30
{
   for (int x=0; x<numiter; x++)
   {
      memset(buffer, 0x00, length);
      memset(buffer, 0xFF, length);
      memset(buffer, 0x00, length);
   }
}


void trashMemory(CString &string)
{
   trashMemory((unsigned char*)string.GetBuffer(string.GetLength()),
               string.GetLength());
   string.ReleaseBuffer();
}



//Generates a passkey-based hash from stuff - used to validate the passkey
void
GenRandhash(CMyString a_passkey,
            unsigned char* a_randstuff,
            unsigned char* a_randhash)
{
   /*
     I'm not quite sure what this is doing, so as I figure out each piece,
     I'll add more comments {jpr}
   */

   /*
     tempSalt <- H(a_randstuff + a_passkey)
   */
   SHA1_CTX keyHash;
   SHA1Init(&keyHash);
   SHA1Update(&keyHash, a_randstuff, 10); // StuffSize
   SHA1Update(&keyHash,
              (unsigned char*)a_passkey.GetBuffer(a_passkey.GetLength()),
              a_passkey.GetLength());
   a_passkey.ReleaseBuffer();

   unsigned char tempSalt[20]; // HashSize
   SHA1Final(tempSalt, &keyHash);

   /*
     tempbuf <- a_randstuff encrypted 1000 times using tempSalt as key?
   */
	
   BlowFish Cipher(tempSalt, 20); // HashSize
	
   unsigned char tempbuf[10]; // StuffSize
   memcpy((char*)tempbuf, (char*)a_randstuff, 10); // StuffSize

   for (int x=0; x<1000; x++)
      Cipher.Encrypt(tempbuf, tempbuf);

   /*
     hmm - seems we're not done with this context
     we throw the tempbuf into the hasher, and extract a_randhash
   */
   SHA1Update(&keyHash, tempbuf, 10); // StuffSize
   SHA1Final(a_randhash, &keyHash);
   trashMemory(keyHash);
}


#if 0
int
not(int x)
{
   ASSERT((x>=0) && (x<=1));
   return 1-x;
}
#endif


unsigned char
newrand()
{
   int	r;
   while ((r = rand()) % 257 == 256)
      ; // 257?!?
   return r;
}


#if 0
BOOL
FileExists(String filename)
{
   int fp = _open(filename, _O_RDONLY);
   if (fp == -1)
      return FALSE;
   else
   {
      _close(fp);
      return TRUE;
   }
}
#endif


#if 0
windows_t
GetOSVersion()
{
   LPOSVERSIONINFO lpos;
   windows_t retval;

   lpos = (LPOSVERSIONINFO)malloc(sizeof(OSVERSIONINFO));
   lpos->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx(lpos);

   switch (lpos->dwPlatformId)
   {
   case VER_PLATFORM_WIN32s:
      retval = Win32s;
      break;
   case VER_PLATFORM_WIN32_WINDOWS:
      retval = Win95;
      break;
   case VER_PLATFORM_WIN32_NT:
      retval = WinNT;
      break;
   default:
      /*
        In case of error, may as well return the value
        that stops the program... 
      */
      retval = Win32s;
      break;
   }

   free(lpos);

   return retval;
}
#endif


char
GetRandAlphaNumChar()
{
   int temp = newrand() % 3;

   if (temp == 0)
      return ((newrand() % ('9'-'0')) + '0');
   else if (temp == 1)
      return ((newrand() % ('Z'-'A')) + 'A');
   else
      return ((newrand() % ('z'-'a')) + 'a');
}


int
_writecbc(int fp,
          CMyString string,
          unsigned char* salt,
          unsigned char* cbcbuffer)
{
   int temp = _writecbc(fp,
                        (unsigned char*)string.GetBuffer(string.GetLength()),
                        string.GetLength(),
                        salt,
                        cbcbuffer);
   string.ReleaseBuffer();
   return temp;
}

int
_writecbc(int fp,
          unsigned char* buffer,
          int length,
          unsigned char* salt,
          unsigned char* cbcbuffer)
{
   int numWritten = 0;

   int BlockLength = ((length+7)/8)*8;
   if (BlockLength == 0)
      BlockLength = 8;

   unsigned char passkey[20];
   VirtualLock(passkey, 20);
   CMyString sPasskey = app.m_passkey;
   // sPasskey?  Didn't he play Bobby Fischer? 

   SHA1_CTX context;
   SHA1Init(&context);
   SHA1Update(&context,
              (unsigned char*)sPasskey.GetBuffer(sPasskey.GetLength()),
              sPasskey.GetLength());
   sPasskey.ReleaseBuffer();
   SHA1Update(&context, salt, SaltLength);
   SHA1Final(passkey, &context);
   BlowFish Algorithm(passkey, 20);

   unsigned char lengthblock[8];
   memset(lengthblock, 0, 8);
   memcpy(lengthblock, (unsigned char*)&length, sizeof length);
   xormem(lengthblock, cbcbuffer, 8);
   Algorithm.Encrypt(lengthblock, lengthblock);
   memcpy(cbcbuffer, lengthblock, 8);
   numWritten = _write(fp, lengthblock, 8);
   trashMemory(lengthblock, 8);

   unsigned char curblock[8];
   for (int x=0;x<BlockLength;x+=8)
   {
      if ((length == 0) || ((length%8 != 0) && (length-x<8)))
      {
         //This is for an uneven last block
         memset(curblock, 0, 8);
         memcpy(curblock, buffer+x, length % 8);
      }
      else
         memcpy(curblock, buffer+x, 8);
      xormem(curblock, cbcbuffer, 8);
      Algorithm.Encrypt(curblock, curblock);
      memcpy(cbcbuffer, curblock, 8);
      numWritten += _write(fp, curblock, 8);
   }
   trashMemory(curblock, 8);

   trashMemory(passkey, 20);
   trashMemory(context);
   VirtualUnlock(passkey, 20);

   return numWritten;
}


int
_readcbc(int fp,
         unsigned char* buffer,
         unsigned int buffer_len,
         unsigned char* salt,
         unsigned char* cbcbuffer)
{
   int numRead = 0;

   unsigned char passkey[20];
   VirtualLock(passkey, 20);
   CMyString sPasskey = app.m_passkey;
   SHA1_CTX context;
   SHA1Init(&context);
   SHA1Update(&context,
              (unsigned char*)sPasskey.GetBuffer(sPasskey.GetLength()),
              sPasskey.GetLength());
   sPasskey.ReleaseBuffer();
   SHA1Update(&context, salt, SaltLength);
   SHA1Final(passkey, &context);
   BlowFish Algorithm(passkey, 20);

   unsigned char lengthblock[8];
   unsigned char lcpy[8];
   numRead = _read(fp, lengthblock, 8);
   memcpy(lcpy, lengthblock, 8);
   Algorithm.Decrypt(lengthblock, lengthblock);
   xormem(lengthblock, cbcbuffer, 8);
   memcpy(cbcbuffer, lcpy, 8);

   int length = *((int*)lengthblock);

   trashMemory(lengthblock, 8);
   trashMemory(lcpy, 8);

   if (numRead != 8)
      return 0;

   if (length < 0)
      return 0;

   if (length > buffer_len)
      return 0;
	
   int BlockLength = ((length+7)/8)*8;
   if (BlockLength == 0)
      BlockLength = 8;

   unsigned char* wholething = new unsigned char[BlockLength];
   unsigned char tempcbc[8];
   numRead += _read(fp, wholething, BlockLength);
   for (int x=0;x<BlockLength;x+=8)
   {
      memcpy(tempcbc, wholething+x, 8);
      Algorithm.Decrypt(wholething+x, wholething+x);
      xormem(wholething+x, cbcbuffer, 8);
      memcpy(cbcbuffer, tempcbc, 8);
   }
   memcpy(buffer, wholething, length);
	
   trashMemory(wholething, BlockLength);
   delete [] wholething;
   trashMemory(tempcbc, 8);

   trashMemory(passkey, 20);
   trashMemory(context);
   VirtualUnlock(passkey, 20);

   return numRead;
}


int
_readcbc(int fp,
         CMyString &deststring,
         unsigned char* salt,
         unsigned char* cbcbuffer)
{
   int numRead = 0;

   unsigned char passkey[20];
   VirtualLock(passkey, 20);
   CMyString sPasskey = app.m_passkey;
   SHA1_CTX context;
   SHA1Init(&context);
   SHA1Update(&context,
              (unsigned char*)sPasskey.GetBuffer(sPasskey.GetLength()),
              sPasskey.GetLength());
   sPasskey.ReleaseBuffer();
   SHA1Update(&context, salt, SaltLength);
   SHA1Final(passkey, &context);
   BlowFish Algorithm(passkey, 20);

   unsigned char lengthblock[8];
   unsigned char lcpy[8];
   numRead = _read(fp, lengthblock, 8);
   memcpy(lcpy, lengthblock, 8);
   Algorithm.Decrypt(lengthblock, lengthblock);
   xormem(lengthblock, cbcbuffer, 8);
   memcpy(cbcbuffer, lcpy, 8);

   int length = *((int*)lengthblock);

   trashMemory(lengthblock, 8);
   trashMemory(lcpy, 8);

   if (numRead != 8)
      return 0;

   if (length < 0)
      return 0;
	
   int BlockLength = ((length+7)/8)*8;
   if (BlockLength == 0)
      BlockLength = 8;

   deststring = "";
   char* stuff = deststring.GetBuffer(length+1);

   unsigned char* wholething = new unsigned char[BlockLength];
   unsigned char tempcbc[8];
   numRead += _read(fp, wholething, BlockLength);
   for (int x=0;x<BlockLength;x+=8)
   {
      memcpy(tempcbc, wholething+x, 8);
      Algorithm.Decrypt(wholething+x, wholething+x);
      xormem(wholething+x, cbcbuffer, 8);
      memcpy(cbcbuffer, tempcbc, 8);
   }
   memcpy(stuff, wholething, length);
   stuff[length] = '\0';
   deststring.ReleaseBuffer();
   trashMemory(wholething, BlockLength);
   delete [] wholething;
   trashMemory(tempcbc, 8);

   trashMemory(passkey, 20);
   trashMemory(context);
   VirtualUnlock(passkey, 20);

   return numRead;
}


#if 0
int
_readToCMyString(int fp, CMyString &deststring)
{
   int numRead = 0;
   int length = (int) _filelength(fp);

   deststring = "";
   char* stuff = deststring.GetBuffer(length+1);

   numRead += _read(fp, stuff, length);

   stuff[length] = '\0';

   deststring.ReleaseBuffer();
   
   return numRead;
}


int
_writeFromCMyString(int fp, CMyString &source)
{
   int numRead = 0;
   char* stuff = source.GetBuffer(source.GetLength());

   numRead = _write(fp, stuff, source.GetLength());
   
   source.ReleaseBuffer();
   
   return numRead;
}
#endif


// I think this is only for the CBC routines...
static void
xormem(unsigned char* mem1, unsigned char* mem2, int length)
{
   for (int x=0;x<length;x++)
      mem1[x] ^= mem2[x];
}


int
SplitName(CMyString name, CMyString &title, CMyString &username)
//Returns split position for a name that was split and -1 for non-split name
{
   int pos = name.Find(SPLTCHR);
   if (pos==-1) //Not a split name
   {
      int pos2 = name.Find(DEFUSERCHR);
      if (pos2 == -1)  //Make certain that you remove the DEFUSERCHR 
      {
         title = name;
      }
      else
      {
         CString temp = name.Left(pos2);
         title = (CMyString)temp;
         trashMemory(temp);
      }

      if ((pos2 != -1)
          && (app.GetProfileInt("", "usedefuser", FALSE)==TRUE))
      {
         CString temp = app.GetProfileString("", "defusername", "");
         username = (CMyString)temp;
         trashMemory(temp);
      }
      else
      {
         username = "";
      }
   }
   else
   {
      /*
       * There should never ever be both a SPLITCHR and a DEFUSERCHR in
       * the same string
       */
      CString temp;
      temp = name.Left(pos);
      temp.TrimRight();
      title = (CMyString)temp;
      trashMemory(temp);
      temp = name.Right(name.GetLength() - (pos+1)); // Zero-index string
      temp.TrimLeft();
      username = (CMyString)temp;
      trashMemory(temp);
   }
   return pos;
}


void
MakeName(CMyString& name, CMyString title, CMyString username)
{
   if (username == "")
      name = title;
   else if (((app.GetProfileInt("", "usedefuser", FALSE))==TRUE)
            && (username.m_mystring ==
                app.GetProfileString("", "defusername", "")))
   {
      name = title + DEFUSERCHR;
   }
   else 
   {
      name = title + SPLTSTR + username;
   }
}

/*
  The following two functions are for use when switching default
  username states.

  They are only called from CPasswordSafeDlg::OnOptions 

  Should be run only if usedefuser == TRUE
*/
void
MakeFullNames(CList<CItemData, CItemData>* plist,
              CMyString defusername)
{
   POSITION listPos = plist->GetHeadPosition();
   while (listPos != NULL)
   {
      CMyString temp;
      plist->GetAt(listPos).GetName(temp);
      //Start MakeFullName
      int pos = temp.Find(SPLTCHR);
      int pos2 = temp.Find(DEFUSERCHR);
      if (pos==-1 && pos2!=-1)
      {
         //Insert defusername if string contains defchr but not splitchr
         plist->GetAt(listPos).SetName(
            (CMyString)temp.Left(pos2) + SPLTSTR + defusername);
      }
      // End MakeFullName
      plist->GetNext(listPos);
   }
}


//Should only be run on full names...
void
DropDefUsernames(CList<CItemData, CItemData>* plist, CMyString defusername)
{
   POSITION listPos = plist->GetHeadPosition();
   while (listPos != NULL)
   {
      CMyString temp;
      plist->GetAt(listPos).GetName(temp);
      //Start DropDefUsername
      CMyString temptitle, tempusername;
      int pos = SplitName(temp, temptitle, tempusername);
      if ((pos!=-1) && (tempusername == defusername))
      {
         //If name is splitable and username is default
         plist->GetAt(listPos).SetName(temptitle + DEFUSERCHR);
      }
      //End DropDefUsername
      plist->GetNext(listPos);
   }
}


int
CheckVersion(CList<CItemData, CItemData>* plist)
{
   POSITION listPos = plist->GetHeadPosition();
   while (listPos != NULL)
   {
      CMyString temp;
      plist->GetAt(listPos).GetName(temp);

      if (temp.Find(SPLTCHR) != -1)
         return V15;

      plist->GetNext(listPos);
   }
   
   return V10;
}


void
SetBlankToDef(CList<CItemData, CItemData>* plist)
{
   POSITION listPos = plist->GetHeadPosition();
   while (listPos != NULL)
   {
      CMyString temp;
      plist->GetAt(listPos).GetName(temp);

      //Start Check
      if ((temp.Find(SPLTCHR) == -1)
          && (temp.Find(DEFUSERCHR) == -1))
      {
         plist->GetAt(listPos).SetName(temp + DEFUSERCHR);
      }
      //End Check

      plist->GetNext(listPos);
   }
}


void
SetBlankToName(CList<CItemData, CItemData>* plist, CMyString username)
{
   POSITION listPos = plist->GetHeadPosition();
   while (listPos != NULL)
   {
      CMyString temp;
      plist->GetAt(listPos).GetName(temp);
      //Start Check
      if ( (temp.Find(SPLTCHR) == -1) && (temp.Find(DEFUSERCHR) == -1) )
      {
         plist->GetAt(listPos).SetName(temp + SPLTSTR + username);
      }
      //End Check
      plist->GetNext(listPos);
   }
}


BOOL
CheckExtension(CMyString name, CMyString ext)
{
   int pos = name.Find(ext);
   return (pos == name.GetLength() - ext.GetLength()); //Is this at the end??
}

//-----------------------------------------------------------------------------

//Complain if the file has not opened correctly

static void
ErrorMessages(String fn, int fp)
{
   if (fp==-1)
   {
      String text;
      text = "A fatal error occured: ";

      if (errno==EACCES)
         text += "Given path is a directory or file is read-only";
      else if (errno==EEXIST)
         text += "The filename already exists.";
      else if (errno==EINVAL)
         text += "Invalid oflag or shflag argument.";
      else if (errno==EMFILE)
         text += "No more file handles available.";
      else if (errno==ENOENT)
         text += "File or path not found.";
      text += "\nProgram will terminate.";

      String title = "Password Safe - " + fn;
      AfxGetMainWnd()->MessageBox(text, title, MB_ICONEXCLAMATION|MB_OK);
   }
}

//-----------------------------------------------------------------------------
#if defined(WITH_LEGACY_CMDLINE)

static void
_encryptFile(CString filepath)
{
   CString out_filepath;
   int len;
   unsigned char* buf;

   int in = _open(filepath,
                  _O_BINARY|_O_RDONLY|_O_SEQUENTIAL,
                  S_IREAD | _S_IWRITE);
   if (in != -1)
   {
      len = _filelength(in);
      buf = new unsigned char[len];

      _read(in, buf, len);

      _close(in);
   }
   else
   {
      ErrorMessages(filepath, in);
      return;
   }

   out_filepath = filepath;
   out_filepath += CIPHERTEXT_SUFFIX;

   int out = _open(out_filepath,
                   _O_BINARY|_O_WRONLY|_O_SEQUENTIAL|_O_TRUNC|_O_CREAT,
                   _S_IREAD | _S_IWRITE);
   if (out != -1)
   {
      _write(out, &len, sizeof(len));
		
      unsigned char* thesalt = new unsigned char[SaltLength];
      for (int x=0;x<SaltLength;x++)
         thesalt[x] = newrand();
      _write(out, thesalt, SaltLength);
		
      unsigned char ipthing[8];
      for (x=0;x<8;x++)
         ipthing[x] = newrand();
      _write(out, ipthing, 8);

      _writecbc(out, buf, len, thesalt, ipthing);
		
      _close(out);

      delete [] thesalt;
   }
   else
      ErrorMessages(out_filepath, out);

   delete [] buf;
}


static void
_decryptFile(CString filepath)
{
   CString out_filepath;
   int len;
   unsigned char* buf;

   int in = _open(filepath,
                  _O_BINARY|_O_RDONLY|_O_SEQUENTIAL,
                  S_IREAD | _S_IWRITE);
   if (in != -1)
   {
      unsigned char* salt = new unsigned char[SaltLength];
      unsigned char ipthing[8];

      _read(in, &len, sizeof(len));
      buf = new unsigned char[len];

      _read(in, salt, SaltLength);
      _read(in, ipthing, 8);
      _readcbc(in, buf, len, salt, ipthing);
		
      delete [] salt;
      _close(in);
   }
   else
   {
      ErrorMessages(filepath, in);
      return;
   }

   int suffix_len = strlen(CIPHERTEXT_SUFFIX);
   int filepath_len = strlen(filepath);

   out_filepath = filepath;
   out_filepath = out_filepath.Left(filepath_len - suffix_len);

   int out = _open(out_filepath,
                   _O_BINARY|_O_WRONLY|_O_SEQUENTIAL|_O_TRUNC|_O_CREAT,
                   _S_IREAD | _S_IWRITE);
   if (out != -1)
   {
      _write(out, buf, len);
		
      _close(out);
   }
   else
      ErrorMessages(out_filepath, out);

   delete [] buf;
}


static void
convertToLongFilePath(CString &filepath)
{
   // find the length of the filename
   CFile fTmp(filepath, CFile::typeBinary);
   int len_filename = strlen(fTmp.GetFileName());

   //Preserve the extension
   int extlength =
      fTmp.GetFileName().GetLength() - fTmp.GetFileTitle().GetLength();
   CString ext = fTmp.GetFileName().Right(extlength);

   // find the long filename
   SHFILEINFO tempSHFileInfo;
   SHGetFileInfo(filepath, 0, &tempSHFileInfo,
                 sizeof(tempSHFileInfo), SHGFI_DISPLAYNAME);

   // strip off the short filename
   filepath = filepath.Left(filepath.GetLength() - len_filename);

   // append the long filename to the path
   filepath += tempSHFileInfo.szDisplayName;

   //Readd the extension if necessary
   if (filepath.Right(extlength) != ext)
      filepath += ext;
}


void
manageCmdLine(CString a_cmdline)
{
   CString filepath;
   CString suffix;
   int len = 0;

   while (len != -1)
   {
      len = a_cmdline.Find(' ');
      if (len == -1) // we've hit the NULL
         filepath = a_cmdline;
      else
      {
         filepath = a_cmdline.Left(len);
         a_cmdline = a_cmdline.Right(a_cmdline.GetLength() - len - 1);
      }
      convertToLongFilePath(filepath);

      suffix = filepath.Right(strlen(CIPHERTEXT_SUFFIX));
		
      if (suffix == CIPHERTEXT_SUFFIX)
      {
         //MessageBox(filepath, "Decrypting File...", MB_ICONINFORMATION);
         _decryptFile(filepath);
      }
      else
      {
         //MessageBox(filepath, "Encrypting File...", MB_ICONINFORMATION);
         _encryptFile(filepath);
      }
   }
}


#endif // WITH_LEGACY_CMDLINE
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
