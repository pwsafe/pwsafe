/// \file ItemData.cpp
//-----------------------------------------------------------------------------

#include "ItemData.h"
#include "BlowFish.h"
#include "TwoFish.h"
#include <time.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool CItemData::IsSessionKeySet = false;
unsigned char CItemData::SessionKey[64];

void CItemData::SetSessionKey()
{
  // must be called once per session, no more, no less
  ASSERT(!IsSessionKeySet);
  GetRandomData( SessionKey, sizeof( SessionKey ) );
  IsSessionKeySet = true;
}

//-----------------------------------------------------------------------------
// Constructors

CItemData::CItemData()
  : m_Name(NAME), m_Title(TITLE), m_User(USER), m_Password(PASSWORD),
    m_Notes(NOTES), m_UUID(UUID), m_Group(GROUP),
    m_URL(URL), m_AutoType(AUTOTYPE), m_cTime(CTIME), m_display_info(NULL)
{
  GetRandomData( m_salt, SaltLength );
}

CItemData::CItemData(const CItemData &that) :
  m_Name(that.m_Name), m_Title(that.m_Title), m_User(that.m_User),
  m_Password(that.m_Password), m_Notes(that.m_Notes), m_UUID(that.m_UUID),
  m_Group(that.m_Group), m_URL(that.m_URL), m_AutoType(that.m_AutoType),
  m_cTime(that.m_cTime), m_display_info(that.m_display_info)
{
  ::memcpy((char*)m_salt, (char*)that.m_salt, SaltLength);
}

//-----------------------------------------------------------------------------
// Accessors

void CItemData::GetField(const CItemField &field, CMyString &value) const
{
  BlowFish *bf = MakeBlowFish();
  field.Get(value, bf);
  delete bf;
}

void CItemData::GetField(const CItemField &field, unsigned char *value, unsigned int &length) const
{
  BlowFish *bf = MakeBlowFish();
  field.Get(value, length, bf);
  delete bf;
}


CMyString
CItemData::GetName() const
{
   CMyString ret;
   GetField(m_Name, ret);
   return ret;
}

CMyString
CItemData::GetTitle() const
{
   CMyString ret;
   GetField(m_Title, ret);
   return ret;
}

CMyString
CItemData::GetUser() const
{
   CMyString ret;
   GetField(m_User, ret);
   return ret;
}


CMyString
CItemData::GetPassword() const
{
   CMyString ret;
   GetField(m_Password, ret);
   return ret;
}

CMyString
CItemData::GetNotes(TCHAR delimiter) const
{
   CMyString ret;
   GetField(m_Notes, ret);
   if (delimiter != 0) {
     ret.Remove(TCHAR('\r'));
     ret.Replace(TCHAR('\n'), delimiter);
   }
   return ret;
}

CMyString
CItemData::GetGroup() const
{
   CMyString ret;
   GetField(m_Group, ret);
   return ret;
}

CMyString
CItemData::GetURL() const
{
   CMyString ret;
   GetField(m_URL, ret);
   return ret;
}

CMyString
CItemData::GetAutoType() const
{
   CMyString ret;
   GetField(m_AutoType, ret);
   return ret;
}

CMyString
CItemData::GetCTime() const
{
  time_t t;
  struct tm *st;
  char *time_str;
  unsigned char in[TwoFish::BLOCKSIZE]; // required by GetField
  unsigned int tlen = sizeof(in); // ditto
  CMyString ret;
   
  GetField(m_cTime, (unsigned char *)in, tlen);
  if (tlen != 0) {
    ASSERT(tlen == sizeof(t));
    memcpy(&t, in, sizeof(t));
  } else {
    t = 0;
  }
  if (t != 0) {
    st = localtime(&t);
    time_str = _tasctime(st);
    ret = time_str;
  } else
    ret = _T("Unknown");
  // remote the trailing EOL char.
  ret.TrimRight();
  return ret;
}

void
CItemData::GetCTime(time_t &t) const
{
   unsigned char in[TwoFish::BLOCKSIZE]; // required by GetField
   unsigned int tlen = sizeof(in); // ditto
   
   GetField(m_cTime, (unsigned char *)in, tlen);
   if (tlen != 0)
     memcpy(&t, in, sizeof(t));
   else
     t = 0;
}

void CItemData::GetUUID(uuid_array_t &uuid_array) const
{
  unsigned int length = sizeof(uuid_array);
  GetField(m_UUID, (unsigned char *)uuid_array, length);
}

CMyString CItemData::GetPlaintext(TCHAR separator, TCHAR delimiter) const
{
  CMyString ret;
  CMyString title;
  CMyString group(GetGroup());

  // a '.' in title gets Import confused re: Groups
  title = GetTitle();
  if (title.Find(TCHAR('.')) != -1)
    title = TCHAR('\"') + title + TCHAR('\"');

  if (!group.IsEmpty())
    title = group + TCHAR('.') + title;

  // Notes field must be last, for ease of parsing import
  ret = title + separator + GetUser() + separator +
    GetPassword() + separator + GetURL() +
    separator + GetAutoType() + separator +
    GetCTime() + separator + _T("\"") + GetNotes(delimiter) + _T("\"");

  return ret;
}

