/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ExternalKeyboardButton.cpp
* 
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include "os/file.h"

#include "ExternalKeyboardButton.h"
#include "wxUtilities.h"

#include "graphics/vkbd.xpm"

ExternalKeyboardButton::ExternalKeyboardButton( wxWindow* parent, 
                                                wxWindowID id, 
                                                const wxPoint& pos,
                                                const wxSize& size, 
                                                long style, 
                                                const wxValidator& validator, 
                                                const wxString& name) : wxBitmapButton(parent, 
                                                                                       id, 
                                                                                       wxBitmap(vkbd_xpm),
                                                                                       pos,
                                                                                       size,
                                                                                       style,
                                                                                       validator,
                                                                                       name)
{
  //Create an event table entry in this class for the button's id
  Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ExternalKeyboardButton::HandleCommandEvent, this, GetId());
  //hook into the button so that we actually get events
  //PushEventHandler(this);
  SetToolTip(_("Virtual Keyboard"));
}

ExternalKeyboardButton::~ExternalKeyboardButton()
{
}

void ExternalKeyboardButton::HandleCommandEvent(wxCommandEvent& evt)
{
  UNREFERENCED_PARAMETER(evt);
#ifdef __WXGTK__
//if this works, we could pass the X window-id of the combination textCtrl
//to make sure it is the only recipient of keystrokes from xvkbd
#if 0
#include <gtk-2.0/gtk/gtkwidget.h>
  GtkWidget* widget = FindWindow(ID_COMBINATION)->GetHandle();
  GdkWindow* window = widget->window;
  int xwinid = GDK_WINDOW_XWINDOW(window);
#endif
  wxString command = wxString(wxT("xvkbd"));

  if (!pws_os::ProgramExists(tostdstring(command))) {
    wxMessageBox(_("Could not launch xvkbd.  Please make sure it's installed and in your PATH"), 
                  _("Could not launch external onscreen keyboard"), wxOK | wxICON_ERROR);
    return;
  }

  switch(wxExecute(command, wxEXEC_ASYNC, nullptr)) //nullptr => we don't want a wxProcess as callback
  {
    case 0:
      wxMessageBox(_("Could not launch xvkbd.  Please make sure it's in your PATH"), 
                    _("Could not launch external onscreen keyboard"), wxOK | wxICON_ERROR);
      break;
      
    case -1:    //only if ASYNC
      wxMessageBox(_("Could not launch a new process for xvkbd.  Simultaneous execution disabled?"), 
                    _("Could not launch external onscreen keyboard"), wxOK | wxICON_ERROR);
      break;
      
    default:
      break;
  }
#endif

#ifdef __WXOSX__
  // If we can't open the virtual keyboard, at least open the settings app so the user can do it for us!
  wxString command = wxString("open x-apple.systempreferences:com.apple.preference.universalaccess?Keyboard");

  if ( wxExecute(command, wxEXEC_ASYNC, nullptr) > 0) {
    wxMessageBox(_("Please enable the Accessibility Keyboard in System Settings; the on-screen keyboard should then appear."), "", wxOK | wxICON_INFORMATION);
  } else {
    wxMessageBox(_("Could not launch the MacOS Settings App"),
                  _("Could not launch external onscreen keyboard"), wxOK | wxICON_ERROR);
  }
#endif

  if (m_TargetSafeCombinationCtrl != nullptr) {
    m_TargetSafeCombinationCtrl->SetFocus();
  }
}
