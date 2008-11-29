/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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
*/

#ifndef __EFILTERHANDLERS_H
#define __EFILTERHANDLERS_H

#include "EFilterValidator.h"

// PWS includes
#include "../../PWSFilters.h"
#include "../../proxy.h"

#include <set>
#include <stack>

// Expat includes
#include <expat.h>

class PWSFilters;

class EFilterHandlers
{
public:
  EFilterHandlers();
  virtual ~EFilterHandlers();

  // Local variables & function
  PWSFilters *m_MapFilters;
  FilterPool m_FPool;
  int m_type;

  void SetVariables(Asker *pAsker, PWSFilters *mapfilters, const FilterPool fpool,
                    const bool &bValidation)
  {m_pAsker = pAsker; m_MapFilters = mapfilters, m_FPool = fpool; m_bValidation = bValidation;}

  // -----------------------------------------------------------------------
  //  Handlers for the ContentHandler interface
  // -----------------------------------------------------------------------
  void XMLCALL startElement(void *userdata, const XML_Char *name,
                            const XML_Char **attrs);
  void XMLCALL endElement(void *userdata, const XML_Char *name);
  void XMLCALL characterData(void *userdata, const XML_Char *s,
                             int length);

  bool getIfErrors() {return m_bErrors;}
  int getErrorCode() {return m_iErrorCode;}
  stringT getErrorMessage() {return m_strErrorMessage;}

private:
  // Local variables
  EFilterValidator *m_pValidator;

  // To ensure filtername is unique
  std::set<const stringT> m_unique_filternames;

  // Date types
  std::stack<int> m_element_datatypes;

  // Local variables
  st_filters *cur_filter;
  st_FilterRow *cur_filterentry;

  StringX m_strElemContent;
  bool m_bValidation;

  stringT m_strErrorMessage;
  int m_iErrorCode;
  int m_fieldlen;
  int m_iXMLVersion;
  bool m_bentrybeingprocessed;
  bool m_bErrors;
  unsigned char m_ctype;
  unsigned char *m_pfield;
  Asker *m_pAsker;
};

#endif /* __EFILTERHANDLERS_H */
