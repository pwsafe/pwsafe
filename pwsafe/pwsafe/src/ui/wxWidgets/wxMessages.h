/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __WXMESSAGES_H
#define __WXMESSAGES_H

#include "corelib/Proxy.h"
#include <wx/msgdlg.h>

class wxAsker : public Asker
{
  bool operator()(const std::wstring &question) {
    wxMessageDialog dlg(NULL, question.c_str(), _("PasswordSafe"),
                        wxYES_NO | wxICON_QUESTION | wxNO_DEFAULT);
    return dlg.ShowModal() == wxID_YES;
  }
  bool operator()(const std::wstring &title, const std::wstring &question) {
    wxMessageDialog dlg(NULL, question.c_str(), title.c_str(),
                        wxYES_NO | wxICON_QUESTION | wxNO_DEFAULT);
    return dlg.ShowModal() == wxID_YES;
  }
};

class wxReporter : public Reporter
{
  void operator()(const std::wstring &message) {
    wxMessageDialog dlg(NULL, message.c_str(), _("PasswordSafe"),
                        wxOK | wxICON_EXCLAMATION);
    dlg.ShowModal();
  }
  void operator()(const std::wstring &title, const std::wstring &message) {
    wxMessageDialog dlg(NULL, message.c_str(), title.c_str(),
                        wxOK | wxICON_EXCLAMATION);
    dlg.ShowModal();
  }
};

#endif /* __WXMESSAGES_H */
