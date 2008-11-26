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
* NOTE: EXPAT is a NON-validating XML Parser.  All conformity with the
* scheam must be performed in the handlers.  Also, the concept of pre-validation
* before importing is not available.
* As per XML parsing rules, any error stops the parsing immediately.
*
*/

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == EXPAT

// PWS includes
#include "EFileXMLProcessor.h"
#include "EFileHandlers.h"
#include "ESecMemMgr.h"

#include "../ItemData.h"
#include "../corelib.h"
#include "../PWScore.h"
#include "../UnknownField.h"
#include "../PWSprefs.h"
#include "../VerifyFormat.h"
#include "../Util.h"

#include <sys/types.h>
#include <sys/stat.h>

// Expat includes
#include <expat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BUFFSIZE 8192

// File Handler Wrappers as Expat is a C not a C++ interface
static EFileHandlers *pFileHandler(NULL);

void WstartFileElement(void *userdata, const XML_Char *name,
                  const XML_Char **attrs)
{
  ASSERT(pFileHandler);
  pFileHandler->startElement(userdata, name, attrs);
}

void WendFileElement(void *userdata, const XML_Char *name)
{
  ASSERT(pFileHandler);
  pFileHandler->endElement(userdata, name);
}

void WcharacterFileData(void *userdata, const XML_Char *s, int length)
{
  ASSERT(pFileHandler);
  pFileHandler->characterData(userdata, s, length);
}

// Secure Memory Wrappers as Expat is a C not a C++ interface
static ESecMemMgr *pSecMM(NULL);

void* WFile_malloc(size_t size)
{
  ASSERT(pSecMM);
  return pSecMM->malloc(size);
}

void* WFile_realloc(void *p, size_t size)
{
  ASSERT(pSecMM);
  return pSecMM->realloc(p, size);
}

void WFile_free(void *p)
{
  ASSERT(pSecMM);
  pSecMM->free(p);
}

EFileXMLProcessor::EFileXMLProcessor(PWScore *core,
                                     UUIDList *possible_aliases,
                                     UUIDList *possible_shortcuts)
  : m_xmlcore(core), m_delimiter(TCHAR('^')),
    m_possible_aliases(possible_aliases), m_possible_shortcuts(possible_shortcuts)
{
  pSecMM = new ESecMemMgr;
  pFileHandler = new EFileHandlers;
}

EFileXMLProcessor::~EFileXMLProcessor()
{
  if (pFileHandler) {
    delete pFileHandler;
    pFileHandler = NULL;
  }
  // Should be after freeing of handlers
  if (pSecMM) {
    delete pSecMM;
    pSecMM = NULL;
  }
}

