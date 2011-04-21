/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// UUIDGen.cpp
// Silly class for generating UUIDs
// Each instance has its own unique value, 
// which can be accessed as an array of bytes or as a human-readable
// ASCII string.
//

#include "UUIDGen.h"
#include "Util.h" /* for trashMemory() */
#include "StringXStream.h"
#include <iomanip>
#include <assert.h>

#ifdef _WIN32
#include <Winsock.h> /* for htonl, htons */
#elif defined(__PWS_MACINTOSH__)
#include <sys/_endian.h>
#else
/* currently here only for Cygwin test harness */
#include <asm/byteorder.h> /* for htonl, htons */
#endif

using namespace std;

#ifdef _WIN32
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
#endif

static CUUIDGen nullUUID;

const CUUIDGen &CUUIDGen::NullUUID()
{
  static bool inited = false;
  if (!inited) {
    inited = true;
    uuid_array_t zua;
    memset(zua, 0, sizeof(zua));
    CUUIDGen zu(zua);
    nullUUID = zu;
  }
  return nullUUID;
}

CUUIDGen::CUUIDGen() : m_ua(NULL), m_canonic(false)
{
#ifdef _WIN32
  UuidCreate(&m_uuid);
#else
  uuid_generate(m_uuid);
#endif
}

CUUIDGen::CUUIDGen(const CUUIDGen &uuid)
  : m_ua(NULL), m_canonic(uuid.m_canonic)
{
#ifdef _WIN32
  std::memcpy(&m_uuid, &uuid.m_uuid, sizeof(m_uuid));
#else
  uuid_copy(m_uuid, uuid.m_uuid);
#endif
}

CUUIDGen &CUUIDGen::operator=(const CUUIDGen &that)
{
  if (this != &that) {
#ifdef _WIN32
    std::memcpy(&m_uuid, &that.m_uuid, sizeof(m_uuid));
#else
    uuid_copy(m_uuid, that.m_uuid);
#endif
    m_ua = NULL;
    m_canonic = that.m_canonic;
  }
  return *this;
}

CUUIDGen::CUUIDGen(const uuid_array_t &uuid_array, bool canonic)
  : m_canonic(canonic), m_ua(NULL)
{
#ifdef _WIN32
  array2UUUID(uuid_array, m_uuid);
#else
  uuid_copy(m_uuid, uuid_array);
#endif
}

CUUIDGen::CUUIDGen(const StringX &s) // s is a hex string as returned by cast to StringX
  : m_ua(NULL), m_canonic(false)
{
  ASSERT(s.length() == 32);
#ifdef _WIN32
  uuid_array_t uu;
#else
  unsigned char *uu = m_uuid;
#endif

  int x;
  for (int i = 0; i < 16; i++) {
    iStringXStream is(s.substr(i*2, 2));
    is >> hex >> x;
    uu[i] = static_cast<unsigned char>(x);
  }
#ifdef _WIN32
  array2UUUID(uu, m_uuid);
#endif
}

CUUIDGen::~CUUIDGen()
{
  trashMemory(reinterpret_cast<unsigned char *>(&m_uuid), sizeof(m_uuid));
  if (m_ua) {
    trashMemory(m_ua, sizeof(uuid_array_t));
    delete[] m_ua;
  }
}

void CUUIDGen::GetUUID(uuid_array_t &uuid_array) const
{
#ifdef _WIN32
  UUID2array(m_uuid, uuid_array);
#else
  uuid_copy(uuid_array, m_uuid);
#endif
}

const uuid_array_t *CUUIDGen::GetUUID() const
{
  if (m_ua == NULL) {
    m_ua = (uuid_array_t *)(new uuid_array_t);
    GetUUID(*m_ua);
  }
  return m_ua;
}

bool CUUIDGen::operator==(const CUUIDGen &that) const
{
#ifdef _WIN32
  return std::memcmp(&this->m_uuid, &that.m_uuid, sizeof(m_uuid)) == 0;
#else
  return uuid_compare(this->m_uuid, that.m_uuid) == 0;
#endif
}

bool CUUIDGen::operator<(const CUUIDGen &that) const
{
#ifdef _WIN32
  return std::memcmp(&m_uuid,
                     &that.m_uuid, sizeof(m_uuid)) < 0;
#else
  return uuid_compare(m_uuid, that.m_uuid) < 0;
#endif
}


ostream &operator<<(ostream &os, const CUUIDGen &uuid)
{
  uuid_array_t uuid_a;
  uuid.GetUUID(uuid_a);
  for (size_t i = 0; i < sizeof(uuid_array_t); i++) {
    os << setw(2) << setfill('0') << hex << int(uuid_a[i]);
    if (uuid.m_canonic && (i == 3 || i == 5 || i == 7 || i == 9))
      os << "-";
  }
  return os;
}

wostream &operator<<(wostream &os, const CUUIDGen &uuid)
{
  uuid_array_t uuid_a;
  uuid.GetUUID(uuid_a);
  for (size_t i = 0; i < sizeof(uuid_array_t); i++) {
    os << setw(2) << setfill(wchar_t('0')) << hex << int(uuid_a[i]);
    if (uuid.m_canonic && (i == 3 || i == 5 || i == 7 || i == 9))
      os << L"-";
  }
  return os;
}

CUUIDGen::operator StringX() const
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
  uuid_array_t uuid_array;

  for (int i = 0; i< 10; i++) {
    CUUIDGen uuid;
    printf("%s\n",str);
    uuid.GetUUID(uuid_array);
    printf(_T("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n"),
              uuid_array[0], uuid_array[1], uuid_array[2], uuid_array[3], 
              uuid_array[4], uuid_array[5], uuid_array[6], uuid_array[7], 
              uuid_array[8], uuid_array[9], uuid_array[10], uuid_array[11], 
              uuid_array[12], uuid_array[13], uuid_array[14], uuid_array[15]);
  }
  return 0;
}
#endif
