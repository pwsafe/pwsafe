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
#include "match.h"
#include "PWSfile.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>

class PWSFilters : public std::map<CString, st_filters> {
 public:
  typedef std::pair<CString, st_filters> Pair;
  
  std::string GetFilterXMLHeader(const CMyString &currentfile,
                                 const PWSfile::HeaderRecord &hdr);

  int WriteFilterXMLFile(const CMyString &filename, const PWSfile::HeaderRecord hdr,
                         const CMyString &currentfile);
  int WriteFilterXMLFile(std::ostream &os, PWSfile::HeaderRecord hdr,
                         const CMyString &currentfile);
  int ImportFilterXMLFile(const CString &strXMLData,
                          const CString &strXMLFileName,
                          const CString &strXSDFileName, CString &strErrors);

  static CString GetFilterDescription(const st_FilterData &st_fldata);
};

#endif  // __PWSFILTERS_H
