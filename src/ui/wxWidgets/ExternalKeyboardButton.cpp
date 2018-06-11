/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ExternalKeyboardButton.cpp
* 
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#include "../../core/PwsPlatform.h"
#include "../../os/file.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "./ExternalKeyboardButton.h"

#include "./graphics/vkbd.xpm"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

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
  Connect(GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ExternalKeyboardButton::HandleCommandEvent));
  //hook into the button so that we actually get events
  //PushEventHandler(this);
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

  if (!pws_os::ProgramExists(command.wc_str())) {
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

}
