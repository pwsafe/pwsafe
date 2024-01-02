/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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

struct WZExecuteThreadParms {
  WZExecuteThreadParms()
  : nID(0), status(0),
  sx_Filename(L""), sx_exportpasskey(L""),
  pWZPSH(nullptr), pWZFinish(nullptr), 
  pcore(nullptr), prpt(nullptr), 
  csResults(L""),
  bAdvanced(false), bExportDBFilters(false), bCancel(false),
  numProcessed(0)
  {}

  WZExecuteThreadParms(const WZExecuteThreadParms &thpms)
    : nID(thpms.nID), status(thpms.status), 
    sx_Filename(thpms.sx_Filename), sx_exportpasskey(thpms.sx_exportpasskey),
    pWZPSH(thpms.pWZPSH), pWZFinish(thpms.pWZFinish), 
    pcore(thpms.pcore), prpt(thpms.prpt),
    csResults(thpms.csResults),
    bAdvanced(thpms.bAdvanced), bExportDBFilters(thpms.bExportDBFilters), bCancel(thpms.bCancel),
    numProcessed(thpms.numProcessed)
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
      sx_exportpasskey = thpms.sx_exportpasskey;
      csResults = thpms.csResults;
      bAdvanced = thpms.bAdvanced;
      bExportDBFilters = thpms.bExportDBFilters;
      numProcessed = thpms.numProcessed;
      bCancel = thpms.bCancel;
    }
    return *this;
  }

  UINT nID;
  int status;
  StringX sx_Filename, sx_exportpasskey;

  CWZPropertySheet *pWZPSH;
  CWZFinish *pWZFinish;
  PWScore *pcore;
  CReport *prpt;

  std::wstring csResults;
  bool bAdvanced, bExportDBFilters, bCancel;
  int numProcessed;
};
