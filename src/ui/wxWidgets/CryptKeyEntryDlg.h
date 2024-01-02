/*
 * Initial version created as 'CryptKeyEntryDlg.h'
 * by rafaelx on 2019-03-02.
 *
 * Copyright (c) 2019-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file CryptKeyEntryDlg.h
* 
*/

#ifndef _CRYPTKEYENTRYDLG_H_
#define _CRYPTKEYENTRYDLG_H_

#include "core/StringX.h"

//(*Headers(CryptKeyEntryDlg)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class CryptKeyEntryDlg : public wxDialog
{
public:

  enum class Mode { ENCRYPT, DECRYPT };

  CryptKeyEntryDlg(Mode mode = Mode::ENCRYPT);
  virtual ~CryptKeyEntryDlg();

  StringX getCryptKey() const {
    return m_CryptKey;
  }

protected:

  //(*Handlers(CryptKeyEntryDialog)
  void OnOk(wxCommandEvent& event);
  void OnCancel(wxCommandEvent& event);
  void OnClose(wxCloseEvent& event);
  //*)

  //(*Identifiers(CryptKeyEntryDialog)
  //*)

  //(*Declarations(CryptKeyEntryDialog)
  wxTextCtrl* TextCtrlKey1;
  wxTextCtrl* TextCtrlKey2;
  //*)

  Mode m_Mode;
  StringX m_CryptKey;

  DECLARE_EVENT_TABLE()
};

#endif // _CRYPTKEYENTRYDLG_H_
