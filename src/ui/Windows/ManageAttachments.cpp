/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ManageAttachments.cpp : implementation file
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "DboxMain.h"
#include "ManageAttachments.h"
#include "ViewAttachmentDlg.h"
#include "GeneralMsgBox.h"

#include "afxdialogex.h"

CAttListCtrl::CAttListCtrl() : m_pOldFont(NULL), m_pstkFont(NULL)
{
  m_crWindowText = ::GetSysColor(COLOR_WINDOWTEXT);
  m_crRedText = RGB(168, 0, 0);
}

CAttListCtrl::~CAttListCtrl()
{
  if (m_pstkFont) {
    m_pstkFont->DeleteObject();
    delete m_pstkFont;
  }
}

BEGIN_MESSAGE_MAP(CAttListCtrl, CListCtrl)
  //{{AFX_MSG_MAP(CAttListCtrl)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CAttListCtrl::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  // Draw text red with strikethough if to be purged
  NMLVCUSTOMDRAW *pLVCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNotifyStruct);

  *pLResult = CDRF_DODEFAULT;
  const int iSubItem = pLVCD->iSubItem;
  const int id = (int)pLVCD->nmcd.lItemlParam;

  static bool bchanged_font(false);
  static CDC *pDC = NULL;
  static CManageAttachments *pParent = NULL;
  LOGFONT lf;

  switch (pLVCD->nmcd.dwDrawStage) {
  case CDDS_PREPAINT:
    bchanged_font = false;
    pDC = CDC::FromHandle(pLVCD->nmcd.hdc);

    if (m_pstkFont == NULL) {
      m_pOldFont = GetFont();
      m_pOldFont->GetLogFont(&lf);

      lf.lfStrikeOut = TRUE;
      m_pstkFont = new CFont();
      m_pstkFont->CreateFontIndirect(&lf);
    }

    if (pParent == NULL) {
      pParent = ((CManageAttachments *)GetParent());
    }

    *pLResult = CDRF_NOTIFYITEMDRAW;
    break;
  case CDDS_ITEMPREPAINT:
    *pLResult |= CDRF_NOTIFYSUBITEMDRAW;
    break;
  case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
    *pLResult |= CDRF_NOTIFYPOSTPAINT;
    if (pParent->GetPurgeStatus(id)) {
      bchanged_font = true;
      pLVCD->clrText = RGB(255, 0, 0);
      pLVCD->clrTextBk = GetTextBkColor();
      pDC->SelectObject(m_pstkFont);
      *pLResult |= CDRF_NEWFONT;
    } else {
      pLVCD->clrText = m_crWindowText;
      pLVCD->clrTextBk = GetTextBkColor();
    }
    break;

  case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
    // Sub-item PostPaint - restore old font if any
    if (bchanged_font) {
      bchanged_font = false;
      pDC->SelectObject(m_pOldFont);
      *pLResult |= CDRF_NEWFONT;
    }
    break;

    /*
    case CDDS_PREERASE:
    case CDDS_POSTERASE:
    case CDDS_ITEMPREERASE:
    case CDDS_ITEMPOSTERASE:
    case CDDS_ITEMPOSTPAINT:
    case CDDS_POSTPAINT:
    */
  default:
    break;
  }
}

// CManageAttachments dialog

IMPLEMENT_DYNAMIC(CManageAttachments, CPWDialog)

CManageAttachments::CManageAttachments(CWnd* pParent)
	: CPWDialog(IDD_MANAGEATTACHMENTS, pParent)
{
}

CManageAttachments::~CManageAttachments()
{
}

void CManageAttachments::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_ORPHANEDATTACHMENTLIST, m_lcAttachments);
}

BEGIN_MESSAGE_MAP(CManageAttachments, CPWDialog)
  ON_BN_CLICKED(IDOK, OnPerformAction)
  ON_NOTIFY(NM_RCLICK, IDC_ORPHANEDATTACHMENTLIST, OnAttachmentRightClick)
  ON_COMMAND(ID_MENUITEM_VIEWATTACHMENT, OnViewAttachment)
  ON_COMMAND(ID_MENUITEM_LISTATTENTRIES, OnListAttEntries)
  ON_COMMAND(ID_MENUITEM_MARKFORPURGING, OnPurgeAttachment)
END_MESSAGE_MAP()

// CManageAttachments message handlers

