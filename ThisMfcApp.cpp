/// \file ThisMfcApp.cpp
/// \brief App object of MFC version of Password Safe
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include "ThisMfcApp.h"
#include "Util.h"
#include "DboxMain.h"
#include "resource.h"

#include "CryptKeyEntry.h"




//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(ThisMfcApp, CWinApp)
//   ON_COMMAND(ID_HELP, CWinApp::OnHelp)
   ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()


ThisMfcApp::ThisMfcApp() :
	m_bUseAccelerator( true ),
	m_pMRU( NULL )
{
   srand((unsigned)time(NULL));
}


ThisMfcApp::~ThisMfcApp()
{
	if ( m_pMRU )
	{
		m_pMRU->WriteList();
		delete m_pMRU;
	}

   /*
     apparently, with vc7, there's a CWinApp::HtmlHelp - I'd like
     to see the docs, someday.  In the meantime, force with :: syntax
   */

   ::HtmlHelp(NULL, NULL, HH_CLOSE_ALL, 0);
}

static void Usage()
{
  AfxMessageBox("Usage: PasswordSafe [password database]\n"
		"or PasswordSafe [-e|-d] filename");
}

// tests if file exists, returns empty string if so, displays error message if not
static BOOL CheckFile(const CString &fn)
{
  DWORD status = ::GetFileAttributes(fn);
  CString ErrMess;

  if (status == -1) {
    ErrMess = _T("Could not access file: ");
    ErrMess += fn;
  } else if (status & FILE_ATTRIBUTE_DIRECTORY) {
    ErrMess = fn;
    ErrMess += _T(" is a directory");
  }

  if (ErrMess.IsEmpty()) {
    return TRUE;
  } else {
    AfxMessageBox(ErrMess);
    return FALSE;
  }
}

//Complain if the file has not opened correctly

static void
ErrorMessages(const CString &fn, int fp)
{
  if (fp==-1)
    {
      CString text;
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

      CString title = "Password Safe - " + fn;
      AfxGetMainWnd()->MessageBox(text, title, MB_ICONEXCLAMATION|MB_OK);
    }
}

static BOOL EncryptFile(const CString &fn, const CMyString &passwd)
{
  unsigned int len;
  unsigned char* buf;

  int in = _open(fn,
		 _O_BINARY|_O_RDONLY|_O_SEQUENTIAL,
		 S_IREAD | _S_IWRITE);
  if (in != -1) {
    len = _filelength(in);
    buf = new unsigned char[len];

    _read(in, buf, len);

    _close(in);
  } else {
    ErrorMessages(fn, in);
    return FALSE;
  }

  CString out_fn = fn;
  out_fn += CIPHERTEXT_SUFFIX;

  int out = _open(out_fn,
		  _O_BINARY|_O_WRONLY|_O_SEQUENTIAL|_O_TRUNC|_O_CREAT,
		  _S_IREAD | _S_IWRITE);
  if (out != -1) {
#ifdef KEEP_FILE_MODE_BWD_COMPAT
    _write(out, &len, sizeof(len)); // XXX portability issue!
#else
    for (int i=0; i < 8; i++)
      global.m_randstuff[i] = newrand();

    // miserable bug - have to fix this way to avoid breaking existing files
    global.m_randstuff[8] = global.m_randstuff[9] = '\0';
    GenRandhash(passwd,
		global.m_randstuff,
		global.m_randhash);
   _write(out, global.m_randstuff, 8);
   _write(out, global.m_randhash, 20);
#endif // KEEP_FILE_MODE_BWD_COMPAT
		
    unsigned char thesalt[SaltLength];
    for (int x=0;x<SaltLength;x++)
      thesalt[x] = newrand();
    _write(out, thesalt, SaltLength);
		
    unsigned char ipthing[8];
    for (x=0;x<8;x++)
      ipthing[x] = newrand();
    _write(out, ipthing, 8);

    LPCSTR pwd = LPCSTR(passwd);
    _writecbc(out, buf, len,
	      (unsigned char *)pwd, passwd.GetLength(),
	      thesalt, SaltLength,
	      ipthing);
		
    _close(out);

  } else {
    ErrorMessages(out_fn, out);
    delete [] buf;
    return FALSE;
  }
  delete[] buf;
  return TRUE;
}

static BOOL DecryptFile(const CString &fn, const CMyString &passwd)
{
  unsigned int len;
  unsigned char* buf;

  int in = _open(fn,
		 _O_BINARY|_O_RDONLY|_O_SEQUENTIAL,
		 S_IREAD | _S_IWRITE);
  if (in != -1) {
      unsigned char salt[SaltLength];
      unsigned char ipthing[8];

#ifdef KEEP_FILE_MODE_BWD_COMPAT
      _read(in, &len, sizeof(len)); // XXX portability issue
#else
      _read(in, global.m_randstuff, 8);
      global.m_randstuff[8] = global.m_randstuff[9] = '\0'; // ugly bug workaround
      _read(in, global.m_randhash, 20);

      unsigned char temphash[20]; // HashSize
      GenRandhash(passwd,
		  global.m_randstuff,
		  temphash);
      if (0 != memcmp((char*)global.m_randhash,
		      (char*)temphash,
		      20)) // HashSize
	{
	  _close(in);
	  AfxMessageBox(_T("Incorrect password"));
	  return FALSE;
	}
#endif // KEEP_FILE_MODE_BWD_COMPAT
      buf = NULL; // allocated by _readcbc - see there for apologia

      _read(in, salt, SaltLength);
      _read(in, ipthing, 8);
      LPCSTR pwd = LPCSTR(passwd);
      if (_readcbc(in, buf, len,
		   (unsigned char *)pwd, passwd.GetLength(),
		   salt, SaltLength, ipthing) == 0) {
	delete[] buf; // if not yet allocated, delete[] NULL, which is OK
	return FALSE;
      }
		
      _close(in);
    } else {
      ErrorMessages(fn, in);
      return FALSE;
    }

  int suffix_len = strlen(CIPHERTEXT_SUFFIX);
  int filepath_len = fn.GetLength();

  CString out_fn = fn;
  out_fn = out_fn.Left(filepath_len - suffix_len);

  int out = _open(out_fn,
		  _O_BINARY|_O_WRONLY|_O_SEQUENTIAL|_O_TRUNC|_O_CREAT,
		  _S_IREAD | _S_IWRITE);
  if (out != -1) {
    _write(out, buf, len);
    _close(out);
    } else
      ErrorMessages(out_fn, out);

  delete[] buf; // allocated by _readcbc
  return TRUE;
}

