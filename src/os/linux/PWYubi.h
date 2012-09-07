/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __PWYUBI_H

#include <pthread.h> // for wxMutex et. al.
#include <string>

class PWYubi {
public:
  PWYubi();
  ~PWYubi();
  bool IsYubiInserted() const;
  bool GetSerial(unsigned int &serial) const;
  // if GetErrStr().empty(), then no error:
  const std::wstring &GetErrStr() const {return m_ykerrstr;}
private:
  void report_error() const;
  mutable bool m_isInited;
  static pthread_mutex_t s_mutex;
  mutable std::wstring m_ykerrstr;
};
#define __PWYUBI_H

#endif /* __PWYUBI_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
