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

#pragma once

class CProgressPieCommon
{
public:
  CProgressPieCommon(CWnd* pwndCtl);
  void SetPercent(double percent);
  double GetPercent() const { return 100.0 * m_curRatio; }
  void SetPieColor(COLORREF clr);
  void SetOutlineColor(COLORREF clr);
  int GetSize();
  void PaintToDC(CDC& dc_compat);
  void PaintToBitmap(CDC& dc, CBitmap& bitmap);
  void OnPaint();
protected:
  COLORREF m_clrPie;
  COLORREF m_clrOutline;
  double m_curRatio;
  CWnd* m_pwndCtl;
};

