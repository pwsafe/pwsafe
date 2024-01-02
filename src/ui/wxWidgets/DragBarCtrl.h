/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file DragBarCtrl.h
 *
 */

#ifndef _DRAGBARCTRL_H_
#define _DRAGBARCTRL_H_

#include <wx/aui/auibar.h>

#include "TreeCtrl.h"

#define DRAGBAR_STYLE wxAUI_TB_DEFAULT_STYLE|wxAUI_TB_GRIPPER|wxAUI_TB_PLAIN_BACKGROUND

class PasswordSafeFrame;
class TreeCtrl;

class DragBarCtrl : public wxAuiToolBar
{
public:
  DragBarCtrl(wxWindow *parent, wxWindowID id=wxID_ANY, const wxPoint &position=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=DRAGBAR_STYLE);
  ~DragBarCtrl();

  void CreateToolbar();
  void UpdateBitmaps();
  void UpdateTooltips();
  bool HasTools() const { return GetToolCount() > 0; }
private:
  wxString GetText(int id) const;
  void OnDrag(wxAuiToolBarEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& evt);
  void CalculateToolsWidth();
};

#endif // _DRAGBARCTRL_H_
