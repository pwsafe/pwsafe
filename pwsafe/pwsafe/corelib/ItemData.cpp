/// \file ItemData.cpp
//-----------------------------------------------------------------------------

#include "global.h"

#include "ItemData.h"
#include "BlowFish.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
// Constructors

CItemData::CItemData()
  : m_Name(NAME), m_Title(TITLE), m_User(USER), m_Password(PASSWORD),
    m_Notes(NOTES), m_UUID(UUID), m_display_info(NULL)
{
  for (int x = 0; x < SaltLength; x++)
    m_salt[x] = newrand();
}

CItemData::CItemData(const CItemData &that) :
  m_Name(that.m_Name), m_Title(that.m_Title), m_User(that.m_User),
  m_Password(that.m_Password), m_Notes(that.m_Notes), m_UUID(that.m_UUID),
  m_display_info(that.m_display_info)
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
CItemData::GetNotes() const
{
   CMyString ret;
   GetField(m_Notes, ret);
   return ret;
}

void CItemData::GetUUID(uuid_array_t &uuid_array) const
{
  unsigned int length = sizeof(uuid_array);
  GetField(m_UUID, (unsigned char *)uuid_array, length);
}

void CItemData::SplitName(const CMyString &name,
		   CMyString &title, CMyString &username)
{
  // XXX TBD - punt Default Name handling to upper layer - just split
  // the damn thing here!

  int pos = name.FindByte(SPLTCHR);
  if (pos==-1) {//Not a split name
    int pos2 = name.FindByte(DEFUSERCHR);
    if (pos2 == -1)  {//Make certain that you remove the DEFUSERCHR 
	title = name;
    } else {
	title = CMyString(name.Left(pos2));
    }
#if 0 // XXX Handle elsewhere
    if ((pos2 != -1)
	&& GetUseDefUser())
      {
	username = GetDefUsername();
      } else {
	username = _T("");
      }
#endif
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
CItemData::SetName(const CMyString &name)
{
  // the m_name is from pre-2.0 versions, and may contain the title and user
  // separated by SPLTCHR. Here we fill the title and user fields so that
  // the application can ignore this difference after an ItemData record
  // has been created
  CMyString title, user;
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
CItemData::SetNotes(const CMyString &notes)
{
  SetField(m_Notes, notes);
}

void
CItemData::SetUUID(const uuid_array_t &UUID)
{
  SetField(m_UUID, (const unsigned char *)UUID, sizeof(UUID));
}



BlowFish *
CItemData::MakeBlowFish() const
{
  LPCSTR passstr = LPCSTR(global.m_passkey);

  return ::MakeBlowFish((const unsigned char *)passstr,
			global.m_passkey.GetLength(),
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
     m_display_info = that.m_display_info;

      memcpy((char*)m_salt, (char*)that.m_salt, SaltLength);
   }

   return *this;
}

//TODO: "General System Fault. Please sacrifice a goat 
//and two chickens to continue."

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
