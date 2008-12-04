/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __MFCMESSAGES_
#define __MFCMESSAGES_

class MFCAsker : public Asker
{
  bool operator()(const stringT &question) {
    int msg_rc = AfxMessageBox(question.c_str(), MB_YESNO | 
                               MB_ICONQUESTION | MB_DEFBUTTON2);
    return msg_rc == IDYES;
  }
};

class MFCReporter : public Reporter
{
  void operator()(const stringT &message) {
    AfxMessageBox(message.c_str(), MB_OK | MB_ICONEXCLAMATION);
  }
};

#endif / * __MFCMESSAGES_ */
