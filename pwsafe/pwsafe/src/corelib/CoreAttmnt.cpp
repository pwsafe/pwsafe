/*
/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// file CoreAttmnt.cpp
//-----------------------------------------------------------------------------

#include "PWScore.h"
#include "corelib.h"
#include "Util.h"
#include "UUIDGen.h"
#include "SysInfo.h"
#include "UTF8Conv.h"
#include "Report.h"
#include "Match.h"
#include "PWSAttfileV3.h" // XXX cleanup with dynamic_cast
#include "return_codes.h"

#include "XML/XMLDefs.h"  // Required if testing "USE_XML_LIBRARY"

#if USE_XML_LIBRARY == EXPAT
#include "XML/Expat/EAtteXMLProcessor.h"
#elif USE_XML_LIBRARY == MSXML
#include "XML/MSXML/MAttXMLProcessor.h"
#elif USE_XML_LIBRARY == XERCES
#include "XML/Xerces/XAttXMLProcessor.h"
#endif

#include "os/typedefs.h"
#include "os/dir.h"
#include "os/file.h"

#include "zlib/zlib.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <set>

#include <errno.h>

using namespace std;

// hide w_char/char differences where possible:
#ifdef UNICODE
typedef std::wifstream ifstreamT;
typedef std::wofstream ofstreamT;
#else
typedef std::ifstream ifstreamT;
typedef
#endif

// Formula to calculate blocksizes for data processing:
//   min(max(MINBLOCKSIZE, filesize/50), MAXBLOCKSIZE)
//      where MINBLOCKSIZE is 32KB and MAXBLOCKSIZE is 256KB.
// Then to nearest 4KB boundary (2^12)

#define MINBLOCKSIZE  32768
#define MAXBLOCKSIZE 262144

#define GetBlocksize(n) ((min(max(MINBLOCKSIZE, n / 50), MAXBLOCKSIZE) >> 12) << 12)

// Return whether uuid elem1 is less than uuid elem2
// Used in set_difference between 2 sets
bool uuid_lesser(st_UUID elem1, st_UUID elem2)
{
  return memcmp(elem1.uuid, elem2.uuid, sizeof(uuid_array_t)) < 0;
}

// Return whether mulitmap pair uuid is less than the other uuid
// Used in set_difference between 2 multimaps
bool mp_uuid_lesser(ItemMap_Pair p1, ItemMap_Pair p2)
{
  uuid_array_t uuid1, uuid2;
  p1.first.GetUUID(uuid1);
  p2.first.GetUUID(uuid2);
  int icomp = memcmp(uuid1, uuid2, sizeof(uuid_array_t));
  if (icomp == 0) {
    p1.second.GetUUID(uuid1);
    p2.second.GetUUID(uuid2);
    return memcmp(uuid1, uuid2, sizeof(uuid_array_t)) < 0;
  } else
    return icomp < 0;
}

struct GetATR {
  GetATR(const st_UUID &st_uuid)
  {
    memcpy(m_attmt_uuid, st_uuid.uuid, sizeof(uuid_array_t));
  }

  bool operator()(pair<st_UUID, ATRecord> p)
  {
    return (memcmp(p.second.attmt_uuid, m_attmt_uuid, sizeof(uuid_array_t)) == 0);
  }

private:
  uuid_array_t m_attmt_uuid;
};

// Internal routines to compress in memory blocks

int PWS_Deflate_Init(z_stream &strm, int level = Z_DEFAULT_COMPRESSION)
{
  // Allocate deflate state
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.total_out = 0;
  return deflateInit(&strm, level);
}

int PWS_Deflate_Buffer(z_stream &strm,
                       const unsigned char *in_buffer, const size_t in_size,
                       unsigned char *out_buffer, size_t &out_size,
                       const bool bLast)
{
  // Deflate block
  strm.avail_in = in_size;
  strm.next_in = (Bytef *)in_buffer;
  strm.avail_out = out_size;
  strm.next_out = out_buffer;

  int ret = deflate(&strm, bLast ? Z_FINISH : Z_BLOCK); /* no bad return value */
  ASSERT(ret != Z_STREAM_ERROR);     /* state not clobbered */

  ASSERT(strm.avail_in == 0);        /* all input will be used */

  if (bLast)
    ASSERT(ret == Z_STREAM_END);     /* stream will be complete */

  // Return size used in output buffer
  out_size -= strm.avail_out;
  return ret;
}

void PWS_Deflate_Term(z_stream &strm)
{
  // Clean up and return
  deflateEnd(&strm);
}

// Internal routines to uncompress memory blocks

int PWS_Inflate_Init(z_stream &strm)
{
  // Allocate inflate state
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  return inflateInit(&strm);
}

int PWS_Inflate_Buffer(z_stream &strm,
                       const unsigned char *in_buffer, const size_t in_size,
                       unsigned char *out_buffer, size_t &out_size,
                       const bool bLast)
{
  // Inflate a block
  strm.avail_in = in_size;
  strm.next_in = (Bytef *)in_buffer;
  strm.avail_out = out_size;
  strm.next_out = (Bytef *)out_buffer;

  int ret = inflate(&strm, Z_NO_FLUSH);

  ASSERT(ret != Z_STREAM_ERROR);  /* state not clobbered */
  switch (ret) {
    case Z_NEED_DICT:
      ret = Z_DATA_ERROR;         /* and fall through */
    case Z_DATA_ERROR:
    case Z_BUF_ERROR:
    case Z_MEM_ERROR:
      inflateEnd(&strm);
      return ret;
  }

  // Update size of uncompressed data produced
  out_size -= strm.avail_out;

  if (bLast)
    ret = (ret == Z_STREAM_END) ? Z_OK : Z_DATA_ERROR;

  return ret;
}

void PWS_Inflate_Term(z_stream &strm)
{
  // Clean up and return
  inflateEnd(&strm);
}

int PWScore::ReadAttachmentFile(bool bVerify)
{
  // Parameter must not be constant as user may cancel verification
  int status;
  stringT attmt_file;
  m_MM_entry_uuid_atr.clear();

  if (m_currfile.empty()) {
    return PWSRC::CANT_OPEN_FILE;
  }

  // Generate attachment file names from database name
  stringT drv, dir, name, ext;

  pws_os::splitpath(m_currfile.c_str(), drv, dir, name, ext);
  attmt_file = drv + dir + name + stringT(ATT_DEFAULT_ATTMT_SUFFIX);

  // If attachment file doesn't exist - OK
  if (!pws_os::FileExists(attmt_file)) {
    pws_os::Trace(_T("No attachment file exists.\n"));
    return PWSRC::SUCCESS;
  }

  PWSAttfile::VERSION version;
  version = PWSAttfile::V30;

  // 'Make' file
  PWSAttfile *in = PWSAttfile::MakePWSfile(attmt_file.c_str(), version,
                                           PWSAttfile::Read, status,
                                           m_pAsker, m_pReporter);

  if (status != PWSRC::SUCCESS) {
    delete in;
    return status;
  }

  // Open file
  status = in->Open(GetPassKey());
  if (status != PWSRC::SUCCESS) {
    pws_os::Trace(_T("Open attachment file failed RC=%d\n"), status);
    delete in;
    return PWSRC::CANT_OPEN_FILE;
  }

  PWSAttfile::AttHeaderRecord ahr;
  ahr = in->GetHeader();

  if (memcmp(ahr.DBfile_uuid, m_hdr.m_file_uuid_array, sizeof(uuid_array_t)) != 0) {
    pws_os::Trace(_T("Attachment header - database UUID inconsistent.\n"));
    in->Close();
    delete in;
    return PWSRC::HEADERS_INVALID;
  }

  // Now open file and build sets & maps and ensure that they
  // are fully consistent
  // Format of these set/map names are:
  //
  //  tt_vvvvvv_uuid, where
  //    tt     = type: st for set, mp for map
  //    vvvvvv = field value:
  //             'attmt' = UUID of attachment record,
  //             'entry' = UUID of associated entry that has this attachment

  UUIDSetPair pr;               // Used to confirm uniqueness during insert into std::set

  // From file
  UUIDSet st_attmt_uuid;        // std::set on attachment uuid
  ItemMap mp_entry_uuid;        // std::map key = st_attmt_uuid, value = entry_uuid

  // Also from attachment records
  UUIDATRMMap mm_entry_uuid_atr; // std::multimap key = entry_uuid, value = attachment record

  ATTProgress st_atpg;

  st_atpg.function = ATT_PROGRESS_START;
  LoadAString(st_atpg.function_text, IDSC_ATT_READVERIFY);
  st_atpg.value = bVerify ? -1 : 0;  // If in verify mode - allow user to stop verification
  AttachmentProgress(st_atpg);
  st_atpg.function_text.clear();

  bool go(true), bCancel(false);
  do {
    ATRecord atr;
    bool bError(false);

    // Read pre-data information
    status = in->ReadAttmntRecordPreData(atr);

    st_atpg.function = ATT_PROGRESS_PROCESSFILE;
    st_atpg.atr = atr;
    st_atpg.value = 0;
    AttachmentProgress(st_atpg);

    switch (status) {
      case PWSRC::FAILURE:
      {
        // Show a useful (?) error message - better than
        // silently losing data (but not by much)
        // Best if title intact. What to do if not?
        if (m_pReporter != NULL) {
          stringT cs_msg, cs_caption;
          LoadAString(cs_caption, IDSC_READ_ERROR);
          Format(cs_msg, IDSC_ENCODING_PROBLEM, (atr.path + atr.filename).c_str());
          cs_msg = cs_caption + _S(": ") + cs_caption;
          (*m_pReporter)(cs_msg);
        }
        break;
      }
      case PWSRC::SUCCESS:
      {
        pr = st_attmt_uuid.insert(atr.attmt_uuid);
        if (!pr.second) {
          bError = true;
          break;
        }

        unsigned char readtype;
        unsigned int cmpsize, count;
        unsigned char cdigest[SHA1::HASHLEN];

        // Get SHA1 hash and CRC of compressed data - only need one but..
        // display CRC and user can check with, say, WinZip to see if the same
        SHA1 ccontext;
        cmpsize = 0;
        count = 0;

        // Read all data
        do {
          unsigned int uiCmpLen;
          unsigned char *pCmpData(NULL);

          status = in->ReadAttmntRecordData(pCmpData, uiCmpLen, readtype, !bVerify);
          cmpsize += uiCmpLen;

          // Should be "atr.cmpsize" but we don't know it yet
          count += atr.blksize;
          st_atpg.function = ATT_PROGRESS_PROCESSFILE;
          st_atpg.value = (int)((count * 1.0E02) / atr.uncsize);
          // Update progress dialog and check if user cancelled verification
          int rc = AttachmentProgress(st_atpg);
          if (rc == 1) {
            // Cancel verification
            bVerify = false;
            // Update progress dialog window text
            LoadAString(st_atpg.function_text, IDSC_ATT_READFILE);
            st_atpg.value = -1;  // Get Window text updated but not progress bar
            AttachmentProgress(st_atpg);
            st_atpg.function_text.clear();
          } else
          if (rc == 2) {
            // Cancel reading attachment file
            bCancel = true;
            go = false;
          }

          if (bVerify) {
            ccontext.Update(pCmpData, uiCmpLen);
          }
          if (pCmpData != 0 && uiCmpLen > 0) {
             trashMemory(pCmpData, uiCmpLen);
             delete [] pCmpData;
             pCmpData = NULL;
          }
         } while (status == PWSRC::SUCCESS && readtype != PWSAttfileV3::ATTMT_LASTDATA);

        // Read post-data
        status = in->ReadAttmntRecordPostData(atr);
        if (bVerify) {
          ccontext.Final(cdigest);
          if (atr.cmpsize != cmpsize ||
              memcmp(atr.cdigest, cdigest, SHA1::HASHLEN) != 0) {
            ASSERT(0);
          }
        }

        st_atpg.function = ATT_PROGRESS_PROCESSFILE;
        st_atpg.value = 100;
        AttachmentProgress(st_atpg);

        mp_entry_uuid.insert(ItemMMap_Pair(atr.entry_uuid, atr.attmt_uuid));
        st_UUID stuuid(atr.entry_uuid);
        mm_entry_uuid_atr.insert(make_pair(stuuid, atr));
        break;
      }
      case PWSRC::END_OF_FILE:
        go = false;
        break;
    } // switch
    if (bError) {
      pws_os::Trace(_T("Duplicate entries found in attachment file and have been ignored.\n"));
    }
  } while (go);

  int closeStatus = in->Close();
  if (bVerify && closeStatus != PWSRC::SUCCESS)
    ASSERT(0);

  delete in;

  if (bCancel) {
    mm_entry_uuid_atr.clear();
    ahr.Clear();
  }

  // All OK - update the entries in PWScore
  m_atthdr = ahr;
  m_MM_entry_uuid_atr = mm_entry_uuid_atr;

  // Terminate thread
  st_atpg.function = ATT_PROGRESS_END;
  AttachmentProgress(st_atpg);

  return bCancel ? PWSRC::USER_CANCEL : PWSRC::SUCCESS;
}

