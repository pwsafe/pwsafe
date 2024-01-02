/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWFiltersEditors.cpp
* 
*/

////@begin includes

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include "wx/string.h"
#include "wx/grid.h"

#include "core/core.h"
#include "core/ItemData.h"
#include "PWFiltersEditor.h"
#include "core/PWSFilters.h"
#include "wxUtilities.h"

#include "SetFiltersDlg.h"
////@end includes

////@begin XPM images
#include "graphics/empty.xpm"
#include "graphics/empty_disabled.xpm"
#include "graphics/checked.xpm"
#include "graphics/checked_disabled.xpm"
#include "graphics/unchecked.xpm"
#include "graphics/unchecked_disabled.xpm"
#if wxVERSION_NUMBER >= 3103
// Same in dark
#include "graphics/empty_dark.xpm"
#include "graphics/empty_disabled_dark.xpm"
#include "graphics/checked_dark.xpm"
#include "graphics/checked_disabled_dark.xpm"
#include "graphics/unchecked_dark.xpm"
#include "graphics/unchecked_disabled_dark.xpm"
#endif
////@end XPM images

// The values of Filters Active Renderer is index into image list
static wxString CheckTypeStringValues[MAX_PWF_ELEMENTS] = { L"0", L"1", L"2", L"3", L"4", L"5" };

static const wxArrayString CheckTypeString(MAX_PWF_ELEMENTS, CheckTypeStringValues);

// Map between FilterType and string
const std::map<int, wxString> FieldTypeString = {
  {FT_GROUPTITLE, towxstring(CItemData::EngFieldName(CItemData::GROUPTITLE))},
  {FT_GROUP, towxstring(CItemData::EngFieldName(CItemData::GROUP))},
  {FT_TITLE, towxstring(CItemData::EngFieldName(CItemData::TITLE))},
  {FT_USER, towxstring(CItemData::EngFieldName(CItemData::USER))},
  {FT_NOTES, towxstring(CItemData::EngFieldName(CItemData::NOTES))},
  {FT_PASSWORD, towxstring(CItemData::EngFieldName(CItemData::PASSWORD))},
  {FT_CTIME, towxstring(CItemData::EngFieldName(CItemData::CTIME))},
  {FT_PMTIME, towxstring(CItemData::EngFieldName(CItemData::PMTIME))},
  {FT_ATIME, towxstring(CItemData::EngFieldName(CItemData::ATIME))},
  {FT_XTIME, towxstring(CItemData::EngFieldName(CItemData::XTIME))},
  {FT_RMTIME, towxstring(CItemData::EngFieldName(CItemData::RMTIME))},
  {FT_URL, towxstring(CItemData::EngFieldName(CItemData::URL))},
  {FT_AUTOTYPE, towxstring(CItemData::EngFieldName(CItemData::AUTOTYPE))},
  {FT_PWHIST, towxstring(CItemData::EngFieldName(CItemData::PWHIST))},
  {FT_POLICY, towxstring(CItemData::EngFieldName(CItemData::POLICY))},
  {FT_XTIME_INT, towxstring(CItemData::EngFieldName(CItemData::XTIME_INT))},
  {FT_RUNCMD, towxstring(CItemData::EngFieldName(CItemData::RUNCMD))},
  {FT_DCA, towxstring(CItemData::EngFieldName(CItemData::DCA))},
  {FT_SHIFTDCA, towxstring(CItemData::EngFieldName(CItemData::SHIFTDCA))},
  {FT_EMAIL, towxstring(CItemData::EngFieldName(CItemData::EMAIL))},
  {FT_PROTECTED, towxstring(CItemData::EngFieldName(CItemData::PROTECTED))},
  {FT_SYMBOLS, towxstring(CItemData::EngFieldName(CItemData::SYMBOLS))},
  {FT_POLICYNAME, towxstring(CItemData::EngFieldName(CItemData::POLICYNAME))},
  {FT_KBSHORTCUT, towxstring(CItemData::EngFieldName(CItemData::KBSHORTCUT))},
  {FT_END, _T("------")}, // Separator
  {FT_ENTRYSIZE, _("Entry size")},
  {FT_ENTRYTYPE, _("Entry type")},
  {FT_ENTRYSTATUS, _("Entry status")},
  {FT_UNKNOWNFIELDS, _("Unknown fields")},
  {FT_PASSWORDLEN, _("Password Length")},
  {HT_PRESENT, _("Field present")},
  {HT_ACTIVE, _("Field active")},
  {HT_NUM, _("Number stored")},
  {HT_MAX, _("Maximal Number stored")},
  {HT_CHANGEDATE, _("Change Date")},
  {HT_PASSWORDS, _("Previous Passwords")},
  {HT_END, _T("------")}, // Separator
  {PT_PRESENT, _("Field present")},
  {PT_LENGTH, _("Password length")},
  {PT_LOWERCASE, _("Minimum lowercase characters")},
  {PT_UPPERCASE, _("Minimum uppercase characters")},
  {PT_DIGITS, _("Minimum digits")},
  {PT_SYMBOLS, _("Minimum symbols")},
  {PT_EASYVISION, _("Easyvision characters")},
  {PT_PRONOUNCEABLE, _("Pronounceable passwords")},
  {PT_HEXADECIMAL, _("Hexadecimal characters")},
  {PT_END, _T("------")}, // Separator
  {AT_PRESENT, _("Attachment")},
  {AT_TITLE, _("Title")},
  {AT_CTIME, _("Date added")},
  {AT_MEDIATYPE, _("Media Type")},
  {AT_FILENAME, _("File Name")},
  {AT_FILEPATH, _("File Path")},
  {AT_FILECTIME, _("File Creation Date")},
  {AT_FILEMTIME, _("File Last Modified Date")},
  {AT_FILEATIME, _("File Last Accessed date")},
  {AT_END, _T("------")}, // Separator
  {FT_ATTACHMENT, _("Attachment")},
  {FT_INVALID, _("Click here to pick a field")},
};


