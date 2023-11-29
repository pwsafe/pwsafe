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

#pragma once

#include "ProgressPieCommon.h"

class CProgressPieCtrl : public CButton
{
  DECLARE_DYNAMIC(CProgressPieCtrl)

public:
  CProgressPieCtrl();
  virtual ~CProgressPieCtrl();
protected:
  DECLARE_MESSAGE_MAP()
public:
  void SetPercent(double percent);
  double GetPercent() const { return m_common.GetPercent(); }

  void SetPieColor(COLORREF clr) { m_common.SetPieColor(clr); }
  COLORREF GetPieColor() const { return m_common.GetPieColor(); }

  void SetOutlineColor(COLORREF clr) { m_common.SetOutlineColor(clr); }
  COLORREF GetOutlineColor() const { return m_common.GetOutlineColor(); }

  void SetFullCircleOutline(bool bFullCircleOutline) { m_common.SetFullCircleOutline(bFullCircleOutline); }
  bool GetFullCircleOutline() const { return m_common.GetFullCircleOutline(); }
protected:
  CProgressPieCommon m_common;
  CBitmap m_lastBitmap;
  COLORREF m_clrBackground;
  CBrush m_backgroundBrush;
  COLORREF m_clrText;
public:
  afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
};


