#include "PWSfile.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

bool PWSfile::FileExists(const CMyString &filename)
{
  struct _stat statbuf;
  int status;

  status = ::_tstat(filename, &statbuf);
  return (status == 0);
}

PWSfile::PWSfile(const CMyString &filename, const CMyString &passkey)
  : m_filename(filename), m_passkey(passkey), m_curversion(UNKNOWN_VERSION), m_fd(-1)
{
}

PWSfile::~PWSfile()
{
  CloseFile(); // idempotent
}

// Used to warn pre-2.0 users, and to identify the database as 2.x:
static const CMyString V2ItemName(" !!!Version 2 File Format!!! "
				  "Please upgrade to PasswordSafe 2.0"
				  " or later");
// Used to specify the exact version
static const CMyString VersionString("pre-2.0");

int PWSfile::WriteV2Header()
{
  CItemData header;
  // Fill out with V2-specific info
  header.SetName(V2ItemName);
  header.SetPassword(VersionString);
  // need to fallback to V17, since the record
  // won't be readable otherwise!
  VERSION sv = m_curversion;
  m_curversion = V17;
  int status = WriteRecord(header);
  // restore after writing V17-format header
  m_curversion = sv;
  return status;
}

int PWSfile::ReadV2Header()
{
  CItemData header;
  // need to fallback to V17, since the header
  // is always written in this format
  VERSION sv = m_curversion;
  m_curversion = V17;
  int status = ReadRecord(header);
  // restore after reading V17-format header
  m_curversion = sv;
  if (status == SUCCESS) {
    const CMyString name = header.GetName();
    // XXX Need to compare header.GetPassword() against VersionString
    // XXX as well, for inter-2.x version checks
    status = (name == V2ItemName) ? SUCCESS : WRONG_VERSION;
  }
  return status;
}

int PWSfile::OpenWriteFile(VERSION v)
{
  if (v != V17 && v != V20)
    return UNSUPPORTED_VERSION;

  m_fd = _open((LPCTSTR)m_filename,
	       _O_BINARY|_O_WRONLY|_O_SEQUENTIAL|_O_TRUNC|_O_CREAT,
	       _S_IREAD | _S_IWRITE);

  if (m_fd == -1)
    return CANT_OPEN_FILE;

  m_curversion = v;
  
  // Following used to verify passkey against file's passkey
  unsigned char randstuff[StuffSize];
  unsigned char randhash[20];   // HashSize
  int x;

  for (x=0; x<8; x++)
    randstuff[x] = newrand();
  randstuff[8] = randstuff[9] = '\0';
  GenRandhash(m_passkey, randstuff, randhash);

  _write(m_fd, randstuff, 8);
  _write(m_fd, randhash, 20);

  for (x=0; x<SaltLength; x++)
    m_salt[x] = newrand();

  _write(m_fd, m_salt, SaltLength);
	
  for (x=0; x<8; x++)
    m_ipthing[x] = newrand();
  _write(m_fd, m_ipthing, 8);

  if (v == V20) {
    int status = WriteV2Header();
    if (status != SUCCESS)
      return status;
  }

  return SUCCESS;
}

int PWSfile::OpenReadFile(VERSION v)
{
  if (v != V17 && v != V20)
    return UNSUPPORTED_VERSION;

  m_fd = _open((LPCTSTR) m_filename,
	       _O_BINARY |_O_RDONLY | _O_SEQUENTIAL,
	       S_IREAD | _S_IWRITE);

  if (m_fd == -1)
    return CANT_OPEN_FILE;
  m_curversion = v;

  int status = CheckPassword();

  if (status != SUCCESS) {
    CloseFile();
    return status;
  }

   _read(m_fd, m_salt, SaltLength);
   _read(m_fd, m_ipthing, 8);

   if (v == V20)
     status = ReadV2Header();

  return status;
}

void PWSfile::CloseFile()
{
  if (m_fd != -1) {
    _close(m_fd);
    m_fd = -1;
  }
}

PWSfile::VERSION PWSfile::GetFileVersion()
{
  VERSION v;
  int status = OpenReadFile(V20);
  CloseFile();
  switch (status) {
  case SUCCESS: v = V20; break;
  case WRONG_VERSION: v = V17; break;
  default: v = UNKNOWN_VERSION; break;
  }
  return v;
}

int PWSfile::CheckPassword()
{
  // if file was opened, leave it open,
  // else open it for check, close when done
  const bool was_open = (m_fd != -1);

  if (!was_open) {
    m_fd = _open((LPCTSTR) m_filename,
		 _O_BINARY |_O_RDONLY | _O_SEQUENTIAL,
		 S_IREAD | _S_IWRITE);

    if (m_fd == -1)
      return CANT_OPEN_FILE;
  }

  unsigned char randstuff[StuffSize];
  unsigned char randhash[20];   // HashSize

   _read(m_fd, randstuff, 8);
   randstuff[8] = randstuff[9] = '\0'; // Gross fugbix
   _read(m_fd, randhash, 20);

  if (!was_open)
    CloseFile();

  unsigned char temphash[20]; // HashSize
  GenRandhash(m_passkey, randstuff, temphash);

  if (0 != ::memcmp((char*)randhash,
		    (char*)temphash,
		    20)) {// HashSize
    return WRONG_PASSWORD;
  } else {
    return SUCCESS;
  }
}

