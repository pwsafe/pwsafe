/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SizeRestrictedPanel.cpp
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

#include "./SizeRestrictedPanel.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

SizeRestrictedPanel::SizeRestrictedPanel(wxWindow* parent, wxWindow* sizingParent, wxWindowID id /*= wxID_ANY*/):
              wxPanel(parent, id), m_sizingParent(sizingParent)
{
}

wxSize SizeRestrictedPanel::GetWindowSizeForVirtualSize(const wxSize& size) const
{
  wxSize bestSize(size);

  wxSize screenSize = ::wxGetClientDisplayRect().GetSize();
  wxSize currentSize = this->GetSize();
  wxSize parentSize = ::wxGetTopLevelParent(const_cast<SizeRestrictedPanel*>(this))->GetSize();
  if (currentSize.GetWidth() == 0 || currentSize.GetHeight() == 0 ||
      !IsShown() ||
      currentSize.GetWidth() > parentSize.GetWidth() || currentSize.GetHeight() > parentSize.GetHeight())
    currentSize = m_sizingParent->GetSize();

  wxSize sizeDiff = parentSize - currentSize;

  if (bestSize.GetWidth() > (screenSize.GetWidth() - sizeDiff.GetWidth())) {
//    wxLogDebug(wxT("Adjusting best width from %d, screen width is %d, parent width is %d, current width is %d, diff is %d"),
//                    bestSize.GetWidth(), screenSize.GetWidth(), parentSize.GetWidth(), currentSize.GetWidth(), sizeDiff.GetWidth());
    bestSize.SetWidth(screenSize.GetWidth()-sizeDiff.GetWidth());
  }

  if (bestSize.GetHeight() > (screenSize.GetHeight() - sizeDiff.GetHeight())) {
//    wxLogDebug(wxT("Adjusting best height from %d, screen height is %d, parent height is %d, current height is %d, diff is %d"),
//                    bestSize.GetHeight(), screenSize.GetHeight(), parentSize.GetHeight(), currentSize.GetHeight(), sizeDiff.GetHeight());
    bestSize.SetHeight(screenSize.GetHeight()-sizeDiff.GetHeight());
  }
  return bestSize;
}
