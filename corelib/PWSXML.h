/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// PWSXML.h : header file
//

#ifndef __PWSXML_H
#define __PWSXML_H

#include <vector>
#include "MyString.h"
#include "UnknownField.h"
#include "UUIDGen.h"

typedef std::vector<CUUIDGen> UUIDList;

class PWScore;

class PWSXML {
public:
  PWSXML(PWScore *core, UUIDList *possible_aliases, UUIDList *possible_shortcuts);
  ~PWSXML();

  bool XMLProcess(const bool &bvalidation, const CString &ImportedPrefix, 
    const CString &strXMLFileName, const CString &strXSDFileName,
    int &nITER, int &nRecordsWithUnknownFields, UnknownFieldList &uhfl);

  CString m_strResultText;
  int m_numEntriesValidated, m_numEntriesImported, m_MSXML_Version;
  bool m_bDatabaseHeaderErrors, m_bRecordHeaderErrors;

private:
  PWScore *m_xmlcore;
  UUIDList *m_possible_aliases;
  UUIDList *m_possible_shortcuts;
  TCHAR m_delimiter;
  bool m_bValidation;
};
#endif /* __PWSXML_H */
