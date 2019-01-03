/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "../stdafx.h"
#include "PWFilterLC.h"
#include "PWFiltersDlg.h"

#include "SetHistoryFiltersDlg.h"
#include "SetPolicyFiltersDlg.h"
#include "SetAttachmentFiltersDlg.h"
#include "core/PWSFilters.h"

#include "../resource3.h"
#include "core/core.h"

#include <algorithm>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPWFilterLC::CPWFilterLC()
  : m_pPWF(NULL), m_iType(DFTYPE_INVALID), m_pfilters(NULL), m_bInitDone(false),
   m_fwidth(-1), m_lwidth(-1), m_rowheight(-1),
   m_bSetFieldActive(false), m_bSetLogicActive(false),
   m_iItem(-1), m_numfilters(0), m_pFont(NULL),
   m_pwchTip(NULL),
  // Following 4 variables only used if "m_iType == DFTYPE_MAIN"
  m_bPWHIST_Set(false), m_bPOLICY_Set(false), m_bATTACHMENT_Set(false),
  m_GoodHistory(false), m_GoodPolicy(false), m_GoodAttachment(false)
{
  m_crGrayText   = ::GetSysColor(COLOR_GRAYTEXT);
  m_crWindow     = ::GetSysColor(COLOR_WINDOW);
  m_crWindowText = ::GetSysColor(COLOR_WINDOWTEXT);
  m_crButtonFace = ::GetSysColor(COLOR_BTNFACE);
  m_crRedText    = RGB(168, 0, 0);
}

CPWFilterLC::~CPWFilterLC()
{
  // Do not delete filters, as they need to passed back to the caller

  m_pCheckImageList->DeleteImageList();
  delete m_pCheckImageList;

  m_pImageList->DeleteImageList();
  delete m_pImageList;

  delete m_pFont;
  delete m_pwchTip;
}

BEGIN_MESSAGE_MAP(CPWFilterLC, CListCtrl)
  //{{AFX_MSG_MAP(CPWFilterLC)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipText)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  ON_WM_LBUTTONDOWN()
  ON_WM_DESTROY()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPWFilterLC::OnDestroy()
{
  m_ComboBox.DestroyWindow();

  CListCtrl::OnDestroy();
}

void CPWFilterLC::Init(CWnd *pParent, st_filters *pfilters, const FilterType &filtertype,
  bool bCanHaveAttachments, const std::set<StringX> *psMediaTypes)
{
  m_pPWF = static_cast<CPWFiltersDlg *>(pParent);
  m_iType = filtertype;
  m_pfilters = pfilters;
  m_bCanHaveAttachments = bCanHaveAttachments;
  m_psMediaTypes = psMediaTypes;

  EnableToolTips(TRUE);

  const COLORREF crTransparent = RGB(192, 192, 192);

  // Load all images as list in enum CheckImageLC and in the order specified in it
  CBitmap bitmap;
  BITMAP bm;
  bitmap.LoadBitmap(IDB_CHECKED);
  bitmap.GetBitmap(&bm); // should be 13 x 13

  m_pCheckImageList = new CImageList;
  VERIFY(m_pCheckImageList->Create(bm.bmWidth, bm.bmHeight,
                                   ILC_MASK | ILC_COLOR, 2, 0));

  m_pCheckImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();
  bitmap.LoadBitmap(IDB_UNCHECKED);
  m_pCheckImageList->Add(&bitmap, crTransparent);
  bitmap.DeleteObject();

  // Set CHeaderDtrl control ID
  m_pHeaderCtrl = GetHeaderCtrl();
  m_pHeaderCtrl->SetDlgCtrlID(IDC_FILTERLC_HEADER);
  CString cs_text;

  cs_text = L" # ";
  InsertColumn(FLC_FILTER_NUMBER, cs_text);
  cs_text = L" ? ";
  InsertColumn(FLC_ENABLE_BUTTON, cs_text);
  cs_text = L" + ";
  InsertColumn(FLC_ADD_BUTTON, cs_text);
  cs_text = L" - ";
  InsertColumn(FLC_REM_BUTTON, cs_text);
  cs_text.LoadString(IDS_AND_OR);
  InsertColumn(FLC_LGC_COMBOBOX, cs_text);
  cs_text.LoadString(IDS_SELECTFIELD);
  InsertColumn(FLC_FLD_COMBOBOX, cs_text);
  cs_text.LoadString(IDS_CRITERIA_DESC);
  InsertColumn(FLC_CRITERIA_TEXT, cs_text);

  for (int i = 0; i <= FLC_CRITERIA_TEXT; i++) {
    SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
  }

  // Make +/- columns same size ('+' is bigger than '-')
  SetColumnWidth(FLC_REM_BUTTON, GetColumnWidth(FLC_ADD_BUTTON));

  // Make checkbox, and button headers centered (and so their contents in the ListCtrl)
  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT;
  lvc.fmt = LVCFMT_CENTER;
  SetColumn(FLC_FILTER_NUMBER, &lvc);
  SetColumn(FLC_ENABLE_BUTTON, &lvc);
  SetColumn(FLC_ADD_BUTTON, &lvc);
  SetColumn(FLC_REM_BUTTON, &lvc);

  switch (m_iType) {
    case DFTYPE_MAIN:
      // Set maximum number of filters per type and their own Control IDs
      m_FLD_ComboID = IDC_MFILTER_FLD_COMBOBOX;
      m_LGC_ComboID = IDC_MFILTER_LGC_COMBOBOX;
      m_pvfdata = &(m_pfilters->vMfldata);
      m_pnumactive = &(m_pfilters->num_Mactive);
      m_GoodHistory = m_pfilters->num_Hactive > 0;
      m_GoodPolicy = m_pfilters->num_Pactive > 0;
      m_GoodAttachment = m_pfilters->num_Aactive > 0;
      break;
    case DFTYPE_PWHISTORY:
      // Set maximum number of filters per type and their own Control IDs
      m_FLD_ComboID = IDC_HFILTER_FLD_COMBOBOX;
      m_LGC_ComboID = IDC_HFILTER_LGC_COMBOBOX;
      m_pvfdata = &(m_pfilters->vHfldata);
      m_pnumactive = &(m_pfilters->num_Hactive);
      break;
    case DFTYPE_PWPOLICY:
      // Set maximum number of filters per type and their own Control IDs
      m_FLD_ComboID = IDC_PFILTER_FLD_COMBOBOX;
      m_LGC_ComboID = IDC_PFILTER_LGC_COMBOBOX;
      m_pvfdata = &(m_pfilters->vPfldata);
      m_pnumactive = &(m_pfilters->num_Pactive);
      break;
    case DFTYPE_ATTACHMENT:
      // Set maximum number of filters per type and their own Control IDs
      m_FLD_ComboID = IDC_AFILTER_FLD_COMBOBOX;
      m_LGC_ComboID = IDC_AFILTER_LGC_COMBOBOX;
      m_pvfdata = &(m_pfilters->vAfldata);
      m_pnumactive = &(m_pfilters->num_Aactive);
      break;
    default:
      ASSERT(0);
  }

  SetUpComboBoxData();

  // Add dummy item to set column widths
  InsertItem(0, L".");  // FLC_ENABLE_BUTTON
  SetItemText(0, FLC_ADD_BUTTON, L".");
  SetItemText(0, FLC_REM_BUTTON, L".");
  SetItemText(0, FLC_LGC_COMBOBOX, L".");
  SetItemText(0, FLC_FLD_COMBOBOX, L".");
  SetItemText(0, FLC_CRITERIA_TEXT, L".");
  SetItemData(0, FLC_CRITERIA_REDTXT | FLC_FLD_CBX_ENABLED);
  m_iItem = 0;

  st_FilterRow newfilter;
  m_pvfdata->push_back(newfilter);

  // Create ComboBox
  CRect rc(0, 0, 10, 10);
  const DWORD dwStyle = CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL;
  VERIFY(m_ComboBox.Create(dwStyle, rc, this, 0));

  // Default is hidden & disabled
  m_ComboBox.EnableWindow(FALSE);
  m_ComboBox.ShowWindow(SW_HIDE);

  // Set up widths of columns with comboboxes - once for each combo
  DrawComboBox(FLC_FLD_COMBOBOX, -1);
  m_ComboBox.ResetContent();
  m_ComboBox.ClearSeparators();
  m_vWCFcbx_data.clear();

  DrawComboBox(FLC_LGC_COMBOBOX, -1);
  m_ComboBox.ResetContent();
  m_ComboBox.ClearSeparators();
  m_vWCFcbx_data.clear();

  // Remove dummy item
  DeleteItem(0);
  m_pvfdata->pop_back();

  // If existing filters, set them up in the dialog
  DeleteFilters();
  (*m_pnumactive) = 0;
  int i(0);
  
  DWORD_PTR dwData;
  vFilterRows::iterator Flt_iter;
  for (Flt_iter = m_pvfdata->begin(); Flt_iter != m_pvfdata->end(); Flt_iter++) {
    st_FilterRow &st_fldata = *Flt_iter;

    AddFilter_Controls();

    if (st_fldata.bFilterActive)
      (*m_pnumactive)++;

    dwData = 0;
    if (st_fldata.bFilterActive)
      dwData |= FLC_FILTER_ENABLED;
    else
      dwData &= ~FLC_FILTER_ENABLED;

    if (st_fldata.ftype == FT_PWHIST)
      m_bPWHIST_Set = true;
    if (st_fldata.ftype == FT_POLICY)
      m_bPOLICY_Set = true;
    if (st_fldata.ftype == FT_ATTACHMENT)
      m_bATTACHMENT_Set = true;

    if (st_fldata.ftype != FT_INVALID) {
      dwData |= FLC_FLD_CBX_SET | FLC_FLD_CBX_ENABLED;
      SetField(i);
      SetLogic(i);
      m_vlast_ft[i] = st_fldata.ftype;
      m_vlast_mt[i] = st_fldata.mtype;
      m_vcbxChanged[i] = false;
      std::vector<st_Fcbxdata>::iterator Fcbxdata_iter;
      Fcbxdata_iter = std::find_if(m_vFcbx_data.begin(), m_vFcbx_data.end(), 
                                   equal_ftype(st_fldata.ftype));
      if (Fcbxdata_iter != m_vFcbx_data.end())
        SetItemText(i, FLC_FLD_COMBOBOX, (*Fcbxdata_iter).cs_text);

      CString cs_criteria;
      if (st_fldata.ftype != FT_PWHIST && st_fldata.ftype != FT_POLICY &&
          st_fldata.rule == PWSMatch::MR_INVALID) {
        cs_criteria.LoadString(IDS_NOTDEFINED);
        dwData |= FLC_CRITERIA_REDTXT;
        m_vCriteriaSet[i] = false;
        st_fldata.bFilterComplete = false;
      } else {
        cs_criteria = PWSFilters::GetFilterDescription(st_fldata).c_str();
        dwData &= ~FLC_CRITERIA_REDTXT;
        m_vCriteriaSet[i] = true;
        st_fldata.bFilterComplete = true;
      }
      SetItemText(i, FLC_CRITERIA_TEXT, cs_criteria);
      SetColumnWidth(FLC_CRITERIA_TEXT, LVSCW_AUTOSIZE_USEHEADER);
    }

    if (i == 0)
      cs_text = L"";
    else
      cs_text = m_vLcbx_data[st_fldata.ltype == LC_AND ? 0 : 1].cs_text;
 
    SetItemText(i, FLC_LGC_COMBOBOX, cs_text);
    SetItemData(i, dwData);

    i++;
  }

  // Always show one filter!
  if (m_pvfdata->empty()) {
    AddFilter_Controls();
    SetItemText(i, FLC_LGC_COMBOBOX, L"");

    // Add a new filter to vector
    newfilter.Empty();
    m_pvfdata->push_back(newfilter);
    (*m_pnumactive)++;
  }

  ResetAndOr();

  if (m_iType == DFTYPE_MAIN)
    m_pPWF->EnableDisableApply();

  m_bInitDone = true;
}

