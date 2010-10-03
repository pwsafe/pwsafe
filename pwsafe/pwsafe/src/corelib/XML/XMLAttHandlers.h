/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __XMLATTHANDLERS_H
#define __XMLATTHANDLERS_H

// PWS includes
#include "../attachments.h"
#include "../coredefs.h"
#include "../PWSAttfileV3.h"

#include "XMLDefs.h"  // Required if testing "USE_XML_LIBRARY"

// New imported attachment
#define ATT_ERR_ENTRYNOTFOUND 0x80
#define ATT_ERR_SIZEDIFFERENT 0x40
#define ATT_ERR_DIGESTFAILED  0x20

struct att_entry {

  att_entry()
  : id(0),
  sx_group(_T("")), sx_title(_T("")), sx_user(_T("")),
  sx_ctime(_T("")), sx_atime(_T("")), sx_mtime(_T("")), sx_dtime(_T("")),
  err_flags(0), datalength(0)
  {
    atr.Clear();
    context.Clear();
  }

  att_entry(const att_entry &ate)
    : atr(ate.atr), id(ate.id), iter(ate.iter),
    sx_group(ate.sx_group), sx_title(ate.sx_title), sx_user(ate.sx_user),
    sx_ctime(ate.sx_ctime), sx_atime(ate.sx_atime), sx_mtime(ate.sx_mtime), sx_dtime(ate.sx_dtime),
    err_flags(ate.err_flags), context(ate.context), datalength(ate.datalength)
  {
  }

  att_entry &operator =(const att_entry &ate)
  {
    if (this != &ate) {
      atr = ate.atr;
      id = ate.id;
      iter = ate.iter;

      sx_group = ate.sx_group;
      sx_title = ate.sx_title;
      sx_user = ate.sx_user;

      sx_ctime = ate.sx_ctime;
      sx_atime = ate.sx_atime;
      sx_mtime = ate.sx_mtime;
      sx_dtime = ate.sx_dtime;

      err_flags = ate.err_flags;
      datalength = ate.datalength;

      context = ate.context;
    }
    return *this;
  }

  ATRecord atr;
  StringX sx_group;
  StringX sx_title;
  StringX sx_user;

  StringX sx_ctime;
  StringX sx_atime;
  StringX sx_mtime;
  StringX sx_dtime;

  ItemListIter iter;
  int id;
  BYTE err_flags;
  SHA1 context;
  unsigned long datalength;
};

typedef std::vector<att_entry *> vatt_entries;

class PWScore;
class MultiCommands;
class CReport;

class XMLAttHandlers
{
  // to allow access to protected members
#if   USE_XML_LIBRARY == EXPAT
  friend class EAttXMLProcessor;
#elif USE_XML_LIBRARY == MSXML
  friend class MAttXMLProcessor;
#elif USE_XML_LIBRARY == XERCES
  friend class XAttXMLProcessor;
#endif

public:
  XMLAttHandlers();
  virtual ~XMLAttHandlers();

  void SetVariables(PWScore *pcore, const bool &bValidation,
                    MultiCommands *pmulticmds, PWSAttfile *pimport,
                    CReport *prpt);

  bool getIfErrors() const {return m_bErrors;}
  int getErrorCode() const {return m_iErrorCode;}
  stringT getErrorMessage() const {return m_strErrorMessage;}
  stringT getXMLErrors() const {return m_strXMLErrors;}
  stringT getSkippedList() const {return m_strSkippedList;}

  int getNumEntries() const {return m_numAttachments;}
  int getNumSkipped() const {return m_numAttachmentsSkipped;}

  vatt_entries & getVAtt_Entries() {return m_ventries;}

protected:
  bool ProcessStartElement(const int icurrent_element);
  void ProcessEndElement(const int icurrent_element);

  vatt_entries m_ventries;
  att_entry *cur_entry;

  StringX m_strElemContent;
  stringT m_strErrorMessage;
  stringT m_strXMLErrors;
  stringT m_strSkippedList;

  int m_iErrorCode;
  int m_numAttachments, m_numAttachmentsSkipped;

  bool m_bAttachmentBeingProcessed;
  bool m_bValidation;
  bool m_bErrors;
  unsigned char m_ctype;

private:
  void ValidateImportData(att_entry * &cur_entry);

  // Local variables
  UUIDAVector m_vatt_uuid;
  PWScore *m_pXMLcore;
  MultiCommands *m_pmulticmds;
  PWSAttfile *m_pimport;
  PWSAttfileV3 *m_pimport3;
  CReport *m_prpt;

  int m_whichtime, m_ipwh;
  int m_fieldlen;
  bool m_bIgnoreThisAttachment;
  unsigned char * m_pfield;
  bool m_bfirst;
};

#endif /* __XMLATTHANDLERS_H */
