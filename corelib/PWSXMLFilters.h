/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// PWSXMLFilters.h : header file
//

#ifndef __PWSXMLFILTERS_H
#define __PWSXMLFILTERS_H

#include <vector>
#include "filters.h"
#include "PWSFilters.h"

class PWSXMLFilters
{
public:
  PWSXMLFilters(PWSFilters &filters);
  ~PWSXMLFilters();

  bool XMLFilterProcess(const bool &bvalidation,
                        const CString &strXMLData,
                        const CString &strXMLFileName, 
                        const CString &strXSDFileName);

  CString m_strResultText;
  int m_MSXML_Version;

private:
  bool m_bValidation;
  PWSFilters &m_filters;
};

#endif /* __PWSXMLFILTERS_H */
