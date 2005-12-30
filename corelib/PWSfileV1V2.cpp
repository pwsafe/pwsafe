#include "PWSfileV1V2.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>


PWSfileV1V2::PWSfileV1V2(const CMyString &filename, RWmode mode, VERSION version)
  : PWSfile(filename,mode)
{
  m_curversion = version;
}

PWSfileV1V2::~PWSfileV1V2()
{
}

// Used to warn pre-2.0 users, and to identify the database as 2.x:
static const CMyString V2ItemName(" !!!Version 2 File Format!!! "
				  "Please upgrade to PasswordSafe 2.0"
				  " or later");
// Used to specify the exact version
static const CMyString VersionString("2.0");

int PWSfileV1V2::WriteV2Header()
{
  CItemData header;
  // Fill out with V2-specific info
  // To make a dictionary attack a little harder, we make the length
  // of the first record (Name) variable, by appending variable length randomness
  // to the fixed string
  // OOPS - can't do this yet, since previous versions (pre-2.14) read the name
  // (in ReadV2Header)
  // and compare it directly to VersionString to check version - a small
  // mistake that would cause a pre-2.14 executable to barf reading a database
  // written by 2.14 and later.
  // #idef-ing this out, while correcting the code
  // in ReadV2Header. Perhaps this can be fixed a year from now?
#ifdef BREAK_PRE_2_14_COMPATIBILITY
  unsigned int rlen = RangeRand(62) + 2; // 64 is a trade-off...
  char *rbuf = new char[rlen];
  GetRandomData(rbuf, rlen-1);
  rbuf[rlen-1] = '\0'; // although zero may be there before - who cares?
  CMyString rname(V2ItemName);
  rname += rbuf;
  delete[] rbuf;
  header.SetName(rname, _T(""));
#else
  header.SetName(V2ItemName, _T(""));
#endif /* BREAK_PRE_2_14_COMPATIBILITY */
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

int PWSfileV1V2::ReadV2Header()
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

int PWSfileV1V2::Open(const CMyString &passkey)
{
  int status = SUCCESS;

  ASSERT(m_curversion == V17 || m_curversion == V20);

  m_passkey = passkey;

  if (m_rw == Write) {
#ifdef UNICODE
    m_fd = _wfopen((LPCTSTR)m_filename, _T("wb") );
#else
    m_fd = fopen((LPCTSTR)m_filename, _T("wb") );
#endif

    if (m_fd == NULL)
      return CANT_OPEN_FILE;


    // Following used to verify passkey against file's passkey
    unsigned char randstuff[StuffSize];
    unsigned char randhash[20];   // HashSize

    GetRandomData( randstuff, 8 );
    randstuff[8] = randstuff[9] = '\0';
    GenRandhash(m_passkey, randstuff, randhash);

    fwrite(randstuff, 1, 8, m_fd);
    fwrite(randhash, 1, 20, m_fd);

    GetRandomData(m_salt, SaltLength);

    fwrite(m_salt, 1, SaltLength, m_fd);
	
    GetRandomData( m_ipthing, 8);
    fwrite(m_ipthing, 1, 8, m_fd);
    if (m_curversion == V20) {
      status = WriteV2Header();
    }
  } else { // open for read
#ifdef UNICODE
    m_fd = _wfopen((LPCTSTR) m_filename, _T("rb"));
#else
    m_fd = fopen((LPCTSTR) m_filename, _T("rb"));
#endif

    if (m_fd == NULL)
      return CANT_OPEN_FILE;
    status = CheckPassword(m_filename, m_passkey, m_fd);
    if (status != SUCCESS) {
      Close();
      return status;
    }
    fread(m_salt, 1, SaltLength, m_fd);
    fread(m_ipthing, 1, 8, m_fd);

    if (m_curversion == V20)
      status = ReadV2Header();
  } // read mode
  return status;
}

int PWSfileV1V2::Close()
{
  return PWSfile::Close();
}


int PWSfileV1V2::CheckPassword(const CMyString &filename,
                               const CMyString &passkey, FILE *a_fd)
{
  FILE *fd = a_fd;
  if (fd == NULL) {
#ifdef UNICODE
    fd = _wfopen((LPCTSTR) filename, _T("rb"));
#else
    fd = fopen((LPCTSTR) filename, _T("rb"));
#endif
  }
  if (fd == NULL)
    return CANT_OPEN_FILE;

  unsigned char randstuff[StuffSize];
  unsigned char randhash[20];   // HashSize

   fread(randstuff, 1, 8, fd);
   randstuff[8] = randstuff[9] = '\0'; // Gross fugbix
   fread(randhash, 1, 20, fd);

   if (a_fd == NULL) // if we opened the file, we close it...
     fclose(fd);

  unsigned char temphash[20]; // HashSize
  GenRandhash(passkey, randstuff, temphash);

  if (0 != ::memcmp((char*)randhash,
		    (char*)temphash,
		    20)) {// HashSize
    return WRONG_PASSWORD;
  } else {
    return SUCCESS;
  }
}

int PWSfileV1V2::WriteCBC(unsigned char type, const CString &data, Fish *fish)
{
  // We do a double cast because the LPCSTR cast operator is overridden
  // by the CString class to access the pointer we need,
  // but we in fact need it as an unsigned char. Grrrr.
  LPCSTR datastr = LPCSTR(data);

  return WriteCBC(type, (const unsigned char *)datastr, data.GetLength(), fish);
}

int PWSfileV1V2::WriteCBC(unsigned char type, const unsigned char *data,
                          unsigned int length, Fish *fish)
{
  return _writecbc(m_fd, data, length, type, fish, m_ipthing);
}


int PWSfileV1V2::WriteRecord(const CItemData &item)
{
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion != UNKNOWN_VERSION);
  int status = SUCCESS;

  // We do a double cast because the LPCSTR cast operator is overridden by the CString class
  // to access the pointer we need,
  // but we in fact need it as an unsigned char. Grrrr.
  LPCSTR passstr = LPCSTR(m_passkey);
  BlowFish *fish = BlowFish::MakeBlowFish((const unsigned char *)passstr, m_passkey.GetLength(),
                                          m_salt, SaltLength);


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
    WriteCBC(dummy_type, name, fish);
    WriteCBC(CItemData::PASSWORD, item.GetPassword(), fish);
    WriteCBC(CItemData::NOTES, item.GetNotes(), fish);
  }
    break;
  case V20: {
    {
      uuid_array_t uuid_array;
      item.GetUUID(uuid_array);
      WriteCBC(CItemData::UUID, uuid_array, sizeof(uuid_array), fish);
    }
    WriteCBC(CItemData::GROUP, item.GetGroup(), fish);
    WriteCBC(CItemData::TITLE, item.GetTitle(), fish);
    WriteCBC(CItemData::USER, item.GetUser(), fish);
    WriteCBC(CItemData::PASSWORD, item.GetPassword(), fish);
    WriteCBC(CItemData::NOTES, item.GetNotes(), fish);
    WriteCBC(CItemData::END, _T(""), fish);
  }
    break;
  default:
    ASSERT(0);
    status = UNSUPPORTED_VERSION;
  }
  delete fish;
  return status;
}

