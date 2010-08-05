/*
 * Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file editshortcut.cpp
* 
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "SafeCombinationCtrl.h"

#include "../graphics/wxWidgets/vkbd.xpm"

//this is just on a hunch that the window name that wxWidgets accepts
//as the last param could be the instance/class name of the X window object
const wxChar xvkbdTargetName[] = wxT("__passwordsafetextctrl__");


void CommandEventHandler::HandleCommandEvent(wxCommandEvent& evt)
{
#ifdef __WXGTK__
//if this works, we could pass the X window-id of the combination textCtrl
//to make sure it is the only recipient of keystrokes from xvkbd
#if 0
#include <gtk-2.0/gtk/gtkwidget.h>
  GtkWidget* widget = FindWindow(ID_COMBINATION)->GetHandle();
  GdkWindow* window = widget->window;
  int xwinid = GDK_WINDOW_XWINDOW(window);
#endif
  wxString command = wxString(wxT("xvkbd -window ")) + xvkbdTargetName;
  
  switch(wxExecute(command, wxEXEC_ASYNC, NULL)) //NULL => we don't want a wxProcess as callback
  {
    case 0:
      wxMessageBox(_("Could not launch xvkbd.  Please make sure its in your PATH"), 
                    _("Could not launch external onscreen keyboard"), wxOK | wxICON_ERROR);
      break;
      
    case -1:    //only if ASYNC
      wxMessageBox(_("Could not launch a new process for xvkbd.  Simultaneous execution is disabled?"), 
                    _("Could not launch external onscreen keyboard"), wxOK | wxICON_ERROR);
      break;
      
    default:
      break;
  }
#endif
}

CSafeCombinationCtrl::CSafeCombinationCtrl(wxWindow* parent, 
                                            wxWindowID textCtrlID /*= wxID_ANY*/,
                                            wxString* valPtr /*= 0*/) : wxBoxSizer(wxHORIZONTAL), 
                                                                        textCtrl(0)
{
#if wxCHECK_VERSION(2,9,1)
  int validatorStyle = wxFILTER_EMPTY;
#else
  int validatorStyle = wxFILTER_NONE;
#endif
  
  textCtrl = new wxTextCtrl(parent, textCtrlID, wxEmptyString, wxDefaultPosition, wxDefaultSize, 
                                                wxTE_PROCESS_ENTER|wxTE_PASSWORD,
                                                wxTextValidator(validatorStyle, valPtr));
  Add(textCtrl, wxSizerFlags().Proportion(1).Expand());
  
  wxBitmapButton* vkbdButton = new wxBitmapButton(parent, wxID_ANY, wxBitmap(vkbd_xpm));
  Add(vkbdButton, wxSizerFlags().Border(wxLEFT));
  //Create an event table entry in this class for the button's id
  Connect(vkbdButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CommandEventHandler::HandleCommandEvent));
  //hook into the button so that we actually get events
  vkbdButton->PushEventHandler(this);
}

CSafeCombinationCtrl::~CSafeCombinationCtrl()
{
}
