/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// file MainAttachments.cpp
//
// Attachment-related methods of DboxMain
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "PWFileDialog.h"
#include "GeneralMsgBox.h"
#include "ExtractAttachment.h"
#include "ViewAttachments.h"
#include "ExportAttDlg.h"

#include "resource3.h"  // String resources

#include "corelib/attachments.h"
#include "corelib/PWSdirs.h"
#include "corelib/corelib.h"
#include "corelib/return_codes.h"

#include "os/file.h"
#include "os/dir.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool DboxMain::GetNewAttachmentInfo(ATRecord &atr, const bool bGetFileName)
{
  StringX sx_Filename;
  CString cs_text(MAKEINTRESOURCE(IDS_CHOOSEATTACHMENT));
  std::wstring dir = L"";
  wchar_t strPath[MAX_PATH] = {0};

  BOOL brc = SHGetSpecialFolderPath(NULL, strPath, CSIDL_PERSONAL, FALSE);
  ASSERT(brc == TRUE);
  if (brc == TRUE)
    dir = strPath;

  while (1) {
    // Open-type dialog box (with extra description edit control)
    INT_PTR rc2(IDOK);
    StringX sFullFileName = atr.filename;

    if (bGetFileName) {
      CPWFileDialog fd(TRUE,
                       NULL,
                       NULL,
                       OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_DONTADDTORECENT |
                          OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
                       CString(MAKEINTRESOURCE(IDS_FDF_ALL)),
                       this, 0, true);

      fd.m_ofn.lpstrTitle = cs_text;
      if (!dir.empty())
        fd.m_ofn.lpstrInitialDir = dir.c_str();

      rc2 = fd.DoModal();
      if (rc2 == IDOK) {
        // Note: Using fd.GetFileName() for the filename may truncate the extension
        // to 3 characters
        sFullFileName = (LPCWSTR)fd.GetPathName();
        atr.description = fd.GetDescription();
      }
    }

    if (m_inExit) {
      // If U3ExitNow called while in CPWFileDialog,
      // PostQuitMessage makes us return here instead
      // of exiting the app. Try resignalling
      PostQuitMessage(0);
      return false;
    }

    if (rc2 == IDOK) {
      struct _stat stat_buffer;
      int irc = _wstat(sFullFileName.c_str(), &stat_buffer);
      if (irc == 0) {
        CGeneralMsgBox gmb;
        if (stat_buffer.st_size == 0) {
          CString cs_msg(MAKEINTRESOURCE(IDS_NOZEROSIZEEXTRACT)),
              cs_title(MAKEINTRESOURCE(IDS_WILLNOTATTACH));
          gmb.MessageBox(cs_msg, cs_title, MB_OK | MB_ICONSTOP);
        } else {
          if (stat_buffer.st_size > (ATT_MAXSIZE << 20)) {
            CString cs_msg, cs_title(MAKEINTRESOURCE(IDS_WILLNOTATTACH));
            cs_msg.Format(IDS_ATTACHMENTTOOBIG, ATT_MAXSIZE);
            gmb.MessageBox(cs_msg, cs_title, MB_OK | MB_ICONSTOP);
            return false;
          }
          std::wstring sDrive, sDir, sName, sExt;
          pws_os::splitpath(sFullFileName.c_str(), sDrive, sDir, sName, sExt);
          atr.path = StringX(sDrive.c_str()) + StringX(sDir.c_str());
          atr.filename = StringX(sName.c_str()) + StringX(sExt.c_str());
          atr.flags = 0;
          atr.uiflags = 0;
          atr.mtime = stat_buffer.st_mtime;
          atr.atime = stat_buffer.st_atime;
          atr.ctime = stat_buffer.st_ctime;
          atr.dtime = 0;
          atr.uncsize = stat_buffer.st_size;
          atr.cmpsize = 0;
          atr.blksize = 0;
          atr.CRC = 0;
          uuid_array_t new_attmt_uuid;
          CUUIDGen attmt_uuid;
          attmt_uuid.GetUUID(new_attmt_uuid);
          memcpy(atr.attmt_uuid, new_attmt_uuid, sizeof(uuid_array_t));
          memset(atr.entry_uuid, 0, sizeof(atr.entry_uuid));
          memset(atr.odigest, 0, SHA1::HASHLEN);
          memset(atr.cdigest, 0, SHA1::HASHLEN);
          return true;
        }
      }
    } else {
      // Can inly be the user pressed cancel
      return false;
    }
  }
  return false;
}

