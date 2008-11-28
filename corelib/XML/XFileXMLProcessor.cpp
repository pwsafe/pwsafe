/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the STANDARD and UNMODIFIED
* Xerces library V3.0.0 released on September 29, 2008
*
* See http://xerces.apache.org/xerces-c/
*
* Note: This is a cross-platform library and can be linked in as a
* Static library or used as a dynamic library e.g. DLL in Windows.
* To use the static version, the following pre-processor statement
* must be defined: XERCES_STATIC_LIBRARY
*
*/

/*
* NOTE: Xerces characters are ALWAYS in UTF-16 (may or may not be wchar_t 
* depending on platform).
* Non-unicode builds will need convert any results from parsing the XML
* document from UTF-16 to ASCII.
*/

#include "XMLDefs.h"

#if USE_XML_LIBRARY == XERCES

// PWS includes
#include "XFileXMLProcessor.h"
#include "XFileSAX2Handlers.h"
#include "XSecMemMgr.h"

#include "../ItemData.h"
#include "../corelib.h"
#include "../PWScore.h"
#include "../UnknownField.h"
#include "../PWSprefs.h"

#include <sys/types.h>
#include <sys/stat.h>

// Xerces includes
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/XMLGrammarDescription.hpp>

#if defined(XERCES_NEW_IOSTREAMS)
#include <fstream>
#else
#include <fstream.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

XFileXMLProcessor::XFileXMLProcessor(PWScore *core, 
                                     UUIDList *possible_aliases,
                                     UUIDList *possible_shortcuts)
  : m_xmlcore(core), m_delimiter(TCHAR('^')),
    m_possible_aliases(possible_aliases), m_possible_shortcuts(possible_shortcuts)
{
}

XFileXMLProcessor::~XFileXMLProcessor()
{
}

