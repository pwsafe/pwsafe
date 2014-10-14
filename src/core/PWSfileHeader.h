/*
* Copyright (c) 2003-2014 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __PWSFILEHEADER_H
#define __PWSFILEHEADER_H

// PWSfileHeader.h
// Represetation of what's written into a PWSfile's header (V3 and later)
// Used to be a nested struct in PWSfile, but outgrew it...
//-----------------------------------------------------------------------------

#include <vector>
#include "os/UUID.h"
#include "StringX.h"
#include "coredefs.h"

struct PWSfileHeader {
  PWSfileHeader();
  PWSfileHeader(const PWSfileHeader &hdr);
  PWSfileHeader &operator =(const PWSfileHeader &hdr);
  bool operator==(const PWSfileHeader &hdr) const; // for unit tests
  ~PWSfileHeader();
  unsigned short m_nCurrentMajorVersion, m_nCurrentMinorVersion;
  pws_os::CUUID m_file_uuid;         // Unique DB ID
  std::vector<bool> m_displaystatus; // Tree expansion state vector
  StringX m_prefString;              // Prefererences stored in the file
  time_t m_whenlastsaved; // When last saved
  StringX m_lastsavedby; // and by whom
  StringX m_lastsavedon; // and by which machine
  StringX m_whatlastsaved; // and by what application
  StringX m_dbname, m_dbdesc;        // Descriptive name, Description
  UUIDList m_RUEList;
  unsigned char *m_yubi_sk;  // YubiKey HMAC key, added in 0x030a / 3.27Y
  enum {YUBI_SK_LEN = 20};
};

#endif /* __PWSFILEHEADER_H */