void DboxMain::OnExtractAttachment()
{
  // Go and retrieve the attachment for the user
  CItemData *pci(NULL);
  if (SelItemOk() == TRUE) {
    pci = getSelectedItem();
    ASSERT(pci != NULL);

    if (pci->IsShortcut())
      pci = GetBaseEntry(pci);
  } else
    return;

  CExtractAttachment dlg(this);

  uuid_array_t entry_uuid;
  pci->GetUUID(entry_uuid);
  ATRVector vATRecords;
  m_core.GetAttachments(entry_uuid, vATRecords);
  dlg.SetAttachments(vATRecords);

  // Now let them decide where to extract it to and the new file name
  // and then do it
  dlg.DoModal();
}

void DboxMain::OnViewAttachments()
{
  CViewAttachments dlg(this);

  ATRExVector vAIRecordExs;
  m_core.GetAllAttachments(vAIRecordExs);
  dlg.SetExAttachments(vAIRecordExs);

  // Now let them decide where to extract it to and the new file name
  // and then do it
  dlg.DoModal();
}

LRESULT DboxMain::OnExportAttachment(WPARAM wParam, LPARAM )
{
  ATRecordEx *pATREx = (ATRecordEx *)wParam;
  if (pATREx->sxTitle.empty()) {
    // Title is mandatory - implies caller didn't fill in necessary fields
    ItemListIter iter = Find(pATREx->atr.entry_uuid);
    if (iter == End())
      return 0L;
    pATREx->sxGroup = iter->second.GetGroup();
    pATREx->sxTitle = iter->second.GetTitle();
    pATREx->sxUser = iter->second.GetUser();
  }
  DoExportAttachmentsXML(pATREx);

  return 0L;
}

LRESULT DboxMain::OnChangeAttachment(WPARAM wParam, LPARAM )
{
  ATRecord *pATR = (ATRecord *)wParam;

  pATR->uiflags |= ATT_ATTACHMENT_FLGCHGD;
  m_core.ChangeAttachment(*pATR);
  WriteAttachmentFile(false);

  return 0L;
}

void DboxMain::OnExportAllAttachments()
{
  DoExportAttachmentsXML();
}

void DboxMain::DoExportAttachmentsXML(ATRecordEx *pATREx)
{
  ATFVector vatf;
  for (int i = 0; i < CAdvancedAttDlg::NUM_TESTS; i++) {
    ATFilter atf;
    vatf.push_back(atf);
  }

  CGeneralMsgBox gmb;
  CExportAttDlg eXML(this, vatf);
  CString cs_text, cs_title, cs_temp;
  size_t num_exported(0);

  INT_PTR rc = eXML.DoModal();

  if (rc == IDOK) {
    CGeneralMsgBox gmb;
    StringX newfile;
    StringX pw(eXML.GetPasskey());
    if (m_core.CheckPasskey(m_core.GetCurFile(), pw) == PWSRC::SUCCESS) {
      // Do the export - SaveAs-type dialog box
      // New filename reflects attachment file name (if only one) or
      // database filename (if all eattachments)
      std::wstring XMLFileName;
      if (pATREx == NULL) {
        XMLFileName = m_core.GetCurFile().c_str();
      } else {
        XMLFileName = pATREx->atr.filename.c_str();
      }
      XMLFileName = PWSUtil::GetNewFileName(XMLFileName.c_str(), L"xml");

      cs_text.LoadString(IDS_NAMEXMLFILE);
      std::wstring dir;
      if (m_core.GetCurFile().empty())
        dir = PWSdirs::GetSafeDir();
      else {
        std::wstring cdrive, cdir, dontCare;
        pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
        dir = cdrive + cdir;
      }

      while (1) {
        CPWFileDialog fd(FALSE,
                         L"xml",
                         XMLFileName.c_str(),
                         OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                            OFN_LONGNAMES | OFN_OVERWRITEPROMPT,
                         CString(MAKEINTRESOURCE(IDS_FDF_X_ALL)),
                         this);

        fd.m_ofn.lpstrTitle = cs_text;

        if (!dir.empty())
          fd.m_ofn.lpstrInitialDir = dir.c_str();

        rc = fd.DoModal();

        if (m_inExit) {
          // If U3ExitNow called while in CPWFileDialog,
          // PostQuitMessage makes us return here instead
          // of exiting the app. Try resignalling
          PostQuitMessage(0);
          return;
        }
        if (rc == IDOK) {
          newfile = fd.GetPathName();
          break;
        } else
          goto exit;
      } // while (1)

      ATRExVector vAIRecordExs;
      if (pATREx != NULL) {
        vAIRecordExs.push_back(*pATREx);
      }

      rc = WriteXMLAttachmentFile(newfile, vatf, vAIRecordExs, num_exported);

      if (rc != PWSRC::SUCCESS) {
        //DisplayFileWriteError(rc, newfile);
        ASSERT(0);
      }
    } else {
      gmb.AfxMessageBox(IDS_BADPASSKEY);
      ::Sleep(3000); // protect against automatic attacks
    }
  }

  cs_text.Format(IDS_EXPORTOK, num_exported);
  cs_title.LoadString(IDS_EXPORTED);
  gmb.MessageBox(cs_text, cs_title, MB_OK | MB_ICONINFORMATION);

exit:
  return;
}

