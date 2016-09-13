/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// SelectAttachment.cpp : implementation file
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "DboxMain.h"
#include "SelectAttachment.h"
#include "ViewAttachmentDlg.h"
#include "GeneralMsgBox.h"
#include "Windowsdefs.h"

#include "afxdialogex.h"

// CSelectAttachment dialog

IMPLEMENT_DYNAMIC(CSelectAttachment, CPWDialog)

CSelectAttachment::CSelectAttachment(CWnd *pParent, pws_os::CUUID *patt_uuid,
  bool *pbOrphaned)
	: CPWDialog(IDD_VIEWAVAILABLEATTACHMENTS, pParent), m_patt_uuid(patt_uuid),
  m_pbOrphaned(pbOrphaned)
{
  // Initialise selected attachment
  *m_patt_uuid = pws_os::CUUID::NullUUID();
  *m_pbOrphaned = false;

  m_iSortedColumn = 0;
  m_bSortAscending = FALSE;
}

CSelectAttachment::~CSelectAttachment()
{
}

void CSelectAttachment::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_ORPHANEDATTACHMENTLIST, m_lcAttachments);
}

BEGIN_MESSAGE_MAP(CSelectAttachment, CPWDialog)
  ON_BN_CLICKED(IDOK, OnSelect)
  ON_NOTIFY(NM_RCLICK, IDC_ORPHANEDATTACHMENTLIST, OnAttachmentRightClick)
  ON_NOTIFY(NM_DBLCLK, IDC_ORPHANEDATTACHMENTLIST, OnAttachmentDoubleClick)
  ON_NOTIFY(HDN_ITEMCLICK, 0, OnHeaderClicked)
  ON_COMMAND(ID_MENUITEM_VIEWATTACHMENT, OnViewAttachment)
  ON_COMMAND(ID_MENUITEM_LISTATTENTRIES, OnListAttEntries)
END_MESSAGE_MAP()

// CSelectAttachment message handlers