void PWScore::AddAttachment(const ATRecord &atr)
{
  // Add attachment record using the DB entry UUID as key
  st_UUID stuuid(atr.entry_uuid);
  m_MM_entry_uuid_atr.insert(make_pair(stuuid, atr));
}

void PWScore::AddAttachments(ATRVector &vNewATRecords)
{
  if (vNewATRecords.empty())
    return;

  // Add attachment record using the DB entry UUID as key
  st_UUID stuuid(vNewATRecords[0].entry_uuid);

  for (size_t i = 0; i < vNewATRecords.size(); i++) {
    m_MM_entry_uuid_atr.insert(make_pair(stuuid, vNewATRecords[i]));
  }
}

void PWScore::ChangeAttachment(const ATRecord &atr)
{
  // First delete old one
  // Find current entry by getting subset for this DB entry
  std::pair<UAMMiter, UAMMiter> uuidairpair;
  uuidairpair = m_MM_entry_uuid_atr.equal_range(atr.entry_uuid);

  // Now find this specific attachment record
  for (UAMMiter iter = uuidairpair.first; iter != uuidairpair.second; ++iter) {
    if (memcmp(iter->second.attmt_uuid, atr.attmt_uuid, sizeof(uuid_array_t)) == 0) {
      m_MM_entry_uuid_atr.erase(iter);
      break;
    }
  }

  // Put back in the changed one
  st_UUID stuuid(atr.entry_uuid);
  m_MM_entry_uuid_atr.insert(make_pair(stuuid, atr));
}

bool PWScore::MarkAttachmentForDeletion(const ATRecord &atr)
{
  bool bRC(false);
  // Find current entry by getting subset for this DB entry
  std::pair<UAMMiter, UAMMiter> uuidairpair;
  uuidairpair = m_MM_entry_uuid_atr.equal_range(atr.entry_uuid);

  // Now find this specific attachment record
  for (UAMMiter iter = uuidairpair.first; iter != uuidairpair.second; ++iter) {
    if (memcmp(iter->second.attmt_uuid, atr.attmt_uuid, sizeof(uuid_array_t)) == 0) {
      iter->second.uiflags |= (ATT_ATTACHMENT_FLGCHGD | ATT_ATTACHMENT_DELETED);
      bRC = true;
      break;
    }
  }
  return bRC;
}

bool PWScore::UnMarkAttachmentForDeletion(const ATRecord &atr)
{
  bool bRC(false);
  // Find current entry by getting subset for this DB entry
  std::pair<UAMMiter, UAMMiter> uuidairpair;
  uuidairpair = m_MM_entry_uuid_atr.equal_range(atr.entry_uuid);

  // Now find this specific attachment record
  for (UAMMiter iter = uuidairpair.first; iter != uuidairpair.second; ++iter) {
    if (memcmp(iter->second.attmt_uuid, atr.attmt_uuid, sizeof(uuid_array_t)) == 0) {
      iter->second.uiflags &= ~(ATT_ATTACHMENT_FLGCHGD | ATT_ATTACHMENT_DELETED);
      bRC = true;
      break;
    }
  }
  return bRC;
}

void PWScore::MarkAllAttachmentsForDeletion(const uuid_array_t &entry_uuid)
{
  // Mark all attachment records for this database entry for deletion
  std::pair<UAMMiter, UAMMiter> uuidairpair;
  uuidairpair = m_MM_entry_uuid_atr.equal_range(entry_uuid);

  // Now update all attachment records
  for (UAMMiter iter = uuidairpair.first; iter != uuidairpair.second; ++iter) {
    iter->second.uiflags |= (ATT_ATTACHMENT_FLGCHGD | ATT_ATTACHMENT_DELETED);
  }
}

void PWScore::UnMarkAllAttachmentsForDeletion(const uuid_array_t &entry_uuid)
{
  // UnMark all attachment records for this database entry for deletion
  std::pair<UAMMiter, UAMMiter> uuidairpair;
  uuidairpair = m_MM_entry_uuid_atr.equal_range(entry_uuid);

  // Now update all attachment records
  for (UAMMiter iter = uuidairpair.first; iter != uuidairpair.second; ++iter) {
    iter->second.uiflags &= ~(ATT_ATTACHMENT_FLGCHGD | ATT_ATTACHMENT_DELETED);
  }
}

size_t PWScore::HasAttachments(const uuid_array_t &entry_uuid)
{
  st_UUID stuuid(entry_uuid);
  return m_MM_entry_uuid_atr.count(entry_uuid);
}

int PWScore::GetAttachment(const ATRecord &in_atr, const int ifunction,
                           unsigned char * &pUncData, size_t &UncLength,
                           unsigned char &readtype)
{
  /*
     ifunction = OPENFILE, GETPRE, GETDATA, GETPOST, CLOSEFILE
       OPENFILE  - open attachment file
       GETPRE    - search for requested attachment
       GETDATA   - get attachment data
       GETPOST   - get post data e.g. CRC, odigest and cdigest
       CLOSEFILE - close attachment file

     During GETDATA processing, pUncData must be NULL and UncLength must be zero.
     If successful, these will be updated to point to the uncompressed original data
     from the attachment and the length.
     If the attachment data is passed back, the caller is responsible for freeing the
     buffer!
  */

  static PWSAttfile *in(NULL);
  static PWSAttfile::AttHeaderRecord ahr;
  static z_stream strm;
  static ATRecord atr;
  static unsigned int cmpsize;
  ATTProgress st_atpg;
  int status;

  if (ifunction == OPENFILE) {
    stringT attmnt_file;
    stringT drv, dir, name, ext;
    StringX sxFilename;

    pws_os::splitpath(m_currfile.c_str(), drv, dir, name, ext);
    ext = stringT(ATT_DEFAULT_ATTMT_SUFFIX);
    attmnt_file = drv + dir + name + ext;
    sxFilename = attmnt_file.c_str();

    PWSAttfile::VERSION version = PWSAttfile::V30;

    // 'Make' the data file
    in = PWSAttfile::MakePWSfile(sxFilename, version,
                                 PWSAttfile::Read, status,
                                 m_pAsker, m_pReporter);

    if (status != PWSRC::SUCCESS) {
      delete in;
      in = NULL;
      return status;
    }

    // Open the data file
    status = in->Open(GetPassKey());
    if (status != PWSRC::SUCCESS) {
      delete in;
      in = NULL;
      return PWSRC::CANT_OPEN_FILE;
    }

    st_atpg.function = ATT_PROGRESS_START;
    LoadAString(st_atpg.function_text, IDSC_ATT_SEARCHFILE);
    st_atpg.value = 0;
    AttachmentProgress(st_atpg);
    st_atpg.function_text.clear();

    st_atpg.function = ATT_PROGRESS_SEARCHFILE;
    st_atpg.value = 0;
    AttachmentProgress(st_atpg);

    // Get the header (not sure why?)
    ahr = in->GetHeader();

    // Set up uncompress environment
    PWS_Inflate_Init(strm);

    return PWSRC::SUCCESS;
  }

  if (ifunction == GETPRE) {
    // Search for our record
    do {
      cmpsize = 0;
      status = in->ReadAttmntRecordPreData(atr);
      if (status != PWSRC::SUCCESS)
        return status;

      // If ours - return so we can be called again for the data
      if (memcmp(atr.attmt_uuid, in_atr.attmt_uuid, sizeof(uuid_array_t)) == 0) {
        cmpsize = 0;
        return PWSRC::SUCCESS;
      }

      // Not ours - read all 'unwanted' data
      unsigned int uiCmpLen;
      unsigned char *pUnwantedData(NULL);
      do {
        status = in->ReadAttmntRecordData(pUnwantedData, uiCmpLen, readtype, true);
        cmpsize += uiCmpLen;

        st_atpg.function = ATT_PROGRESS_SEARCHFILE;
        st_atpg.value = (int)((cmpsize * 1.0E02) / atr.cmpsize);
        AttachmentProgress(st_atpg);
      } while (status == PWSRC::SUCCESS && readtype != PWSAttfileV3::ATTMT_LASTDATA);

      // Read post-data for this attachment
      status = in->ReadAttmntRecordPostData(atr);
    } while (status == PWSRC::SUCCESS);

    // If we got here - we didn't find the attachment!
    return PWSRC::FAILURE;
  }

  if (ifunction == GETDATA) {
    // Got our record - now just give them the data
    if (cmpsize == 0) {
      st_atpg.function = ATT_PROGRESS_START;
      LoadAString(st_atpg.function_text, IDSC_ATT_EXTRACTINGFILE);
      st_atpg.value = 0;
      AttachmentProgress(st_atpg);
      st_atpg.function_text.clear();
    }

    unsigned char *pCmpData(NULL);
    unsigned int uiCmpLen;
    status = in->ReadAttmntRecordData(pCmpData, uiCmpLen, readtype, false);

    if (status != PWSRC::SUCCESS) {
      trashMemory(pCmpData, uiCmpLen);
      delete [] pCmpData;
      pCmpData = NULL;
      return status;
    }

    cmpsize += uiCmpLen;
    st_atpg.function = ATT_PROGRESS_EXTRACTFILE;
    st_atpg.value = (int)((cmpsize * 1.0E02) / atr.cmpsize);
    AttachmentProgress(st_atpg);

    // Save the data pointer and length
    pUncData = new unsigned char[atr.blksize + 1];
    unsigned int uiUncLength = atr.blksize + 1;
    int zRC;
    zRC = PWS_Inflate_Buffer(strm, pCmpData, uiCmpLen,
                             pUncData, uiUncLength,
                             readtype == PWSAttfileV3::ATTMT_LASTDATA);

    if (zRC != Z_OK) {
      // Error message to the user?
      trashMemory(pCmpData, uiCmpLen);
      delete [] pCmpData;
      pCmpData = NULL;
      trashMemory(pUncData, uiUncLength);
      delete [] pUncData;
      pUncData = NULL;
      return PWSRC::FAILURE;
    }

    // Delete the compressed data buffer
    trashMemory(pCmpData, uiCmpLen);
    delete [] pCmpData;
    pCmpData = NULL;

    // Update with actual length
    UncLength = uiUncLength;
    return status;
  }

  if (ifunction == GETPOST) {
    // Caller has the data - get the post-data info
    status = in->ReadAttmntRecordPostData(atr);

    st_atpg.function = ATT_PROGRESS_EXTRACTFILE;
    st_atpg.value = 100;
    AttachmentProgress(st_atpg);

    return status;
  }

  if (ifunction == CLOSEFILE) {
    st_atpg.function = ATT_PROGRESS_END;
    AttachmentProgress(st_atpg);

    ahr.Clear();
    atr.Clear();

    // Close the data file
    in->Close();
    delete in;
    in = NULL;

    // Tidy up uncompress environment
    PWS_Inflate_Term(strm);

    st_atpg.function = ATT_PROGRESS_END;
    AttachmentProgress(st_atpg);
    return PWSRC::SUCCESS;
  }
  ASSERT(0);

  st_atpg.function = ATT_PROGRESS_END;
  AttachmentProgress(st_atpg);
  return PWSRC::FAILURE;
}

