/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __VERIFYFORMAT_H
#define __VERIFYFORMAT_H

// VerifyFormat.h
//-----------------------------------------------------------------------------

#include "StringX.h"
#include "PwsPlatform.h"
#include "os/typedefs.h"

// Verify PWHistory String return codes
enum {PWH_OK = 0, PWH_IGNORE, PWH_INVALID_HDR, PWH_INVALID_STATUS,
      PWH_INVALID_NUM, PWH_INVALID_DATETIME, PWH_PSWD_LENGTH_NOTHEX,
      PWH_INVALID_PSWD_LENGTH, PWH_TOO_SHORT, PWH_TOO_LONG,
      PWH_INVALID_FIELD_LENGTH, PWH_INVALID_CHARACTER};
bool VerifyASCDateTimeString(const stringT &time_str, time_t &t);
bool VerifyXMLDateTimeString(const stringT &time_str, time_t &t);
bool VerifyXMLDateString(const stringT &time_str, time_t &t);
bool VerifyImportDateTimeString(const stringT &time_str, time_t &t);
int VerifyTextImportPWHistoryString(const StringX &PWHistory, StringX &newPWHistory,
                                    stringT &strErrors);
int VerifyXMLImportPWHistoryString(const StringX &PWHistory, StringX &newPWHistory,
                                   stringT &strErrors);
bool verifyDTvalues(int yyyy, int mon, int dd,
                    int hh, int min, int ss);

#endif /* __VERIFYFORMAT_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