/*!
 *  Return pre-defined text as return value and type wxString - do not translate, is done in LoadAString from core
 */

const wxString LoadAString(int id)
{
  stringT str;
  LoadAString(str, id);
  return towxstring(str);
}

/*!
 * Export string related to FieldType
 */

wxString pwFiltersFTChoiceRenderer::getFieldTypeString(int ft)
{
  std::map<int, wxString>::const_iterator iter = FieldTypeString.find(ft);
  ASSERT(iter != FieldTypeString.end());
  return _(iter->second);
}

/*!
 * Creation of pwFiltersActiveRenderer::m_images and other static fields of classes
 */
bool pwFiltersActiveRenderer::m_initialied = false;
wxImageList pwFiltersActiveRenderer::m_images = { };

wxSize pwFiltersActiveRenderer::m_bestSize = wxDefaultSize;
wxSize pwFiltersFTChoiceRenderer::m_bestSize = wxDefaultSize;
wxSize pwFiltersLCChoiceRenderer::m_bestSize = wxDefaultSize;

int pwFiltersActiveRenderer::m_fontHeight = PWF_DEF_FONT_HEIGTH;

/*!
 * Control creation for pwFiltersActiveRenderer
 */

void pwFiltersActiveRenderer::CreateControls()
{
  // As the list is static it has to be build up only at first time
  if(m_initialied == false) {
    static const char* const* const xpmPWFList[] = {
      empty_xpm,              // 0
      checked_xpm,            // 1
      unchecked_xpm,          // 2
      empty_disabled_xpm,     // 3
      checked_disabled_xpm,   // 4
      unchecked_disabled_xpm, // 5
    };
#if wxVERSION_NUMBER >= 3103
    static const char* const* const xpmPWFDarkList[] = {
      empty_dark_xpm,              // 0
      checked_dark_xpm,            // 1
      unchecked_dark_xpm,          // 2
      empty_disabled_dark_xpm,     // 3
      checked_disabled_dark_xpm,   // 4
      unchecked_disabled_dark_xpm, // 5
    };
#endif

    constexpr int Nimages = sizeof(xpmPWFList)/sizeof(xpmPWFList[0]);
#if wxVERSION_NUMBER >= 3103
    const bool bIsDark = wxSystemSettings::GetAppearance().IsUsingDarkBackground();
    wxASSERT(Nimages == (sizeof(xpmPWFDarkList)/sizeof(xpmPWFDarkList[0])));
#endif
    int pixels = 13; // Default number of pixels in xpm
    if(m_fontHeight >= 17) {
      // Adapt pixel size of image in case the font height is bigger than default
      pixels = m_fontHeight - 2;
    }
    m_images.Create(pixels, pixels, false, Nimages);
    for (int i = 0; i < Nimages; i++) {
      if(pixels == 13) {
#if wxVERSION_NUMBER >= 3103
        m_images.Add(wxBitmap(bIsDark ? xpmPWFDarkList[i] : xpmPWFList[i]));
#else
        m_images.Add(wxBitmap(xpmPWFList[i]));
#endif
      }
      else {
#if wxVERSION_NUMBER >= 3103
        wxImage image(bIsDark ? xpmPWFDarkList[i] : xpmPWFList[i]);
#else
        wxImage image(xpmPWFList[i]);
#endif
        image.Rescale(pixels, pixels, wxIMAGE_QUALITY_HIGH);
        m_images.Add(wxBitmap(image));
      }
    }
    m_initialied = true;
  }
}

/*!
 * GetCellValueOfCheckType returns string value related to the check type
 */