BOOL CPWFilterLC::OnCommand(WPARAM wParam, LPARAM lParam)
{
  // Since we are dynamically adding controls, we need to handle their
  // messages (i.e. user clicking on them) ourselves and not via "ON_BN_CLICKED"
  WORD wNotify  = HIWORD(wParam);       // notification message
  UINT nID      = (UINT)LOWORD(wParam); // control identifier

  if (nID == m_FLD_ComboID || nID == m_LGC_ComboID) {
    switch (wNotify) {
      case CBN_SELENDOK:
        if (nID == m_FLD_ComboID) {
          m_bSetFieldActive = true;
          SetField(m_iItem);            
        } else {
          m_bSetLogicActive = true;
          SetLogic(m_iItem);
        }
        return TRUE;
      case CBN_CLOSEUP:
      case CBN_KILLFOCUS:
        if (!m_bSetFieldActive && !m_bSetLogicActive)
          CloseKillCombo();
        return TRUE;
      case CBN_DROPDOWN:
        if (!m_bSetFieldActive && !m_bSetLogicActive)
          DropDownCombo(nID);
        return TRUE;
      case CBN_SELENDCANCEL:
        if (nID == m_FLD_ComboID && !m_bSetFieldActive)
          CancelField(m_iItem);
        else if (nID == m_LGC_ComboID && !m_bSetLogicActive)
          CancelLogic(m_iItem);
        return TRUE;
    }
  }
  return CListCtrl::OnCommand(wParam, lParam);
}

int CPWFilterLC::AddFilter()
{
  int newID = AddFilter_Controls();

  // Check one was added
  if (newID < 0)
    return (-1);

  // Add a new filter to vector
  st_FilterRow newfilter;
  if (newID == 0)
    newfilter.ltype = LC_INVALID;
  else
    newfilter.ltype = LC_AND;

  m_pvfdata->push_back(newfilter);
  // Increase active number
  (*m_pnumactive)++;

  // Only if adding in the middle do we need to move things up and
  // reset the 'new' one
  if (m_iItem != newID - 1) {
    // Now move up any after us (only states - not actual pointers)
    // and reset the one immediately after use
    MoveUp(m_iItem);
  }

  ResetAndOr();

  m_pPWF->UpdateStatusText();

  // Make sure that the field column is wide enough.
  if (GetColumnWidth(FLC_FLD_COMBOBOX) < m_fwidth)
    SetColumnWidth(FLC_FLD_COMBOBOX, m_fwidth);

  return newID;
}

void CPWFilterLC::RemoveFilter()
{
  ASSERT(m_iItem >= 0);

  // Main Filters dialog need to handle History/Policy before calling this
  if (m_iType == DFTYPE_MAIN) {
    st_FilterRow &st_fldata = m_pvfdata->at(m_iItem);
    if (m_bPWHIST_Set && st_fldata.ftype == FT_PWHIST) {
      // We are deleting the History so put option back in other Comboboxes
      m_bPWHIST_Set = false;
    }
    else if (m_bPOLICY_Set && st_fldata.ftype == FT_POLICY) {
      // We are deleting the Policy so put option back in other Comboboxes
      m_bPOLICY_Set = false;
    }
  }

  if (m_iItem == 0 && m_pvfdata->size() == 1) {
    // Don't remove the one and only filter - just reset it
    ResetFilter(0);

    // Set active counter & number of filters
    (*m_pnumactive) = 1;
    m_numfilters = 1;
  }
  else if ((m_iItem + 1) == (int)m_pvfdata->size()) {
    // Last entry - is relatively easy!
    // Decrement active filters if necessary
    if (m_pvfdata->at(m_iItem).bFilterActive)
      (*m_pnumactive)--;

    // Decrement number of filters
    m_numfilters--;

    // Remove Control windows, controls and filterdata
    RemoveLast();
  } else {
    // Now remove one that is "not the only one" nor "the last one".
    if (m_pvfdata->at(m_iItem).bFilterActive)
      (*m_pnumactive)--;

    // Move all the information down one and then delete the last
    // Decrement active filters if necessary
    MoveDown();

    // Decrement number of filters
    m_numfilters--;

    // Now remove the end one
    RemoveLast();
  }
  ResetAndOr();

  m_pPWF->UpdateStatusText();
}

void CPWFilterLC::ResetFilter(const int num)
{
  // Mustn't remove first and ONLY one - just reinitialise the controls.
  // Enable filter, disable GetCriterion button, set AND, unset Combobox selection
  DWORD_PTR dwData;
  dwData = FLC_CRITERIA_REDTXT | FLC_FILTER_ENABLED;
  SetItemData(num, dwData);

  CString cs_text(MAKEINTRESOURCE(IDS_PICKFIELD));
  SetItemText(num, FLC_FLD_COMBOBOX, cs_text);

  m_vlast_ft[num] = FT_INVALID;
  m_vlast_mt[num] = PWSMatch::MT_INVALID;
  m_vcbxChanged[num] = true;
  m_vCriteriaSet[num] = false;
  m_vAddPresent[num] = false;

  // Make sure entry is pristine
  m_pvfdata->at(num).Empty();
  if (num == 0) {
    m_pvfdata->at(num).ltype = LC_INVALID;
    SetItemText(num, FLC_LGC_COMBOBOX, L"");
  } else {
    m_pvfdata->at(num).ltype = LC_AND;
    SetItemText(num, FLC_LGC_COMBOBOX, m_vLcbx_data[0].cs_text);
  }

  // Update criterion text and window text
  CString cs_criteria(MAKEINTRESOURCE(IDS_NOTDEFINED));
  SetItemText(num, FLC_CRITERIA_TEXT, cs_criteria);
  SetColumnWidth(FLC_CRITERIA_TEXT, LVSCW_AUTOSIZE_USEHEADER);
}

void CPWFilterLC::RemoveLast()
{
  int num = GetItemCount();
  ASSERT(num > 0);

  m_vlast_ft.pop_back();
  m_vlast_mt.pop_back();
  m_vcbxChanged.pop_back();
  m_vCriteriaSet.pop_back();
  m_vAddPresent.pop_back();

  // Remove filter
  m_pvfdata->pop_back();

  // Remove ListCtrl entry
  DeleteItem(num - 1);
}

int CPWFilterLC::AddFilter_Controls()
{
  // First add filter at the end of current filters
  CString cs_text;
  CRect rect;

  int newID = m_numfilters;

  cs_text.Format(L"%d", newID + 1);
  InsertItem(newID, cs_text);  // FLC_FILTER_NUMBER
  SetItemText(newID, FLC_ENABLE_BUTTON, L"");
  SetItemText(newID, FLC_ADD_BUTTON, L"+");
  SetItemText(newID, FLC_REM_BUTTON, L"-");
  cs_text.LoadString(IDSC_AND);
  SetItemText(newID, FLC_LGC_COMBOBOX, cs_text);
  cs_text.LoadString(IDS_PICKFIELD);
  SetItemText(newID, FLC_FLD_COMBOBOX, cs_text);

  // set status in cell
  cs_text.LoadString(IDS_NOTDEFINED);
  SetItemText(newID, FLC_CRITERIA_TEXT, cs_text);
  SetColumnWidth(FLC_CRITERIA_TEXT, LVSCW_AUTOSIZE_USEHEADER);
  SetItemData(newID, FLC_CRITERIA_REDTXT | FLC_FILTER_ENABLED);

  m_vlast_ft.push_back(FT_INVALID);
  m_vlast_mt.push_back(PWSMatch::MT_INVALID);
  m_vcbxChanged.push_back(true);
  m_vCriteriaSet.push_back(false);
  m_vAddPresent.push_back(false);

  // Increase total number
  m_numfilters++;

  return newID;
}

void CPWFilterLC::MoveDown()
{
  // Move  "states" down. i.e. deleting '1' means
  //  '0' unchanged; '2'->'1'; '3'->'2'; '4'->'3' etc.
  DWORD_PTR dwData;
  CString cs_text;

  for (int i = m_iItem; i < m_numfilters - 1; i++) {
    cs_text.Format(L"%d", i + 1);
    SetItemText(i, FLC_FILTER_NUMBER, cs_text);

    cs_text = GetItemText(i + 1, FLC_FLD_COMBOBOX);
    SetItemText(i, FLC_FLD_COMBOBOX, cs_text);

    cs_text = GetItemText(i + 1, FLC_LGC_COMBOBOX);
    SetItemText(i, FLC_LGC_COMBOBOX, cs_text);

    cs_text = GetItemText(i + 1, FLC_CRITERIA_TEXT);
    SetItemText(i, FLC_CRITERIA_TEXT, cs_text);
    SetColumnWidth(FLC_CRITERIA_TEXT, LVSCW_AUTOSIZE_USEHEADER);

    // Data field contains flags for enabled state and text colour
    dwData = GetItemData(i + 1);
    SetItemData(i, dwData);

    m_vlast_ft[i] = m_vlast_ft[i + 1];
    m_vlast_mt[i] = m_vlast_mt[i + 1];
    m_vcbxChanged[i] = m_vcbxChanged[i + 1];
    m_vCriteriaSet[i] = m_vCriteriaSet[i + 1];
    m_vAddPresent[i] = m_vAddPresent[i + 1];
    m_pvfdata->at(i) = m_pvfdata->at(i + 1);
  }
}

void CPWFilterLC::MoveUp(const int nAfter)
{
  // Move up. i.e. adding after '1' means
  //  '0' & '1' unchanged; '5'->'6'; '4'->'5'; '3'->'4';
  //  '2' then reset.

  // Move "states" up one; can't change pointers
  CString cs_text;
  DWORD_PTR dwData;

  for (int i = m_numfilters - 2; i > nAfter; i--) {
    cs_text.Format(L"%d", i + 2);
    SetItemText(i + 1, FLC_FILTER_NUMBER, cs_text);

    cs_text = GetItemText(i, FLC_FLD_COMBOBOX);
    SetItemText(i + 1, FLC_FLD_COMBOBOX, cs_text);

    cs_text = GetItemText(i + 1, FLC_LGC_COMBOBOX);
    SetItemText(i, FLC_LGC_COMBOBOX, cs_text);

    cs_text = GetItemText(i, FLC_CRITERIA_TEXT);
    SetItemText(i + 1, FLC_CRITERIA_TEXT, cs_text);
    SetColumnWidth(FLC_CRITERIA_TEXT, LVSCW_AUTOSIZE_USEHEADER);

    // Data field contains flags for enabled state and text colour
    dwData = GetItemData(i);
    SetItemData(i + 1, dwData);

    m_vlast_ft[i + 1] = m_vlast_ft[i];
    m_vlast_mt[i + 1] = m_vlast_mt[i];
    m_vcbxChanged[i + 1] = m_vcbxChanged[i];
    m_vCriteriaSet[i + 1] = m_vCriteriaSet[i];
    m_vAddPresent[i + 1] = m_vAddPresent[i];
    m_pvfdata->at(i + 1) = m_pvfdata->at(i);
  }

  // Now reset the 'new' one.
  ResetFilter(nAfter + 1);
}

