/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include "PWSfileV1V2.h"
#include "PWSrand.h"
#include "corelib.h"
#include "os/file.h"
#include "os/utf8conv.h"

#ifdef _WIN32
#include <io.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>

PWSfileV1V2::PWSfileV1V2(const StringX &filename, RWmode mode, VERSION version)
  : PWSfile(filename, mode)
{
  m_curversion = version;
  m_IV = m_ipthing;
}

PWSfileV1V2::~PWSfileV1V2()
{
}

// Used to warn pre-2.0 users, and to identify the database as 2.x:
static const StringX V2ItemName(_T(" !!!Version 2 File Format!!! Please upgrade to PasswordSafe 2.0 or later"));
// Used to specify the exact version
static const StringX VersionString(_T("2.0"));
static const StringX AltVersionString(_T("pre-2.0")); 

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
  PWSrand::GetInstance()->GetRandomData(rbuf, rlen-1);
  rbuf[rlen-1] = TCHAR('\0'); // although zero may be there before - who cares?
  stringT rname(V2ItemName);
  rname += rbuf;
  delete[] rbuf;
  header.SetName(rname, _T(""));
#else
  header.SetName(V2ItemName, _T(""));
#endif /* BREAK_PRE_2_14_COMPATIBILITY */
  header.SetPassword(VersionString);
  header.SetNotes(m_hdr.m_prefString);
  // need to fallback to V17, since the record
  // won't be readable otherwise!
  VERSION sv = m_curversion;
  m_curversion = V17;
  int status = WriteRecord(header);
  // restore after writing V17-format header
  m_curversion = sv;
  m_hdr.m_nCurrentMajorVersion = 2;
  m_hdr.m_nCurrentMinorVersion = 0;
  return status;
}

int PWSfileV1V2::ReadV2Header()
{
  m_hdr.m_nCurrentMajorVersion = 1;
  m_hdr.m_nCurrentMinorVersion = 0;
  CItemData header;
  // need to fallback to V17, since the header
  // is always written in this format
  VERSION sv = m_curversion;
  m_curversion = V17;
  int status = ReadRecord(header);
  // restore after reading V17-format header
  m_curversion = sv;
  if (status == SUCCESS) {
    const StringX version = header.GetPassword();
    // Compare to AltVersionString due to silly mistake
    // "2.0" as well as "pre-2.0" are actually 2.0. sigh.
    if (version == VersionString || version == AltVersionString) {
      status = SUCCESS;
      m_hdr.m_nCurrentMajorVersion = 2;
    } else
      status = WRONG_VERSION;
  }
  if (status == SUCCESS)
    m_hdr.m_prefString = header.GetNotes();
  return status;
}

int PWSfileV1V2::Open(const StringX &passkey)
{
  int status = SUCCESS;

  ASSERT(m_curversion == V17 || m_curversion == V20);

  m_passkey = passkey;
  FOpen();
  if (m_fd == NULL)
    return CANT_OPEN_FILE;

  LPCTSTR passstr = m_passkey.c_str();
  unsigned long passLen = passkey.length();
  unsigned char *pstr;

#ifdef UNICODE
  pstr = new unsigned char[3*passLen];
  size_t len = pws_os::wcstombs((char *)pstr, 3 * passLen, passstr, passLen);
  ASSERT(len != 0);
  passLen = len;
#else
  pstr = (unsigned char *)passstr;
#endif

  if (m_rw == Write) {
    // Following used to verify passkey against file's passkey
    unsigned char randstuff[StuffSize];
    unsigned char randhash[20];   // HashSize

    PWSrand::GetInstance()->GetRandomData( randstuff, 8 );
    randstuff[8] = randstuff[9] = TCHAR('\0');
    GenRandhash(m_passkey, randstuff, randhash);

    fwrite(randstuff, 1, 8, m_fd);
    fwrite(randhash, 1, 20, m_fd);

    PWSrand::GetInstance()->GetRandomData(m_salt, SaltLength);

    fwrite(m_salt, 1, SaltLength, m_fd);

    PWSrand::GetInstance()->GetRandomData( m_ipthing, 8);
    fwrite(m_ipthing, 1, 8, m_fd);

    m_fish = BlowFish::MakeBlowFish(pstr, passLen,
                                    m_salt, SaltLength);
    if (m_curversion == V20) {
      status = WriteV2Header();
    }
  } else { // open for read
    status = CheckPassword(m_filename, m_passkey, m_fd);
    if (status != SUCCESS) {
#ifdef UNICODE
      trashMemory(pstr, 3*passLen);
      delete[] pstr;
#endif
      Close();
      return status;
    }
    fread(m_salt, 1, SaltLength, m_fd);
    fread(m_ipthing, 1, 8, m_fd);

    m_fish = BlowFish::MakeBlowFish(pstr, passLen,
      m_salt, SaltLength);
    if (m_curversion == V20)
      status = ReadV2Header();
  } // read mode
#ifdef UNICODE
  trashMemory(pstr, 3*passLen);
  delete[] pstr;
#endif
  return status;
}

