/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWFiltersEditors.h
* 
*/

#ifndef _PWFILTERSEDITORS_H_
#define _PWFILTERSEDITORS_H_

/*!
 * Includes
 */

////@begin includes
#include <map>
#include <wx/string.h>
#include <wx/grid.h>
#include <wx/image.h>
#include <wx/imaglist.h>
#include "core/PWSFilters.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers

extern const std::map<int, wxString> FieldTypeString;
const wxString LoadAString(int id);

// State of icons
typedef enum { PWF_UNCHECKED, PWF_CHECKED, PWF_DISABLED, NO_PWF_ELEMENTS, PWF_UNCHECKED_DISABLED = NO_PWF_ELEMENTS, PWF_CHECKED_DISBALED, PWF_DISBALED_DISABLED, MAX_PWF_ELEMENTS } PWFCheckType;

#define PWF_DEF_FONT_HEIGTH 15

/*!
 * pwFiltersActiveRenderer class declaration
 */

class pwFiltersActiveRenderer : public wxGridCellRenderer
{
public:
  
  pwFiltersActiveRenderer(int fontHeight = PWF_DEF_FONT_HEIGTH) { m_fontHeight = fontHeight; CreateControls(); };
  
  // draw the Icon
  virtual void Draw(wxGrid& grid,
                    wxGridCellAttr& attr,
                    wxDC& dc,
                    const wxRect& rect,
                    int row, int col,
                    bool isSelected);

  // return the icon extent
  virtual wxSize GetBestSize(wxGrid& grid,
                             wxGridCellAttr& attr,
                             wxDC& dc,
                             int row, int col);
#if wxVERSION_NUMBER >= 3100
  virtual int GetBestHeight(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                            int row, int col, int width);
  virtual int GetBestWidth(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                           int row, int col, int height);
#endif
#if wxVERSION_NUMBER >= 3104
  virtual wxSize GetMaxBestSize(wxGrid& grid,
                                wxGridCellAttr& attr,
                                wxDC& dc);
#endif

  virtual pwFiltersActiveRenderer *Clone() const
        { return new pwFiltersActiveRenderer; }
  
  static const wxString GetCellValueOfCheckType(PWFCheckType ct);

private:
  static bool m_initialied;
  static wxImageList m_images;
  static wxSize m_bestSize;
  static int m_fontHeight;
  
  static void CreateControls();
};

/*!
 * pwFiltersChoiceRenderer class declaration
 */

class pwFiltersFTChoiceRenderer : public wxGridCellStringRenderer
{
public:    // draw the string
  virtual void Draw(wxGrid& grid,
                    wxGridCellAttr& attr,
                    wxDC& dc,
                    const wxRect& rect,
                    int row, int col,
                    bool isSelected);
    
  virtual pwFiltersFTChoiceRenderer *Clone() const;
  
  // return the string extent
  virtual wxSize GetBestSize(wxGrid& grid,
                             wxGridCellAttr& attr,
                             wxDC& dc,
                             int row, int col);
#if wxVERSION_NUMBER >= 3100
  virtual int GetBestHeight(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                            int row, int col, int width);
  virtual int GetBestWidth(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                           int row, int col, int height);
#endif
#if wxVERSION_NUMBER >= 3104
  virtual wxSize GetMaxBestSize(wxGrid& grid,
                                wxGridCellAttr& attr,
                                wxDC& dc);
#endif
  
  static wxString getFieldTypeString(int ft);
  
protected:
  static wxSize m_bestSize;
  
  wxString GetString(const wxGrid& grid, int row, int col);
};

/*!
 * pwFiltersFTChoiceEditor class declaration
 */

class pwFiltersFTChoiceEditor : public wxGridCellChoiceEditor
{
  // Extension to FieldType enum about "menu -->"
  enum { FT_HISTORY_MENU = (FT_INVALID - 1), FT_POLICY_MENU = (FT_INVALID - 2), FT_ATTACHMENT_MENU = (FT_INVALID - 3) };
  
public:
  pwFiltersFTChoiceEditor(const vFilterRows *currentFilter, const FilterType &filtertype = DFTYPE_INVALID, const bool bCanHaveAttachments = false);
  virtual ~pwFiltersFTChoiceEditor() { m_choicemap.clear(); };

  virtual pwFiltersFTChoiceEditor*  Clone() const;

  virtual void BeginEdit(int row, int col, wxGrid* grid);
  virtual bool EndEdit(int row, int col, const wxGrid* grid,
                       const wxString& oldval, wxString *newval);
  virtual void ApplyEdit(int row, int col, wxGrid* grid);

private:
  int m_index;
  long m_value;
  long m_subMenu;
  const vFilterRows *m_currentFilter;
  const FilterType m_filtertype;
  const bool m_bCanHaveAttachments;
  std::vector<FieldType> m_choicemap;

  void CreateControls();
  void UpdateControls(int row = -1);
  void AppendToChoicesString(wxString &choices, FieldType ft);
  void CheckFilterOnSpecialValues(bool &pwhist, bool &policy, bool &attachment);

  wxDECLARE_NO_COPY_CLASS(pwFiltersFTChoiceEditor);
};

/*!
 * pwFiltersLCChoiceRenderer class declaration
 */

class pwFiltersLCChoiceRenderer : public wxGridCellEnumRenderer
{
public:
  pwFiltersLCChoiceRenderer() { CreateControls(); };
    
  virtual pwFiltersLCChoiceRenderer *Clone() const;
  
  // return the string extent
  virtual wxSize GetBestSize(wxGrid& grid,
                             wxGridCellAttr& attr,
                             wxDC& dc,
                             int row, int col);
#if wxVERSION_NUMBER >= 3100
  virtual int GetBestHeight(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                            int row, int col, int width);
  virtual int GetBestWidth(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                           int row, int col, int height);
#endif
#if wxVERSION_NUMBER >= 3104
  virtual wxSize GetMaxBestSize(wxGrid& grid,
                                wxGridCellAttr& attr,
                                wxDC& dc);
#endif
  
  static wxString getFieldTypeString(int lc);
  static wxString getLCChoiceString(bool bIncludeInvalid);

protected:
  static wxSize m_bestSize; // best size is static, as all fields are having same content and size (And or Or)
  
  void CreateControls();
};

/*!
 * pwFiltersLCChoiceEditor class declaration
 */

class pwFiltersLCChoiceEditor : public wxGridCellChoiceEditor
{
public:
  pwFiltersLCChoiceEditor();
  virtual ~pwFiltersLCChoiceEditor() {};

  virtual pwFiltersLCChoiceEditor*  Clone() const;

  virtual void BeginEdit(int row, int col, wxGrid* grid);
  virtual bool EndEdit(int row, int col, const wxGrid* grid,
                       const wxString& oldval, wxString *newval);
  virtual void ApplyEdit(int row, int col, wxGrid* grid);

private:
  int m_index;

  wxDECLARE_NO_COPY_CLASS(pwFiltersLCChoiceEditor);
};

#endif // _PWFILTERSEDITORS_H_
