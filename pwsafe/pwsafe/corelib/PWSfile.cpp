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

int PWSfile::RenameFile(const CMyString &oldname, const CMyString &newname)
{
  _tremove(newname); // otherwise rename will fail if newname exists
  int status = _trename(oldname, newname);

  return (status == 0) ? SUCCESS : FAILURE;
}


PWSfile::PWSfile(const CMyString &filename, const CMyString &passkey)
  : m_filename(filename), m_passkey(passkey),  m_defusername(_T("")),
    m_curversion(UNKNOWN_VERSION), m_fd(NULL), m_prefString(_T(""))
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
static const CMyString VersionString("2.0");

int PWSfile::WriteV2Header()
{
  CItemData header;
  // Fill out with V2-specific info
  // To make a dictionary attack a little harder, we make the length
  // of the first record (Name) variable, by appending variable length randomness
  // to the fixed string
  unsigned int rlen = RangeRand(62) + 2; // 64 is a trade-off...
  char *rbuf = new char[rlen];
  GetRandomData(rbuf, rlen-1);
  rbuf[rlen-1] = '\0'; // although zero may be there before - who cares?
  CMyString rname(V2ItemName);
  rname += rbuf;
  delete[] rbuf;
  header.SetName(rname, _T(""));
  header.SetPassword(VersionString);
  header.SetNotes(m_prefString);
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
    const CMyString version = header.GetPassword();
    status = (version == VersionString) ? SUCCESS : WRONG_VERSION;
  }
  if (status == SUCCESS)
    m_prefString = header.GetNotes();
  return status;
}

int PWSfile::OpenWriteFile(VERSION v)
{
  if (v != V17 && v != V20)
    return UNSUPPORTED_VERSION;

#ifdef UNICODE
	m_fd = _wfopen((LPCTSTR)m_filename, _T("wb") );
#else
	m_fd = fopen((LPCTSTR)m_filename, _T("wb") );
#endif

  if (m_fd == NULL)
    return CANT_OPEN_FILE;

  m_curversion = v;
  
  // Following used to verify passkey against file's passkey
  unsigned char randstuff[StuffSize];
  unsigned char randhash[20];   // HashSize

  GetRandomData( randstuff, 8 );
  randstuff[8] = randstuff[9] = '\0';
  GenRandhash(m_passkey, randstuff, randhash);

  fwrite(randstuff, 1, 8, m_fd);
  fwrite(randhash, 1, 20, m_fd);

  GetRandomData( m_salt, SaltLength );

  fwrite(m_salt, 1, SaltLength, m_fd);
	
  GetRandomData( m_ipthing, 8 );
  fwrite(m_ipthing, 1, 8, m_fd);

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

#ifdef UNICODE
	  m_fd = _wfopen((LPCTSTR) m_filename, _T("rb"));
#else
	  m_fd = fopen((LPCTSTR) m_filename, _T("rb"));
#endif

  if (m_fd == NULL)
    return CANT_OPEN_FILE;
  m_curversion = v;

  int status = CheckPassword();

  if (status != SUCCESS) {
    CloseFile();
    return status;
  }

   fread(m_salt, 1, SaltLength, m_fd);
   fread(m_ipthing, 1, 8, m_fd);

   if (v == V20)
     status = ReadV2Header();

  return status;
}

void PWSfile::CloseFile()
{
  if (m_fd != NULL) {
    fclose(m_fd);
    m_fd = NULL;
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
  const bool was_open = (m_fd != NULL);

  if (!was_open) {
#ifdef UNICODE
	  m_fd = _wfopen((LPCTSTR) m_filename, _T("rb"));
#else
	  m_fd = fopen((LPCTSTR) m_filename, _T("rb"));
#endif

    if (m_fd == NULL)
      return CANT_OPEN_FILE;
  }

  unsigned char randstuff[StuffSize];
  unsigned char randhash[20];   // HashSize

   fread(randstuff, 1, 8, m_fd);
   randstuff[8] = randstuff[9] = '\0'; // Gross fugbix
   fread(randhash, 1, 20, m_fd);

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

int PWSfile::WriteCBC(unsigned char type, const unsigned char *data, unsigned int length)
{
  // We do a double cast because the LPCSTR cast operator is overridden by the CString class
  // to access the pointer we need,
  // but we in fact need it as an unsigned char. Grrrr.
  LPCSTR passstr = LPCSTR(m_passkey);

  return _writecbc(m_fd, data, length, type,
		   (const unsigned char *)passstr, m_passkey.GetLength(),
		   m_salt, SaltLength, m_ipthing);
}


int PWSfile::WriteRecord(const CItemData &item)
{
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion != UNKNOWN_VERSION);

  switch (m_curversion) {
  case V17: {
    // 1.x programs totally ignore the type byte, hence safe to write it
    // (no need for two WriteCBC functions)
    // Note that 2.0 format still requires that the header be in this format,
    // So that old programs reading new databases won't crash,
    // This introduces a small security issue, in that the header is known text,
    // making the password susceptible to a dictionary attack on the first block,
    // rather than the hash^n in the beginning of the file.
    // we can help minimize this here by writing a random byte in the "type"
    // byte of the first block.

    CMyString name = item.GetName();
    // If name field already exists - use it. This is for the 2.0 header, as well as for files
    // that were imported and re-exported.
    if (name.IsEmpty()) {
      // The name in 1.7 consists of title + SPLTCHR + username
      // DEFUSERNAME was used in previous versions, but 2.0 converts this upon import
      // so it is not an issue here.
      // Prepend 2.0 group field to name, if not empty
      // i.e. group "finances" name "broker" -> "finances.broker"
      CMyString group = item.GetGroup();
      CMyString title = item.GetTitle();
      if (!group.IsEmpty()) {
	group += _T(".");
	group += title;
	title = group;
      }
      name = title;
      name += SPLTCHR;
      name += item.GetUser();
    }
    unsigned char dummy_type;
    GetRandomData(&dummy_type, 1);
    WriteCBC(dummy_type, name);
    WriteCBC(CItemData::PASSWORD, item.GetPassword());
    WriteCBC(CItemData::NOTES, item.GetNotes());
    return SUCCESS;
  }
  break;
  case V20: {
    {
      uuid_array_t uuid_array;
      item.GetUUID(uuid_array);
      WriteCBC(CItemData::UUID, uuid_array, sizeof(uuid_array));
    }
    WriteCBC(CItemData::GROUP, item.GetGroup());
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
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion != UNKNOWN_VERSION);

  CMyString tempdata;  
  int numread = 0;
  unsigned char type;

  switch (m_curversion) {
  case V17: {
    // type is meaningless, but why write two versions of ReadCBC?
    numread += ReadCBC(type, tempdata);
    item.SetName(tempdata, m_defusername);
    numread += ReadCBC(type, tempdata);
    item.SetPassword(tempdata);
    numread += ReadCBC(type, tempdata);
    item.SetNotes(tempdata);
    // No UUID, so we create one here
    item.CreateUUID();
    // No Group - currently leave empty
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
	case CItemData::UUID: {
	  LPCSTR ptr = LPCSTR(tempdata);
	  uuid_array_t uuid_array;
	  for (int i = 0; i < sizeof(uuid_array); i++)
	    uuid_array[i] = ptr[i];
	  item.SetUUID(uuid_array); break;
	}
	case CItemData::GROUP:
	  item.SetGroup(tempdata); break;
	  // just silently ignore fields we don't support.
	  // this is forward compatability...
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

