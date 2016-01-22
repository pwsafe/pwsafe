// UnicodeDlg.h : header file
//

#pragma once

#include "..\VirtualKeyboard\VKBButton.h"
#include "GroupBox.h"
#include "WComboBox.h"

#include "Unicode_Blocks.h"
#include "Unicode_Characters.h"

#include "resource.h"

#include <vector>
#include <map>
#include <string>

#define SWAP_UINT16(n) (((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8))

#define SWAP_UINT32(n) ((SWAP_UINT16((n & 0xFFFF0000) >> 16)) | ((SWAP_UINT16(n & 0x0000FFFF)) << 16))

// cmap in Big-Endian i.e. "pamc"
#define CMAP 0x70616d63

// Define default font
#define DEFAULT_FONT L"Arial Unicode MS"

/////////////////////////////////////////////////////////////////////////////
// CUCPickerDlg dialog

class CUCPickerDlg : public CDialog
{
  // Construction
public:
  CUCPickerDlg(CWnd *pParent = NULL);  // standard constructor

  ~CUCPickerDlg();

  // Dialog Data
  //{{AFX_DATA(CUCPickerDlg)
  enum { IDD = IDD_UNICODE_DIALOG };

  CWComboBox m_cboxUnicodeBlockByName, m_cboxUnicodeBlockByValue;
  CRichEditCtrl m_richedit;

  CString GetUnicodeBuffer() { return m_csBuffer; }
  CString GetUnicodeRTFBuffer() { return m_csRTFBuffer; }
  CString m_csBuffer, m_csRTFBuffer;
  int GetNumberCharacters() { return m_numcharacters; }

  MapFont2UBlock2NumChars m_mapFont2UBlock2NumChars;
  MapFont2NumChars m_mapFont2NumChars[NUMUNICODERANGES];

  int Index2Block[NUMUNICODERANGES];

  CVKBButton m_UnicodeButtons[128];
  CVKBButton m_btnN_Next, m_btnN_Prev, m_btnV_Next, m_btnV_Prev, m_btnC_Next, m_btnC_Prev;
  CVKBButton m_btnExit, m_btnSetFont, m_btnHelp, m_btnClearbuffer, m_btnBackspace;
  CGroupBox m_groupbox;

  int SetUserFont(const int iNumber, const int index, std::wstring wsFontName);

  //}}AFX_DATA
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CUCPickerDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support
  virtual BOOL PreTranslateMessage(MSG *pMsg);
  //}}AFX_VIRTUAL

  static BOOL CALLBACK EnumFontFamExProc(LOGFONT *lpelfe, NEWTEXTMETRIC *lpntme, DWORD FontType, LPVOID lParam);
  void ProcessFontCMAPTable(const wchar_t * wcFontname, char *p);

  // Implementation
  void GetInstalledFonts();
  void SetUnicodeBlockFonts();
  void CreateAllFonts();
  void SetUnicodeFont(CString cs_FontName);
  void SetUnicodeCharacters();
  void GetConfigFile();
  bool LoadUserFonts();
  bool SaveUserFonts();

  void MakeUnicodeRTFBuffer();
  void DrawBorderAndCaption(UINT nState);

  CFont m_fntWarning, m_fntRCHeadings, m_fntDefaultUnicode, m_fntByValue;
  CFont m_fntUnicode, m_fntButtons, m_fntDialogButtons;
  HICON m_hIcon;
  HDC m_hdc;
  CBrush m_pBkBrush;

  CToolTipCtrl *m_pToolTipCtrl;

  std::vector<int> m_viCharacterFonts;
  std::vector<ucode_info> m_vuinfo;
  std::vector<bool> m_vbFontProcessed;
  std::wstring m_wsUserFonts[NUMUNICODERANGES];
  std::wstring m_wsSaveUserFonts[NUMUNICODERANGES];
  std::wstring m_wsConfigFile;
  int m_numcharacters, m_offset;
  int m_currentCharacterFont;
  int m_iSaveCurrentUBlockFont[NUMUNICODERANGES];
  int iMapUBlockName2Range[NUMUNICODERANGES];
  int m_currentUBlock;
  bool m_bDoneByValue;
  bool m_bEnumFontsFirst;

  wchar_t m_wcReserved[2], m_wcUnsupported[2];

  // Generated message map functions
  //{{AFX_MSG(CUCPickerDlg)
  virtual BOOL OnInitDialog();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg BOOL OnTTNNeedText(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

  afx_msg void OnOK();
  afx_msg void OnUnicodeBlockByNameChange();
  afx_msg void OnUnicodeBlockByValueChange();
  afx_msg void OnSelectFont();
  afx_msg void OnBlockNextByName();
  afx_msg void OnBlockPrevByName();
  afx_msg void OnBlockNextByValue();
  afx_msg void OnBlockPrevByValue();
  afx_msg void OnNextChars();
  afx_msg void OnPrevChars();
  afx_msg void OnClearbuffer();
  afx_msg void OnBackspace();
  afx_msg void OnUnicodeButtonClick(UINT nID);
  afx_msg void OnSetFonts();
  afx_msg void OnHelpAbout();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};
