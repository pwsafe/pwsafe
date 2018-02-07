/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ExpiredList.cpp : implementation file
//

#include "ExpiredList.h"
#include "ItemData.h"
#include "os/funcwrap.h"

#include <functional>
#include <algorithm>

using namespace std;

ExpPWEntry::ExpPWEntry(const CItemData &ci)
{
  time_t tttXTime;

  uuid = ci.GetUUID();
  ci.GetXTime(tttXTime);

  if (tttXTime > time_t(0) && tttXTime <= time_t(3650)) {
    time_t tttCPMTime;
    ci.GetPMTime(tttCPMTime);
    if (tttCPMTime == time_t(0))
      ci.GetCTime(tttCPMTime);
    tttXTime = static_cast<time_t>(static_cast<long>(tttCPMTime) + static_cast<long>(tttXTime) * 86400);
  }
  expirytttXTime = tttXTime;
}

void ExpiredList::Add(const CItemData &ci)
{
  // Not valid for aliases or shortcuts!
  if (ci.IsDependent())
    return;

  // We might be called from Update with a ci
  // that doesn't have an expiration date - check!
  time_t tttXTime;
  ci.GetXTime(tttXTime);
  if (tttXTime != time_t(0))
    push_back(ExpPWEntry(ci));
}

void ExpiredList::Remove(const CItemData &ci)
{
  ExpiredList::iterator iter = std::find_if(begin(), end(),
                                            std::bind2nd(std::equal_to<pws_os::CUUID>(),
                                                         ci.GetUUID()));
  if (iter != end())
    erase(iter);
}

ExpiredList ExpiredList::GetExpired(const int &idays)
{
  ExpiredList retval;
  ExpiredList::iterator iter;
  struct tm st;

  time_t now, exptime=time_t(-1);
  time(&now);
  errno_t err;
  err = localtime_s(&st, &now);
  ASSERT(err == 0);
  if (!err) {
    st.tm_mday += idays;
    // Note: mktime will normalize the date structure before converting to time_t
    exptime = mktime(&st);
  }

  if (exptime == time_t(-1))
    exptime = now;

  for (iter = begin(); iter != end(); iter++) {
    if (iter->expirytttXTime < exptime) {
      retval.push_back(*iter);
    }
  }
  return retval;
}