int PWSfile::WriteCBC(unsigned char type, const CString &data)
{
  // We do a double cast because the LPCSTR cast operator is overridden by the CString class
  // to access the pointer we need,
  // but we in fact need it as an unsigned char. Grrrr.
  LPCSTR passstr = LPCSTR(m_passkey);
  LPCSTR datastr = LPCSTR(data);

  return _writecbc(m_fd, (const unsigned char *)datastr, data.GetLength(), type,
		   (const unsigned char *)passstr, m_passkey.GetLength(),
		   m_salt, SaltLength, m_ipthing);
}


int PWSfile::WriteRecord(const CItemData &item)
{
  ASSERT(m_fd != -1);
  ASSERT(m_curversion != UNKNOWN_VERSION);

  switch (m_curversion) {
  case V17: {
    // 1.x programs totally ignore the type byte, hence safe to write it
    // (no need for two WriteCBC functions)
    WriteCBC(CItemData::NAME, item.GetName());
    WriteCBC(CItemData::PASSWORD, item.GetPassword());
    WriteCBC(CItemData::NOTES, item.GetNotes());
    return SUCCESS;
  }
  break;
  case V20: {
    WriteCBC(CItemData::TITLE, item.GetTitle());
    WriteCBC(CItemData::USER, item.GetUser());
    WriteCBC(CItemData::PASSWORD, item.GetPassword());
    WriteCBC(CItemData::NOTES, item.GetNotes());
    WriteCBC(CItemData::END, _T(""));
    return SUCCESS;
  }
  default:
    ASSERT(0);
    return UNSUPPORTED_VERSION;
  }
  return SUCCESS;
}

int
PWSfile::ReadCBC(unsigned char &type, CMyString &data)
{
  // We do a double cast because the LPCSTR cast operator is overridden by the CString class
  // to access the pointer we need,
  // but we in fact need it as an unsigned char. Grrrr.
  LPCSTR passstr = LPCSTR(m_passkey);

  unsigned char *buffer = NULL;
  unsigned int buffer_len = 0;
  int retval;

  retval = _readcbc(m_fd, buffer, buffer_len, type,
		   (const unsigned char *)passstr, m_passkey.GetLength(),
		   m_salt, SaltLength, m_ipthing);
  if (buffer_len > 0) {
    CMyString str(LPCSTR(buffer), buffer_len);
    data = str;
    trashMemory(buffer, buffer_len);
    delete[] buffer;
  } else {
    data = _T("");
    // no need to delete[] buffer, since _readcbc will not allocate if
    // buffer_len is zero
  }
  return retval;
}



int PWSfile::ReadRecord(CItemData &item)
{
  ASSERT(m_fd != -1);
  ASSERT(m_curversion != UNKNOWN_VERSION);

  CMyString tempdata;  
  int numread = 0;
  unsigned char type;

  switch (m_curversion) {
  case V17: {
    // type is meaningless, but why write two versions of ReadCBC?
    numread += ReadCBC(type, tempdata);
    item.SetName(tempdata);
    numread += ReadCBC(type, tempdata);
    item.SetPassword(tempdata);
    numread += ReadCBC(type, tempdata);
    item.SetNotes(tempdata);

    return (numread > 0) ? SUCCESS : END_OF_FILE;
  }
  case V20: {
    int emergencyExit = 255; // to avoid endless loop.
    int fieldLen; // zero means end of file reached
    bool endFound = false; // set to true when record end detected - happy end
    do {
      fieldLen = ReadCBC(type, tempdata);
      if (fieldLen > 0) {
	numread += fieldLen;
	switch (type) {
	case CItemData::TITLE:
	  item.SetTitle(tempdata); break;
	case CItemData::USER:
	  item.SetUser(tempdata); break;
	case CItemData::PASSWORD:
	  item.SetPassword(tempdata); break;
	case CItemData::NOTES:
	  item.SetNotes(tempdata); break;
	case CItemData::END:
	  endFound = true; break;
	  // just silently ignore fields we don't support.
	  // this is forward compatability...
	case CItemData::GUID:
	case CItemData::GROUP:
	case CItemData::CTIME:
	case CItemData::MTIME:
	case CItemData::ATIME:
	case CItemData::LTIME:
	case CItemData::POLICY:
	default:
	  // XXX Set a flag here so user can be warned that
	  // XXX we read a file format we don't fully support
	  break;
	} // switch
      } // if (fieldLen > 0)
    } while (!endFound && fieldLen > 0 && --emergencyExit > 0);
    return (numread > 0) ? SUCCESS : END_OF_FILE;
  }
  default:
    ASSERT(0);
    return UNSUPPORTED_VERSION;
  }
}

