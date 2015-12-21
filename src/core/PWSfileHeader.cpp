/*
 * Copyright (c) 2003-2014 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#include "PWSfileHeader.h"

PWSfileHeader::PWSfileHeader()
  : m_nCurrentMajorVersion(0), m_nCurrentMinorVersion(0),
    m_file_uuid(pws_os::CUUID::NullUUID()),
    m_prefString(_T("")), m_whenlastsaved(0),
    m_lastsavedby(_T("")), m_lastsavedon(_T("")),
    m_whatlastsaved(_T("")),
    m_dbname(_T("")), m_dbdesc(_T("")), m_yubi_sk(NULL)
{
  m_RUEList.clear();
}

PWSfileHeader::PWSfileHeader(const PWSfileHeader &h) 
  : m_nCurrentMajorVersion(h.m_nCurrentMajorVersion),
    m_nCurrentMinorVersion(h.m_nCurrentMinorVersion),
    m_file_uuid(h.m_file_uuid),
    m_displaystatus(h.m_displaystatus),
    m_prefString(h.m_prefString), m_whenlastsaved(h.m_whenlastsaved),
    m_lastsavedby(h.m_lastsavedby), m_lastsavedon(h.m_lastsavedon),
    m_whatlastsaved(h.m_whatlastsaved),
    m_dbname(h.m_dbname), m_dbdesc(h.m_dbdesc), m_RUEList(h.m_RUEList)
{
  if (h.m_yubi_sk != NULL) {
    m_yubi_sk = new unsigned char[YUBI_SK_LEN];
    memcpy(m_yubi_sk, h.m_yubi_sk, YUBI_SK_LEN);
  } else {
    m_yubi_sk = NULL;
  }
}

PWSfileHeader::~PWSfileHeader()
{
  if (m_yubi_sk)
    trashMemory(m_yubi_sk, YUBI_SK_LEN);
  delete[] m_yubi_sk;
}

PWSfileHeader &PWSfileHeader::operator=(const PWSfileHeader &h)
{
  if (this != &h) {
    m_nCurrentMajorVersion = h.m_nCurrentMajorVersion;
    m_nCurrentMinorVersion = h.m_nCurrentMinorVersion;
    m_file_uuid = h.m_file_uuid;
    m_displaystatus = h.m_displaystatus;
    m_prefString = h.m_prefString;
    m_whenlastsaved = h.m_whenlastsaved;
    m_lastsavedby = h.m_lastsavedby;
    m_lastsavedon = h.m_lastsavedon;
    m_whatlastsaved = h.m_whatlastsaved;
    m_dbname = h.m_dbname;
    m_dbdesc = h.m_dbdesc;
    m_RUEList = h.m_RUEList;
    if (h.m_yubi_sk != NULL) {
      if (m_yubi_sk)
        trashMemory(m_yubi_sk, YUBI_SK_LEN);
      delete[] m_yubi_sk;
      m_yubi_sk = new unsigned char[YUBI_SK_LEN];
      memcpy(m_yubi_sk, h.m_yubi_sk, YUBI_SK_LEN);
    } else {
      m_yubi_sk = NULL;
    }
  }
  return *this;
}

bool PWSfileHeader::operator==(const PWSfileHeader &h) const
{
  bool retval = (m_nCurrentMajorVersion == h.m_nCurrentMajorVersion &&
                 m_nCurrentMinorVersion == h.m_nCurrentMinorVersion &&
                 m_file_uuid == h.m_file_uuid &&
                 m_displaystatus == h.m_displaystatus &&
                 m_prefString == h.m_prefString &&
                 m_whenlastsaved == h.m_whenlastsaved &&
                 m_lastsavedby == h.m_lastsavedby &&
                 m_lastsavedon == h.m_lastsavedon &&
                 m_whatlastsaved == h.m_whatlastsaved &&
                 m_dbname == h.m_dbname &&
                 m_dbdesc == h.m_dbdesc &&
                 m_RUEList == h.m_RUEList);
  if (!retval)
    return false;
  if (m_yubi_sk == NULL && h.m_yubi_sk == NULL)
    return true;
  if ((m_yubi_sk == NULL && h.m_yubi_sk != NULL) ||
      (m_yubi_sk != NULL && h.m_yubi_sk == NULL))
    return false;
  // here iff both m_yubi_sk's != NULL
  return (memcmp(m_yubi_sk, h.m_yubi_sk, YUBI_SK_LEN) == 0);
}