BOOL
ThisMfcApp::InitInstance()
{



   /*
    * It's always best to start at the beginning.  [Glinda, Witch of the North]
    */

   /*
     this pulls 'Counterpane Systems' out of the string table, verifies
     that it exists (actually, only verifies that *some* IDS_COMPANY
     string exists -- it could be 'Microsoft' for all we know), and then
     instructs the app to use the registry instead of .ini files.  The
     path ends up being

     HKEY_CURRENT_USER\Software\(companyname)\(appname)\(sectionname)\(valuename)

     Assuming the open-source version of this is going to become less
     Counterpane-centric, I expect this may change, but if it does, an
     automagic migration ought to happen. -- {jpr}
   */

   CMyString companyname;
   VERIFY(companyname.LoadString(IDS_COMPANY) != 0);
   SetRegistryKey(companyname);

   int	nMRUItems = GetProfileInt("", "maxmruitems", 4);
   m_pMRU = new CRecentFileList( 0, "MRU", "Safe%d", nMRUItems );;
   m_pMRU->ReadList();

   DboxMain dbox(NULL);

   /*
    * Command line processing:
    * Historically, it appears that if a filename was passed as a commadline argument,
    * the application would prompt the user for the password, and the encrypt or decrypt
    * the named file, based on the file's suffix. Ugh.
    *
    * What I'll do is as follows:
    * If a file is given in the command line, it is used as the database, overriding the
    * registry value. This will allow the user to have several databases, say, one for work
    * and one for personal use, and to set up a different shortcut for each.
    *
    * I think I'll keep the old functionality, but activate it with a "-e" or "-d" flag. (ronys)
    */

   if (m_lpCmdLine[0] != '\0') {
     CString args = m_lpCmdLine;

     if (args[0] != _T('-')) {

       if (CheckFile(args)) {
	 dbox.SetCurFile(args);
       } else {
	 return FALSE;
       }
     } else { // here if first char of arg is '-'
       // first, let's check that there's a second arg
       CString fn = args.Right(args.GetLength()-2);
       fn.TrimLeft();
       if (fn.IsEmpty() || !CheckFile(fn)) {
	 Usage();
	 return FALSE;
       }

       CMyString passkey;
       if (args[1] == 'e' || args[1] == 'E' || args[1] == 'd' || args[1] == 'D') {
	 // get password from user if valid flag given. If note, default below will
	 // pop usage message
	 CCryptKeyEntry dlg(NULL);
	 int nResponse = dlg.DoModal();

	 if (nResponse==IDOK) {
	     passkey = dlg.m_cryptkey1;
	 } else {
	   return FALSE;
	 }
       }
       BOOL status;
       switch (args[1]) {
       case 'e': case 'E': // do encrpytion
	 status = EncryptFile(fn, passkey);
	 if (!status) {
	   AfxMessageBox("Encryption failed");
	 }
	 break;
       case 'd': case 'D': // do decryption
	 status = DecryptFile(fn, passkey);
	 if (!status) {
	   // nothing to do - DecryptFile displays its own error messages
	 }
	 break;
       default:
	 Usage();
	 return FALSE;
       } // switch
       return FALSE;
     } // else
   } // m_lpCmdLine[0] != '\0';
       
   /*
    * normal startup
    */

   /*
     Here's where PWS currently does DboxMain, which in turn will do
     the initial PasskeyEntry (the one that looks like a splash screen).
     This makes things very hard to control.
     The app object (here) should instead do the initial PasskeyEntry,
     and, if successful, move on to DboxMain.  I think. {jpr}
    */
   m_maindlg = &dbox;
   //  This doesn't do anything as best I can tell.
   //m_pMainWnd = m_maindlg;

   // Set up an Accelerator table
   m_ghAccelTable = LoadAccelerators(AfxGetInstanceHandle(),
                                     MAKEINTRESOURCE(IDR_ACCS));
   //Run dialog
   (void) dbox.DoModal();

   /*
     note that we don't particularly care what the response was
   */


   // Since the dialog has been closed, return FALSE so that we exit the
   //  application, rather than start the application's message pump.

   return FALSE;
}


//Copied from Knowledge Base article Q100770
BOOL
ThisMfcApp::ProcessMessageFilter(int code,
                                 LPMSG lpMsg)
{
   if (code < 0)
      CWinApp::ProcessMessageFilter(code, lpMsg);
	
   if (m_bUseAccelerator &&
	   m_maindlg != NULL
       && m_ghAccelTable != NULL)
   {
      if (::TranslateAccelerator(m_maindlg->m_hWnd, m_ghAccelTable, lpMsg))
         return TRUE;
   }
   return CWinApp::ProcessMessageFilter(code, lpMsg);
}


void
ThisMfcApp::OnHelp()
{
   ::HtmlHelp(NULL,
              "pwsafe.chm::/html/pws_intro.htm",
              HH_DISPLAY_TOPIC, 0);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