void DboxMain::OnImportAttachments()
{
  // disable in read-only mode and empty database
  if (m_core.IsReadOnly() || m_core.GetNumEntries() == 0)
    return;

  CString cs_title, cs_temp, cs_text;
  cs_title.LoadString(IDS_XMLIMPORTFAILED);

  CGeneralMsgBox gmb;
  // Initialize set
  GTUSet setGTU;
  if (!m_core.GetUniqueGTUValidated() && !m_core.InitialiseGTU(setGTU)) {
    // Database is not unique to start with - tell user to validate it first
    cs_temp.Format(IDS_DBHASDUPLICATES, m_core.GetCurFile().c_str());
    gmb.MessageBox(cs_temp, cs_title, MB_ICONEXCLAMATION);
    return;
  }

  const std::wstring XSDfn(L"pwsafe_att.xsd");
  std::wstring XSDFilename = PWSdirs::GetXMLDir() + XSDfn;

#if USE_XML_LIBRARY == MSXML || USE_XML_LIBRARY == XERCES
  // Expat is a non-validating parser - no use for Schema!
  if (!pws_os::FileExists(XSDFilename)) {
    CGeneralMsgBox gmb;
    cs_temp.Format(IDSC_MISSINGXSD, XSDfn.c_str());
    cs_title.LoadString(IDSC_CANTVALIDATEXML);
    gmb.MessageBox(cs_temp, cs_title, MB_OK | MB_ICONSTOP);
    return;
  }
#endif

  std::wstring dir;
  if (m_core.GetCurFile().empty())
    dir = PWSdirs::GetSafeDir();
  else {
    std::wstring cdrive, cdir, dontCare;
    pws_os::splitpath(m_core.GetCurFile().c_str(), cdrive, cdir, dontCare, dontCare);
    dir = cdrive + cdir;
  }

  CPWFileDialog fd(TRUE,
                   L"xml",
                   NULL,
                   OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES,
                   CString(MAKEINTRESOURCE(IDS_FDF_XML)),
                   this);

  cs_text.LoadString(IDS_PICKXMLFILE);
  fd.m_ofn.lpstrTitle = cs_text;

  if (!dir.empty())
    fd.m_ofn.lpstrInitialDir = dir.c_str();

  INT_PTR rc = fd.DoModal();

  if (m_inExit) {
    // If U3ExitNow called while in CPWFileDialog,
    // PostQuitMessage makes us return here instead
    // of exiting the app. Try resignalling
    PostQuitMessage(0);
    return;
  }

  if (rc == IDOK) {
    std::wstring strXMLErrors, strSkippedList;
    CString XMLFilename = fd.GetPathName();
    int numValidated, numImported, numSkipped;

    CWaitCursor waitCursor;  // This may take a while!

    /* Create report as we go */
    CReport rpt;
    CString cs_text;
    cs_text.LoadString(IDS_RPTIMPORTXML);
    rpt.StartReport(cs_text, m_core.GetCurFile().c_str());
    cs_text.LoadString(IDS_XML);
    cs_temp.Format(IDS_IMPORTFILE, cs_text, XMLFilename);
    rpt.WriteLine((LPCWSTR)cs_temp);
    rpt.WriteLine();
    std::vector<StringX> vgroups;
    Command *pcmd = NULL;

    int xrc = m_core.ImportXMLAttachmentFile(std::wstring(XMLFilename),
                                             XSDFilename.c_str(),
                                             strXMLErrors, strSkippedList,
                                             numValidated, numImported, numSkipped,
                                             rpt, pcmd);
    waitCursor.Restore();  // Restore normal cursor

    std::wstring csErrors(L"");
    switch (xrc) {
      case PWSRC::XML_FAILED_VALIDATION:
        rpt.WriteLine(strXMLErrors.c_str());
        cs_temp.Format(IDS_FAILEDXMLVALIDATE, fd.GetFileName(), L"");
        delete pcmd;
        break;
      case PWSRC::XML_FAILED_IMPORT:
        rpt.WriteLine(strXMLErrors.c_str());
        cs_temp.Format(IDS_XMLERRORS, fd.GetFileName(), L"");
        delete pcmd;
        break;
      case PWSRC::SUCCESS:
      case PWSRC::OK_WITH_ERRORS:
        cs_title.LoadString(rc == PWSRC::SUCCESS ? IDS_COMPLETE : IDS_OKWITHERRORS);
        if (pcmd != NULL)
          Execute(pcmd);

        if (!strXMLErrors.empty() || numSkipped > 0) {
          if (!strXMLErrors.empty()) {
            csErrors = strXMLErrors + L"\n";
          }

          if (!csErrors.empty()) {
            rpt.WriteLine(csErrors.c_str());
          }

          CString cs_renamed(L""), cs_skipped(L"");
          if (numSkipped > 0) {
            cs_skipped.LoadString(IDS_TITLESKIPPED);
            rpt.WriteLine((LPCWSTR)cs_skipped);
            cs_skipped.Format(IDS_XMLIMPORTSKIPPED, numSkipped);
            rpt.WriteLine(strSkippedList.c_str());
            rpt.WriteLine();
          }

          cs_temp.Format(IDS_XMLIMPORTWITHERRORS,
                         fd.GetFileName(), numValidated, numImported,
                         cs_skipped, cs_renamed, L"");

          ChangeOkUpdate();
        } else {
          const CString cs_validate(MAKEINTRESOURCE(numValidated == 1 ? IDSC_ENTRY : IDSC_ENTRIES));
          const CString cs_imported(MAKEINTRESOURCE(numImported == 1 ? IDSC_ENTRY : IDSC_ENTRIES));
          cs_temp.Format(IDS_XMLIMPORTOK, numValidated, cs_validate, numImported, cs_imported);
          ChangeOkUpdate();
        }

        RefreshViews();
        break;
      default:
        ASSERT(0);
    } // switch

    // Finish Report
    rpt.WriteLine((LPCWSTR)cs_temp);
    rpt.EndReport();

    if (rc != PWSRC::SUCCESS || !strXMLErrors.empty())
      gmb.SetStandardIcon(MB_ICONEXCLAMATION);
    else
      gmb.SetStandardIcon(MB_ICONINFORMATION);

    gmb.SetTitle(cs_title);
    gmb.SetMsg(cs_temp);
    gmb.AddButton(IDS_OK, IDS_OK, TRUE, TRUE);
    gmb.AddButton(IDS_VIEWREPORT, IDS_VIEWREPORT);
    INT_PTR rc = gmb.DoModal();
    if (rc == IDS_VIEWREPORT)
      ViewReport(rpt);
  }
}

