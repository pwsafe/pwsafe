/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#include "PWSfileHeader.h"

PWSfileHeader::PWSfileHeader()
  : m_nCurrentMajorVersion(0), m_nCurrentMinorVersion(0),
    m_file_uuid(pws_os::CUUID::NullUUID()),
    m_prefString(_T("")), m_whenlastsaved(0), m_whenpwdlastchanged(0),
    m_lastsavedby(_T("")), m_lastsavedon(_T("")),
    m_whatlastsaved(_T("")),
    m_DB_Name(_T("")), m_DB_Description(_T("")), m_yubi_sk(nullptr)
{
}

PWSfileHeader::PWSfileHeader(const PWSfileHeader &h) 
  : m_nCurrentMajorVersion(h.m_nCurrentMajorVersion),
    m_nCurrentMinorVersion(h.m_nCurrentMinorVersion),
    m_file_uuid(h.m_file_uuid),
    m_prefString(h.m_prefString),
    m_displaystatus(h.m_displaystatus),
    m_whenlastsaved(h.m_whenlastsaved),
    m_whenpwdlastchanged(h.m_whenpwdlastchanged),
    m_lastsavedby(h.m_lastsavedby), m_lastsavedon(h.m_lastsavedon),
    m_whatlastsaved(h.m_whatlastsaved),
    m_DB_Name(h.m_DB_Name), m_DB_Description(h.m_DB_Description), m_RUEList(h.m_RUEList)
{
  if (h.m_yubi_sk != nullptr) {
    m_yubi_sk = new unsigned char[YUBI_SK_LEN];
    memcpy(m_yubi_sk, h.m_yubi_sk, YUBI_SK_LEN);
  } else {
    m_yubi_sk = nullptr;
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
    m_whenpwdlastchanged = h.m_whenpwdlastchanged;
    m_DB_Name = h.m_DB_Name;
    m_DB_Description = h.m_DB_Description;
    m_RUEList = h.m_RUEList;
    if (h.m_yubi_sk != nullptr) {
      if (m_yubi_sk)
        trashMemory(m_yubi_sk, YUBI_SK_LEN);
      delete[] m_yubi_sk;
      m_yubi_sk = new unsigned char[YUBI_SK_LEN];
      memcpy(m_yubi_sk, h.m_yubi_sk, YUBI_SK_LEN);
    } else {
      m_yubi_sk = nullptr;
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
                 m_whenpwdlastchanged == m_whenpwdlastchanged &&
                 m_lastsavedby == h.m_lastsavedby &&
                 m_lastsavedon == h.m_lastsavedon &&
                 m_whatlastsaved == h.m_whatlastsaved &&
                 m_DB_Name == h.m_DB_Name &&
                 m_DB_Description == h.m_DB_Description &&
                 m_RUEList == h.m_RUEList);
  if (!retval)
    return false;
  if (m_yubi_sk == nullptr && h.m_yubi_sk == nullptr)
    return true;
  if ((m_yubi_sk == nullptr && h.m_yubi_sk != nullptr) ||
      (m_yubi_sk != nullptr && h.m_yubi_sk == nullptr))
    return false;
  // here iff both m_yubi_sk's != nullptr
  return (memcmp(m_yubi_sk, h.m_yubi_sk, YUBI_SK_LEN) == 0);
}
