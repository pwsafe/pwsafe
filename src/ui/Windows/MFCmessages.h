/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __MFCMESSAGES_H
#define __MFCMESSAGES_H

#include "GeneralMsgBox.h"

class MFCAsker : public Asker
{
  bool operator()(const std::wstring &question) {
    CGeneralMsgBox gmb;
    INT_PTR msg_rc = gmb.AfxMessageBox(question.c_str(), NULL,
                                   MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
    return msg_rc == IDYES;
  }
  virtual bool operator()(const std::wstring &title,
                          const std::wstring &question) {
    CGeneralMsgBox gmb;
    INT_PTR msg_rc = gmb.AfxMessageBox(question.c_str(), title.c_str(),
                                   MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
    return msg_rc == IDYES;
  }
};

class MFCReporter : public Reporter
{
  void operator()(const std::wstring &message) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(message.c_str(), NULL, MB_OK | MB_ICONEXCLAMATION);
  }

  void operator()(const std::wstring &title, const std::wstring &message) {
    CGeneralMsgBox gmb;
    gmb.AfxMessageBox(message.c_str(), title.c_str(), MB_OK | MB_ICONEXCLAMATION);
  }
};

#endif /* __MFCMESSAGES_H */
