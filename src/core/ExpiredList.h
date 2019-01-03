/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ExpiredList.h
//-----------------------------------------------------------------------------

#ifndef __EXPIREDLIST_H
#define __EXPIREDLIST_H

#include "StringX.h"
#include "os/UUID.h"
#include "ItemData.h"

#include <vector>

struct ExpPWEntry {
  ExpPWEntry(const CItemData &ci);
  ExpPWEntry(const ExpPWEntry &ee) : uuid(ee.uuid), expirytttXTime(ee.expirytttXTime) {}
  ExpPWEntry &operator=(const ExpPWEntry &that) {
    if (this != &that) {
      expirytttXTime = that.expirytttXTime;
      uuid = that.uuid;
    }
    return *this;
  };

  operator pws_os::CUUID() {return uuid;}
  pws_os::CUUID uuid;
  time_t expirytttXTime;
};

class ExpiredList: public std::vector<ExpPWEntry>
{
public:
  void Add(const CItemData &ci);
  void Update(const CItemData &ci) {Remove(ci); Add(ci);}
  void Remove(const CItemData &ci);
  ExpiredList GetExpired(const int &idays); // return a subset
};

#endif /* __EXPIREDLIST_H */
