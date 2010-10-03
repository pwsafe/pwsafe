/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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
*/

#include "XMLDefs.h"    // Required if testing "USE_XML_LIBRARY"

#ifdef USE_XML_LIBRARY

#include "XMLAttValidation.h"
#include "XMLAttHandlers.h"

// PWS includes
#include "../Util.h"
#include "../UUIDGen.h"
#include "../PWScore.h"
#include "../VerifyFormat.h"
#include "../Command.h"
#include "../corelib.h"

#include <algorithm>
#include <errno.h>

using namespace std;

void ConvertFromHex(const StringX sx_in, const size_t &out_len, unsigned char *pout_buffer)
{
    if (!sx_in.empty()) {
      // _stscanf_s always outputs to an "int" using %x even though
      // target is only 1.  Read into larger buffer to prevent data being
      // overwritten and then copy to where we want it!
      unsigned char *ptemp_array = new unsigned char [out_len + sizeof(int)];
      int nscanned = 0;
      const TCHAR *lpszuuid = sx_in.c_str();
      for (unsigned i = 0; i < out_len; i++) {
#if (_MSC_VER >= 1400)
        nscanned += _stscanf_s(lpszuuid, _T("%02x"), &ptemp_array[i]);
#else
        nscanned += _stscanf(lpszuuid, _T("%02x"), &ptemp_array[i]);
#endif
        lpszuuid += 2;
      }
      memcpy((void *)pout_buffer, (void *)ptemp_array, out_len);
      delete ptemp_array;
    } else {
      memset((void *)pout_buffer, 0, out_len);
    }
}

XMLAttHandlers::XMLAttHandlers()
{
  cur_entry = NULL;
  m_strElemContent.clear();

  m_strErrorMessage = _T("");

  m_iErrorCode = 0;

  m_bIgnoreThisAttachment = false;
  m_bErrors = false;
}

XMLAttHandlers::~XMLAttHandlers()
{
}

void XMLAttHandlers::SetVariables(PWScore *pcore, const bool &bValidation,
                                  MultiCommands *pmulticmds, PWSAttfile *pimport,
                                  CReport *prpt)
{
  m_bValidation = bValidation;
  m_pXMLcore = pcore;
  m_pmulticmds = pmulticmds;
  m_pimport = pimport;
  m_pimport3 = dynamic_cast<PWSAttfileV3 *>(m_pimport);
  m_prpt = prpt;
}

bool XMLAttHandlers::ProcessStartElement(const int icurrent_element)
{
  switch (icurrent_element) {
    case XLA_PASSWORDSAFE_ATTACHMENTS:
      m_numAttachments = m_numAttachmentsSkipped = 0;
      m_bAttachmentBeingProcessed = false;
      if (!m_bValidation) {
        // Set up map of current attachements
        m_vatt_uuid = m_pXMLcore->GetAttachmentUUIDs();
      }
      break;
    case XLA_ATTACHMENT:
      m_bAttachmentBeingProcessed = true;
      m_bIgnoreThisAttachment = false;
      m_bfirst = true;
      if (m_bValidation)
        return false;

      cur_entry = new att_entry;
      // Clear all fields
      cur_entry->atr.Clear();
      m_whichtime = -1;
      break;
    case XLA_CTIME:
    case XLA_ATIME:
    case XLA_MTIME:
    case XLA_DTIME:
      m_whichtime = icurrent_element;
      break;
    default:
      break;
  }
  return true;
}

