/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// UUID.cpp
// Wrapper class for UUIDs, generating and converting them to/from
// various representations.
// Each instance has its own unique value, 
// which can be accessed as an array of bytes or as a human-readable
// ASCII string.
//

#include "../UUID.h"
#include "../../core/Util.h" /* for trashMemory() */
#include "../../core/StringXStream.h"
#include <iomanip>
#include <assert.h>

using namespace std;

static void array2UUUID(const uuid_array_t &ua, UUID &uuid)
{
  unsigned long *p0 = (unsigned long *)ua;
  uuid.Data1 = htonl(*p0);
  unsigned short *p1 = (unsigned short *)&ua[4];
  uuid.Data2 = htons(*p1);
  unsigned short *p2 = (unsigned short *)&ua[6];
  uuid.Data3 = htons(*p2);
  for (int i = 0; i < 8; i++)
    uuid.Data4[i] = ua[i + 8];
}

static void UUID2array(const UUID &uuid, uuid_array_t &ua)
{
  unsigned long *p0 = (unsigned long *)ua;
  *p0 = htonl(uuid.Data1);
  unsigned short *p1 = (unsigned short *)&ua[4];
  *p1 = htons(uuid.Data2);
  unsigned short *p2 = (unsigned short *)&ua[6];
  *p2 = htons(uuid.Data3);
  for (int i = 0; i < 8; i++)
    ua[i + 8] = uuid.Data4[i];
}

static const uuid_array_t zua = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static pws_os::CUUID nullUUID(zua);

const pws_os::CUUID &pws_os::CUUID::NullUUID()
{
  return nullUUID;
}

pws_os::CUUID::CUUID()
  : m_ua(NULL), m_canonic(false)
{
  UuidCreate(&m_uuid);
}

pws_os::CUUID::CUUID(const CUUID &uuid)
  : m_ua(NULL), m_canonic(uuid.m_canonic)
{
  std::memcpy(&m_uuid, &uuid.m_uuid, sizeof(m_uuid));
}

pws_os::CUUID::CUUID(const uuid_array_t &ua, bool canonic)
  : m_ua(NULL), m_canonic(canonic)
{
  array2UUUID(ua, m_uuid);
}

pws_os::CUUID::CUUID(const StringX &s)
  : m_ua(NULL), m_canonic(false)
{
  // s is a hex string as returned by cast to StringX
  ASSERT(s.length() == 32);
  uuid_array_t ua;

  unsigned int x(0);
  for (size_t i = 0; i < 16; i++) {
    iStringXStream is(s.substr(i * 2, 2));
    is >> hex >> x;
    ua[i] = static_cast<unsigned char>(x);
  }
  array2UUUID(ua, m_uuid);
}

pws_os::CUUID::~CUUID()
{
  trashMemory(reinterpret_cast<unsigned char *>(&m_uuid), sizeof(m_uuid));
  if (m_ua) {
    trashMemory(m_ua, sizeof(uuid_array_t));
    delete[] m_ua;
  }
}

pws_os::CUUID &pws_os::CUUID::operator=(const CUUID &that)
{
  if (this != &that) {
    std::memcpy(&m_uuid, &that.m_uuid, sizeof(m_uuid));
    if (m_ua) {
      trashMemory(m_ua, sizeof(uuid_array_t));
      delete[] m_ua;
      m_ua = NULL;
    }
    m_canonic = that.m_canonic;
  }
  return *this;
}

void pws_os::CUUID::GetARep(uuid_array_t &ua) const
{
  UUID2array(m_uuid, ua);
}

const uuid_array_t *pws_os::CUUID::GetARep() const
{
  if (m_ua == NULL) {
    m_ua = (uuid_array_t *)(new uuid_array_t);
    GetARep(*m_ua);
  }
  return m_ua;
}

bool pws_os::CUUID::operator==(const pws_os::CUUID &that) const
{
  return std::memcmp(&this->m_uuid, &that.m_uuid, sizeof(m_uuid)) == 0;
}

bool pws_os::CUUID::operator<(const pws_os::CUUID &that) const
{
  return std::memcmp(&this->m_uuid, &that.m_uuid, sizeof(m_uuid)) < 0;
}

std::ostream &pws_os::operator<<(std::ostream &os, const pws_os::CUUID &uuid)
{
  uuid_array_t ua;
  uuid.GetARep(ua);
  for (size_t i = 0; i < sizeof(uuid_array_t); i++) {
    os << setw(2) << setfill('0') << hex << int(ua[i]);
    if (uuid.m_canonic && (i == 3 || i == 5 || i == 7 || i == 9))
      os << "-";
  }
  return os;
}

std::wostream &pws_os::operator<<(std::wostream &os, const pws_os::CUUID &uuid)
{
  uuid_array_t ua;
  uuid.GetARep(ua);
  for (size_t i = 0; i < sizeof(uuid_array_t); i++) {
    os << setw(2) << setfill(wchar_t('0')) << hex << int(ua[i]);
    if (uuid.m_canonic && (i == 3 || i == 5 || i == 7 || i == 9))
      os << L"-";
  }
  return os;
}

pws_os::CUUID::operator StringX() const
{
  oStringXStream os;
  bool sc = m_canonic;
  m_canonic = false;
  os << *this;
  m_canonic = sc;
  return os.str();
}

#ifdef TEST
#include <stdio.h>
int main()
{
  //uuid_str_t str;
  uuid_array_t ua;

  for (int i = 0; i < 10; i++) {
    pws_os::CUUID uuid;
    uuid.GetARep(ua);
    /* Need a Windows equivalent to the Linux/Mac "uuid_unparse_lower" !!!
    uuid_unparse_lower(ua, str);
    printf(_T("%s\n"), str);
    */
    printf(_T("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n"),
              ua[0], ua[1], ua[2],  ua[3],  ua[4],  ua[5],  ua[6],  ua[7], 
              ua[8], ua[9], ua[10], ua[11], ua[12], ua[13], ua[14], ua[15]);
  }
  return 0;
}
#endif
