/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __PWSHELPPROVIDER_H__
#define __PWSHELPPROVIDER_H__

#include <wx/event.h> // Base class: wxEvtHandler
#include <map>

class wxHtmlHelpController;


class PWSHelpProvider : public wxEvtHandler {
  
  DECLARE_CLASS(PWSHelpProvider)
  
public:
  static PWSHelpProvider& Instance();

  void SetHelpContext(wxWindow* win, const wxString& helpSection);
  void OnHelp(wxCommandEvent& evt);

private:

  WX_DECLARE_HASH_MAP( int, wxString, wxIntegerHash, wxIntegerEqual, HelpContextMap );
  //typedef std::map<int, int> HelpContextMap;
  HelpContextMap m_map;

  wxHtmlHelpController* m_controller;
  
  PWSHelpProvider();
  ~PWSHelpProvider();

  DECLARE_EVENT_TABLE()
};

#endif // __PWSHELPPROVIDER_H__