void CPWFilterLC::ResetAndOr()
{
  // Ensure And/Or enabled
  DWORD_PTR dwData;
  for (int i = 1; i < GetItemCount(); i++) {
    dwData = GetItemData(i);
    dwData |= FLC_LGC_CBX_ENABLED;
    SetItemData(i, dwData);
  }

  // Except for the first one
  SetItemText(0, FLC_LGC_COMBOBOX, L"");
  dwData = GetItemData(0);
  dwData &= ~FLC_LGC_CBX_ENABLED;
  SetItemData(0, dwData);
}

void CPWFilterLC::DeleteFilters()
{
  if (m_vlast_ft.empty())
    return;

  // Clear the other vectors
  m_vlast_ft.clear();
  m_vlast_mt.clear();
  m_vcbxChanged.clear();
  m_vCriteriaSet.clear();
  m_vAddPresent.clear();

  // Clear filter data
  m_pvfdata->clear();

  // Zero counters
  *m_pnumactive = 0;
  m_numfilters = 0;
}

FieldType CPWFilterLC::EnableCriteria()
{
  // User clicked "Enable/Disable" filter checkbox
  ASSERT(m_iItem >= 0);

  // Get offset into vector of controls
  st_FilterRow &st_fldata = m_pvfdata->at(m_iItem);

  // Did they enable or disable the filter?
  bool bIsChecked = !st_fldata.bFilterActive;
  st_fldata.bFilterActive = bIsChecked;

  // Check, when activating me, if there are any active filters before us
  // If not, we won't show the logic enable the And/Or logic value
  // This is a real pain!
  int j(0);
  for (int i = 0; i < (int)m_pvfdata->size(); i++) {
    if (m_pvfdata->at(i).bFilterActive) {
      DWORD_PTR dwData2 = GetItemData(i);
      dwData2 &= ~FLC_LGC_CBX_ENABLED;
      SetItemData(i, dwData2);
      j = i;
      break;
    }
  }
  for (int i = j + 1; i < (int)m_pvfdata->size(); i++) {
    if (m_pvfdata->at(i).bFilterActive) {
      DWORD_PTR dwData2 = GetItemData(i);
      dwData2 |= FLC_LGC_CBX_ENABLED;
      SetItemData(i, dwData2);
    }
  }

  DWORD_PTR dwData = GetItemData(m_iItem);
  FieldType ft = st_fldata.ftype;

  // If user (had) selected an entry, get its field type
  // If not, don't let them try and select a criterion
  if ((ft != FT_INVALID) && bIsChecked)
    dwData |= FLC_FLD_CBX_ENABLED;
  else
    dwData &= ~FLC_FLD_CBX_ENABLED;

  // Set state of this filter and adjust active count
  if (bIsChecked) {
    // Now active
    dwData |= FLC_FILTER_ENABLED;
    st_fldata.bFilterActive = true;
    (*m_pnumactive)++;
  } else {
    // Now inactive
    dwData &= ~FLC_FILTER_ENABLED;
    st_fldata.bFilterActive = false;
    (*m_pnumactive)--;
  }

  SetItemData(m_iItem, dwData);

  CString cs_text(MAKEINTRESOURCE(IDS_PICKFIELD));
  if (m_iType == DFTYPE_MAIN) {
    if (st_fldata.bFilterActive) {
      if (ft == FT_PWHIST) {
        if (!m_bPWHIST_Set)
          m_bPWHIST_Set = true;
        else {
          SetItemText(m_iItem, FLC_FLD_COMBOBOX, cs_text);
          st_fldata.ftype = FT_INVALID;
        }
      }
      if (ft == FT_POLICY) {
        if (!m_bPOLICY_Set)
          m_bPOLICY_Set = true;
        else {
          SetItemText(m_iItem, FLC_FLD_COMBOBOX, cs_text);
          st_fldata.ftype = FT_INVALID;
        }
      }
      if (ft == FT_ATTACHMENT) {
        if (!m_bATTACHMENT_Set)
          m_bATTACHMENT_Set = true;
        else {
          SetItemText(m_iItem, FLC_FLD_COMBOBOX, cs_text);
          st_fldata.ftype = FT_INVALID;
        }
      }
    } else {
      if (m_bPWHIST_Set && ft == FT_PWHIST)
        m_bPWHIST_Set = false;
      if (m_bPOLICY_Set && ft == FT_POLICY)
        m_bPOLICY_Set = false;
      if (m_bATTACHMENT_Set && ft == FT_ATTACHMENT)
        m_bATTACHMENT_Set = false;
    }
    m_pPWF->EnableDisableApply();
  }

  // Update dialog window text
  m_pPWF->UpdateStatusText();

  return ft;
}

bool CPWFilterLC::SetField(const int iItem)
{
  // User has selected field
  bool retval(false);

  // Set focus to main window in case user does page up/down next
  // so that it changes the scroll bar not the value in this
  // ComboBox
  GetParent()->SetFocus();

  // Get offset into vector of controls
  st_FilterRow &st_fldata = m_pvfdata->at(iItem);

  FieldType ft(FT_INVALID);

  // Don't get selection from ComboBox during inital setup
  if (m_bInitDone) {
    int iSelect = m_ComboBox.GetCurSel();
    if (iSelect != CB_ERR) {
      ft = (FieldType)m_ComboBox.GetItemData(iSelect);
    }
    st_fldata.ftype = ft;
  } else {
    ft = st_fldata.ftype;
  }

  CString cs_text(MAKEINTRESOURCE(IDS_PICKFIELD));
  if (ft != FT_INVALID) {
    std::vector<st_Fcbxdata>::iterator Fcbxdata_iter;
    Fcbxdata_iter = std::find_if(m_vWCFcbx_data.begin(), m_vWCFcbx_data.end(), equal_ftype(ft));
    if (Fcbxdata_iter != m_vWCFcbx_data.end())
      cs_text = (*Fcbxdata_iter).cs_text;
  }

  SetItemText(iItem, FLC_FLD_COMBOBOX, cs_text);

  DWORD_PTR dwData = GetItemData(iItem);
  if (ft == FT_INVALID) {
    // Oh no they haven't!
    dwData &= ~FLC_FLD_CBX_SET;
    SetItemData(iItem, dwData);
    goto reset_combo;
  } else {
    dwData |= (FLC_FLD_CBX_ENABLED | FLC_FLD_CBX_SET);
  }

  // Get related fieldtype
  PWSMatch::MatchType mt(PWSMatch::MT_INVALID);

  // Default - do not add 'is present'/'is not present' to the match dialog
  bool bAddPresent(false);

  // Now get associated match type
  switch (m_iType) {
    case DFTYPE_MAIN:
      switch (ft) {
        case FT_GROUPTITLE:
        case FT_TITLE:
          // Title MUST be present - no need to add that check
          mt = PWSMatch::MT_STRING;
          break;
        case FT_PASSWORD:
          // Password MUST be present - no need to add that check
          mt = PWSMatch::MT_PASSWORD;
          break;
        case FT_PASSWORDLEN:
          mt = PWSMatch::MT_INTEGER;
          break;
        case FT_GROUP:
        case FT_USER:
        case FT_NOTES:
        case FT_URL:
        case FT_AUTOTYPE:
        case FT_RUNCMD:
        case FT_EMAIL:
        case FT_SYMBOLS:
        case FT_POLICYNAME:
          bAddPresent = true;
          mt = PWSMatch::MT_STRING;
          break;
        case FT_DCA:
        case FT_SHIFTDCA:
          mt = PWSMatch::MT_DCA;
          break;
        case FT_CTIME:
        case FT_PMTIME:
        case FT_ATIME:
        case FT_XTIME:
        case FT_RMTIME:
          bAddPresent = true;
          mt = PWSMatch::MT_DATE;
          break;
        case FT_PWHIST:
          // 'Add Present' doesn't make sense here - see sub-dialog
          mt = PWSMatch::MT_PWHIST;
          break;
        case FT_POLICY:
          // 'Add Present' doesn't make sense here - see sub-dialog
          mt = PWSMatch::MT_POLICY;
          break;
        case FT_XTIME_INT:
          bAddPresent = true;
          mt = PWSMatch::MT_INTEGER;
          break;
        case FT_PROTECTED:
          m_fbool.m_bt = CFilterBoolDlg::BT_IS;
          mt = PWSMatch::MT_BOOL;
          break;
        case FT_KBSHORTCUT:
          // 'Add Present' not needed - bool match
          m_fbool.m_bt = CFilterBoolDlg::BT_PRESENT;
          mt = PWSMatch::MT_BOOL;
          break;
        case FT_UNKNOWNFIELDS:
          // 'Add Present' not needed - bool match
          m_fbool.m_bt = CFilterBoolDlg::BT_PRESENT;
          mt = PWSMatch::MT_BOOL;
          break;
        case FT_ENTRYTYPE:
          // Entrytype is an entry attribute and MUST be present
          //  - no need to add that check
          mt = PWSMatch::MT_ENTRYTYPE;
          break;
        case FT_ENTRYSTATUS:
          // Entrystatus is an entry attribute and MUST be present
          //  - no need to add that check
          mt = PWSMatch::MT_ENTRYSTATUS;
          break;
        case FT_ENTRYSIZE:
          // Entrysize is an entry attribute and MUST be present
          //  - no need to add that check
          mt = PWSMatch::MT_ENTRYSIZE;
          break;
        case FT_ATTACHMENT:
          // 'Add Present' doesn't make sense here - see sub-dialog
          mt = PWSMatch::MT_ATTACHMENT;
          break;
        default:
          ASSERT(0);
      }
      break;

    case DFTYPE_PWHISTORY:
      switch (ft) {
        case HT_PRESENT:
          // 'Add Present' not needed - bool match
          m_fbool.m_bt = CFilterBoolDlg::BT_PRESENT;
          mt = PWSMatch::MT_BOOL;
          break;
        case HT_ACTIVE:
          // 'Add Present' not needed - bool match
          m_fbool.m_bt = CFilterBoolDlg::BT_ACTIVE;
          mt = PWSMatch::MT_BOOL;
          break;
        case HT_NUM:
        case HT_MAX:
          // 'Add Present' doesn't make sense here
          mt = PWSMatch::MT_INTEGER;
          break;
        case HT_CHANGEDATE:
          // 'Add Present' doesn't make sense here
          mt = PWSMatch::MT_DATE;
          break;
        case HT_PASSWORDS:
          // 'Add Present' doesn't make sense here
          mt = PWSMatch::MT_PASSWORD;
          break;
        default:
          ASSERT(0);
      }
      break;

    case DFTYPE_PWPOLICY:
      switch (ft) {
        case PT_PRESENT:
          // 'Add Present' not needed - bool match
          m_fbool.m_bt = CFilterBoolDlg::BT_PRESENT;
          mt = PWSMatch::MT_BOOL;
          break;
        case PT_EASYVISION:
        case PT_PRONOUNCEABLE:
        case PT_HEXADECIMAL:
          // 'Add Present' not needed - bool match
          m_fbool.m_bt = CFilterBoolDlg::BT_SET;
          mt = PWSMatch::MT_BOOL;
          break;
        case PT_LENGTH:
        case PT_LOWERCASE:
        case PT_UPPERCASE:
        case PT_DIGITS:
        case PT_SYMBOLS:
          // 'Add Present' doesn't make sense here
          mt = PWSMatch::MT_INTEGER;
          break;
        default:
          ASSERT(0);
      }
      break;
    case DFTYPE_ATTACHMENT:
      switch (ft) {
        case AT_PRESENT:
          // 'Add Present' not needed - bool match
          m_fbool.m_bt = CFilterBoolDlg::BT_PRESENT;
          mt = PWSMatch::MT_BOOL;
          break;
        case AT_FILENAME:
          // File name MUST be present - no need to add that check
          mt = PWSMatch::MT_STRING;
          break;
        case AT_TITLE:
        case AT_FILEPATH:
          bAddPresent = true;
          mt = PWSMatch::MT_STRING;
          break;
        case AT_MEDIATYPE:
          // Media type  MUST be present - no need to add that check
          mt = PWSMatch::MT_MEDIATYPE;
          break;
        case AT_CTIME:
        case AT_FILECTIME:
        case AT_FILEMTIME:
        case AT_FILEATIME:
          bAddPresent = true;
          mt = PWSMatch::MT_DATE;
          break;
        default:
          ASSERT(0);
      }
      break;
    default:
      ASSERT(0);
  }

  // Save if user has just re-selected previous selection - i.e. no change
  if (m_vlast_ft[iItem] != ft || m_vlast_mt[iItem] != mt) {
    m_vcbxChanged[iItem] = true;
    m_vCriteriaSet[iItem] = false;
    st_fldata.bFilterComplete = false;
  } else
    m_vcbxChanged[iItem] = false;

  m_vAddPresent[iItem] = bAddPresent;
  if (m_iType == DFTYPE_MAIN) {
    if (m_vlast_ft[iItem] == FT_PWHIST && ft != FT_PWHIST)
      m_bPWHIST_Set = false;
    else if (ft == FT_PWHIST)
      m_bPWHIST_Set = true;

    if (m_vlast_ft[iItem] == FT_POLICY && ft != FT_POLICY)
      m_bPOLICY_Set = false;
    else if (ft == FT_POLICY)
      m_bPOLICY_Set = true;

    if (m_vlast_ft[iItem] == FT_ATTACHMENT && ft != FT_ATTACHMENT)
      m_bATTACHMENT_Set = false;
    else if (ft == FT_ATTACHMENT)
      m_bATTACHMENT_Set = true;
  }

  // Now we can update the last selected field
  m_vlast_ft[iItem] = ft;

  // Save last match type now
  m_vlast_mt[iItem] = mt;

  if (mt == PWSMatch::MT_INVALID) {
    dwData &= ~FLC_FLD_CBX_ENABLED;
    m_vCriteriaSet[iItem] = false;
    st_fldata.bFilterComplete = false;
  }

  // If criterion not set, update static text message to user
  if (m_vCriteriaSet[iItem] == false) {
    CString cs_criteria(MAKEINTRESOURCE(IDS_NOTDEFINED));
    dwData |= FLC_CRITERIA_REDTXT;
    SetItemText(iItem, FLC_CRITERIA_TEXT, cs_criteria);
    SetColumnWidth(FLC_CRITERIA_TEXT, LVSCW_AUTOSIZE_USEHEADER);
  }

  if (mt == PWSMatch::MT_INVALID) {
    SetItemData(iItem, dwData);
    goto reset_combo;
  }

  dwData |= FLC_FLD_CBX_ENABLED;
  SetItemData(iItem, dwData);
  st_fldata.mtype = mt;
  st_fldata.ftype = ft;
  retval = true;

reset_combo:
  if (m_bInitDone) {
    m_ComboBox.ShowWindow(SW_HIDE);
    m_ComboBox.EnableWindow(FALSE);
    m_ComboBox.ResetContent();
    m_ComboBox.ClearSeparators();
    m_vWCFcbx_data.clear();
  }

  RedrawItems(iItem, iItem);
  UpdateWindow();

  m_bSetFieldActive = false;
  return retval;
}