LRESULT DboxMain::OnExtractAttachment(WPARAM wParam, LPARAM )
{
  ATRecord *pATR = (ATRecord *)wParam;
  DoAttachmentExtraction(*pATR);

  return 0L;
}

void DboxMain::DoAttachmentExtraction(const ATRecord &atr)
{
  CGeneralMsgBox gmb;
  CString cs_msg, cs_title, csFilter;
  StringX sxFile;
  std::wstring sDrv, sDir, sFname, sExt, sxFilter, wsNewName(L"");

  GetAttachmentStatus ga_status(OK);

  // If user mandated that a secure delete program is there, first check that they have
  // specified one and then check it exists on this system.
  if ((atr.flags & ATT_ERASEPGMEXISTS) == ATT_ERASEPGMEXISTS) {
    StringX sxErasePgm = PWSprefs::GetInstance()->GetPref(PWSprefs::EraseProgram);
    if (sxErasePgm.empty() || !pws_os::FileExists(sxErasePgm.c_str())) {
      cs_msg.LoadString(IDS_NOERASEPGM);
      cs_title.LoadString(IDS_WILLNOTEXTRACT);
      gmb.MessageBox(cs_msg, cs_title, MB_OK | MB_ICONSTOP);
      return;
    }
  }

  sxFile = atr.path + atr.filename;
  pws_os::splitpath(sxFile.c_str(), sDrv, sDir, sFname, sExt);
  bool bNoExtn = sExt.empty();
  cs_title.LoadString(IDS_EXTRACTTITLE);

  if (bNoExtn) {
    csFilter.LoadString(IDS_FDF_ALL);
  } else {
    csFilter.Format(IDS_ATTACHMENT_EXTN, sExt.c_str(), sExt.c_str());
  }

  while (1) {
    CPWFileDialog fd(FALSE,
                     bNoExtn ? NULL : sExt.substr(1).c_str(),  // skip leading '.'
                     sxFile.c_str(),
                     OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                        OFN_LONGNAMES | OFN_OVERWRITEPROMPT,
                     csFilter,
                     this);
    fd.m_ofn.lpstrTitle = cs_title;
    INT_PTR rc = fd.DoModal();
    if (ExitRequested()) {
      // If U3ExitNow called while in CPWFileDialog,
      // PostQuitMessage makes us return here instead
      // of exiting the app. Try resignalling
      ::PostQuitMessage(0);
      return;
    }

    if (rc == IDOK) {
      wsNewName = fd.GetPathName();
      // If user mandated that it can only be extracted to a removeable device
      // check that they have selected such a device
      if ((atr.flags & ATT_EXTRACTTOREMOVEABLE) == ATT_EXTRACTTOREMOVEABLE) {
        std::wstring wsNewDrive, wsNewDir, wsNewFileName, wsNewExt;

        pws_os::splitpath(wsNewName, wsNewDrive, wsNewDir, wsNewFileName, wsNewExt);
        wsNewDrive += L"\\";

        UINT uiDT = GetDriveType(wsNewDrive.c_str());
        // Do not allow unless to a removeable device
        if (uiDT != DRIVE_REMOVABLE && uiDT != DRIVE_RAMDISK) {
          ga_status = BADTARGETDEVICE;
          goto exit;
        }
      } else {
        break;
      }
    } else
      return;
  }

  if (wsNewName.empty())
    return;

  // Invoke thread
  ga_status = (GetAttachmentStatus)GetAttachment(wsNewName, atr);

exit:
  UINT uiTitle(0), uiMSG(0);
  int flags(MB_OK | MB_ICONEXCLAMATION);
  switch (ga_status) {
    case OK:
      uiTitle = IDS_EXTRACTED;
      uiMSG = IDS_COMPLETEDOK;
      flags = MB_OK | MB_ICONINFORMATION;
      break;
    case BADTARGETDEVICE:
      uiTitle = IDS_WILLNOTEXTRACT;
      uiMSG = IDS_NOTREMOVEABLE;
      break;
    case CANTCREATEFILE:
      uiTitle =IDS_WILLNOTEXTRACT;
      uiMSG = IDS_CANTCREATEFILE;
      break;
    case CANTFINDATTACHMENT:
      uiTitle = IDS_WILLNOTEXTRACT;
      uiMSG = IDS_CANTGETATTACHMENT;
      break;
    case BADCRCDIGEST:
      uiTitle = IDS_CRCHASHERROR;
      uiMSG = IDS_CONTINUEEXTRACT;
      break;
    case BADATTACHMENTWRITE:
      uiTitle = IDS_EXTRACTED;
      uiMSG = IDS_BADATTACHMENTWRITE;
      break;
    default:
      break;
  }

  if (uiTitle != 0) {
    cs_title.LoadString(uiTitle);
    cs_msg.LoadString(uiMSG);
    gmb.MessageBox(cs_msg, cs_title, flags);
  }
}

