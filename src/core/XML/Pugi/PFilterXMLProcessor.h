/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the PUGI 
*
* See http://pugixml.org/
*
* Note: An actual version of pugixml library is linked to password safe
* in parallel folder ../../pugixml
*
*/

#ifndef __PFILTERXMLPROCESSOR_H
#define __PFILTERXMLPROCESSOR_H

#include "../../pugixml/pugixml.hpp"

// PWS includes
#include "../../StringX.h"
#include "../../PWSFilters.h"
#include "../../Proxy.h"

#include <stdlib.h>
#include <string.h>
#include <vector>

class PFilterXMLProcessor
{
public:
  PFilterXMLProcessor(PWSFilters &mapfilters, const FilterPool fpool, Asker *pAsker, Reporter *pReporter);
  ~PFilterXMLProcessor();
    
  bool ReadXML(const StringX &strXMLData,
               const stringT &strXMLFileName); // No XSD file required

  bool Process(const bool &bValidation);

  stringT getXMLErrors() {return m_strXMLErrors;}

private:
  bool ReadXMLFilter(pugi::xml_node &froot, const stringT &fname, st_filters &cur_filter);
  bool ReadXMLFilterEntry(pugi::xml_node &eroot, const stringT &fname, int idx, st_FilterRow &frow, FilterType &ftype);
  bool ReadXMLFilterTest(pugi::xml_node &troot, const stringT &fname, int idx, st_FilterRow &frow);
    
  PFilterXMLProcessor& operator=(const PFilterXMLProcessor&) = delete; // Do not implement
  Asker *m_pAsker;
  Reporter *m_pReporter;
  PWSFilters &m_MapXMLFilters; // So as not to confuse with UI & core
  FilterPool m_FPool;
  stringT m_strXMLErrors;
  bool m_bValidation;
  // Store document tree between Read and Process
  pugi::xml_document m_doc;
};

#endif /* __PFILTERXMLPROCESSOR_H */
