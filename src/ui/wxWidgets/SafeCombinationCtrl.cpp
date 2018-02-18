/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SafeCombinationCtrl.cpp
* 
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "../../core/PwsPlatform.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "SafeCombinationCtrl.h"
#include "./wxutils.h"
#include "./ExternalKeyboardButton.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

/*
 * This serves to transfer the data from wxTextCtrl directly into a StringX.
 * Maybe it should also check if the combination is valid...
 */
class SafeCombinationValidator: public wxValidator
{
  //no default ctor
  SafeCombinationValidator();
//  DECLARE_NO_COPY_CLASS(SafeCombinationValidator)
public:
  SafeCombinationValidator(StringX* str): m_str(str), m_allowBlank(false) {}
  virtual ~SafeCombinationValidator() { m_str = 0; }
  
  virtual wxObject *Clone() const { return new SafeCombinationValidator(m_str); }

  // This function can pop up an error message.
  virtual bool Validate(wxWindow *WXUNUSED(parent));

  // Called to transfer data to the window
  virtual bool TransferToWindow();

  // Called to transfer data from the window
  virtual bool TransferFromWindow();

  void AllowEmptyCombinationOnce() {m_allowBlank = true;}

private:
  StringX* m_str;
  bool m_allowBlank;
};

//
// Right now, we only validate if the user entered something in the combination box
// Maybe we could hook it up with the wxFilePickerCtrl and validate the safe 
// combination itself
//
bool SafeCombinationValidator::Validate(wxWindow* parent)
{
  bool retval = true;
  wxTextCtrl* win = wxDynamicCast(GetWindow(), wxTextCtrl);
  wxCHECK_MSG(win, false, wxT("You must associate a wxTextCtrl window with SafeCombinationValidator"));
  if (!m_allowBlank && win->IsEmpty()) {
    wxMessageBox(_("The combination cannot be blank."), _("Error"), wxOK | wxICON_EXCLAMATION, parent);
    win->SetFocus();
    retval = false;
  }
  m_allowBlank = false; // allowBlank is a one-shot: must be set each time!
  return retval;
}

bool SafeCombinationValidator::TransferToWindow()
{
  if (m_str) {
    wxTextCtrl* win = wxDynamicCast(GetWindow(), wxTextCtrl);
    wxCHECK_MSG(win, false, wxT("You must associate a wxTextCtrl window with SafeCombinationValidator"));
    wxString tmp = towxstring(*m_str);
    win->SetValue(tmp);
    //clear out the memory.  Is there a way to prevent this from getting optimized away?
    for( size_t idx = 0; idx < tmp.Len(); ++idx)
      tmp[idx] = 0;
  }
  return true;
}

bool SafeCombinationValidator::TransferFromWindow()
{
  if (m_str) {
    wxTextCtrl* win = wxDynamicCast(GetWindow(), wxTextCtrl);
    wxCHECK_MSG(win, false, wxT("You must associate a wxTextCtrl window with SafeCombinationValidator"));
    wxString tmp = win->GetValue();
    *m_str = tostringx(tmp);
    //clear out the memory.  Is there a way to prevent this from getting optimized away?
    for( size_t idx = 0; idx < tmp.Len(); ++idx)
      tmp[idx] = 0;
  }
  return true;
}

void CSafeCombinationCtrl::Init(wxWindow* parent, 
                                wxWindowID textCtrlID /*= wxID_ANY*/,
                                StringX* valPtr /*= 0*/,
                                const wxPoint& pos /* = wxDefaultPosition*/,
                                const wxSize& size /* = wxDefaultSize */)
{
  SafeCombinationValidator scValidator(valPtr);
  textCtrl = new wxTextCtrl(parent, textCtrlID, wxEmptyString, pos, size, 
                                                wxTE_PASSWORD,
                                                scValidator);
  ApplyPasswordFont(textCtrl);
  Add(textCtrl, wxSizerFlags().Proportion(1).Expand());
  
  ExternalKeyboardButton* vkbdButton = new ExternalKeyboardButton(parent);
  Add(vkbdButton, wxSizerFlags().Border(wxLEFT));
}

CSafeCombinationCtrl::~CSafeCombinationCtrl()
{
}

StringX CSafeCombinationCtrl::GetCombination() const
{
  return tostringx(textCtrl->GetValue());
}

void CSafeCombinationCtrl::SetValidatorTarget(StringX* str)
{
  SafeCombinationValidator scValidator(str);
  textCtrl->SetValidator(scValidator);
}

void CSafeCombinationCtrl::SelectCombinationText() const
{
  textCtrl->SetFocus();
  textCtrl->SetSelection(-1,-1);
}

void CSafeCombinationCtrl::AllowEmptyCombinationOnce()
{
  SafeCombinationValidator *scValidator = dynamic_cast<SafeCombinationValidator *>(textCtrl->GetValidator());
  if (scValidator != nullptr)
    scValidator->AllowEmptyCombinationOnce();
}