BOOL CSelectAttachment::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  CString cs_text;

  // Add columns
  cs_text.LoadString(IDS_NUMREFS);
  m_lcAttachments.InsertColumn(LCATT_NUM, cs_text);
  cs_text.LoadString(IDS_FILETITLE);
  m_lcAttachments.InsertColumn(LCATT_TITLE, cs_text);
  cs_text.LoadString(IDS_FILENAME);
  m_lcAttachments.InsertColumn(LCATT_NAME, cs_text);
  cs_text.LoadString(IDS_FILEPATH);
  m_lcAttachments.InsertColumn(LCATT_PATH, cs_text);
  cs_text.LoadString(IDS_FILEMEDIATYPE);
  m_lcAttachments.InsertColumn(LCATT_MEDIA, cs_text);
  cs_text.LoadString(IDS_FILESIZE);
  m_lcAttachments.InsertColumn(LCATT_SIZE, cs_text, LVCFMT_RIGHT);
  cs_text.LoadString(IDS_CTIME);
  m_lcAttachments.InsertColumn(LCATT_CTIME, cs_text);
  cs_text.LoadString(IDS_FILECTIME);
  m_lcAttachments.InsertColumn(LCATT_FILECTIME, cs_text);
  cs_text.LoadString(IDS_FILEMTIME);
  m_lcAttachments.InsertColumn(LCATT_FILEMTIME, cs_text);
  cs_text.LoadString(IDS_FILEATIME);
  m_lcAttachments.InsertColumn(LCATT_FILEATIME, cs_text);

  DWORD dwStyle = m_lcAttachments.GetExtendedStyle();
  dwStyle |= (LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
  m_lcAttachments.SetExtendedStyle(dwStyle);

  PWScore *pcore = (PWScore *)GetMainDlg()->GetCore();

  // Get details of non-Orphaned attachments
  for (auto attPos = pcore->GetAttIter(); attPos != pcore->GetAttEndIter();
            attPos++) {
    CItemAtt &att = pcore->GetAtt(attPos);

    st_att statt;
    statt.bOrphaned = att.IsOrphaned();
    statt.bToBePurged = att.IsToBePurged();
    statt.att_uuid = att.GetUUID();
    statt.numreferenced = pcore->GetNumReferences(statt.att_uuid);
    statt.size = att.GetContentLength();
    statt.sxFileTitle = att.GetTitle();
    statt.sxFileName = att.GetFileName();
    statt.sxFilePath = att.GetFilePath();
    statt.sxFileMediaType = att.GetMediaType();
    att.GetFileCTime(statt.tFileCTime);
    att.GetFileMTime(statt.tFileMTime);
    att.GetFileATime(statt.tFileATime);

    m_vAttDetails.push_back(statt);
  }

  // Now add in all attachments. ItemData == distance from beginning of attachment list
  int nPos = 0;
  for (size_t i = 0; i < m_vAttDetails.size(); i++) {
    CString csText;
    csText.Format(L"%d", m_vAttDetails[i].numreferenced);
    nPos = m_lcAttachments.InsertItem(nPos, csText);
    m_lcAttachments.SetItemText(nPos, LCATT_TITLE, m_vAttDetails[i].sxFileTitle.c_str());
    m_lcAttachments.SetItemText(nPos, LCATT_NAME, m_vAttDetails[i].sxFileName.c_str());
    m_lcAttachments.SetItemText(nPos, LCATT_PATH, m_vAttDetails[i].sxFilePath.c_str());
    m_lcAttachments.SetItemText(nPos, LCATT_MEDIA, m_vAttDetails[i].sxFileMediaType.c_str());
    int iSizeKB = (m_vAttDetails[i].size + 1023) >> 10;
    csText.Format(L"%d KB", iSizeKB);
    m_lcAttachments.SetItemText(nPos, LCATT_SIZE, csText);
    csText = PWSUtil::ConvertToDateTimeString(m_vAttDetails[i].tCTime, PWSUtil::TMC_LOCALE).c_str();
    m_lcAttachments.SetItemText(nPos, LCATT_CTIME, csText);
    csText = PWSUtil::ConvertToDateTimeString(m_vAttDetails[i].tFileMTime, PWSUtil::TMC_LOCALE).c_str();
    m_lcAttachments.SetItemText(nPos, LCATT_FILECTIME, csText);
    csText = PWSUtil::ConvertToDateTimeString(m_vAttDetails[i].tFileMTime, PWSUtil::TMC_LOCALE).c_str();
    m_lcAttachments.SetItemText(nPos, LCATT_FILEMTIME, csText);
    csText = PWSUtil::ConvertToDateTimeString(m_vAttDetails[i].tFileATime, PWSUtil::TMC_LOCALE).c_str();
    m_lcAttachments.SetItemText(nPos, LCATT_FILEATIME, csText);
    m_lcAttachments.SetItemData(nPos, i);
    nPos++;
  }

  // First column is special if an image
  m_lcAttachments.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

  // Resize columns
  int nCols = m_lcAttachments.GetHeaderCtrl()->GetItemCount();

  // Set column widths
  for (int i = 0; i < nCols; i++) {
    m_lcAttachments.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int nColumnWidth = m_lcAttachments.GetColumnWidth(i);
    m_lcAttachments.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int nHeaderWidth = m_lcAttachments.GetColumnWidth(i);
    m_lcAttachments.SetColumnWidth(i, std::max(nColumnWidth, nHeaderWidth));
  }
  m_lcAttachments.SetColumnWidth(nCols - 1, LVSCW_AUTOSIZE_USEHEADER);

  return TRUE;
}

