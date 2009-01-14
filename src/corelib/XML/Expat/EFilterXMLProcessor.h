/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the STANDARD and UNMODIFIED
* Expat library V2.0.1 released on June 5, 2007
*
* See http://expat.sourceforge.net/
*
* Note: This is a cross-platform library and can be linked in as a
* Static library or used as a dynamic library e.g. DLL in Windows.
*
* As per XML parsing rules, any error stops the parsing immediately.
*/

#ifndef __EFILTERXMLPROCESSOR_H
#define __EFILTERXMLPROCESSOR_H

#include "EFilterHandlers.h"

// PWS includes
#include "../../StringX.h"
#include "../../PWSFilters.h"
#include "../../Proxy.h"

#include <stdlib.h>
#include <string.h>
#include <vector>

// Expat includes
#include <expat.h>

class EFilterXMLProcessor
{
public:
  EFilterXMLProcessor(PWSFilters &mapfilters, const FilterPool fpool, Asker *pAsker);
  ~EFilterXMLProcessor();

  bool Process(const bool &bvalidation,
               const StringX &strXMLData,
               const stringT &strXMLFileName,
               const stringT &strXSDFileName);

  stringT getResultText() {return m_strResultText;}

private:
  Asker *m_pAsker;
  PWSFilters &m_MapFilters;
  FilterPool m_FPool;
  stringT m_strResultText;
  bool m_bValidation;
};

#endif /* __EFILTERXMLPROCESSOR_H */