size_t PWScore::GetAttachments(const uuid_array_t &entry_uuid,
                               ATRVector &vATRecords)
{
  std::pair<UAMMciter, UAMMciter> uuidairpair;
  vATRecords.clear();

  st_UUID stuuid(entry_uuid);
  size_t num = m_MM_entry_uuid_atr.count(stuuid);
  uuidairpair = m_MM_entry_uuid_atr.equal_range(stuuid);
  if (uuidairpair.first != m_MM_entry_uuid_atr.end()) {
    for (UAMMciter citer = uuidairpair.first; citer != uuidairpair.second; ++citer) {
      if ((citer->second.uiflags & ATT_ATTACHMENT_DELETED) != ATT_ATTACHMENT_DELETED)
        vATRecords.push_back(citer->second);
    }
  }
  return num;
}

size_t PWScore::GetAllAttachments(ATRExVector &vATRecordExs)
{
  // Used by View All Attachments
  vATRecordExs.clear();
  ATRecordEx atrex;
  size_t num = 0;

  for (UAMMciter citer = m_MM_entry_uuid_atr.begin(); citer != m_MM_entry_uuid_atr.end();
       ++citer) {
    if ((citer->second.uiflags & ATT_ATTACHMENT_DELETED) != ATT_ATTACHMENT_DELETED) {
      atrex.Clear();
      atrex.atr = citer->second;
      ItemListIter entry_iter = Find(citer->second.entry_uuid);
      if (entry_iter != GetEntryEndIter()) {
        atrex.sxGroup = entry_iter->second.GetGroup();
        atrex.sxTitle = entry_iter->second.GetTitle();
        atrex.sxUser = entry_iter->second.GetUser();
      } else {
        atrex.sxGroup = _T("?");
        atrex.sxTitle = _T("?");
        atrex.sxUser = _T("?");
      }
      vATRecordExs.push_back(atrex);
      num++;
    }
  }

  return num;
}

void PWScore::SetAttachments(const uuid_array_t &entry_uuid,
                             ATRVector &vATRecords)
{
  // Delete any existing
  st_UUID stuuid(entry_uuid);
  m_MM_entry_uuid_atr.erase(stuuid);

  // Put back supplied versions
  for (size_t i = 0; i < vATRecords.size(); i++) {
    st_UUID stuuid(entry_uuid);
    m_MM_entry_uuid_atr.insert(make_pair(stuuid, vATRecords[i]));
  }
}

/**
 * There is a lot in common between the member functions:
 *   WriteAttachmentFile & DuplicateAttachments
 *
 * There needs to be some splitting out of common functions
 */

int PWScore::WriteAttachmentFile(const bool bCleanup, PWSAttfile::VERSION version)
{
  int status;
  /*
    Generate a temporary file name - write to this file.  If OK, rename current
    file and then rename the temporary file to the proper name.
    This does leave the older file around as backup (for the moment).
  */

  // First check if anything to do!
  // 1. Yes - if there are new entries not yet added to attachment file
  // 2. Yes - if there are existing entries with changed flags
  // 3. Yes - if there are existing entries that are no longer required or missing
  //   otherwise No!

  bool bContinue(false);
  for (UAMMciter citer = m_MM_entry_uuid_atr.begin(); citer != m_MM_entry_uuid_atr.end();
       citer++) {
    ATRecord atr = citer->second;
    bool bDelete = bCleanup && ((atr.uiflags & ATT_ATTACHMENT_DELETED) != 0);
    bool bChanged = (atr.uiflags & ATT_ATTACHMENT_FLGCHGD) == ATT_ATTACHMENT_FLGCHGD;

    if ((!bDelete && atr.uncsize != 0) ||
        (bCleanup && bChanged)) {
      bContinue = true;
      break;
    }
  }

  if (!bContinue)
    return PWSRC::SUCCESS;

  PWSAttfile *in(NULL), *out(NULL);
  stringT tempfilename, current_file, timestamp;
  stringT sDrive, sDir, sName, sExt;
  time_t time_now;

  // Generate temporary name
  time(&time_now);
  timestamp = PWSUtil::ConvertToDateTimeString(time_now, TMC_FILE).c_str();
  pws_os::splitpath(m_currfile.c_str(), sDrive, sDir, sName, sExt);

  tempfilename = sDrive + sDir + sName + timestamp +
                 stringT(ATT_DEFAULT_ATTMT_SUFFIX);
  current_file = sDrive + sDir + sName +
                 stringT(ATT_DEFAULT_ATTMT_SUFFIX);

  // 'Make' new file
  out = PWSAttfile::MakePWSfile(tempfilename.c_str(), version,
                                PWSAttfile::Write, status,
                                m_pAsker, m_pReporter);
  if (status != PWSRC::SUCCESS) {
    delete out;
    return status;
  }

  // If there is an existing attachment file, 'make' old file
  if (pws_os::FileExists(current_file)) {
    in = PWSAttfile::MakePWSfile(current_file.c_str(), version,
                                 PWSAttfile::Read, status,
                                 m_pAsker, m_pReporter);

    if (status != PWSRC::SUCCESS) {
      delete out;
      return status;
    }
  }

  // XXX cleanup gross dynamic_cast
  PWSAttfileV3 *out3 = dynamic_cast<PWSAttfileV3 *>(out);

  // Set up header records
  SetupAttachmentHeader();

  // Set them - will be written during Open below
  out3->SetHeader(m_atthdr);

  ATRVector vATRWritten;
  ATTProgress st_atpg;

  try { // exception thrown on write error
    // Open new attachment file
    status = out3->Open(GetPassKey());

    if (status != PWSRC::SUCCESS) {
      delete out3;
      delete in;
      return status;
    }

    // If present, open current data file
    if (in != NULL) {
      status = in->Open(GetPassKey());

      if (status != PWSRC::SUCCESS) {
        out3->Close();
        delete out3;
        delete in;
        return status;
      }
    }

    st_atpg.function = ATT_PROGRESS_START;
    LoadAString(st_atpg.function_text, IDSC_ATT_COPYFILE);
    st_atpg.value = 0;
    AttachmentProgress(st_atpg);
    st_atpg.function_text.clear();

    std::pair<UAMMciter, UAMMciter> uuidairpair;

    // Time stamp for date added for new attachments
    time_t dtime;
    time(&dtime);

    ATRecord atr;
    UUIDSet st_attmt_uuid;         // std::set from attachment records on attmt_uuid
    UUIDATRMap mp_attmt_uuid_atr;  // std::map key = attmt_uuid, value = attachment record

    // Find all existing attachments we want to keep (atr.cmpsize is filled in)
    for (UAMMiter iter = m_MM_entry_uuid_atr.begin(); iter != m_MM_entry_uuid_atr.end();
         iter++) {
      ATRecord atr = iter->second;
      bool bDelete = bCleanup && ((atr.uiflags & ATT_ATTACHMENT_DELETED) != 0);

      if (!bDelete && atr.cmpsize != 0) {
        // Update set with the attachments we want to keep.
        st_attmt_uuid.insert(atr.attmt_uuid);
        // Save the attachment record
        st_UUID stuuid(atr.attmt_uuid);
        mp_attmt_uuid_atr.insert(make_pair(stuuid, atr));
      }
    }

    // First process all existing attachments
    bool go(true);
    while (go && in != NULL) {
      atr.Clear();
      status = in->ReadAttmntRecordPreData(atr);
      switch (status) {
        case PWSRC::FAILURE:
        {
          // Show a useful (?) error message - better than
          // silently losing data (but not by much)
          // Best if title intact. What to do if not?
          if (m_pReporter != NULL) {
            stringT cs_msg, cs_caption;
            LoadAString(cs_caption, IDSC_READ_ERROR);
            Format(cs_msg, IDSC_ENCODING_PROBLEM, _T("???"));
            cs_msg = cs_caption + _S(": ") + cs_caption;
            (*m_pReporter)(cs_msg);
          }
          break;
        }
        case PWSRC::SUCCESS:
        {
          unsigned char readtype;
          unsigned int count(0);
          int status_r, status_w;

          // Only need to write it out if we want to keep it
          if (st_attmt_uuid.find(atr.attmt_uuid) != st_attmt_uuid.end()) {
            // Get attachment record
            UAMiter iter = mp_attmt_uuid_atr.find(atr.attmt_uuid);
            if (iter != mp_attmt_uuid_atr.end()) {
              // Update this entry with the fields that could have been changed:
              atr.flags = iter->second.flags;
              atr.description = iter->second.description;

              st_atpg.function = ATT_PROGRESS_START;
              LoadAString(st_atpg.function_text, IDSC_ATT_COPYFILE);
              st_atpg.atr = atr;
              st_atpg.value = 0;
              AttachmentProgress(st_atpg);

              // Write out pre-data
              out3->WriteAttmntRecordPreData(atr);

              do {
                unsigned char *pCmpData(NULL);
                unsigned int uiCmpLen;

                // Read in data records
                status_r = in->ReadAttmntRecordData(pCmpData, uiCmpLen, readtype, false);
                ASSERT(status_r == PWSRC::SUCCESS);

                // Write them back out
                status_w = out3->WriteAttmntRecordData(pCmpData, uiCmpLen, readtype);
                ASSERT(status_w == PWSRC::SUCCESS);

                // tidy up
                trashMemory(pCmpData, uiCmpLen);
                delete [] pCmpData;
                pCmpData = NULL;

                // Update progress dialog
                count += atr.blksize;
                st_atpg.function = ATT_PROGRESS_PROCESSFILE;
                st_atpg.value = (int)((count * 1.0E02) / atr.uncsize);
                AttachmentProgress(st_atpg);
              } while (readtype != PWSAttfile::ATTMT_LASTDATA);

              ASSERT(readtype == PWSAttfile::ATTMT_LASTDATA);

              // Read in post-date
              status = in->ReadAttmntRecordPostData(atr);

              // Write out post-data
              out3->WriteAttmntRecordPostData(atr);
              vATRWritten.push_back(atr);

              st_atpg.function = ATT_PROGRESS_PROCESSFILE;
              st_atpg.value = 100;
              AttachmentProgress(st_atpg);
            } else {
              // Should not happen!
              ASSERT(0);
            }
          } else {
            // We don't want this one - so we must skip over its data
            st_atpg.function = ATT_PROGRESS_START;
            LoadAString(st_atpg.function_text, IDSC_ATT_SKIPPINGFILE);
            st_atpg.atr = atr;
            st_atpg.value = 0;
            AttachmentProgress(st_atpg);
            do {
              unsigned char *pCmpData(NULL);
              unsigned int uiCmpLen;

              // Read in data records
              status_r = in->ReadAttmntRecordData(pCmpData, uiCmpLen, readtype, true);
              ASSERT(status_r == PWSRC::SUCCESS);

              // Update progress dialog
              count += atr.blksize;
              st_atpg.function = ATT_PROGRESS_PROCESSFILE;
              st_atpg.value = (int)((count * 1.0E02) / atr.uncsize);
              AttachmentProgress(st_atpg);
            } while (readtype != PWSAttfile::ATTMT_LASTDATA);

            ASSERT(readtype == PWSAttfile::ATTMT_LASTDATA);

            // Read in post-date
            status = in->ReadAttmntRecordPostData(atr);

            st_atpg.function = ATT_PROGRESS_PROCESSFILE;
            st_atpg.value = 100;
            AttachmentProgress(st_atpg);
          }
          break;
        }
        case PWSRC::END_OF_FILE:
          go = false;
          break;
      } // switch
    };

    int num_new(0);
    // Now process new attachments
    for (UAMMiter iter = m_MM_entry_uuid_atr.begin(); iter != m_MM_entry_uuid_atr.end();
         iter++) {
      ATRecord atr = iter->second;
      // A zero compressed file size says that it hasn't been added yet
      if (atr.cmpsize == 0) {
        if (num_new == 0) {
          st_atpg.function = ATT_PROGRESS_PROCESSFILE;
          LoadAString(st_atpg.function_text, IDSC_ATT_APPEND_NEW);
          st_atpg.value = -1;
          AttachmentProgress(st_atpg);
          st_atpg.function_text.clear();
        }

        num_new++;
        // Insert date added in attachment record
        atr.dtime = dtime;

        StringX fname = atr.path + atr.filename;

        // Verify that the file is still there!
        int irc = _taccess(fname.c_str(), F_OK);
        if (irc != 0) {
          // irc = -1 ->
          //   errno = ENOENT (2) - file not found!
          //         = EACCES (13) - no access
          //         = EINVAL (22) - invalid parameter
          pws_os::Trace(_T("Attachment file _taccess failed. RC=%d, errno=%d\n"), irc, errno);
          break;
        }

        // Open the file
        FILE *fh;
        errno_t err = _tfopen_s(&fh, fname.c_str(), _T("rb"));
        if (err != 0) {
          pws_os::Trace(_T("Attachment file _tfopen_s failed. Error code=%d\n"), err);
          break;
        }

        const bool bDoInIncrements = (atr.uncsize > MINBLOCKSIZE);
        if (atr.uncsize > MINBLOCKSIZE)
          atr.blksize = GetBlocksize(atr.uncsize);
        else
          atr.blksize = atr.uncsize;

        unsigned int uiUncLen(atr.blksize);
        unsigned int uiCmpLen = compressBound(uiUncLen);
        const unsigned int uiMaxCmpLen = uiCmpLen;

        BYTE *pUncData = new BYTE[atr.blksize];
        BYTE *pCmpData = new BYTE[uiMaxCmpLen];

        const unsigned int num_left = atr.uncsize % atr.blksize;
        unsigned int count(0), numread, totalread;
        totalread = 0;

        // Write out pre-data fields
        out3->WriteAttmntRecordPreData(atr);

        st_atpg.function = ATT_PROGRESS_PROCESSFILE;
        st_atpg.value = 0;
        AttachmentProgress(st_atpg);

        // Get SHA1 hash and CRC of uncompressed data and hash of compressed data
        // Only need one of hash/CRC for uncompressed data (hash more secure) but
        // the user can check the displayed CRC against that shown by, say, WinZip
        // to see if the same
        SHA1 ocontext, ccontext;

        if (!bDoInIncrements) {
          // Read it
          numread = fread(pUncData, 1, atr.uncsize, fh);
          ASSERT(numread == atr.uncsize);
          totalread = numread;

          // Calculate complete CRC and odigest
          ocontext.Update(pUncData, atr.uncsize);
          ocontext.Final(atr.odigest);
          atr.CRC = PWSUtil::Get_CRC(pUncData, atr.uncsize);

          int zRC;
          unsigned long ulCmpLen(uiCmpLen);
          zRC = compress(pCmpData, &ulCmpLen, pUncData, uiUncLen);
          ASSERT(zRC == Z_OK);

          atr.cmpsize = ulCmpLen;
          ccontext.Update(pCmpData, atr.cmpsize);
          ccontext.Final(atr.cdigest);

          out3->WriteAttmntRecordData(pCmpData, atr.cmpsize,
                                      PWSAttfileV3::ATTMT_LASTDATA);

          trashMemory(pCmpData, uiMaxCmpLen);
          delete [] pCmpData;
          trashMemory(pUncData, atr.uncsize);
          delete [] pUncData;
        } else {
          // Read in increments
          count = 0;
          z_stream strm;
          int zRC = PWS_Deflate_Init(strm);
          ASSERT(zRC == Z_OK);

          PWSUtil::Get_CRC_Incremental_Init();

          do {
            // Read it
            numread = fread(pUncData, 1, atr.blksize, fh);
            totalread += numread;

            // Update CRC & odigest
            ocontext.Update(pUncData, numread);
            PWSUtil::Get_CRC_Incremental_Update(pUncData, numread);

            count += atr.blksize;

            // Compress it - but first reset size f buffer available
            uiCmpLen = uiMaxCmpLen;
            zRC = PWS_Deflate_Buffer(strm, pUncData, numread,
                                     pCmpData, uiCmpLen,
                                     (count == atr.uncsize) ? true : false);

            // Update CRC & cdigest
            ccontext.Update(pCmpData, uiCmpLen);

            // Need to check - it just could be that the file size is an exact
            // multipe of the block size chosen!
            out3->WriteAttmntRecordData(pCmpData, uiCmpLen,
                            (unsigned char)((count == atr.uncsize) ?
                                        PWSAttfileV3::ATTMT_LASTDATA : PWSAttfileV3::ATTMT_DATA));
            atr.cmpsize += uiCmpLen;

            // Update progress dialog
            st_atpg.value = (int)((count * 1.0E02) / atr.uncsize);
            AttachmentProgress(st_atpg);
          } while (count < atr.uncsize - atr.blksize);

          // Write out the rump
          if (num_left > 0) {
            numread = fread(pUncData, 1, num_left, fh);
            ASSERT(numread == num_left);
            totalread += numread;

            ocontext.Update(pUncData, numread);
            PWSUtil::Get_CRC_Incremental_Update(pUncData, numread);

            // Compress it - but first reset size of buffer available
            uiCmpLen = uiMaxCmpLen;
            zRC = PWS_Deflate_Buffer(strm, pUncData, numread,
                                     pCmpData, uiCmpLen,
                                     true);
            ccontext.Update(pCmpData, uiCmpLen);
            atr.cmpsize += uiCmpLen;
            out3->WriteAttmntRecordData(pCmpData, uiCmpLen, PWSAttfileV3::ATTMT_LASTDATA);
          }

          // Tidy up compression
          if (zRC == Z_STREAM_END)
            zRC = Z_OK;

          PWS_Deflate_Term(strm);

          // Finish off CRC and digest and compressed size
          ocontext.Final(atr.odigest);
          ccontext.Final(atr.cdigest);
          atr.CRC = PWSUtil::Get_CRC_Incremental_Final();

          trashMemory(pCmpData, uiMaxCmpLen);
          delete [] pCmpData;
          trashMemory(pUncData, atr.blksize);
          delete [] pUncData;
        }

        // Write out post-data
        out3->WriteAttmntRecordPostData(atr);

        // Update progress dialog
        st_atpg.function = ATT_PROGRESS_PROCESSFILE;
        st_atpg.value = 100;
        AttachmentProgress(st_atpg);

        fclose(fh);
        if (totalread != atr.uncsize) {
          pws_os::Trace(_T("Attachment file fread - mismatch of data read. Expected=%d, Read=%d\n"),
                   atr.uncsize, totalread);
        }

        st_atpg.function = ATT_PROGRESS_END;
        AttachmentProgress(st_atpg);

        // Update in-memory records
        iter->second.dtime = atr.dtime;
        memcpy(iter->second.odigest, atr.odigest, SHA1::HASHLEN);
        memcpy(iter->second.cdigest, atr.cdigest, SHA1::HASHLEN);
        iter->second.CRC = atr.CRC;
        iter->second.cmpsize = atr.cmpsize;
        iter->second.blksize = atr.blksize;
        memcpy(iter->second.attmt_uuid, atr.attmt_uuid, sizeof(uuid_array_t));

        // Update info about records written
        vATRWritten.push_back(atr);
      }
    }

    st_atpg.function = ATT_PROGRESS_END;
    AttachmentProgress(st_atpg);

  } catch (...) {
    if (in != NULL) {
      in->Close();
      delete in;
    }
    out3->Close();
    delete out3;
    pws_os::DeleteAFile(tempfilename);

    st_atpg.function = ATT_PROGRESS_END;
    AttachmentProgress(st_atpg);
    return PWSRC::FAILURE;
  }

  if (in != NULL) {
    in->Close();
    delete in;
  }
  out3->Close();
  delete out3;

  // Remove change flag now that records have been written
  std::pair<UAMMiter, UAMMiter> uuidairpair;
  for (size_t i = 0; i < vATRWritten.size(); i++) {
    if ((vATRWritten[i].uiflags & ATT_ATTACHMENT_FLGCHGD) == 0)
      continue;

    uuidairpair = m_MM_entry_uuid_atr.equal_range(vATRWritten[i].entry_uuid);
    // Now find this specific attachment record
    for (UAMMiter iter = uuidairpair.first; iter != uuidairpair.second; ++iter) {
      if (memcmp(iter->second.attmt_uuid, vATRWritten[i].attmt_uuid, sizeof(uuid_array_t)) == 0) {
        iter->second.uiflags &= ~ATT_ATTACHMENT_FLGCHGD;
        break;
      }
    }
  }

  // If cleaning up - delete ATRecords no longer needed
  if (bCleanup) {
    for (UAMMiter iter = m_MM_entry_uuid_atr.begin(); iter != m_MM_entry_uuid_atr.end();) {
      if ((iter->second.uiflags & ATT_ATTACHMENT_DELETED) != 0) {
        m_MM_entry_uuid_atr.erase(iter++);
      } else {
        ++iter;
      }
    }
  }

  status = SaveAttachmentFile(tempfilename);
  return status;
}