int DboxMain::XGetAttachment(const stringT &newfile, const ATRecord &atr)
{
  // Thread version!
  unsigned char *pUncData(NULL);
  HANDLE hFile(INVALID_HANDLE_VALUE);
  unsigned char odigest[SHA1::HASHLEN];
  unsigned long CRC;
  int status;
  unsigned char readtype;
  size_t unclength;
  size_t uncsize(0);
  SHA1 context;
  BOOL brc;

  GetAttachmentStatus ga_status(OK);

  // Open the file
  hFile = CreateFile(newfile.c_str(), GENERIC_WRITE,
                         0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hFile == INVALID_HANDLE_VALUE) {
    ga_status = CANTCREATEFILE;
    goto exit;
  }

  // Long job
  BeginWaitCursor();

  PWSUtil::Get_CRC_Incremental_Init();

  // Open Attachment database
  status = m_core.GetAttachment(atr, OPENFILE, pUncData, unclength, readtype);

  if (status != PWSRC::SUCCESS) {
    // Error message to user
    ga_status = CANTFINDATTACHMENT;
    EndWaitCursor();
    goto exit;
  }

  // GETPRE - tells GetAttachment to search for out record and come back
  // when it has it or can't find it
  status = m_core.GetAttachment(atr, GETPRE, pUncData, unclength, readtype);

  if (status != PWSRC::SUCCESS) {
    // Error message to user - exit will do te CLOSEFILE
    ga_status = CANTFINDATTACHMENT;
    EndWaitCursor();
    goto exit;
  }

  // We now have it - go get all the data
  do {
    status = m_core.GetAttachment(atr, GETDATA, pUncData, unclength, readtype);
    uncsize += unclength;

    // Now update CRC and SHA1 hash of data retrieved
    PWSUtil::Get_CRC_Incremental_Update(pUncData, unclength);
    context.Update(pUncData, unclength);

    // Write out the data
    DWORD numwritten(0);
    brc = WriteFile(hFile, pUncData, unclength, &numwritten, NULL);
    ASSERT(numwritten == unclength);

    if (pUncData != NULL) {
      trashMemory(pUncData, unclength);
      delete [] pUncData;
      pUncData = NULL;
    }

    if (brc == 0 || (unsigned int)numwritten != unclength) {
      ga_status = BADATTACHMENTWRITE;
      EndWaitCursor();
      goto exit;
    }
  } while (status == PWSRC:: SUCCESS && readtype != PWSAttfile::ATTMT_LASTDATA);

  if (status != PWSRC::SUCCESS || readtype != PWSAttfile::ATTMT_LASTDATA) {
    // Error message to user
    ga_status = BADDATA;
    EndWaitCursor();
    goto exit;
  }

  // Get the final data to check CRC and digest
  status = m_core.GetAttachment(atr, GETPOST, pUncData, unclength, readtype);
  CRC = PWSUtil::Get_CRC_Incremental_Final();
  context.Final(odigest);

  EndWaitCursor();

  if (CRC != atr.CRC ||
      memcmp(odigest, atr.odigest, SHA1::HASHLEN) != 0) {
    // Need  our own CGeneralMsgBox as may use one defined above at end and
    // it can only be used once.
    // CRC and/or SHA1 Hash mis-match, ask user whether to continue anyway
    ga_status = BADCRCDIGEST;
  }

  ASSERT(uncsize == atr.uncsize);

  // Set times back to original
  FILETIME ct, at, mt;
  LONGLONG LL;  // Note that LONGLONG is a 64-bit integer value

  LL = Int32x32To64(atr.ctime, 10000000) + 116444736000000000;
  ct.dwLowDateTime = (DWORD)LL;
  ct.dwHighDateTime = (DWORD)(LL >> 32);

  LL = Int32x32To64(atr.atime, 10000000) + 116444736000000000;
  at.dwLowDateTime = (DWORD)LL;
  at.dwHighDateTime = (DWORD)(LL >> 32);

  LL = Int32x32To64(atr.mtime, 10000000) + 116444736000000000;
  mt.dwLowDateTime = (DWORD)LL;
  mt.dwHighDateTime = (DWORD)(LL >> 32);

  //write the new creation, accessed, and last written time
  SetFileTime(hFile, &ct, &at, &mt);
  CloseHandle(hFile);
  hFile = INVALID_HANDLE_VALUE;

exit:
  if (hFile != INVALID_HANDLE_VALUE)
    CloseHandle(hFile);

  // Close attachment database
  m_core.GetAttachment(atr, CLOSEFILE, pUncData, unclength, readtype);
  return (int)ga_status;
}

