/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file AttThreadParms.h
//-----------------------------------------------------------------------------

#pragma once

#include "core/StringX.h"

// Thread parameters

class CWZPropertySheet;
class CWZFinish;
class PWScore;
class CReport;

#define NULL 0

struct WZExecuteThreadParms {
  WZExecuteThreadParms()
  : status(0), nID(0), pWZPSH(NULL), pWZFinish(NULL), pcore(NULL),
  prpt(NULL), sx_Filename(L""), bAdvanced(false), csResults(L""),
  numProcessed(0)
  {}

  WZExecuteThreadParms(const WZExecuteThreadParms &thpms)
    : status(thpms.status), nID(thpms.nID), pWZPSH(thpms.pWZPSH),
    pWZFinish(thpms.pWZFinish), pcore(thpms.pcore), prpt(thpms.prpt),
    sx_Filename(thpms.sx_Filename), csResults(thpms.csResults),
    bAdvanced(thpms.bAdvanced), numProcessed(thpms.numProcessed)
  {
  }

  WZExecuteThreadParms &operator=(const WZExecuteThreadParms &thpms)
  {
    if (this != &thpms) {
      status = thpms.status;
      nID = thpms.nID;
      pWZPSH = thpms.pWZPSH;
      pWZFinish = thpms.pWZFinish;
      pcore = thpms.pcore;
      prpt = thpms.prpt;
      sx_Filename = thpms.sx_Filename;
      csResults = thpms.csResults;
      bAdvanced = thpms.bAdvanced;
      numProcessed = thpms.numProcessed;
    }
    return *this;
  }

  UINT nID;
  int status;
  StringX sx_Filename;

  CWZPropertySheet *pWZPSH;
  CWZFinish *pWZFinish;
  PWScore *pcore;
  CReport *prpt;

  std::wstring csResults;
  bool bAdvanced;
  int numProcessed;
};