int PWScore::DuplicateAttachments(const uuid_array_t &old_entry_uuid,
                                  const uuid_array_t &new_entry_uuid,
                                  PWSAttfile::VERSION version)
{
  int status;

  st_UUID stuuid(old_entry_uuid);
  size_t num = m_MM_entry_uuid_atr.count(stuuid);
  if (num == 0)
    return PWSRC::FAILURE;

  /*
    Generate a temporary file name - write to this file.  If OK, rename current
    file and then rename the temporary file to the proper name.
    This does leave the older file around as backup (for the moment).
  */

  PWSAttfile *in(NULL), *out(NULL), *dup(NULL);
  stringT tempfilename, current_file, dupfilename, timestamp;
  stringT sDrive, sDir, sName, sExt;
  time_t time_now;

  // Generate temporary names
  time(&time_now);
  timestamp = PWSUtil::ConvertToDateTimeString(time_now, TMC_FILE).c_str();
  pws_os::splitpath(m_currfile.c_str(), sDrive, sDir, sName, sExt);

  tempfilename = sDrive + sDir + sName + timestamp +
                 stringT(ATT_DEFAULT_ATTMT_SUFFIX);
  dupfilename = sDrive + sDir + sName + timestamp +
                 stringT(ATT_DEFAULT_ATTDUP_SUFFIX);
  current_file = sDrive + sDir + sName +
                 stringT(ATT_DEFAULT_ATTMT_SUFFIX);

  // If duplicating - they must be there to begin with!
  if (!pws_os::FileExists(current_file))
    return PWSRC::FAILURE;

  // 'Make' new file
  out = PWSAttfile::MakePWSfile(tempfilename.c_str(), version,
                                PWSAttfile::Write, status,
                                m_pAsker, m_pReporter);
  if (status != PWSRC::SUCCESS) {
    delete out;
    return status;
  }

  // Must be an existing attachment file, 'make' old file
  in = PWSAttfile::MakePWSfile(current_file.c_str(), version,
                               PWSAttfile::Read, status,
                               m_pAsker, m_pReporter);

  if (status != PWSRC::SUCCESS) {
    delete in;
    delete out;
    return status;
  }

  // 'Make' duplicates temporary file
  dup = PWSAttfile::MakePWSfile(dupfilename.c_str(), version,
                                PWSAttfile::Write, status,
                                m_pAsker, m_pReporter);

  if (status != PWSRC::SUCCESS) {
    delete in;
    delete out;
    delete dup;
    return status;
  }

  // XXX cleanup gross dynamic_cast
  PWSAttfileV3 *out3 = dynamic_cast<PWSAttfileV3 *>(out);
  PWSAttfileV3 *dup3 = dynamic_cast<PWSAttfileV3 *>(dup);

  // Set up header records
  SetupAttachmentHeader();

  // Set them - will be written during Open below
  out3->SetHeader(m_atthdr);
  dup3->SetHeader(m_atthdr);

  ATTProgress st_atpg;

  ATRVector vATRWritten, vATRDuplicates;

  try { // exception thrown on write error
    // Open new attachment file
    status = out3->Open(GetPassKey());

    if (status != PWSRC::SUCCESS) {
      delete out3;
      delete dup3;
      delete in;
      return status;
    }

    // Open temporary duplicates file
    status = dup3->Open(GetPassKey());
    if (status != PWSRC::SUCCESS) {
      out3->Close();
      delete out3;
      delete dup3;
      delete in;
      return status;
    }

    // Open current data file
    status = in->Open(GetPassKey());

    if (status != PWSRC::SUCCESS) {
      out3->Close();
      dup3->Close();
      delete out3;
      delete dup3;
      delete in;
      return status;
    }

    st_atpg.function = ATT_PROGRESS_START;
    LoadAString(st_atpg.function_text, IDSC_ATT_COPYFILE);
    st_atpg.value = 0;
    AttachmentProgress(st_atpg);
    st_atpg.function_text.clear();

    std::pair<UAMMciter, UAMMciter> uuidairpair;

    // Time stamp for date added for new attachments
    time_t dtime;
    time(&dtime);

    ATRecord atr;
    UUIDSet st_attmt_uuid;         // std::set from records on attmt_uuid
    UUIDATRMap mp_attmt_uuid_atr;  // std::map key = attmnt_uuid, value = attachment record

    // Find all existing attachments we want to keep (atr.cmpsize is filled in)
    for (UAMMiter iter = m_MM_entry_uuid_atr.begin(); iter != m_MM_entry_uuid_atr.end();
         iter++) {
      if (iter->second.cmpsize != 0) {
        // Update set with the attachments we want to keep.
        st_attmt_uuid.insert(iter->second.attmt_uuid);
        // Save the attachment record
        st_UUID stuuid(iter->second.attmt_uuid);
        mp_attmt_uuid_atr.insert(make_pair(stuuid, iter->second));
      }
    }

    // First process all existing attachments
    bool go(true);
    while (go && in != NULL) {
      atr.Clear();
      status = in->ReadAttmntRecordPreData(atr);

      switch (status) {
        case PWSRC::FAILURE:
        {
          // Show a useful (?) error message - better than
          // silently losing data (but not by much)
          // Best if title intact. What to do if not?
          if (m_pReporter != NULL) {
            stringT cs_msg, cs_caption;
            LoadAString(cs_caption, IDSC_READ_ERROR);
            Format(cs_msg, IDSC_ENCODING_PROBLEM, _T("???"));
            cs_msg = cs_caption + _S(": ") + cs_caption;
            (*m_pReporter)(cs_msg);
          }
          break;
        }
        case PWSRC::SUCCESS:
        {
          // Only need to write it out if we want to keep it - can't duplicate
          // attachments being deleted
          if (st_attmt_uuid.find(atr.attmt_uuid) != st_attmt_uuid.end()) {
            // Get attachment record
            UAMiter iter = mp_attmt_uuid_atr.find(atr.attmt_uuid);
            if (iter != mp_attmt_uuid_atr.end()) {
              // Update this entry with the fields that could have been changed:
              atr.flags = iter->second.flags;
              atr.description = iter->second.description;

              st_atpg.function = ATT_PROGRESS_PROCESSFILE;
              st_atpg.atr = atr;
              st_atpg.value = 0;
              AttachmentProgress(st_atpg);

              unsigned char readtype(0);
              unsigned char *pCmpData;
              unsigned int uiCmpLen, count(0);
              int status_r, status_w, status_d;

              ATRecord atr_dup(atr);
              bool bDuplicate(false);
              // Now maybe duplicate it
              if (memcmp(iter->second.entry_uuid, old_entry_uuid, sizeof(uuid_array_t)) == 0) {
                bDuplicate = true;
                uuid_array_t new_attmt_uuid;
                CUUIDGen attmt_uuid;
                attmt_uuid.GetUUID(new_attmt_uuid);
                // Change date added timestamp even though it was added to original entry
                atr_dup.dtime = dtime;
                memcpy(atr_dup.attmt_uuid, new_attmt_uuid, sizeof(uuid_array_t));
                memcpy(atr_dup.entry_uuid, new_entry_uuid, sizeof(uuid_array_t));
              }

              // Write out pre-data
              out3->WriteAttmntRecordPreData(atr);
              if (bDuplicate)
                dup3->WriteAttmntRecordPreData(atr_dup);

              status_d = PWSRC::SUCCESS;
              do {
                // Read in data records
                status_r = in->ReadAttmntRecordData(pCmpData, uiCmpLen, readtype, false);

                // Write them back out
                if (status_r == PWSRC::SUCCESS) {
                  status_w = out3->WriteAttmntRecordData(pCmpData, uiCmpLen, readtype);
                  if (bDuplicate)
                    status_d = dup3->WriteAttmntRecordData(pCmpData, uiCmpLen, readtype);
                }

                // tidy up
                trashMemory(pCmpData, uiCmpLen);
                delete [] pCmpData;
                pCmpData = NULL;

                // Update progress dialog
                count += atr.blksize;
                st_atpg.function = ATT_PROGRESS_PROCESSFILE;
                st_atpg.value = (int)((count * 1.0E02) / atr.uncsize);
                AttachmentProgress(st_atpg);
              } while(status_r == PWSRC::SUCCESS &&
                      status_w == PWSRC::SUCCESS &&
                      status_d == PWSRC::SUCCESS &&
                      readtype != PWSAttfileV3::ATTMT_LASTDATA);

              // Write out post-data
              in->ReadAttmntRecordPostData(atr);
              out3->WriteAttmntRecordPostData(atr);
              if (bDuplicate) {
                // Update post-data fields
                atr_dup.cmpsize = atr.cmpsize;
                atr_dup.CRC = atr.CRC;
                memcpy(atr_dup.odigest, atr.odigest, SHA1::HASHLEN);
                memcpy(atr_dup.cdigest, atr.cdigest, SHA1::HASHLEN);
                dup3->WriteAttmntRecordPostData(atr_dup);
                vATRDuplicates.push_back(atr_dup);
              }

              vATRWritten.push_back(atr);

              st_atpg.function = ATT_PROGRESS_PROCESSFILE;
              st_atpg.value = 100;
              AttachmentProgress(st_atpg);
            }
          }
          break;
        }
        case PWSRC::END_OF_FILE:
          go = false;
          break;
      } // switch
    };
  } catch (...) {
    in->Close();
    delete in;
    out3->Close();
    delete out3;
    dup3->Close();
    delete dup3;

    pws_os::DeleteAFile(tempfilename);
    pws_os::DeleteAFile(dupfilename);

    st_atpg.function = ATT_PROGRESS_END;
    AttachmentProgress(st_atpg);

    return PWSRC::FAILURE;
  }

  // Close input file
  in->Close();
  delete in;
  in = NULL;

  // Close duplicates temporary file
  dup3->Close();
  delete dup3;
  dup = dup3 = NULL;

  // Now re-open duplicates file and now copy them at the end of our new file
  dup = PWSAttfile::MakePWSfile(dupfilename.c_str(), version,
                                PWSAttfile::Read, status,
                                m_pAsker, m_pReporter);

  if (status != PWSRC::SUCCESS) {
    out3->Close();
    delete out3;
    delete dup;

    pws_os::DeleteAFile(tempfilename);
    pws_os::DeleteAFile(dupfilename);
    return status;
  }

  dup3 = dynamic_cast<PWSAttfileV3 *>(dup);
  try { // exception thrown on write error
    // Open duplicate attachment file
    status = dup3->Open(GetPassKey());

    if (status != PWSRC::SUCCESS) {
      out3->Close();
      delete out3;
      delete dup3;

      pws_os::DeleteAFile(tempfilename);
      pws_os::DeleteAFile(dupfilename);

      st_atpg.function = ATT_PROGRESS_END;
      AttachmentProgress(st_atpg);
      return status;
    }

    st_atpg.function = ATT_PROGRESS_START;
    LoadAString(st_atpg.function_text, IDSC_ATT_APPEND_DUPS);
    st_atpg.value = 0;
    AttachmentProgress(st_atpg);
    st_atpg.function_text.clear();

  // Process all duplicate attachments and append to output file
  bool go(true);
  ATRecord atr;
  while (go && dup != NULL) {
    atr.Clear();
    status = dup3->ReadAttmntRecordPreData(atr);
    switch (status) {
      case PWSRC::FAILURE:
      {
        // Show a useful (?) error message - better than
        // silently losing data (but not by much)
        // Best if title intact. What to do if not?
        if (m_pReporter != NULL) {
          stringT cs_msg, cs_caption;
          LoadAString(cs_caption, IDSC_READ_ERROR);
          Format(cs_msg, IDSC_ENCODING_PROBLEM, _T("???"));
          cs_msg = cs_caption + _S(": ") + cs_caption;
          (*m_pReporter)(cs_msg);
        }
        break;
      }
      case PWSRC::SUCCESS:
      {
        st_atpg.function = ATT_PROGRESS_PROCESSFILE;
        st_atpg.atr = atr;
        st_atpg.value = 0;
        AttachmentProgress(st_atpg);

        unsigned char readtype(0);
        unsigned char *pCmpData;
        unsigned int uiCmpLen, count(0);
        int status_r, status_w;

        // Write out pre-data
        out3->WriteAttmntRecordPreData(atr);

        do {
          // Read in data records
          status_r = dup3->ReadAttmntRecordData(pCmpData, uiCmpLen, readtype, false);

          // Write them back out
          if (status_r == PWSRC::SUCCESS)
            status_w = out3->WriteAttmntRecordData(pCmpData, uiCmpLen, readtype);

          // tidy up
          trashMemory(pCmpData, uiCmpLen);
          delete [] pCmpData;
          pCmpData = NULL;

          // Update progress dialog
          count += atr.blksize;
          st_atpg.value = (int)((count * 1.0E02) / atr.uncsize);
          AttachmentProgress(st_atpg);
        } while(status_r == PWSRC::SUCCESS &&
                status_w == PWSRC::SUCCESS &&
                readtype != PWSAttfileV3::ATTMT_LASTDATA);

        // Write out post-data
        dup3->ReadAttmntRecordPostData(atr);
        out3->WriteAttmntRecordPostData(atr);
        vATRWritten.push_back(atr);

        st_atpg.function = ATT_PROGRESS_PROCESSFILE;
        st_atpg.value = 100;
        AttachmentProgress(st_atpg);
        break;
      }
      case PWSRC::END_OF_FILE:
        go = false;
        break;
    } // switch
  };
  } catch (...) {
    out3->Close();
    delete out3;
    dup3->Close();
    delete dup3;

    pws_os::DeleteAFile(tempfilename);
    pws_os::DeleteAFile(dupfilename);

    st_atpg.function = ATT_PROGRESS_END;
    AttachmentProgress(st_atpg);
    return PWSRC::FAILURE;
  }

  // Now close output file
  out3->Close();
  delete out3;

  // Now close temporary duplicates file
  dup3->Close();
  delete dup3;

  // Delete temporary duplicates file
  pws_os::DeleteAFile(dupfilename);

  // Remove change flag now that records have been written
  std::pair<UAMMiter, UAMMiter> uuidairpair;
  for (size_t i = 0; i < vATRWritten.size(); i++) {
    if ((vATRWritten[i].uiflags & ATT_ATTACHMENT_FLGCHGD) == 0)
      continue;

    uuidairpair = m_MM_entry_uuid_atr.equal_range(vATRWritten[i].entry_uuid);
    // Now find this specific attachment record
    for (UAMMiter iter = uuidairpair.first; iter != uuidairpair.second; ++iter) {
      if (memcmp(iter->second.attmt_uuid, vATRWritten[i].attmt_uuid, sizeof(uuid_array_t)) == 0) {
        iter->second.uiflags &= ~ATT_ATTACHMENT_FLGCHGD;
        break;
      }
    }
  }

  // Now update main multimap with the duplicate attachments
  st_UUID st_newuuid(new_entry_uuid);
  for (size_t i = 0; i < vATRDuplicates.size(); i++) {
    m_MM_entry_uuid_atr.insert(make_pair(st_newuuid, vATRDuplicates[i]));
  }

  st_atpg.function = ATT_PROGRESS_END;
  AttachmentProgress(st_atpg);

  status = SaveAttachmentFile(tempfilename);
  return status;
}

