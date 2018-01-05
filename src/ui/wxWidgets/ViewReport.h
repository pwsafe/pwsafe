/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef __VIEWREPORT_H__
#define __VIEWREPORT_H__

#include <wx/dialog.h> // Base class: wxDialog

class CReport;

class CViewReport : public wxDialog {

  CReport* m_pRpt;
  
public:
  CViewReport(wxWindow* pParent, CReport* pRpt);
  ~CViewReport();

  void OnSave(wxCommandEvent& event);
  void OnClose(wxCommandEvent& event);
  void OnCopy(wxCommandEvent& event);
};

#endif // __VIEWREPORT_H__