void CPWFilterLC::CancelField(const int iItem)
{
  // User has selected field

  // Set focus to main window in case user does page up/down next
  // so that it changes the scroll bar not the value in this
  // ComboBox
  GetParent()->SetFocus();

  // Get offset into vector of controls
  st_FilterRow &st_fldata = m_pvfdata->at(iItem);

  FieldType ft(st_fldata.ftype);

  CString cs_text(MAKEINTRESOURCE(IDS_PICKFIELD));
  if (ft != FT_INVALID) {
    std::vector<st_Fcbxdata>::iterator Fcbxdata_iter;
    Fcbxdata_iter = std::find_if(m_vWCFcbx_data.begin(), m_vWCFcbx_data.end(), equal_ftype(ft));
    if (Fcbxdata_iter != m_vWCFcbx_data.end())
      cs_text = (*Fcbxdata_iter).cs_text;
  }

  SetItemText(iItem, FLC_FLD_COMBOBOX, cs_text);

  // m_ComboBox is NULL during inital setup
  if (m_bInitDone) {
    m_ComboBox.ShowWindow(SW_HIDE);
    m_ComboBox.EnableWindow(FALSE);
    m_ComboBox.ResetContent();
    m_ComboBox.ClearSeparators();
    m_vWCFcbx_data.clear();
  }
  return;
}

void CPWFilterLC::SetLogic(const int iItem)
{
  CString cs_text(L"");

  if (iItem == 0) {
    SetItemText(iItem, FLC_LGC_COMBOBOX, cs_text);
    goto reset_combo;
  }

  // Get offset into vector of controls
  st_FilterRow &st_fldata = m_pvfdata->at(iItem);

  LogicConnect lt(st_fldata.ltype);

  // Don't get selection from ComboBox during inital setup
  if (m_bInitDone) {
    int iSelect = m_ComboBox.GetCurSel();
    if (iSelect != CB_ERR) {
      lt = (LogicConnect)m_ComboBox.GetItemData(iSelect);
    }
    m_pvfdata->at(iItem).ltype = lt;
  }

  cs_text = m_vLcbx_data[0].cs_text;
  if (lt != LC_INVALID) {
    std::vector<st_Lcbxdata>::iterator Lcbxdata_iter;
    Lcbxdata_iter = std::find_if(m_vLcbx_data.begin(), m_vLcbx_data.end(), equal_ltype(lt));
    if (Lcbxdata_iter != m_vLcbx_data.end())
      cs_text = (*Lcbxdata_iter).cs_text;
  }

  SetItemText(iItem, FLC_LGC_COMBOBOX, cs_text);

reset_combo:
  if (m_bInitDone) {
    m_ComboBox.ShowWindow(SW_HIDE);
    m_ComboBox.EnableWindow(FALSE);
    m_ComboBox.ResetContent();
    m_ComboBox.ClearSeparators();
    m_vWCFcbx_data.clear();
  }
  
  RedrawItems(iItem, iItem);
  UpdateWindow();

  m_bSetLogicActive = false;
}

void CPWFilterLC::CancelLogic(const int iItem)
{
  CString cs_text(L"");

  // Get offset into vector of controls
  if (iItem == 0) {
    SetItemText(iItem, FLC_LGC_COMBOBOX, cs_text);
    goto reset_combo;
  }

  st_FilterRow &st_fldata = m_pvfdata->at(iItem);

  LogicConnect lt(st_fldata.ltype);

  cs_text = m_vLcbx_data[0].cs_text;
  if (lt != LC_INVALID) {
    std::vector<st_Lcbxdata>::iterator Lcbxdata_iter;
    Lcbxdata_iter = std::find_if(m_vLcbx_data.begin(), m_vLcbx_data.end(), equal_ltype(lt));
    if (Lcbxdata_iter != m_vLcbx_data.end())
      cs_text = (*Lcbxdata_iter).cs_text;
  }

  SetItemText(iItem, FLC_LGC_COMBOBOX, cs_text);

reset_combo:
  // Reset ComboBox
  if (m_bInitDone) {
    m_ComboBox.ShowWindow(SW_HIDE);
    m_ComboBox.EnableWindow(FALSE);
    m_ComboBox.ResetContent();
    m_ComboBox.ClearSeparators();
    m_vWCFcbx_data.clear();
  }

  RedrawItems(iItem, iItem);
  UpdateWindow();
}

void CPWFilterLC::CloseKillCombo()
{
  // Reset ComboBox
  if (m_bInitDone) {
    m_ComboBox.ShowWindow(SW_HIDE);
    m_ComboBox.EnableWindow(FALSE);
    m_ComboBox.ResetContent();
    m_ComboBox.ClearSeparators();
    m_vWCFcbx_data.clear();
  }
}

void CPWFilterLC::DropDownCombo(const UINT nID)
{
  CString cs_text;
  int index(-1);

  if (nID == m_FLD_ComboID) {
    FieldType ft = m_pvfdata->at(m_iItem).ftype;
    cs_text.LoadString(IDS_PICKFIELD);
    if (ft != FT_INVALID) {
      std::vector<st_Fcbxdata>::iterator Fcbxdata_iter;
      Fcbxdata_iter = std::find_if(m_vWCFcbx_data.begin(), m_vWCFcbx_data.end(), equal_ftype(ft));
      if (Fcbxdata_iter != m_vWCFcbx_data.end()) {
        cs_text = (*Fcbxdata_iter).cs_text;
        index = (int)(Fcbxdata_iter - m_vWCFcbx_data.begin());
      }
    }
    SetItemText(m_iItem, FLC_FLD_COMBOBOX, cs_text);
  } else {
    LogicConnect lt = m_pvfdata->at(m_iItem).ltype;
    index = (lt == LC_AND) ? 0 : 1;
    cs_text.LoadString(lt == LC_AND ? IDSC_AND : IDSC_OR);
    SetItemText(m_iItem, FLC_LGC_COMBOBOX, cs_text);
  }
  
  SetComboBoxWidth(nID == m_FLD_ComboID ? FLC_FLD_COMBOBOX : FLC_LGC_COMBOBOX);

  m_ComboBox.SetCurSel(index);
}

