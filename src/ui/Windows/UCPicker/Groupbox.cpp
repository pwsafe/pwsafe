
#include "StdAfx.h"
#include "GroupBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CGroupBox, CButton)
  //{{AFX_MSG_MAP(CGroupBox)
  ON_WM_PAINT()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CGroupBox::CGroupBox()
{
}

CGroupBox::~CGroupBox()
{
  m_pnLine.DeleteObject();
  m_brLine.DeleteObject();
}

void CGroupBox::PreSubclassWindow()
{
  LOGBRUSH lbrLine;

  // Save box dimensions
  GetWindowRect(&m_Box_Rect);
  ScreenToClient(&m_Box_Rect);

  // Create thicker pen
  m_brLine.CreateSolidBrush(RGB(0, 0, 0));
  m_brLine.GetLogBrush(&lbrLine);
  m_pnLine.CreatePen(PS_GEOMETRIC | PS_SOLID, 2, &lbrLine);

  CButton::PreSubclassWindow();
}

void CGroupBox::OnPaint()
{
  CPen *ppnOldPen;

  // Create device context
  CPaintDC dc(this);
  int nSavedDC = dc.SaveDC();

  // Select thicker pen
  ppnOldPen = dc.SelectObject(&m_pnLine);

  // Draw the box
  dc.MoveTo(m_Box_Rect.TopLeft());
  dc.LineTo(m_Box_Rect.left, m_Box_Rect.bottom);
  dc.LineTo(m_Box_Rect.BottomRight());
  dc.LineTo(m_Box_Rect.right, m_Box_Rect.top);
  dc.LineTo(m_Box_Rect.top, m_Box_Rect.left);

  // Release resources
  dc.SelectObject(ppnOldPen);
  dc.RestoreDC(nSavedDC);
}
