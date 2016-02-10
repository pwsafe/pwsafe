
#pragma once

class CWComboBox : public CComboBox
{
  DECLARE_DYNCREATE(CWComboBox)

public:
  CWComboBox();      // protected constructor used by dynamic creation
  virtual ~CWComboBox();

protected:

  virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

  DECLARE_MESSAGE_MAP()

private:
  COLORREF m_clrHilight;
  COLORREF m_clrNormalText;
  COLORREF m_clrHilightText;
  COLORREF m_clrBkgnd;
};
