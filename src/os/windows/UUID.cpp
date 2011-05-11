/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include <Winsock.h> /* for htonl, htons */

using namespace std;

static void array2UUUID(const uuid_array_t &uuid_array, UUID &uuid)
{
  unsigned long *p0 = (unsigned long *)uuid_array;
  uuid.Data1 = htonl(*p0);
  unsigned short *p1 = (unsigned short *)&uuid_array[4];
  uuid.Data2 = htons(*p1);
  unsigned short *p2 = (unsigned short *)&uuid_array[6];
  uuid.Data3 = htons(*p2);
  for (int i = 0; i < 8; i++)
    uuid.Data4[i] = uuid_array[i + 8];
}

static void UUID2array(const UUID &uuid, uuid_array_t &uuid_array)
{
  unsigned long *p0 = (unsigned long *)uuid_array;
  *p0 = htonl(uuid.Data1);
  unsigned short *p1 = (unsigned short *)&uuid_array[4];
  *p1 = htons(uuid.Data2);
  unsigned short *p2 = (unsigned short *)&uuid_array[6];
  *p2 = htons(uuid.Data3);
  for (int i = 0; i < 8; i++)
    uuid_array[i + 8] = uuid.Data4[i];
}

static pws_os::CUUID nullUUID;

const pws_os::CUUID &pws_os::CUUID::NullUUID()
{
  static bool inited = false;
  if (!inited) {
    inited = true;
    uuid_array_t zua;
    memset(zua, 0, sizeof(zua));
    CUUID zu(zua);
    nullUUID = zu;
  }
  return nullUUID;
}

pws_os::CUUID::CUUID() : m_ua(NULL), m_canonic(false)
{
  UuidCreate(&m_uuid);
}

pws_os::CUUID::CUUID(const CUUID &uuid)
  : m_ua(NULL), m_canonic(uuid.m_canonic)
{
  std::memcpy(&m_uuid, &uuid.m_uuid, sizeof(m_uuid));
}

pws_os::CUUID &pws_os::CUUID::operator=(const CUUID &that)
{
  if (this != &that) {
    std::memcpy(&m_uuid, &that.m_uuid, sizeof(m_uuid));
    m_ua = NULL;
    m_canonic = that.m_canonic;
  }
  return *this;
}

pws_os::CUUID::CUUID(const uuid_array_t &uuid_array, bool canonic)
  : m_ua(NULL), m_canonic(canonic)
{
  array2UUUID(uuid_array, m_uuid);
}

pws_os::CUUID::CUUID(const StringX &s)
  : m_ua(NULL), m_canonic(false)
{
  // s is a hex string as returned by cast to StringX
  ASSERT(s.length() == 32);
  uuid_array_t uu;

  int x;
  for (int i = 0; i < 16; i++) {
    iStringXStream is(s.substr(i*2, 2));
    is >> hex >> x;
    uu[i] = static_cast<unsigned char>(x);
  }
  array2UUUID(uu, m_uuid);
}

pws_os::CUUID::~CUUID()
{
  trashMemory(reinterpret_cast<unsigned char *>(&m_uuid), sizeof(m_uuid));
  if (m_ua) {
    trashMemory(m_ua, sizeof(uuid_array_t));
    delete[] m_ua;
  }
}

void pws_os::CUUID::GetARep(uuid_array_t &uuid_array) const
{
  UUID2array(m_uuid, uuid_array);
}

const uuid_array_t *pws_os::CUUID::GetARep() const
{
  if (m_ua == NULL) {
    m_ua = (uuid_array_t *)(new uuid_array_t);
    GetARep(*m_ua);
  }
  return m_ua;
}

bool pws_os::CUUID::operator==(const CUUID &that) const
{
  return std::memcmp(&this->m_uuid, &that.m_uuid, sizeof(m_uuid)) == 0;
}

bool pws_os::CUUID::operator<(const pws_os::CUUID &that) const
{
  return std::memcmp(&m_uuid,
                     &that.m_uuid, sizeof(m_uuid)) < 0;
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
  uuid_str_t str;
  uuid_array_t ua;

  for (int i = 0; i< 10; i++) {
    CUUID uuid;
    printf("%s\n",str);
    uuid.GetARep(ua);
    printf(_T("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n"),
              ua[0], ua[1], ua[2],  ua[3],  ua[4],  ua[5],  ua[6],  ua[7], 
              ua[8], ua[9], ua[10], ua[11], ua[12], ua[13], ua[14], ua[15]);
  }
  return 0;
}
#endif