void CSelectAttachment::OnAttachmentRightClick(NMHDR * /*pNotifyStruct*/, LRESULT *pLResult)
{
  *pLResult = 0; // Perform default processing on return
  POSITION pos = m_lcAttachments.GetFirstSelectedItemPosition();

  if (pos == NULL)
    return;

  int nItem = m_lcAttachments.GetNextSelectedItem(pos);
  int iattpos = m_lcAttachments.GetItemData(nItem);

  if (m_vAttDetails[iattpos].numreferenced == 0 &&
      m_vAttDetails[iattpos].sxFileMediaType.substr(0,5) != L"image") {
    // No point in doing a popup menu if orphaned and not an image!
    return;
  }

  CPoint msg_pt = ::GetMessagePos();
  CMenu menu;
  int ipopup = IDR_POPATTACHMENTS;

  if (menu.LoadMenu(ipopup)) {
    MENUINFO minfo = { 0 };
    minfo.cbSize = sizeof(MENUINFO);
    minfo.fMask = MIM_MENUDATA;
    minfo.dwMenuData = ipopup;
    BOOL brc = menu.SetMenuInfo(&minfo);
    ASSERT(brc != 0);

    CMenu *pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);

    // Orphaned - don't give option to list entries
    if (m_vAttDetails[iattpos].numreferenced == 0) {
      pPopup->RemoveMenu(ID_MENUITEM_LISTATTENTRIES, MF_BYCOMMAND);
    }

    // Not an image - don't give option to view attachment
    if (m_vAttDetails[iattpos].sxFileMediaType.substr(0, 5) != L"image") {
      pPopup->RemoveMenu(ID_MENUITEM_VIEWATTACHMENT, MF_BYCOMMAND);
    }

    // As a shared popup menu - delete unwanted enty
    pPopup->RemoveMenu(ID_MENUITEM_MARKFORPURGING, MF_BYCOMMAND);

    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, msg_pt.x, msg_pt.y, this);
  }
}

void CSelectAttachment::OnAttachmentDoubleClick(NMHDR *pNMHDR, LRESULT *pLResult)
{
  *pLResult = 0; // Perform default processing on return

  NMITEMACTIVATE *pitem = (NMITEMACTIVATE *)pNMHDR;
  int row = pitem->iItem;

  if (row < 0)
    return;

  OnSelect();
}

void CSelectAttachment::OnListAttEntries()
{
  POSITION pos = m_lcAttachments.GetFirstSelectedItemPosition();

  if (pos == NULL)
    return;

  int nItem = m_lcAttachments.GetNextSelectedItem(pos);
  int iattpos = m_lcAttachments.GetItemData(nItem);

  PWScore *pcore = (PWScore *)GetMainDlg()->GetCore();

  ItemMMap_Range eq = pcore->GetAttRange(m_vAttDetails[iattpos].att_uuid);

  std::vector<st_gtui> vgtui;

  for (ItemMMapConstIter it = eq.first; it != eq.second; ++it) {
    pws_os::CUUID itemUUID = it->second;
    ItemListIter iter = pcore->Find(itemUUID);
    
    st_gtui stgtui;

    CItemData &item = pcore->GetEntry(iter);
    stgtui.sxGroup = item.GetGroup();
    stgtui.sxTitle = item.GetTitle();
    stgtui.sxUser = item.GetUser();
    stgtui.image = GetMainDlg()->GetEntryImage(item);

    vgtui.push_back(stgtui);
  }

  CViewAttachmentEntriesDlg dlg(this, &vgtui);

  dlg.DoModal();
}

void CSelectAttachment::OnViewAttachment()
{
  POSITION pos = m_lcAttachments.GetFirstSelectedItemPosition();

  if (pos == NULL)
    return;

  int nItem = m_lcAttachments.GetNextSelectedItem(pos);
  int iattpos = m_lcAttachments.GetItemData(nItem);

  PWScore *pcore = (PWScore *)GetMainDlg()->GetCore();

  CItemAtt &att = pcore->GetAtt(m_vAttDetails[iattpos].att_uuid);

  // Shouldn't be here if no content
  if (!att.HasContent())
    return;

  // Get media type before we find we can't load it
  if (m_vAttDetails[iattpos].sxFileMediaType.substr(0, 5) != L"image") {
    CGeneralMsgBox gmb;
    CString csMessage(MAKEINTRESOURCE(IDS_NOPREVIEW_AVAILABLE));
    CString csTitle(MAKEINTRESOURCE(IDS_VIEWATTACHMENT));
    gmb.MessageBox(csMessage, csTitle, MB_OK);
    return;
  }

  CViewAttachmentDlg viewdlg(this, &att);

  viewdlg.DoModal();
}

