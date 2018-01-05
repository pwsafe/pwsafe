/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "FilterBaseDlg.h"
#include "core/PWSFilters.h"

#include "afxdtctl.h" // only needed for date/time controls

// CFilterDateDlg dialog

class CEditInt : public CEdit
{
  // Allow negative numbers
  DECLARE_DYNAMIC(CEditInt)

protected:
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

  DECLARE_MESSAGE_MAP()
};

class CFilterDateDlg : public CFilterBaseDlg
{
  DECLARE_DYNAMIC(CFilterDateDlg)

public:
  CFilterDateDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CFilterDateDlg();

  // Dialog Data
  enum { IDD = IDD_FILTER_DATE };
  CTime m_ctime1, m_ctime2;
  int m_datetype;
  int m_num1, m_num2;
  time_t m_time_t1, m_time_t2;
  bool m_add_present;
  void SetFieldType(const FieldType ft) {m_ft = ft;}

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

  afx_msg void OnCbnSelchangeDateRule();
  afx_msg void OnCancel();
  afx_msg void OnBnClickedOk();
  afx_msg void OnAbsolute();
  afx_msg void OnRelative();
  afx_msg void OnDtnDatetime1Change(NMHDR *pNotifyStruct, LRESULT *pLResult);

  DECLARE_MESSAGE_MAP()

  CComboBox m_cbxRule;
  CEditInt m_edtInteger1, m_edtInteger2;
  CDateTimeCtrl m_dtp1, m_dtp2;
  CStatic m_stcAnd, m_stcAnd2, m_stcRelativeDesc;

private:
  void AFXAPI DDV_CheckDateValid(CDataExchange* pDX,
                                 const int &num);
  void AFXAPI DDV_CheckDateValid(CDataExchange* pDX,
                                 const CTime &ctime);
  void AFXAPI DDV_CheckDates(CDataExchange* pDX,
                             const CTime &ctime1,  const CTime &ctime2);
  void AFXAPI DDV_CheckDates(CDataExchange* pDX,
                             const int &num1, const int &num2);
  void AFXAPI DDV_CheckMinMax(CDataExchange* pDX,
                              const int &num);
  FieldType m_ft;
};
