/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

class DboxMain; // for GetMainDlg()

class CPWPropertyPage : public CPropertyPage
{
public:
  CPWPropertyPage(UINT nID);
  virtual ~CPWPropertyPage() {delete m_pToolTipCtrl;}

  // Following override to reset idle timeout on any event
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

  enum {PP_DATA_CHANGED = 0,
        PP_UPDATE_VARIABLES,
        PP_UPDATE_PWPOLICY,
        PP_UPDATE_TIMES};

  DECLARE_DYNAMIC(CPWPropertyPage)
protected:
  DboxMain *GetMainDlg() const;
  bool InitToolTip(int Flags = TTS_BALLOON | TTS_NOPREFIX, int delayTimeFactor = 1);
  void AddTool(int DlgItemID, int ResID);
  void ActivateToolTip();
  void RelayToolTipEvent(MSG *pMsg);
  void ShowHelp(const CString &topicFile);

  CToolTipCtrl *m_pToolTipCtrl;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