int PWScore::SaveAttachmentFile(const stringT &tempfilename)
{
  // Now get current date/time to be used to rename the current file
  // so that we can save it and then rename the new one appropriately
  stringT current_data_file, timestamp;
  stringT sDrive, sDir, sName, sExt;
  stringT oldname, backup_name;
  time_t now;

  time(&now);
  timestamp = PWSUtil::ConvertToDateTimeString(now, TMC_FILE).c_str();

  pws_os::splitpath(m_currfile.c_str(), sDrive, sDir, sName, sExt);

  backup_name = sDrive + sDir + sName + timestamp +
                        stringT(ATT_DEFAULT_ATTBKUP_SUFFIX);

  oldname = sDrive + sDir + sName +
                    stringT(ATT_DEFAULT_ATTMT_SUFFIX);

  bool brc(true);

  bool bOldFileExists = pws_os::FileExists(oldname);

  if (bOldFileExists) {
    // Rename old file
    brc = pws_os::RenameFile(oldname, backup_name);
  }

  if (!brc) {
    pws_os::Trace(_T("Unable to rename existing attachment file.\n"));
    return PWSRC::FAILURE;
  }

  if (m_MM_entry_uuid_atr.size() == 0) {
    // Delete new attachment file if they empty
    pws_os::DeleteAFile(tempfilename);
    return PWSRC::SUCCESS;
  }

  if (brc) {
    // Either the file existed and was renamed successfully or
    // it didn't exist in the first place
    if (!pws_os::RenameFile(tempfilename, oldname)) {
      // Rename failed - rename back if it previously existed and also keep new file!
      pws_os::Trace(_T("Unable to rename new file.\n"));
      // Put old ones back
      if (bOldFileExists)
        pws_os::RenameFile(backup_name, oldname);
      return PWSRC::FAILURE;
    }
  }

  SetChanged(false, false);
  return PWSRC::SUCCESS;
}