const wxString pwFiltersActiveRenderer::GetCellValueOfCheckType(PWFCheckType ct)
{
  ASSERT(ct < MAX_PWF_ELEMENTS);
  return CheckTypeString[ct];
}

/*!
 * pwFiltersActiveRenderer override implementations
 * ----------------------------------------------------------
 */

/*!
 * Draw the icon, according the index
 */

void pwFiltersActiveRenderer::Draw(wxGrid& grid,
                  wxGridCellAttr& attr,
                  wxDC& dc,
                  const wxRect& rect,
                  int row, int col,
                  bool isSelected)
{
  wxString sValue = grid.GetCellValue(row, col);
  // normal drawing
  wxGridCellRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);
  // Remove by one for the border
  wxRect rectCell = rect;
  rectCell.Inflate(-1);
  dc.SetClippingRegion( rectCell );
  long num = PWF_UNCHECKED;
  
  if(sValue.empty()) {
    num = PWF_UNCHECKED;
  }
  else if(! sValue.ToLong(&num)) {
    wxFAIL_MSG( wxT("this cell doesn't have numeric value") );
    num = PWF_UNCHECKED;
  }
  int width, height;
  if(! m_images.GetSize(static_cast<int>(num), width, height)) {
    width = 13;
    height = 13;
  }
  // adjust x and y-offset of image to keep in vertical mid
  m_images.Draw(static_cast<int>(num), dc, rect.x + ((rect.width - width) / 2),rect.y + ((rect.height - height) / 2), wxIMAGELIST_DRAW_NORMAL | (isSelected ? wxIMAGELIST_DRAW_SELECTED : 0));
  dc.DestroyClippingRegion();  
}

/*!
 * GetBestSize: return the icon extent
 */

wxSize pwFiltersActiveRenderer::GetBestSize(wxGrid& grid,
                           wxGridCellAttr& attr,
                           wxDC& dc,
                           int WXUNUSED(row), int WXUNUSED(col))
{
  if(m_bestSize.GetWidth() <= 0 ||  m_bestSize.GetHeight() <= 0) {
    m_bestSize.Set(17,17); // Assume 13x13 and add 2 pixel for the border
    int width, height;
    if(m_images.GetImageCount() > 0 && m_images.GetSize(0, width, height)) {
      m_bestSize.Set(width + 4, height + 4);
    }
  }
  return m_bestSize;
}

#if wxVERSION_NUMBER >= 3100

/*!
 * GetBestHeight: return the icon extent hight (is independent from width)
 */

int pwFiltersActiveRenderer::GetBestHeight(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                            int row, int col, int WXUNUSED(width))
{
  (void) GetBestSize(grid, attr, dc, row, col);
  return m_bestSize.GetHeight();
}

/*!
 * GetBestWidth: return the icon extent width (is independent from hight)
 */

int pwFiltersActiveRenderer::GetBestWidth(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                                          int row, int col, int WXUNUSED(height))
{
  (void) GetBestSize(grid, attr, dc, row, col);
  return m_bestSize.GetWidth();
}
#endif

#if wxVERSION_NUMBER >= 3104

/*!
 * GetMaxBestSize: return the icon extent
 */

wxSize pwFiltersActiveRenderer::GetMaxBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc)
{
  return GetBestSize(grid, attr, dc, 0, 0);
}
#endif

/*!
 * pwFiltersFTChoiceRenderer override implementations
 * --------------------------------------------------------------
 */

/*!
 * Draw the Filter Type String, according the index
 */

void pwFiltersFTChoiceRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                                     const wxRect& rect, int row, int col, bool isSelected)
{
  wxGridCellRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

  SetTextColoursAndFont(grid, attr, dc, isSelected);

  // draw the text left aligned by default
  attr.SetAlignment(wxALIGN_LEFT, wxALIGN_CENTER);

  wxRect rectCell = rect;
  rectCell.Inflate(-1);

  grid.DrawTextRectangle(dc, GetString(grid, row, col), rectCell, wxALIGN_LEFT, wxALIGN_CENTER);
}

/*!
 * Clone implementation
 */
    
pwFiltersFTChoiceRenderer *pwFiltersFTChoiceRenderer::Clone() const
{
  return new pwFiltersFTChoiceRenderer();
}

/*!
 * GetBestSize: return the text best size
 */

wxSize pwFiltersFTChoiceRenderer::GetBestSize(wxGrid& WXUNUSED(grid), wxGridCellAttr& attr, wxDC& dc, int WXUNUSED(row), int WXUNUSED(col))
{
  if(m_bestSize.GetWidth() <= 0 ||  m_bestSize.GetHeight() <= 0) {
    m_bestSize.Set(50, 15);
  
    /* C++17 like
    for(const auto& [key, value] : FieldTypeString) {
      m_bestSize.IncTo(DoGetBestSize(attr, dc, _(value)));
    }
     */
    for(const auto& kv : FieldTypeString) {
      m_bestSize.IncTo(DoGetBestSize(attr, dc, _(kv.second)));
    }
  }
  return m_bestSize;
}

