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
#include "util.h" /* for trashMemory() */
#include <assert.h>

#ifdef _WIN32
#include <Winsock2.h> /* for htonl, htons */
#else
/* currently here only for Cygwin test harness */
#include <asm/byteorder.h> /* for htonl, htons */
#endif

CUUIDGen::CUUIDGen()
{
  UuidCreate(&uuid);
}

CUUIDGen::CUUIDGen(const uuid_array_t &uuid_array)
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

CUUIDGen::~CUUIDGen()
{
  trashMemory((unsigned char *)&uuid, sizeof(uuid));
}

void CUUIDGen::GetUUID(uuid_array_t &uuid_array) const
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

static void hexify(unsigned char byte, char *&out)
{
  static const char v[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  *out++ = v[(byte >> 4) & 0xf];
  *out++ = v[byte & 0xf];
}

void CUUIDGen::GetUUIDStr(const uuid_array_t &uuid_array,
                          uuid_str_NH_t &uuid_buffer)
{
  // No hyphens
  char *p = uuid_buffer;
  for (int i = 0; i < sizeof(uuid_array); i++)
    hexify(uuid_array[i], p);
  *p = '\0';
}

void CUUIDGen::GetUUIDStr(const uuid_array_t &uuid_array,
                          uuid_str_WH_t &uuid_buffer)
{
  // With hyphens!
  char *p = uuid_buffer;
  for (int i = 0; i < sizeof(uuid_array); i++) {
    hexify(uuid_array[i], p);
    if (i == 3 || i == 5 || i == 7 || i == 9)
      *p++ = '-';
  }
  *p = '\0';
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
