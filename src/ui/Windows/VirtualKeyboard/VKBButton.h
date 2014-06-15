/*
* Copyright (c) 2014 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// VKBButton.h : header file
//-----------------------------------------------------------------------------

/*

  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!
  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!
  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!  NO MFC CLASSES ALLOWED!!!!!

*/

// Special Flat button for Virtual Keyboards
// Also, if a Push button, will show pushed state by change of colour (unless disabled)

class CVKBButton
{
public:
  CVKBButton();
  virtual ~CVKBButton();

public:
  void SetDeadKeyState(bool bState)
  {m_bDeadKey = bState;}
  void SetFlatState(bool bState)
  {m_bFlat = bState;}
  void SetPushedState(bool bPushed)
  {m_bPushed = bPushed;}
  void ChangePushColour(bool bChangePushColour)
  {m_bChangePushColour = bChangePushColour;}

protected:
  // ClassWizard generated virtual function overrides
  void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
  friend class CVKeyBoardDlg;

  HWND m_hWnd;
  //bool m_bMouseInWindow;
  bool m_bDeadKey, m_bFlat, m_bPushed, m_bChangePushColour;
};
