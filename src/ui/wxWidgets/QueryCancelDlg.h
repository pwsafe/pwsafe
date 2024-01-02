/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file QueryCancelDlg.h
*/

#ifndef _QUERYCANCELDLG_H_
#define _QUERYCANCELDLG_H_

/*!
 * Includes
 */

#include <wx/dialog.h>

/*!
 * QueryCancelDlg class declaration
 */

class QueryCancelDlg : public wxDialog
{
public:
  virtual void OnCancelClick(wxCommandEvent& event);
  virtual void OnClose(wxCloseEvent& event);
  virtual bool SyncAndQueryCancel(bool showDialog);
  virtual bool IsChanged() const = 0;
};

#endif // _QUERYCANCELDLG_H_
