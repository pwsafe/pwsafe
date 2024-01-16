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

#define NOMINMAX
#include <Afxcmn.h>
#include <algorithm>
#include "ProgressPieCommon.h"

CProgressPieCommon::CProgressPieCommon(CWnd* pwndCtl)
  :
  m_pwndCtl(pwndCtl),
  m_curRatio(0),
  m_clrPie(RGB(0, 192, 255)),
  m_clrOutline(RGB(0, 0, 0)),
  m_bFullOutline(true)
{
}

void CProgressPieCommon::SetPercent(double percent)
{
  ASSERT(percent >= 0 && percent <= 100.0);
  m_curRatio = percent / 100.0;
}

int CProgressPieCommon::GetSize()
{
  CRect r;
  m_pwndCtl->GetWindowRect(&r);
  if (r.Width() != r.Height()) {
    m_pwndCtl->GetParent()->ScreenToClient(&r);
    const int size = std::min(r.Width(), r.Height());
    r.left = r.left + ((r.Width() - size) / 2);
    r.top = r.top + ((r.Height() - size) / 2);
    r.right = r.left + size;
    r.bottom = r.top + size;
    m_pwndCtl->MoveWindow(&r);
  }

  CRect rc;
  m_pwndCtl->GetClientRect(&rc);
  int size = std::min(rc.Width(), rc.Height()) - 10;
  ASSERT(size > 0);
  return size;
}


void CProgressPieCommon::PaintToDC(CDC& dcCompat)
{
  const int iMargin = 1;
  const double wholeCircle = 360.0;
  const double angle = wholeCircle - (wholeCircle * m_curRatio);
  const int xPos = iMargin;
  const int yPos = iMargin;
  const int iPenWidth = 1;
  const int iProgressSize = GetSize() - (iMargin * 2) - iPenWidth;
  const CRect rcProgress(xPos, yPos, xPos + iProgressSize, yPos + iProgressSize);
  const int radius = rcProgress.Width() / 2;
  const int cx = rcProgress.left + radius;
  const int cy = rcProgress.top + radius;
  CPen penProgressOutline(PS_SOLID, iPenWidth, m_clrOutline);

  CBrush brushBackground;
  brushBackground.CreateSolidBrush(::GetSysColor(COLOR_3DFACE));
  CRect rcClient;
  m_pwndCtl->GetClientRect(&rcClient);
  dcCompat.FillRect(rcClient, &brushBackground);

  dcCompat.BeginPath();
  CPen* oldPen = dcCompat.SelectObject(&penProgressOutline);
  dcCompat.MoveTo(cx, cy);
  dcCompat.LineTo(cx, rcProgress.top);
  dcCompat.AngleArc(cx, cy, radius, 90, static_cast<float>(angle));
  dcCompat.LineTo(cx, cy);
  dcCompat.SelectObject(&oldPen);
  dcCompat.EndPath();

  CBrush fill_brush(m_clrPie);
  CBrush* oldBrush = dcCompat.SelectObject(&fill_brush);
  dcCompat.StrokeAndFillPath();
  dcCompat.SelectObject(&oldBrush);

  if (m_bFullOutline) {
    oldPen = dcCompat.SelectObject(&penProgressOutline);
    dcCompat.MoveTo(cx, rcProgress.top);
    dcCompat.AngleArc(cx, cy, radius, 90, 360.0);
    dcCompat.SelectObject(&oldPen);
  }
}


void CProgressPieCommon::PaintToBitmap(CDC& dc, CBitmap& bitmap)
{
  CDC dcCompat;
  dcCompat.CreateCompatibleDC(&dc);
  const int iSize = GetSize();
  if (iSize <= 0)
    return;
  bitmap.CreateCompatibleBitmap(&dc, iSize, iSize);
  CBitmap* oldBitmap = dcCompat.SelectObject(&bitmap);

  PaintToDC(dcCompat);

  dcCompat.SelectObject(&oldBitmap);
}

void CProgressPieCommon::OnPaint()
{
  CPaintDC dc(m_pwndCtl); // device context for painting
  CBitmap bitmap;

  CDC dcCompat;
  dcCompat.CreateCompatibleDC(&dc);
  const int iSize = GetSize();
  bitmap.CreateCompatibleBitmap(&dc, iSize, iSize);
  CBitmap* o_bmp = dcCompat.SelectObject(&bitmap);

  PaintToDC(dcCompat);
  dc.BitBlt(0, 0, iSize, iSize, &dcCompat, 0, 0, SRCCOPY);

  dcCompat.SelectObject(&o_bmp);
}
