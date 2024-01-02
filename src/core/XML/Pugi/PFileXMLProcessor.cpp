/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* his routine processes File XML using the PUGI library
*
* See http://pugixml.org/
*
* Note: An actual version of pugixml library is linked to password safe
* in parallel folder ../../pugixml
*
*/
#if !defined(_WIN32) || defined(__WX__)
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif
#endif // !defined(_WIN32) || defined(__WX__)
#ifdef _WIN32
#define _(x) _T(x)
#endif // _WIN32

#include "../XMLDefs.h"    // Required if testing "USE_XML_LIBRARY"

#if !defined(USE_XML_LIBRARY)

// PWS includes
#include "PFileXMLProcessor.h"

#include "../../StringX.h"
#include "../../core.h"
#include "../../VerifyFormat.h"
#include "core/PWScore.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <map>
#include <algorithm>
#include <stdexcept>

#include "os/pws_tchar.h"  // For Linux build not finding _tcslen!

/*!
 * Check pointer before calling compare
 */
static bool SafeCompare(const TCHAR *v1, const TCHAR *v2)
{
  return (v1 != nullptr && v2 != nullptr && stringT(v1) == v2);
}

/*!
 * Constructor. CReport is only needed when following report is written while working
 */

PFileXMLProcessor::PFileXMLProcessor(PWScore *pcore,
                                     UUIDVector *pPossible_Aliases,
                                     UUIDVector *pPossible_Shortcuts,
                                     MultiCommands *p_multicmds,
                                     CReport *pRpt)
  : m_pXMLcore(pcore),
    m_pPossible_Aliases(pPossible_Aliases),
    m_pPossible_Shortcuts(pPossible_Shortcuts),
    m_pmulticmds(p_multicmds),
    m_pRpt(pRpt)
{
  m_pValidator = new XMLFileValidation;
  ASSERT(m_pValidator);
  m_bEntryBeingProcessed = false;
  m_delimiter = TCHAR('^');
  m_bValidation = true;
  m_numEntriesValidated = 0;
}

PFileXMLProcessor::~PFileXMLProcessor()
{
  delete m_pValidator;
}

/*!
 * Build internal tree with content from buffer of file. If File name is given this one is processed.
 */

bool PFileXMLProcessor::ReadXML(const StringX &strXMLData,
                                const stringT &strXMLFileName)
{
  pugi::xml_parse_result result;
    
  m_strXMLErrors = _T("");
  m_bEntryBeingProcessed = false;
  m_numEntriesValidated = 0;

  if(strXMLFileName.empty())
    result = m_doc.load(strXMLData.c_str());
  else
    result = m_doc.load_file(strXMLFileName.c_str());
    
  if (!result) {
    // An XML load error occurred so display the reason
    // Note: "result.description()" returns char* even in Unicode builds.
    stringT sErrorDesc;
    sErrorDesc = pugi::as_wide(result.description());
    Format(m_strXMLErrors, _("XML error:\n%ls\n%ls\noffset approximately at %d"),
           sErrorDesc.c_str(), strXMLFileName.c_str(), result.offset);
    return false;
  } // load failed
    
  return true;
}

/*!
 * Process the data structure on XML file. In case of validation no external buffer is changed.
 */

