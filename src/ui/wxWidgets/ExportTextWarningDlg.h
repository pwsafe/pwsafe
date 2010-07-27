#ifndef __EXPORTTEXTWARNINGDLG_H__
#define __EXPORTTEXTWARNINGDLG_H__

#include <wx/dialog.h> // Base class: wxDialog

class CExportTextWarningDlg : public wxDialog {

public:
  CExportTextWarningDlg(wxWindow* parent, const wxString& title);
  ~CExportTextWarningDlg();

};

#endif // __EXPORTTEXTWARNINGDLG_H__