#if wxVERSION_NUMBER >= 3100

/*!
 * GetBestHeight: return the test size maximum hight (is independent from width)
 */

int pwFiltersFTChoiceRenderer::GetBestHeight(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                            int row, int col, int WXUNUSED(width))
{
  (void) GetBestSize(grid, attr, dc, row, col);
  return m_bestSize.GetHeight();
}

/*!
 * GetBestWidth: return the text size maximum width (is independent from hight)
 */

int pwFiltersFTChoiceRenderer::GetBestWidth(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                           int row, int col, int WXUNUSED(height))
{
  (void) GetBestSize(grid, attr, dc, row, col);
  return m_bestSize.GetWidth();
}
#endif

#if wxVERSION_NUMBER >= 3104

/*!
 * GetMaxBestSize: return the text size maximum
 */

wxSize pwFiltersFTChoiceRenderer::GetMaxBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc)
{
  return GetBestSize(grid, attr, dc, 0, 0);
}
#endif

/*!
 * GetString: return the string at actual row (and colum) depending from table value
 */

wxString pwFiltersFTChoiceRenderer::GetString(const wxGrid& grid, int row, int col)
{
  wxGridTableBase *table = grid.GetTable();
  wxString text(wxEmptyString);
  if ( table->CanGetValueAs(row, col, wxGRID_VALUE_NUMBER) )
  {
      long choiceno = table->GetValueAsLong(row, col);
      text.Printf(wxT("%s"), getFieldTypeString(static_cast<int>(choiceno)) );
  }
  else
  {
      text = table->GetValue(row, col);
  }
  
  //If we faild to parse string just show what we where given?
  return text;
}

/*!
 * pwFiltersFTChoiceEditor (override) implementations
 * ------------------------------------------------------------
 */

/*!
 * AppendToChoicesString: extend the list of choices selectable in field type choice
 */

void pwFiltersFTChoiceEditor::AppendToChoicesString(wxString &choices, FieldType ft)
{
  // Fill in choice map with field type and add choice to choice map (comma separated list of all choices)
  m_choicemap.push_back(ft);
  if(! choices.empty())
    choices += _T(",");
  if(ft == static_cast<FieldType>(FT_HISTORY_MENU)) {
    choices += pwFiltersFTChoiceRenderer::getFieldTypeString(static_cast<int>(FT_PWHIST));
    choices += _T(" -->");
  }
  else if(ft == static_cast<FieldType>(FT_POLICY_MENU)) {
    choices += pwFiltersFTChoiceRenderer::getFieldTypeString(static_cast<int>(FT_POLICY));
    choices += _T(" -->");
  }
  else if(ft == static_cast<FieldType>(FT_ATTACHMENT_MENU)) {
    choices += pwFiltersFTChoiceRenderer::getFieldTypeString(static_cast<int>(FT_ATTACHMENT));
    choices += _T(" -->");
  }
  else {
    choices += pwFiltersFTChoiceRenderer::getFieldTypeString(static_cast<int>(ft));
  }
}

/*!
 * CheckFilterOnSpecialValues: check filter is including history, policy or attachment
 */

void pwFiltersFTChoiceEditor::CheckFilterOnSpecialValues(bool &pwhist, bool &policy, bool &attachment)
{
  pwhist = false;
  policy = false;
  attachment = false;
  
  for_each(m_currentFilter->begin(), m_currentFilter->end(), [&pwhist, &policy, &attachment] (st_FilterRow filter) {
    if(filter.ftype == FT_PWHIST) pwhist = true;
    else if(filter.ftype == FT_POLICY) policy = true;
    else if(filter.ftype == FT_ATTACHMENT) attachment = true;
  } );
}

/*!
 * UpdateControls: update filter type choice - if history, policy or attachment is already selected it might not be selected a second time
 * Mark not selectable entries by [---] and set field type in choice map as deleimiter
 */