void CSelectAttachment::OnSelect()
{
  int iattpos(-1);
  POSITION pos = m_lcAttachments.GetFirstSelectedItemPosition();

  if (pos != NULL && m_patt_uuid != NULL) {
    int nItem = m_lcAttachments.GetNextSelectedItem(pos);
    iattpos = m_lcAttachments.GetItemData(nItem);
    *m_patt_uuid = m_vAttDetails[iattpos].att_uuid;
    *m_pbOrphaned = m_vAttDetails[iattpos].bOrphaned;
  }

  CPWDialog::EndDialog(iattpos);
}

void CSelectAttachment::OnHeaderClicked(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  HD_NOTIFY *phdn = (HD_NOTIFY *)pNotifyStruct;

  if (phdn->iButton == 0) {
    // User clicked on header using left mouse button
    if (phdn->iItem == m_iSortedColumn)
      m_bSortAscending = !m_bSortAscending;
    else
      m_bSortAscending = TRUE;

    m_iSortedColumn = phdn->iItem;
    m_lcAttachments.SortItems(CompareFunc, (LPARAM)this);

    // Note: WINVER defines the minimum system level for which this is program compiled and
    // NOT the level of system it is running on!
    // In this case, these values are defined in Windows XP and later and supported
    // by V6 of comctl32.dll (supplied with Windows XP) and later.
    // They should be ignored by earlier levels of this dll or .....
    //     we can check the dll version (code available on request)!

#if (WINVER < 0x0501)  // These are already defined for WinXP and later
#define HDF_SORTUP 0x0400
#define HDF_SORTDOWN 0x0200
#endif
    HDITEM HeaderItem;
    HeaderItem.mask = HDI_FORMAT;
    m_lcAttachments.GetHeaderCtrl()->GetItem(m_iSortedColumn, &HeaderItem);
    // Turn off all arrows
    HeaderItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
    // Turn on the correct arrow
    HeaderItem.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
    m_lcAttachments.GetHeaderCtrl()->SetItem(m_iSortedColumn, &HeaderItem);
  }

  *pLResult = 0;
}

int CALLBACK CSelectAttachment::CompareFunc(LPARAM lParam1, LPARAM lParam2,
                                            LPARAM closure)
{
  CSelectAttachment *self = (CSelectAttachment *)closure;

  int nSortColumn = self->m_iSortedColumn;
  const st_att pLHS = self->m_vAttDetails[lParam1];
  const st_att pRHS = self->m_vAttDetails[lParam2];

  int iResult(0);
  switch (nSortColumn) {
  case LCATT_NUM:
    if (pLHS.numreferenced != pRHS.numreferenced)
      iResult = (pLHS.numreferenced < pRHS.numreferenced) ? -1 : 1;
    break;
  case LCATT_TITLE:
    iResult = CompareNoCase(pLHS.sxFileTitle, pRHS.sxFileTitle);
    break;
  case LCATT_NAME:
    iResult = CompareNoCase(pLHS.sxFileName, pRHS.sxFileName);
    break;
  case LCATT_PATH:
    iResult = CompareNoCase(pLHS.sxFilePath, pRHS.sxFilePath);
    break;
  case LCATT_MEDIA:
    iResult = CompareNoCase(pLHS.sxFileMediaType, pRHS.sxFileMediaType);
    break;
  case LCATT_SIZE:
    if (pLHS.size != pRHS.size)
      iResult = (pLHS.size < pRHS.size) ? -1 : 1;
    break;
  case LCATT_CTIME:
    if (pLHS.tCTime != pRHS.tCTime)
      iResult = (pLHS.tCTime < pRHS.tCTime) ? -1 : 1;
    break;
  case LCATT_FILECTIME:
    if (pLHS.tFileCTime != pRHS.tFileCTime)
      iResult = (pLHS.tFileCTime < pRHS.tFileCTime) ? -1 : 1;
    break;
  case LCATT_FILEMTIME:
    if (pLHS.tFileMTime != pRHS.tFileMTime)
      iResult = (pLHS.tFileMTime < pRHS.tFileMTime) ? -1 : 1;
    break;
  case LCATT_FILEATIME:
    if (pLHS.tFileATime != pRHS.tFileATime)
      iResult = (pLHS.tFileATime < pRHS.tFileATime) ? -1 : 1;
    break;
  default:
    ASSERT(FALSE);
  }

  if (!self->m_bSortAscending && iResult != 0)
    iResult *= -1;

  return iResult;
}
