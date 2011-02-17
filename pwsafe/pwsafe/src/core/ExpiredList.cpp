/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ExpiredList.cpp : implementation file
//

#include "ExpiredList.h"
#include "ItemData.h"

#include <algorithm>

using namespace std;

ExpPWEntry::ExpPWEntry(const CItemData &ci)
{
  time_t tttXTime;

  ci.GetUUID(uuid);
  ci.GetXTime(tttXTime);

  group = ci.GetGroup();
  title = ci.GetTitle();
  if (ci.IsProtected())
    title += L" #";
  user = ci.GetUser();
  et = ci.GetEntryType();

  if ((long)tttXTime > 0L && (long)tttXTime <= 3650L) {
    time_t tttCPMTime;
    ci.GetPMTime(tttCPMTime);
    if ((long)tttCPMTime == 0L)
      ci.GetCTime(tttCPMTime);
    tttXTime = (time_t)((long)tttCPMTime + (long)tttXTime * 86400);
  }
  expirytttXTime = tttXTime;
  expirylocdate = PWSUtil::ConvertToDateTimeString(tttXTime, TMC_LOCALE);
  expiryexpdate = PWSUtil::ConvertToDateTimeString(tttXTime, TMC_EXPORT_IMPORT);
}

ExpPWEntry::ExpPWEntry(const ExpPWEntry &ee)
  : group(ee.group), title(ee.title), user(ee.user),
  expirylocdate(ee.expirylocdate), expiryexpdate(ee.expiryexpdate),
  expirytttXTime(ee.expirytttXTime), et(ee.et)
{
  memcpy(uuid, ee.uuid, sizeof(uuid));
}

ExpPWEntry &ExpPWEntry::operator=(const ExpPWEntry &that)
{
  if (this != &that) {
    memcpy(uuid, that.uuid, sizeof(uuid));
    group = that.group;
    title = that.title;
    user = that.user;
    expirylocdate = that.expirylocdate;
    expiryexpdate = that.expiryexpdate;
    expirytttXTime = that.expirytttXTime;
    et = that.et;
  }
  return *this;
}

void ExpiredList::Add(const CItemData &ci)
{
  // Not valid for aliases or shortcuts!
  const CItemData::EntryType et = ci.GetEntryType();
  if (et == CItemData::ET_ALIAS || et == CItemData::ET_SHORTCUT)
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
  uuid_array_t uuid;
  ci.GetUUID(uuid);
  ExpiredList::iterator iter;

  iter = std::find_if(begin(), end(), ee_equal_uuid(uuid));
  if (iter != end())
    erase(iter);
}

ExpiredList ExpiredList::GetExpired(const int &idays)
{
  ExpiredList retval;
  ExpiredList::iterator iter;
  struct tm st;

  time_t now, exptime;
  time(&now);
#if (_MSC_VER >= 1400)
  errno_t err;
  err = localtime_s(&st, &now);  // secure version
  ASSERT(err == 0);
#else
  st = *localtime(&now);
#endif
  st.tm_mday += idays;
  // Note: mktime will normalise the date structure before converting to time_t
  exptime = mktime(&st);
  if (exptime == (time_t)-1)
    exptime = now;

  for (iter = begin(); iter != end(); iter++) {
    if (iter->expirytttXTime < exptime) {
      retval.push_back(*iter);
    }
  }

  return retval;
}
