// Copyright 2023 Ashley R. Thomas
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Adapted from https://github.com/AshleyT3/MFCmisc for Password Safe.

#include <Afxcmn.h>
#include "ProgressPieCtrl.h"

IMPLEMENT_DYNAMIC(CProgressPieCtrl, CButton)

CProgressPieCtrl::CProgressPieCtrl()
  :
  m_common(this),
  m_clrBackground(::GetSysColor(COLOR_3DFACE)),
  m_backgroundBrush(m_clrBackground),
  m_clrText(RGB(0, 0, 0))
{
}

CProgressPieCtrl::~CProgressPieCtrl()
{
}

BEGIN_MESSAGE_MAP(CProgressPieCtrl, CButton)
  ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

void CProgressPieCtrl::SetPercent(double percent)
{
  m_common.SetPercent(percent);

  CDC* pDC = GetDC();
  CBitmap newBitmap;
  m_common.PaintToBitmap(*pDC, newBitmap);
  ReleaseDC(pDC);

  SetBitmap(newBitmap);
  m_lastBitmap.DeleteObject();
  m_lastBitmap.Attach(newBitmap.Detach());

  Invalidate(FALSE);
}

HBRUSH CProgressPieCtrl::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
  pDC->SetTextColor(m_clrText);
  pDC->SetBkColor(m_clrBackground);
  return m_backgroundBrush;
}