int PWSfileV1V2::Close()
{
  return PWSfile::Close();
}

int PWSfileV1V2::CheckPassword(const StringX &filename,
                               const StringX &passkey, FILE *a_fd)
{
  FILE *fd = a_fd;
  if (fd == NULL) {
    fd = pws_os::FOpen(filename.c_str(), _T("rb"));
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

static StringX ReMergeNotes(const CItemData &item)
{
  StringX notes = item.GetNotes();
  const StringX url(item.GetURL());
  if (!url.empty()) {
    notes += _T("\r\n"); notes += url;
  }
  const StringX at(item.GetAutoType());
  if (!at.empty()) {
    stringT cs_autotype;
    LoadAString(cs_autotype, IDSC_AUTOTYPE);
    notes += _T("\r\n");
    notes += cs_autotype.c_str();
    notes += at;
  }
  return notes;
}

size_t PWSfileV1V2::WriteCBC(unsigned char type, const StringX &data)
{
#ifndef UNICODE
  const unsigned char *datastr = (const unsigned char *)data.c_str();

  return PWSfile::WriteCBC(type, datastr, data.length());
#else
  wchar_t *wcPtr = const_cast<wchar_t *>(data.c_str());
  int wcLen = data.length()+1;
  int mbLen = 3*wcLen;
  unsigned char *acp = new unsigned char[mbLen];
  size_t acpLen = pws_os::wcstombs(reinterpret_cast<char *>(acp), mbLen,
                                   wcPtr, wcLen);
  ASSERT(acpLen != 0);
  acpLen--; // remove unneeded null termination
  size_t retval = PWSfile::WriteCBC(type, acp, acpLen);
  trashMemory(acp, mbLen);
  delete[] acp;
  return retval;
#endif
}

int PWSfileV1V2::WriteRecord(const CItemData &item)
{
  ASSERT(m_fd != NULL);
  ASSERT(m_curversion != UNKNOWN_VERSION);
  int status = SUCCESS;

  switch (m_curversion) {
    case V17:
    {
      // 1.x programs totally ignore the type byte, hence safe to write it
      // (no need for two WriteCBC functions)
      // Note that 2.0 format still requires that the header be in this format,
      // So that old programs reading new databases won't crash,
      // This introduces a small security issue, in that the header is known text,
      // making the password susceptible to a dictionary attack on the first block,
      // rather than the hash^n in the beginning of the file.
      // we can help minimize this here by writing a random byte in the "type"
      // byte of the first block.

      StringX name = item.GetName();
      // If name field already exists - use it. This is for the 2.0 header, as well as for files
      // that were imported and re-exported.
      if (name.empty()) {
        // The name in 1.7 consists of title + SPLTCHR + username
        // DEFUSERNAME was used in previous versions, but 2.0 converts this upon import
        // so it is not an issue here.
        // Prepend 2.0 group field to name, if not empty
        // i.e. group "finances" name "broker" -> "finances.broker"
        StringX group = item.GetGroup();
        StringX title = item.GetTitle();
        if (!group.empty()) {
          group += _T(".");
          group += title;
          title = group;
        }
        name = title;
        name += SPLTCHR;
        name += item.GetUser();
      }
      unsigned char dummy_type;
      PWSrand::GetInstance()->GetRandomData(&dummy_type, 1);
      WriteCBC(dummy_type, name);
      WriteCBC(CItemData::PASSWORD, item.GetPassword());
      WriteCBC(CItemData::NOTES, ReMergeNotes(item));
      break;
    }
    case V20:
    {
      {
        uuid_array_t uuid_array;
        item.GetUUID(uuid_array);
        PWSfile::WriteCBC(CItemData::UUID, uuid_array, sizeof(uuid_array));
      }
      WriteCBC(CItemData::GROUP, item.GetGroup());
      WriteCBC(CItemData::TITLE, item.GetTitle());
      WriteCBC(CItemData::USER, item.GetUser());
      WriteCBC(CItemData::PASSWORD, item.GetPassword());
      WriteCBC(CItemData::NOTES, ReMergeNotes(item));
      WriteCBC(CItemData::END, _T(""));
      break;
    }
    default:
      ASSERT(0);
      status = UNSUPPORTED_VERSION;
  }
  return status;
}

static void ExtractAutoTypeCmd(StringX &notesStr, StringX &autotypeStr)
{
  StringX instr(notesStr);
  stringT cs_autotype;
  LoadAString(cs_autotype, IDSC_AUTOTYPE);
  StringX::size_type left = instr.find(cs_autotype.c_str(), 0);
  if (left == StringX::npos) {
    autotypeStr = _T(""); 
  } else {
    StringX tmp(notesStr);
    tmp = tmp.substr(left+9); // throw out everything left of "autotype:"
    instr = instr.substr(0, left);
    StringX::size_type right = tmp.find_first_of(_T("\r\n"));
    if (right != StringX::npos) {
      instr += tmp.substr(right);
      tmp = tmp.substr(0, right);
    }
    autotypeStr = tmp;
    notesStr = instr;
  }
}

static void ExtractURL(StringX &notesStr, StringX &outurl)
{
  StringX instr(notesStr);
  // Extract first instance of (http|https|ftp)://[^ \t\r\n]+
  StringX::size_type left = instr.find(_T("http://"));
  if (left == StringX::npos)
    left = instr.find(_T("https://"));
  if (left == StringX::npos)
    left = instr.find(_T("ftp://"));
  if (left == StringX::npos) {
    outurl = _T("");
  } else {
    StringX url(instr);
    instr = notesStr.substr(0, left);
    url = url.substr(left); // throw out everything left of URL
    StringX::size_type right = url.find_first_of(_T(" \t\r\n"));
    if (right != StringX::npos) {
      instr += url.substr(right);
      url = url.substr(0, right);    
    }
    outurl = url;
    notesStr = instr;
  }
}

size_t PWSfileV1V2::ReadCBC(unsigned char &type, StringX &data)
{
  unsigned char *buffer = NULL;
  size_t buffer_len = 0;
  size_t retval;

  ASSERT(m_fish != NULL && m_IV != NULL);
  retval = _readcbc(m_fd, buffer, buffer_len, type,
                    m_fish, m_IV, m_terminal);

  if (buffer_len > 0) {
#ifdef UNICODE
    wchar_t *wc = new wchar_t[buffer_len+1];

    size_t wcLen = pws_os::mbstowcs(wc, buffer_len + 1,
                                    reinterpret_cast<const char *>(buffer),
                                    buffer_len);
    ASSERT(wcLen != 0);
    if (wcLen < buffer_len + 1)
      wc[wcLen] = TCHAR('\0');
    else
      wc[buffer_len] = TCHAR('\0');
    data = wc;
    trashMemory(wc, wcLen);
    delete[] wc;
#else
    StringX str((const char *)buffer, buffer_len);
    data = str;
#endif
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

  StringX tempdata;  
  signed long numread = 0;
  unsigned char type;

  switch (m_curversion) {
    case V17:
    {
      // type is meaningless, but why write two versions of ReadCBC?
      numread += static_cast<signed long>(ReadCBC(type, tempdata));
      item.SetName(tempdata, m_defusername);
      numread += static_cast<signed long>(ReadCBC(type, tempdata));
      item.SetPassword(tempdata);
      numread += static_cast<signed long>(ReadCBC(type, tempdata));
      item.SetNotes(tempdata);
      // No UUID, so we create one here
      item.CreateUUID();
      // No Group - currently leave empty
      return (numread > 0) ? SUCCESS : END_OF_FILE;
    }
    case V20:
    {
      int emergencyExit = 255; // to avoid endless loop.
      signed long fieldLen; // zero means end of file reached
      bool endFound = false; // set to true when record end detected - happy end
      do {
        fieldLen = static_cast<signed long>(ReadCBC(type, tempdata));
        if (signed(fieldLen) > 0) {
          numread += fieldLen;
          switch (type) {
            case CItemData::TITLE:
              item.SetTitle(tempdata); break;
            case CItemData::USER:
              item.SetUser(tempdata); break;
            case CItemData::PASSWORD:
              item.SetPassword(tempdata); break;
            case CItemData::NOTES:
            {
              StringX autotypeStr, URLStr;
              ExtractAutoTypeCmd(tempdata, autotypeStr);
              ExtractURL(tempdata, URLStr);
              item.SetNotes(tempdata);
              if (!autotypeStr.empty())
                item.SetAutoType(autotypeStr);
              if (!URLStr.empty())
                item.SetURL(URLStr);
              break;
            }
            case CItemData::END:
              endFound = true; break;
            case CItemData::UUID:
            {
              LPCTSTR ptr = tempdata.c_str();
              uuid_array_t uuid_array;
              for (unsigned i = 0; i < sizeof(uuid_array); i++)
                uuid_array[i] = (unsigned char)ptr[i];
              item.SetUUID(uuid_array);
              break;
            }
            case CItemData::GROUP:
              item.SetGroup(tempdata); break;
              // just silently ignore fields we don't support.
              // this is forward compatability...
            case CItemData::CTIME:
            case CItemData::PMTIME:
            case CItemData::ATIME:
            case CItemData::XTIME:
            case CItemData::RMTIME:
            case CItemData::POLICY:
            case CItemData::XTIME_INT:
            default:
              break;
          } // switch
        } // if (fieldLen > 0)
      } while (!endFound && fieldLen > 0 && --emergencyExit > 0);
      return (numread > 0 && endFound) ? SUCCESS : END_OF_FILE;
    }
    default:
      ASSERT(0);
      return UNSUPPORTED_VERSION;
  }
}