void PWScore::SetupAttachmentHeader()
{
  // Set up header records
  m_atthdr.whatlastsaved = m_AppNameAndVersion.c_str();

  time_t time_now;
  time(&time_now);
  m_atthdr.whenlastsaved = time_now;

  uuid_array_t attfile_uuid, null_uuid = {0};
  if (memcmp(m_atthdr.attfile_uuid, null_uuid, sizeof(uuid_array_t)) == 0) {
    CUUIDGen att_uuid;
    att_uuid.GetUUID(attfile_uuid);
    memcpy(m_atthdr.attfile_uuid, attfile_uuid, sizeof(uuid_array_t));
    memcpy(m_atthdr.DBfile_uuid, m_hdr.m_file_uuid_array, sizeof(uuid_array_t));
  }
}

// functor to check if the current attachment matches the one we want to export
struct MatchAUUID {
  MatchAUUID(const uuid_array_t &attmt_uuid)
  {
    memcpy(m_attmt_uuid, attmt_uuid, sizeof(uuid_array_t));
  }

  // Does it match?
  bool operator()(const ATRecordEx &atrex) {
    return memcmp(m_attmt_uuid, atrex.atr.attmt_uuid, sizeof(uuid_array_t)) == 0;
  }

private:
  MatchAUUID& operator=(const MatchAUUID&); // Do not implement
  uuid_array_t m_attmt_uuid;
};

int PWScore::WriteXMLAttachmentFile(const StringX &filename, ATFVector &vatf,
                                    ATRExVector &vAIRecordExs,
                                    size_t &num_exported)
{
  num_exported = 0;
  const bool bAll = vAIRecordExs.size() == 0;
  if (bAll)
    GetAllAttachments(vAIRecordExs);

  CUTF8Conv utf8conv;
  const unsigned char *utf8 = NULL;
  int utf8Len = 0;

#ifdef UNICODE
  utf8conv.ToUTF8(filename, utf8, utf8Len);
#else
  utf8 = filename.c_str();
#endif

  ofstream ofs(reinterpret_cast<const char *>(utf8));

  if (!ofs)
    return PWSRC::CANT_OPEN_FILE;

  ATTProgress st_atpg;
  int status;

  stringT attmnt_file;
  stringT drv, dir, name, ext;
  StringX sxFilename;

  pws_os::splitpath(m_currfile.c_str(), drv, dir, name, ext);
  ext = stringT(ATT_DEFAULT_ATTMT_SUFFIX);
  attmnt_file = drv + dir + name + ext;
  sxFilename = attmnt_file.c_str();

  PWSAttfile::VERSION version = PWSAttfile::V30;

  // 'Make' the data file
  PWSAttfile *in = PWSAttfile::MakePWSfile(sxFilename, version,
                               PWSAttfile::Read, status,
                               m_pAsker, m_pReporter);

  if (status != PWSRC::SUCCESS) {
    delete in;
    in = NULL;
    return status;
  }

  // Open the data file
  status = in->Open(GetPassKey());
  if (status != PWSRC::SUCCESS) {
    delete in;
    in = NULL;
    return PWSRC::CANT_OPEN_FILE;
  }

  UUIDATRMap mp_attmt_uuid_atr;  // std::map key = attmt_uuid, value = attachment record

  // Find all existing attachments
  for (UAMMiter iter = m_MM_entry_uuid_atr.begin(); iter != m_MM_entry_uuid_atr.end();
       iter++) {
    ATRecord atr = iter->second;
    // Save the attachment record
    st_UUID stuuid(atr.attmt_uuid);
    mp_attmt_uuid_atr.insert(make_pair(stuuid, atr));
  }

  oStringXStream oss_xml;
  time_t time_now;
  StringX tmp, temp;

  time(&time_now);
  const StringX now = PWSUtil::ConvertToDateTimeString(time_now, TMC_XML);

  ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  ofs << "<?xml-stylesheet type=\"text/xsl\" href=\"pwsafe.xsl\"?>" << endl;
  ofs << endl;
  ofs << "<passwordsafe_attachments" << endl;
  temp = m_currfile;
  Replace(temp, StringX(_T("&")), StringX(_T("&amp;")));

  utf8conv.ToUTF8(temp, utf8, utf8Len);
  ofs << "Database=\"";
  ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
  ofs << "\"" << endl;
  utf8conv.ToUTF8(now, utf8, utf8Len);
  ofs << "ExportTimeStamp=\"";
  ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
  ofs << "\"" << endl;
  ofs << "FromDatabaseFormat=\"";
  ostringstream osv; // take advantage of UTF-8 == ascii for version string
  osv << m_hdr.m_nCurrentMajorVersion
      << "." << setw(2) << setfill('0')
      << m_hdr.m_nCurrentMinorVersion;
  ofs.write(osv.str().c_str(), osv.str().length());
  ofs << "\"" << endl;
  if (!m_hdr.m_lastsavedby.empty() || !m_hdr.m_lastsavedon.empty()) {
    oStringXStream oss;
    oss << m_hdr.m_lastsavedby << _T(" on ") << m_hdr.m_lastsavedon;
    utf8conv.ToUTF8(oss.str(), utf8, utf8Len);
    ofs << "WhoSaved=\"";
    ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
    ofs << "\"" << endl;
  }
  if (!m_hdr.m_whatlastsaved.empty()) {
    utf8conv.ToUTF8(m_hdr.m_whatlastsaved, utf8, utf8Len);
    ofs << "WhatSaved=\"";
    ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
    ofs << "\"" << endl;
  }
  if (m_hdr.m_whenlastsaved != 0) {
    StringX wls = PWSUtil::ConvertToDateTimeString(m_hdr.m_whenlastsaved,
                                                   TMC_XML);
    utf8conv.ToUTF8(wls, utf8, utf8Len);
    ofs << "WhenLastSaved=\"";
    ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
    ofs << "\"" << endl;
  }

  CUUIDGen db_uuid(m_hdr.m_file_uuid_array, true); // true to print canoncally
  CUUIDGen att_uuid(m_atthdr.attfile_uuid, true); // true to print canoncally

  ofs << "Database_uuid=\"" << db_uuid << "\"" << endl;
  ofs << "Attachment_file_uuid=\"" << att_uuid << "\"" << endl;
  ofs << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl;
  ofs << "xsi:noNamespaceSchemaLocation=\"pwsafe.xsd\">" << endl;
  ofs << endl;

  st_atpg.function = ATT_PROGRESS_START;
  st_atpg.value = 0;
  AttachmentProgress(st_atpg);

  // Get the header (not sure why?)
  PWSAttfile::AttHeaderRecord ahr = in->GetHeader();

  // We will use in-storage copy of atr (as it has all the data we need)
  // but we have to read the attachment data from the file!
  unsigned int cmpsize;

  do {
    ATRecordEx atrex;
    ATRecord atr;
    status = in->ReadAttmntRecordPreData(atr);
    if (status == PWSRC::END_OF_FILE)
      break;

    if (status != PWSRC::SUCCESS) {
      in->Close();
      delete in;
      in = NULL;

      st_atpg.function = ATT_PROGRESS_END;
      AttachmentProgress(st_atpg);
      return status;
    }

    st_atpg.function = ATT_PROGRESS_PROCESSFILE;
    st_atpg.value = 0;
    AttachmentProgress(st_atpg);

    // Now find this in the in-storage copy (so we don't have to wait till the
    // end of the data to get items we need to write now!
    st_UUID stuuid(atr.attmt_uuid);
    UAMciter citer = mp_attmt_uuid_atr.find(stuuid);
    ASSERT(citer != mp_attmt_uuid_atr.end());
    atrex.atr = citer->second;

    // Set up extra fields
    ItemListIter entry_iter = Find(atrex.atr.entry_uuid);
    atrex.sxGroup = entry_iter->second.GetGroup();
    atrex.sxTitle = entry_iter->second.GetTitle();
    atrex.sxUser = entry_iter->second.GetUser();

    // If matches - write out pre-data
    const bool bMatches = AttMatches(atrex, vatf) && (bAll ||
       std::find_if(vAIRecordExs.begin(), vAIRecordExs.end(), MatchAUUID(atr.attmt_uuid)) !=
                    vAIRecordExs.end());
    if (bMatches) {
      num_exported++;
      ofs << "\t<attachment id=\"" << dec << num_exported << "\" >" << endl;

      // NOTE - ORDER MAUST CORRESPOND TO ORDER IN SCHEMA
      if (!atrex.sxGroup.empty())
        PWSUtil::WriteXMLField(ofs, "group", atrex.sxGroup, utf8conv);

      PWSUtil::WriteXMLField(ofs, "title", atrex.sxTitle, utf8conv);

      if (!atrex.sxUser.empty())
        PWSUtil::WriteXMLField(ofs, "username", atrex.sxUser, utf8conv);

      uuid_array_t attachment_uuid, entry_uuid;
      memcpy(attachment_uuid, atrex.atr.attmt_uuid, sizeof(uuid_array_t));
      memcpy(entry_uuid, atrex.atr.entry_uuid, sizeof(uuid_array_t));
      const CUUIDGen a_uuid(attachment_uuid), e_uuid(entry_uuid);
      ofs << "\t\t<attachment_uuid><![CDATA[" << a_uuid << "]]></attachment_uuid>" << endl;
      ofs << "\t\t<entry_uuid><![CDATA[" << e_uuid << "]]></entry_uuid>" << endl;

      PWSUtil::WriteXMLField(ofs, "filename", atrex.atr.filename, utf8conv);
      PWSUtil::WriteXMLField(ofs, "path", atrex.atr.path, utf8conv);
      PWSUtil::WriteXMLField(ofs, "description", atrex.atr.description, utf8conv);

      ofs << "\t\t<osize>" << dec << atrex.atr.uncsize << "</osize>" << endl;
      ofs << "\t\t<bsize>" << dec << atrex.atr.blksize << "</bsize>" << endl;
      ofs << "\t\t<csize>" << dec << atrex.atr.cmpsize << "</csize>" << endl;

      ofs << "\t\t<crc>" << hex << setfill('0') << setw(8)
          << atrex.atr.CRC << "</crc>" << endl;

      // add digest of original file
      for (unsigned int i = 0; i < SHA1::HASHLEN; i++) {
        Format(temp, _T("%02x"), atrex.atr.odigest[i]);
        tmp += temp;
      }
      utf8conv.ToUTF8(tmp, utf8, utf8Len);
      ofs << "\t\t<odigest>" << utf8 << "</odigest>" << endl;

      tmp.clear();
      temp.clear();
      // add digest of compressed file
      for (unsigned int i = 0; i < SHA1::HASHLEN; i++) {
        Format(temp, _T("%02x"), atrex.atr.cdigest[i]);
        tmp += temp;
      }
      utf8conv.ToUTF8(tmp, utf8, utf8Len);
      ofs << "\t\t<cdigest>" << utf8 << "</cdigest>" << endl;

      ofs << PWSUtil::GetXMLTime(2, "ctime", atrex.atr.ctime, utf8conv);
      ofs << PWSUtil::GetXMLTime(2, "atime", atrex.atr.atime, utf8conv);
      ofs << PWSUtil::GetXMLTime(2, "mtime", atrex.atr.mtime, utf8conv);
      ofs << PWSUtil::GetXMLTime(2, "dtime", atrex.atr.dtime, utf8conv);

      if (atrex.atr.flags != 0) {
        ofs << "\t\t<flags>" << endl;
        if ((atrex.atr.flags & ATT_EXTRACTTOREMOVEABLE) == ATT_EXTRACTTOREMOVEABLE)
          ofs << "\t\t\t<extracttoremoveable>1</extracttoremoveable>" << endl;
        if ((atrex.atr.flags & ATT_ERASEPGMEXISTS) == ATT_ERASEPGMEXISTS)
          ofs << "\t\t\t<eraseprogamexists>1</eraseprogamexists>" << endl;
        if ((atrex.atr.flags & ATT_ERASEONDBCLOSE) == ATT_ERASEONDBCLOSE)
          ofs << "\t\t\t<eraseondatabaseclose>1</eraseondatabaseclose>" << endl;
        ofs << "\t\t</flags>" << endl;
      }
    }

    // Read all data
    unsigned int uiCmpLen;
    unsigned char *pCmpData(NULL);
    cmpsize = 0;
    int num_data_records = 0;
    unsigned char readtype;

    do {
      status = in->ReadAttmntRecordData(pCmpData, uiCmpLen, readtype, !bMatches);

      if (bMatches) {
        ofs << "\t\t<data" << (readtype == PWSAttfileV3::ATTMT_DATA ? "80" : "81") << "><![CDATA[" << endl;
        cmpsize += uiCmpLen;
        num_data_records++;

        tmp = PWSUtil::Base64Encode(pCmpData, uiCmpLen).c_str();
        utf8conv.ToUTF8(tmp, utf8, utf8Len);
        for (unsigned int i = 0; i < (unsigned int)utf8Len; i += 64) {
          char buffer[65];
          memset(buffer, 0, sizeof(buffer));
          if (utf8Len - i > 64)
            memcpy(buffer, utf8 + i, 64);
          else
            memcpy(buffer, utf8 + i, utf8Len - i);

          ofs << "\t\t" << buffer << endl;
        }

        ofs << "\t\t]]></data" << (readtype == PWSAttfileV3::ATTMT_DATA ? "80" : "81") << ">" << endl;

        // tidy up
        trashMemory(pCmpData, uiCmpLen);
        delete [] pCmpData;
        pCmpData = NULL;
      }

      st_atpg.atr = atrex.atr;
      st_atpg.function = bMatches ? ATT_PROGRESS_EXPORTFILE : ATT_PROGRESS_SEARCHFILE;
      st_atpg.value = (int)((cmpsize * 1.0E02) / atrex.atr.cmpsize);
      AttachmentProgress(st_atpg);
    } while (status == PWSRC::SUCCESS && readtype != PWSAttfileV3::ATTMT_LASTDATA);

    // Read post-data for this attachment
    status = in->ReadAttmntRecordPostData(atr);
    if (bMatches) {
      ofs << "\t</attachment>" << endl;
    }

    st_atpg.function = bMatches ? ATT_PROGRESS_EXPORTFILE : ATT_PROGRESS_SEARCHFILE;
    st_atpg.value = 100;
    AttachmentProgress(st_atpg);

  } while (status == PWSRC::SUCCESS);

  st_atpg.function = ATT_PROGRESS_END;
  AttachmentProgress(st_atpg);

  ahr.Clear();

  // Close the data file
  in->Close();
  delete in;
  in = NULL;

  ofs << "</passwordsafe_attachments>" << endl;
  ofs.close();

  return PWSRC::SUCCESS;
}