void XMLAttHandlers::ProcessEndElement(const int icurrent_element)
{
  StringX buffer(_T(""));
  ItemListIter iter1, iter2;

  // Note: all XML elements are in the order specified in the schema.
  // In particular, we need to know {group, title, username} and associated entry uuid
  // first in order to decide if we cancel imprting this attachment.
  switch (icurrent_element) {
    case XLA_ATTACHMENT:
      if (m_bIgnoreThisAttachment) {
        stringT cs_error, cs_temp, cs_reason;
        Format(cs_temp, IDSC_IMPORTATTACHMENT, cur_entry->id,
               cur_entry->sx_group.c_str(), cur_entry->sx_title.c_str(),
               cur_entry->sx_user.c_str());
        UINT ui_msgid(0);
        if (cur_entry->err_flags & ATT_ERR_ENTRYNOTFOUND)
          ui_msgid = IDSC_IMPORT_ENTRYNOTFOUND;
        else if (cur_entry->err_flags & ATT_ERR_SIZEDIFFERENT)
          ui_msgid = IDSC_IMPORT_SIZEDIFFERENT;
        else if (cur_entry->err_flags & ATT_ERR_DIGESTFAILED)
          ui_msgid = IDSC_IMPORT_DIGESTFAILED;
        LoadAString(cs_reason, ui_msgid);
        Format(cs_error, IDSC_IMPORTATTSKIPPED, cs_temp.c_str(), cs_reason.c_str());
        m_strSkippedList += cs_error;
        m_numAttachmentsSkipped++;
      } else {
        m_ventries.push_back(cur_entry);
        m_numAttachments++;
      }
      delete cur_entry;
      cur_entry = NULL;
      break;
    case XLA_GROUP:
      cur_entry->sx_group = m_strElemContent;
      break;
    case XLA_TITLE:
      cur_entry->sx_title = m_strElemContent;
      break;
    case XLA_USER:
      cur_entry->sx_user = m_strElemContent;
      break;
    case XLA_ATTACHMENT_UUID:
      ConvertFromHex(m_strElemContent, sizeof(uuid_array_t),
                     cur_entry->atr.attmt_uuid);
      break;
    case XLA_ENTRY_UUID:
      ConvertFromHex(m_strElemContent, sizeof(uuid_array_t),
                     cur_entry->atr.entry_uuid);
      // We now should have all info to ensure we have the appropriate entry
      // in the database.
      iter1 = m_pXMLcore->Find(cur_entry->sx_group,
                               cur_entry->sx_title,
                               cur_entry->sx_user);
      iter2 = m_pXMLcore->Find(cur_entry->atr.entry_uuid);
      // Use entry found by {group, title, username} by preference!
      if (iter1 != m_pXMLcore->GetEntryEndIter())
        cur_entry->iter = iter1;
      else if(iter2 != m_pXMLcore->GetEntryEndIter())
        cur_entry->iter = iter2;
      else {
        cur_entry->err_flags |= ATT_ERR_ENTRYNOTFOUND;
        m_bIgnoreThisAttachment = true;
      }
      break;
    case XLA_FILENAME:
      cur_entry->atr.filename = m_strElemContent;
      break;
    case XLA_PATH:
      cur_entry->atr.path = m_strElemContent;
      break;
    case XLA_DESCRIPTION:
      cur_entry->atr.description = m_strElemContent;
      break;
    case XLA_BSIZE:
      cur_entry->atr.blksize = _wtoi(m_strElemContent.c_str());
      break;
    case XLA_OSIZE:
      cur_entry->atr.uncsize = _wtoi(m_strElemContent.c_str());
      break;
    case XLA_CSIZE:
      cur_entry->atr.cmpsize = _wtoi(m_strElemContent.c_str());
      break;
    case XLA_CRC:
      {
      wchar_t *p;
      StringX hex_string = StringX(_T("0x")) + m_strElemContent;
      cur_entry->atr.CRC = wcstoul(hex_string.c_str(), &p, 16);
      }
      break;
    case XLA_ODIGEST:
      ConvertFromHex(m_strElemContent, sizeof(cur_entry->atr.odigest),
                     cur_entry->atr.odigest);
      break;
    case XLA_CDIGEST:
      ConvertFromHex(m_strElemContent, sizeof(cur_entry->atr.cdigest),
                     cur_entry->atr.cdigest);
      break;
    case XLA_CTIME:
      Replace(cur_entry->sx_ctime, _T('-'), _T('/'));
      m_whichtime = -1;
      break;
    case XLA_ATIME:
      Replace(cur_entry->sx_atime, _T('-'), _T('/'));
      m_whichtime = -1;
      break;
    case XLA_MTIME:
      Replace(cur_entry->sx_mtime, _T('-'), _T('/'));
      m_whichtime = -1;
      break;
    case XLA_DTIME:
      Replace(cur_entry->sx_dtime, _T('-'), _T('/'));
      m_whichtime = -1;
      break;
    case XLA_FLAG_EXTRACTTOREMOVEABLE:
      if (m_strElemContent == _T("1"))
        cur_entry->atr.flags |= ATT_EXTRACTTOREMOVEABLE;
      else
        cur_entry->atr.flags &= ~ATT_EXTRACTTOREMOVEABLE;
      break;
    case XLA_FLAG_ERASEPROGAMEXISTS:
      if (m_strElemContent == _T("1"))
        cur_entry->atr.flags |= ATT_ERASEPGMEXISTS;
      else
        cur_entry->atr.flags &= ~ATT_ERASEPGMEXISTS;
      break;
    case XLA_FLAG_ERASEONDATABASECLOSE:
      if (m_strElemContent == _T("1"))
        cur_entry->atr.flags |= ATT_ERASEONDBCLOSE;
      else
        cur_entry->atr.flags &= ~ATT_ERASEONDBCLOSE;
      break;
    case XLA_DATE:
      switch (m_whichtime) {
        case XLA_CTIME:
          cur_entry->sx_ctime = m_strElemContent;
          break;
        case XLA_ATIME:
          cur_entry->sx_atime = m_strElemContent;
          break;
        case XLA_MTIME:
          cur_entry->sx_mtime = m_strElemContent;
          break;
        case XLA_DTIME:
          cur_entry->sx_dtime = m_strElemContent;
          break;
        default:
          ASSERT(0);
      }
      break;
    case XLA_TIME:
      switch (m_whichtime) {
        case XLA_CTIME:
          cur_entry->sx_ctime += _T(" ") + m_strElemContent;
          break;
        case XLA_ATIME:
          cur_entry->sx_atime += _T(" ") + m_strElemContent;
          break;
        case XLA_MTIME:
          cur_entry->sx_mtime += _T(" ") + m_strElemContent;
          break;
        case XLA_DTIME:
          cur_entry->sx_dtime += _T(" ") + m_strElemContent;
          break;
        default:
          ASSERT(0);
      }
      break;
    case XLA_DATA80:
    {
      ATTProgress st_atpg;
      if (m_bfirst) {
        ValidateImportData(cur_entry);

        m_pimport3->WriteAttmntRecordPreData(cur_entry->atr);

        st_atpg.function = ATT_PROGRESS_PROCESSFILE;
        st_atpg.atr = cur_entry->atr;
        st_atpg.value = 0;
        m_pXMLcore->AttachmentProgress(st_atpg);
        m_bfirst = false;
      }

      Replace(m_strElemContent, StringX(_T("\n")), StringX(_T("")));
      Replace(m_strElemContent, StringX(_T("\r")), StringX(_T("")));
      Replace(m_strElemContent, StringX(_T("\t")), StringX(_T("")));
      Replace(m_strElemContent, StringX(_T(" ")), StringX(_T("")));
      size_t out_len = (m_strElemContent.length() / 3) * 4 + 4;

      unsigned char *pData = new unsigned char[out_len];
      PWSUtil::Base64Decode(m_strElemContent, pData, out_len);
      cur_entry->datalength += out_len;

      cur_entry->context.Update(pData, out_len);
      m_pimport3->WriteAttmntRecordData(pData, out_len, PWSAttfile::ATTMT_DATA);

      trashMemory(pData, out_len);
      delete [] pData;

      st_atpg.function = ATT_PROGRESS_PROCESSFILE;
      st_atpg.value = (int)((cur_entry->datalength * 1.0E02) / cur_entry->atr.uncsize);
      m_pXMLcore->AttachmentProgress(st_atpg);

      delete [] pData;
      break;
    }
    case XLA_DATA81:
    {
      ATTProgress st_atpg;
      if (m_bfirst) {
        ValidateImportData(cur_entry);

        m_pimport3->WriteAttmntRecordPreData(cur_entry->atr);

        st_atpg.function = ATT_PROGRESS_PROCESSFILE;
        st_atpg.value = 0;
        m_pXMLcore->AttachmentProgress(st_atpg);
        m_bfirst = false;
      }

      Replace(m_strElemContent, StringX(_T("\n")), StringX(_T("")));
      Replace(m_strElemContent, StringX(_T("\r")), StringX(_T("")));
      Replace(m_strElemContent, StringX(_T("\t")), StringX(_T("")));
      Replace(m_strElemContent, StringX(_T(" ")), StringX(_T("")));
      size_t out_len = (m_strElemContent.length() / 3) * 4 + 4;

      unsigned char *pData = new unsigned char[out_len];
      PWSUtil::Base64Decode(m_strElemContent, pData, out_len);
      cur_entry->datalength += out_len;

      ASSERT(cur_entry->atr.cmpsize == cur_entry->datalength);

      unsigned char cdigest[SHA1::HASHLEN];
      cur_entry->context.Update(pData, out_len);
      cur_entry->context.Final(cdigest);
      m_pimport3->WriteAttmntRecordData(pData, out_len, PWSAttfile::ATTMT_LASTDATA);

      m_pimport3->WriteAttmntRecordPostData(cur_entry->atr);

      trashMemory(pData, out_len);
      delete [] pData;

      st_atpg.function = ATT_PROGRESS_PROCESSFILE;
      st_atpg.value = 100;
      m_pXMLcore->AttachmentProgress(st_atpg);

      if (cur_entry->atr.cmpsize != cur_entry->datalength) {
        cur_entry->err_flags |= ATT_ERR_SIZEDIFFERENT;
        m_bIgnoreThisAttachment = true;
        break;
      }
      if (memcmp(cdigest, cur_entry->atr.cdigest, SHA1::HASHLEN) != 0) {
        cur_entry->err_flags |= ATT_ERR_DIGESTFAILED;
        m_bIgnoreThisAttachment = true;
        break;
      }
      break;
    }
    case XLA_PASSWORDSAFE_ATTACHMENTS:
      m_pimport3->Close();
      delete m_pimport3;
      m_pimport3 =  NULL;
      m_pimport = NULL;
      break;
    default:
      break;
  }
}