bool CPWFilterLC::GetCriterion()
{
  // User has already enabled the filter and selected the field type
  // Now get the criterion
  ASSERT(m_iItem >= 0);

  // Get offset into vector of controls
  INT_PTR rc(IDCANCEL);

  st_FilterRow &st_fldata = m_pvfdata->at(m_iItem);

  // Get Text
  const CString cs_selected = GetItemText(m_iItem, FLC_LGC_COMBOBOX);

  // Save fieldtype (need it when we reset the filter data)
  const FieldType ft = st_fldata.ftype;

  bool b_good(false);
  LogicConnect ltype(st_fldata.ltype);

  // Now go display the correct match dialog
  switch (st_fldata.mtype) {
    case PWSMatch::MT_STRING:
      m_fstring.m_add_present = m_vAddPresent[m_iItem];
      m_fstring.m_title = cs_selected;
      if (!m_vcbxChanged[m_iItem] &&
          st_fldata.rule != PWSMatch::MR_INVALID) {
        m_fstring.m_rule = st_fldata.rule;
        m_fstring.m_string = st_fldata.fstring.c_str();
        m_fstring.m_case = st_fldata.fcase ? BST_CHECKED : BST_UNCHECKED;
      } else {
        m_fstring.m_rule = PWSMatch::MR_INVALID;
      }
      m_fstring.SetSymbol(st_fldata.ftype == FT_SYMBOLS);
      rc = m_fstring.DoModal();
      if (rc == IDOK) {
        st_fldata.Empty();
        st_fldata.bFilterActive = true;
        st_fldata.mtype = PWSMatch::MT_STRING;
        st_fldata.ftype = ft;
        st_fldata.rule = m_fstring.m_rule;
        st_fldata.fstring = m_fstring.m_string;
        if (st_fldata.rule == PWSMatch::MR_PRESENT ||
            st_fldata.rule == PWSMatch::MR_NOTPRESENT)
          st_fldata.fcase = false;
        else
          st_fldata.fcase = (m_fstring.m_case == BST_CHECKED);
        b_good = true;
      }
      break;
    case PWSMatch::MT_PASSWORD:
      m_fpswd.m_title = cs_selected;
      if (!m_vcbxChanged[m_iItem] &&
          st_fldata.rule != PWSMatch::MR_INVALID) {
        m_fpswd.m_rule = st_fldata.rule;
        m_fpswd.m_string = st_fldata.fstring.c_str();
        m_fpswd.m_case = st_fldata.fcase ? BST_CHECKED : BST_UNCHECKED;
        m_fpswd.m_num1 = st_fldata.fnum1;
      } else {
        m_fpswd.m_rule = PWSMatch::MR_INVALID;
      }
      rc = m_fpswd.DoModal();
      if (rc == IDOK) {
        st_fldata.Empty();
        st_fldata.bFilterActive = true;
        st_fldata.mtype = PWSMatch::MT_PASSWORD;
        st_fldata.ftype = ft;
        st_fldata.rule = m_fpswd.m_rule;
        st_fldata.fstring = LPCWSTR(m_fpswd.m_string);
        st_fldata.fcase = (m_fpswd.m_case == BST_CHECKED);
        if (st_fldata.rule != PWSMatch::MR_WILLEXPIRE)
          st_fldata.fnum1 = 0;
        else
          st_fldata.fnum1 = m_fpswd.m_num1;
        b_good = true;
      }
      break;
    case PWSMatch::MT_INTEGER:
      m_finteger.m_add_present = m_vAddPresent[m_iItem];
      m_finteger.m_title = cs_selected;
      if (!m_vcbxChanged[m_iItem] &&
          st_fldata.rule != PWSMatch::MR_INVALID) {
        m_finteger.m_rule = st_fldata.rule;
        m_finteger.m_num1 = st_fldata.fnum1;
        m_finteger.m_num2 = st_fldata.fnum2;
      } else {
        m_finteger.m_rule = PWSMatch::MR_INVALID;
      }
      rc = m_finteger.DoModal();
      if (rc == IDOK) {
        st_fldata.Empty();
        st_fldata.bFilterActive = true;
        st_fldata.mtype = PWSMatch::MT_INTEGER;
        st_fldata.ftype = ft;
        st_fldata.rule = m_finteger.m_rule;
        st_fldata.fnum1 = m_finteger.m_num1;
        st_fldata.fnum2 = m_finteger.m_num2;
        b_good = true;
      }
      break;
    case PWSMatch::MT_DATE:
      m_fdate.m_add_present = m_vAddPresent[m_iItem];
      m_fdate.m_title = cs_selected;
      if (!m_vcbxChanged[m_iItem] &&
          st_fldata.rule != PWSMatch::MR_INVALID) {
        m_fdate.m_rule = st_fldata.rule;
        m_fdate.m_datetype = st_fldata.fdatetype;
        m_fdate.m_time_t1 = st_fldata.fdate1;
        m_fdate.m_time_t2 = st_fldata.fdate2;
        m_fdate.m_num1 = st_fldata.fnum1;
        m_fdate.m_num2 = st_fldata.fnum2;
      } else {
        m_fdate.m_rule = PWSMatch::MR_INVALID;
      }
      m_fdate.SetFieldType(ft);  // Only FT_XTIME is allowed a future time
      rc = m_fdate.DoModal();
      if (rc == IDOK) {
        st_fldata.Empty();
        st_fldata.bFilterActive = true;
        st_fldata.mtype = PWSMatch::MT_DATE;
        st_fldata.ftype = ft;
        st_fldata.rule = m_fdate.m_rule;
        st_fldata.fdatetype = m_fdate.m_datetype;
        st_fldata.fdate1 = m_fdate.m_time_t1;
        st_fldata.fdate2 = m_fdate.m_time_t2;
        st_fldata.fnum1 = m_fdate.m_num1;
        st_fldata.fnum2 = m_fdate.m_num2;
        b_good = true;
      }
      break;
    case PWSMatch::MT_PWHIST:
      {
        st_filters filters(*m_pfilters);
        CSetHistoryFiltersDlg fhistory(this, &filters, m_pPWF->GetFiltername());
        rc = fhistory.DoModal();
        if (rc == IDOK) {
          st_fldata.Empty();
          st_fldata.bFilterActive = true;
          st_fldata.mtype = PWSMatch::MT_PWHIST;
          st_fldata.ftype = ft;
          m_pfilters->num_Hactive = filters.num_Hactive;
          m_pfilters->vHfldata = filters.vHfldata;
          m_bPWHIST_Set = true;
          m_pPWF->UpdateStatusText();
        }
        m_GoodHistory = m_pfilters->num_Hactive > 0;
        b_good = m_GoodHistory;
      }
      break;
    case PWSMatch::MT_POLICY:
      {
        st_filters filters(*m_pfilters);
        CSetPolicyFiltersDlg fpolicy(this, &filters, m_pPWF->GetFiltername());
        rc = fpolicy.DoModal();
        if (rc == IDOK) {
          st_fldata.Empty();
          st_fldata.bFilterActive = true;
          st_fldata.mtype = PWSMatch::MT_POLICY;
          st_fldata.ftype = ft;
          m_pfilters->num_Pactive = filters.num_Pactive;
          m_pfilters->vPfldata = filters.vPfldata;
          m_bPOLICY_Set = true;
          m_pPWF->UpdateStatusText();
        }
        m_GoodPolicy = m_pfilters->num_Pactive > 0;
        b_good = m_GoodPolicy;
      }
      break;
    case PWSMatch::MT_BOOL:
      m_fbool.m_title = cs_selected;
      if (!m_vcbxChanged[m_iItem] &&
          st_fldata.rule != PWSMatch::MR_INVALID) {
        m_fbool.m_rule = st_fldata.rule;
      } else {
        m_fbool.m_rule = PWSMatch::MR_INVALID;
      }
      rc = m_fbool.DoModal();
      if (rc == IDOK) {
        st_fldata.Empty();
        st_fldata.bFilterActive = true;
        st_fldata.mtype = PWSMatch::MT_BOOL;
        st_fldata.ftype = ft;
        st_fldata.rule = m_fbool.m_rule;
        b_good = true;
      }
      break;
    case PWSMatch::MT_ENTRYTYPE:
      m_fentry.m_title = cs_selected;
      if (!m_vcbxChanged[m_iItem] &&
          st_fldata.rule != PWSMatch::MR_INVALID) {
        m_fentry.m_rule = st_fldata.rule;
        m_fentry.m_etype = st_fldata.etype;
      } else {
        m_fentry.m_rule = PWSMatch::MR_INVALID;
      }
      rc = m_fentry.DoModal();
      if (rc == IDOK) {
        st_fldata.Empty();
        st_fldata.bFilterActive = true;
        st_fldata.mtype = PWSMatch::MT_ENTRYTYPE;
        st_fldata.ftype = ft;
        st_fldata.rule = m_fentry.m_rule;
        st_fldata.etype = m_fentry.m_etype;
        b_good = true;
      }
      break;
    case PWSMatch::MT_DCA:
    case PWSMatch::MT_SHIFTDCA:
      m_fDCA.m_title = cs_selected;
      if (!m_vcbxChanged[m_iItem] &&
          st_fldata.rule != PWSMatch::MR_INVALID) {
        m_fDCA.m_rule = st_fldata.rule;
        m_fDCA.m_DCA = st_fldata.fdca;
      } else {
        m_fDCA.m_rule = PWSMatch::MR_INVALID;
      }
      m_fDCA.m_type = st_fldata.mtype;  // MT_DCA or MT_SHIFTDCA
      rc = m_fDCA.DoModal();
      if (rc == IDOK) {
        st_fldata.Empty();
        st_fldata.bFilterActive = true;
        st_fldata.mtype = m_fDCA.m_type;
        st_fldata.ftype = ft;
        st_fldata.rule = m_fDCA.m_rule;
        st_fldata.fdca = m_fDCA.m_DCA;
        b_good = true;
      }
      break;
    case PWSMatch::MT_ENTRYSTATUS:
      m_fstatus.m_title = cs_selected;
      if (!m_vcbxChanged[m_iItem] &&
          st_fldata.rule != PWSMatch::MR_INVALID) {
        m_fstatus.m_rule = st_fldata.rule;
        m_fstatus.m_estatus = st_fldata.estatus;
      } else {
        m_fstatus.m_rule = PWSMatch::MR_INVALID;
      }
      rc = m_fstatus.DoModal();
      if (rc == IDOK) {
        st_fldata.Empty();
        st_fldata.bFilterActive = true;
        st_fldata.mtype = PWSMatch::MT_ENTRYSTATUS;
        st_fldata.ftype = ft;
        st_fldata.rule = m_fstatus.m_rule;
        st_fldata.estatus = m_fstatus.m_estatus;
        b_good = true;
      }
      break;
    case PWSMatch::MT_ENTRYSIZE:
      m_fsize.m_title = cs_selected;
      if (!m_vcbxChanged[m_iItem] &&
          st_fldata.rule != PWSMatch::MR_INVALID) {
        m_fsize.m_rule = st_fldata.rule;
        m_fsize.m_unit = st_fldata.funit;
        m_fsize.m_size1 = st_fldata.fnum1 >> (m_fsize.m_unit * 10);
        m_fsize.m_size2 = st_fldata.fnum2 >> (m_fsize.m_unit * 10);
      } else {
        m_fsize.m_rule = PWSMatch::MR_INVALID;
      }
      rc = m_fsize.DoModal();
      if (rc == IDOK) {
        st_fldata.Empty();
        st_fldata.bFilterActive = true;
        st_fldata.mtype = PWSMatch::MT_ENTRYSIZE;
        st_fldata.ftype = ft;
        st_fldata.rule = m_fsize.m_rule;
        st_fldata.funit = m_fsize.m_unit;
        st_fldata.fnum1 = m_fsize.m_size1 << (m_fsize.m_unit * 10);
        st_fldata.fnum2 = m_fsize.m_size2 << (m_fsize.m_unit * 10);
        b_good = true;
      }
      break;
    case PWSMatch::MT_ATTACHMENT:
      {
        st_filters filters(*m_pfilters);
        CSetAttachmentFiltersDlg fattachment(this, &filters, m_pPWF->GetFiltername(),
          m_bCanHaveAttachments, m_psMediaTypes);
        rc = fattachment.DoModal();
        if (rc == IDOK) {
          st_fldata.Empty();
          st_fldata.bFilterActive = true;
          st_fldata.mtype = PWSMatch::MT_ATTACHMENT;
          st_fldata.ftype = ft;
          m_pfilters->num_Aactive = filters.num_Aactive;
          m_pfilters->vAfldata = filters.vAfldata;
          m_bATTACHMENT_Set = true;
          m_pPWF->UpdateStatusText();
        }
        m_GoodAttachment = m_pfilters->num_Aactive > 0;
        b_good = m_GoodAttachment;
      }
      break;
    case PWSMatch::MT_MEDIATYPE:
      m_fmediatype.m_add_present = m_vAddPresent[m_iItem];
      m_fmediatype.m_title = cs_selected;
      if (!m_vcbxChanged[m_iItem] &&
          st_fldata.rule != PWSMatch::MR_INVALID) {
        m_fmediatype.m_rule = st_fldata.rule;
        m_fmediatype.m_string = st_fldata.fstring.c_str();
        m_fmediatype.m_case = st_fldata.fcase ? BST_CHECKED : BST_UNCHECKED;
      } else {
        m_fmediatype.m_rule = PWSMatch::MR_INVALID;
      }
      m_fmediatype.m_psMediaTypes = m_psMediaTypes;
      rc = m_fmediatype.DoModal();
      if (rc == IDOK) {
        st_fldata.Empty();
        st_fldata.bFilterActive = true;
        st_fldata.mtype = PWSMatch::MT_MEDIATYPE;
        st_fldata.ftype = ft;
        st_fldata.rule = m_fmediatype.m_rule;
        st_fldata.fstring = m_fmediatype.m_string;
        if (st_fldata.rule == PWSMatch::MR_PRESENT ||
            st_fldata.rule == PWSMatch::MR_NOTPRESENT)
          st_fldata.fcase = false;
        else
          st_fldata.fcase = (m_fmediatype.m_case == BST_CHECKED);
        b_good = true;
      }
      break;
    default:
      ASSERT(0);
  }

  // Update static text if user has selected a new criterion
  if (rc == IDOK) {
    st_fldata.ltype = ltype;
    CString cs_criteria;
    DWORD_PTR dwData;
    dwData = GetItemData(m_iItem);
    if (b_good) {
      cs_criteria = PWSFilters::GetFilterDescription(st_fldata).c_str();
      dwData &= ~FLC_CRITERIA_REDTXT;
      m_vcbxChanged[m_iItem] = false;
      m_vCriteriaSet[m_iItem] = true;
      st_fldata.bFilterComplete = true;
    } else {
      cs_criteria.LoadString(IDS_NOTDEFINED);
      dwData |= FLC_CRITERIA_REDTXT;
    }
    SetItemData(m_iItem, dwData);
    SetItemText(m_iItem, FLC_CRITERIA_TEXT, cs_criteria);
    SetColumnWidth(FLC_CRITERIA_TEXT, LVSCW_AUTOSIZE_USEHEADER);
    m_pPWF->UpdateDialogMaxWidth();
  }

  return true;
}