bool DboxMain::AnyAttachments(HTREEITEM hItem)
{
  CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(hItem);
  if (pci != NULL) {
    // Leaf
    uuid_array_t entry_uuid;
    pci->GetUUID(entry_uuid);
    return (m_core.HasAttachments(entry_uuid) > 0);
  }

  // Must be a group - Walk the Tree from here to determine if any entries have attachments
  while ((hItem = m_ctlItemTree.GetNextTreeItem(hItem)) != NULL) {
    if (!m_ctlItemTree.ItemHasChildren(hItem)) {
      CItemData *pci = (CItemData *)m_ctlItemTree.GetItemData(hItem);
      if (pci != NULL) {  // NULL if there's an empty group [bug #1633516]
        uuid_array_t entry_uuid;
        pci->GetUUID(entry_uuid);
        if (m_core.HasAttachments(entry_uuid) > 0)
          return true;
      }
    }
  }
  return false;
}

int DboxMain::AttachmentProgress(const ATTProgress &st_atpg)
{
  int rc = 0;
  if (m_pProgressDlg != NULL) {
    m_pProgressDlg->SendMessage(ATTPRG_UPDATE_GUI, (WPARAM)&st_atpg, 0);

    if (m_bStopVerify)
      rc = 1;
    if (m_bAttachmentCancel)
      rc += 2;
  }
  return rc;
}

