/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes File XML using the PUGI library
*
* See http://pugixml.org/
*
* Note: An actual version of pugixml library is linked to password safe
* in parallel folder ../../pugixml
*
*/

#ifndef __PFILEXMLPROCESSOR_H
#define __PFILEXMLPROCESSOR_H

#include "../XMLDefs.h"    // Required if testing "USE_XML_LIBRARY"

#if !defined(USE_XML_LIBRARY)

#include "../../pugixml/pugixml.hpp"

#include "../../UnknownField.h"
#include "os/typedefs.h"
#include "os/UUID.h"

#include "../XMLFileValidation.h"
#include "../XMLFileHandlers.h"

#include <stdlib.h>
#include <string.h>
#include <vector>

class PWScore;

class PFileXMLProcessor : public XMLFileHandlers
{
public:
  PFileXMLProcessor(PWScore *pcore, UUIDVector *pPossible_Aliases,
                    UUIDVector *pPossible_Shortcuts, MultiCommands *p_multicmds,
                    CReport *pRpt);
  ~PFileXMLProcessor();

  bool ReadXML(const StringX &strXMLData,
               const stringT &strXMLFileName); // No XSD file required

  bool Process(const bool &bvalidation, const stringT &ImportedPrefix, 
               const bool &bImportPSWDsOnly);

  int getNumEntriesValidated() {return m_numEntriesValidated;}
  int getNumEntriesImported() {return XMLFileHandlers::getNumEntries();}
  int getNumEntriesSkipped() {return m_numEntriesSkipped;}
  int getNumEntriesRenamed() {return m_numEntriesRenamed;}
  int getNumEntriesPWHErrors() {return m_numEntriesPWHErrors;}

private:
  PWScore *m_pXMLcore;
  UUIDVector *m_pPossible_Aliases;
  UUIDVector *m_pPossible_Shortcuts;
  MultiCommands *m_pmulticmds;
  CReport *m_pRpt;

  XMLFileValidation *m_pValidator;

  int m_numEntriesValidated;

  // Store document tree between Read and Process
  pugi::xml_document m_doc;

  bool ReadXMLElements(pugi::xml_node &froot, const stringT &tag, int iroot);
  bool CheckElementHierachy(int iroot, int icurrent);
  bool CheckElementValue(const TCHAR *value, int icurrent);
};

#endif /* !defined(USE_XML_LIBRARY) */
#endif /* __PFILEXMLPROCESSOR_H */