bool PFileXMLProcessor::Process(const bool &bValidation,
                                const stringT &ImportedPrefix,
                                const bool &bImportPSWDsOnly)
{
  bool b_into_empty = false;
    
  if (!m_bValidation) {
    b_into_empty = (m_pXMLcore->GetNumEntries() == 0);
  }
    
  pugi::xml_node Root = m_doc.first_child();
    
  m_bValidation = bValidation;

  if (!Root || !SafeCompare(Root.name(), _T("passwordsafe"))) {
    Format(m_strXMLErrors, _("Error in XML structure: excpected \"%ls\", found \"%ls\""),
           _T("passwordsafe"), Root.name());
    return false;
  }
  // Check on mnadatory atttribute "delimiter="
  const TCHAR *delimiter = Root.attribute(_T("delimiter")).value();

  if(delimiter == nullptr || ! *delimiter) {
    Format(m_strXMLErrors, _("Missing delimiter attribute in <passwordsafe>"));
    return false;
  }
    
  m_delimiter = delimiter[0]; // Only one character delimiter is used
    
  // Set start value for XMLFileHandlers
  SetVariables(m_bValidation ? nullptr : m_pXMLcore,
               m_bValidation,
               ImportedPrefix,
               m_delimiter,
               bImportPSWDsOnly,
               m_bValidation ? nullptr : m_pPossible_Aliases,
               m_bValidation ? nullptr : m_pPossible_Shortcuts,
               m_pmulticmds,
               m_pRpt);
    
  st_file_element_data edata;
  if(! m_pValidator->GetElementInfo(Root.name(), edata)) {
    Format(m_strXMLErrors, _("Processing error on XML tag \"<passwordsafe>\""));
    return false;
  }
  int iroot_element = edata.element_code;
  (void) XMLFileHandlers::ProcessStartElement(iroot_element);  // Returns false on validation

  // Iterate over all xml items in the buffer/file
  for (pugi::xml_node_iterator it = Root.begin(); it != Root.end(); ++it) {
    pugi::xml_node node = *it;
    if(node.type() == pugi::node_comment || node.type() == pugi::node_declaration)
      continue;
    if(node.type() != pugi::node_element) {
      Format(m_strXMLErrors, _("Unexpected XML node type %d of \"%ls\", inside of <passwordsafe>"),
             node.type(), it->name());
      return false;
    }
    if(! m_pValidator->GetElementInfo(it->name(), edata)) {
      Format(m_strXMLErrors, _("Unknown XML tag <%ls> in <%ls>"),
             it->name(), _T("passwordsafe"));
      return false;
    }
    int icurrent_element = edata.element_code;
    (void) XMLFileHandlers::ProcessStartElement(icurrent_element); // Return's false on validation
      
    if(m_bValidation && ! CheckElementHierachy(iroot_element, icurrent_element)) {
      Format(m_strXMLErrors, _("Not allowed XML tag <%ls> in <%ls>"),
             it->name(), _T("passwordsafe"));
      return false;
    }
    if(! m_bValidation && (icurrent_element == XLE_ENTRY)) {
      int id = node.attribute(_T("id")).as_int(-1);
      const TCHAR *normal_value = node.attribute(_T("normal")).value();
      ASSERT(m_cur_entry);
      if(id != -1)
        m_cur_entry->id = id;
      m_cur_entry->bforce_normal_entry =
        (SafeCompare(normal_value, _T("1")) ||
         SafeCompare(normal_value, _T("true")));
    }
    // Handle sub-elements of entry
    if(! ReadXMLElements(*it, it->name(), icurrent_element)) {
      // m_strXMLErrors is filled in ReadXMLElements
      return false;
    }
    if(m_bValidation) {
      // Counter number of <entries></entries>
      if(icurrent_element == XLE_ENTRY)
          m_numEntriesValidated++;
      continue;
    }
    XMLFileHandlers::ProcessEndElement(icurrent_element);
  }
  // Close handling of <passwordsafe>
  if(! m_bValidation) {
    XMLFileHandlers::ProcessEndElement(iroot_element);
  }

  // If no error occured and no validation, add imported entries
  if (! m_bValidation) {
    AddXMLEntries();
    if (b_into_empty)
      AddDBPreferences();
  }
    
  return true;
}

/*!
 * Read one XML element, either value (copied into m_sxElementContent) or all child entries of the current element.
 * Call ProcessStartElement and ProcessEndelement (second only when not validating) for each element after processing.
 */

