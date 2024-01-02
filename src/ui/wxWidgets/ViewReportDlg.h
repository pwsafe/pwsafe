/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ViewReportDlg.h
* 
*/

#ifndef _VIEWREPORTDLG_H_
#define _VIEWREPORTDLG_H_

#include <wx/dialog.h> // Base class: wxDialog

class CReport;

class ViewReportDlg : public wxDialog {

  CReport* m_pRpt;
  
public:
  static ViewReportDlg* Create(wxWindow *parent, CReport* pRpt, bool fromFile = false);
  ~ViewReportDlg() = default;
protected:
  ViewReportDlg(wxWindow *parent, CReport* pRpt, bool fromFile);
  void OnSave(wxCommandEvent& event);
  void OnRemove(wxCommandEvent& event);
  void OnClose(wxCommandEvent& event);
  void OnCopy(wxCommandEvent& event);
};

#endif // _VIEWREPORTDLG_H_