BOOL CManageAttachments::OnInitDialog()
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

  // Get details of attachments
  for (auto attPos = pcore->GetAttIter(); attPos != pcore->GetAttEndIter();
            attPos++) {
    CItemAtt &att = pcore->GetAtt(attPos);

    st_att statt;
    statt.bOrphaned = att.IsOrphaned();
    statt.bToBePurged = statt.bInitalToBePurged = false;
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
    nPos = m_lcAttachments.InsertItem(nPos, csText);  // LCATT_NUM
    m_lcAttachments.SetItemText(nPos, LCATT_TITLE, m_vAttDetails[i].sxFileTitle.c_str());
    m_lcAttachments.SetItemText(nPos, LCATT_NAME, m_vAttDetails[i].sxFileName.c_str());
    m_lcAttachments.SetItemText(nPos, LCATT_PATH, m_vAttDetails[i].sxFilePath.c_str());
    m_lcAttachments.SetItemText(nPos, LCATT_MEDIA, m_vAttDetails[i].sxFileMediaType.c_str());
    int iSizeKB = (m_vAttDetails[i].size + 1023) >> 10;
    csText.Format(L"%d KB", iSizeKB);
    m_lcAttachments.SetItemText(nPos, LCATT_SIZE, csText);
    csText = PWSUtil::ConvertToDateTimeString(m_vAttDetails[i].tCTime, PWSUtil::TMC_LOCALE).c_str();
    m_lcAttachments.SetItemText(nPos, LCATT_CTIME, csText);
    csText = PWSUtil::ConvertToDateTimeString(m_vAttDetails[i].tFileCTime, PWSUtil::TMC_LOCALE).c_str();
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

  // Disable initially
  GetDlgItem(IDOK)->EnableWindow(FALSE);

  return TRUE;
}

void CManageAttachments::OnAttachmentRightClick(NMHDR * /*pNotifyStruct*/, LRESULT *pLResult)
{
  *pLResult = 0; // Perform default processing on return
  POSITION pos = m_lcAttachments.GetFirstSelectedItemPosition();

  if (pos == NULL)
    return;

  int nItem = m_lcAttachments.GetNextSelectedItem(pos);
  int iattpos = m_lcAttachments.GetItemData(nItem);

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

    // Orphaned - disable list entries
    pPopup->EnableMenuItem(ID_MENUITEM_LISTATTENTRIES, MF_BYCOMMAND |
      (m_vAttDetails[iattpos].numreferenced == 0 ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED));

    // Not an image - disable view attachment
    pPopup->EnableMenuItem(ID_MENUITEM_VIEWATTACHMENT, MF_BYCOMMAND |
      (m_vAttDetails[iattpos].sxFileMediaType.substr(0, 5) != L"image" ?
          (MF_DISABLED | MF_GRAYED) : MF_ENABLED));

    // Set the menu item text even if going to be disabled
    CString csMenu(MAKEINTRESOURCE(m_vAttDetails[iattpos].bToBePurged ?
                                   IDS_UNMARKFORPURGING : IDS_MARKFORPURGING));
    pPopup->ModifyMenu(ID_MENUITEM_MARKFORPURGING, MF_BYCOMMAND, ID_MENUITEM_MARKFORPURGING, csMenu);

    // If not orphaned - disable purge menu item
    pPopup->EnableMenuItem(ID_MENUITEM_MARKFORPURGING, MF_BYCOMMAND |
      (m_vAttDetails[iattpos].numreferenced != 0 ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED));

    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, msg_pt.x, msg_pt.y, this);
  }
}

void CManageAttachments::OnListAttEntries()
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

void CManageAttachments::OnViewAttachment()
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

void CManageAttachments::OnPurgeAttachment()
{
  POSITION pos = m_lcAttachments.GetFirstSelectedItemPosition();

  if (pos == NULL)
    return;

  int nItem = m_lcAttachments.GetNextSelectedItem(pos);
  int iattpos = m_lcAttachments.GetItemData(nItem);

  m_vAttDetails[iattpos].bToBePurged = !m_vAttDetails[iattpos].bToBePurged;

  int numActions = 0;
  for (size_t i = 0; i < m_vAttDetails.size(); i++) {
    if (m_vAttDetails[i].bToBePurged != m_vAttDetails[i].bInitalToBePurged)
      numActions++;
  }

  GetDlgItem(IDOK)->EnableWindow(numActions > 0 ? TRUE : FALSE);

  m_lcAttachments.Invalidate();
}

void CManageAttachments::OnPerformAction()
{
  CGeneralMsgBox gmb;
  CString csMessage(MAKEINTRESOURCE(IDS_PURGEATTACHMENT));
  CString csTitle(MAKEINTRESOURCE(IDS_MANAGEATTACHMENTS));
  INT_PTR rc = gmb.MessageBox(csMessage, csTitle, MB_YESNO);

  if (rc == IDYES) {
    PWScore *pcore = (PWScore *)GetMainDlg()->GetCore();

    // Can only mark orphaned attachments for purging later
    for (size_t i = 0; i < m_vAttDetails.size(); i++) {
      if (m_vAttDetails[i].bOrphaned &&
          m_vAttDetails[i].bToBePurged != m_vAttDetails[i].bInitalToBePurged) {
        CItemAtt &att = pcore->GetAtt(m_vAttDetails[i].att_uuid);
        att.SetToBePurged(m_vAttDetails[i].bToBePurged);
      }
    }
  }

  CPWDialog::OnOK();
}