void pwFiltersFTChoiceEditor::UpdateControls(int row)
{
  if(m_filtertype == DFTYPE_MAIN) {
    bool pwhist = false, policy = false, attachment = false;
    
    if(row != -1) {
      CheckFilterOnSpecialValues(pwhist, policy, attachment);
    
      // If current entry is already special value, let selection part of choice
      if(row < static_cast<int>(m_currentFilter->size())) {
        if(m_currentFilter->at(row).ftype == FT_PWHIST)
          pwhist = false;
        else if(m_currentFilter->at(row).ftype == FT_POLICY)
          policy = false;
        else if(m_currentFilter->at(row).ftype == FT_ATTACHMENT)
          attachment = false;
      }
    }

    if(pwhist) {
      Combo()->SetString (13, _T("[---]"));
      Combo()->SetString (31, _T("[---]"));
      m_choicemap[13] = FT_END;
      m_choicemap[31] = FT_END;
    }
    else {
      Combo()->SetString (13, pwFiltersFTChoiceRenderer::getFieldTypeString(static_cast<int>(FT_PWHIST)));
      Combo()->SetString (31, pwFiltersFTChoiceRenderer::getFieldTypeString(static_cast<int>(FT_PWHIST)) + _T(" -->"));
      m_choicemap[13] = FT_PWHIST;
      m_choicemap[31] = static_cast<FieldType>(FT_HISTORY_MENU);
    }
    if(policy) {
      Combo()->SetString (14, _T("[---]"));
      Combo()->SetString (32, _T("[---]"));
      m_choicemap[14] = FT_END;
      m_choicemap[32] = FT_END;
    }
    else {
      Combo()->SetString (14, pwFiltersFTChoiceRenderer::getFieldTypeString(static_cast<int>(FT_POLICY)));
      Combo()->SetString (32, pwFiltersFTChoiceRenderer::getFieldTypeString(static_cast<int>(FT_POLICY)) + _T(" -->"));
      m_choicemap[14] = FT_POLICY;
      m_choicemap[32] = static_cast<FieldType>(FT_POLICY_MENU);
    }
    if(m_bCanHaveAttachments) {
      if(attachment) {
        Combo()->SetString (33, _T("[---]"));
        m_choicemap[33] = FT_END;
      }
      else {
        Combo()->SetString (33, pwFiltersFTChoiceRenderer::getFieldTypeString(static_cast<int>(FT_ATTACHMENT)) + _T(" -->"));
        m_choicemap[33] = static_cast<FieldType>(FT_ATTACHMENT_MENU);
      }
    }
  }
}

/*!
 * CreateControls: Build up choice map for field type
 */

void pwFiltersFTChoiceEditor::CreateControls()
{
  wxString choices;

  m_choicemap.clear();
  if(m_filtertype == DFTYPE_MAIN) {
    AppendToChoicesString(choices, FT_GROUPTITLE);
    AppendToChoicesString(choices, FT_GROUP);
    AppendToChoicesString(choices, FT_TITLE);
    AppendToChoicesString(choices, FT_USER);
    AppendToChoicesString(choices, FT_NOTES);
    AppendToChoicesString(choices, FT_PASSWORD);
    AppendToChoicesString(choices, FT_CTIME);
    AppendToChoicesString(choices, FT_PMTIME);
    AppendToChoicesString(choices, FT_ATIME);
    AppendToChoicesString(choices, FT_XTIME);
    AppendToChoicesString(choices, FT_RMTIME);
    AppendToChoicesString(choices, FT_URL);
    AppendToChoicesString(choices, FT_AUTOTYPE);
    AppendToChoicesString(choices, FT_PWHIST); // 13
    AppendToChoicesString(choices, FT_POLICY); // 14
    AppendToChoicesString(choices, FT_XTIME_INT);
    AppendToChoicesString(choices, FT_RUNCMD);
    AppendToChoicesString(choices, FT_DCA);
    AppendToChoicesString(choices, FT_SHIFTDCA);
    AppendToChoicesString(choices, FT_EMAIL);
    AppendToChoicesString(choices, FT_PROTECTED);
    AppendToChoicesString(choices, FT_SYMBOLS);
    AppendToChoicesString(choices, FT_POLICYNAME);
    AppendToChoicesString(choices, FT_KBSHORTCUT);
    AppendToChoicesString(choices, FT_END);
    AppendToChoicesString(choices, FT_ENTRYSIZE);
    AppendToChoicesString(choices, FT_ENTRYTYPE);
    AppendToChoicesString(choices, FT_ENTRYSTATUS);
    AppendToChoicesString(choices, FT_UNKNOWNFIELDS);
    AppendToChoicesString(choices, FT_PASSWORDLEN);
    AppendToChoicesString(choices, FT_END);
    AppendToChoicesString(choices, static_cast<FieldType>(FT_HISTORY_MENU)); // 31
    AppendToChoicesString(choices, static_cast<FieldType>(FT_POLICY_MENU)); // 32
    if(m_bCanHaveAttachments) {
      AppendToChoicesString(choices, static_cast<FieldType>(FT_ATTACHMENT_MENU)); // 33
    }
  }
  else if(m_filtertype == DFTYPE_PWHISTORY) {
    AppendToChoicesString(choices, HT_PRESENT);
    AppendToChoicesString(choices, HT_ACTIVE);
    AppendToChoicesString(choices, HT_NUM);
    AppendToChoicesString(choices, HT_MAX);
    AppendToChoicesString(choices, HT_CHANGEDATE);
    AppendToChoicesString(choices, HT_PASSWORDS);
  }
  else if(m_filtertype == DFTYPE_PWPOLICY) {
    AppendToChoicesString(choices, PT_PRESENT);
    AppendToChoicesString(choices, PT_LENGTH);
    AppendToChoicesString(choices, PT_LOWERCASE);
    AppendToChoicesString(choices, PT_UPPERCASE);
    AppendToChoicesString(choices, PT_DIGITS);
    AppendToChoicesString(choices, PT_EASYVISION);
    AppendToChoicesString(choices, PT_PRONOUNCEABLE);
    AppendToChoicesString(choices, PT_HEXADECIMAL);
  }
  else if(m_filtertype == DFTYPE_ATTACHMENT) {
    AppendToChoicesString(choices, AT_PRESENT);
    AppendToChoicesString(choices, AT_TITLE);
    AppendToChoicesString(choices, AT_CTIME);
    AppendToChoicesString(choices, AT_MEDIATYPE);
    AppendToChoicesString(choices, AT_FILENAME);
    AppendToChoicesString(choices, AT_FILEPATH);
    AppendToChoicesString(choices, AT_FILECTIME);
    AppendToChoicesString(choices, AT_FILEMTIME);
    AppendToChoicesString(choices, AT_FILEATIME);
  }
  else {
    ASSERT(false);
  }
  SetParameters(choices);
}

