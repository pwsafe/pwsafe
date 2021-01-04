/*
 * Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SyncWizard.h
* 
*/

#ifndef _SYNCWIZARD_H_
#define _SYNCWIZARD_H_

#include <wx/wizard.h> 

class PWScore;
struct SelectionCriteria;
struct SyncData;
class CReport;

/*!
 * SyncWizard class declaration
 * 
 * A simple wizard that implements the "Synchronize" menu item as a series of wizard
 * pages instead of a warning dialog followed by a "File Open" dialog followed by
 * SafeCombinationPrompt dialog (like in the MFC version)
 */
class SyncWizard : public wxWizard
{
  wxWizardPageSimple* m_page1;
  SyncData* m_syncData;

public:
  SyncWizard(wxWindow* parent, PWScore* core);
  ~SyncWizard();

  //users must pass the return value of this function to RunWizard
  //for launching the wizard 
  wxWizardPage* GetFirstPage() { return m_page1; }

  size_t GetNumUpdated() const;
  bool   ShowReport() const;
  CReport*   GetReport() const;
  void OnWizardPageChanging(wxWizardEvent& evt);

  DECLARE_EVENT_TABLE()
};

#endif // _SYNCWIZARD_H_