int DboxMain::ReadAttachmentFile(bool bVerify)
{
  if (m_bNoAttachments)
    return PWSRC::SUCCESS;

  if (m_core.GetCurFile().empty()) {
    return PWSRC::CANT_OPEN_FILE;
  }

  // Generate attachment file names from database name
  stringT attmt_file, drv, dir, name, ext;

  pws_os::splitpath(m_core.GetCurFile().c_str(), drv, dir, name, ext);
  attmt_file = drv + dir + name + stringT(ATT_DEFAULT_ATTMT_SUFFIX);

  // If attachment file doesn't exist - OK
  if (!pws_os::FileExists(attmt_file)) {
    pws_os::Trace(_T("No attachment file exists.\n"));
    return PWSRC::SUCCESS;
  }

  ATThreadParms *pthdpms = new ATThreadParms;
  pthdpms->function = READ;
  pthdpms->bVerify = bVerify;

  int status = DoAttachmentThread(pthdpms);

  return status;
}

int DboxMain::WriteAttachmentFile(const bool bCleanup,
                                  PWSAttfile::VERSION version)
{
  if (m_bNoAttachments)
    return PWSRC::SUCCESS;

  ATThreadParms *pthdpms = new ATThreadParms;
  pthdpms->function = WRITE;
  pthdpms->bCleanup = bCleanup;
  pthdpms->version = version;

  int status = DoAttachmentThread(pthdpms);

  return status;
}

int DboxMain::DuplicateAttachments(const uuid_array_t &old_entry_uuid,
                                   const uuid_array_t &new_entry_uuid,
                                   PWSAttfile::VERSION version)
{
  if (m_bNoAttachments)
    return PWSRC::SUCCESS;

  ATThreadParms *pthdpms = new ATThreadParms;
  pthdpms->function = DUPLICATE;
  memcpy(pthdpms->old_entry_uuid, old_entry_uuid, sizeof(uuid_array_t));
  memcpy(pthdpms->new_entry_uuid, new_entry_uuid, sizeof(uuid_array_t));
  pthdpms->version = version;

  int status = DoAttachmentThread(pthdpms);

  return status;
}

int DboxMain::WriteXMLAttachmentFile(const StringX &filename, ATFVector &vatf,
                                     ATRExVector &vAIRecordExs, size_t &num_exported)
{
  if (m_bNoAttachments)
    return PWSRC::SUCCESS;

  ATThreadParms *pthdpms = new ATThreadParms;
  pthdpms->function = IMPORT_WRITE;
  pthdpms->filename = filename;
  pthdpms->vatf = vatf;
  pthdpms->vAIRecordExs = vAIRecordExs;
  pthdpms->num_exported = num_exported;

  int status = DoAttachmentThread(pthdpms);

  num_exported = pthdpms->num_exported;
  return status;
}

int DboxMain::GetAttachment(const stringT &newfile, const ATRecord &atr)
{
  if (m_bNoAttachments)
    return PWSRC::SUCCESS;

  ATThreadParms *pthdpms = new ATThreadParms;
  pthdpms->function = GET;
  pthdpms->atr = atr;
  pthdpms->newfile = newfile;

  int status = DoAttachmentThread(pthdpms);

  return status;
}

int DboxMain::CompleteImportFile(const stringT &filename,
                                 PWSAttfile::VERSION version)
{
  if (m_bNoAttachments)
    return PWSRC::SUCCESS;

  ATThreadParms *pthdpms = new ATThreadParms;
  pthdpms->function = IMPORT_COMPLETE;
  pthdpms->filename = filename.c_str();
  pthdpms->version = version;

  int status = DoAttachmentThread(pthdpms);

  return status;
}