// ---------------------------------------------------------------------------
bool EFileXMLProcessor::Process(const bool &bvalidation, 
                                const stringT &ImportedPrefix,
                                const stringT &strXMLFileName, 
                                const stringT & /* XML Schema File Name */,
                                int &nITER, 
                                int &nRecordsWithUnknownFields, 
                                UnknownFieldList &uhfl)
{
  // Open the file
  std::FILE *fd = NULL;
#if _MSC_VER >= 1400
  _tfopen_s(&fd, strXMLFileName.c_str(), _T("r"));
#else
  fd = _tfopen(strXMLFileName.c_str(), _T("r"));
#endif
  if (fd == NULL)
    return false;

  bool bEerrorOccurred = false;
  bool b_into_empty = m_xmlcore->GetNumEntries() == 0;
  stringT cs_validation;
  LoadAString(cs_validation, IDSC_XMLVALIDATION);
  stringT cs_import;
  LoadAString(cs_import, IDSC_XMLIMPORT);
  m_strResultText = _T("");
  m_ImportedPrefix = ImportedPrefix;

  // Validate or Import
  pFileHandler->SetMode(bvalidation);

  // Tell Expat about our memory suites
  XML_Memory_Handling_Suite ms = {WFile_malloc, WFile_realloc, WFile_free};

  //  Create a parser object.
  XML_Parser pParser = XML_ParserCreate_MM(NULL, &ms, NULL);

  // Set non-default features
  XML_SetParamEntityParsing(pParser, XML_PARAM_ENTITY_PARSING_NEVER);

  // Create handler object and install it on the Parser, as the
  // document Handler.
  XML_SetElementHandler(pParser, WstartFileElement, WendFileElement);
  XML_SetCharacterDataHandler(pParser, WcharacterFileData);
  // Set userdata paraemter for handlers to be the parser itself
  XML_UseParserAsHandlerArg(pParser);

  void * buffer = pSecMM->malloc(BUFFSIZE);
  int done(0), numread;
  enum XML_Status status;
  while (done == 0) {
    numread = (int)fread(buffer, 1, BUFFSIZE, fd);
    done = feof(fd);

    status = XML_Parse(pParser, (char *)buffer, numread, done);
    if (status == XML_STATUS_ERROR || status == XML_STATUS_SUSPENDED) {
      break;
    }
  };
  pSecMM->free(buffer);
  fclose(fd);

  if (bvalidation)
    m_numEntriesValidated = pFileHandler->getNumEntries();
  else
    m_numEntriesImported = pFileHandler->getNumEntries();

  if (pFileHandler->getIfErrors()) {
    bEerrorOccurred = true;
    Format(m_strResultText, _T("Parse error at line %d, column %d:\nError code %d, %s\n%s\n"),
           XML_GetCurrentLineNumber(pParser),
           XML_GetCurrentColumnNumber(pParser),
           pFileHandler->getErrorCode(),
           pFileHandler->getErrorMessage().c_str(),
           XML_ErrorString(XML_GetErrorCode(pParser)));
  } else {
    if (!bvalidation) {
      wchar_t delimiter = pFileHandler->getDelimiter();
      if (delimiter != _T('\0'))
        m_delimiter = (TCHAR)delimiter;

      // Now add entries
      AddEntries();

      // Maybe import errors (PWHistory field processing)
      m_strResultText = pFileHandler->getErrorMessage();

      m_bRecordHeaderErrors = pFileHandler->getRecordHeaderErrors();
      nRecordsWithUnknownFields = pFileHandler->getNumRecordsWithUnknownFields();

      if (b_into_empty) {
        m_bDatabaseHeaderErrors = pFileHandler->getDatabaseHeaderErrors();
        nITER = pFileHandler->getNumIterations();

        UnknownFieldList::const_iterator vi_IterUXFE;
        for (vi_IterUXFE = pFileHandler->m_ukhxl.begin();
             vi_IterUXFE != pFileHandler->m_ukhxl.end();
             vi_IterUXFE++) {
          UnknownFieldEntry ukxfe = *vi_IterUXFE;
          if (ukxfe.st_length > 0) {
            uhfl.push_back(ukxfe);
          }
        }

        PWSprefs *prefs = PWSprefs::GetInstance();
        int ivalue;
        if ((ivalue = pFileHandler->getXMLPref(XLP_BDISPLAYEXPANDEDADDEDITDLG)) != -1)
          prefs->SetPref(PWSprefs::DisplayExpandedAddEditDlg, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BMAINTAINDATETIMESTAMPS)) != -1)
          prefs->SetPref(PWSprefs::MaintainDateTimeStamps, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BPWUSEDIGITS)) != -1)
          prefs->SetPref(PWSprefs::PWUseDigits, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BPWUSEEASYVISION)) != -1)
          prefs->SetPref(PWSprefs::PWUseEasyVision, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BPWUSEHEXDIGITS)) != -1)
          prefs->SetPref(PWSprefs::PWUseHexDigits, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BPWUSELOWERCASE)) != -1)
          prefs->SetPref(PWSprefs::PWUseLowercase, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BPWUSESYMBOLS)) != -1)
          prefs->SetPref(PWSprefs::PWUseSymbols, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BPWUSEUPPERCASE)) != -1)
          prefs->SetPref(PWSprefs::PWUseUppercase, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BPWMAKEPRONOUNCEABLE)) != -1)
          prefs->SetPref(PWSprefs::PWMakePronounceable, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BSAVEIMMEDIATELY)) != -1)
          prefs->SetPref(PWSprefs::SaveImmediately, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BSAVEPASSWORDHISTORY)) != -1)
          prefs->SetPref(PWSprefs::SavePasswordHistory, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BSHOWNOTESDEFAULT)) != -1)
          prefs->SetPref(PWSprefs::ShowNotesDefault, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BSHOWPASSWORDINTREE)) != -1)
          prefs->SetPref(PWSprefs::ShowPasswordInTree, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BSHOWPWDEFAULT)) != -1)
          prefs->SetPref(PWSprefs::ShowPWDefault, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BSHOWUSERNAMEINTREE)) != -1)
          prefs->SetPref(PWSprefs::ShowUsernameInTree, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BSORTASCENDING)) != -1)
          prefs->SetPref(PWSprefs::SortAscending, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_BUSEDEFAULTUSER)) != -1)
          prefs->SetPref(PWSprefs::UseDefaultUser, ivalue == 1);
        if ((ivalue = pFileHandler->getXMLPref(XLP_IIDLETIMEOUT)) != -1)
          prefs->SetPref(PWSprefs::IdleTimeout, ivalue);
        if ((ivalue = pFileHandler->getXMLPref(XLP_INUMPWHISTORYDEFAULT)) != -1)
          prefs->SetPref(PWSprefs::NumPWHistoryDefault, ivalue);
        if ((ivalue = pFileHandler->getXMLPref(XLP_IPWDEFAULTLENGTH)) != -1)
          prefs->SetPref(PWSprefs::PWDefaultLength, ivalue);
        if ((ivalue = pFileHandler->getXMLPref(XLP_ITREEDISPLAYSTATUSATOPEN)) != -1)
          prefs->SetPref(PWSprefs::TreeDisplayStatusAtOpen, ivalue);
        if ((ivalue = pFileHandler->getXMLPref(XLP_IPWDIGITMINLENGTH)) != -1)
          prefs->SetPref(PWSprefs::PWDigitMinLength, ivalue);
        if ((ivalue = pFileHandler->getXMLPref(XLP_IPWLOWERCASEMINLENGTH)) != -1)
          prefs->SetPref(PWSprefs::PWLowercaseMinLength, ivalue);
        if ((ivalue = pFileHandler->getXMLPref(XLP_IPWSYMBOLMINLENGTH)) != -1)
          prefs->SetPref(PWSprefs::PWSymbolMinLength, ivalue);
        if ((ivalue = pFileHandler->getXMLPref(XLP_IPWUPPERCASEMINLENGTH)) != -1)
          prefs->SetPref(PWSprefs::PWUppercaseMinLength, ivalue);
        if (!pFileHandler->getDefaultAutotypeString().empty())
          prefs->SetPref(PWSprefs::DefaultAutotypeString,
                         pFileHandler->getDefaultAutotypeString().c_str());
        if (!pFileHandler->getDefaultUsername().empty())
          prefs->SetPref(PWSprefs::DefaultUsername,
                         pFileHandler->getDefaultUsername().c_str());
      } else
        m_bDatabaseHeaderErrors = false;
    }
  }

  // Free the parser
  XML_ParserFree(pParser);

  return !bEerrorOccurred;
}

