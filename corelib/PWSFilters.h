/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __PWSFILTERS_H
#define __PWSFILTERS_H

#include "MyString.h"
#include "filters.h"
#include "PWSfile.h"
#include "match.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

#ifdef UNICODE
typedef std::wifstream ifstreamT;
typedef std::wofstream ofstreamT;
#else
typedef std::ifstream ifstreamT;
typedef std::ofstream ofstreamT;
#endif
typedef std::vector<stringT>::const_iterator vciter;
typedef std::vector<stringT>::iterator viter;

namespace PWSFilters {

  std::string GetFilterXML(const st_filters &filters, bool bFile);
  void GetFilterTestXML(const st_FilterData &st_fldata, std::ostringstream &oss,
                        bool bFile);
  std::string GetFilterXMLHeader(const CMyString &currentfile,
                                 PWSfile::HeaderRecord &hdr);

  int WriteFilterXMLFile(const CMyString &filename, PWSfile::HeaderRecord hdr,
                         const CMyString currentfile, MapFilters &mapfilters);
  int WriteFilterXMLFile(std::ostringstream &oss, PWSfile::HeaderRecord hdr,
                         const CMyString currentfile, MapFilters &mapfilters);
  int ImportFilterXMLFile(MapFilters &mapfilters,
                          const CString &strXMLData,
                          const CString &strXMLFileName,
                          const CString &strXSDFileName, CString &strErrors);

  CString GetFilterDescription(const st_FilterData &st_fldata);

  const char * szentry[];
};

#endif  // __PWSFILTERS_H