void CItemData::SplitName(const CMyString &name,
		   CMyString &title, CMyString &username)
{
  int pos = name.FindByte(SPLTCHR);
  if (pos==-1) {//Not a split name
    int pos2 = name.FindByte(DEFUSERCHR);
    if (pos2 == -1)  {//Make certain that you remove the DEFUSERCHR 
	title = name;
    } else {
	title = CMyString(name.Left(pos2));
    }
  } else {
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
}

//-----------------------------------------------------------------------------
// Setters

void CItemData::SetField(CItemField &field, const CMyString &value)
{
  BlowFish *bf = MakeBlowFish();
  field.Set(value, bf);
  delete bf;
}

void CItemData::SetField(CItemField &field, const unsigned char *value, unsigned int length)
{
  BlowFish *bf = MakeBlowFish();
  field.Set(value, length, bf);
  delete bf;
}

void CItemData::CreateUUID()
{
  CUUIDGen uuid;
  uuid_array_t uuid_array;
  uuid.GetUUID(uuid_array);
  SetUUID(uuid_array);
}


void
CItemData::SetName(const CMyString &name, const CMyString &defaultUsername)
{
  // the m_name is from pre-2.0 versions, and may contain the title and user
  // separated by SPLTCHR. Also, DEFUSERCHR signified that the default username is to be used.
  // Here we fill the title and user fields so that
  // the application can ignore this difference after an ItemData record
  // has been created
  CMyString title, user;
  int pos = name.FindByte(DEFUSERCHR);
  if (pos != -1) {
    title = CMyString(name.Left(pos));
    user = defaultUsername;
  } else
    SplitName(name, title, user);
  // In order to avoid unecessary BlowFish construction/deletion,
  // we forego SetField here...
  BlowFish *bf = MakeBlowFish();
  m_Name.Set(name, bf);
  m_Title.Set(title, bf);
  m_User.Set(user, bf);
  delete bf;
}

void
CItemData::SetTitle(const CMyString &title)
{
  SetField(m_Title, title);
}

void
CItemData::SetUser(const CMyString &user)
{
  SetField(m_User, user);
}

void
CItemData::SetPassword(const CMyString &password)
{
  SetField(m_Password, password);
}

void
CItemData::SetNotes(const CMyString &notes, char delimiter)
{
  if (delimiter == 0)
    SetField(m_Notes, notes);
  else {
	const CMyString CRCRLF = _T("\r\r\n");
	CMyString multiline_notes(_T(""));

	CMyString newCString;
	CMyString tmpCString;

	int pos = 0;

	newCString = notes;
	do {
      pos = newCString.Find(delimiter);
      if ( pos != -1 ) {
        multiline_notes += CMyString(newCString.Left(pos)) + CRCRLF;

        tmpCString = CMyString(newCString.Mid(pos + 1));
        newCString = tmpCString;
      }
	} while ( pos != -1 );
	
	if (!newCString.IsEmpty())
      multiline_notes += newCString;

	SetField(m_Notes, multiline_notes);
  }
}

void
CItemData::SetGroup(const CMyString &title)
{
  SetField(m_Group, title);
}

void
CItemData::SetUUID(const uuid_array_t &UUID)
{
  SetField(m_UUID, (const unsigned char *)UUID, sizeof(UUID));
}

void
CItemData::SetURL(const CMyString &URL)
{
  SetField(m_URL, URL);
}

void
CItemData::SetAutoType(const CMyString &autotype)
{
  SetField(m_AutoType, autotype);
}

void
CItemData::SetCTime(time_t t)
{
  SetField(m_cTime, (const unsigned char *)&t, sizeof(t));
}

void
CItemData::SetCTime()
{
  time_t t;
  time(&t);
  SetCTime(t);
}

void
CItemData::SetCTime(const CMyString &)
{
  // XXX TBD
  time_t t;
  time(&t);
  SetCTime(t);
}

BlowFish *
CItemData::MakeBlowFish() const
{
  ASSERT(IsSessionKeySet);
  return BlowFish::MakeBlowFish(SessionKey, sizeof(SessionKey),
                                m_salt, SaltLength);
}

CItemData&
CItemData::operator=(const CItemData &that)
{
   //Check for self-assignment
   if (this != &that)
   {
     m_UUID = that.m_UUID;
     m_Name = that.m_Name;
     m_Title = that.m_Title;
     m_User = that.m_User;
     m_Password = that.m_Password;
     m_Notes = that.m_Notes;
     m_Group = that.m_Group;
     m_URL = that.m_URL;
     m_AutoType = that.m_AutoType;
     m_cTime = that.m_cTime;
     m_display_info = that.m_display_info;

     memcpy((char*)m_salt, (char*)that.m_salt, SaltLength);
   }

   return *this;
}

void
CItemData::Clear()
{
  CMyString blank(_T(""));
  SetTitle(blank);
  SetUser(blank);
  SetPassword(blank);
  SetNotes(blank);
  SetGroup(blank);
  SetURL(blank);
  SetAutoType(blank);
}

//TODO: "General System Fault. Please sacrifice a goat 
//and two chickens to continue."

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