void EFileXMLProcessor::AddEntries()
{
  vdb_entries::iterator entry_iter;
  CItemData tempitem;
  vdb_entries &ventries = pFileHandler->getVDB_Entries();
  m_strImportErrors = pFileHandler->getErrorMessage();

  for (entry_iter = ventries.begin(); entry_iter != ventries.end(); entry_iter++) {
    pw_entry *cur_entry = *entry_iter;
    uuid_array_t uuid_array;
    tempitem.Clear();
    if (cur_entry->uuid.empty())
      tempitem.CreateUUID();
    else {
      // _stscanf_s always outputs to an "int" using %x even though
      // target is only 1.  Read into larger buffer to prevent data being
      // overwritten and then copy to where we want it!
      unsigned char temp_uuid_array[sizeof(uuid_array_t) + sizeof(int)];
      int nscanned = 0;
      const TCHAR *lpszuuid = cur_entry->uuid.c_str();
      for (unsigned i = 0; i < sizeof(uuid_array_t); i++) {
#if _MSC_VER >= 1400
        nscanned += _stscanf_s(lpszuuid, _T("%02x"), &temp_uuid_array[i]);
#else
        nscanned += _stscanf(lpszuuid, _T("%02x"), &temp_uuid_array[i]);
#endif
        lpszuuid += 2;
      }
      memcpy(uuid_array, temp_uuid_array, sizeof(uuid_array_t));
      if (nscanned != sizeof(uuid_array_t) ||
        m_xmlcore->Find(uuid_array) != m_xmlcore->GetEntryEndIter())
        tempitem.CreateUUID();
      else {
        tempitem.SetUUID(uuid_array);
      }
    }
    StringX newgroup;
    if (!m_ImportedPrefix.empty()) {
      newgroup = m_ImportedPrefix.c_str(); newgroup += _T(".");
    }
    EmptyIfOnlyWhiteSpace(cur_entry->group);
    newgroup += cur_entry->group;
    if (m_xmlcore->Find(newgroup, cur_entry->title, cur_entry->username) !=
      m_xmlcore->GetEntryEndIter()) {
        // Find a unique "Title"
        StringX Unique_Title;
        ItemListConstIter iter;
        int i = 0;
        stringT s_import;
        do {
          i++;
          Format(s_import, IDSC_IMPORTNUMBER, i);
          Unique_Title = cur_entry->title + s_import.c_str();
          iter = m_xmlcore->Find(newgroup, Unique_Title, cur_entry->username);
        } while (iter != m_xmlcore->GetEntryEndIter());
        cur_entry->title = Unique_Title;
    }
    tempitem.SetGroup(newgroup);
    EmptyIfOnlyWhiteSpace(cur_entry->title);
    if (!cur_entry->title.empty())
      tempitem.SetTitle(cur_entry->title, m_delimiter);
    EmptyIfOnlyWhiteSpace(cur_entry->username);
    if (!cur_entry->username.empty())
      tempitem.SetUser(cur_entry->username);
    if (!cur_entry->password.empty())
      tempitem.SetPassword(cur_entry->password);
    EmptyIfOnlyWhiteSpace(cur_entry->url);
    if (!cur_entry->url.empty())
      tempitem.SetURL(cur_entry->url);
    EmptyIfOnlyWhiteSpace(cur_entry->autotype);
    if (!cur_entry->autotype.empty())
      tempitem.SetAutoType(cur_entry->autotype);
    if (!cur_entry->ctime.empty())
      tempitem.SetCTime(cur_entry->ctime.c_str());
    if (!cur_entry->pmtime.empty())
      tempitem.SetPMTime(cur_entry->pmtime.c_str());
    if (!cur_entry->atime.empty())
      tempitem.SetATime(cur_entry->atime.c_str());
    if (!cur_entry->xtime.empty())
      tempitem.SetXTime(cur_entry->xtime.c_str());
    if (!cur_entry->xtime_interval.empty()) {
      int numdays = _ttoi(cur_entry->xtime_interval.c_str());
      if (numdays > 0 && numdays <= 3650)
        tempitem.SetXTimeInt(numdays);
    }
    if (!cur_entry->rmtime.empty())
      tempitem.SetRMTime(cur_entry->rmtime.c_str());

    if (cur_entry->pwp.flags != 0) {
      tempitem.SetPWPolicy(cur_entry->pwp);
    }

    StringX newPWHistory;
    stringT strPWHErrors;
    switch (VerifyImportPWHistoryString(cur_entry->pwhistory, newPWHistory, strPWHErrors)) {
      case PWH_OK:
        tempitem.SetPWHistory(newPWHistory.c_str());
        break;
      case PWH_IGNORE:
        break;
      case PWH_INVALID_HDR:
      case PWH_INVALID_STATUS:
      case PWH_INVALID_NUM:
      case PWH_INVALID_DATETIME:
      case PWH_INVALID_PSWD_LENGTH:
      case PWH_TOO_SHORT:
      case PWH_TOO_LONG:
      case PWH_INVALID_CHARACTER:
      {
        stringT buffer;
        Format(buffer, IDSC_SAXERRORPWH, cur_entry->group.c_str(),
               cur_entry->title.c_str(), cur_entry->username.c_str());
        m_strImportErrors += buffer;
        m_strImportErrors += strPWHErrors;
        break;
      }
      default:
        ASSERT(0);
    }

    EmptyIfOnlyWhiteSpace(cur_entry->notes);
    if (!cur_entry->notes.empty())
      tempitem.SetNotes(cur_entry->notes, m_delimiter);

    if (!cur_entry->uhrxl.empty()) {
      UnknownFieldList::const_iterator vi_IterUXRFE;
      for (vi_IterUXRFE = cur_entry->uhrxl.begin();
        vi_IterUXRFE != cur_entry->uhrxl.end();
        vi_IterUXRFE++) {
          UnknownFieldEntry unkrfe = *vi_IterUXRFE;
          /* #ifdef _DEBUG
          stringT cs_timestamp;
          cs_timestamp = PWSUtil::GetTimeStamp();
          TRACE(_T("%s: Record %s, %s, %s has unknown field: %02x, length %d/0x%04x, value:\n",
          cs_timestamp, cur_entry->group, cur_entry->title, cur_entry->username,
          unkrfe.uc_Type, (int)unkrfe.st_length, (int)unkrfe.st_length);
          PWSDebug::HexDump(unkrfe.uc_pUField, (int)unkrfe.st_length, cs_timestamp);
          #endif /* DEBUG */
          tempitem.SetUnknownField(unkrfe.uc_Type, (int)unkrfe.st_length, unkrfe.uc_pUField);
      }
    }

    // If a potential alias, add to the vector for later verification and processing
    if (cur_entry->entrytype == ALIAS && !cur_entry->bforce_normal_entry) {
      tempitem.GetUUID(uuid_array);
      m_possible_aliases->push_back(uuid_array);
    }
    if (cur_entry->entrytype == SHORTCUT && !cur_entry->bforce_normal_entry) {
      tempitem.GetUUID(uuid_array);
      m_possible_shortcuts->push_back(uuid_array);
    }

    m_xmlcore->AddEntry(tempitem);
    delete cur_entry;
  }
  ventries.clear();
}

#endif /* USE_XML_LIBRARY == EXPAT */