// ---------------------------------------------------------------------------
bool XFileXMLProcessor::Process(const bool &bvalidation, const stringT &ImportedPrefix,
                                const stringT &strXMLFileName, const stringT &strXSDFileName,
                                int &nITER, int &nRecordsWithUnknownFields, UnknownFieldList &uhfl)
{
  bool bEerrorOccurred = false;
  bool b_into_empty = false;
  stringT cs_validation;
  LoadAString(cs_validation, IDSC_XMLVALIDATION);
  stringT cs_import;
  LoadAString(cs_import, IDSC_XMLIMPORT);
  m_strResultText = _T("");
  m_bValidation = bvalidation;  // Validate or Import

  XSecMemMgr sec_mm;

  // Initialize the XML4C2 system
  try
  {
    XMLPlatformUtils::Initialize(XMLUni::fgXercescDefaultLocale, 0, 0, &sec_mm);
  }
  catch (const XMLException& toCatch)
  {
#ifdef UNICODE
    m_strResultText = stringT(toCatch.getMessage());
#else
    char *szData = XMLString::transcode(toCatch.getMessage());
    m_strResultText = stringT(szData);
    XMLString::release(&szData);
#endif
    return false;
  }

  //  Create a SAX2 parser object.
  SAX2XMLReader* pSAX2Parser = XMLReaderFactory::createXMLReader(&sec_mm);

  // Set non-default features
  pSAX2Parser->setFeature(XMLUni::fgSAX2CoreNameSpacePrefixes, true);
  pSAX2Parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
  pSAX2Parser->setFeature(XMLUni::fgXercesDynamic, false);
  pSAX2Parser->setFeature(XMLUni::fgXercesSchemaFullChecking, true);
  pSAX2Parser->setFeature(XMLUni::fgXercesLoadExternalDTD, false);
  pSAX2Parser->setFeature(XMLUni::fgXercesSkipDTDValidation, true);
 
  // Set properties
  pSAX2Parser->setProperty(XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation,
                           (void *)strXSDFileName.c_str());
  pSAX2Parser->setProperty(XMLUni::fgXercesScannerName,
                           (void *)XMLUni::fgSGXMLScanner);
  pSAX2Parser->setInputBufferSize(4096);

  // Create SAX handler object and install it on the pSAX2Parser, as the
  // document and error pSAX2Handler.
  XFileSAX2Handlers * pSAX2Handler = new XFileSAX2Handlers;
  pSAX2Parser->setContentHandler(pSAX2Handler);
  pSAX2Parser->setErrorHandler(pSAX2Handler);

  if (m_bValidation)
    pSAX2Handler->SetVariables(NULL, m_bValidation, ImportedPrefix, m_delimiter,
                               NULL, NULL);
  else {
    b_into_empty = m_xmlcore->GetNumEntries() == 0;
    pSAX2Handler->SetVariables(m_xmlcore, m_bValidation, ImportedPrefix, m_delimiter,
                               m_possible_aliases, m_possible_shortcuts);
  }

  try
  {
    // Let's begin the parsing now
    pSAX2Parser->parse(strXMLFileName.c_str());
  }
  catch (const OutOfMemoryException&)
  {
    m_strResultText = _T("OutOfMemoryException");
    bEerrorOccurred = true;
  }
  catch (const XMLException& e)
  {
#ifdef UNICODE
    m_strResultText = stringT(e.getMessage());
#else
    char *szData = XMLString::transcode(e.getMessage());
    m_strResultText = stringT(szData);
    XMLString::release(&szData);
#endif
    bEerrorOccurred = true;
  }

  catch (...)
  {
    m_strResultText = _T("\nUnexpected exception during parsing'");
    bEerrorOccurred = true;
  }

  if (pSAX2Handler->getIfErrors())
    bEerrorOccurred = true;
  else {
      if(pSAX2Handler->getIfErrors()) {
        m_strResultText = pSAX2Handler->getValidationResult();
      } else {
        if (m_bValidation) {
          m_numEntriesValidated = pSAX2Handler->getNumEntries();
          m_delimiter = pSAX2Handler->getDelimiter();
        } else {
          m_numEntriesImported = pSAX2Handler->getNumEntries();
          // Maybe import errors (PWHistory field processing)
          m_strResultText = pSAX2Handler->getImportErrors();

          m_bRecordHeaderErrors = pSAX2Handler->getRecordHeaderErrors();
          nRecordsWithUnknownFields = pSAX2Handler->getNumRecordsWithUnknownFields();

          if (b_into_empty) {
            m_bDatabaseHeaderErrors = pSAX2Handler->getDatabaseHeaderErrors();
            nITER = pSAX2Handler->getNumIterations();

            UnknownFieldList::const_iterator vi_IterUXFE;
            for (vi_IterUXFE = pSAX2Handler->m_ukhxl.begin();
                 vi_IterUXFE != pSAX2Handler->m_ukhxl.end();
                 vi_IterUXFE++) {
              UnknownFieldEntry ukxfe = *vi_IterUXFE;
              if (ukxfe.st_length > 0) {
                uhfl.push_back(ukxfe);
              }
            }

            PWSprefs *prefs = PWSprefs::GetInstance();
            int ivalue;
            if ((ivalue = pSAX2Handler->getXMLPref(bDisplayExpandedAddEditDlg)) != -1)
              prefs->SetPref(PWSprefs::DisplayExpandedAddEditDlg, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bMaintainDateTimeStamps)) != -1)
              prefs->SetPref(PWSprefs::MaintainDateTimeStamps, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bPWUseDigits)) != -1)
              prefs->SetPref(PWSprefs::PWUseDigits, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bPWUseEasyVision)) != -1)
              prefs->SetPref(PWSprefs::PWUseEasyVision, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bPWUseHexDigits)) != -1)
              prefs->SetPref(PWSprefs::PWUseHexDigits, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bPWUseLowercase)) != -1)
              prefs->SetPref(PWSprefs::PWUseLowercase, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bPWUseSymbols)) != -1)
              prefs->SetPref(PWSprefs::PWUseSymbols, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bPWUseUppercase)) != -1)
              prefs->SetPref(PWSprefs::PWUseUppercase, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bPWMakePronounceable)) != -1)
              prefs->SetPref(PWSprefs::PWMakePronounceable, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bSaveImmediately)) != -1)
              prefs->SetPref(PWSprefs::SaveImmediately, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bSavePasswordHistory)) != -1)
              prefs->SetPref(PWSprefs::SavePasswordHistory, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bShowNotesDefault)) != -1)
              prefs->SetPref(PWSprefs::ShowNotesDefault, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bShowPasswordInTree)) != -1)
              prefs->SetPref(PWSprefs::ShowPasswordInTree, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bShowPWDefault)) != -1)
              prefs->SetPref(PWSprefs::ShowPWDefault, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bShowUsernameInTree)) != -1)
              prefs->SetPref(PWSprefs::ShowUsernameInTree, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bSortAscending)) != -1)
              prefs->SetPref(PWSprefs::SortAscending, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(bUseDefaultUser)) != -1)
              prefs->SetPref(PWSprefs::UseDefaultUser, ivalue == 1);
            if ((ivalue = pSAX2Handler->getXMLPref(iIdleTimeout)) != -1)
              prefs->SetPref(PWSprefs::IdleTimeout, ivalue);
            if ((ivalue = pSAX2Handler->getXMLPref(iNumPWHistoryDefault)) != -1)
              prefs->SetPref(PWSprefs::NumPWHistoryDefault, ivalue);
            if ((ivalue = pSAX2Handler->getXMLPref(iPWDefaultLength)) != -1)
              prefs->SetPref(PWSprefs::PWDefaultLength, ivalue);
            if ((ivalue = pSAX2Handler->getXMLPref(iTreeDisplayStatusAtOpen)) != -1)
              prefs->SetPref(PWSprefs::TreeDisplayStatusAtOpen, ivalue);
            if ((ivalue = pSAX2Handler->getXMLPref(iPWDigitMinLength)) != -1)
              prefs->SetPref(PWSprefs::PWDigitMinLength, ivalue);
            if ((ivalue = pSAX2Handler->getXMLPref(iPWLowercaseMinLength)) != -1)
              prefs->SetPref(PWSprefs::PWLowercaseMinLength, ivalue);
            if ((ivalue = pSAX2Handler->getXMLPref(iPWSymbolMinLength)) != -1)
              prefs->SetPref(PWSprefs::PWSymbolMinLength, ivalue);
            if ((ivalue = pSAX2Handler->getXMLPref(iPWUppercaseMinLength)) != -1)
              prefs->SetPref(PWSprefs::PWUppercaseMinLength, ivalue);
            if (!pSAX2Handler->getDefaultAutotypeString().empty())
              prefs->SetPref(PWSprefs::DefaultAutotypeString,
                             pSAX2Handler->getDefaultAutotypeString().c_str());
            if (!pSAX2Handler->getDefaultUsername().empty())
              prefs->SetPref(PWSprefs::DefaultUsername,
                             pSAX2Handler->getDefaultUsername().c_str());
          } else
            m_bDatabaseHeaderErrors = false;
        }
      }
    if (!pSAX2Handler->getIfErrors()) {
      m_strResultText = pSAX2Handler->getValidationResult();
    } else {
      Format(m_strResultText, IDSC_XMLPARSEERROR, 
             m_bValidation ? cs_validation.c_str() : cs_import.c_str());
    }
  }

  //  Delete the pSAX2Parser itself.  Must be done prior to calling Terminate, below.
  delete pSAX2Parser;
  delete pSAX2Handler;

  // And call the termination method
  XMLPlatformUtils::Terminate();

  return !bEerrorOccurred;
}

#endif /* USE_XML_LIBRARY == XERCES */