void CPWFilterLC::SetUpComboBoxData()
{
  // Set up the Field selection Combobox

  // NOTE: The ComboBox strings are NOT sorted by design !
  if (m_vLcbx_data.empty()) {
    st_Lcbxdata stl;
    stl.cs_text.LoadString(IDSC_AND);
    stl.ltype = LC_AND;
    m_vLcbx_data.push_back(stl);

    stl.cs_text.LoadString(IDSC_OR);
    stl.ltype = LC_OR;
    m_vLcbx_data.push_back(stl);
  }

  if (m_vFcbx_data.empty()) {
    CString cs_temp;
    st_Fcbxdata stf;
    switch (m_iType) {
      case DFTYPE_MAIN:
        stf.cs_text = CItemData::FieldName(CItemData::GROUP).c_str();
        stf.ftype = FT_GROUP;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::TITLE).c_str();
        stf.ftype = FT_TITLE;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::GROUPTITLE).c_str();
        stf.ftype = FT_GROUPTITLE;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::USER).c_str();
        stf.ftype = FT_USER;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::PASSWORD).c_str();
        stf.ftype = FT_PASSWORD;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::NOTES).c_str();
        stf.ftype = FT_NOTES;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::URL).c_str();
        stf.ftype = FT_URL;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::AUTOTYPE).c_str();
        stf.ftype = FT_AUTOTYPE;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::RUNCMD).c_str();
        stf.ftype = FT_RUNCMD;
        m_vFcbx_data.push_back(stf);

        //stf.cs_text = CItemData::FieldName(CItemData::DCA).c_str();
        stf.cs_text.LoadString(IDS_DCALONG);
        stf.ftype = FT_DCA;
        m_vFcbx_data.push_back(stf);

        //stf.cs_text = CItemData::FieldName(CItemData::SHIFTDCA).c_str();
        stf.cs_text.LoadString(IDS_SHIFTDCALONG);
        stf.ftype = FT_SHIFTDCA;
        m_vFcbx_data.push_back(stf);

        //stf.cs_text = CItemData::FieldName(CItemData::EMAIL).c_str();
        stf.cs_text.LoadString(IDS_EMAIL);
        stf.ftype = FT_EMAIL;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::SYMBOLS).c_str();
        stf.ftype = FT_SYMBOLS;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::POLICYNAME).c_str();
        stf.ftype = FT_POLICYNAME;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::PROTECTED).c_str();
        stf.ftype = FT_PROTECTED;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::CTIME).c_str();
        stf.ftype = FT_CTIME;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::ATIME).c_str();
        stf.ftype = FT_ATIME;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::PMTIME).c_str();
        stf.ftype = FT_PMTIME;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::XTIME).c_str();
        stf.ftype = FT_XTIME;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::XTIME_INT).c_str();
        stf.ftype = FT_XTIME_INT;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::RMTIME).c_str();
        stf.ftype = FT_RMTIME;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_PASSWORDHISTORY);
        stf.ftype = FT_PWHIST;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::POLICY).c_str();
        stf.ftype = FT_POLICY;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_PASSWORDLEN);
        stf.ftype = FT_PASSWORDLEN;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_KBSHORTCUT);
        stf.ftype = FT_KBSHORTCUT;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.Empty();  // Separator
        stf.ftype = FT_END;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_ENTRYTYPE);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = FT_ENTRYTYPE;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_ENTRYSTATUS);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = FT_ENTRYSTATUS;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_ENTRYSIZE);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = FT_ENTRYSIZE;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_UNKNOWNFIELDSFILTER);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = FT_UNKNOWNFIELDS;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.Empty();  // Separator
        stf.ftype = FT_END;
        m_vFcbx_data.push_back(stf);

        cs_temp.LoadString(IDS_PASSWORDHISTORY);
        stf.cs_text = cs_temp + L" -->";  // Normal 3 dots hard to see
        stf.ftype = FT_PWHIST;
        m_vFcbx_data.push_back(stf);

        stf.cs_text = CItemData::FieldName(CItemData::POLICY).c_str() + CString(L" -->");  // Normal 3 dots hard to see
        stf.ftype = FT_POLICY;
        m_vFcbx_data.push_back(stf);

        // Only add attachment fields if DB has attachments
        if (m_bCanHaveAttachments && m_psMediaTypes != NULL && !m_psMediaTypes->empty()) {
          cs_temp.LoadString(IDS_ATTACHMENTS);
          stf.cs_text = cs_temp + L" -->";  // Normal 3 dots hard to see
          stf.ftype = FT_ATTACHMENT;
          m_vFcbx_data.push_back(stf);
        }
        break;

      case DFTYPE_PWHISTORY:
        stf.cs_text.LoadString(IDS_PRESENT);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = HT_PRESENT;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_HACTIVE);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = HT_ACTIVE;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_HNUM);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = HT_NUM;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_HMAX);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = HT_MAX;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_HDATE);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = HT_CHANGEDATE;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_HPSWD);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = HT_PASSWORDS;
        m_vFcbx_data.push_back(stf);
        break;

      case DFTYPE_PWPOLICY:
        stf.cs_text.LoadString(IDS_PRESENT);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = PT_PRESENT;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDSC_PLENGTH);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = PT_LENGTH;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_PLOWER);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = PT_LOWERCASE;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_PUPPER);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = PT_UPPERCASE;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_PDIGITS);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = PT_DIGITS;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_PSYMBOL);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = PT_SYMBOLS;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDSC_PHEXADECIMAL);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = PT_HEXADECIMAL;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDSC_PEASYVISION);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = PT_EASYVISION;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDSC_PPRONOUNCEABLE);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = PT_PRONOUNCEABLE;
        m_vFcbx_data.push_back(stf);
        break;

      case DFTYPE_ATTACHMENT:
        stf.cs_text.LoadString(IDS_ATTACHMENTS);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = AT_PRESENT;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_FILETITLE);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = AT_TITLE;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_CTIME);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = AT_CTIME;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_FILEMEDIATYPE);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = AT_MEDIATYPE;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_FILENAME);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = AT_FILENAME;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_FILEPATH);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = AT_FILEPATH;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_FILECTIME);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = AT_FILECTIME;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_FILEMTIME);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = AT_FILEMTIME;
        m_vFcbx_data.push_back(stf);

        stf.cs_text.LoadString(IDS_FILEATIME);
        stf.cs_text.TrimRight(L'\t');
        stf.ftype = AT_FILEATIME;
        m_vFcbx_data.push_back(stf);
        break;

      default:
        ASSERT(0);
    }
  }
}

void CPWFilterLC::OnLButtonDown(UINT nFlags, CPoint point)
{
  int iItem = -1;
  int iSubItem = -1;

  LVHITTESTINFO lvhti;
  lvhti.pt = point;
  SubItemHitTest(&lvhti);

  if (lvhti.flags & LVHT_ONITEM) {
    iItem = lvhti.iItem;
    iSubItem = lvhti.iSubItem;
    m_iItem = iItem;
  }

  if ((iItem < 0)    || (iItem >= m_numfilters) ||
      (iSubItem < 0) || (iSubItem >= FLC_NUM_COLUMNS)) {
    CListCtrl::OnLButtonDown(nFlags, point);
  } else {
    FieldType ftype = m_pvfdata->at(m_iItem).ftype;
    DWORD_PTR dwData = GetItemData(m_iItem);
    bool bFilterActive = (dwData & FLC_FILTER_ENABLED) == FLC_FILTER_ENABLED;
    bool bLogicCBXEnabled = (dwData & FLC_LGC_CBX_ENABLED) == FLC_LGC_CBX_ENABLED;
    switch (iSubItem) {
      case FLC_FILTER_NUMBER:
        EnsureVisible(m_iItem, FALSE);
        SetItemState(m_iItem, 0, LVIS_SELECTED);
        break;
      case FLC_ENABLE_BUTTON:
        // Change what user sees - then implement the result
        EnableCriteria();
        break;
      case FLC_ADD_BUTTON:
        AddFilter();
        break;
      case FLC_REM_BUTTON:
        RemoveFilter();
        break;
      case FLC_LGC_COMBOBOX:
        if (bFilterActive && bLogicCBXEnabled && iItem > 0) {
          DrawComboBox(FLC_LGC_COMBOBOX, m_pvfdata->at(m_iItem).ltype == LC_OR ? 1 : 0);
          m_ComboBox.ShowDropDown(TRUE);
        }
        break;
      case FLC_FLD_COMBOBOX:
        if (bFilterActive) {
          DrawComboBox(FLC_FLD_COMBOBOX, (int)ftype);
          m_ComboBox.ShowDropDown(TRUE);
        }
        break;
      case FLC_CRITERIA_TEXT:
        dwData = GetItemData(m_iItem);
        if ((dwData & FLC_FILTER_ENABLED) &&
            (dwData & FLC_FLD_CBX_SET))
          GetCriterion();
        break;
      default:
        break;
    }
  }
  if (m_iItem >= 0)
    SetItemState(m_iItem, 0, LVIS_SELECTED | LVIS_DROPHILITED);

  Invalidate();
}