#if !defined(USE_XML_LIBRARY) || (!defined(_WIN32) && USE_XML_LIBRARY == MSXML)
// Don't support importing XML on non-Windows platforms using Microsoft XML libraries
int PWScore::ImportXMLAttachmentFile(const stringT &,
                           const stringT &,
                           stringT &, stringT &,
                           int &, int &, int &,
                           CReport &, Command *&)
{
  return PWSRC::UNIMPLEMENTED;
}
#else
int PWScore::ImportXMLAttachmentFile(const stringT &strXMLFileName,
                           const stringT &strXSDFileName,
                           stringT &strXMLErrors, stringT &strSkippedList,
                           int &numValidated, int &numImported, int &numSkipped,
                           CReport &rpt, Command *&pcommand)
{
  UUIDVector Possible_Aliases, Possible_Shortcuts;
  MultiCommands *pmulticmds = MultiCommands::Create(this);
  pcommand = pmulticmds;

#if   USE_XML_LIBRARY == EXPAT
  EAttXMLProcessor iXML(this, pmulticmds, &rpt);
#elif USE_XML_LIBRARY == MSXML
  MAttXMLProcessor iXML(this, pmulticmds, &rpt);
#elif USE_XML_LIBRARY == XERCES
  XAttXMLProcessor iXML(this, pmulticmds, &rpt);
#endif

  bool status, validation;
  int istatus;

  strXMLErrors = _T("");

  // Expat is not a validating parser - but we now do it ourselves!
  validation = true;
  status = iXML.Process(validation, strXMLFileName, strXSDFileName, NULL);
  strXMLErrors = iXML.getXMLErrors();

  if (!status) {
    return PWSRC::XML_FAILED_VALIDATION;
  }
  numValidated = iXML.getNumAttachmentsValidated();

  validation = false;
  stringT impfilename;
  PWSAttfile *pimport = CreateImportFile(impfilename);

  status = iXML.Process(validation, strXMLFileName,
                        strXSDFileName, pimport);

  istatus = XCompleteImportFile(impfilename);

  numImported = iXML.getNumAttachmentsImported();
  numSkipped = iXML.getNumAttachmentsSkipped();

  strXMLErrors = iXML.getXMLErrors();
  strSkippedList = iXML.getSkippedList();

  if (!status || istatus != PWSRC::SUCCESS) {
    delete pcommand;
    pcommand = NULL;
    return PWSRC::XML_FAILED_IMPORT;
  }

  if (numImported > 0)
    SetDBChanged(true);

  return PWSRC::SUCCESS ;
}

PWSAttfile *PWScore::CreateImportFile(stringT &impfilename, PWSAttfile::VERSION version)
{
  PWSAttfile *imp(NULL);
  stringT timestamp;
  stringT sDrive, sDir, sName, sExt;
  time_t time_now;
  int status;

  time(&time_now);
  timestamp = PWSUtil::ConvertToDateTimeString(time_now, TMC_FILE).c_str();
  pws_os::splitpath(m_currfile.c_str(), sDrive, sDir, sName, sExt);

  impfilename = sDrive + sDir + sName + timestamp +
                 stringT(ATT_DEFAULT_ATTIMP_SUFFIX);

  // 'Make' new file
  imp = PWSAttfile::MakePWSfile(impfilename.c_str(), version,
                                PWSAttfile::Write, status,
                                m_pAsker, m_pReporter);

  if (status != PWSRC::SUCCESS) {
    delete imp;
    imp = NULL;
    return imp;
  }

  PWSAttfileV3 *imp3 = dynamic_cast<PWSAttfileV3 *>(imp);
  status = imp3->Open(GetPassKey());

  if (status != PWSRC::SUCCESS) {
    delete imp3;
    imp = NULL;
  }
  return imp;
}

