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

static const uuid_array_t zua = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static pws_os::CUUID nullUUID(zua);

const pws_os::CUUID &pws_os::CUUID::NullUUID()
{
  return nullUUID;
}

pws_os::CUUID::CUUID() : m_ua(NULL), m_canonic(false)
{
  uuid_generate(m_uuid);
}

pws_os::CUUID::CUUID(const CUUID &uuid)
  : m_ua(NULL), m_canonic(uuid.m_canonic)
{
  uuid_copy(m_uuid, uuid.m_uuid);
}

pws_os::CUUID &pws_os::CUUID::operator=(const CUUID &that)
{
  if (this != &that) {
    uuid_copy(m_uuid, that.m_uuid);
    if (m_ua) {
#ifndef TEST
      trashMemory(m_ua, sizeof(uuid_array_t));
#endif
      delete[] m_ua;
      m_ua = NULL;
    }
    m_canonic = that.m_canonic;
  }
  return *this;
}

pws_os::CUUID::CUUID(const uuid_array_t &uuid_array, bool canonic)
  : m_ua(NULL), m_canonic(canonic)
{
  uuid_copy(m_uuid, uuid_array);
}

pws_os::CUUID::CUUID(const StringX &s)
  : m_ua(NULL), m_canonic(false)
{
  // s is a hex string as returned by cast to StringX
  ASSERT(s.length() == 32);
  unsigned char *uu = m_uuid;

  int x;
  for (int i = 0; i < 16; i++) {
    iStringXStream is(s.substr(i*2, 2));
    is >> hex >> x;
    uu[i] = static_cast<unsigned char>(x);
  }
}

pws_os::CUUID::~CUUID()
{
#ifndef TEST
  trashMemory(reinterpret_cast<unsigned char *>(&m_uuid), sizeof(m_uuid));
#endif
  if (m_ua) {
#ifndef TEST
    trashMemory(m_ua, sizeof(uuid_array_t));
#endif
    delete[] m_ua;
  }
}

void pws_os::CUUID::GetARep(uuid_array_t &uuid_array) const
{
  uuid_copy(uuid_array, m_uuid);
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
  return uuid_compare(this->m_uuid, that.m_uuid) == 0;
}

bool pws_os::CUUID::operator<(const pws_os::CUUID &that) const
{
  return uuid_compare(m_uuid, that.m_uuid) < 0;
}

std::ostream &pws_os::operator<<(std::ostream &os, const pws_os::CUUID &uuid)
{
  uuid_array_t uuid_a;
  uuid.GetARep(uuid_a);
  for (size_t i = 0; i < sizeof(uuid_array_t); i++) {
    os << setw(2) << setfill('0') << hex << int(uuid_a[i]);
    if (uuid.m_canonic && (i == 3 || i == 5 || i == 7 || i == 9))
      os << "-";
  }
  return os;
}

std::wostream &pws_os::operator<<(std::wostream &os, const pws_os::CUUID &uuid)
{
  uuid_array_t uuid_a;
  uuid.GetARep(uuid_a);
  for (size_t i = 0; i < sizeof(uuid_array_t); i++) {
    os << setw(2) << setfill(wchar_t('0')) << hex << int(uuid_a[i]);
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
  uuid_string_t str;
  uuid_array_t uuid_array;

  for (int i = 0; i< 10; i++) {
    pws_os::CUUID uuid;
    uuid.GetARep(uuid_array);
    uuid_unparse_lower(uuid_array, str);
    printf("%s\n",str);
    uuid.GetARep(uuid_array);
    printf(_T("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n"),
              uuid_array[0], uuid_array[1], uuid_array[2], uuid_array[3], 
              uuid_array[4], uuid_array[5], uuid_array[6], uuid_array[7], 
              uuid_array[8], uuid_array[9], uuid_array[10], uuid_array[11], 
              uuid_array[12], uuid_array[13], uuid_array[14], uuid_array[15]);
  }
  return 0;
}
#endif
