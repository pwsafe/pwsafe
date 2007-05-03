/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// UUIDGen.cpp
// Silly class for generating UUIDs
// Each instance has its own unique value, 
// which can be accessed as an array of bytes.
//

#include "UUIDGen.h"
#include "Util.h" /* for trashMemory() */

#ifdef _WIN32
#include <Winsock2.h> /* for htonl, htons */
#else
/* currently here only for Cygwin test harness */
#include <asm/byteorder.h> /* for htonl, htons */
#endif

CUUIDGen::CUUIDGen()
{
  CoCreateGuid(&guid);
}

CUUIDGen::CUUIDGen(const uuid_array_t &uuid_array)
{
  unsigned long *p0 = (unsigned long *)uuid_array;
  guid.Data1 = htonl(*p0);
  unsigned short *p1 = (unsigned short *)&uuid_array[4];
  guid.Data2 = htons(*p1);
  unsigned short *p2 = (unsigned short *)&uuid_array[6];
  guid.Data3 = htons(*p2);
  for (int i = 0; i < 8; i++)
    guid.Data4[i] = uuid_array[i + 8];
}


CUUIDGen::~CUUIDGen()
{
  trashMemory((unsigned char *)&guid, sizeof(guid));
}

void CUUIDGen::GetUUID(uuid_array_t &uuid_array) const
{
  unsigned long *p0 = (unsigned long *)uuid_array;
  *p0 = htonl(guid.Data1);
  unsigned short *p1 = (unsigned short *)&uuid_array[4];
  *p1 = htons(guid.Data2);
  unsigned short *p2 = (unsigned short *)&uuid_array[6];
  *p2 = htons(guid.Data3);
  for (int i = 0; i < 8; i++)
    uuid_array[i + 8] = guid.Data4[i];
}

#ifdef TEST
#include <stdio.h>
int main()
{
  uuid_array_t uuid_array;

  for (int i = 0; i< 10; i++) {
    CUUIDGen uuid;
    uuid.GetUUID(uuid_array);
    printf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
	   uuid_array[0], uuid_array[1], uuid_array[2], uuid_array[3], 
	   uuid_array[4], uuid_array[5], uuid_array[6], uuid_array[7], 
	   uuid_array[8], uuid_array[9], uuid_array[10], uuid_array[11], 
	   uuid_array[12], uuid_array[13], uuid_array[14], uuid_array[15]);
  }
  return 0;
}
#endif