bool PFileXMLProcessor::ReadXMLElements(pugi::xml_node &froot, const stringT &tag, int iroot)
{
  bool bValueFilled = false;
    
  m_sxElemContent = _T("");
  for (pugi::xml_node node = froot.first_child(); node; node = node.next_sibling()) {
    if(node.type() == pugi::node_comment)
      continue;
    if(node.type() == pugi::node_element) {
      st_file_element_data edata;
      if(! m_pValidator->GetElementInfo(node.name(), edata)) {
        Format(m_strXMLErrors, _("Unknown XML tag <%ls> in <%ls>"), node.name(), tag.c_str());
        return false;
      }
      int icurrent_element = m_bEntryBeingProcessed ? edata.element_entry_code : edata.element_code;
      (void) XMLFileHandlers::ProcessStartElement(icurrent_element); // Returns false on validation
      if(m_bValidation && ! CheckElementHierachy(iroot, icurrent_element)) {
        Format(m_strXMLErrors, _("Not allowed XML tag <%ls> in <%ls>"),
               node.name(), tag.c_str());
        return false;
      }
      if(! ReadXMLElements(node, node.name(), icurrent_element))
        return false;
      if(! m_bValidation)
        XMLFileHandlers::ProcessEndElement(icurrent_element);
    }
    else if(node.type() == pugi::node_pcdata || node.type() == pugi::node_cdata) {
      const TCHAR *value = node.value();
      m_sxElemContent += value;
      bValueFilled = true;
    }
  }
  if(m_bValidation && bValueFilled) {
    if(! CheckElementValue(m_sxElemContent.c_str(), iroot)) {
      Format(m_strXMLErrors, _("Wrong XML value \"%ls\" for <%ls>"),
             m_sxElemContent.c_str(), tag.c_str());
      return false;
    }
  }
  return true;
}

/*!
 * Check correct hierachy of XML elements
 */

