#ifndef __PWS_SYNCWIZARD__
#define __PWS_SYNCWIZARD__

#include <wx/wizard.h> 


/*!
 * PwsSyncWizard class declaration
 * 
 * A simple wizard that imlements the "Synchronize" menu item as a series of wizard
 * pages instead of a warning dialog followed by a "File Open" dialog followed by
 * SafeCombinationPrompt dialog (like in the MFC version)
 */
class PwsSyncWizard : public wxWizard {

  wxWizardPageSimple* m_page1;

public:
  PwsSyncWizard(wxWindow* parent, const wxString& currentDB);
  ~PwsSyncWizard();

  //users must pass the return value of this function to RunWizard
  //for launching the wizard 
  wxWizardPage* GetFirstPage() { return m_page1; }

  void OnWizardPageChanging(wxWizardEvent& evt);

  DECLARE_EVENT_TABLE()
};

#endif // __PWS_SYNCWIZARD__
