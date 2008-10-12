/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include <Winsock2.h> /* for htonl, htons */
#else
/* currently here only for Cygwin test harness */
#include <asm/byteorder.h> /* for htonl, htons */
#endif

using namespace std;

CUUIDGen::CUUIDGen() : m_canonic(false)
{
#ifdef _WIN32
  UuidCreate(&uuid);
#else
  uuid_generate(uuid);
#endif
}

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

CUUIDGen::CUUIDGen(const uuid_array_t &uuid_array, bool canonic) : m_canonic(canonic)
{
#ifdef _WIN32
  array2UUUID(uuid_array, uuid);
#else
  uuid_copy(uuid, uuid_array);
#endif
}

CUUIDGen::CUUIDGen(const StringX &s) // s is a hex string as returned by GetHexStr()
{
  ASSERT(s.length() == 32);
#ifdef _WIN32
  uuid_array_t uu;
#else
  unsigned char *uu == uuid;
#endif

  int x;
  for (int i = 0; i < 16; i++) {
    iStringXStream is(s.substr(i*2, 2));
    is >> hex >> x;
    uu[i] = (unsigned char)x;
  }
#ifdef _WIN32
  array2UUUID(uu, uuid);
#endif
}

CUUIDGen::~CUUIDGen()
{
  trashMemory((unsigned char *)&uuid, sizeof(uuid));
}

void CUUIDGen::GetUUID(uuid_array_t &uuid_array) const
{
#ifdef _WIN32
  UUID2array(uuid, uuid_array);
#else
  uuid_copy(uuid_array, uuid);
#endif
}


ostream &operator<<(ostream &os, const CUUIDGen &uuid)
{
 uuid_array_t uuid_a;
  uuid.GetUUID(uuid_a);
  for (size_t i = 0; i < sizeof(uuid_a); i++) {
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
  for (size_t i = 0; i < sizeof(uuid_a); i++) {
    os << setw(2) << setfill(wchar_t('0')) << hex << int(uuid_a[i]);
    if (uuid.m_canonic && (i == 3 || i == 5 || i == 7 || i == 9))
      os << L"-";
  }
  return os;
}

StringX CUUIDGen::GetHexStr() const
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