static UINT AttachmentThread(LPVOID pParam)
{
  ATThreadParms *pthdpms = (ATThreadParms *)pParam;

  int status(PWSRC::SUCCESS);
  switch (pthdpms->function) {
    case READ:
      status = pthdpms->pcore->ReadAttachmentFile(pthdpms->bVerify);
      break;
    case WRITE:
      status = pthdpms->pcore->WriteAttachmentFile(pthdpms->bCleanup, pthdpms->version);
      break;
    case DUPLICATE:
      status = pthdpms->pcore->DuplicateAttachments(pthdpms->old_entry_uuid,
                                                    pthdpms->new_entry_uuid,
                                                    pthdpms->version);
      break;
    case IMPORT_COMPLETE:
      status = pthdpms->pcore->CompleteImportFile(pthdpms->impfilename,
                                                  pthdpms->version);
      break;
    case IMPORT_WRITE:
      status = pthdpms->pcore->WriteXMLAttachmentFile(pthdpms->filename, pthdpms->vatf,
                                                      pthdpms->vAIRecordExs,
                                                      pthdpms->num_exported);
      break;
    case GET:
      status = pthdpms->pDbx->XGetAttachment(pthdpms->newfile, pthdpms->atr);
      break;
    default:
      ASSERT(0);
  }

  // Set the thread return code for caller
  pthdpms->status = status;
  // Tell DboxMain that thread has ended
  if (pthdpms->pDbx != NULL)
    pthdpms->pDbx->PostMessage(PWS_MSG_ATTACHMENT_THREAD_ENDED, (WPARAM)pthdpms, 0);

  return 0;
}

int DboxMain::DoAttachmentThread(ATThreadParms *pthdpms)
{
  pthdpms->pDbx = this;
  pthdpms->pcore = &m_core;

  m_bStopVerify = m_bAttachmentCancel = false;
  m_pProgressDlg = new CAttProgressDlg(this,
                 pthdpms->function == READ ? &m_bStopVerify : NULL,
                 pthdpms->function == READ ? &m_bAttachmentCancel : NULL);

  pthdpms->pProgressDlg = m_pProgressDlg;

  m_AttThread = AfxBeginThread(AttachmentThread, pthdpms, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);

  if (m_AttThread == NULL) {
    pws_os::Trace(_T("Unable to create Attachment thread\n"));
    return PWSRC::FAILURE;
  }

  // Stop automatic deletion and then resume thread
  m_AttThread->m_bAutoDelete = FALSE;
  m_AttThread->ResumeThread();

  // Show the progress dialog
  m_pProgressDlg->DoModal();

  // Only ever gets here when thread ends - even if user presses cancel,
  // it goes back to the thread which will end.
  if (pthdpms->function == READ && m_bAttachmentCancel)
    m_bNoAttachments = true;

  delete m_pProgressDlg;
  m_pProgressDlg = NULL;

  // Now get the attachment info (need to to set return code after deleting parms)
  const int status = pthdpms->status;
  if (status != PWSRC::SUCCESS) {
#ifdef _DEBUG
    CString cs_function;
    switch (pthdpms->function) {
      case READ:
        cs_function = L"READ";
        break;
      case WRITE:
        cs_function = L"WRITE";
        break;
      case DUPLICATE:
        cs_function = L"DUPLICATE";
        break;
      case IMPORT_COMPLETE:
        cs_function = L"IMPORT_COMPLETE";
        break;
      case IMPORT_WRITE:
        cs_function = L"IMPORT_WRITE";
        break;
      case GET:
        cs_function = L"GET";
        break;
      default:
        ASSERT(0);
    }
    pws_os::Trace(_T("Problem processing attachments. Function=%s, rc=%d\n"),
                  cs_function, pthdpms->status);
#endif
    CString cs_msg, cs_title;
    cs_title.LoadString(IDSC_ATT_ERRORS);
    if (pthdpms->status == PWSRC::HEADERS_INVALID)
      cs_msg.LoadString(IDSC_ATT_HDRMISMATCH);
    else
      cs_msg.Format(IDSC_ATT_UNKNOWNERROR, pthdpms->status);

    // Tell user
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(cs_msg, cs_title, MB_OK | MB_ICONEXCLAMATION);
  }

  // Delete thread parameters structure and return
  // Before deleting it - clear pointer to PWScore
  pthdpms->pcore = NULL;
  delete pthdpms;
  pthdpms = NULL;
  return status;
}

LRESULT DboxMain::OnAttachmentThreadEnded(WPARAM wParam, LPARAM )
{
  ATThreadParms *pthdpms = (ATThreadParms *)wParam;

  // Wait for it
  WaitForSingleObject(m_AttThread->m_hThread, INFINITE);

  // Thread ended - send message to progress dialog to end
  pthdpms->pProgressDlg->SendMessage(ATTPRG_THREAD_ENDED);

  // Now tidy up (m_bAutoDelete was set to FALSE)
  delete m_AttThread;
  m_AttThread = NULL;

  return 0L;
}
