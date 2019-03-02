#ifndef CRYPTKEYENTRY_H
#define CRYPTKEYENTRY_H

#include "core/StringX.h"

//(*Headers(CryptKeyEntry)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class CryptKeyEntry: public wxDialog
{
public:

  enum class Mode { ENCRYPT, DECRYPT };

  CryptKeyEntry(Mode mode = Mode::ENCRYPT);
  virtual ~CryptKeyEntry();

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
  static const long ID_TEXTCTRL_KEY1;
  static const long ID_TEXTCTRL_KEY2;
  //*)

  //(*Declarations(CryptKeyEntryDialog)
  wxTextCtrl* TextCtrlKey1;
  wxTextCtrl* TextCtrlKey2;
  //*)

  Mode m_Mode;
  StringX m_CryptKey;

  DECLARE_EVENT_TABLE()
};

#endif // CRYPTKEYENTRY_H
