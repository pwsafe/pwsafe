/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// PWSXMLFilters.h : header file
//

#ifndef __MFILTERXMLPROCESSOR_H
#define __MFILTERXMLPROCESSOR_H

#include "../../PWSFilters.h"
#include "../../StringX.h"

#include <vector>

class MFilterXMLProcessor
{
public:
  MFilterXMLProcessor(PWSFilters &mapfilters, const FilterPool fpool, Asker *pAsker);
  ~MFilterXMLProcessor();

  bool Process(const bool &bvalidation,
               const StringX &strXMLData,
               const stringT &strXMLFileName,
               const stringT &strXSDFileName);

  stringT getXMLErrors() {return m_strXMLErrors;}
  stringT getPWHErrorList() {return m_strPWHErrorList;}
  stringT getRenameList() {return m_strRenameList;}

private:
  Asker *m_pAsker;
  PWSFilters &m_MapXMLFilters;   // So as not to confuse with UI & core
  FilterPool m_FPool;
  stringT m_strXMLErrors;
  stringT m_strRenameList;
  stringT m_strPWHErrorList;
  bool m_bValidation;
};

#endif /* __MFILTERXMLPROCESSOR_H */