int PWScore::CompleteImportFile(const stringT &impfilename, PWSAttfile::VERSION version)
{
  PWSAttfile *in(NULL), *out(NULL), *imp(NULL);
  stringT tempfilename, current_file, timestamp;
  stringT sDrive, sDir, sName, sExt;
  time_t time_now;
  int status;

  // Generate temporary names
  time(&time_now);
  timestamp = PWSUtil::ConvertToDateTimeString(time_now, TMC_FILE).c_str();
  pws_os::splitpath(m_currfile.c_str(), sDrive, sDir, sName, sExt);

  tempfilename = sDrive + sDir + sName + timestamp +
                 stringT(ATT_DEFAULT_ATTMT_SUFFIX);

  current_file = sDrive + sDir + sName +
                 stringT(ATT_DEFAULT_ATTMT_SUFFIX);

  // 'Make' new file
  out = PWSAttfile::MakePWSfile(tempfilename.c_str(), version,
                                PWSAttfile::Write, status,
                                m_pAsker, m_pReporter);
  if (status != PWSRC::SUCCESS) {
    delete out;
    return status;
  }

  // Must be an existing attachment file, 'make' old file
  in = PWSAttfile::MakePWSfile(current_file.c_str(), version,
                               PWSAttfile::Read, status,
                               m_pAsker, m_pReporter);

  if (status != PWSRC::SUCCESS) {
    delete in;
    delete out;
    return status;
  }

  // XXX cleanup gross dynamic_cast
  PWSAttfileV3 *out3 = dynamic_cast<PWSAttfileV3 *>(out);

  // Set up header records
  SetupAttachmentHeader();

  // Set them - will be written during Open below
  out3->SetHeader(m_atthdr);

  ATTProgress st_atpg;

  st_atpg.function = ATT_PROGRESS_START;
  LoadAString(st_atpg.function_text, IDSC_ATT_COPYFILE);
  st_atpg.value = 0;
  AttachmentProgress(st_atpg);
  st_atpg.function_text.clear();

  ATRVector vATRWritten, vATRImports;

  try { // exception thrown on write error
    // Open new attachment file
    status = out3->Open(GetPassKey());

    if (status != PWSRC::SUCCESS) {
      delete out3;
      delete in;

      st_atpg.function = ATT_PROGRESS_END;
      AttachmentProgress(st_atpg);
      return status;
    }

    // Open current data file
    status = in->Open(GetPassKey());

    if (status != PWSRC::SUCCESS) {
      out3->Close();
      delete out3;
      delete in;

      st_atpg.function = ATT_PROGRESS_END;
      AttachmentProgress(st_atpg);
      return status;
    }

    std::pair<UAMMciter, UAMMciter> uuidairpair;

    // Time stamp for date added for new attachments
    time_t dtime;
    time(&dtime);

    ATRecord atr;
    UUIDSet st_attmt_uuid;         // std::set from records on attmt_uuid
    UUIDATRMap mp_attmt_uuid_atr;  // std::map key = attmnt_uuid, value = attachment record

    // Find all existing attachments we want to keep (atr.cmpsize is filled in)
    for (UAMMiter iter = m_MM_entry_uuid_atr.begin(); iter != m_MM_entry_uuid_atr.end();
         iter++) {
      if (iter->second.cmpsize != 0) {
        // Update set with the attachments we want to keep.
        st_attmt_uuid.insert(iter->second.attmt_uuid);
        // Save the attachment record
        st_UUID stuuid(iter->second.attmt_uuid);
        mp_attmt_uuid_atr.insert(make_pair(stuuid, iter->second));
      }
    }

    // First process all existing attachments
    bool go(true);
    while (go && in != NULL) {
      atr.Clear();
      status = in->ReadAttmntRecordPreData(atr);

      st_atpg.function = ATT_PROGRESS_PROCESSFILE;
      st_atpg.atr = atr;
      st_atpg.value = 0;
      AttachmentProgress(st_atpg);

      switch (status) {
        case PWSRC::FAILURE:
        {
          // Show a useful (?) error message - better than
          // silently losing data (but not by much)
          // Best if title intact. What to do if not?
          if (m_pReporter != NULL) {
            stringT cs_msg, cs_caption;
            LoadAString(cs_caption, IDSC_READ_ERROR);
            Format(cs_msg, IDSC_ENCODING_PROBLEM, _T("???"));
            cs_msg = cs_caption + _S(": ") + cs_caption;
            (*m_pReporter)(cs_msg);
          }
          break;
        }
        case PWSRC::SUCCESS:
        {
          // Only need to write it out if we want to keep it - can't duplicate
          // attachments being deleted
          if (st_attmt_uuid.find(atr.attmt_uuid) != st_attmt_uuid.end()) {
            // Get attachment record
            UAMiter iter = mp_attmt_uuid_atr.find(atr.attmt_uuid);
            if (iter != mp_attmt_uuid_atr.end()) {
              // Update this entry with the fields that could have been changed:
              atr.flags = iter->second.flags;
              atr.description = iter->second.description;

              unsigned char readtype(0);
              unsigned char *pCmpData;
              unsigned int uiCmpLen, count(0);
              int status_r, status_w;

              // Write out pre-data
              out3->WriteAttmntRecordPreData(atr);
              status_w = PWSRC::SUCCESS;

              do {
                // Read in data records
                status_r = in->ReadAttmntRecordData(pCmpData, uiCmpLen, readtype, false);

                // Write them back out
                if (status_r == PWSRC::SUCCESS) {
                  status_w = out3->WriteAttmntRecordData(pCmpData, uiCmpLen, readtype);
                }

                // tidy up
                trashMemory(pCmpData, uiCmpLen);
                delete [] pCmpData;
                pCmpData = NULL;

                // Update progress dialog
                count += atr.blksize;
                st_atpg.function = ATT_PROGRESS_PROCESSFILE;
                st_atpg.value = (int)((count * 1.0E02) / atr.uncsize);
                AttachmentProgress(st_atpg);
              } while(status_r == PWSRC::SUCCESS &&
                      status_w == PWSRC::SUCCESS &&
                      readtype != PWSAttfileV3::ATTMT_LASTDATA);

              // Write out post-data
              in->ReadAttmntRecordPostData(atr);
              out3->WriteAttmntRecordPostData(atr);
              vATRWritten.push_back(atr);

              st_atpg.function = ATT_PROGRESS_PROCESSFILE;
              st_atpg.value = 100;
              AttachmentProgress(st_atpg);
            }
          }
          break;
        }
        case PWSRC::END_OF_FILE:
          go = false;
          break;
      } // switch
    };
  } catch (...) {
    in->Close();
    delete in;
    out3->Close();
    delete out3;

    pws_os::DeleteAFile(tempfilename);
    pws_os::DeleteAFile(impfilename);

    st_atpg.function = ATT_PROGRESS_END;
    AttachmentProgress(st_atpg);
    return PWSRC::FAILURE;
  }

  // Close input file
  in->Close();
  delete in;
  in = NULL;

  // Now re-open duplicates file and now copy them at the end of our new file
  imp = PWSAttfile::MakePWSfile(impfilename.c_str(), version,
                                PWSAttfile::Read, status,
                                m_pAsker, m_pReporter);

  if (status != PWSRC::SUCCESS) {
    out3->Close();
    delete out3;

    pws_os::DeleteAFile(tempfilename);
    pws_os::DeleteAFile(impfilename);

    st_atpg.function = ATT_PROGRESS_END;
    AttachmentProgress(st_atpg);
    return status;
  }

  PWSAttfileV3 *imp3 = dynamic_cast<PWSAttfileV3 *>(imp);
  try { // exception thrown on write error
    // Open duplicate attachment file
    status = imp3->Open(GetPassKey());

    if (status != PWSRC::SUCCESS) {
      out3->Close();
      delete out3;
      delete imp3;

      pws_os::DeleteAFile(tempfilename);
      pws_os::DeleteAFile(impfilename);

      st_atpg.function = ATT_PROGRESS_END;
      AttachmentProgress(st_atpg);
      return status;
    }

    st_atpg.function = ATT_PROGRESS_START;
    LoadAString(st_atpg.function_text, IDSC_ATT_APPEND_IMP);
    st_atpg.value = 0;
    AttachmentProgress(st_atpg);
    st_atpg.function_text.clear();

    // Process all imported attachments and append to output file
    bool go(true);
    ATRecord atr;
    while (go && imp != NULL) {
      atr.Clear();
      status = imp3->ReadAttmntRecordPreData(atr);
      switch (status) {
        case PWSRC::FAILURE:
        {
          // Show a useful (?) error message - better than
          // silently losing data (but not by much)
          // Best if title intact. What to do if not?
          if (m_pReporter != NULL) {
            stringT cs_msg, cs_caption;
            LoadAString(cs_caption, IDSC_READ_ERROR);
            Format(cs_msg, IDSC_ENCODING_PROBLEM, _T("???"));
            cs_msg = cs_caption + _S(": ") + cs_caption;
            (*m_pReporter)(cs_msg);
          }
          break;
        }
        case PWSRC::SUCCESS:
        {
          unsigned char readtype(0);
          unsigned char *pCmpData;
          unsigned int uiCmpLen, count(0);
          int status_r, status_w;

          st_atpg.function = ATT_PROGRESS_PROCESSFILE;
          st_atpg.atr = atr;
          st_atpg.value = 0;
          AttachmentProgress(st_atpg);

          // Write out pre-data
          out3->WriteAttmntRecordPreData(atr);
          status_w = PWSRC::SUCCESS;

          do {
            // Read in data records
            status_r = imp3->ReadAttmntRecordData(pCmpData, uiCmpLen, readtype, false);

            // Write them back out
            if (status_r == PWSRC::SUCCESS)
              status_w = out3->WriteAttmntRecordData(pCmpData, uiCmpLen, readtype);

            // tidy up
            trashMemory(pCmpData, uiCmpLen);
            delete [] pCmpData;
            pCmpData = NULL;

            // Update progress dialog
            count += atr.blksize;
            st_atpg.value = (int)((count * 1.0E02) / atr.uncsize);
            AttachmentProgress(st_atpg);
          } while(status_r == PWSRC::SUCCESS &&
                  status_w == PWSRC::SUCCESS &&
                  readtype != PWSAttfileV3::ATTMT_LASTDATA);

          // Write out post-data
          imp3->ReadAttmntRecordPostData(atr);
          out3->WriteAttmntRecordPostData(atr);
          vATRImports.push_back(atr);

          st_atpg.function = ATT_PROGRESS_PROCESSFILE;
          st_atpg.value = 100;
          AttachmentProgress(st_atpg);
          break;
        }
        case PWSRC::END_OF_FILE:
          go = false;
          break;
      } // switch
    };
  } catch (...) {
    out3->Close();
    delete out3;
    imp3->Close();
    delete imp3;

    pws_os::DeleteAFile(tempfilename);
    pws_os::DeleteAFile(impfilename);

    st_atpg.function = ATT_PROGRESS_END;
    AttachmentProgress(st_atpg);
    return PWSRC::FAILURE;
  }

  // Now close output file
  out3->Close();
  delete out3;

  // Now close temporary import file
  imp3->Close();
  delete imp3;

  // Delete temporary duplicates file
  pws_os::DeleteAFile(impfilename);

  // Remove change flag now that records have been written
  std::pair<UAMMiter, UAMMiter> uuidairpair;
  for (size_t i = 0; i < vATRWritten.size(); i++) {
    if ((vATRWritten[i].uiflags & ATT_ATTACHMENT_FLGCHGD) == 0)
      continue;

    uuidairpair = m_MM_entry_uuid_atr.equal_range(vATRWritten[i].entry_uuid);
    // Now find this specific attachment record
    for (UAMMiter iter = uuidairpair.first; iter != uuidairpair.second; ++iter) {
      if (memcmp(iter->second.attmt_uuid, vATRWritten[i].attmt_uuid, sizeof(uuid_array_t)) == 0) {
        iter->second.uiflags &= ~ATT_ATTACHMENT_FLGCHGD;
        break;
      }
    }
  }

  // Now update main multimap with the imported attachments
  for (size_t i = 0; i < vATRImports.size(); i++) {
    st_UUID st_uuid(vATRImports[i].entry_uuid);
    m_MM_entry_uuid_atr.insert(make_pair(st_uuid, vATRImports[i]));
  }

  st_atpg.function = ATT_PROGRESS_END;
  AttachmentProgress(st_atpg);

  status = SaveAttachmentFile(tempfilename);
  return status;
}
#endif

struct get_att_uuid {
  get_att_uuid(UUIDAVector &vatt_uuid)
  :  m_vatt_uuid(vatt_uuid)
  {}

  void operator()(std::pair<const st_UUID, ATRecord> const& p) const {
    st_UUID st(p.second.attmt_uuid);
    m_vatt_uuid.push_back(st);
  }

private:
  UUIDAVector &m_vatt_uuid;
};

UUIDAVector PWScore::GetAttachmentUUIDs()
{
  UUIDAVector vatt_uuid;
  get_att_uuid gatt_uuid(vatt_uuid);

  for_each(m_MM_entry_uuid_atr.begin(), m_MM_entry_uuid_atr.end(), gatt_uuid);
  return vatt_uuid;
}

bool PWScore::AttMatches(const ATRecordEx &atrex, const ATFVector &atfv)
{
  // Tests are only OR - not AND
  int iMatch(0);
  size_t num_tests = atfv.size();
  for (size_t i = 0; i < num_tests; i++) {
    if (atfv[i].set != 0) {
      if (AttMatches(atrex, atfv[i].object, atfv[i].function, atfv[i].value))
        return true;  // Passed - get out now
      else
        iMatch--;     // Failed test - never know, another might be ok
    }
  }
  // If it passed a test - already returned true
  // If no tests - would have dropped through and iMatch == 0
  // So if iMatch < 0, it didn't pass any user tests
  return (iMatch == 0);
}

bool PWScore::AttMatches(const ATRecordEx &atrex, const int &iObject, const int &iFunction,
                         const stringT &value) const
{
  ASSERT(iFunction != 0); // must be positive or negative!

  StringX csObject;
  switch(iObject) {
    case ATTGROUP:
      csObject = atrex.sxGroup;
      break;
    case ATTTITLE:
      csObject = atrex.sxTitle;
      break;
    case ATTUSER:
      csObject = atrex.sxUser;
      break;
    case ATTGROUPTITLE:
      csObject = atrex.sxGroup + TCHAR('.') + atrex.sxTitle;
      break;
    case ATTPATH:
      csObject = atrex.atr.path;
      break;
    case ATTFILENAME:
      csObject = atrex.atr.filename;
      break;
    case ATTDESCRIPTION:
      csObject = atrex.atr.description;
      break;
    default:
      ASSERT(0);
  }

  const bool bValue = !csObject.empty();
  if (iFunction == PWSMatch::MR_PRESENT || iFunction == PWSMatch::MR_NOTPRESENT) {
    return PWSMatch::Match(bValue, iFunction);
  }

  if (!bValue) // String empty - always return false for other comparisons
    return false;
  else
    return PWSMatch::Match(value.c_str(), csObject, iFunction);
}
