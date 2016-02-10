#pragma once

#include "..\VirtualKeyboard\VKBButton.h"

#include "Unicode_Blocks.h"
#include "Unicode_Characters.h"
#include "LCHdrCtrlNoChng.h"

#include "resource.h"

// CFontCoverageDlg dialog

enum { FC_UNICODE_RANGE = 0, FC_PERCENTAGE };

class CFontCoverageDlg : public CDialog
{
  DECLARE_DYNAMIC(CFontCoverageDlg)

public:
  CFontCoverageDlg(CWnd *Parent = NULL, std::wstring wsFontname = L"",
    MapFont2NumChars *pMapFont2NumChars = NULL);
  virtual ~CFontCoverageDlg();

// Dialog Data
  enum { IDD = IDD_FONTCOVERAGE };

  CListCtrl m_lcFontCoverage;
  CLCHdrCtrlNoChng m_LCHeader;
  CFont m_fntListCtrl, m_fntDialogButtons;

  CVKBButton m_btnOK;
  CBrush m_pBkBrush;

  static int CALLBACK SortFontCoverageList(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();

  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnSort(NMHDR *pNMHDR, LRESULT *pResult);

  DECLARE_MESSAGE_MAP()

private:
  MapFont2NumChars *m_pMapFont2NumChars;
  int m_numFontChars[NUMUNICODERANGES];
  std::wstring m_wsFontname;
  int m_nSortedColumn;
  bool m_bSortAscending;

  void SubitemPostPaint(LPNMLVCUSTOMDRAW pNMCD);
};
