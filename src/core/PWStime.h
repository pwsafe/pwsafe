/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// PWStime.h
//-----------------------------------------------------------------------------
#ifndef __PWSTIME_H
#define __PWSTIME_H

/** \file
 * "Time is on my side, yes it is" - the Rolling Stones
 *
 * A class to convert between time_t (which can be 32 or 64 bits) and a
 * 5 byte (40 bit) little-endian representation, which is what's stored
 * in the PWSafe V4 format. This resolves the 2038 issue for my lifetime,
 * at least, and provides platform-independence, to boot.
 */

#include <ctime>

class PWStime
{
public:
  enum {TIME_LEN = 5};
  PWStime(); // default c'tor initiates value to current time
  PWStime(const PWStime &);
  PWStime(std::time_t);
  PWStime(const unsigned char *pbuf); // pbuf points to a TIME_LEN array
  ~PWStime();
  PWStime &operator=(const PWStime &);
  PWStime &operator=(std::time_t);

  operator time_t() const;
  operator const unsigned char *() const;
  operator const char *() const;

  unsigned int GetLength() const {return TIME_LEN;} // size of representation
  

private:
  void setTime(time_t t);
  unsigned char m_time[TIME_LEN];
};

#endif /* __PWSTIME_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