bool PFileXMLProcessor::CheckElementHierachy(int iroot, int icurrent)
{
  switch(iroot) {
    case XLE_PASSWORDSAFE:
      switch(icurrent) {
        case XLE_PREFERENCES:
        case XLE_ENTRY:
        case XLE_PASSWORDPOLICYNAMES:
        case XLE_EMPTYGROUPS:
          return true;
      }
      break;
    case XLE_PREFERENCES:
      switch(icurrent) {
        case XLE_PREF_SHOWPWDEFAULT:
        case XLE_PREF_SHOWPASSWORDINTREE:
        case XLE_PREF_SORTASCENDING:
        case XLE_PREF_USEDEFAULTUSER:
        case XLE_PREF_SAVEIMMEDIATELY:
        case XLE_PREF_PWUSELOWERCASE:
        case XLE_PREF_PWUSEUPPERCASE:
        case XLE_PREF_PWUSEDIGITS:
        case XLE_PREF_PWUSESYMBOLS:
        case XLE_PREF_PWUSEHEXDIGITS:
        case XLE_PREF_PWUSEEASYVISION:
        case XLE_PREF_MAINTAINDATETIMESTAMPS:
        case XLE_PREF_SAVEPASSWORDHISTORY:
        case XLE_PREF_SHOWNOTESDEFAULT:
        case XLE_PREF_SHOWUSERNAMEINTREE:
        case XLE_PREF_PWMAKEPRONOUNCEABLE:
        case XLE_PREF_LOCKDBONIDLETIMEOUT:
        case XLE_PREF_COPYPASSWORDWHENBROWSETOURL:
        case XLE_PREF_EXCLUDEFROMSCREENCAPTURE:
        case XLE_PREF_PWDEFAULTLENGTH:
        case XLE_PREF_IDLETIMEOUT:
        case XLE_PREF_TREEDISPLAYSTATUSATOPEN:
        case XLE_PREF_NUMPWHISTORYDEFAULT:
        case XLE_PREF_PWDIGITMINLENGTH:
        case XLE_PREF_PWLOWERCASEMINLENGTH:
        case XLE_PREF_PWSYMBOLMINLENGTH:
        case XLE_PREF_PWUPPERCASEMINLENGTH:
        case XLE_PREF_DEFAULTUSERNAME:
        case XLE_PREF_DEFAULTAUTOTYPESTRING:
        case XLE_PREF_DEFAULTSYMBOLS:
          return true;
      }
      break;
    case XLE_ENTRY:
      switch(icurrent) {
        case XLE_GROUP:
        case XLE_TITLE:
        case XLE_USERNAME:
        case XLE_PASSWORD:
        case XLE_TWOFACTORKEY:
        case XLE_TOTPCONFIG:
        case XLE_TOTPSTARTTIME:
        case XLE_TOTPTIMESTEP:
        case XLE_TOTPLENGTH:
        case XLE_URL:
        case XLE_AUTOTYPE:
        case XLE_NOTES:
        case XLE_UUID:
        case XLE_CTIMEX:
        case XLE_ATIMEX:
        case XLE_XTIMEX:
        case XLE_PMTIMEX:
        case XLE_RMTIMEX:
        case XLE_XTIME_INTERVAL:
        case XLE_PWHISTORY:
        case XLE_RUNCOMMAND:
        case XLE_DCA:
        case XLE_SHIFTDCA:
        case XLE_EMAIL:
        case XLE_PROTECTED:
        case XLE_SYMBOLS:
        case XLE_KBSHORTCUT:
        case XLE_ENTRY_PASSWORDPOLICY:
        case XLE_ENTRY_PASSWORDPOLICYNAME:
          return true;
      }
      break;
    case XLE_PASSWORDPOLICYNAMES:
      switch(icurrent) {
        case XLE_POLICY:
          return true;
      }
      break;
    case XLE_EMPTYGROUPS:
      switch(icurrent) {
        case XLE_EGNAME:
          return true;
      }
      break;
    case XLE_POLICY:
      switch(icurrent) {
        case XLE_PWNAME:
        case XLE_PREF_PWDEFAULTLENGTH:
        case XLE_PREF_PWUSEDIGITS:
        case XLE_PREF_PWUSEEASYVISION:
        case XLE_PREF_PWUSEHEXDIGITS:
        case XLE_PREF_PWUSELOWERCASE:
        case XLE_PREF_PWUSESYMBOLS:
        case XLE_PREF_PWUSEUPPERCASE:
        case XLE_PREF_PWMAKEPRONOUNCEABLE:
        case XLE_PREF_PWLOWERCASEMINLENGTH:
        case XLE_PREF_PWUPPERCASEMINLENGTH:
        case XLE_PREF_PWDIGITMINLENGTH:
        case XLE_PREF_PWSYMBOLMINLENGTH:
        case XLE_SYMBOLS:
          return true;
      }
      break;
    case XLE_ENTRY_PASSWORDPOLICY:
      switch(icurrent) {
        case XLE_ENTRY_PWLENGTH:
        case XLE_ENTRY_PWUSEDIGITS:
        case XLE_ENTRY_PWUSEEASYVISION:
        case XLE_ENTRY_PWUSEHEXDIGITS:
        case XLE_ENTRY_PWUSELOWERCASE:
        case XLE_ENTRY_PWUSESYMBOLS:
        case XLE_ENTRY_PWUSEUPPERCASE:
        case XLE_ENTRY_PWMAKEPRONOUNCEABLE:
        case XLE_ENTRY_PWLOWERCASEMINLENGTH:
        case XLE_ENTRY_PWUPPERCASEMINLENGTH:
        case XLE_ENTRY_PWDIGITMINLENGTH:
        case XLE_ENTRY_PWSYMBOLMINLENGTH:
        case XLE_SYMBOLS:
          return true;
      }
      break;
    case XLE_PWHISTORY:
      switch(icurrent) {
        case XLE_STATUS:
        case XLE_MAX:
        case XLE_NUM:
        case XLE_HISTORY_ENTRIES:
          return true;
      }
      break;
    case XLE_HISTORY_ENTRIES:
      switch(icurrent) {
        case XLE_HISTORY_ENTRY:
          return true;
      }
      break;
    case XLE_HISTORY_ENTRY:
      switch(icurrent) {
        case XLE_CHANGEDX:
        case XLE_OLDPASSWORD:
          return true;
      }
      break;
  }
  return false;
}

/*!
 * Check XML elements content is according to type
 */