int
PWSfileV1V2::ReadCBC(unsigned char &type, CMyString &data, Fish *fish)
{

  unsigned char *buffer = NULL;
  unsigned int buffer_len = 0;
  int retval;

  retval = _readcbc(m_fd, buffer, buffer_len, type, fish, m_ipthing);

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



int PWSfileV1V2::ReadRecord(CItemData &item)
{
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion != UNKNOWN_VERSION);

  CMyString tempdata;  
  int numread = 0;
  unsigned char type;
  // We do a double cast because the LPCSTR cast operator is overridden by the CString class
  // to access the pointer we need,
  // but we in fact need it as an unsigned char. Grrrr.
  LPCSTR passstr = LPCSTR(m_passkey);
  BlowFish *fish = BlowFish::MakeBlowFish((const unsigned char *)passstr, m_passkey.GetLength(),
                                          m_salt, SaltLength);

  switch (m_curversion) {
  case V17: {
    // type is meaningless, but why write two versions of ReadCBC?
    numread += ReadCBC(type, tempdata, fish);
    item.SetName(tempdata, m_defusername);
    numread += ReadCBC(type, tempdata, fish);
    item.SetPassword(tempdata);
    numread += ReadCBC(type, tempdata, fish);
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
      fieldLen = ReadCBC(type, tempdata, fish);
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