void CPWFilterLC::DrawComboBox(const int iSubItem, const int index)
{
  // Draw the drop down list
  int iItem;
  CRect rect;
  GetSubItemRect(m_iItem, iSubItem, LVIR_BOUNDS, rect);

  UINT nID;
  if (iSubItem == FLC_FLD_COMBOBOX)
    nID = m_FLD_ComboID;
  else if (iSubItem == FLC_LGC_COMBOBOX)
    nID = m_LGC_ComboID;
  else {
    ASSERT(0);
    return;
  }

  rect.top -= 1;
  CRect m_rectComboBox(rect);
  m_rectComboBox.left += 1;

  m_ComboBox.SetDlgCtrlID(nID);
  m_ComboBox.SetWindowPos(NULL, m_rectComboBox.left, m_rectComboBox.top, 0, 0,
    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

  if (!m_pFont) {
    CFont *pFont = GetFont();
    ASSERT(pFont);

    LOGFONT logFont;
    SecureZeroMemory(&logFont, sizeof(logFont));

    pFont->GetLogFont(&logFont);
    m_pFont = new CFont;
    m_pFont->CreateFontIndirect(&logFont);
  }
  m_ComboBox.SetFont(m_pFont);

  // add strings & data to ComboBox
  m_vWCFcbx_data = m_vFcbx_data;

  if (m_iType == DFTYPE_MAIN && iSubItem == FLC_FLD_COMBOBOX) {
    // Is PW History, Policy or Attachment already selected somewhere else?
    //  if so remove them as an option on this filter
    st_FilterRow &st_fldata = m_pvfdata->at(m_iItem);
    if (m_bPWHIST_Set && st_fldata.ftype != FT_PWHIST)
      DeleteEntry(FT_PWHIST);
    if (m_bPOLICY_Set && st_fldata.ftype != FT_POLICY)
      DeleteEntry(FT_POLICY);
    if (m_bATTACHMENT_Set && st_fldata.ftype != FT_ATTACHMENT)
      DeleteEntry(FT_ATTACHMENT);
  }

  switch (iSubItem) {
    case FLC_FLD_COMBOBOX:
    {
      for (size_t i = 0; i < m_vWCFcbx_data.size(); i++) {
        if (m_vWCFcbx_data[i].ftype != FT_END) {
          iItem = m_ComboBox.AddString(m_vWCFcbx_data[i].cs_text);
          m_ComboBox.SetItemData(iItem, m_vWCFcbx_data[i].ftype);
        } else {
          m_ComboBox.SetSeparator();
        }
      }
      break;
    }
    case FLC_LGC_COMBOBOX:
      for (int i = 0; i < (int)m_vLcbx_data.size(); i++) {
        iItem = m_ComboBox.AddString(m_vLcbx_data[i].cs_text);
        m_ComboBox.SetItemData(iItem, (int)m_vLcbx_data[i].ltype);
      }
      break;
    default:
      ASSERT(0);
  }

  // Find widths and height - only the first time this is called for each.
  // In fact, called twice during 'Init' processing just for this purpose
  // once for each Combobox
  SetComboBoxWidth(iSubItem);
  
  if (m_rowheight < 0) {
    // Set row height to take image by adding a dummy ImageList
    // Good trick - save making ComboBox "ownerdraw"
    CRect combo_rect;
    m_ComboBox.GetClientRect(&combo_rect);
    IMAGEINFO imageinfo;
    m_pCheckImageList->GetImageInfo(0, &imageinfo);
    m_rowheight = std::max(combo_rect.Height(),
      (int)abs(imageinfo.rcImage.top - imageinfo.rcImage.bottom));

    m_pImageList = new CImageList;
    m_pImageList->Create(1, m_rowheight, ILC_COLOR4, 1, 1);
    SetImageList(m_pImageList, LVSIL_SMALL);
  }

  // Since now reusing control- have to reset size
  SetColumnWidth(iSubItem, iSubItem == FLC_FLD_COMBOBOX ? m_fwidth : m_lwidth);

  // Try to ensure that dropdown list is big enough for half the entries (fits
  // in the same height as the dialog) but add vertical scrolling
  int n = m_ComboBox.GetCount();

  int ht = m_ComboBox.GetItemHeight(0);
  m_ComboBox.GetWindowRect(&rect);

  CRect wrc;
  GetWindowRect(&wrc);

  CSize sz;
  sz.cx = (iSubItem == FLC_FLD_COMBOBOX) ? m_fwidth : m_lwidth;
  sz.cy = ht * (n + 2);

  if (ht * n >= wrc.Height()) {
    // Only resize if combobox listctrl won't fit in dialog
    sz.cy = ht * ((n / 2) + 2);

    if ((rect.top - sz.cy) < 0 ||
        (rect.bottom + sz.cy > ::GetSystemMetrics(SM_CYSCREEN))) {
      int ifit = max((rect.top / ht), (::GetSystemMetrics(SM_CYSCREEN) - rect.bottom) / ht);
      int ht2 = ht * ifit;
      sz.cy = std::min((long)ht2, sz.cy);
    }
  }

  m_ComboBox.SetWindowPos(NULL, 0, 0, sz.cx, sz.cy, SWP_NOMOVE | SWP_NOZORDER);

  int nindex(index);
  if (nindex >= n || nindex < 0)
    nindex = -1;
  else {
    if (iSubItem == FLC_FLD_COMBOBOX) {
      std::vector<st_Fcbxdata>::iterator Fcbxdata_iter;
      Fcbxdata_iter = std::find_if(m_vWCFcbx_data.begin(), m_vWCFcbx_data.end(),
                                   equal_ftype((FieldType)index));
      if (Fcbxdata_iter == m_vWCFcbx_data.end())
        nindex = -1;
      else
        nindex = (int)(Fcbxdata_iter - m_vWCFcbx_data.begin());
    }
  }

  m_ComboBox.SetCurSel(nindex);

  if (index >= 0) {
    // Don't bother to show during first initialisation calls
    m_ComboBox.EnableWindow(TRUE);
    m_ComboBox.ShowWindow(SW_SHOW);
    m_ComboBox.BringWindowToTop();
  }
}

void CPWFilterLC::SetComboBoxWidth(const int iSubItem)
{
  // Find the longest string in the combo box.
  CString str;
  CSize sz;
  int dx = 0;
  TEXTMETRIC tm;
  CDC *pDC = m_ComboBox.GetDC();
  CFont *pFont = m_ComboBox.GetFont();

  // Select the listbox font, save the old font
  CFont *pOldFont = pDC->SelectObject(pFont);

  // Get the text metrics for avg char width
  pDC->GetTextMetrics(&tm);

  for (int i = 0; i < m_ComboBox.GetCount(); i++) {
    m_ComboBox.GetLBText(i, str);
    sz = pDC->GetTextExtent(str);

    // Add the avg width to prevent clipping
    sz.cx += tm.tmAveCharWidth;

    if (sz.cx > dx)
      dx = sz.cx;
  }

  // Select the old font back into the DC
  pDC->SelectObject(pOldFont);
  m_ComboBox.ReleaseDC(pDC);

  // Adjust the width for the vertical scroll bar and the left and right border.
  dx += ::GetSystemMetrics(SM_CXVSCROLL) + 2 * ::GetSystemMetrics(SM_CXEDGE);

  // Now set column widths
  if (iSubItem == FLC_FLD_COMBOBOX) {
    m_fwidth = max(dx, GetColumnWidth(FLC_FLD_COMBOBOX));
    SetColumnWidth(FLC_FLD_COMBOBOX, m_fwidth);
  } else {
    m_lwidth = max(dx, GetColumnWidth(FLC_LGC_COMBOBOX));
    SetColumnWidth(FLC_LGC_COMBOBOX, m_lwidth);
  }

  // If the width of the list box is too small, adjust it so that every
  // item is completely visible.
  m_ComboBox.SetDroppedWidth(dx);
}

void CPWFilterLC::DeleteEntry(FieldType ftype)
{
  // User has selected Password History or Policy
  // Remove it from the Combobox data - Working Copy only!
  std::vector<st_Fcbxdata>::iterator Fcbxdata_iter;
  Fcbxdata_iter = std::find_if(m_vWCFcbx_data.begin(), m_vWCFcbx_data.end(),
                               equal_ftype(ftype));

  // Check if already deleted - should never happen!
  if (Fcbxdata_iter != m_vWCFcbx_data.end())
    m_vWCFcbx_data.erase(Fcbxdata_iter);
}

void CPWFilterLC::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNotifyStruct);

  *pLResult = CDRF_DODEFAULT;
  const int iItem = (int)pLVCD->nmcd.dwItemSpec;
  const int iSubItem = pLVCD->iSubItem;
  const DWORD_PTR dwData = pLVCD->nmcd.lItemlParam;
  bool bFilterActive = (dwData & FLC_FILTER_ENABLED) == FLC_FILTER_ENABLED;
  bool bLogicCBXEnabled = (dwData & FLC_LGC_CBX_ENABLED) == FLC_LGC_CBX_ENABLED;

  int ix, iy;

  switch(pLVCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      *pLResult = CDRF_NOTIFYITEMDRAW;
      break;
    case CDDS_ITEMPREPAINT:
      *pLResult = CDRF_NOTIFYSUBITEMDRAW;
      break;
    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      {
        CRect rect;
        GetSubItemRect(iItem, iSubItem, LVIR_BOUNDS, rect);
        if (rect.top < 0) {
          *pLResult = CDRF_SKIPDEFAULT;
          break;
        }
        if (iSubItem == 0) {
          CRect rect1;
          GetSubItemRect(iItem, 1, LVIR_BOUNDS, rect1);
          rect.right = rect1.left;
        }
        pLVCD->clrText = m_crWindowText;
        pLVCD->clrTextBk = GetTextBkColor();
        CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
        CRect inner_rect(rect), first_rect(rect);
        inner_rect.DeflateRect(2, 2);
        switch (iSubItem) {
          case FLC_FILTER_NUMBER:
            first_rect.DeflateRect(1, 1);
            pDC->FillSolidRect(&first_rect, m_crButtonFace);
            DrawSubItemText(iItem, iSubItem, pDC, 
                            m_iItem == iItem ? m_crRedText : m_crWindowText, 
                            m_crButtonFace, inner_rect, 
                            m_iItem == iItem, true);
            *pLResult = CDRF_SKIPDEFAULT;
            break;
          case FLC_ENABLE_BUTTON:
            // Draw checked/unchecked image
            ix = inner_rect.CenterPoint().x;
            iy = inner_rect.CenterPoint().y;
            // The '7' below is ~ half the bitmap size of 13.
            inner_rect.SetRect(ix - 7, iy - 7, ix + 7, iy + 7);
            DrawImage(pDC, inner_rect, bFilterActive ? CHECKEDLC : UNCHECKEDLC);
            *pLResult = CDRF_SKIPDEFAULT;
            break;
          case FLC_ADD_BUTTON:
          case FLC_REM_BUTTON:
            // Always bold
            DrawSubItemText(iItem, iSubItem, pDC, m_crWindowText, m_crWindow,
                            inner_rect, true, false);
            *pLResult = CDRF_SKIPDEFAULT;
            break;
          case FLC_LGC_COMBOBOX:
            // Greyed out if filter inactive or logic not set
            if (!bFilterActive || !bLogicCBXEnabled) {
              pLVCD->clrText = m_crGrayText;
            }
            break;
          case FLC_FLD_COMBOBOX:
            // Greyed out if filter inactive or field not set
            if (!bFilterActive) {
              pLVCD->clrText = m_crGrayText;
            }
            break;
          case FLC_CRITERIA_TEXT:
            // Greyed out if filter inactive
            if (!bFilterActive) {
              pLVCD->clrText = m_crGrayText;
            } else if (dwData & FLC_CRITERIA_REDTXT) {
              // Red text if criteria not set
              pLVCD->clrText = m_crRedText;
            }
            break;
          default:
            break;
        }
      }
      break;
    default:
      break;
  }
}

