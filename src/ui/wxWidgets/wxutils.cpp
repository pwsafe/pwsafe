/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file wxutils.cpp
*
* Contains generic utility functions that should be global and don't fit anywhere else
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "../../core/PWScore.h"
#include "./wxutils.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include <wx/taskbar.h>
#include <wx/tokenzr.h>
#include <wx/versioninfo.h>

/*
 * Reads a file into a PWScore object, and displays an appropriate msgbox
 * in case of failure.  Returns PWScore::SUCCESS on success
 */

int ReadCore(PWScore& othercore, const wxString& file, const StringX& combination,
             bool showMsgbox /*= true*/, wxWindow* msgboxParent /*= nullptr*/,
        bool setupCopy /*= false*/)
{
  othercore.ClearDBData();

  StringX dbpath(tostringx(file));
  int rc = othercore.ReadFile(dbpath, combination);

  if (setupCopy)
    PWSprefs::GetInstance()->SetupCopyPrefs();

  switch (rc) {
    case PWScore::SUCCESS:
      othercore.SetCurFile(tostringx(file));
      break;

    case PWScore::CANT_OPEN_FILE:
      if (showMsgbox)
        wxMessageBox(wxString(file) << wxT("\n\n") << _("Could not open file for reading!"),
                    _("File Read Error"), wxOK | wxICON_ERROR, msgboxParent );
      break;

    case PWScore::BAD_DIGEST:
      if (showMsgbox && wxMessageBox(wxString(file) << wxT("\n\n") << _("File corrupt or truncated!\nData may have been lost or modified.\nContinue anyway?"),
            _("File Read Error"), wxYES_NO | wxICON_QUESTION, msgboxParent) == wxYES) {
        rc = PWScore::SUCCESS;
      }
      break;

    default:
      if (showMsgbox)
        wxMessageBox( wxString(file) << wxT("\n\n") << _("Unknown error"), _("File Read Error"), wxOK | wxICON_ERROR, msgboxParent);
      break;
  }

  return rc;
}

void HideWindowRecursively(wxTopLevelWindow* win, wxWindowList& hiddenWindows)
{
  if (!win)
    return;
  wxWindowList& children = win->GetChildren();
  for(wxWindowList::iterator itr = children.begin(); itr != children.end(); ++itr) {
    if ((*itr)->IsTopLevel() && (*itr)->IsShown()) {
      HideWindowRecursively(wxDynamicCast(*itr, wxTopLevelWindow), hiddenWindows);
    }
  }
  //Don't call Hide() here, which just calls Show(false), which is overridden in
  //derived classes, and wxDialog actually cancels the modal loop and closes the window
  win->wxWindow::Show(false);
  //push_front ensures we Show() in the reverse order of Hide()'ing
  hiddenWindows.push_front(win);
}

void ShowWindowRecursively(wxWindowList& hiddenWindows)
{
  for(wxWindowList::iterator itr = hiddenWindows.begin(); itr != hiddenWindows.end(); ++itr) {
    wxWindow* win = (*itr);
    //Show is virtual, and dialog windows assume the window is just starting up when Show()
    //is called.  Make sure to call the base version
    win->wxWindow::Show(true);
    win->Raise();
    win->Update();
  }
  hiddenWindows.clear();
}

/////////////////////////////////////////////////////////////
// MultiCheckboxValidator
//
MultiCheckboxValidator::MultiCheckboxValidator(int ids[],
                                               size_t num,
                                               const wxString& msg,
                                               const wxString& title): m_ids(new int[num]),
                                                                       m_count(num),
                                                                       m_msg(msg),
                                                                       m_title(title)

{
  memcpy(m_ids, ids, sizeof(m_ids[0])*m_count);
}

MultiCheckboxValidator::MultiCheckboxValidator(const MultiCheckboxValidator& other):
  /*Copy constructor for wxValidator is banned in 2.8.x, so explicitly call constructor, to prevent warning */
                                                                        wxValidator(),
                                                                        m_ids(new int[other.m_count]),
                                                                        m_count(other.m_count),
                                                                        m_msg(other.m_msg),
                                                                        m_title(other.m_title)
{
  memcpy(m_ids, other.m_ids, sizeof(m_ids[0])*m_count);
}

MultiCheckboxValidator::~MultiCheckboxValidator()
{
  delete [] m_ids;
}

wxObject* MultiCheckboxValidator::Clone() const
{
  return new MultiCheckboxValidator(m_ids, m_count, m_msg, m_title);
}

bool MultiCheckboxValidator::Validate(wxWindow* parent)
{
  bool allDisabled = true;
  for(size_t idx = 0; idx < m_count; ++idx) {
    wxWindow* win = GetWindow()->FindWindow(m_ids[idx]);
    if (win) {
      if (win->IsEnabled()) {
        allDisabled = false;
        wxCheckBox* cb = wxDynamicCast(win, wxCheckBox);
        if (cb) {
          if (cb->IsChecked()) {
            return true;
          }
        }
        else {
          wxFAIL_MSG(wxString::Format(wxT("Child(id %d) is not a checkbox"), m_ids[idx]));
        }
      }
    }
    else {
      wxFAIL_MSG(wxString::Format(wxT("No child with id (%d) found in MultiCheckboxValidator"), m_ids[idx]));
    }
  }
  if (allDisabled)
    return true;
  else {
    wxMessageBox(m_msg, m_title, wxOK|wxICON_EXCLAMATION, parent);
    return false;
  }
}

void ShowHideText(wxTextCtrl *&txtCtrl, const wxString &text,
                  wxSizer *sizer, bool show)
{
  wxWindow *parent = txtCtrl->GetParent();
  wxWindowID id = txtCtrl->GetId();
  wxValidator *validator = txtCtrl->GetValidator();

  // Per Dave Silvia's suggestion:
  // Following kludge since wxTE_PASSWORD style is immutable
  wxTextCtrl *tmp = txtCtrl;
  txtCtrl = new wxTextCtrl(parent, id, text,
                           wxDefaultPosition, wxDefaultSize,
                           show ? 0 : wxTE_PASSWORD);
  if (validator != nullptr)
    txtCtrl->SetValidator(*validator);
  ApplyPasswordFont(txtCtrl);
  sizer->Replace(tmp, txtCtrl);
  delete tmp;
  sizer->Layout();
  if (!text.IsEmpty()) {
    txtCtrl->ChangeValue(text);
    txtCtrl->SetModified(true);
  }
}

int pless(int* first, int* second) { return *first - *second; }

// Wrapper for wxTaskBarIcon::IsAvailable() that doesn't crash
// on Fedora or Ubuntu
bool IsTaskBarIconAvailable()
{
#if defined(__WXGTK__)
  const wxVersionInfo verInfo = wxGetLibraryVersionInfo();
  int major = verInfo.GetMajor();
  int minor = verInfo.GetMinor();
  int micro = verInfo.GetMicro();
  if (major < 3 || (major == 3 && ((minor == 0 && micro < 4) || (minor == 1 && micro < 1)))) {
    const wxLinuxDistributionInfo ldi = wxGetLinuxDistributionInfo();
    if (ldi.Id.IsEmpty() || ldi.Id == wxT("Ubuntu") || ldi.Id == wxT("Fedora"))
      return false;
  }
#endif
  return wxTaskBarIcon::IsAvailable();
}