/*!
 * pwFiltersFTChoiceEditor: Constructor
 */

pwFiltersFTChoiceEditor::pwFiltersFTChoiceEditor(const vFilterRows *currentFilter, const FilterType &filtertype, const bool bCanHaveAttachments) :
wxGridCellChoiceEditor(), m_currentFilter(currentFilter), m_filtertype(filtertype), m_bCanHaveAttachments(bCanHaveAttachments)
{
  m_index = -1;
  m_value = FT_INVALID;
  m_subMenu = FT_INVALID;
  CreateControls();
}

/*!
 * Clone: Implementation
 */

pwFiltersFTChoiceEditor *pwFiltersFTChoiceEditor::Clone() const
{
  pwFiltersFTChoiceEditor *renderer = new pwFiltersFTChoiceEditor(m_currentFilter, m_filtertype, m_bCanHaveAttachments);
  ASSERT(renderer);
  renderer->m_index = m_index;
  return renderer;
}

/*!
 * BeginEdit: Start editing
 */

void pwFiltersFTChoiceEditor::BeginEdit(int row, int col, wxGrid* grid)
{
  wxASSERT_MSG(m_control,
               wxT("The pwFiltersFTChoiceEditor must be Created first!"));
  
  // Check if choice must be updated
  if(m_filtertype == DFTYPE_MAIN) {
    UpdateControls(row);
  }
  
  wxGridCellEditorEvtHandler* evtHandler = NULL;
  if(m_control) {
    evtHandler = static_cast<wxGridCellEditorEvtHandler *>(m_control->GetEventHandler());
  }

  // Don't immediately end if we get a kill focus event within BeginEdit
  if(evtHandler)
    evtHandler->SetInSetFocus(true);

  wxGridTableBase *table = grid->GetTable();

  if(table->CanGetValueAs(row, col, wxGRID_VALUE_NUMBER))
  {
    m_value = table->GetValueAsLong(row, col);
  }
  else
  {
    wxString startValue = table->GetValue(row, col);
    if(startValue.IsNumber() && !startValue.empty())
    {
      startValue.ToLong(&m_value);
    }
    else
    {
      m_value = -1;
    }
  }
  if(m_value != -1) {
    std::vector<FieldType>::iterator it = std::find(m_choicemap.begin(), m_choicemap.end(), static_cast<FieldType>(m_value));
    if(it == m_choicemap.end()) {
      if(m_bCanHaveAttachments && (m_filtertype == DFTYPE_MAIN) && (m_value == FT_ATTACHMENT)) {
        m_index = 33;
        m_value = FT_ATTACHMENT_MENU;
      }
      else {
        m_index = 0;
        m_value = -1;
      }
    }
    else {
      m_index = static_cast<int>(std::distance(m_choicemap.begin(), it));
    }
  }
  else
    m_index = 0;

  Combo()->SetSelection(m_index);
  Combo()->SetFocus();
  
  if(m_value == -1)
    m_index = -1;

#ifdef __WXOSX_COCOA__
  // This is a work around for the combobox being simply dismissed when a
  // choice is made in it under OS X. The bug is almost certainly due to a
  // problem in focus events generation logic but it's not obvious to fix and
  // for now this at least allows to use wxGrid.
  Combo()->Popup();
#endif

    if (evtHandler)
    {
        // When dropping down the menu, a kill focus event
        // happens after this point, so we can't reset the flag yet.
#if !defined(__WXGTK20__)
        evtHandler->SetInSetFocus(false);
#endif
    }
}

