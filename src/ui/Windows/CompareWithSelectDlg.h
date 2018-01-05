/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// CompareWithSelectDlg.h
//-----------------------------------------------------------------------------

#pragma once

#include "PWDialog.h"
#include "SecString.h"
#include "CWTreeCtrl.h"

#include "os/UUID.h"

class CItemData;
class PWScore;

class CCompareWithSelectDlg : public CPWDialog
{
public:
  // default constructor
  CCompareWithSelectDlg(CWnd *pParent, CItemData *pci, PWScore *pcore,
                        CString &csProtect, CString &csAttachment);
  virtual ~CCompareWithSelectDlg();

  pws_os::CUUID GetUUID();

  enum { IDD = IDD_COMPARE_WITH };

protected:
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  afx_msg void OnDestroy();
  afx_msg void OnItemSelected(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnItemDblClick(NMHDR *pNotifyStruct, LRESULT *pLResult);

  DECLARE_MESSAGE_MAP()

private:
  void InsertItemIntoGUITree(CItemData &ci);

  CCWTreeCtrl m_cwItemTree;
  CImageList *m_pImageList;

  CSecString m_group;
  CSecString m_title;
  CSecString m_username;

  PWScore *m_pcore;
  CItemData *m_pci, *m_pSelected;
  
  CString m_csProtect, m_csAttachment;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
