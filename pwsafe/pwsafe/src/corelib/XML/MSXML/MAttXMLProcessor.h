/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// MAttXMLProcessor.h : header file
//

#ifndef __MATTXMLPROCESSOR_H
#define __MATTXMLPROCESSOR_H

#include "../../UUIDGen.h"
#include "../../Command.h"

#include "os/typedefs.h"

#include <vector>

typedef std::vector<CUUIDGen> UUIDVector;

class PWScore;
class PWSAttfile;

class MAttXMLProcessor
{
public:
  MAttXMLProcessor(PWScore *pcore, MultiCommands *p_multicmds,
                    CReport *prpt);
  ~MAttXMLProcessor();

  bool Process(const bool &bvalidation,
               const stringT &strXMLFileName, const stringT &strXSDFileName,
               PWSAttfile *pimport);

  stringT getXMLErrors() {return m_strXMLErrors;}
  stringT getSkippedList() {return m_strSkippedList;}

  int getNumAttachmentsValidated() {return m_numAttachmentsValidated;}
  int getNumAttachmentsImported() {return m_numAttachmentsImported;}
  int getNumAttachmentsSkipped() {return m_numAttachmentsSkipped;}

private:
  PWScore *m_pXMLcore;
  MultiCommands *m_pmulticmds;
  CReport *m_prpt;
  PWSAttfile *m_pimport;

  stringT m_strXMLErrors, m_strSkippedList;
  int m_numAttachmentsValidated, m_numAttachmentsImported, m_numAttachmentsSkipped;
  int m_MSXML_Version;
  bool m_bValidation;
};

#endif /* __MATTXMLPROCESSOR_H */