/*!
 * EndEdit: End editing
 */

bool pwFiltersFTChoiceEditor::EndEdit(int row, int col, const wxGrid* grid, const wxString& oldval, wxString *newval)
{
  int idx = Combo()->GetSelection();
  long value;
  
  m_subMenu = FT_INVALID;
  
  if(idx == m_index)
      return false;
  
  if(idx >= static_cast<int>(m_choicemap.size()))
    return false;
  
  value = m_choicemap[idx];
  
  // On delimiter (or not selectable entry) selected, return false to not overtake the change
  if(value == FT_END || value == HT_END || value == PT_END || value == AT_END)
    return false;
  
  // Handle filter with "-->"
  if(value == FT_HISTORY_MENU) {
    m_subMenu = FT_HISTORY_MENU;
    value = FT_PWHIST;
    std::vector<FieldType>::iterator it = std::find(m_choicemap.begin(), m_choicemap.end(), static_cast<FieldType>(value));
    if(it != m_choicemap.end()) {
      idx = static_cast<int>(std::distance(m_choicemap.begin(), it));
    }
    else
      return false;
  }
  else if(value == FT_POLICY_MENU) {
    m_subMenu = FT_POLICY_MENU;
    value = FT_POLICY;
    std::vector<FieldType>::iterator it = std::find(m_choicemap.begin(), m_choicemap.end(), static_cast<FieldType>(value));
    if(it != m_choicemap.end()) {
      idx = static_cast<int>(std::distance(m_choicemap.begin(), it));
    }
    else
      return false;
  }
  else if(value == FT_ATTACHMENT_MENU) {
    m_subMenu = FT_ATTACHMENT_MENU;
    value = FT_ATTACHMENT;
  }

  // Remember the new value to handle in ApplyEdit()
  m_index = idx;
  m_value = value;

  if(newval)
    newval->Printf("%ld", m_value);

  return true;
}

/*!
 * ApplyEdit: Overtake change and store in table
 */

void pwFiltersFTChoiceEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
  wxGridTableBase * const table = grid->GetTable();
  if ( table->CanSetValueAs(row, col, wxGRID_VALUE_NUMBER) )
      table->SetValueAsLong(row, col, m_value);
  else
      table->SetValue(row, col, wxString::Format("%ld", m_value));
  
  if(m_subMenu != FT_INVALID) {
    // Simulate click on criteria field
    wxRect rect = grid->CellToRect(row, FLC_CRITERIA_TEXT);
    wxRect devRect = grid->BlockToDeviceRect(grid->XYToCell(rect.GetTopLeft()), grid->XYToCell(rect.GetTopRight()));

    wxGridEvent event(grid->GetId(), wxEVT_GRID_CELL_LEFT_CLICK, grid, row, FLC_CRITERIA_TEXT, devRect.GetX()+(devRect.GetWidth()/ 2), devRect.GetY()+(devRect.GetHeight()/2));
    
    grid->GetEventHandler()->AddPendingEvent(event);
    m_subMenu = FT_INVALID;
  }
}

/*!
 * pwFiltersLCChoiceRenderer (override) implementations
 * ----------------------------------------------------------------
 */

/*!
 * Clone implementation
 */

pwFiltersLCChoiceRenderer *pwFiltersLCChoiceRenderer::Clone() const
{
  pwFiltersLCChoiceRenderer *renderer = new pwFiltersLCChoiceRenderer;
  ASSERT(renderer);
  renderer->CreateControls();
  return renderer;
}

/*!
 * GetBestSize: return the icon extent
 */

wxSize pwFiltersLCChoiceRenderer::GetBestSize(wxGrid& WXUNUSED(grid), wxGridCellAttr& attr, wxDC& dc, int WXUNUSED(row), int WXUNUSED(col))
{
  if(m_bestSize.GetWidth() <= 0 ||  m_bestSize.GetHeight() <= 0) {
    m_bestSize.Set(25,15);
  
    m_bestSize.IncTo(DoGetBestSize(attr, dc, getFieldTypeString(LC_AND)));
    m_bestSize.IncTo(DoGetBestSize(attr, dc, getFieldTypeString(LC_OR)));
  }
  
  return m_bestSize;
}

#if wxVERSION_NUMBER >= 3100

/*!
 * GetBestHeight: return the test size maximum hight (is independent from width)
 */

int pwFiltersLCChoiceRenderer::GetBestHeight(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                            int row, int col, int WXUNUSED(width))
{
  (void) GetBestSize(grid, attr, dc, row, col);
  return m_bestSize.GetHeight();
}

/*!
 * GetBestWidth: return the text size maximum width (is independent from hight)
 */

int pwFiltersLCChoiceRenderer::GetBestWidth(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                           int row, int col, int WXUNUSED(height))
{
  (void) GetBestSize(grid, attr, dc, row, col);
  return m_bestSize.GetWidth();
}
#endif

