/*
 * Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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


CSafeCombinationCtrl::CSafeCombinationCtrl(wxWindow* parent, 
                                            wxWindowID textCtrlID /*= wxID_ANY*/,
                                            wxString* valPtr /*= 0*/,
                                            const wxPoint& pos /* = wxDefaultPosition*/,
                                            const wxSize& size /* = wxDefaultSize */) : 
                                                                        wxBoxSizer(wxHORIZONTAL), 
                                                                        textCtrl(0)
{
#if wxCHECK_VERSION(2,9,1)
  int validatorStyle = wxFILTER_EMPTY;
#else
  int validatorStyle = wxFILTER_NONE;
#endif
  
  textCtrl = new wxTextCtrl(parent, textCtrlID, wxEmptyString, pos, size, 
                                                wxTE_PASSWORD,
                                                wxTextValidator(validatorStyle, valPtr));
  ApplyPasswordFont(textCtrl);
  Add(textCtrl, wxSizerFlags().Proportion(1).Expand());
  
  ExternalKeyboardButton* vkbdButton = new ExternalKeyboardButton(parent);
  Add(vkbdButton, wxSizerFlags().Border(wxLEFT));
}

CSafeCombinationCtrl::~CSafeCombinationCtrl()
{
}
