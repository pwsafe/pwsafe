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
