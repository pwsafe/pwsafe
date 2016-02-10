/*
* Copyright (c) 2009 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// VKBButton.h : header file
//-----------------------------------------------------------------------------

// Special Flat button for Virtual Keyboards
// Also, if a Push button, will show pushed state by change of colour (unless disabled)

struct ucode_info {
  ucode_info()
    : ucode(-1), len(-1), iRTF1(-1), iRTF2(-1)
  {  }

  ucode_info(const ucode_info &ui)
    : ucode(ui.ucode), len(ui.len), iRTF1(ui.iRTF1), iRTF2(ui.iRTF2)
  {}

  ucode_info &operator =(const ucode_info &ui)
  {
    if (this != &ui) {
      ucode = ui.ucode; len = ui.len; iRTF1 = ui.iRTF1; iRTF2 = ui.iRTF2;
    }
    return *this;
  }

  int ucode;
  short int len;
  short int iRTF1;
  short int iRTF2;
};

class CVKBButton : public CButton
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
  void ChangePushColour(bool bDoChangePushColour)
  { m_bDoChangePushColour = bDoChangePushColour;}
  void SetColourChanges(bool bDoColourChange)
  { m_bDoColourChange = bDoColourChange; }
  void SetButtonColour(COLORREF crefButtonColour)
  { m_crefButtonColour = crefButtonColour; }
  void SetButtonUCode(ucode_info uinfo)
  { m_uinfo = uinfo; }
  void GetButtonUCode(ucode_info& uinfo)
  { uinfo = m_uinfo; }

  bool GetDeadKeyState() { return m_bDeadKey; }

protected:
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CVKBButton)
  virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
  virtual void PreSubclassWindow();
  //}}AFX_VIRTUAL

  // Generated message map functions
  //{{AFX_MSG(CVKBButton)
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  bool m_bMouseInWindow;
  bool m_bDeadKey, m_bFlat, m_bPushed;
  bool m_bDoChangePushColour, m_bDoColourChange;

  COLORREF m_crefButtonColour;

  ucode_info m_uinfo;
};