void CPWFilterLC::DrawSubItemText(int iItem, int iSubItem, CDC *pDC,
                                  COLORREF crText, COLORREF crBkgnd,
                                  CRect &rect, bool bBold, bool bOpaque)
{
  CRect rectText(rect);
  CString str = GetItemText(iItem, iSubItem);

  if (!str.IsEmpty()) {
    HDITEM hditem;
    hditem.mask = HDI_FORMAT;
    m_pHeaderCtrl->GetItem(iSubItem, &hditem);
    int nFmt = hditem.fmt & HDF_JUSTIFYMASK;
    UINT nFormat = DT_VCENTER | DT_SINGLELINE;
    if (nFmt == HDF_CENTER)
      nFormat |= DT_CENTER;
    else if (nFmt == HDF_LEFT)
      nFormat |= DT_LEFT;
    else
      nFormat |= DT_RIGHT;

    CFont *pOldFont = NULL;
    CFont boldfont;

    if (bBold) {
      CFont *pFont = pDC->GetCurrentFont();
      if (pFont) {
        LOGFONT lf;
        pFont->GetLogFont(&lf);
        lf.lfWeight = FW_BOLD;
        boldfont.CreateFontIndirect(&lf);
        pOldFont = pDC->SelectObject(&boldfont);
      }
    }
    pDC->SetBkMode(bOpaque ? OPAQUE : TRANSPARENT);
    pDC->SetTextColor(crText);
    pDC->SetBkColor(crBkgnd);
    pDC->DrawText(str, &rectText, nFormat);
    if (pOldFont)
      pDC->SelectObject(pOldFont);
  }
}

void CPWFilterLC::DrawImage(CDC *pDC, CRect &rect, CheckImageLC nImage)
{
  // Draw check image in given rectangle
  if (rect.IsRectEmpty() || nImage < 0) {
    return;
  }

  if (m_pCheckImageList) {
    SIZE sizeImage = {0, 0};
    IMAGEINFO info;

    if (m_pCheckImageList->GetImageInfo(nImage, &info)) {
      sizeImage.cx = info.rcImage.right - info.rcImage.left;
      sizeImage.cy = info.rcImage.bottom - info.rcImage.top;
    }

    if (nImage >= 0) {
      if (rect.Width() > 0) {
        POINT point;

        point.y = rect.CenterPoint().y - (sizeImage.cy >> 1);
        point.x = rect.left;

        SIZE size;
        size.cx = rect.Width() < sizeImage.cx ? rect.Width() : sizeImage.cx;
        size.cy = rect.Height() < sizeImage.cy ? rect.Height() : sizeImage.cy;

#if _MSC_VER == 1600
        // Due to a bug in VS2010 MFC the following does not work!
        //   m_pCheckImageList->DrawIndirect(pDC, nImage, point, size, CPoint(0, 0));
        // So do it the hard way!
        IMAGELISTDRAWPARAMS imldp = {0};
        imldp.cbSize = sizeof(imldp);
        imldp.i = nImage;
        imldp.hdcDst = pDC->m_hDC;
        imldp.x = point.x;
        imldp.y = point.y;
        imldp.xBitmap = imldp.yBitmap = 0;
        imldp.cx = size.cx;
        imldp.cy = size.cy;
        imldp.fStyle = ILD_NORMAL;
        imldp.dwRop = SRCCOPY;
        imldp.rgbBk = CLR_DEFAULT;
        imldp.rgbFg = CLR_DEFAULT;
        imldp.fState = ILS_NORMAL;
        imldp.Frame = 0;
        imldp.crEffect = CLR_DEFAULT;
        m_pCheckImageList->DrawIndirect(&imldp);
#else
        m_pCheckImageList->DrawIndirect(pDC, nImage, point, size, CPoint(0, 0));
#endif
      }
    }
  }
}

INT_PTR CPWFilterLC::OnToolHitTest(CPoint point, TOOLINFO *pTI) const
{
  LVHITTESTINFO lvhti;
  lvhti.pt = point;

  ListView_SubItemHitTest(this->m_hWnd, &lvhti);
  int nSubItem = lvhti.iSubItem;

  // nFlags is 0 if the SubItemHitTest fails
  // Therefore, 0 & <anything> will equal false
  if (lvhti.flags & LVHT_ONITEMLABEL) {
    // get the client (area occupied by this control
    RECT rcClient;
    GetClientRect(&rcClient);

    // fill in the TOOLINFO structure
    pTI->hwnd = m_hWnd;
    pTI->uId = (UINT) (nSubItem + 1);
    pTI->lpszText = LPSTR_TEXTCALLBACK;
    pTI->rect = rcClient;

    return pTI->uId;  // By returning a unique value per listItem,
              // we ensure that when the mouse moves over another
              // list item, the tooltip will change
  } else {
    //Otherwise, we aren't interested, so let the message propagate
    return -1;
  }
}

BOOL CPWFilterLC::OnToolTipText(UINT /*id*/, NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  UINT_PTR nID = pNotifyStruct->idFrom;

  // check if this is the automatic tooltip of the control
  if (nID == 0) 
    return TRUE;  // do not allow display of automatic tooltip,
                  // or our tooltip will disappear

  TOOLTIPTEXTW *pTTTW = (TOOLTIPTEXTW *)pNotifyStruct;

  *pLResult = 0;

  // get the mouse position
  const MSG* pMessage;
  pMessage = GetCurrentMessage();
  ASSERT(pMessage);
  CPoint pt;
  pt = pMessage->pt;    // get the point from the message
  ScreenToClient(&pt);  // convert the point's coords to be relative to this control

  // see if the point falls onto a list item
  LVHITTESTINFO lvhti;
  lvhti.pt = pt;
  
  SubItemHitTest(&lvhti);
  int nSubItem = lvhti.iSubItem;
  
  // nFlags is 0 if the SubItemHitTest fails
  // Therefore, 0 & <anything> will equal false
  if (lvhti.flags & LVHT_ONITEMLABEL) {
    // If it did fall on a list item,
    // and it was also hit one of the
    // item specific subitems we wish to show tooltips for
    
    switch (nSubItem) {
      case FLC_FILTER_NUMBER:
        nID = IDS_FLC_TOOLTIP0;
        break;
      case FLC_ENABLE_BUTTON:
        nID = IDS_FLC_TOOLTIP1;
        break;
      case FLC_ADD_BUTTON:
        nID = IDS_FLC_TOOLTIP2;
        break;
      case FLC_REM_BUTTON:
        nID = IDS_FLC_TOOLTIP3;
        break;
      case FLC_LGC_COMBOBOX:
        nID = IDS_FLC_TOOLTIP4;
        break;
      case FLC_FLD_COMBOBOX:
        nID = IDS_FLC_TOOLTIP5;
        break;
      case FLC_CRITERIA_TEXT:
        nID = IDS_FLC_TOOLTIP6;
        break;
      default:
        return FALSE;
    }

    // If there was a CString associated with the list item,
    // copy it's text (up to 80 characters worth, limitation 
    // of the TOOLTIPTEXT structure) into the TOOLTIPTEXT 
    // structure's szText member
    CString cs_TipText(MAKEINTRESOURCE(nID));
    delete m_pwchTip;

    m_pwchTip = new WCHAR[cs_TipText.GetLength() + 1];
    wcsncpy_s(m_pwchTip, cs_TipText.GetLength() + 1,
                cs_TipText, _TRUNCATE);
    pTTTW->lpszText = (LPWSTR)m_pwchTip;

    return TRUE;   // we found a tool tip,
  }
  
  return FALSE;  // we didn't handle the message, let the 
                 // framework continue propagating the message
}

void CPWFilterLC::OnProcessKey(UINT nID)
{
  /*
    Ctrl+S      Select filter
    Up arrow    Select previous
    Down Arrow  Select next
    Ctrl+E      Enable selected filter
    Ctrl+F      Select Field
    Ctrl+C      Select Criteria
    Ctrl+L      Select Logic (And/Or)
    Delete      Delete selected filter
    Insert      Add after selected filter
  */

  POSITION pos;
  DWORD_PTR dwData(0);
  if (m_iItem >=0 && m_iItem < GetItemCount())
    dwData = GetItemData(m_iItem);

  switch (nID) {
    case ID_FLC_CRITERIA:
      if (m_iItem >= 0) {
        if ((dwData & FLC_FILTER_ENABLED) &&
            (dwData & FLC_FLD_CBX_SET) &&
            (dwData & FLC_FLD_CBX_ENABLED))
          GetCriterion();
      }
      break;
    case ID_FLC_ENABLE:
      if (m_iItem >= 0)
        EnableCriteria();
      break;
    case ID_FLC_FIELD:
      if (m_iItem >= 0) {
        if (dwData & FLC_FILTER_ENABLED) {
          m_bSetFieldActive = true;
          DrawComboBox(FLC_FLD_COMBOBOX, (int)m_pvfdata->at(m_iItem).ftype);
          m_ComboBox.ShowDropDown(TRUE);
          m_ComboBox.SetFocus();
        }
      }
      break;
    case ID_FLC_LOGIC:
      if (m_iItem > 0) {
        if ((dwData & FLC_FILTER_ENABLED) && 
            (dwData & FLC_LGC_CBX_ENABLED)) {
          m_bSetLogicActive = true;
          DrawComboBox(FLC_LGC_COMBOBOX, m_pvfdata->at(m_iItem).ltype == LC_OR ? 1 : 0);
          m_ComboBox.ShowDropDown(TRUE);
          m_ComboBox.SetFocus();
        }
      }
      break;
    case ID_FLC_SELECT:
      if (m_iItem < 0)
        m_iItem = 0;
      pos = GetFirstSelectedItemPosition();
      if (pos > 0)
        m_iItem = (int)(INT_PTR)pos - 1;
      EnsureVisible(m_iItem, FALSE);
      SetItemState(m_iItem, 0, LVIS_SELECTED);
      break;
    case ID_FLC_DELETE:
      if (m_iItem >= 0) {
        RemoveFilter();
        if (m_iItem > 0)
          m_iItem--;
      }
      break;
    case ID_FLC_INSERT:
      if (m_iItem >= 0) {
        m_iItem = AddFilter();
        if (m_iItem >= 0) {
          EnsureVisible(m_iItem, FALSE);
        }
      }
      break;
    case ID_FLC_PREVIOUS:
      if (m_bSetFieldActive || m_bSetLogicActive)
        return;

      if (m_iItem <= 0)
        m_iItem = 0;
      else
        m_iItem--;
      EnsureVisible(m_iItem, FALSE);
      break;
    case ID_FLC_NEXT:
      if (m_bSetFieldActive || m_bSetLogicActive)
        return;

      if (m_iItem >= GetItemCount() - 1)
        m_iItem = GetItemCount() - 1;
      else
        m_iItem++;
      EnsureVisible(m_iItem, FALSE);
      break;
    default:
      ASSERT(0);
  }
  Invalidate();
}