void XMLAttHandlers::ValidateImportData(att_entry * &cur_entry)
{
  // Ensure attachment UUID is unique
  st_UUID st(cur_entry->atr.attmt_uuid);
  if (std::find(m_vatt_uuid.begin(), m_vatt_uuid.end(), st) !=
                m_vatt_uuid.end()) {
    // Attachment uuid not unique - get a new one
    CUUIDGen auuid;
    auuid.GetUUID(cur_entry->atr.attmt_uuid);
  }

  // Ensure dates are correct
  if (!VerifyImportDateTimeString(cur_entry->sx_ctime.c_str(), cur_entry->atr.ctime))
    cur_entry->atr.ctime = (time_t)0;
  if (!VerifyImportDateTimeString(cur_entry->sx_atime.c_str(), cur_entry->atr.atime))
    cur_entry->atr.atime = (time_t)0;
  if (!VerifyImportDateTimeString(cur_entry->sx_mtime.c_str(), cur_entry->atr.mtime))
    cur_entry->atr.mtime = (time_t)0;

  // We really don't care about added time from XML
  time_t dtime;
  time(&dtime);
  cur_entry->atr.dtime = dtime;

  // Ensure attachment is unique
  ATRVector vATRecords;
  size_t num_atts = m_pXMLcore->GetAttachments(cur_entry->atr.entry_uuid,
                                               vATRecords);
  if (num_atts > 0) {
    for (ATRViter iter = vATRecords.begin(); iter != vATRecords.end(); iter++) {
      if (iter->path == cur_entry->atr.path &&
          iter->filename == cur_entry->atr.filename &&
          iter->description == cur_entry->atr.description) {
        StringX str;
        time_t time_now;
        time(&time_now);
        const StringX sx_now = PWSUtil::ConvertToDateTimeString(time_now, TMC_LOCALE);
        Format(str, IDSC_IMPORTTIMESTAMP, sx_now.c_str());
        cur_entry->atr.description += str;
        break;
     }
   }
 }
}

#endif /* USE_XML_LIBRARY */
