/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file AttProgressDlg.h
//-----------------------------------------------------------------------------

#pragma once

#include "stdafx.h"
#include "PWDialog.h"
#include "resource.h"

#include "corelib/UUIDGen.h"  // for uuid_array_t
#include "corelib/StringX.h"
#include "corelib/attachments.h"
#include "corelib/PWSAttfile.h"

class DboxMain;
class PWScore;

// CAttProgressDlg dialog

#define ATTPRG_UPDATE_GUI          (WM_APP + 100)
#define ATTPRG_THREAD_ENDED        (WM_APP + 101)

class CAttProgressDlg : public CPWDialog
{
  DECLARE_DYNAMIC(CAttProgressDlg)

public:
  CAttProgressDlg(CWnd* pParent, bool *pbStopVerify = NULL,
  bool *pbAttachmentCancel = NULL);

// Dialog Data
  enum { IDD = IDD_ATTPROGRESS };
  CProgressCtrl m_progress;

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnStopVerify();
  afx_msg void OnCancel();
  LRESULT OnUpdateProgress(WPARAM wparam, LPARAM );
  LRESULT OnThreadFinished(WPARAM , LPARAM );

  DECLARE_MESSAGE_MAP()

private:
  DboxMain *m_pDbx;
  CString m_function, m_owner, m_filename, m_description;
  bool *m_pbStopVerify, *m_pbAttachmentCancel;
};

// Thread parameters
enum {INVALID = -1, READ, WRITE, DUPLICATE, IMPORT_COMPLETE,
                       IMPORT_WRITE, GET};

struct ATThreadParms {
  ATThreadParms()
  : status(0), function(INVALID), pDbx(NULL), pProgressDlg(NULL), pcore(NULL),
  bVerify(false), bCleanup(false), impfilename(_T("")), newfile(_T("")),
  filename(_T("")), num_exported(0), version(PWSAttfile::VCURRENT)

  {
    memset(old_entry_uuid, 0, sizeof(uuid_array_t));
    memset(new_entry_uuid, 0, sizeof(uuid_array_t));
  }

  ATThreadParms(const ATThreadParms &thpms)
    : status(thpms.status), function(thpms.function), pDbx(thpms.pDbx),
    pProgressDlg(thpms.pProgressDlg), pcore(thpms.pcore),
    bVerify(thpms.bVerify), bCleanup(thpms.bCleanup),
    impfilename(thpms.impfilename), newfile(thpms.newfile),
    atr(thpms.atr), filename(thpms.filename), vatf(thpms.vatf),
    vAIRecordExs(thpms.vAIRecordExs), num_exported(thpms.num_exported),
    version(thpms.version)
  {
    memcpy(old_entry_uuid, thpms.old_entry_uuid, sizeof(uuid_array_t));
    memcpy(new_entry_uuid, thpms.new_entry_uuid, sizeof(uuid_array_t));
  }

  ATThreadParms &operator=(const ATThreadParms &thpms)
  {
    if (this != &thpms) {
      status = thpms.status;
      function = thpms.function;
      pDbx = thpms.pDbx;
      pProgressDlg = thpms.pProgressDlg;
      pcore = thpms.pcore;

      bVerify = thpms.bVerify;
      bCleanup = thpms.bCleanup;

      impfilename = thpms.impfilename;

      newfile = thpms.newfile;
      atr = thpms.atr;

      filename = thpms.filename;
      vatf = thpms.vatf;
      vAIRecordExs = thpms.vAIRecordExs;
      num_exported = thpms.num_exported;

      version = thpms.version;

      memcpy(old_entry_uuid, thpms.old_entry_uuid, sizeof(uuid_array_t));
      memcpy(new_entry_uuid, thpms.new_entry_uuid, sizeof(uuid_array_t));
    }
    return *this;
  }

  int function;
  int status;
  DboxMain *pDbx;
  CAttProgressDlg *pProgressDlg;
  PWScore *pcore;

  // Read Attachment file
  bool bVerify;

  // Write Attachment file
  bool bCleanup;

  // Duplicate Attachments
  uuid_array_t old_entry_uuid;
  uuid_array_t new_entry_uuid;

  // Complete XML Import
  stringT impfilename;

  // Get Attachment
  ATRecord atr;
  stringT newfile;

  // Write Attachment XML File
  StringX filename;
  ATFVector vatf;
  ATRExVector vAIRecordExs;
  size_t num_exported;

  // Version used by: WRITE, Duplicate, Complete XML,
  PWSAttfile::VERSION version;
};
