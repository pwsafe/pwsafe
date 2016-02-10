
#pragma once

#include "afxwin.h"

#include "..\VirtualKeyboard\VKBButton.h"
#include "LCHdrCtrlNoChng.h"

#include "Unicode_Characters.h"
#include "Unicode_Blocks.h"

#include "GFResizeDialogHelper.h"
#include "RichEditCtrlExtn.h"

#include "NListCtrl.h"
#include "WComboBox.h"

#include "resource.h"

class CUCPickerDlg;

enum { UNICODE_RANGE = 0, UNICODE_NAME, CURRECT_FONT };
enum { FONT_NAME = 0, PERCENTAGE, FONT_NUMCHARS };
enum { ALL = 0, USER_SELECTED, PREFFERRED, BEST_AVAILABLE, NONE_FOUND };

#define CF_USERSELECTEDFONT  RGB(255, 0, 0)   // User selected font - red
#define CF_PREFERREDFONT     RGB(0, 0, 255)   // Preferred font - blue
#define CF_BESTAVAILABLEFONT RGB(255, 128, 0) // Best Available font - orange

class CSetFontsDlg :  public CDialog
{
public:
  CSetFontsDlg(CWnd *pParent /*=NULL*/);
  ~CSetFontsDlg();

  enum { IDD = IDD_SETFONTS };

  CUCPickerDlg *m_pParent;
  CFont m_fntListCtrl, m_BoldItalicFont, m_fntDialogButtons;
  CLCHdrCtrlNoChng m_URLCHeader, m_AFLCHeader;
  CNListCtrl m_UnicodeRangeList;
  CListCtrl m_AvailableFontList;
  CWComboBox m_cbxDisplayType;
  int m_nSortedColumn1, m_nSortedColumn2;
  bool m_bSortAscending1, m_bSortAscending2;
  bool m_bRowHighlighted;
  int m_nRangeItem;
  int m_nDisplayType;

  CToolTipCtrl *m_pToolTipCtrl;
  GFResizeDialogHelper m_Resizer;
  bool m_bResizing;
  double m_dfAspectRatio;

  CVKBButton m_btnSelect, m_btnResetAll, m_btnCancel, m_btnOK;
  CBrush m_pBkBrush;
  CScrollBar m_gripper;

  static int CALLBACK SortAvailableFontsList(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange *pDX);
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  void SelectFont(const int nFontItem, const int nOldFontItem);
  void ResetFont(const int nFontItem);

  // Generated message map functions
  //{{AFX_MSG(CSetFontsDlg)
  afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO *lpMMI);
  afx_msg LRESULT OnNcHitTest(CPoint point);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
  afx_msg void OnSelectFont();
  afx_msg void OnResetAllFonts();
  afx_msg void OnDisplayChange();
  afx_msg void OnGetDispInfoRangeList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnSortUnicodeRangeList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnSortAvailableFontsList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnUnicodeRangeChanged(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnAvailableFontChanged(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnRClickURFont(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnRClickAFFont(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnCustomDrawAvailableFontsList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnCustomDrawUnicodeRangeList(NMHDR *pNMHDR, LRESULT *pResult);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  short int m_iSaveCurrentUBlockFont[NUMUNICODERANGES];
  std::wstring m_wsSaveUserFonts[NUMUNICODERANGES];
  CRichEditCtrlExtn m_RECEx;
};
