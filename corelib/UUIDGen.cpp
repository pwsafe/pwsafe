// UUIDGen.cpp
// Silly class for generating UUIDs
// Each instance has its own unique value, 
// which can be accessed as an array of bytes or as a human-readable
// ASCII string.
//

#include "UUIDGen.h"
#include "util.h" /* for trashMemory() */
#include <stdio.h> /* for sprintf() */
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

void CUUIDGen::GetUUIDStr(uuid_str_t &str) const
{
#if _MSC_VER >= 1400
  sprintf_s((char *)str, 36,
#else
  sprintf((char *)str,
#endif
	  "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
	  uuid.Data1, uuid.Data2, uuid.Data3,
	  (unsigned char) uuid.Data4[0], (unsigned char) uuid.Data4[1],
	  (unsigned char) uuid.Data4[2], (unsigned char) uuid.Data4[3],
	  (unsigned char) uuid.Data4[4], (unsigned char) uuid.Data4[5],
	  (unsigned char) uuid.Data4[6], (unsigned char) uuid.Data4[7]);
}

#ifdef TEST
#include <stdio.h>
int main()
{
  uuid_str_t str;
  uuid_array_t uuid_array;

  for (int i = 0; i< 10; i++) {
    CUUIDGen uuid;
    uuid.GetUUIDStr(str);
    printf("%s\n",str);
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