#if wxVERSION_NUMBER >= 3104

/*!
 * GetMaxBestSize: return the text size maximum
 */

wxSize pwFiltersLCChoiceRenderer::GetMaxBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc)
{
  return GetBestSize(grid, attr, dc, 0, 0);
}
#endif

/*!
 * getFieldTypeString: Return the choice string for logic control
 */

wxString pwFiltersLCChoiceRenderer::getFieldTypeString(int lc)
{
  switch(static_cast<LogicConnect>(lc)) {
    case LC_INVALID:
      return wxEmptyString;
    case LC_AND:
      return _("And");
    case LC_OR:
      return _("Or");
    default:
      ASSERT(false);
  }
  return wxEmptyString;
}

/*!
 * getLCChoiceString: Create the choice map for logic control
 */

wxString pwFiltersLCChoiceRenderer::getLCChoiceString(bool bIncludeInvalid)
{
  return (bIncludeInvalid ? getFieldTypeString(LC_INVALID) + _T(",") : _T("")) + getFieldTypeString(LC_AND) + _T(",") + getFieldTypeString(LC_OR);
}

/*!
 * CreateControls: Build up choice map for logic control
 */

void pwFiltersLCChoiceRenderer::CreateControls()
{
  SetParameters(getLCChoiceString(true));
}

/*!
 * pwFiltersLCChoiceEditor (override) implementations
 * ------------------------------------------------------------
 */

/*!
 * pwFiltersLCChoiceEditor: Constructor
 */

pwFiltersLCChoiceEditor::pwFiltersLCChoiceEditor() : wxGridCellChoiceEditor()
{
  m_index = -1;

  SetParameters(pwFiltersLCChoiceRenderer::getLCChoiceString(false));
}

/*!
 * Clone implementation
 */

pwFiltersLCChoiceEditor* pwFiltersLCChoiceEditor::Clone() const
{
  pwFiltersLCChoiceEditor *renderer = new pwFiltersLCChoiceEditor;
  ASSERT(renderer);
  renderer->m_index = m_index;
  return renderer;
}

/*!
 * BeginEdit: Start editing
 */

void pwFiltersLCChoiceEditor::BeginEdit(int row, int col, wxGrid* grid)
{
  wxASSERT_MSG(m_control,
               wxT("The pwFiltersLCChoiceEditor must be Created first!"));

  wxGridCellEditorEvtHandler* evtHandler = NULL;
  if(m_control) {
    evtHandler = static_cast<wxGridCellEditorEvtHandler *>(m_control->GetEventHandler());
  }
  
  // Don't immediately end if we get a kill focus event within BeginEdit
  if(evtHandler)
    evtHandler->SetInSetFocus(true);

  wxGridTableBase *table = grid->GetTable();

  if(table->CanGetValueAs(row, col, wxGRID_VALUE_NUMBER))
  {
    m_index = static_cast<int>(table->GetValueAsLong(row, col));
  }
  else
  {
    wxString startValue = table->GetValue(row, col);
    if(startValue.IsNumber() && !startValue.empty())
    {
      long value;
      startValue.ToLong(&value);
      m_index = static_cast<int>(value);
    }
    else
    {
      m_index = 0;
    }
  }
  // Skip invalid as empty string with index 0
  if(m_index) m_index--;
  
  Combo()->SetSelection(m_index);
  Combo()->SetFocus();

#ifdef __WXOSX_COCOA__
  // This is a work around for the combobox being simply dismissed when a
  // choice is made in it under OS X. The bug is almost certainly due to a
  // problem in focus events generation logic but it's not obvious to fix and
  // for now this at least allows to use wxGrid.
  Combo()->Popup();
#endif

    if (evtHandler)
    {
        // When dropping down the menu, a kill focus event
        // happens after this point, so we can't reset the flag yet.
#if !defined(__WXGTK20__)
        evtHandler->SetInSetFocus(false);
#endif
    }
}

/*!
 * EndEdit: End editing
 */

bool pwFiltersLCChoiceEditor::EndEdit(int row, int col, const wxGrid* grid, const wxString& oldval, wxString *newval)
{
  int idx = Combo()->GetSelection();
  
  if(idx == m_index)
      return false;
  
  if(idx >= LC_OR)
    return false;

  m_index = idx + 1; // INVALID is not part of choice, so add one

  if(newval)
    newval->Printf("%d", m_index);

  return true;
}

/*!
 * ApplyEdit: Overtake change and store in table
 */

void pwFiltersLCChoiceEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
  wxGridTableBase * const table = grid->GetTable();
  if ( table->CanSetValueAs(row, col, wxGRID_VALUE_NUMBER) )
      table->SetValueAsLong(row, col, m_index);
  else
      table->SetValue(row, col, wxString::Format("%d", m_index));
}