bool PFileXMLProcessor::CheckElementValue(const TCHAR *value, int icurrent)
{
  long lval = ((value && iswdigit(*value)) ? wcstol(value, NULL, 10) : -1L);
    
  ASSERT(value);
  switch(icurrent) {
    case XLE_PREF_LOCKDBONIDLETIMEOUT:
    case XLE_PREF_COPYPASSWORDWHENBROWSETOURL:
    case XLE_PREF_EXCLUDEFROMSCREENCAPTURE:
    case XLE_PREF_MAINTAINDATETIMESTAMPS:
    case XLE_PREF_PWMAKEPRONOUNCEABLE:
    case XLE_PREF_PWUSEDIGITS:
    case XLE_PREF_PWUSESYMBOLS:
    case XLE_PREF_PWUSEHEXDIGITS:
    case XLE_PREF_PWUSEEASYVISION:
    case XLE_PREF_PWUSELOWERCASE:
    case XLE_PREF_PWUSEUPPERCASE:
    case XLE_PREF_SAVEIMMEDIATELY:
    case XLE_PREF_SAVEPASSWORDHISTORY:
    case XLE_PREF_SHOWNOTESDEFAULT:
    case XLE_PREF_SHOWUSERNAMEINTREE:
    case XLE_PREF_SHOWPWDEFAULT:
    case XLE_PREF_SHOWPASSWORDINTREE:
    case XLE_PREF_SORTASCENDING:
    case XLE_PREF_USEDEFAULTUSER:
    case XLE_PROTECTED:
    case XLE_STATUS:
      return(SafeCompare(value, _T("1")) || SafeCompare(value, _T("true")) || SafeCompare(value, _T("0")) || SafeCompare(value, _T("false")));
          
    case XLE_PREF_IDLETIMEOUT: // 1 to 600
      return (lval >= 1 && lval <= 600);
          
    case XLE_ENTRY_PWLENGTH: // 4 to 1024
    case XLE_PREF_PWDEFAULTLENGTH: // 4 to 1024
      return (lval >= 4 && lval <= 1024);
          
    case XLE_MAX: // 0 to 255
    case XLE_NUM: // 0 to 255
    case XLE_PREF_NUMPWHISTORYDEFAULT: // 0 to 255
      return (lval >= 0 && lval <= 255);
          
    case XLE_PREF_PWDIGITMINLENGTH: // 0 to 1024
    case XLE_PREF_PWLOWERCASEMINLENGTH: // 0 to 1024
    case XLE_PREF_PWSYMBOLMINLENGTH: // 0 to 1024
    case XLE_PREF_PWUPPERCASEMINLENGTH: // 0 to 1024
      return (lval >= 0 && lval <= 1024);
          
    case XLE_PREF_TREEDISPLAYSTATUSATOPEN:
      return(SafeCompare(value, _T("AllCollapsed")) || SafeCompare(value, _T("AllExpanded")) || SafeCompare(value, _T("AsPerLastSave")));
          
    case XLE_UUID: // 32 digit in hex (16 byte)
      if(value && wcslen(value) == 32) {
        for(int i = 0; i < 32; ++i)
           if(! iswxdigit(value[i]))
             return false;
      }
      else
        return false;
      break;
          
    case XLE_CTIMEX:
    case XLE_ATIMEX:
    case XLE_XTIMEX:
    case XLE_PMTIMEX:
    case XLE_RMTIMEX:
    case XLE_CHANGEDX: // "[-]CCYY-MM-DDThh:mm:ss[Z|(+|-)hh:mm]"
      // Example: "2001-10-26T21:32:52", "2001-10-26T21:32:52+02:00", "2001-10-26T19:32:52Z", "2001-10-26T19:32:52+00:00" or "-2001-10-26T21:32:52".
      {
        if(! value)
          return false;
          
        time_t t(0);
        if(! VerifyXMLDateTimeString(value, t) || (t == time_t(-1)))
          return false;
      }
      break;
          
    case XLE_XTIME_INTERVAL: // 1 to 3650
      return (lval >= 1 && lval <= 3650);
          
    case XLE_KBSHORTCUT: // "[ACSEMWD]+:[0-9a-fA-F]{4}"
                         // Modifier:    A(lt), C(trl), S(hift), E(xt), M(eta), W(in), cm(D)
                         // Note: Meta, Win & Cmd are not supported by Windows
                         // Virtual Key: Any except: space (&\20), tab (\t), newline (\n) and return (\r)
                         // Written as 4 hex characters
      if(value && wcslen(value) == 6) {
        if(*value == L'A' || *value == L'C' || *value == L'S' || *value == L'E' || *value == L'M' || *value == L'W' || *value == L'D') {
          if(value[1] == L':') {
            for(int i = 2; i < 6; ++i)
               if(! iswxdigit(value[i]))
                 return false;
          }
          else
            return false;
        }
        else
          return false;
      }
      else
        return false;
      break;
          
    case XLE_DCA:
    case XLE_SHIFTDCA: // 0 to 9
      return (lval >= 0 && lval <= 9);
  }
  // Not checked content is string type and ok
  return true;
}

#endif /* !defined(USE_XML_LIBRARY) */
